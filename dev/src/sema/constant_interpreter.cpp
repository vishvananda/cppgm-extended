#include "sema/constant_interpreter.h"

#include <algorithm>
#include <cstring>

#include "lex/number_literal.h"
#include "sema/analyzer.h"
#include "sema/constant_memo.h"
#include "sema/integral_arithmetic.h"
#include "sema/string_literals.h"
#include "support/small_list.h"
#include "support/stack_budget.h"

namespace sema {

namespace {

// Annex B recommends at least 512 nested constexpr calls; the step and
// value budgets bound one evaluation's work and storage.
const size_t kCallDepth = 512;
const uint64_t kStepBudget = uint64_t(1) << 24;
const uint64_t kValueBudget = uint64_t(1) << 22;

bool StaticStorage(const VariableEntity* variable)
{
	const Scope* owner = variable->owner;
	if (owner == nullptr || owner->kind == ScopeKind::Namespace || owner->kind == ScopeKind::Class)
		return !variable->member;
	return variable->is_static || variable->is_extern || variable->is_thread_local;
}

} // namespace

ConstantInterpreter::ConstantInterpreter(Analyzer& analyzer)
	: analyzer_(analyzer), arena_(analyzer.constant_scratch_), stacks_(analyzer.constant_stacks_),
	  objects_(stacks_.objects), locals_(stacks_.locals), frames_(stacks_.frames), first_object_(objects_.size()),
	  first_local_(locals_.size()), first_frame_(frames_.size()), enclosing_swaps_(suspend_swaps()), steps_(0),
	  failure_location_(0), failure_(nullptr), constructing_(nullptr), constructing_value_(nullptr), seeking_(nullptr),
	  source_site_(kNoSourceSite), incomplete_class_(false)
{
	// The scratch arena keeps its first block across evaluations.
	if (arena_.bytes_reserved() == 0)
		arena_.allocate(1, 1);
	mark_ = arena_.mark();
}

ConstantInterpreter::~ConstantInterpreter()
{
	objects_.resize(first_object_);
	locals_.resize(first_local_);
	frames_.resize(first_frame_);
	resume_swaps(enclosing_swaps_);
	arena_.rewind(mark_);
	analyzer_.graph_.constant_steps += steps_;
}

// ------------------------------------------------------ failure and budget

bool ConstantInterpreter::fail(uint32_t location, const char* why)
{
	if (failure_ == nullptr)
	{
		failure_location_ = location;
		failure_ = why;
	}
	return false;
}

bool ConstantInterpreter::step(uint32_t location)
{
	if (++steps_ > kStepBudget)
		return fail(location, "constant evaluation exceeds its step budget");
	return stack_left(location);
}

bool ConstantInterpreter::reuse_call(uint64_t steps, size_t depth, uint32_t location)
{
	if (steps > kStepBudget - steps_)
		return fail(location, "constant evaluation exceeds its step budget");
	if (depth > kCallDepth - (frames_.size() - first_frame_))
		return fail(location, "constexpr calls nested too deeply");
	steps_ += steps;
	if (frames_.size() > first_frame_)
		frames_.back().peak_depth = std::max(frames_.back().peak_depth, frames_.size() + depth);
	return true;
}

bool ConstantInterpreter::stack_left(uint32_t location)
{
	// Out of stack the analysis around the evaluation nests too deeply (an
	// instantiation recursing without end, say): its error, as in the
	// analysis itself, not an expression that is no constant expression
	// (in a substitution, a failure that would select another candidate;
	// g++ and Clang stop such a recursion with an error too).
	analyzer_.check_stack(location);
	return true;
}

bool ConstantInterpreter::subobjects_too_deep(uint32_t depth) const
{
	char marker;
	return depth > kSubobjectDepth && stack_bytes_used(analyzer_.stack_base_, &marker) > analyzer_.stack_budget_;
}

// ------------------------------------------------------ objects and places

ConstValue ConstantInterpreter::skeleton(const Type* type)
{
	// The object's storage with every scalar indeterminate (8.5p11): an
	// array's elements, a class's bases and members, allocated at once so
	// a subobject's storage never moves.
	ConstValue value = ConstValue();
	value.kind = ValueKind::Indeterminate;
	value.type = type->unqualified;
	++steps_;
	if (type->is_reference())
		return value;
	if (IsComplex(type))
	{
		// A GNU complex object: its two parts (see constant_complex.cpp).
		value.kind = ValueKind::Array;
		value.count = 2;
		value.elements = arena_.allocate_array<ConstValue>(2);
		value.elements[0] = skeleton(type->unqualified->element);
		value.elements[1] = value.elements[0];
		return value;
	}
	if (LaneVector(type))
	{
		// A GNU vector object: its lanes (see LaneVector).
		const uint32_t lanes = static_cast<uint32_t>(type->unqualified->bound);
		value.kind = ValueKind::Array;
		value.count = lanes;
		value.elements = arena_.allocate_array<ConstValue>(lanes);
		value.elements[0] = skeleton(type->unqualified->element);
		for (uint32_t i = 1; i < lanes; ++i)
			value.elements[i] = value.elements[0];
		return value;
	}
	if (type->kind == TypeKind::Array)
	{
		value.kind = ValueKind::Array;
		value.type = type;
		const uint64_t bound = type->bound;
		if (bound > kValueBudget || steps_ + bound > kStepBudget)
		{
			steps_ = kStepBudget;
			value.count = 0;
			value.elements = nullptr;
			return value;
		}
		value.count = static_cast<uint32_t>(bound);
		value.elements = arena_.allocate_array<ConstValue>(value.count);
		for (uint32_t i = 0; i < value.count; ++i)
			value.elements[i] = skeleton(type->element);
		return value;
	}
	const ClassEntity* cls = ClassOf(type);
	if (cls == nullptr)
		return value;
	analyzer_.require_complete(cls);
	value.kind = ValueKind::Class;
	value.count = cls->base_count + cls->member_count;
	value.elements = arena_.allocate_array<ConstValue>(value.count);
	uint32_t index = 0;
	for (const BaseClass* base = cls->bases; base != nullptr; base = base->next)
		value.elements[index++] = skeleton(base->base->type);
	for (const DataMember* member = cls->members; member != nullptr; member = member->next)
		value.elements[index + member->ordinal] = skeleton(member->variable->type);
	return value;
}

bool ConstantInterpreter::new_object(const Type* type, ConstPlace& place, uint32_t location)
{
	// 5.19p2: an object the evaluation makes (a temporary, parameter or
	// variable) is of a class whose destructor is trivial, as a literal
	// type's (3.9p10): no destructor that is not constexpr runs.
	const Type* element = type;
	while (element->kind == TypeKind::Array)
		element = element->element;
	const ClassEntity* cls = element->is_reference() ? nullptr : ClassOf(element);
	if (cls != nullptr && cls->destructor != nullptr && !cls->destructor->trivial)
		return fail(location, "an object with a non-trivial destructor in a constant expression");
	size_of(type, location);
	if (leaves(type, 0) > kValueBudget)
		return fail(location, "constant evaluation exceeds its storage budget");
	ConstValue* value = static_cast<ConstValue*>(arena_.allocate(sizeof(ConstValue), alignof(ConstValue)));
	*value = skeleton(type);
	if (steps_ >= kStepBudget)
		return fail(location, "constant evaluation exceeds its storage budget");
	Object object = Object();
	object.value = value;
	object.type = type;
	object.generation = ++stacks_.generation;
	objects_.push_back(object);
	place = ConstPlace();
	place.root = RootKind::Object;
	place.object = static_cast<uint32_t>(objects_.size() - 1);
	place.generation = object.generation;
	place.type = type;
	return true;
}

ConstantInterpreter::Region ConstantInterpreter::region() const
{
	return Region{ objects_.size(), locals_.size(), arena_.mark() };
}

void ConstantInterpreter::release(const Region& region)
{
	// The region's objects end, and their storage is reused: nothing made
	// in the region outlives it (a result initializes an object made
	// before it, a pointer names its object by slot and generation). The
	// scratch is rewound within the block the region began in only, so a
	// region crossing into a new block does not free and take it again at
	// every iteration.
	if (objects_.size() > region.objects)
		objects_.resize(region.objects);
	if (locals_.size() > region.locals)
		locals_.resize(region.locals);
	if (arena_.mark().head == region.mark.head)
		arena_.rewind(region.mark);
}

size_t ConstantInterpreter::suspend_swaps()
{
	// A call's body, or a nested evaluation, may meet a node whose facts
	// conversion_call swapped: it sees the node's own facts again.
	const size_t active = stacks_.active;
	for (size_t i = active; i < stacks_.swaps.size(); ++i)
		stacks_.swaps[i].node->facts = stacks_.swaps[i].converted;
	stacks_.active = stacks_.swaps.size();
	return active;
}

void ConstantInterpreter::resume_swaps(size_t active)
{
	for (size_t i = active; i < stacks_.swaps.size(); ++i)
		stacks_.swaps[i].node->facts = stacks_.swaps[i].original;
	stacks_.active = active;
}

const ConstValue* ConstantInterpreter::literal_object(const syntax::LiteralExpr* literal, uint32_t location)
{
	// 2.14.5p8: the array of the literal's code units and its null (a
	// user-defined string literal's is the array its literal operator
	// receives, 2.14.8p5), kept with the unit's facts for every evaluation.
	const uint32_t index = analyzer_.literal_index_.find(literal);
	if (index != PointerMap::kMissing)
		return analyzer_.literal_values_[index];
	StringLiteralFacts facts;
	std::vector<uint64_t> units;
	const ExprFacts* expr = literal->facts;
	const bool user = expr != nullptr && (expr->flags & kUserLiteral) != 0;
	// A raw literal operator's argument: the number's characters (2.14.8p3).
	const bool raw = user && expr->value == kLiteralCallRaw;
	if (raw)
	{
		const NumberLiteral number = ClassifyNumber(literal->spelling.text, literal->spelling.length);
		facts.element = FundamentalType::Char;
		for (uint32_t i = 0; i < number.suffix_begin; ++i)
			units.push_back(static_cast<unsigned char>(literal->spelling.text[i]));
		units.push_back(0);
	}
	if (expr == nullptr || expr->type == nullptr || (expr->type->kind != TypeKind::Array && !user) ||
		(!raw && !StringLiteralUnits(literal->spelling, facts, units)))
	{
		fail(location, "a string literal outside constant evaluation");
		return nullptr;
	}
	const Type* array = user
		? analyzer_.types_.array(
			  analyzer_.types_.qualified(analyzer_.types_.fundamental(facts.element), kConst), units.size())
		: expr->type;
	const Type* element = array->element->unqualified;
	IntegralKind kind;
	IntegralKindOf(element, kind);
	Arena& arena = analyzer_.arena_;
	ConstValue* value = static_cast<ConstValue*>(arena.allocate(sizeof(ConstValue), alignof(ConstValue)));
	*value = ConstValue();
	value->kind = ValueKind::Array;
	value->type = array;
	value->count = static_cast<uint32_t>(units.size());
	value->elements = arena.allocate_array<ConstValue>(value->count);
	for (uint32_t i = 0; i < value->count; ++i)
		value->elements[i] = IntegerValue(element, ConvertIntegral(units[i], kind));
	analyzer_.literal_index_.set(literal, static_cast<uint32_t>(analyzer_.literal_values_.size()));
	analyzer_.literal_values_.push_back(value);
	return value;
}

bool ConstantInterpreter::same_literal(const ConstValue& a, const ConstValue& b)
{
	// Two string literals of the same code units are one object: the
	// implementation's choice (2.14.5p12), as the lowering emits them.
	if (a.root != RootKind::Literal || b.root != RootKind::Literal)
		return false;
	const ConstValue* x = literal_object(static_cast<const syntax::LiteralExpr*>(a.address.entity), 0);
	const ConstValue* y = literal_object(static_cast<const syntax::LiteralExpr*>(b.address.entity), 0);
	if (x == nullptr || y == nullptr || x->count != y->count ||
		x->type->element->unqualified != y->type->element->unqualified)
		return false;
	for (uint32_t i = 0; i < x->count; ++i)
	{
		if (x->elements[i].bits != y->elements[i].bits)
			return false;
	}
	return true;
}

const ConstValue* ConstantInterpreter::provisional_value(const VariableEntity* variable)
{
	// A static data member whose in-class initializer converted a pointer
	// to member to its class still being defined (deferred, see
	// record_constant_initializer): its value with the provisional base
	// offsets (base_offset), distinct per subobject, which this
	// evaluation's result then holds too (met_incomplete_class).
	VariableEntity* evaluated = const_cast<VariableEntity*>(variable);
	evaluated->constant = ConstantState::Evaluating;
	ConstantInterpreter inner(analyzer_);
	const ConstValue* value = nullptr;
	const bool constant = inner.initialize(variable, variable->member_initializer, value, false);
	evaluated->constant = ConstantState::None;
	if (!constant)
		return nullptr;
	incomplete_class_ = true;
	return value;
}

bool ConstantInterpreter::root_storage(
	const ConstPlace& place, bool write, ConstValue*& storage, const Type*& type, uint32_t location)
{
	switch (place.root)
	{
	case RootKind::Object:
		if (place.object >= objects_.size() || objects_[place.object].generation != place.generation)
			return fail(location, "an access to an object whose lifetime has ended");
		storage = objects_[place.object].value;
		type = objects_[place.object].type;
		return true;
	case RootKind::Variable:
	{
		const VariableEntity* variable = static_cast<const VariableEntity*>(place.entity);
		if (variable == constructing_)
		{
			storage = constructing_value_;
			type = variable->type;
			return true;
		}
		if (write)
			return fail(location, "a modification of an object whose lifetime began outside the evaluation");
		if ((variable->type->cv & kVolatile) != 0)
			return fail(location, "a read of a volatile object");
		const ConstValue* value = analyzer_.variable_value(variable);
		if (value == nullptr && variable->constant_deferred && variable->constant == ConstantState::None)
			value = provisional_value(variable);
		if (value == nullptr)
			return fail(location, "a variable is not usable in a constant expression");
		storage = const_cast<ConstValue*>(value);
		type = variable->type;
		return true;
	}
	case RootKind::Literal:
	{
		if (write)
			return fail(location, "a modification of a string literal");
		const syntax::LiteralExpr* literal = static_cast<const syntax::LiteralExpr*>(place.entity);
		const ConstValue* value = literal_object(literal, location);
		if (value == nullptr)
			return false;
		storage = const_cast<ConstValue*>(value);
		type = value->type;
		return true;
	}
	case RootKind::Temporary:
	{
		// A const one only is read (as a const variable's object).
		if (write || (place.type->cv & (kConst | kVolatile)) != kConst)
			return fail(location, "an access to a temporary that is not const, bound outside the evaluation");
		storage = const_cast<ConstValue*>(static_cast<const ConstValue*>(place.entity));
		type = storage->type;
		return true;
	}
	case RootKind::Function:
		return fail(location, "a function is not an object");
	case RootKind::TypeInfo:
		return fail(location, "an access to a std::type_info object");
	default:
		return fail(location, "an indirection through a null pointer");
	}
}

bool ConstantInterpreter::root_type(const ConstPlace& place, const Type*& type, uint32_t location)
{
	// The type of the storage a place is in, its value unread: an address's
	// arithmetic and conversions (5.7p5, 5.2.9p11) read no object, so a
	// variable not usable in constant expressions has address constants
	// too (5.19p4).
	switch (place.root)
	{
	case RootKind::Object:
		if (place.object >= objects_.size() || objects_[place.object].generation != place.generation)
			return fail(location, "an access to an object whose lifetime has ended");
		type = objects_[place.object].type;
		return true;
	case RootKind::Variable:
		type = static_cast<const VariableEntity*>(place.entity)->type;
		return true;
	case RootKind::Temporary:
		type = static_cast<const ConstValue*>(place.entity)->type;
		return true;
	case RootKind::Literal:
	{
		const ConstValue* value = literal_object(static_cast<const syntax::LiteralExpr*>(place.entity), location);
		if (value == nullptr)
			return false;
		type = value->type;
		return true;
	}
	case RootKind::Function:
		return fail(location, "a function is not an object");
	case RootKind::TypeInfo:
		return fail(location, "an access to a std::type_info object");
	default:
		return fail(location, "an indirection through a null pointer");
	}
}

namespace {

// The object types agree apart from cv-qualifiers (an array's element's).
bool SameObjectType(const Type* a, const Type* b)
{
	while (a->kind == TypeKind::Array && b->kind == TypeKind::Array)
	{
		if (a->bound != b->bound)
			return false;
		a = a->element;
		b = b->element;
	}
	return a->unqualified == b->unqualified || (a->is_reference() && a == b);
}

// An empty potentially-overlapping member has no bytes of its own: it
// lies under another member's (Analyzer::place_overlapping_member).
bool OverlappingEmpty(const DataMember* member)
{
	const ClassEntity* cls = member->overlapping ? ClassOf(member->variable->type) : nullptr;
	return cls != nullptr && cls->empty;
}

// Every scalar of a value indeterminate and no union member active: an
// inactive union member's storage as its lifetime begins (3.8p1).
void Indeterminate(ConstValue& value)
{
	value.flags = 0;
	if (value.kind != ValueKind::Array && value.kind != ValueKind::Class)
	{
		value.kind = ValueKind::Indeterminate;
		return;
	}
	if (value.kind == ValueKind::Class)
		value.extra = 0;
	for (uint32_t i = 0; i < value.count; ++i)
		Indeterminate(value.elements[i]);
}

// A member named in the object `place` designates: a union's member is the
// place's innermost union member from then on (see ConstPlace::variant; a
// union's bit-fields have no active-member record).
void VariantStep(ConstPlace& place, const VariableEntity* member)
{
	const ClassEntity* owner = static_cast<const ClassEntity*>(member->owner->entity);
	if (owner->key != TokenKind::KW_UNION || member->data_member == nullptr || member->bit_field)
		return;
	place.variant = member->data_member;
	place.variant_offset = static_cast<uint32_t>(place.offset);
}

} // namespace

ConstValue* ConstantInterpreter::descend(
	ConstValue* value, const Type* type, int64_t offset, const Type* wanted, bool write, uint32_t depth)
{
	// The subobject of type `wanted` at `offset` bytes into the object of
	// type `type` whose value is `value`: an array's element, a class's base
	// or member whose bytes contain it (an empty base may share its offset
	// with a member). A union's active member only is read; a write to
	// another member makes it the active one, its lifetime beginning.
	if (offset == 0 && SameObjectType(type, wanted))
		return value;
	if (subobjects_too_deep(depth))
		return nullptr;
	if (value->kind == ValueKind::Array)
	{
		const uint64_t size = size_of(type->element, 0);
		if (size == 0 || offset < 0)
			return nullptr;
		const uint64_t index = static_cast<uint64_t>(offset) / size;
		if (index >= value->count)
			return nullptr;
		return descend(&value->elements[index], type->element, offset - static_cast<int64_t>(index * size), wanted,
			write, depth + 1);
	}
	if (value->kind != ValueKind::Class)
		return nullptr;
	const ClassEntity* cls = ClassOf(type);
	if (cls == nullptr)
		return nullptr;
	uint32_t index = 0;
	for (const BaseClass* base = cls->bases; base != nullptr; base = base->next, ++index)
	{
		const int64_t start = static_cast<int64_t>(base->offset);
		const ClassEntity* base_class = static_cast<const ClassEntity*>(base->base);
		const int64_t size = static_cast<int64_t>(base_class->size != 0 ? base_class->size : 1);
		if (offset < start || offset >= start + size)
			continue;
		if (ConstValue* found = descend(
				&value->elements[index], base_class->type, offset - start, wanted, write, depth + 1))
			return found;
	}
	const bool is_union = cls->key == TokenKind::KW_UNION;
	if (!is_union)
	{
		// The member whose bytes hold the offset, else an empty
		// potentially-overlapping member lying there.
		int64_t start = 0;
		const DataMember* member = member_at(cls, offset, start);
		ConstValue* found = member == nullptr ? nullptr
											  : descend(&value->elements[index + member->ordinal],
													member->variable->type, offset - start, wanted, write, depth + 1);
		if (found == nullptr && (member = empty_member_at(cls, offset, wanted, start, depth)) != nullptr)
			found = descend(&value->elements[index + member->ordinal], member->variable->type, offset - start, wanted,
				write, depth + 1);
		return found;
	}
	// The active member first: a subobject of it keeps it active, though an
	// earlier member of the same type may share the offset (a union member
	// under construction, 7.1.5p4, is active before its own members are
	// initialized); then, for a write, an inactive one.
	for (int pass = 0; pass < (write ? 2 : 1); ++pass)
	{
		for (const DataMember* member = cls->members; member != nullptr; member = member->next)
		{
			if (member->bit_field || member->variable->kind != EntityKind::Variable)
				continue;
			const bool inactive = value->extra != member->ordinal + 1;
			if (inactive != (pass == 1))
				continue;
			const Type* member_type = member->variable->type;
			const int64_t start = static_cast<int64_t>(member->offset / 8);
			const int64_t size = member_type->is_reference() ? 8 : static_cast<int64_t>(size_of(member_type, 0));
			if (offset < start || offset >= start + (size != 0 ? size : 1))
				continue;
			ConstValue& slot = value->elements[index + member->ordinal];
			if (inactive)
			{
				// The member becomes the active one when it holds the
				// target (through any member of a union within it, which
				// the write below makes active in turn), its storage (its
				// skeleton, kept while inactive) made indeterminate.
				if (!holds(member_type, offset - start, wanted, depth + 1))
					continue;
				Indeterminate(slot);
				value->extra = member->ordinal + 1;
			}
			if (ConstValue* found = descend(&slot, member_type, offset - start, wanted, write, depth + 1))
				return found;
		}
	}
	return nullptr;
}

const DataMember* ConstantInterpreter::member_at(const ClassEntity* cls, int64_t offset, int64_t& start)
{
	// The non-static data member (not a bit-field) whose bytes hold
	// `offset` in a class that is not a union (its members do not overlap:
	// see member_extent): by a scan in a small class, else by binary search
	// of the class's member spans, made once per unit.
	if (cls->member_count <= 8)
	{
		for (const DataMember* member = cls->members; member != nullptr; member = member->next)
		{
			if (member->bit_field || member->variable->kind != EntityKind::Variable || OverlappingEmpty(member))
				continue;
			start = static_cast<int64_t>(member->offset / 8);
			if (offset >= start && offset < start + member_extent(member))
				return member;
		}
		return nullptr;
	}
	const SpanTable& table = span_table(cls);
	const MemberSpan* end = table.spans + table.count;
	const MemberSpan* found = std::upper_bound(
		table.spans, end, offset, [](int64_t at, const MemberSpan& span) { return at < span.start; });
	if (found == table.spans || offset >= (found - 1)->start + (found - 1)->size)
		return nullptr;
	start = (found - 1)->start;
	return (found - 1)->member;
}

int64_t ConstantInterpreter::member_extent(const DataMember* member)
{
	// A member's own bytes: a potentially-overlapping one's non-virtual
	// size (later members may lie in its tail padding; an empty one has
	// none, see OverlappingEmpty), a reference's address.
	const Type* type = member->variable->type;
	const ClassEntity* cls = member->overlapping ? ClassOf(type) : nullptr;
	const uint64_t size = type->is_reference() ? 8 : cls != nullptr ? cls->nvsize : size_of(type, 0);
	return size != 0 ? static_cast<int64_t>(size) : 1;
}

const SpanTable& ConstantInterpreter::span_table(const ClassEntity* cls)
{
	// A class's member spans by offset, its empty potentially-overlapping
	// members' apart by offset and type.
	uint32_t index = stacks_.span_index.find(cls);
	if (index != PointerMap::kMissing)
		return stacks_.span_tables[index];
	MemberSpan* spans = analyzer_.arena_.allocate_array<MemberSpan>(cls->member_count);
	MemberSpan* empties = spans;
	uint32_t count = 0, empty_count = 0;
	int64_t empty_extent = 1;
	for (const DataMember* member = cls->members; member != nullptr; member = member->next)
	{
		if (member->bit_field || member->variable->kind != EntityKind::Variable)
			continue;
		const int64_t at = static_cast<int64_t>(member->offset / 8);
		if (!OverlappingEmpty(member))
			spans[count++] = MemberSpan{ at, member_extent(member), member };
		else
		{
			if (empty_count == 0)
				empties = analyzer_.arena_.allocate_array<MemberSpan>(cls->member_count);
			const int64_t size = static_cast<int64_t>(ClassOf(member->variable->type)->size);
			empty_extent = std::max(empty_extent, size);
			empties[empty_count++] = MemberSpan{ at, size, member };
		}
	}
	std::sort(spans, spans + count, [](const MemberSpan& a, const MemberSpan& b) { return a.start < b.start; });
	std::sort(empties, empties + empty_count, [](const MemberSpan& a, const MemberSpan& b) {
		return a.start != b.start ? a.start < b.start
								  : a.member->variable->type->unqualified < b.member->variable->type->unqualified;
	});
	index = static_cast<uint32_t>(stacks_.span_tables.size());
	stacks_.span_index.set(cls, index);
	stacks_.span_tables.push_back(SpanTable{ spans, count, empties, empty_count, empty_extent });
	return stacks_.span_tables.back();
}

const DataMember* ConstantInterpreter::empty_member_at(
	const ClassEntity* cls, int64_t offset, const Type* wanted, int64_t& start, uint32_t depth)
{
	// The empty potentially-overlapping member holding a subobject of type
	// `wanted` at `offset` (member_at finds the member whose bytes it lies
	// under): by a scan in a small class, else that member itself by binary
	// search of the class's empty members, then those that may hold it.
	if (cls->member_count <= 8)
	{
		for (const DataMember* member = cls->members; member != nullptr; member = member->next)
		{
			if (member->bit_field || member->variable->kind != EntityKind::Variable || !OverlappingEmpty(member))
				continue;
			start = static_cast<int64_t>(member->offset / 8);
			if (holds(member->variable->type, offset - start, wanted, depth + 1))
				return member;
		}
		return nullptr;
	}
	const SpanTable& table = span_table(cls);
	const MemberSpan* end = table.empties + table.empty_count;
	const Type* key = wanted->unqualified;
	const MemberSpan* exact = std::lower_bound(table.empties, end, offset, [key](const MemberSpan& span, int64_t at) {
		return span.start != at ? span.start < at : span.member->variable->type->unqualified < key;
	});
	if (exact != end && exact->start == offset && exact->member->variable->type->unqualified == key)
	{
		start = offset;
		return exact->member;
	}
	const MemberSpan* first = std::lower_bound(table.empties, end, offset - table.empty_extent + 1,
		[](const MemberSpan& span, int64_t at) { return span.start < at; });
	for (const MemberSpan* span = first; span != end && span->start <= offset; ++span)
	{
		if (holds(span->member->variable->type, offset - span->start, wanted, depth + 1))
		{
			start = span->start;
			return span->member;
		}
	}
	return nullptr;
}

bool ConstantInterpreter::holds(const Type* type, int64_t offset, const Type* wanted, uint32_t depth)
{
	// Whether an object of `type` has a subobject of type `wanted` at
	// `offset` bytes (by the types alone, any union member).
	if (offset == 0 && SameObjectType(type, wanted))
		return true;
	if (subobjects_too_deep(depth) || offset < 0)
		return false;
	if (type->kind == TypeKind::Array)
	{
		const uint64_t size = size_of(type->element, 0);
		if (size == 0 || static_cast<uint64_t>(offset) / size >= type->bound)
			return false;
		return holds(type->element, offset % static_cast<int64_t>(size), wanted, depth + 1);
	}
	const ClassEntity* cls = ClassOf(type);
	if (cls == nullptr)
		return false;
	for (const BaseClass* base = cls->bases; base != nullptr; base = base->next)
	{
		const ClassEntity* base_class = static_cast<const ClassEntity*>(base->base);
		const int64_t start = static_cast<int64_t>(base->offset);
		const int64_t size = static_cast<int64_t>(base_class->size != 0 ? base_class->size : 1);
		if (offset >= start && offset < start + size && holds(base_class->type, offset - start, wanted, depth + 1))
			return true;
	}
	if (cls->key != TokenKind::KW_UNION)
	{
		int64_t start = 0;
		// A reference member is its own binding's slot (as descend finds it).
		const DataMember* member = member_at(cls, offset, start);
		if (member != nullptr &&
			(member->variable->type->is_reference() ? offset == start && SameObjectType(member->variable->type, wanted)
													: holds(member->variable->type, offset - start, wanted, depth + 1)))
			return true;
		return empty_member_at(cls, offset, wanted, start, depth) != nullptr;
	}
	for (const DataMember* member = cls->members; member != nullptr; member = member->next)
	{
		if (member->bit_field || member->variable->kind != EntityKind::Variable ||
			member->variable->type->is_reference())
			continue;
		const Type* member_type = member->variable->type;
		const int64_t start = static_cast<int64_t>(member->offset / 8);
		const int64_t size = static_cast<int64_t>(size_of(member_type, 0));
		if (offset >= start && offset < start + (size != 0 ? size : 1) &&
			holds(member_type, offset - start, wanted, depth + 1))
			return true;
	}
	return false;
}

bool ConstantInterpreter::ordered(const Type* type, int64_t a, int64_t b, uint32_t depth)
{
	// 5.9p2: whether pointers to the subobjects at offsets `a` and `b` of
	// an object of `type` have an order: elements of one array, or
	// non-static data members of one object (not a union's) with the same
	// access control, recursively; distinct base subobjects have none.
	if (a == b || subobjects_too_deep(depth))
		return true;
	if (type->kind == TypeKind::Array)
	{
		const int64_t size = static_cast<int64_t>(size_of(type->element, 0));
		if (size == 0 || a / size != b / size)
			return true;
		return ordered(type->element, a % size, b % size, depth + 1);
	}
	const ClassEntity* cls = type->is_reference() ? nullptr : ClassOf(type);
	if (cls == nullptr || cls->key == TokenKind::KW_UNION)
		return true;
	// The base or member holding each offset.
	const auto subobject = [this, cls](
							   int64_t offset, const BaseClass*& base, const DataMember*& member, int64_t& start) {
		base = nullptr;
		for (const BaseClass* step = cls->bases; step != nullptr; step = step->next)
		{
			const ClassEntity* base_class = static_cast<const ClassEntity*>(step->base);
			start = static_cast<int64_t>(step->offset);
			if (!base_class->empty && offset >= start && offset < start + static_cast<int64_t>(base_class->size))
			{
				base = step;
				return;
			}
		}
		member = member_at(cls, offset, start);
	};
	const BaseClass* base_a = nullptr;
	const BaseClass* base_b = nullptr;
	const DataMember* member_a = nullptr;
	const DataMember* member_b = nullptr;
	int64_t start_a = 0, start_b = 0;
	subobject(a, base_a, member_a, start_a);
	subobject(b, base_b, member_b, start_b);
	if (base_a != nullptr || base_b != nullptr)
		return base_a == base_b && ordered(base_a->base->type, a - start_a, b - start_b, depth + 1);
	if (member_a == nullptr || member_b == nullptr)
		return true;
	if (member_a == member_b)
		return member_a->variable->type->is_reference() ||
			ordered(member_a->variable->type, a - start_a, b - start_b, depth + 1);
	const NameEntry* entry_a = analyzer_.names_.find(cls->scope, member_a->variable->name);
	const NameEntry* entry_b = analyzer_.names_.find(cls->scope, member_b->variable->name);
	return entry_a != nullptr && entry_b != nullptr && entry_a->value != nullptr && entry_b->value != nullptr &&
		entry_a->value->access == entry_b->value->access;
}

uint64_t ConstantInterpreter::leaves(const Type* type, uint32_t depth)
{
	// The number of values an object's skeleton holds (saturating past
	// the budget), counted before any is allocated.
	uint64_t count = 1;
	if (IsComplex(type))
		return 3;
	if (LaneVector(type))
		return 1 + type->unqualified->bound;
	while (type->kind == TypeKind::Array)
	{
		count *= type->bound != 0 ? type->bound : 1;
		if (count > kValueBudget)
			return kValueBudget + 1;
		type = type->element;
	}
	const ClassEntity* cls = type->is_reference() ? nullptr : ClassOf(type);
	if (cls == nullptr || depth > 64)
		return count;
	analyzer_.require_complete(cls);
	uint64_t each = 1;
	for (const BaseClass* base = cls->bases; base != nullptr && each <= kValueBudget; base = base->next)
		each += leaves(base->base->type, depth + 1);
	for (const DataMember* member = cls->members; member != nullptr && each <= kValueBudget; member = member->next)
		each += leaves(member->variable->type, depth + 1);
	return each > kValueBudget || count * each > kValueBudget ? kValueBudget + 1 : count * each;
}

bool ConstantInterpreter::locate(const ConstPlace& place, bool write, ConstValue*& out, uint32_t location)
{
	if (place.past_end)
		return fail(location, "an access one past the end of an array");
	ConstValue* storage = nullptr;
	const Type* type = nullptr;
	if (!root_storage(place, write, storage, type, location))
		return false;
	if (place.bit_field != nullptr)
	{
		// The bit-field member of the class object at the offset.
		const ClassEntity* owner = static_cast<const ClassEntity*>(place.bit_field->owner->entity);
		ConstValue* object = descend(storage, type, place.offset, owner->type, write, 0);
		if (object == nullptr || object->kind != ValueKind::Class)
			return fail(location, "an access outside the designated object");
		out = &object->elements[owner->base_count + place.bit_field->data_member->ordinal];
		return true;
	}
	if (place.variant != nullptr)
		return locate_variant(place, storage, type, write, out, location);
	out = descend(storage, type, place.offset, place.type, write, 0);
	if (out == nullptr)
		return fail(location, "an access outside the designated object or to an inactive union member");
	return true;
}

bool ConstantInterpreter::locate_variant(
	const ConstPlace& place, ConstValue* storage, const Type* type, bool write, ConstValue*& out, uint32_t location)
{
	// The place within the union member a member access named: a write
	// makes that member the active one (its lifetime begins, 9.5p1, its
	// storage indeterminate), a read of it, or of a subobject of it, when
	// it is not active is no constant expression (5.19p2). By offset and
	// type alone another member of the same type at the same offset would
	// be taken for it.
	const DataMember* member = place.variant;
	const ClassEntity* owner = static_cast<const ClassEntity*>(member->variable->owner->entity);
	const int64_t at = static_cast<int64_t>(place.variant_offset);
	ConstValue* object = descend(storage, type, at, owner->type, write, 0);
	if (object == nullptr || object->kind != ValueKind::Class)
		return fail(location, "an access outside the designated object or to an inactive union member");
	ConstValue& slot = object->elements[owner->base_count + member->ordinal];
	if (object->extra != member->ordinal + 1)
	{
		if (!write)
			return fail(location, "a read of an inactive union member");
		Indeterminate(slot);
		object->extra = member->ordinal + 1;
	}
	out = descend(&slot, member->variable->type, place.offset - at - static_cast<int64_t>(member->offset / 8),
		place.type, write, 0);
	if (out == nullptr)
		return fail(location, "an access outside the designated object");
	return true;
}

bool ConstantInterpreter::read(const ConstPlace& place, ConstValue& out, uint32_t location)
{
	// 4.1 as 5.19p2 allows it: an object initialized within the evaluation
	// or a variable usable in constant expressions, not volatile.
	if ((place.type->cv & kVolatile) != 0)
		return fail(location, "a read of a volatile glvalue");
	ConstValue* value = nullptr;
	if (!locate(place, false, value, location))
		return false;
	if (value->kind == ValueKind::Indeterminate)
		return fail(location, "a read of an object that is not initialized");
	out = *value;
	return valid_pointer(out, location);
}

bool ConstantInterpreter::valid_pointer(const ConstValue& value, uint32_t location)
{
	// 3.7.4.2p4, 3.8: a pointer to an object of the evaluation whose
	// lifetime has ended is an invalid pointer value; using it is not a
	// constant expression.
	if (value.kind != ValueKind::Pointer || value.root != RootKind::Object)
		return true;
	if (value.count >= objects_.size() || objects_[value.count].generation != value.extra)
		return fail(location, "a pointer to an object whose lifetime has ended");
	return true;
}

bool ConstantInterpreter::write(const ConstPlace& place, const ConstValue& value, uint32_t location)
{
	ConstValue* target = nullptr;
	if (!locate(place, true, target, location))
		return false;
	if (target->kind == ValueKind::Array || target->kind == ValueKind::Class)
		copy_into(*target, value);
	else
	{
		const uint8_t flags = value.flags;
		*target = value;
		target->flags = flags;
		const DataMember* field = place.bit_field != nullptr ? place.bit_field->data_member : nullptr;
		const Type* declared = place.bit_field != nullptr ? place.bit_field->type->unqualified : nullptr;
		const bool boolean = declared != nullptr && declared->kind == TypeKind::Fundamental &&
			declared->fundamental == FundamentalType::Bool;
		if (field != nullptr && value.kind == ValueKind::Integer && field->width < 64 && !boolean)
		{
			// 9.6p1, 4.7p2-p3: a bit-field holds its width's bits, a signed
			// one's value sign-extended from them (as GCC converts).
			IntegralKind kind;
			IntegralKindOf(place.bit_field->type, kind);
			const uint64_t mask = (uint64_t(1) << field->width) - 1;
			uint64_t bits = value.bits & mask;
			if (kind.is_signed && field->width != 0 && (bits >> (field->width - 1)) != 0)
				bits |= ~mask;
			target->bits = ConvertIntegral(bits, kind);
		}
	}
	return true;
}

uint64_t ConstantInterpreter::size_of(const Type* type, uint32_t location)
{
	(void)location;
	uint64_t size = 0, alignment = 0;
	if (type->is_reference())
		return 8;
	if (type->kind == TypeKind::Function || IsVoid(type))
		return 1;
	analyzer_.object_layout(type, size, alignment);
	return size;
}

bool ConstantInterpreter::base_offset(
	const ClassEntity* derived, const ClassEntity* base, int64_t& offset, uint32_t location)
{
	// The offset of the (unambiguous, non-virtual) base class subobject.
	// A class still being defined has no layout yet (its bases are
	// complete): in its own definition (a pointer to member converted to
	// it, 4.11p2) its k-th direct base subobject takes the provisional
	// offset (k + 1) << 32, distinct per subobject, so pointers to members
	// compare as they will (5.10p2) and a 5.2.9p12 cast back loses it; a
	// value keeping one is not published (met_incomplete_class).
	base_misses_.clear();
	if (derived->complete)
		return base_search(derived, base, offset, location);
	incomplete_class_ = true;
	int64_t provisional = 0;
	for (const BaseClass* step = derived->bases; step != nullptr; step = step->next)
	{
		provisional += int64_t(1) << 32;
		const ClassEntity* next = static_cast<const ClassEntity*>(step->base);
		int64_t inner = 0;
		if (!step->is_virtual && step->base != nullptr && step->base->kind == EntityKind::Class &&
			(next == base || base_search(next, base, inner, location)))
		{
			offset = provisional + inner;
			return true;
		}
	}
	return false;
}

bool ConstantInterpreter::base_search(
	const ClassEntity* derived, const ClassEntity* base, int64_t& offset, uint32_t location)
{
	// Along a chain of classes of one base each (any depth); at a class of
	// several, its direct base (Analyzer::direct_base), else a search of
	// each base not known to miss `base` (base_misses_, per query: a
	// lattice of repeated bases costs its edges, not its paths).
	int64_t total = 0;
	const ClassEntity* cls = derived;
	while (cls != base)
	{
		++analyzer_.graph_.base_steps;
		const BaseClass* step = cls->bases;
		if (step == nullptr)
			return false;
		size_of(cls->type, location);
		if (step->next == nullptr)
		{
			if (step->is_virtual)
				return false;
			total += static_cast<int64_t>(step->offset);
			cls = static_cast<const ClassEntity*>(step->base);
			continue;
		}
		if (const BaseClass* direct = analyzer_.direct_base(cls, base))
		{
			offset = total + static_cast<int64_t>(direct->offset);
			return true;
		}
		if (!stack_left(location))
			return false;
		for (; step != nullptr; step = step->next)
		{
			const ClassEntity* next = static_cast<const ClassEntity*>(step->base);
			if (step->is_virtual || base_misses_.find(next) != PointerMap::kMissing)
				continue;
			int64_t inner = 0;
			if (base_search(next, base, inner, location))
			{
				offset = total + static_cast<int64_t>(step->offset) + inner;
				return true;
			}
			base_misses_.set(next, 1);
		}
		return false;
	}
	offset = total;
	return true;
}

bool ConstantInterpreter::to_base(ConstPlace& place, const ClassEntity* base, uint32_t location)
{
	const ClassEntity* derived = ClassOf(place.type);
	int64_t offset = 0;
	if (place.past_end)
		return fail(location, "a base class of an object one past the end of an array");
	if (derived == nullptr || !base_offset(derived, base, offset, location))
		return fail(location, "a conversion to a base class outside constant evaluation");
	place.offset += offset;
	place.type = analyzer_.types_.qualified(base->type, place.type->cv);
	return true;
}

bool ConstantInterpreter::to_derived(ConstPlace& place, const ClassEntity* derived, const Type* type, uint32_t location)
{
	// 5.2.9p2, p11: the derived object containing the base subobject, which
	// must be one (the evaluation checks the object is there).
	const ClassEntity* base = ClassOf(place.type);
	int64_t offset = 0;
	if (base == nullptr || !base_offset(derived, base, offset, location))
		return fail(location, "a conversion to a derived class outside constant evaluation");
	ConstPlace result = place;
	result.offset -= offset;
	result.type = type;
	const Type* root = nullptr;
	if (!root_type(result, root, location) || !holds(root, result.offset, derived->type, 0))
		return fail(location, "a cast to a derived class of an object that is not one");
	place = result;
	return true;
}

bool ConstantInterpreter::member_adjusted(ConstValue& value, const Type* to, bool to_base, uint32_t location)
{
	// 4.11p2: a non-null pointer to member of a base class gains the base's
	// offset in the derived class; 5.2.9p12's inverse loses it, its result
	// undefined (so no constant, 5.19p2) unless the base class contains the
	// member or is a base or derived class of the member's class.
	if (value.kind != ValueKind::MemberPointer)
		return fail(location, "a pointer to member conversion of no pointer to member");
	to = to->unqualified;
	if (value.address.entity != nullptr)
	{
		const ClassEntity* from = ClassOf(value.type->owner);
		const ClassEntity* target = ClassOf(to->owner);
		int64_t offset = 0;
		if (from == nullptr || target == nullptr ||
			!(to_base ? base_offset(from, target, offset, location) : base_offset(target, from, offset, location)))
			return fail(location, "a pointer to member conversion between unrelated classes");
		const ClassEntity* home = MemberClassOf(static_cast<const Entity*>(value.address.entity));
		if (to_base && home != target && !analyzer_.conversions_.is_base_of(home, target) &&
			!analyzer_.conversions_.is_base_of(target, home))
			return fail(location, "a pointer to member cast to a class unrelated to the member's");
		value.address.offset += to_base ? -offset : offset;
	}
	value.type = to;
	return true;
}

bool ConstantInterpreter::member_pointer_object(const syntax::BinaryExpr& expr, ConstValue& pointer, ConstPlace& out)
{
	// 5.5: `.*`'s object or the object `->*`'s pointer points to, as an
	// object of the pointer's class, then the subobject of the member's
	// class at the pointer's adjustment (which the object must contain);
	// 5.5p6: a null pointer to member designates no member.
	ConstPlace object;
	if (expr.op == TokenKind::OP_ARROWSTAR)
	{
		ConstValue address;
		if (!operand(*expr.left, address))
			return false;
		if (address.kind != ValueKind::Pointer || address.root == RootKind::Null)
			return fail(expr.location, "->* through a null pointer");
		object = PlaceOf(address, address.type->element);
	}
	else if (!object_place(*expr.left, object))
		return false;
	if (!operand(*expr.right, pointer))
		return false;
	if (pointer.kind != ValueKind::MemberPointer || pointer.address.entity == nullptr)
		return fail(expr.location, "a null pointer to member applied to an object");
	const ClassEntity* cls = ClassOf(pointer.type->owner);
	if (ClassOf(object.type) != cls && !to_base(object, cls, expr.location))
		return false;
	const ClassEntity* home = MemberClassOf(static_cast<const Entity*>(pointer.address.entity));
	if (pointer.address.offset != 0 || home != cls)
	{
		object.offset += pointer.address.offset;
		object.type = analyzer_.types_.qualified(home->type, object.type->cv);
		const Type* root = nullptr;
		if (!root_type(object, root, expr.location) || !holds(root, object.offset, home->type, 0))
			return fail(expr.location, "a pointer to member of a class the object does not contain");
	}
	out = object;
	return true;
}

bool ConstantInterpreter::member_slot(
	const ConstPlace& object, const VariableEntity* member, ConstPlace& out, uint32_t location)
{
	// The member's own storage (a reference member's binding), in the
	// class object designated by `object`: through the anonymous union or
	// struct objects holding it (9.5p5) unless `object` is one, from the
	// object's class converted to the named class holding it.
	// One past the end of an array there is no object to have members.
	if (object.past_end)
		return fail(location, "a member of an object one past the end of an array");
	const ClassEntity* home = static_cast<const ClassEntity*>(member->owner->entity);
	const ClassEntity* designated = ClassOf(object.type);
	SmallList<const VariableEntity*, 4> chain;
	while (home != designated && home->anonymous && home->storage != nullptr &&
		home->storage->entity->owner->kind == ScopeKind::Class)
	{
		const VariableEntity* storage = static_cast<const VariableEntity*>(home->storage->entity);
		chain.push_back(storage);
		home = static_cast<const ClassEntity*>(storage->owner->entity);
	}
	ConstPlace place = object;
	if (ClassOf(place.type) != home && !to_base(place, home, location))
		return false;
	const uint8_t cv = place.type->cv;
	for (size_t i = chain.size(); i-- > 0;)
	{
		uint64_t offset = 0;
		analyzer_.member_offset(chain[i], offset);
		VariantStep(place, chain[i]);
		place.offset += static_cast<int64_t>(offset);
		place.type = analyzer_.types_.qualified(chain[i]->type, cv);
	}
	VariantStep(place, member);
	if (member->bit_field)
	{
		out = place;
		out.bit_field = member;
		out.type = analyzer_.types_.qualified(member->type, member->is_mutable ? 0 : cv);
		return true;
	}
	uint64_t offset = 0;
	if (!analyzer_.member_offset(member, offset))
		return fail(location, "a member without a layout");
	out = place;
	out.offset += static_cast<int64_t>(offset);
	out.bit_field = nullptr;
	out.type = member->type->is_reference() ? member->type
											: analyzer_.types_.qualified(member->type, member->is_mutable ? 0 : cv);
	return true;
}

bool ConstantInterpreter::member_place(
	const ConstPlace& object, const VariableEntity* member, ConstPlace& out, uint32_t location)
{
	// A reference member designates its referent (8.3.2p1).
	if (!member_slot(object, member, out, location))
		return false;
	if (!member->type->is_reference())
		return true;
	ConstValue bound;
	if (!read(out, bound, location))
		return false;
	out = PlaceOf(bound, member->type->element);
	return true;
}

bool ConstantInterpreter::element_bounds(const ConstPlace& place, int64_t& first, int64_t& count, uint32_t location)
{
	// The array the designated element belongs to (its first element's
	// offset and its bound), or the object itself as an array of one
	// element (5.7p4): the innermost array of the element type containing
	// the offset.
	const Type* type = nullptr;
	if (!root_type(place, type, location))
		return false;
	const Type* wanted = place.type;
	int64_t base = 0;
	bool found = false;
	for (uint32_t depth = 0; depth < 256; ++depth)
	{
		const int64_t offset = place.offset - base;
		if (type->kind == TypeKind::Array)
		{
			const int64_t size = static_cast<int64_t>(size_of(type->element, location));
			if (size == 0 || offset < 0 || offset >= size * static_cast<int64_t>(type->bound))
				break;
			if (SameObjectType(type->element, wanted) && offset % size == 0)
			{
				first = base;
				count = static_cast<int64_t>(type->bound);
				found = true;
			}
			base += (offset / size) * size;
			type = type->element;
			continue;
		}
		if (offset == 0 && SameObjectType(type, wanted) && !found)
		{
			first = base;
			count = 1;
			return true;
		}
		const ClassEntity* cls = ClassOf(type);
		if (cls == nullptr)
			break;
		const Type* next = nullptr;
		int64_t start = 0;
		for (const BaseClass* step = cls->bases; step != nullptr && next == nullptr; step = step->next)
		{
			const ClassEntity* base_class = static_cast<const ClassEntity*>(step->base);
			const int64_t size = static_cast<int64_t>(base_class->size != 0 ? base_class->size : 1);
			if (offset >= static_cast<int64_t>(step->offset) && offset < static_cast<int64_t>(step->offset) + size &&
				!base_class->empty)
			{
				next = base_class->type;
				start = static_cast<int64_t>(step->offset);
			}
		}
		if (next == nullptr && cls->key != TokenKind::KW_UNION)
		{
			int64_t at = 0;
			const DataMember* member = member_at(cls, offset, at);
			if (member != nullptr && !member->variable->type->is_reference())
			{
				next = member->variable->type;
				start = at;
			}
		}
		for (const DataMember* member = cls->key == TokenKind::KW_UNION ? cls->members : nullptr;
			member != nullptr && next == nullptr; member = member->next)
		{
			if (member->bit_field || member->variable->kind != EntityKind::Variable ||
				member->variable->type->is_reference())
				continue;
			const int64_t at = static_cast<int64_t>(member->offset / 8);
			const int64_t size = static_cast<int64_t>(size_of(member->variable->type, location));
			if (offset >= at && offset < at + (size != 0 ? size : 1))
			{
				next = member->variable->type;
				start = at;
			}
		}
		if (next == nullptr)
			break;
		base += start;
		type = next;
	}
	return found;
}

bool ConstantInterpreter::advance(ConstValue& pointer, int64_t count, uint32_t location)
{
	// 5.7p5: within the array (or one past its end); a null pointer moves
	// by zero only.
	if (pointer.root == RootKind::Null)
		return count == 0 || fail(location, "arithmetic on a null pointer");
	if (pointer.root == RootKind::Function || pointer.type->kind != TypeKind::Pointer)
		return fail(location, "arithmetic on a pointer that designates no object");
	const Type* element = pointer.type->element;
	const int64_t size = static_cast<int64_t>(size_of(element, location));
	if (count == 0)
		return true;
	if (size == 0)
		return fail(location, "arithmetic on a pointer to an incomplete type");
	// The array of the element the pointer designates; a pointer past an
	// array's end that of the element before it.
	ConstPlace place = PlaceOf(pointer, element);
	int64_t first = 0, bound = 0;
	if (!array_of(place, size, first, bound, location))
		return false;
	const int64_t moved = (place.offset - first) / size + count;
	if (moved < 0 || moved > bound)
		return fail(location, "pointer arithmetic beyond the array's bounds");
	pointer.address.offset = first + moved * size;
	pointer.flags = static_cast<uint8_t>(moved == bound ? pointer.flags | kPastEnd : pointer.flags & ~kPastEnd);
	return true;
}

bool ConstantInterpreter::array_of(
	const ConstPlace& place, int64_t size, int64_t& first, int64_t& bound, uint32_t location)
{
	// The array a pointer's place is an element of, or one past the end of.
	ConstPlace element = place;
	if (place.past_end)
		element.offset -= size;
	element.past_end = false;
	if (!element_bounds(element, first, bound, location))
		return fail(location, "pointer arithmetic outside an array");
	return true;
}

// ---------------------------------------------------------------- values

void ConstantInterpreter::copy_into(ConstValue& target, const ConstValue& source)
{
	if ((target.kind == ValueKind::Array || target.kind == ValueKind::Class) && target.kind == source.kind &&
		target.count == source.count)
	{
		for (uint32_t i = 0; i < target.count; ++i)
			copy_into(target.elements[i], source.elements[i]);
		target.extra = source.extra;
		target.flags = source.flags;
		return;
	}
	if (target.kind == ValueKind::Array || target.kind == ValueKind::Class)
	{
		// An indeterminate source leaves each scalar indeterminate.
		for (uint32_t i = 0; i < target.count; ++i)
		{
			ConstValue none = ConstValue();
			none.kind = ValueKind::Indeterminate;
			none.type = target.elements[i].type;
			copy_into(target.elements[i], none);
		}
		return;
	}
	target = source;
}

void ConstantInterpreter::zero_into(ConstValue& target)
{
	// 8.5p5: zero-initialization: scalars zero (a null pointer), each
	// element, base and member, a union's first named member.
	if (target.kind == ValueKind::Array)
	{
		for (uint32_t i = 0; i < target.count; ++i)
			zero_into(target.elements[i]);
		return;
	}
	if (target.kind == ValueKind::Class)
	{
		const ClassEntity* cls = ClassOf(target.type);
		if (cls != nullptr && cls->key == TokenKind::KW_UNION)
		{
			for (const DataMember* member = cls->members; member != nullptr; member = member->next)
			{
				if (member->variable->name == nullptr && member->bit_field)
					continue;
				target.extra = member->ordinal + 1;
				zero_into(target.elements[cls->base_count + member->ordinal]);
				break;
			}
			return;
		}
		for (uint32_t i = 0; i < target.count; ++i)
			zero_into(target.elements[i]);
		return;
	}
	const Type* type = target.type;
	if (type == nullptr || type->is_reference())
		return;
	if (IsComplex(type))
	{
		// A complex value made here (an object's skeleton holds its parts):
		// both parts zero.
		ConstValue zero = ConstValue();
		zero.type = type->unqualified->element->unqualified;
		zero_into(zero);
		complex_value(type, zero, zero, target);
	}
	else if (LaneVector(type))
	{
		// A vector value made here: every lane zero.
		ConstValue zero = ConstValue();
		zero.type = type->unqualified->element->unqualified;
		zero_into(zero);
		vector_value(type, nullptr, 0, zero, target);
	}
	else if (IsFloating(type))
		target = FloatingValue(type, 0);
	else if (type->kind == TypeKind::MemberPointer)
		target = NullMemberPointer(type);
	else if (type->kind == TypeKind::Pointer ||
		(type->kind == TypeKind::Fundamental && type->fundamental == FundamentalType::NullptrT))
	{
		target = ConstValue();
		target.kind = ValueKind::Pointer;
		target.root = RootKind::Null;
		target.type = type;
	}
	else
		target = IntegerValue(type, 0);
	target.flags = kValueZeroed;
}

bool ConstantInterpreter::complete(const ConstValue& value) const
{
	// 7.1.5p9: every scalar subobject of a constexpr object is initialized
	// (a union's active member; unnamed bit-fields hold no value).
	if (value.kind == ValueKind::Indeterminate)
		return false;
	if (value.kind != ValueKind::Array && value.kind != ValueKind::Class)
		return true;
	const ClassEntity* cls = value.kind == ValueKind::Class ? ClassOf(value.type) : nullptr;
	if (cls != nullptr)
	{
		const bool is_union = cls->key == TokenKind::KW_UNION;
		uint32_t index = 0;
		for (const BaseClass* base = cls->bases; base != nullptr; base = base->next, ++index)
		{
			if (!complete(value.elements[index]))
				return false;
		}
		for (const DataMember* member = cls->members; member != nullptr; member = member->next)
		{
			if (member->variable->name == nullptr && member->bit_field)
				continue;
			if (is_union && value.extra != member->ordinal + 1)
				continue;
			if (!complete(value.elements[index + member->ordinal]))
				return false;
		}
		return !is_union || value.extra != 0 || cls->members == nullptr;
	}
	for (uint32_t i = 0; i < value.count; ++i)
	{
		if (!complete(value.elements[i]))
			return false;
	}
	return true;
}

bool ConstantInterpreter::publishable(const ConstValue& value) const
{
	// A constant's pointers designate objects with static storage
	// duration, string literals or functions (5.19p3's address constants;
	// a thread-local object's address is each thread's own, 3.7.2).
	if (value.kind == ValueKind::Pointer)
	{
		if (value.root == RootKind::Object)
			return false;
		if (value.root == RootKind::Variable)
		{
			const VariableEntity* variable = static_cast<const VariableEntity*>(value.address.entity);
			return StaticStorage(variable) && !variable->is_thread_local;
		}
		return true;
	}
	if (value.kind != ValueKind::Array && value.kind != ValueKind::Class)
		return true;
	for (uint32_t i = 0; i < value.count; ++i)
	{
		if (!publishable(value.elements[i]))
			return false;
	}
	return true;
}

namespace {

// A value's copy whose elements live in `arena`.
void CopyValue(ConstValue& target, const ConstValue& source, Arena& arena)
{
	target = source;
	if (source.kind != ValueKind::Array && source.kind != ValueKind::Class)
		return;
	target.elements = arena.allocate_array<ConstValue>(source.count);
	for (uint32_t i = 0; i < source.count; ++i)
		CopyValue(target.elements[i], source.elements[i], arena);
}

} // namespace

const ConstValue* ConstantInterpreter::publish(const ConstValue& value, uint32_t location)
{
	// The value copied into the semantic arena, where the unit's facts live.
	if (!publishable(value))
	{
		fail(location, "a constant designates an object without static storage duration");
		return nullptr;
	}
	ConstValue* copy = static_cast<ConstValue*>(analyzer_.arena_.allocate(sizeof(ConstValue), alignof(ConstValue)));
	CopyValue(*copy, value, analyzer_.arena_);
	return copy;
}

// ------------------------------------------------------------- entry points

bool ConstantInterpreter::evaluate(const syntax::Expr& expr, bool converted, ConstValue& out)
{
	++analyzer_.graph_.constant_evaluations;
	if (!(converted ? operand(expr, out) : rvalue(expr, out)))
		return false;
	if (out.kind == ValueKind::Indeterminate || out.kind == ValueKind::Array || out.kind == ValueKind::Class)
		return fail(expr.location, "not a constant scalar value");
	return true;
}

bool ConstantInterpreter::evaluate_after(const syntax::Expr& expr, const Conversion* conversion, ConstValue& out)
{
	++analyzer_.graph_.constant_evaluations;
	if (!converted(expr, conversion, out))
		return false;
	if (out.kind == ValueKind::Indeterminate || out.kind == ValueKind::Array || out.kind == ValueKind::Class)
		return fail(expr.location, "not a constant scalar value");
	return true;
}

bool ConstantInterpreter::static_variable_place(const ConstPlace& place) const
{
	// A place in the variable of static storage duration (not thread-local)
	// whose initialization is evaluated.
	return place.root == RootKind::Variable && constructing_ != nullptr && place.entity == constructing_ &&
		StaticStorage(constructing_) && !constructing_->is_thread_local;
}

bool ConstantInterpreter::extend_temporary(ConstPlace& place, uint32_t location)
{
	// A temporary that persists with a reference of static storage
	// duration (12.2p5): its value published with the reference's, which
	// then designates the published copy.
	if (place.object >= objects_.size() || objects_[place.object].generation != place.generation)
		return fail(location, "a reference bound to an object whose lifetime has ended");
	const ConstValue& temporary = *objects_[place.object].value;
	if (!complete(temporary))
		return fail(location, "a temporary bound to a reference leaves a subobject uninitialized");
	const ConstValue* published = publish(temporary, location);
	if (published == nullptr)
		return false;
	place.root = RootKind::Temporary;
	place.entity = published;
	place.object = 0;
	place.generation = 0;
	return true;
}

bool ConstantInterpreter::initialize(
	const VariableEntity* variable, const syntax::Initializer* initializer, const ConstValue*& out, bool transient)
{
	++analyzer_.graph_.constant_evaluations;
	const Type* type = variable->type;
	const uint32_t location = initializer != nullptr ? initializer->location : 0;
	if (type->is_reference())
	{
		if (initializer == nullptr)
			return fail(location, "a reference without an initializer");
		// A reference designates its referent (5.19p3: an object with static
		// storage duration or a function).
		const syntax::Expr* single = nullptr;
		if (initializer->kind == syntax::InitializerKind::Assign)
			single = initializer->expression;
		else if (initializer->kind == syntax::InitializerKind::Parenthesized && initializer->arguments.size == 1)
			single = initializer->arguments[0];
		if (single != nullptr && single->kind == syntax::ExprKind::BracedInit)
		{
			const syntax::BracedInitList* list = static_cast<const syntax::BracedInitList*>(single);
			single = list->elements.size == 1 ? list->elements[0] : nullptr;
		}
		ConstPlace place;
		const syntax::Expr* inner = single;
		while (inner != nullptr && inner->kind == syntax::ExprKind::Paren)
			inner = static_cast<const syntax::ParenExpr*>(inner)->inner;
		if (single == nullptr || inner->facts == nullptr || !bind(*single, inner->facts->conversion, type, place))
			return fail(location, "a reference initializer outside constant evaluation");
		// 12.2p5: the temporary it is bound to persists with the reference
		// (3.6.2p2's constant initialization of a reference bound to a
		// temporary).
		if (place.root == RootKind::Object && binds_temporary(*single, inner->facts->conversion) &&
			!extend_temporary(place, location))
			return false;
		out = publish(PointerTo(place, type), location);
		return out != nullptr;
	}
	if (leaves(type, 0) > kValueBudget)
		return fail(location, "constant evaluation exceeds its storage budget");
	ConstValue* storage = static_cast<ConstValue*>(arena_.allocate(sizeof(ConstValue), alignof(ConstValue)));
	*storage = skeleton(type);
	if (steps_ >= kStepBudget)
		return fail(location, "constant evaluation exceeds its storage budget");
	constructing_ = variable;
	constructing_value_ = storage;
	ConstPlace target = ConstPlace();
	target.root = RootKind::Variable;
	target.entity = variable;
	target.type = type;
	if (!construct(type, target, initializer, variable->constructor, location))
		return false;
	if (!complete(*storage))
		return fail(location, "a constexpr object leaves a subobject uninitialized");
	// An integer a Constant holds is the caller's to record; any other
	// value is published (a 128-bit one beyond a Constant's too).
	if (transient && storage->kind == ValueKind::Integer && (storage->flags & kValueZeroed) == 0 &&
		HeldAsConstant(*storage))
	{
		out = storage;
		return true;
	}
	out = publish(*storage, location);
	return out != nullptr;
}

} // namespace sema
