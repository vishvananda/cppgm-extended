#include "lower/function_lowering.h"

#include <algorithm>
#include <cstring>

#include "lex/identifier_table.h"
#include "lowir/literals.h"
#include "sema/aggregates.h"
#include "sema/conversions.h"
#include "sema/integral_arithmetic.h"
#include "sema/string_literals.h"

// Class aggregates and bit-fields (8.5.1, 9.6): a class aggregate's members
// initialized in declaration order, each store addressing its member again
// from the object's root (a member's value before its address, an array
// element's address before its value); bit-field members read and written
// through their storage unit with masks and shifts.
namespace lower {

namespace {

const syntax::Expr* WithoutParentheses(const syntax::Expr* expr)
{
	while (expr->kind == syntax::ExprKind::Paren)
		expr = static_cast<const syntax::ParenExpr*>(expr)->inner;
	return expr;
}

bool IsSignedType(const sema::Type* type)
{
	// A bit-field of an enumeration without a fixed underlying type holds
	// the values of the enumeration (9.6p4): negative ones only when some
	// enumerator is (7.2p7), whatever type represents them.
	const sema::Type* unqualified = type->unqualified;
	if (unqualified->kind == sema::TypeKind::Enum)
	{
		const sema::EnumEntity* entity = static_cast<const sema::EnumEntity*>(unqualified->entity);
		if (!entity->fixed)
			return entity->negative;
	}
	sema::IntegralKind kind;
	return sema::IntegralKindOf(type, kind) && kind.is_signed && kind.bits > 1;
}

// `width` one bits shifted up by `shift` (or every other bit of the unit,
// when `clear`), as a literal of the unit's type: a 128-bit unit's mask
// fills both halves.
uint64_t Ones(unsigned width)
{
	return width >= 64 ? ~uint64_t(0) : (uint64_t(1) << width) - 1;
}

lowir::Literal FieldMask(lowir::Type unit, unsigned width, unsigned shift, bool clear)
{
	const unsigned bits = lowir::integer_bits(unit.kind());
	if (bits <= 64)
	{
		const uint64_t mask = Ones(width) << shift;
		return lowir::integer_literal(static_cast<int64_t>(clear ? ~mask & Ones(bits) : mask));
	}
	uint64_t low = Ones(width);
	uint64_t high = width <= 64 ? 0 : Ones(width - 64);
	if (shift >= 64)
	{
		high = low << (shift - 64);
		low = 0;
	}
	else if (shift != 0)
	{
		high = (high << shift) | (low >> (64 - shift));
		low <<= shift;
	}
	lowir::Literal literal;
	literal.low = clear ? ~low : low;
	literal.high = clear ? ~high : high;
	return literal;
}

// A vector member's or element's braced list, or a complex one's list of
// its two parts: its lanes (GNU's list-initialization, see
// FunctionLowering::vector_list); null otherwise.
const syntax::BracedInitList* VectorLanes(const sema::Type* type, const syntax::Expr* element)
{
	if (element == nullptr || element->kind != syntax::ExprKind::BracedInit)
		return nullptr;
	const syntax::BracedInitList* list = static_cast<const syntax::BracedInitList*>(element);
	return sema::LaneList(type, *list) ? list : nullptr;
}

} // namespace

bool ProgramLowering::runs_no_constructor(const sema::Type* type, uint32_t depth)
{
	// Whether a member built in a temporary and copied by its bytes is
	// initialized as in place (8.5.1p2): no constructor that could observe
	// the object's address runs. Known answers are kept; one cut short by
	// the depth limit is not.
	while (type->kind == sema::TypeKind::Array)
		type = type->element;
	const sema::ClassEntity* entity = type->is_reference() ? nullptr : sema::ClassOf(type);
	if (entity == nullptr)
		return true;
	const uint32_t known = constructor_free_.find(entity);
	if (known != PointerMap::kMissing)
		return known == 1;
	if (depth > 64)
		return false;
	bool free = sema::IsAggregateClass(entity);
	bool cut = false;
	for (const sema::DataMember* member = entity->members; member != nullptr && free; member = member->next)
	{
		if (member->variable->name == nullptr || runs_no_constructor(member->variable->type, depth + 1))
			continue;
		free = false;
		const sema::ClassEntity* inner = FunctionLowering::object_class(member->variable->type);
		cut = inner != nullptr && constructor_free_.find(inner) == PointerMap::kMissing;
	}
	if (!cut)
		constructor_free_.set(entity, free ? 1 : 2);
	return free;
}

bool ProgramLowering::empty_list_constructs(const sema::ClassEntity* entity, uint32_t depth)
{
	// Aggregate initialization from an empty list initializes each member
	// from an empty list (8.5.1p7): a member of a class that is no
	// aggregate runs its default constructor unless that is trivial.
	const uint32_t known = empty_list_constructs_.find(entity);
	if (known != PointerMap::kMissing)
		return known == 1;
	if (depth > 64)
		return true;
	bool constructs = false;
	bool cut = false;
	for (const sema::DataMember* member = entity->members; member != nullptr && !constructs; member = member->next)
	{
		const sema::ClassEntity* cls = member->variable->type->is_reference()
			? nullptr
			: FunctionLowering::object_class(member->variable->type);
		if (cls == nullptr)
			continue;
		if (!sema::IsAggregateClass(cls))
			constructs = cls->default_constructor == nullptr || !cls->default_constructor->trivial;
		else if (empty_list_constructs(cls, depth + 1))
		{
			constructs = true;
			cut = empty_list_constructs_.find(cls) == PointerMap::kMissing;
		}
	}
	if (!cut)
		empty_list_constructs_.set(entity, constructs ? 1 : 2);
	return constructs;
}

// ------------------------------------------------------------ address paths

lowir::Value FunctionLowering::path_address(const AddressPath& path)
{
	// The root addressed again, then each member and element step.
	lowir::Value address;
	switch (path.root)
	{
	case AddressPath::Root::Storage:
	{
		Place place = Place();
		place.storage = path.base;
		address = address_of(place);
		break;
	}
	case AddressPath::Root::This:
		address = this_pointer(0);
		break;
	default:
		address = path.base;
		break;
	}
	const lowir::Type i64 = lowir::Type::scalar(lowir::TypeKind::I64);
	for (size_t i = path.first_step; i < path_steps_.size(); ++i)
	{
		const PathStep& step = path_steps_[i];
		switch (step.kind)
		{
		case PathStep::Kind::Field:
			address = index(
				lowir::Type::scalar(lowir::TypeKind::I8), ir_model::IPK_FIELD, address, integer(i64, step.amount))
						  .value;
			if (step.untyped)
				mark_untyped(address);
			break;
		case PathStep::Kind::Base:
			address = index(
				lowir::Type::scalar(lowir::TypeKind::I8), ir_model::IPK_NONE, address, integer(i64, step.amount))
						  .value;
			break;
		case PathStep::Kind::Element:
			address = index(step.element, ir_model::IPK_ARRAY_ELEMENT, address, integer(i64, step.amount)).value;
			break;
		case PathStep::Kind::ScaledElement:
			address = element_address(address, step.amount, step.size);
			break;
		}
	}
	return address;
}

FunctionLowering::PathStep FunctionLowering::element_step(const sema::Type* element, uint32_t location)
{
	// A class element's index is scaled to its byte offset (as a subscript
	// is); a scalar element is indexed in its LowIR type.
	if (element->kind != sema::TypeKind::Array && sema::ClassOf(element) != nullptr)
		return PathStep{ PathStep::Kind::ScaledElement, 0, lowir::Type(), lowering_.size_of(element, location) };
	return PathStep{ PathStep::Kind::Element, 0, lowering_.storage_type(element), 0 };
}

// ------------------------------------------------------- class aggregates

bool FunctionLowering::aggregate_stores(const sema::ClassEntity* entity)
{
	// Whether initializing the aggregate stores anything: every member but
	// one of empty class type does.
	for (const sema::DataMember* member = entity->members; member != nullptr; member = member->next)
	{
		if (member->bit_field && member->variable->name == nullptr)
			continue;
		const sema::Type* type = member->variable->type;
		const sema::ClassEntity* cls = type->is_reference() ? nullptr : object_class(type);
		if (cls == nullptr || !sema::IsAggregateClass(cls) || aggregate_stores(cls))
			return true;
	}
	return false;
}

void FunctionLowering::aggregate_class(const sema::ClassEntity* entity, uint8_t cv, const AddressPath& path,
	const syntax::BracedInitList* list, uint32_t& next, uint32_t location)
{
	// 8.5.1p2: the members from the list's elements in declaration order,
	// unnamed bit-fields skipped (9.6p2); the others value-initialized
	// (8.5.1p7).
	bit_unit_ = 0;
	bit_high_ = 0;
	// Its own units (an aggregate's members lie within its data).
	const uint64_t saved_limit = bit_limit_;
	bit_limit_ = UINT64_MAX;
	// In a constructor (from `this`) each member's address comes before its
	// value; elsewhere a member's value comes first.
	const bool value_first = path.root != AddressPath::Root::This;
	// 8.5.1p15: a union's one member (see sema::UnionListMember).
	const sema::DataMember* selected = entity->key == TokenKind::KW_UNION ? sema::ListUnionMember(entity, list, next)
																		  : nullptr;
	for (const sema::DataMember* member = entity->members; member != nullptr; member = member->next)
	{
		const sema::VariableEntity* variable = static_cast<const sema::VariableEntity*>(member->variable);
		if ((variable->name == nullptr && member->bit_field) || (selected != nullptr && member != selected))
			continue;
		// A volatile object's members are volatile.
		const sema::Type* type = lowering_.graph().types.qualified(
			variable->type, static_cast<uint8_t>(cv & sema::kVolatile));
		if (member->bit_field)
		{
			const syntax::Expr* element = list != nullptr && next < list->elements.size ? list->elements[next++]
																						: nullptr;
			if (element != nullptr && element->kind == syntax::ExprKind::BracedInit)
			{
				const syntax::BracedInitList* inner = static_cast<const syntax::BracedInitList*>(element);
				element = inner->elements.empty() ? nullptr : inner->elements[0];
			}
			initialize_bit_field(member, variable, path, element, value_first, location, cv);
			bit_high_ = member_end_bits(member, location);
			if (entity->key == TokenKind::KW_UNION)
				break;
			continue;
		}
		bit_unit_ = 0;
		path_steps_.push_back(PathStep{ PathStep::Kind::Field, lowering_.member_offset(variable, location),
			lowir::Type(), 0, through_union(variable, entity) });
		{
			const OverlapScope overlap(overlapping_empty_, member);
			aggregate_item(type, path, list, next, value_first, location);
		}
		path_steps_.pop_back();
		bit_unit_ = 0;
		bit_high_ = member_end_bits(member, location);
		if (entity->key == TokenKind::KW_UNION)
			break;
	}
	bit_unit_ = 0;
	bit_limit_ = saved_limit;
}

uint64_t FunctionLowering::member_end_bits(const sema::DataMember* member, uint32_t location)
{
	// The bit after a member in its class.
	const sema::VariableEntity* variable = static_cast<const sema::VariableEntity*>(member->variable);
	if (member->bit_field)
		return lowering_.member_bit_offset(variable, location) + member->width;
	const uint64_t size = variable->type->is_reference() ? 8 : lowering_.size_of(variable->type, location);
	return (lowering_.member_offset(variable, location) + size) * 8;
}

void FunctionLowering::aggregate_item(const sema::Type* type, const AddressPath& path,
	const syntax::BracedInitList* list, uint32_t& next, bool value_first, uint32_t location)
{
	// One member or element at the path's address from the list at `next`
	// (8.5.1p11: a subaggregate an expression does not initialize takes
	// its elements from this list). A scalar member's value comes before
	// its address, an array element's address before its value.
	if (element_count_ != nullptr && type->kind != sema::TypeKind::Array)
	{
		// An element of the array being counted: made (an aggregate's
		// members tracked), then counted.
		ElementCount* const count = element_count_;
		element_count_ = nullptr;
		const size_t first = cleanups_.size();
		const sema::ClassEntity* cls = type->is_reference() ? nullptr : sema::ClassOf(type);
		const size_t saved = aggregate_members_;
		aggregate_members_ = cls != nullptr && sema::IsAggregateClass(cls) ? first : SIZE_MAX;
		member_regions_ = false;
		counted_item_ = true;
		aggregate_item(type, path, list, next, value_first, location);
		counted_item_ = false;
		element_made(*count, first);
		aggregate_members_ = saved;
		member_regions_ = false;
		element_count_ = count;
		return;
	}
	const syntax::Expr* element = list != nullptr && next < list->elements.size ? list->elements[next] : nullptr;
	if (type->kind == sema::TypeKind::Array)
	{
		if (element != nullptr && element->kind == syntax::ExprKind::BracedInit)
		{
			++next;
			aggregate_array(type, path, element, location);
			return;
		}
		if (element != nullptr && sema::InitializesSubaggregate(type, *element))
		{
			++next;
			aggregate_array(type, path, element, location);
			return;
		}
		aggregate_elements(type, path, element != nullptr ? list : nullptr, next, location);
		return;
	}
	const sema::ClassEntity* cls = type->is_reference() ? nullptr : sema::ClassOf(type);
	if (cls != nullptr)
	{
		aggregate_class_item(cls, type, path, list, next, location,
			path_steps_.size() > path.first_step && path_steps_.back().kind == PathStep::Kind::Element);
		return;
	}
	if (element != nullptr)
		++next;
	// A reference's list binds its one element, or is the temporary the
	// list initializes (8.5.4p3, see reference_initializer); a vector's
	// list gives its lanes (see vector_list).
	const syntax::BracedInitList* lanes = VectorLanes(type, element);
	if (element != nullptr && type->is_reference())
		element = reference_initializer(element);
	else if (element != nullptr && element->kind == syntax::ExprKind::BracedInit)
	{
		const syntax::BracedInitList* inner = static_cast<const syntax::BracedInitList*>(element);
		element = inner->elements.empty() ? nullptr : inner->elements[0];
	}
	const lowir::Type storage = type->is_reference() ? lowir::Type::scalar(lowir::TypeKind::Ptr)
													 : lowering_.storage_type(type);
	Place place = Place();
	qualify(place, type);
	// Once a tracked aggregate's member is made, a later scalar member's
	// address comes first (the course's construction form).
	for (size_t i = aggregate_members_; value_first && aggregate_members_ != SIZE_MAX && i < cleanups_.size(); ++i)
		value_first = !cleanups_[i].member;
	if (!value_first)
		place.storage = path_address(path);
	RValue value;
	if (type->is_reference())
	{
		if (element == nullptr)
			throw LoweringError(location, "a reference member without an initializer");
		value.value = aggregate_extension_ != nullptr
			? bind_extended(*element, type, aggregate_extension_)
			: bind_reference(*element, type, ConversionContext::Initialization, static_object_);
		value.type = storage;
	}
	else if (lanes != nullptr)
		value = vector_list(type, *lanes);
	else if (element == nullptr)
		value = zero_value(type);
	else if (path_steps_.size() > path.first_step && path_steps_.back().kind == PathStep::Kind::Element)
	{
		// An element of a member array: its literal conversions fold.
		value = fit(operand(*element, ConversionContext::Initialization), type);
	}
	else
		value = fit(member_operand(*element), type);
	if (value_first)
		place.storage = path_address(path);
	store(storage, value, place);
}

void FunctionLowering::aggregate_class_item(const sema::ClassEntity* cls, const sema::Type* type,
	const AddressPath& path, const syntax::BracedInitList* list, uint32_t& next, uint32_t location, bool array_element)
{
	// A class member or element: an aggregate member by member (from its
	// own list, or this one's elements); a class with constructors by the
	// constructor its element selects, its default constructor without one
	// (value-initialization), or the class prvalue that initializes it.
	const syntax::Expr* element = list != nullptr && next < list->elements.size ? list->elements[next] : nullptr;
	if (sema::IsAggregateClass(cls))
	{
		// A tracked aggregate's aggregate member, once made, is one object
		// (see member_made); an element of a counted array is counted.
		const bool unit = aggregate_members_ != SIZE_MAX && !counted_item_ && needs_destruction(type);
		counted_item_ = false;
		const size_t first = cleanups_.size();
		if (element != nullptr && element->kind == syntax::ExprKind::BracedInit)
		{
			++next;
			uint32_t inner = 0;
			aggregate_class(cls, type->cv, path, static_cast<const syntax::BracedInitList*>(element), inner, location);
		}
		else if (element != nullptr && sema::InitializesSubaggregate(type, *element))
		{
			++next;
			materialize_member(*element, path, location);
		}
		else if (element != nullptr)
			aggregate_class(cls, type->cv, path, list, next, location);
		else
		{
			uint32_t none = 0;
			aggregate_class(cls, type->cv, path, nullptr, none, location);
		}
		if (unit)
			member_made(type, path_address(path), first);
		return;
	}
	counted_item_ = false;
	const bool tracked = aggregate_members_ != SIZE_MAX;
	// (The previous region ends, the member's address is taken, then its
	// region opens; one no member joined since stays open.)
	bool region = tracked && member_may_throw(cls, element);
	if (region && region_open_ && cleanups_.size() == region_depth_ && !stale_region_ &&
		(open_serial_ > aggregate_serial_ || reopened_construction_))
		region = false;
	if (region)
	{
		close_region();
		eager_pending_ = false;
	}
	const lowir::Value object = path_address(path);
	if (region)
	{
		open_region(false, true);
		member_regions_ = member_regions_ || region_open_;
	}
	const syntax::BracedInitList* listed = element != nullptr ? list_object(*element) : nullptr;
	if (element == nullptr)
	{
		// An omitted element's default construction ends its temporaries
		// before the next element (12.2p4). No call inside
		// opens the alive objects' region: one that may throw opens it
		// first (the elements made so far, the objects before them).
		bool throws = false;
		const bool temporaries = array_element && element_temporaries(cls->default_constructor, &throws);
		if (temporaries && throws)
			before_throwing_call();
		const ElementScope scope = begin_element_temporaries(temporaries, true);
		value_initialize(type, object, cls->default_constructor, EntryVariant::Complete, location);
		end_element_temporaries(scope);
	}
	else if (listed != nullptr && element->kind == syntax::ExprKind::BracedInit)
	{
		// A std::initializer_list member's array lives as long as a
		// reference member's temporary would (8.5.4p6, 12.2p5).
		++next;
		initializer_list_object(
			*listed, object, cls, aggregate_extension_, aggregate_extension_ == nullptr && static_object_, location);
	}
	else if (element->kind == syntax::ExprKind::BracedInit)
	{
		++next;
		syntax::Initializer view = syntax::Initializer();
		view.kind = syntax::InitializerKind::Braced;
		view.expression = const_cast<syntax::Expr*>(element);
		view.location = element->location;
		const sema::Entity* selected = element->facts != nullptr ? element->facts->entity : nullptr;
		construct(
			type, object, &view, static_cast<const sema::FunctionEntity*>(selected), EntryVariant::Complete, location);
	}
	else
	{
		++next;
		materialize_member(*element, path, location, object);
	}
	if (tracked && needs_destruction(type))
	{
		cleanups_.push_back(Cleanup{ object, type, nullptr, lowir::kNoId, lowir::kNoId, lowir::kNoId, false, false,
			lowir::kNoId, false, 0, 0, 0, true });
		// The open region's landing does not destroy it: it ends here (a
		// later throwing call opens one that does).
		if (region_open_)
			close_region();
	}
}

bool FunctionLowering::member_may_throw(const sema::ClassEntity* cls, const syntax::Expr* element)
{
	// A class member's initialization from its element (or by its default
	// constructor) may throw.
	if (element != nullptr)
		return may_throw(element);
	const sema::FunctionEntity* constructor = cls->default_constructor;
	return constructor != nullptr && !constructor->trivial && call_may_throw(constructor);
}

void FunctionLowering::materialize_member(
	const syntax::Expr& element, const AddressPath& path, uint32_t location, lowir::Value address)
{
	// A member copy-initialized from an element (13.3.1.4 by a converting
	// constructor, 12.8 by a copy or move of its class), or a class
	// prvalue initializing it in place (its constructor called even when
	// its body is empty: the arguments and by-value parameters' lifetimes
	// remain).
	const syntax::Expr* e = WithoutParentheses(&element);
	const sema::Conversion* conversion = e->facts->conversion;
	const lowir::Value object = address.is(lowir::ValueKind::None) ? path_address(path) : address;
	if ((conversion != nullptr && conversion->constructor != nullptr) ||
		e->facts->category != sema::ValueCategory::PRValue)
	{
		initialize_from(element, conversion, object, location);
		return;
	}
	materialize(*e, object, location);
}

bool FunctionLowering::does_nothing(const sema::FunctionEntity* constructor)
{
	// A user-provided constructor whose body and subobject initializations
	// have no effect (a polymorphic class's stores its table pointer).
	if (!constructor->constructor || constructor->body == nullptr || constructor->body->ctor_initializer != nullptr ||
		!constructor->body->body->items.empty() || sema::ClassOf(constructor->owner->entity->type)->dynamic)
		return false;
	for (uint32_t i = 0; i < constructor->init_count; ++i)
	{
		const sema::SubobjectInit& init = constructor->inits[i];
		if (init.mem_initializer != nullptr || init.default_initializer != nullptr ||
			(init.constructor != nullptr && !init.constructor->trivial))
			return false;
	}
	return true;
}

void FunctionLowering::aggregate_array(
	const sema::Type* type, const AddressPath& path, const syntax::Expr* item, uint32_t location)
{
	// An array member element by element from its own braced list (or a
	// string literal's code units for a character array, 8.5.2): each
	// element's address (the member's, then the element's index), then its
	// value. (An array of character arrays from one string literal takes it
	// for its first row, by brace elision.)
	const sema::Type* element = type->element;
	const syntax::Expr* literal = item != nullptr ? WithoutParentheses(item) : nullptr;
	const syntax::BracedInitList* list = nullptr;
	if (literal != nullptr && literal->kind == syntax::ExprKind::BracedInit)
	{
		list = static_cast<const syntax::BracedInitList*>(literal);
		const syntax::Expr* single = list->elements.size == 1 ? WithoutParentheses(list->elements[0]) : nullptr;
		if (single != nullptr && single->kind == syntax::ExprKind::Literal && sema::IsCharacterArrayElement(element) &&
			single->facts->type->kind == sema::TypeKind::Array)
		{
			literal = WithoutParentheses(list->elements[0]);
			list = nullptr;
		}
		else
			literal = nullptr;
	}
	const lowir::Type element_storage = lowering_.storage_type(element);
	if (literal == nullptr)
	{
		uint32_t next = 0;
		aggregate_elements(type, path, list, next, location);
		return;
	}
	std::vector<uint64_t> units;
	sema::StringLiteralFacts facts;
	if (literal->kind != syntax::ExprKind::Literal ||
		!sema::StringLiteralUnits(static_cast<const syntax::LiteralExpr*>(literal)->spelling, facts, units))
		throw LoweringError(location, "an array member initialized from an expression");
	for (uint64_t i = 0; i < type->bound; ++i)
	{
		path_steps_.push_back(PathStep{ PathStep::Kind::Element, i, element_storage, 0 });
		Place place = Place();
		place.storage = path_address(path);
		path_steps_.pop_back();
		qualify(place, element);
		store(element_storage, integer(element_storage, i < units.size() ? units[static_cast<size_t>(i)] : 0), place);
	}
}

void FunctionLowering::aggregate_elements(const sema::Type* type, const AddressPath& path,
	const syntax::BracedInitList* list, uint32_t& next, uint32_t location)
{
	// Each element at its index from the list at `next`. A local
	// aggregate's or a constructor's member array of destructible class
	// objects whose elements may throw counts those made (15.2p2, see
	// push_counted); a local aggregate's later members then destroy it
	// whole.
	ElementCount count = ElementCount();
	size_t counted = SIZE_MAX;
	lowir::Value storage = lowir::Value();
	const size_t saved_members = aggregate_members_;
	const bool saved_regions = member_regions_;
	ElementCount* const parent = element_count_;
	if (parent != nullptr && parent->rows)
	{
		// A row of a counted array (see begin_row).
		const size_t row = begin_row(type, path_address(path), count);
		const PathStep step = element_step(type->element, location);
		for (uint64_t i = 0; i < type->bound; ++i)
		{
			path_steps_.push_back(step);
			path_steps_.back().amount = i;
			aggregate_item(type->element, path, list, next, false, location);
			path_steps_.pop_back();
		}
		end_row(parent, count, row);
		return;
	}
	if (element_count_ == nullptr && (aggregate_members_ != SIZE_MAX || path.root == AddressPath::Root::This) &&
		counts_elements(type, list))
	{
		storage = path_address(path);
		counted = push_counted(type, storage, count);
		count.rows = type->element->kind == sema::TypeKind::Array && list != nullptr && creates_temporaries(list);
		element_count_ = &count;
		aggregate_members_ = SIZE_MAX;
	}
	const sema::Type* innermost = type->element;
	while (innermost->kind == sema::TypeKind::Array)
		innermost = innermost->element;
	const bool member = path_steps_.size() > path.first_step && path_steps_.back().kind == PathStep::Kind::Field;
	if (member && !innermost->is_reference() && sema::ClassOf(innermost) != nullptr)
	{
		// A member array's class elements are constructed at their final
		// addresses, each derived from the array's (the course's form, as an
		// automatic array's elements are).
		const lowir::Value base = counted != SIZE_MAX ? storage : path_address(path);
		for (uint64_t i = 0; i < type->bound; ++i)
			structured_item(type->element, structured_element(base, type->element, i, location, false), list, next,
				location, false, true);
	}
	else
	{
		const PathStep step = element_step(type->element, location);
		for (uint64_t i = 0; i < type->bound; ++i)
		{
			path_steps_.push_back(step);
			path_steps_.back().amount = i;
			aggregate_item(type->element, path, list, next, false, location);
			path_steps_.pop_back();
		}
	}
	if (counted == SIZE_MAX)
		return;
	element_count_ = nullptr;
	aggregate_members_ = saved_members;
	member_regions_ = saved_regions;
	end_counted(counted);
	if (aggregate_members_ != SIZE_MAX)
	{
		cleanups_.push_back(Cleanup{ storage, type, nullptr, lowir::kNoId, lowir::kNoId, lowir::kNoId, false });
		cleanups_.back().member = true;
	}
}

RValue FunctionLowering::member_operand(const syntax::Expr& expr)
{
	// A member's initializer converted to its type (8.5.1p2: copy-
	// initialized, perhaps by a conversion function, 12.3.2): a literal
	// narrowed to another signedness keeps its conversion (the course's
	// aggregate member convention).
	return operand(expr, ConversionContext::MemberInitialization);
}

void FunctionLowering::aggregate_object(const sema::Type* type, const AddressPath& path,
	const syntax::BracedInitList* list, bool root_address, uint32_t location)
{
	// A class aggregate from a braced list; a named object's address is
	// taken first when anything is stored.
	const sema::ClassEntity* entity = sema::ClassOf(type);
	if (!aggregate_stores(entity))
		return;
	if (root_address)
		path_address(path);
	// A local's members: each one constructed is destroyed by an exception
	// a later one's initialization throws (15.2p2), in a region of its own
	// (the course's form); the object owns them once complete.
	const size_t saved_members = aggregate_members_;
	const bool saved_regions = member_regions_;
	const bool saved_landing = member_landing_;
	const uint32_t saved_serial = aggregate_serial_;
	// So are a constructor's member's, and a host object's materialized
	// object's (a temporary's, a new-expression's), when one may throw
	// (15.2p2); none is tracked when no member's initialization may throw
	// (a hidden region's calls count). A block-scope static's are (its
	// initialization is retried, 6.7p4); a namespace-scope static's
	// initialization has no regions (see begin_static_initializer).
	bool tracking = root_address && !static_initializer_ && (may_throw(list) || omitted_may_throw(entity, 0));
	if (!tracking &&
		(path.root == AddressPath::Root::This || (path.root == AddressPath::Root::Value && !static_initializer_)))
	{
		const bool hidden = hidden_eh_;
		hidden_eh_ = false;
		tracking = may_throw(list) || omitted_may_throw(entity, 0);
		hidden_eh_ = hidden;
	}
	// A tracked full-expression's temporaries made before the construction
	// are no objects of its chains (the course's construction cleanup):
	// set aside meanwhile, then back under the ones it made.
	std::vector<Cleanup> aside;
	const size_t aside_at = tracked_.floor;
	if (tracking && tracked_.active && aggregate_members_ == SIZE_MAX && cleanups_.size() > aside_at)
	{
		bool temporaries = true;
		for (size_t i = aside_at; i < cleanups_.size() && temporaries; ++i)
			temporaries = cleanups_[i].temporary && cleanups_[i].guarded;
		if (temporaries)
		{
			aside.assign(cleanups_.begin() + static_cast<std::ptrdiff_t>(aside_at), cleanups_.end());
			cleanups_.resize(aside_at);
		}
	}
	if (tracking)
	{
		// A temporary's construction starts with the full-expression's
		// region, empty (the course's form).
		if (path.root == AddressPath::Root::Value && full_expression_ && !region_open_ && explicit_regions_ == 0)
		{
			open_region(true, true);
			close_region();
		}
		// (A tracked full-expression's construction cleanup from here on.)
		if (tracked_.active && full_expression_)
			tracked_.construction = true;
		aggregate_members_ = cleanups_.size();
		member_regions_ = false;
		member_landing_ = false;
		aggregate_serial_ = regions_opened_;
	}
	uint32_t next = 0;
	// An object at a computed address (a temporary, a result) has each
	// member's address derived once from it, before the member's value
	// (the course's form, as for a new-expression's object).
	if (path.root == AddressPath::Root::Value && path.first_step == path_steps_.size())
		structured_class(entity, type->cv, path.base, list, next, location);
	else
		aggregate_class(entity, type->cv, path, list, next, location);
	if (tracking)
	{
		// A named object whose members' landings destroyed members takes
		// its address again once complete (the course's form).
		if (member_landing_ && path.root == AddressPath::Root::Storage)
			completion_address_ = path_address(AddressPath{ path.root, path.base, path_steps_.size() });
		if (member_regions_ || cleanups_.size() > aggregate_members_)
			close_region();
		drop_member_cleanups(aggregate_members_);
	}
	if (!aside.empty())
	{
		cleanups_.insert(cleanups_.begin() + static_cast<std::ptrdiff_t>(aside_at), aside.begin(), aside.end());
		for (size_t i = aside_at; i < cleanups_.size(); ++i)
		{
			Cleanup& cleanup = cleanups_[i];
			cleanup.unwind = cleanup.shared_return = cleanup.conditional = lowir::kNoId;
			cleanup.eh_marked = cleanup.eh_unmarked = 0;
		}
	}
	aggregate_members_ = saved_members;
	member_regions_ = saved_regions;
	member_landing_ = saved_landing;
	aggregate_serial_ = saved_serial;
}

void FunctionLowering::drop_member_cleanups(size_t first)
{
	// The members leave the chain; the temporaries their initializers made
	// stay until the full-expression ends (12.2p3), or as long as the
	// object (12.2p5), their chain blocks made again where they led to a
	// member's.
	size_t kept = first;
	bool below = false;
	for (size_t i = first; i < cleanups_.size(); ++i)
	{
		if (cleanups_[i].member || cleanups_[i].counted || cleanups_[i].deallocator != nullptr)
		{
			below = true;
			continue;
		}
		Cleanup cleanup = cleanups_[i];
		if (below)
		{
			cleanup.unwind = cleanup.shared_return = cleanup.conditional = lowir::kNoId;
			cleanup.eh_marked = cleanup.eh_unmarked = 0;
		}
		cleanups_[kept++] = cleanup;
	}
	cleanups_.resize(kept);
}

void FunctionLowering::element_made(ElementCount& count, size_t first)
{
	// The members an aggregate element's initialization tracked leave the
	// chain (the region holding them ends); the count covers the element
	// once the next one may throw.
	bool members = member_regions_;
	for (size_t i = first; i < cleanups_.size() && !members; ++i)
		members = cleanups_[i].member;
	if (members)
	{
		close_region();
		drop_member_cleanups(first);
	}
	if (++count.made < count.total)
	{
		const lowir::Type i64 = lowir::Type::scalar(lowir::TypeKind::I64);
		Place slot = Place();
		slot.storage = lowir::Value::slot(count.slot);
		store(i64, integer(i64, count.made), slot);
	}
}

// ---------------------------------------------- arrays of class aggregates

void FunctionLowering::aggregate_array_object(const sema::Type* type, const Place& place,
	const syntax::BracedInitList& list, const sema::VariableEntity* variable, uint32_t location)
{
	// An automatic array of class aggregates, its address taken once: each
	// element by a call of the class's initialization helper when every
	// element gives each scalar member an initializer; otherwise every
	// element member by member, each address derived once from its
	// enclosing object's. The helper evaluates an element's values before
	// storing them, so an initializer that reads the array itself (the
	// clauses are sequenced with their initializations, 8.5.4p4) takes the
	// member by member form.
	const lowir::Value base = address_of(place);
	const sema::Type* element = type->element;
	const sema::ClassEntity* entity = element->kind == sema::TypeKind::Array ? nullptr : sema::ClassOf(element);
	// 15.2p2 (1.8p2: the elements are subobjects): the elements made are
	// destroyed, last first, when a later one's initialization throws.
	ElementCount count = ElementCount();
	const size_t counted = counts_elements(type, &list) ? push_counted(type, place.storage, count) : SIZE_MAX;
	count.rows = counted != SIZE_MAX && type->element->kind == sema::TypeKind::Array && creates_temporaries(&list);
	ElementCount* const saved_count = element_count_;
	const size_t saved_members = aggregate_members_;
	const bool saved_regions = member_regions_;
	element_count_ = counted != SIZE_MAX ? &count : nullptr;
	aggregate_members_ = SIZE_MAX;
	if (entity == nullptr || !sema::IsAggregateClass(entity) || !helper_eligible(entity, element, list, type->bound) ||
		mentions(&list, variable))
	{
		uint32_t next = 0;
		const syntax::Expr* self = &list;
		syntax::BracedInitList wrapper = syntax::BracedInitList();
		wrapper.elements.data = const_cast<syntax::Expr**>(&self);
		wrapper.elements.size = 1;
		structured_item(type, base, &wrapper, next, location);
	}
	else
	{
		element_count_ = nullptr;
		uint32_t next = 0;
		for (uint64_t i = 0; i < type->bound; ++i)
		{
			const size_t first = cleanups_.size();
			const lowir::Value object = structured_element(base, element, i, location, false);
			const syntax::Expr* item = next < list.elements.size ? list.elements[next] : nullptr;
			if (item != nullptr && item->kind == syntax::ExprKind::BracedInit)
			{
				++next;
				uint32_t inner = 0;
				aggregate_helper_call(
					entity, UINT32_MAX, object, static_cast<const syntax::BracedInitList*>(item), inner, location);
			}
			else
				aggregate_helper_call(entity, UINT32_MAX, object, item != nullptr ? &list : nullptr, next, location);
			if (counted != SIZE_MAX)
				element_made(count, first);
		}
	}
	element_count_ = saved_count;
	aggregate_members_ = saved_members;
	member_regions_ = saved_regions;
	// Made: its owner destroys it.
	if (counted != SIZE_MAX)
		end_counted(counted);
}

bool FunctionLowering::counts_elements(const sema::Type* type, const syntax::Expr* list)
{
	// Its elements are destroyed and their initialization from the list
	// (or an omitted element's value-initialization, 8.5.1p7) may really
	// throw (the hidden regions are behavior-neutral), outside a
	// namespace-scope static's initialization (a block-scope static's
	// elements made are destroyed, 15.2p2).
	const sema::Type* element = type;
	while (element->kind == sema::TypeKind::Array)
		element = element->element;
	const sema::ClassEntity* cls = sema::ClassOf(element);
	if (cls == nullptr || !needs_destruction(element) || static_initializer_)
		return false;
	const bool hidden = hidden_eh_;
	hidden_eh_ = false;
	const bool throws = (list != nullptr && may_throw(list)) || omitted_may_throw(cls, 0);
	hidden_eh_ = hidden;
	return throws;
}

bool FunctionLowering::omitted_may_throw(const sema::ClassEntity* cls, uint32_t depth)
{
	// A class with constructors by its default constructor; an aggregate
	// member by member (a deep nest is assumed to).
	if (!sema::IsAggregateClass(cls))
		return member_may_throw(cls, nullptr);
	if (depth > 64)
		return true;
	for (const sema::DataMember* member = cls->members; member != nullptr; member = member->next)
	{
		const sema::Type* type = member->variable->type;
		const sema::ClassEntity* inner = type->is_reference() ? nullptr : object_class(type);
		if (member->variable->name != nullptr && inner != nullptr && omitted_may_throw(inner, depth + 1))
			return true;
	}
	return false;
}

size_t FunctionLowering::push_counted(
	const sema::Type* type, lowir::Value storage, ElementCount& count, lowir::SlotId slot_id)
{
	// The array (at `storage`) joins the chain from the start, under its
	// initializer's temporaries (as the partial array of g++ and clang),
	// with none made: the course's counted form, linear in the elements.
	count = ElementCount{ lowir::kNoId, 0, 1, false, lowir::kNoId };
	for (const sema::Type* element = type; element->kind == sema::TypeKind::Array; element = element->element)
		count.total *= element->bound;
	const lowir::Type i64 = lowir::Type::scalar(lowir::TypeKind::I64);
	count.slot = slot_id != lowir::kNoId ? slot_id : generated_slot(lowir::SlotKind::ConstructorArrayIndex, i64);
	Place slot = Place();
	slot.storage = lowir::Value::slot(count.slot);
	store(i64, integer(i64, 0), slot);
	const size_t index = cleanups_.size();
	cleanups_.push_back(
		Cleanup{ storage, type, nullptr, lowir::kNoId, lowir::kNoId, lowir::kNoId, false, false, count.slot });
	cleanups_.back().counted = true;
	// From here on the alive objects' region destroys it too.
	split_region();
	return index;
}

void FunctionLowering::end_counted(size_t counted)
{
	// Made: the array leaves the chain, and the region holding it ends.
	if (region_open_)
		close_region();
	drop_member_cleanups(counted);
}

size_t FunctionLowering::begin_row(const sema::Type* type, lowir::Value storage, ElementCount& row)
{
	// A row of an array whose initializer makes temporaries: its elements
	// made are destroyed before the temporaries made before it (clang's
	// order: each subarray's partial cleanup begins with it), the rows
	// before it after them, by the parent's count.
	ElementCount* const parent = element_count_;
	const size_t index = push_counted(type, storage, row, parent->row_slot);
	parent->row_slot = row.slot;
	row.rows = type->element->kind == sema::TypeKind::Array;
	element_count_ = &row;
	return index;
}

void FunctionLowering::end_row(ElementCount* parent, const ElementCount& row, size_t counted)
{
	element_count_ = parent;
	end_counted(counted);
	parent->made += row.total;
	if (parent->made < parent->total)
	{
		const lowir::Type i64 = lowir::Type::scalar(lowir::TypeKind::I64);
		Place slot = Place();
		slot.storage = lowir::Value::slot(parent->slot);
		store(i64, integer(i64, parent->made), slot);
	}
}

void FunctionLowering::member_made(const sema::Type* type, lowir::Value storage, size_t first)
{
	// 15.2p2: a fully constructed subobject is destroyed by its destructor
	// (an aggregate's user-provided one included); the region holding its
	// members ends.
	if (region_open_)
		close_region();
	drop_member_cleanups(first);
	cleanups_.push_back(Cleanup{ storage, type, nullptr, lowir::kNoId, lowir::kNoId, lowir::kNoId, false });
	cleanups_.back().member = true;
	split_region();
}

bool FunctionLowering::mentions(const syntax::Expr* root, const sema::Entity* entity)
{
	// A work list over the evaluated operands (see may_throw).
	std::vector<const syntax::Expr*>& work = throw_scan_;
	const size_t base = work.size();
	work.push_back(root);
	bool found = false;
	while (!found && work.size() > base)
	{
		const syntax::Expr* e = work.back();
		work.pop_back();
		if (e == nullptr)
			continue;
		e = WithoutParentheses(e);
		switch (e->kind)
		{
		case syntax::ExprKind::Id:
			found = e->facts != nullptr && e->facts->entity == entity;
			break;
		case syntax::ExprKind::Binary:
		case syntax::ExprKind::Assign:
			work.push_back(static_cast<const syntax::BinaryExpr*>(e)->right);
			work.push_back(static_cast<const syntax::BinaryExpr*>(e)->left);
			break;
		case syntax::ExprKind::Conditional:
		{
			const syntax::ConditionalExpr* choice = static_cast<const syntax::ConditionalExpr*>(e);
			work.push_back(choice->else_expr);
			work.push_back(choice->then_expr);
			work.push_back(choice->condition);
			break;
		}
		case syntax::ExprKind::Unary:
		case syntax::ExprKind::Postfix:
			work.push_back(static_cast<const syntax::UnaryExpr*>(e)->operand);
			break;
		case syntax::ExprKind::Call:
		{
			const syntax::CallExpr* call = static_cast<const syntax::CallExpr*>(e);
			work.push_back(call->callee);
			for (const syntax::Expr* argument : call->arguments)
				work.push_back(argument);
			break;
		}
		case syntax::ExprKind::FunctionalCast:
		{
			const syntax::FunctionalCastExpr* cast = static_cast<const syntax::FunctionalCastExpr*>(e);
			for (const syntax::Expr* argument : cast->arguments)
				work.push_back(argument);
			work.push_back(cast->braced);
			break;
		}
		case syntax::ExprKind::Subscript:
			work.push_back(static_cast<const syntax::SubscriptExpr*>(e)->index);
			work.push_back(static_cast<const syntax::SubscriptExpr*>(e)->base);
			break;
		case syntax::ExprKind::Member:
			work.push_back(static_cast<const syntax::MemberExpr*>(e)->base);
			break;
		case syntax::ExprKind::Cast:
			work.push_back(static_cast<const syntax::CastExpr*>(e)->operand);
			break;
		case syntax::ExprKind::BracedInit:
			for (const syntax::Expr* element : static_cast<const syntax::BracedInitList*>(e)->elements)
				work.push_back(element);
			break;
		case syntax::ExprKind::New:
		{
			const syntax::NewExpr* allocation = static_cast<const syntax::NewExpr*>(e);
			for (const syntax::Expr* argument : allocation->placement)
				work.push_back(argument);
			const syntax::Initializer* init = allocation->initializer;
			if (init != nullptr && init->kind == syntax::InitializerKind::Parenthesized)
			{
				for (const syntax::Expr* argument : init->arguments)
					work.push_back(argument);
			}
			else if (init != nullptr)
				work.push_back(init->expression);
			break;
		}
		default:
			break;
		}
	}
	work.resize(base);
	return found;
}

void FunctionLowering::skip_item(const sema::Type* type, const syntax::BracedInitList* list, uint32_t& next)
{
	// Moves `next` past the list elements an object of `type` takes (one,
	// or its members' and elements' under brace elision, 8.5.1p11).
	if (list == nullptr || next >= list->elements.size)
		return;
	const syntax::Expr* element = list->elements[next];
	if (!sema::IsSubaggregate(type) || element->kind == syntax::ExprKind::BracedInit ||
		sema::InitializesSubaggregate(type, *element))
	{
		++next;
		return;
	}
	if (type->kind == sema::TypeKind::Array)
	{
		for (uint64_t i = 0; i < type->bound; ++i)
			skip_item(type->element, list, next);
		return;
	}
	for (const sema::DataMember* member = sema::ClassOf(type)->members; member != nullptr; member = member->next)
	{
		if (member->variable->name != nullptr || !member->bit_field)
			skip_item(member->variable->type, list, next);
	}
}

bool FunctionLowering::helper_members(
	const sema::ClassEntity* entity, const syntax::BracedInitList* list, uint32_t& next, bool* full)
{
	if (entity->key == TokenKind::KW_UNION)
		return false;
	// Whether the helper can initialize one element from the list at
	// `next`: each scalar or reference member has an initializer (or the
	// element is `{}`, all zero); members that are aggregates are passed as
	// temporaries, a member of a class with constructors or a bit-field is
	// not.
	const bool empty = list != nullptr && list->elements.size == 0;
	bool eligible = true;
	bool given = true;
	for (const sema::DataMember* member = entity->members; member != nullptr; member = member->next)
	{
		const sema::Entity* variable = member->variable;
		if (variable->name == nullptr && member->bit_field)
			continue;
		if (variable->name == nullptr)
		{
			// An anonymous union's or struct's member object: member by member.
			eligible = false;
			skip_item(variable->type, list, next);
			continue;
		}
		const sema::Type* type = variable->type;
		const bool present = list != nullptr && next < list->elements.size;
		if (member->bit_field)
			eligible = false;
		else if (type->is_reference() || (type->kind != sema::TypeKind::Array && sema::ClassOf(type) == nullptr))
		{
			eligible = eligible && (present || (empty && !type->is_reference()));
			given = given && present;
		}
		else if (!sema::IsSubaggregate(type))
			eligible = false;
		else if (present && list->elements[next]->kind != syntax::ExprKind::BracedInit &&
			sema::InitializesSubaggregate(type, *list->elements[next]))
			eligible = false;
		skip_item(type, list, next);
	}
	if (full != nullptr)
		*full = given;
	return eligible;
}

bool FunctionLowering::helper_eligible(
	const sema::ClassEntity* entity, const sema::Type* element, const syntax::BracedInitList& list, uint64_t bound)
{
	uint32_t next = 0;
	bool named = false;
	for (const sema::DataMember* member = entity->members; member != nullptr; member = member->next)
	{
		// Aggregate members pass through temporaries of the caller; class
		// members are initialized in place (see helper_shape).
		if (member->variable->name != nullptr && !lowering_.runs_no_constructor(member->variable->type))
			return false;
		const sema::Type* innermost = member->variable->type;
		while (innermost->kind == sema::TypeKind::Array)
			innermost = innermost->element;
		if (member->variable->name != nullptr && !innermost->is_reference() && sema::ClassOf(innermost) != nullptr)
			return false;
		named = named || member->variable->name != nullptr;
	}
	if (!named)
		return false;
	// Some element gives every scalar member a value; the others do too or
	// are `{}`.
	bool any_full = false;
	for (uint64_t i = 0; i < bound; ++i)
	{
		const syntax::Expr* item = next < list.elements.size ? list.elements[next] : nullptr;
		bool full = false;
		if (item != nullptr && item->kind == syntax::ExprKind::BracedInit)
		{
			++next;
			uint32_t inner = 0;
			if (!helper_members(entity, static_cast<const syntax::BracedInitList*>(item), inner, &full))
				return false;
		}
		else if (item != nullptr && sema::InitializesSubaggregate(element, *item))
			return false;
		else if (!helper_members(entity, item != nullptr ? &list : nullptr, next, &full))
			return false;
		any_full = any_full || full;
	}
	return any_full;
}

bool FunctionLowering::helper_shape(
	const sema::ClassEntity* entity, const syntax::BracedInitList* list, uint32_t& count)
{
	if (entity->key == TokenKind::KW_UNION)
		return false;
	// The helper one object's list can call: its leading members through
	// the last one initialized, no member before them omitted; the trailing
	// members the helper zeroes itself. It takes scalar and reference
	// members only: a class member or an array is initialized at its final
	// destination (the course's form): passing it through a helper's
	// parameter would add a transfer, and relocating an array would change
	// its elements' addresses. So the helper owns no object and cannot
	// throw.
	uint32_t next = 0;
	uint32_t index = 0;
	bool omitted = false;
	count = 0;
	for (const sema::DataMember* member = entity->members; member != nullptr; member = member->next)
	{
		const sema::Entity* variable = member->variable;
		if (variable->name == nullptr && member->bit_field)
			continue;
		if (variable->name == nullptr)
			return false;
		++index;
		const sema::Type* type = variable->type;
		if (member->bit_field || type->kind == sema::TypeKind::Array ||
			(!type->is_reference() && sema::ClassOf(type) != nullptr))
			return false;
		if (list != nullptr && next < list->elements.size)
		{
			if (omitted)
				return false;
			count = index;
		}
		else
			omitted = true;
		skip_item(type, list, next);
	}
	return count != 0;
}

void FunctionLowering::aggregate_helper_call(const sema::ClassEntity* entity, uint32_t count, lowir::Value object,
	const syntax::BracedInitList* list, uint32_t& next, uint32_t location, unsigned cv)
{
	// The helper's arguments: the element's address, each scalar member's
	// value, each array member (of scalars: an array of class aggregates'
	// element, see helper_eligible) initialized in a temporary passed by its
	// address; `count` leading members (helper_shape: no class member).
	const size_t base = arguments_.size();
	arguments_.push_back(object);
	uint32_t taken = 0;
	for (const sema::DataMember* member = entity->members; member != nullptr && taken < count; member = member->next)
	{
		const sema::Entity* variable = member->variable;
		if (variable->name == nullptr)
			continue;
		++taken;
		const sema::Type* type = variable->type;
		if (type->kind == sema::TypeKind::Array)
		{
			Place temporary = Place();
			temporary.storage = lowir::Value::slot(
				generated_slot(lowir::SlotKind::Argarr, lowering_.storage_type(type)));
			const lowir::Value address = address_of(temporary);
			structured_item(type, address, list, next, location);
			arguments_.push_back(address);
			continue;
		}
		const syntax::Expr* element = list != nullptr && next < list->elements.size ? list->elements[next++] : nullptr;
		// A reference's list binds its one element, or is the temporary the
		// list initializes (8.5.4p3, see reference_initializer); a vector's
		// list gives its lanes.
		const syntax::BracedInitList* lanes = VectorLanes(type, element);
		if (element != nullptr && type->is_reference())
			element = reference_initializer(element);
		else if (element != nullptr && element->kind == syntax::ExprKind::BracedInit)
		{
			const syntax::BracedInitList* inner = static_cast<const syntax::BracedInitList*>(element);
			element = inner->elements.empty() ? nullptr : inner->elements[0];
		}
		if (type->is_reference())
			arguments_.push_back(aggregate_extension_ != nullptr
					? bind_extended(*element, type, aggregate_extension_)
					: bind_reference(*element, type, ConversionContext::Initialization));
		else if (lanes != nullptr)
			arguments_.push_back(vector_list(type, *lanes).value);
		else
			arguments_.push_back(element != nullptr ? fit(operand(*element, ConversionContext::Argument), type).value
													: zero_value(type).value);
	}
	// (A helper cannot throw: its call opens no region.)
	const lowir::Value callee = lowir::Value::symbol(lowering_.aggregate_helper(entity, taken, cv));
	++lowering_.counters().instructions;
	if (terminated_)
		place(new_block(lowir::BlockKind::Unreachable));
	builder_.call(lowir::Type::scalar(lowir::TypeKind::Void), callee, arguments_.data() + base,
		static_cast<uint32_t>(arguments_.size() - base), lowir::kNoId, 0);
	arguments_.resize(base);
}

lowir::Value FunctionLowering::structured_element(
	lowir::Value array, const sema::Type* element, uint64_t index, uint32_t location, bool field)
{
	// A member array's scalar element is indexed in its type; otherwise the
	// first element is the array's address and another is at its byte
	// offset (a class element's without a projection).
	const bool object = element->kind == sema::TypeKind::Array || sema::ClassOf(element) != nullptr;
	if (field && !object)
		return this
			->index(lowering_.storage_type(element), ir_model::IPK_ARRAY_ELEMENT, array,
				integer(lowir::Type::scalar(lowir::TypeKind::I64), index))
			.value;
	if (index == 0)
		return array;
	const uint64_t size = lowering_.size_of(element, location);
	return this
		->index(lowir::Type::scalar(lowir::TypeKind::I8), object ? ir_model::IPK_NONE : ir_model::IPK_ARRAY_ELEMENT,
			array, integer(lowir::Type::scalar(lowir::TypeKind::I64), index * size))
		.value;
}

void FunctionLowering::structured_class(const sema::ClassEntity* entity, uint8_t cv, lowir::Value object,
	const syntax::BracedInitList* list, uint32_t& next, uint32_t location)
{
	// Each member's address from the object's, then its initialization.
	bit_unit_ = 0;
	bit_high_ = 0;
	// Its own units (an aggregate's members lie within its data).
	const uint64_t saved_limit = bit_limit_;
	bit_limit_ = UINT64_MAX;
	// 8.5.1p15: a union's one member (see sema::UnionListMember).
	const sema::DataMember* selected = entity->key == TokenKind::KW_UNION ? sema::ListUnionMember(entity, list, next)
																		  : nullptr;
	for (const sema::DataMember* member = entity->members; member != nullptr; member = member->next)
	{
		const sema::VariableEntity* variable = static_cast<const sema::VariableEntity*>(member->variable);
		if ((variable->name == nullptr && member->bit_field) || (selected != nullptr && member != selected))
			continue;
		if (member->bit_field)
		{
			const syntax::Expr* element = list != nullptr && next < list->elements.size ? list->elements[next++]
																						: nullptr;
			if (element != nullptr && element->kind == syntax::ExprKind::BracedInit)
			{
				const syntax::BracedInitList* inner = static_cast<const syntax::BracedInitList*>(element);
				element = inner->elements.empty() ? nullptr : inner->elements[0];
			}
			initialize_bit_field(member, variable, AddressPath{ AddressPath::Root::Value, object, path_steps_.size() },
				element, false, location, cv);
			bit_high_ = member_end_bits(member, location);
			if (entity->key == TokenKind::KW_UNION)
				break;
			continue;
		}
		bit_unit_ = 0;
		const sema::Type* type = lowering_.graph().types.qualified(
			variable->type, static_cast<uint8_t>(cv & sema::kVolatile));
		// A tracked class member's region ends before its address (see
		// aggregate_class_item).
		const sema::ClassEntity* cls = type->is_reference() ? nullptr : sema::ClassOf(type);
		const syntax::Expr* element = list != nullptr && next < list->elements.size ? list->elements[next] : nullptr;
		if (cls != nullptr && !sema::IsAggregateClass(cls) && aggregate_members_ != SIZE_MAX &&
			member_may_throw(cls, element) &&
			!(region_open_ && cleanups_.size() == region_depth_ && !stale_region_ &&
				(open_serial_ > aggregate_serial_ || reopened_construction_)))
		{
			close_region();
			eager_pending_ = false;
		}
		const lowir::Value address = index(lowir::Type::scalar(lowir::TypeKind::I8), ir_model::IPK_FIELD, object,
			integer(lowir::Type::scalar(lowir::TypeKind::I64), lowering_.member_offset(variable, location)))
										 .value;
		if (through_union(variable, entity))
			mark_untyped(address);
		const OverlapScope overlap(overlapping_empty_, member);
		structured_item(type, address, list, next, location, true);
		bit_unit_ = 0;
		bit_high_ = member_end_bits(member, location);
		if (entity->key == TokenKind::KW_UNION)
			break;
	}
	bit_unit_ = 0;
	bit_limit_ = saved_limit;
}

void FunctionLowering::structured_item(const sema::Type* type, lowir::Value address, const syntax::BracedInitList* list,
	uint32_t& next, uint32_t location, bool field, bool array_element)
{
	// One object at a computed address from the list at `next` (8.5.1p11 as
	// in aggregate_item).
	if (element_count_ != nullptr && type->kind != sema::TypeKind::Array)
	{
		// An element of the array being counted: made (an aggregate's
		// members tracked), then counted.
		ElementCount* const count = element_count_;
		element_count_ = nullptr;
		const size_t first = cleanups_.size();
		const sema::ClassEntity* cls = type->is_reference() ? nullptr : sema::ClassOf(type);
		aggregate_members_ = cls != nullptr && sema::IsAggregateClass(cls) ? first : SIZE_MAX;
		member_regions_ = false;
		counted_item_ = true;
		structured_item(type, address, list, next, location, field, array_element);
		counted_item_ = false;
		element_made(*count, first);
		aggregate_members_ = SIZE_MAX;
		member_regions_ = false;
		element_count_ = count;
		return;
	}
	const syntax::Expr* element = list != nullptr && next < list->elements.size ? list->elements[next] : nullptr;
	if (type->kind == sema::TypeKind::Array)
	{
		// A row of a counted array (see begin_row).
		ElementCount* const parent = element_count_;
		ElementCount row = ElementCount();
		const size_t counted = parent != nullptr && parent->rows ? begin_row(type, address, row) : SIZE_MAX;
		if (element != nullptr &&
			(element->kind == syntax::ExprKind::BracedInit || sema::InitializesSubaggregate(type, *element)))
		{
			++next;
			syntax::BracedInitList single = syntax::BracedInitList();
			const syntax::BracedInitList* own = static_cast<const syntax::BracedInitList*>(element);
			if (element->kind != syntax::ExprKind::BracedInit)
			{
				single.elements.data = const_cast<syntax::Expr**>(&element);
				single.elements.size = 1;
				own = &single;
			}
			uint32_t inner = 0;
			structured_array(type, address, own, inner, location, field);
		}
		else
			structured_array(type, address, element != nullptr ? list : nullptr, next, location, field);
		if (counted != SIZE_MAX)
			end_row(parent, row, counted);
		return;
	}
	const sema::ClassEntity* cls = type->is_reference() ? nullptr : sema::ClassOf(type);
	if (cls != nullptr && sema::IsAggregateClass(cls))
	{
		// A tracked aggregate's aggregate member, once made, is one object
		// (see member_made); an element of a counted array is counted.
		const bool unit = aggregate_members_ != SIZE_MAX && !counted_item_ && needs_destruction(type);
		counted_item_ = false;
		const size_t first = cleanups_.size();
		if (element != nullptr && element->kind == syntax::ExprKind::BracedInit)
		{
			++next;
			uint32_t inner = 0;
			structured_class(
				cls, type->cv, address, static_cast<const syntax::BracedInitList*>(element), inner, location);
		}
		else if (element != nullptr && sema::InitializesSubaggregate(type, *element))
		{
			++next;
			materialize_member(
				*element, AddressPath{ AddressPath::Root::Value, address, path_steps_.size() }, location);
		}
		else
			structured_class(cls, type->cv, address, element != nullptr ? list : nullptr, next, location);
		if (unit)
			member_made(type, address, first);
		return;
	}
	if (cls != nullptr)
	{
		aggregate_class_item(cls, type, AddressPath{ AddressPath::Root::Value, address, path_steps_.size() }, list,
			next, location, array_element);
		return;
	}
	if (element != nullptr)
		++next;
	// A reference's list binds its one element, or is the temporary the
	// list initializes (8.5.4p3, see reference_initializer); a vector's
	// list gives its lanes.
	const syntax::BracedInitList* lanes = VectorLanes(type, element);
	if (element != nullptr && type->is_reference())
		element = reference_initializer(element);
	else if (element != nullptr && element->kind == syntax::ExprKind::BracedInit)
	{
		const syntax::BracedInitList* inner = static_cast<const syntax::BracedInitList*>(element);
		element = inner->elements.empty() ? nullptr : inner->elements[0];
	}
	const lowir::Type storage = type->is_reference() ? lowir::Type::scalar(lowir::TypeKind::Ptr)
													 : lowering_.storage_type(type);
	Place place = Place();
	place.storage = address;
	qualify(place, type);
	RValue value;
	if (type->is_reference())
	{
		if (element == nullptr)
			throw LoweringError(location, "a reference member without an initializer");
		value.value = aggregate_extension_ != nullptr
			? bind_extended(*element, type, aggregate_extension_)
			: bind_reference(*element, type, ConversionContext::Initialization, static_object_);
		value.type = storage;
	}
	else if (lanes != nullptr)
		value = vector_list(type, *lanes);
	else if (element == nullptr)
		value = zero_value(type);
	else if (array_element)
		value = fit(operand(*element, ConversionContext::ArrayElement), type);
	else
		value = fit(member_operand(*element), type);
	store(storage, value, place);
}

void FunctionLowering::structured_array(const sema::Type* type, lowir::Value address,
	const syntax::BracedInitList* list, uint32_t& next, uint32_t location, bool field)
{
	// Each element from the list at `next` (its own list, or the enclosing
	// one's elements); a character array from a string literal takes its
	// code units (8.5.2).
	const sema::Type* element = type->element;
	const syntax::Expr* first = list != nullptr && next < list->elements.size ? WithoutParentheses(list->elements[next])
																			  : nullptr;
	if (first != nullptr && first->kind == syntax::ExprKind::Literal &&
		first->facts->type->kind == sema::TypeKind::Array &&
		(element->kind == sema::TypeKind::Fundamental || element->unqualified->kind == sema::TypeKind::Fundamental))
	{
		++next;
		std::vector<uint64_t> units;
		sema::StringLiteralFacts facts;
		if (!sema::StringLiteralUnits(static_cast<const syntax::LiteralExpr*>(first)->spelling, facts, units))
			throw LoweringError(location, "an invalid string literal");
		const lowir::Type storage = lowering_.storage_type(element);
		for (uint64_t i = 0; i < type->bound; ++i)
		{
			Place place = Place();
			place.storage = structured_element(address, element, i, location, field);
			qualify(place, element);
			store(storage, integer(storage, i < units.size() ? units[static_cast<size_t>(i)] : 0), place);
		}
		return;
	}
	for (uint64_t i = 0; i < type->bound; ++i)
		structured_item(
			element, structured_element(address, element, i, location, field), list, next, location, false, true);
}

void FunctionLowering::copy_object(
	const sema::Type* type, lowir::Value source, lowir::Value destination, uint32_t location)
{
	// An empty class has no value; a [[no_unique_address]] one's byte is
	// another member's.
	const sema::ClassEntity* empty = overlapping_empty_ && type->kind != sema::TypeKind::Array ? sema::ClassOf(type)
																							   : nullptr;
	if (empty != nullptr && empty->empty)
		return;
	const uint64_t alignment = lowering_.alignment_of(type, location);
	lowir::Instruction inst = lowir::make_instruction(
		lowir::Opcode::CopyObject, lowir::Type::scalar(lowir::TypeKind::Void));
	inst.ops[0] = source;
	inst.ops[1] = destination;
	// A class object owns its nvsize bytes: past them its tail padding may
	// hold another object (a derived class's member or virtual base,
	// Itanium C++ ABI 2.4), so a class is transferred from storage as those
	// bytes (an object value whole).
	uint64_t size = lowering_.size_of(type, location);
	const sema::ClassEntity* cls = type->kind == sema::TypeKind::Array ? nullptr : sema::ClassOf(type);
	const bool object_value = source.kind == lowir::ValueKind::Temp &&
		builder_.body().temps[source.index].type.kind() == lowir::TypeKind::Obj;
	if (cls != nullptr && cls->nvsize != 0 && !object_value)
		size = std::min(size, cls->nvsize);
	inst.immediate = static_cast<int64_t>(size);
	unsigned log2 = 0;
	while ((uint64_t(1) << log2) < alignment)
		++log2;
	inst.sub = static_cast<uint8_t>(log2);
	emit(inst);
}

void FunctionLowering::lower_aggregate_helper(const sema::ClassEntity* entity, uint32_t count)
{
	// `this` and each leading member's incoming value stored in slots (an
	// array member's address), then each member initialized from its slot
	// in declaration order (an array's elements copied by their bytes), the
	// others zeroed. It takes no class member (helper_shape,
	// helper_eligible): nothing it does calls a function.
	const lowir::Type pointer = lowir::Type::scalar(lowir::TypeKind::Ptr);
	this_slot_ = new_slot(lowering_.this_name(), pointer);
	uint32_t index = 0;
	{
		RValue incoming;
		incoming.value = lowir::Value::temp(builder_.parameter(index++));
		incoming.type = pointer;
		Place place = Place();
		place.storage = lowir::Value::slot(this_slot_);
		store(pointer, incoming, place);
	}
	std::vector<lowir::SlotId> slots;
	for (const sema::DataMember* member = entity->members; member != nullptr && slots.size() < count;
		member = member->next)
	{
		const sema::Entity* variable = member->variable;
		if (variable->name == nullptr)
			continue;
		const sema::Type* type = variable->type;
		const lowir::Type slot_type = type->is_reference() || type->kind == sema::TypeKind::Array
			? pointer
			: lowering_.storage_type(type);
		const lowir::SlotId slot = new_slot(
			lowering_.intern(variable->name->spelling, static_cast<uint32_t>(std::strlen(variable->name->spelling))),
			slot_type);
		slots.push_back(slot);
		Place place = Place();
		place.storage = lowir::Value::slot(slot);
		store(slot_type, RValue{ lowir::Value::temp(builder_.parameter(index++)), slot_type }, place);
	}
	size_t next = 0;
	for (const sema::DataMember* member = entity->members; member != nullptr; member = member->next)
	{
		const sema::VariableEntity* variable = static_cast<const sema::VariableEntity*>(member->variable);
		if (variable->name == nullptr)
			continue;
		const sema::Type* type = variable->type;
		const bool taken = next < slots.size();
		Place place = Place();
		if (taken)
			place.storage = lowir::Value::slot(slots[next++]);
		const lowir::Value offset =
			integer(lowir::Type::scalar(lowir::TypeKind::I64), lowering_.member_offset(variable, 0)).value;
		if (type->kind != sema::TypeKind::Array)
		{
			const lowir::Type storage = type->is_reference() ? pointer : lowering_.storage_type(type);
			const RValue value = taken ? load(storage, place) : zero_value(type);
			Place target = Place();
			target.storage = this->index(lowir::Type::scalar(lowir::TypeKind::I8), ir_model::IPK_FIELD, this_pointer(0),
									 RValue{ offset, lowir::Type::scalar(lowir::TypeKind::I64) })
								 .value;
			if (through_union(variable, entity))
				mark_untyped(target.storage);
			// A volatile member's initializing store is a volatile access (the
			// course's form; what is one is implementation-defined, 7.1.6.1p6).
			target.is_volatile = !type->is_reference() && (type->cv & sema::kVolatile) != 0;
			store(storage, value, target);
			continue;
		}
		const lowir::Value target = this->index(lowir::Type::scalar(lowir::TypeKind::I8), ir_model::IPK_FIELD,
											this_pointer(0),
											RValue{ offset, lowir::Type::scalar(lowir::TypeKind::I64) })
										.value;
		if (through_union(variable, entity))
			mark_untyped(target);
		copy_object(type, load(pointer, place).value, target, 0);
	}
	finish_body();
	if (!lowering_.streaming())
		builder_.finish();
}

// --------------------------------------------------------------- bit-fields

FunctionLowering::BitField FunctionLowering::bit_field(const sema::VariableEntity* variable, uint32_t location)
{
	// 9.6: the field's storage unit as the layout assigned it (see
	// sema::Analyzer::assign_bit_units), its bit position there and width.
	// The unit is read and written in the declared type when it has the
	// type's width (an unsigned type narrower than int, or an unsigned int
	// field narrower than it, as int: the access type); in the access type
	// when it has that width; else as the integer of its width, signed like
	// the declared type. A split unit is a 64-bit (past 8 bytes, 128-bit)
	// integer assembled from its pieces.
	const sema::DataMember* member = variable->data_member;
	BitField field;
	field.variable = variable;
	const sema::Type* declared = variable->type->unqualified;
	field.declared = lowering_.storage_type(declared);
	const unsigned declared_bits = lowir::integer_bits(field.declared.kind());
	const uint64_t bit = lowering_.member_bit_offset(variable, location);
	field.unit_offset = member->unit_offset;
	field.shift = static_cast<unsigned>(bit - member->unit_offset * 8);
	// Bits beyond the type's width are padding (9.6p1).
	field.width = static_cast<unsigned>(std::min<uint64_t>(member->width, declared_bits));
	field.is_signed = IsSignedType(declared);
	field.shared = member->unit_shared;
	field.split = member->unit_split;
	const lowir::Type access = declared_bits < 32 || (!field.is_signed && declared_bits == 32 && field.width < 32)
		? lowir::Type::scalar(lowir::TypeKind::I32)
		: field.declared;
	const unsigned unit_bits = member->unit_bytes * 8u;
	const unsigned access_bits = lowir::integer_bits(access.kind());
	if (field.split != 0)
		field.unit = lowir::Type::scalar(field.split > 8 ? lowir::TypeKind::I128 : lowir::TypeKind::I64);
	else if (unit_bits == 0 || unit_bits == declared_bits)
		field.unit = declared_bits == access_bits ? access : field.declared;
	else if (unit_bits == access_bits)
		field.unit = access;
	else
	{
		static const lowir::TypeKind kSigned[] = { lowir::TypeKind::I8, lowir::TypeKind::I16, lowir::TypeKind::I32,
			lowir::TypeKind::I64, lowir::TypeKind::I128 };
		static const lowir::TypeKind kUnsigned[] = { lowir::TypeKind::U8, lowir::TypeKind::U16, lowir::TypeKind::U32,
			lowir::TypeKind::I64, lowir::TypeKind::I128 };
		unsigned k = 0;
		while (k < 4 && (8u << k) < unit_bits)
			++k;
		if ((8u << k) != unit_bits)
			throw LoweringError(location, "a bit-field storage unit of no integer width");
		field.unit = lowir::Type::scalar(field.is_signed ? kSigned[k] : kUnsigned[k]);
	}
	// (The access type int is signed; a 64- or 128-bit unit is signed like
	// the declared type.)
	const lowir::TypeKind kind = field.unit.kind();
	field.unit_unsigned = kind == lowir::TypeKind::U8 || kind == lowir::TypeKind::U16 || kind == lowir::TypeKind::U32 ||
		((kind == lowir::TypeKind::I64 || kind == lowir::TypeKind::I128) && !field.is_signed);
	return field;
}

namespace {

// The naturally sized pieces of a split unit's bytes, lowest first.
unsigned SplitPieces(unsigned bytes, unsigned* offsets, unsigned* widths)
{
	unsigned count = 0;
	for (unsigned at = 0; at < bytes; ++count)
	{
		unsigned width = 8;
		while (width > bytes - at)
			width /= 2;
		offsets[count] = at;
		widths[count] = width;
		at += width;
	}
	return count;
}

lowir::Type PieceType(unsigned width)
{
	return lowir::Type::scalar(width == 1 ? lowir::TypeKind::U8
			: width == 2                  ? lowir::TypeKind::U16
			: width == 4                  ? lowir::TypeKind::U32
										  : lowir::TypeKind::I64);
}

} // namespace

RValue FunctionLowering::load_unit(const BitField& field, lowir::Type type, const Place& unit)
{
	// The unit (a split one assembled from its pieces, each zero-extended
	// and shifted to its place).
	const Place storage = untyped_unit(unit);
	if (field.split == 0)
		return load(type, storage);
	unsigned offsets[4], widths[4];
	const unsigned count = SplitPieces(field.split, offsets, widths);
	RValue assembled;
	for (unsigned i = 0; i < count; ++i)
	{
		Place piece_place = storage;
		if (offsets[i] != 0)
			piece_place.storage = index(lowir::Type::scalar(lowir::TypeKind::I8), ir_model::IPK_NONE, storage.storage,
				integer(lowir::Type::scalar(lowir::TypeKind::I64), offsets[i]))
									  .value;
		const lowir::Type piece_type = PieceType(widths[i]);
		RValue piece = load(piece_type, piece_place);
		if (piece_type.kind() != type.kind())
			piece = convert_op(lowir::ConvertOp::ZExt, type, piece);
		if (offsets[i] != 0)
			piece = binary(lowir::BinaryOp::Shl, type, piece, integer(type, 8u * offsets[i]));
		assembled = i == 0 ? piece : binary(lowir::BinaryOp::Or, type, assembled, piece);
	}
	return assembled;
}

void FunctionLowering::store_unit(const BitField& field, lowir::Type type, const RValue& value, const Place& unit)
{
	// A split unit is stored piecewise, each piece its own bytes.
	const Place storage = untyped_unit(unit);
	if (field.split == 0)
	{
		store(type, value, storage);
		return;
	}
	unsigned offsets[4], widths[4];
	const unsigned count = SplitPieces(field.split, offsets, widths);
	for (unsigned i = 0; i < count; ++i)
	{
		RValue piece = value;
		if (offsets[i] != 0)
			piece = binary(lowir::BinaryOp::UShr, type, value, integer(type, 8u * offsets[i]));
		const lowir::Type piece_type = PieceType(widths[i]);
		if (piece_type.kind() != type.kind())
			piece = convert_op(lowir::ConvertOp::Trunc, piece_type, piece);
		Place piece_place = storage;
		if (offsets[i] != 0)
			piece_place.storage = index(lowir::Type::scalar(lowir::TypeKind::I8), ir_model::IPK_NONE, storage.storage,
				integer(lowir::Type::scalar(lowir::TypeKind::I64), offsets[i]))
									  .value;
		store(piece_type, piece, piece_place);
	}
}

RValue FunctionLowering::read_bit_field(const BitField& field, const Place& unit, bool typed)
{
	// The unit loaded (not `typed`: a storage unit, lowir.md "Instruction
	// Facts"), the field shifted down and masked; a signed field's sign
	// extended by a shift up and an arithmetic shift down.
	RValue value = load_unit(field, field.unit, unit);
	if (field.shift != 0)
		value = binary(field.unit_unsigned ? lowir::BinaryOp::UShr : lowir::BinaryOp::Shr, field.unit, value,
			integer(field.unit, field.shift));
	value = binary(
		lowir::BinaryOp::And, field.unit, value, integer(field.unit, FieldMask(field.unit, field.width, 0, false)));
	if (field.is_signed)
		value = sign_extend(field, value);
	return declared_width(field, value, typed);
}

Place FunctionLowering::untyped_unit(const Place& unit)
{
	Place storage = unit;
	storage.untyped = true;
	return storage;
}

RValue FunctionLowering::declared_width(const BitField& field, const RValue& value, bool typed)
{
	// A value read or written, `typed`: in the field's declared type (a
	// typed copy when the unit's integer type differs only in signedness);
	// else in the unit's (a field-wise copy's transfer). A unit may be
	// narrower or wider than the field's type (narrowed to its run, shared
	// with fields of other types, packed): the value is then extended (by
	// its signedness) or truncated to the declared type.
	const unsigned declared = lowir::integer_bits(field.declared.kind());
	const unsigned bits = lowir::integer_bits(value.type.kind());
	if (bits == declared)
		return !typed || value.type == field.declared ? value : copy(field.declared, value);
	if (bits > declared)
		return convert_op(lowir::ConvertOp::Trunc, field.declared, value);
	return convert_op(field.is_signed ? lowir::ConvertOp::SExt : lowir::ConvertOp::ZExt, field.declared, value);
}

RValue FunctionLowering::sign_extend(const BitField& field, const RValue& value)
{
	const unsigned bits = lowir::integer_bits(field.unit.kind());
	const RValue up = binary(lowir::BinaryOp::Shl, field.unit, value, integer(field.unit, bits - field.width));
	return binary(lowir::BinaryOp::Shr, field.unit, up, integer(field.unit, bits - field.width));
}

RValue FunctionLowering::write_bit_field(const BitField& field, const Place& unit, const RValue& value, bool typed)
{
	// The value masked (a signed field's result sign-extended), shifted
	// into place and merged into the unit's other bits; returns the value
	// the field now holds.
	const RValue masked = binary(
		lowir::BinaryOp::And, field.unit, value, integer(field.unit, FieldMask(field.unit, field.width, 0, false)));
	RValue result = masked;
	if (field.is_signed)
		result = sign_extend(field, masked);
	result = declared_width(field, result, typed);
	merge_bit_field(field, field.unit, unit, masked);
	return result;
}

const sema::Type* FunctionLowering::field_operation_type(const BitField& field)
{
	// 4.5p3: the type an integral bit-field's value promotes to: int when
	// int represents all its values, else unsigned int when that does, else
	// its own (an enumeration's or bool's: its type's promotion).
	const sema::Type* type = field.variable->type->unqualified;
	if (type->kind != sema::TypeKind::Fundamental || type->fundamental == FundamentalType::Bool)
		return type;
	const unsigned declared = lowir::integer_bits(field.declared.kind());
	const unsigned width = field.width < declared ? field.width : declared;
	if (width < 32 || (width == 32 && field.is_signed))
		return lowering_.graph().types.fundamental(FundamentalType::Int);
	return width == 32 ? lowering_.graph().types.fundamental(FundamentalType::UnsignedInt) : type;
}

void FunctionLowering::merge_bit_field(const BitField& field, lowir::Type type, const Place& unit, const RValue& masked)
{
	RValue positioned = masked;
	if (field.shift != 0)
		positioned = binary(lowir::BinaryOp::Shl, type, masked, integer(type, field.shift));
	const RValue old = load_unit(field, type, unit);
	const RValue cleared = binary(
		lowir::BinaryOp::And, type, old, integer(type, FieldMask(type, field.width, field.shift, true)));
	store_unit(field, type, binary(lowir::BinaryOp::Or, type, cleared, positioned), unit);
}

RValue FunctionLowering::to_unit(const BitField& field, const RValue& value, const sema::Type* from)
{
	// The value (of the field's type) in the unit's LowIR type; a constant
	// fits it directly, truncated to its width and extended by its
	// signedness (the course's immediates).
	if (value.type == field.unit)
		return value;
	uint64_t bits = 0;
	const unsigned to_bits = lowir::integer_bits(field.unit.kind());
	if (is_integer_literal(value, bits) && lowir::integer_bits(value.type.kind()) <= 64)
	{
		const bool to_signed = !field.unit_unsigned;
		uint64_t folded = to_bits >= 64 ? bits : bits & ((uint64_t(1) << to_bits) - 1);
		if (to_signed && to_bits < 64 && ((folded >> (to_bits - 1)) & 1) != 0)
			folded |= ~((uint64_t(1) << to_bits) - 1);
		if (to_bits == 128)
		{
			lowir::Literal wide = lowir::integer_literal(static_cast<int64_t>(folded));
			if (!to_signed)
				wide.high = 0;
			return integer(field.unit, wide);
		}
		return integer(field.unit, folded);
	}
	return convert_integer(value, from, field.unit, ConversionContext::Assignment,
		field.unit.kind() == lowir::TypeKind::I32 || field.unit.kind() == lowir::TypeKind::I64 ||
			field.unit.kind() == lowir::TypeKind::I8 || field.unit.kind() == lowir::TypeKind::I16,
		true);
}

Place FunctionLowering::bit_field_unit(
	const syntax::MemberExpr* access, const syntax::Expr& expr, const BitField& field)
{
	// The field's unit in its object: the member access's object, a
	// structured binding's object e, or `*this` for an implicit member.
	const sema::VariableEntity* variable = field.variable;
	lowir::Value object;
	const sema::ClassEntity* from = nullptr;
	if (const sema::BindingEntity* alias = sema::BindingAlias(WithoutParentheses(&expr)->facts->entity))
	{
		const sema::Decomposition* decomposition = alias->decomposition;
		object = address_of(variable_place(decomposition->object, decomposition->type, expr.location));
		from = sema::ClassOf(decomposition->type);
	}
	else if (access == nullptr)
	{
		object = object_pointer(expr.location);
		from = object_class();
	}
	else
	{
		const sema::Type* object_type = WithoutParentheses(access->base)->facts->type;
		if (access->op == TokenKind::OP_ARROW)
		{
			object = operand(*access->base, ConversionContext::Operand).value;
			object_type = object_type->kind == sema::TypeKind::Array ? object_type->element
																	 : object_type->unqualified->element;
		}
		else
			object = object_address(*access->base);
		from = sema::ClassOf(object_type);
	}
	// Through a qualified name's naming class (11.2p6), which selects a
	// repeated base's subobject.
	if (const sema::ClassEntity* naming = lowering_.member_naming_class(*WithoutParentheses(&expr)))
	{
		object = to_base(object, from, naming, expr.location);
		from = naming;
	}
	uint64_t anonymous = 0;
	const sema::ClassEntity* holder = lowering_.member_holder(variable, from, anonymous, expr.location);
	object = anonymous_object(object, from, holder, expr.location);
	object = to_base(object, from, holder, expr.location);
	Place place = Place();
	place.storage = index(lowir::Type::scalar(lowir::TypeKind::I8), ir_model::IPK_FIELD, object,
		integer(lowir::Type::scalar(lowir::TypeKind::I64), anonymous + field.unit_offset))
						.value;
	qualify(place, expr.facts->type);
	return place;
}

void FunctionLowering::initialize_bit_field(const sema::DataMember* member, const sema::VariableEntity* variable,
	const AddressPath& path, const syntax::Expr* element, bool value_first, uint32_t location, uint8_t cv)
{
	// A bit-field's initialization (by an aggregate's element or the
	// constructor's member initialization): its value (of the field's
	// type), then its unit's address; the first field an initialization
	// writes in a unit stores the whole unit unless a subobject written
	// before lies in it (9.6p1), the next ones merge into it.
	(void)member;
	const BitField field = bit_field(variable, location);
	const sema::Type* type = variable->type;
	RValue value;
	if (value_first)
		value = element != nullptr ? fit(member_operand(*element), type) : zero_value(type);
	// The course's other form (a constructor of a class whose implicit
	// assignment operator is used): a whole unit's value before its
	// address; a merge reads and clears the unit first and addresses it
	// again for the store.
	const bool alternate = bit_order_alternate_ && value_first;
	Place unit = Place();
	// (A shared unit holds another unit's or subobject's bytes too.)
	const bool whole = !field.shared && bit_unit_ != field.unit_offset + 1 && field.unit_offset * 8 >= bit_high_ &&
		(field.unit_offset * 8 + lowir::integer_bits(field.unit.kind())) <= bit_limit_;
	if (!alternate || !whole)
	{
		path_steps_.push_back(PathStep{ PathStep::Kind::Field, field.unit_offset, lowir::Type(), 0 });
		unit.storage = path_address(path);
		path_steps_.pop_back();
	}
	if (!value_first)
		value = element != nullptr ? fit(member_operand(*element), type) : zero_value(type);
	// A volatile object's bit-field is volatile too (3.9.3p3), its unit
	// written by volatile accesses.
	qualify(unit, lowering_.graph().types.qualified(type, static_cast<uint8_t>(cv & sema::kVolatile)));
	unit.untyped = true;
	// Stored in the field's type, or in a packed class's unit of another
	// width.
	lowir::Type storage = field.declared;
	if (lowir::integer_bits(field.unit.kind()) != lowir::integer_bits(field.declared.kind()))
	{
		storage = field.unit;
		value = to_unit(field, value, type);
	}
	if (alternate)
	{
		bit_unit_ = field.unit_offset + 1;
		RValue merged;
		if (!whole)
		{
			const RValue old = load_unit(field, storage, unit);
			const RValue cleared = binary(lowir::BinaryOp::And, storage, old,
				integer(storage, FieldMask(storage, field.width, field.shift, true)));
			merged = cleared;
		}
		RValue positioned = binary(
			lowir::BinaryOp::And, storage, integer(storage, FieldMask(storage, field.width, 0, false)), value);
		if (field.shift != 0)
			positioned = binary(lowir::BinaryOp::Shl, storage, positioned, integer(storage, field.shift));
		if (!whole)
			positioned = binary(lowir::BinaryOp::Or, storage, merged, positioned);
		path_steps_.push_back(PathStep{ PathStep::Kind::Field, field.unit_offset, lowir::Type(), 0 });
		Place target = Place();
		target.storage = path_address(path);
		target.is_volatile = unit.is_volatile;
		target.untyped = true;
		path_steps_.pop_back();
		store_unit(field, storage, positioned, target);
		return;
	}
	const RValue masked = binary(
		lowir::BinaryOp::And, storage, integer(storage, FieldMask(storage, field.width, 0, false)), value);
	if (whole)
	{
		RValue positioned = masked;
		if (field.shift != 0)
			positioned = binary(lowir::BinaryOp::Shl, storage, masked, integer(storage, field.shift));
		store_unit(field, storage, positioned, unit);
	}
	else
		merge_bit_field(field, storage, unit, masked);
	bit_unit_ = field.unit_offset + 1;
}

// ------------------------------------------------ bit-field expressions

const sema::VariableEntity* FunctionLowering::designated_bit_field(const syntax::Expr& expr)
{
	// The member a bit-field glvalue designates: the one it names, or a
	// structured binding's member (C++17's 11.5p4).
	const sema::Entity* entity = WithoutParentheses(&expr)->facts->entity;
	if (const sema::BindingEntity* alias = sema::BindingAlias(entity))
		return alias->member;
	return static_cast<const sema::VariableEntity*>(entity);
}

const syntax::MemberExpr* FunctionLowering::bit_field_access(const syntax::Expr& expr, bool& found)
{
	const syntax::Expr* e = WithoutParentheses(&expr);
	found = false;
	const sema::ExprFacts* facts = e->facts;
	if (facts == nullptr || facts->entity == nullptr || facts->entity->kind != sema::EntityKind::Variable)
		return nullptr;
	if (const sema::BindingEntity* alias = sema::BindingAlias(facts->entity))
	{
		found = e->kind == syntax::ExprKind::Id && alias->member != nullptr && alias->member->bit_field;
		return nullptr;
	}
	const sema::VariableEntity* variable = static_cast<const sema::VariableEntity*>(facts->entity);
	if (!variable->bit_field || !variable->member)
		return nullptr;
	if (e->kind == syntax::ExprKind::Member)
	{
		found = true;
		return static_cast<const syntax::MemberExpr*>(e);
	}
	found = e->kind == syntax::ExprKind::Id && (facts->flags & sema::kImplicitThis) != 0;
	return nullptr;
}

RValue FunctionLowering::bit_field_value(const syntax::Expr& expr)
{
	// The value of a glvalue designating a bit-field.
	const syntax::Expr* e = WithoutParentheses(&expr);
	switch (e->kind)
	{
	case syntax::ExprKind::Assign:
	{
		const syntax::BinaryExpr& assign = static_cast<const syntax::BinaryExpr&>(*e);
		return assign.op == TokenKind::OP_ASS ? bit_field_assignment(assign) : bit_field_compound_assignment(assign);
	}
	case syntax::ExprKind::Unary:
		return bit_field_increment(static_cast<const syntax::UnaryExpr&>(*e), true);
	case syntax::ExprKind::Binary:
	{
		const syntax::BinaryExpr& comma = static_cast<const syntax::BinaryExpr&>(*e);
		if (comma.op != TokenKind::OP_COMMA)
			break;
		discard(*comma.left);
		return bit_field_value(*comma.right);
	}
	default:
	{
		bool found = false;
		const syntax::MemberExpr* access = bit_field_access(*e, found);
		if (!found)
			break;
		const BitField field = bit_field(designated_bit_field(*e), e->location);
		return read_bit_field(field, bit_field_unit(access, *e, field));
	}
	}
	throw LoweringError(e->location, "a bit-field glvalue outside the supported forms");
}

RValue FunctionLowering::bit_field_assignment(const syntax::BinaryExpr& expr)
{
	// 5.17 for a bit-field: the unit's address, then the value converted to
	// the field's type and merged into the unit.
	BitFieldUpdate update = assignment_update(expr);
	return update_bit_field(*expr.left, update);
}

RValue FunctionLowering::bit_field_compound_assignment(const syntax::BinaryExpr& expr)
{
	// 5.17p7: the field's value, the operation in its type, the result
	// converted back and merged.
	BitFieldUpdate update = assignment_update(expr);
	return update_bit_field(*expr.left, update);
}

FunctionLowering::BitFieldUpdate FunctionLowering::assignment_update(const syntax::BinaryExpr& expr)
{
	BitFieldUpdate update = BitFieldUpdate();
	update.source = &expr;
	update.location = expr.location;
	if (expr.op == TokenKind::OP_ASS)
	{
		update.kind = BitFieldUpdate::Kind::Assign;
		return update;
	}
	update.kind = BitFieldUpdate::Kind::Compound;
	update.operation = expr.facts->target;
	switch (expr.op)
	{
	case TokenKind::OP_PLUSASS:
		update.op = static_cast<uint32_t>(TokenKind::OP_PLUS);
		break;
	case TokenKind::OP_MINUSASS:
		update.op = static_cast<uint32_t>(TokenKind::OP_MINUS);
		break;
	case TokenKind::OP_STARASS:
		update.op = static_cast<uint32_t>(TokenKind::OP_STAR);
		break;
	case TokenKind::OP_DIVASS:
		update.op = static_cast<uint32_t>(TokenKind::OP_DIV);
		break;
	case TokenKind::OP_MODASS:
		update.op = static_cast<uint32_t>(TokenKind::OP_MOD);
		break;
	case TokenKind::OP_XORASS:
		update.op = static_cast<uint32_t>(TokenKind::OP_XOR);
		break;
	case TokenKind::OP_BANDASS:
		update.op = static_cast<uint32_t>(TokenKind::OP_AMP);
		break;
	case TokenKind::OP_BORASS:
		update.op = static_cast<uint32_t>(TokenKind::OP_BOR);
		break;
	case TokenKind::OP_LSHIFTASS:
		update.op = static_cast<uint32_t>(TokenKind::OP_LSHIFT);
		break;
	case TokenKind::OP_RSHIFTASS:
		update.op = static_cast<uint32_t>(TokenKind::OP_RSHIFT);
		break;
	default:
		update.op = static_cast<uint32_t>(expr.op);
		break;
	}
	return update;
}

RValue FunctionLowering::bit_field_increment(const syntax::UnaryExpr& expr, bool prefix)
{
	// 5.3.2, 5.2.6: the field's value stepped by one in its unit's type and
	// merged through the same unit.
	BitFieldUpdate update = BitFieldUpdate();
	update.kind = BitFieldUpdate::Kind::Increment;
	update.decrement = expr.op == TokenKind::OP_DEC;
	update.prefix = prefix;
	update.location = expr.location;
	return update_bit_field(*expr.operand, update);
}

const RValue& FunctionLowering::update_operand(BitFieldUpdate& update)
{
	// An assignment's right operand converted as the course converts a
	// bit-field's assigned value (a class object by its conversion function,
	// 13.3.1.2p7); a compound assignment's to the operation.
	if (update.evaluated || update.kind == BitFieldUpdate::Kind::Increment)
		return update.right;
	update.evaluated = true;
	const syntax::Expr& right = *update.source->right;
	if (update.kind == BitFieldUpdate::Kind::Compound)
	{
		update.right = operand(right, ConversionContext::CompoundAssignment);
		return update.right;
	}
	if (right.kind == syntax::ExprKind::BracedInit)
	{
		const syntax::BracedInitList& list = static_cast<const syntax::BracedInitList&>(right);
		update.right = list.elements.empty() ? zero_value(update.type)
											 : operand(*list.elements[0], ConversionContext::BitFieldAssignment);
	}
	else
		update.right = operand(right, ConversionContext::BitFieldAssignment);
	return update.right;
}

RValue FunctionLowering::update_bit_field(const syntax::Expr& target, BitFieldUpdate& update)
{
	const syntax::Expr* e = WithoutParentheses(&target);
	if (e->kind == syntax::ExprKind::Conditional)
	{
		// 5.16p4: the operand the condition selects is updated; the right
		// operand is evaluated first (an assignment's operands are
		// unsequenced, 5.17p1) and each operand's result kept in a slot.
		const syntax::ConditionalExpr& choice = static_cast<const syntax::ConditionalExpr&>(*e);
		update.type = e->facts->type;
		update_operand(update);
		// The operands' temporaries are guarded (5.16p1, 12.2p3).
		const lowir::BlockId then_block = new_block(lowir::BlockKind::CondThen);
		const lowir::BlockId else_block = new_block(lowir::BlockKind::CondElse);
		const lowir::BlockId end = new_block(lowir::BlockKind::CondEnd);
		begin_guard_scope();
		branch_condition(*choice.condition, then_block, else_block, false);
		place(then_block);
		++guarded_branches_;
		const RValue first = update_bit_field(*choice.then_expr, update);
		--guarded_branches_;
		Place slot = Place();
		slot.storage = lowir::Value::slot(generated_slot(lowir::SlotKind::Cond, first.type));
		store(first.type, first, slot);
		jump(end);
		place(else_block);
		++guarded_branches_;
		RValue second = update_bit_field(*choice.else_expr, update);
		--guarded_branches_;
		if (second.type != first.type)
			second = copy(first.type, second);
		store(first.type, second, slot);
		jump(end);
		place(end);
		--guard_scopes_;
		return load(first.type, slot);
	}
	if (e->facts == nullptr || (e->facts->flags & sema::kBitField) == 0)
		return apply_object_update(*e, update);
	BitField field;
	Place unit;
	if (e->kind == syntax::ExprKind::Binary && static_cast<const syntax::BinaryExpr*>(e)->op == TokenKind::OP_COMMA)
	{
		const syntax::BinaryExpr& comma = static_cast<const syntax::BinaryExpr&>(*e);
		discard(*comma.left);
		return update_bit_field(*comma.right, update);
	}
	if (!resolve_bit_field(*e, field, unit))
		throw LoweringError(e->location, "a bit-field designator outside the supported forms");
	if (update.kind == BitFieldUpdate::Kind::Increment && update.prefix && side_effect_free(e, false, 0))
	{
		// The course's prefix form addresses the field again to write it
		// (only where that repeats no side effect).
		const RValue old = read_bit_field(field, unit);
		const sema::Type* type = field.variable->type->unqualified;
		const sema::Type* operation = field_operation_type(field);
		const RValue widened = operation == type ? old
												 : convert(old, type, operation, ConversionContext::CompoundAssignment);
		const RValue next = stepped(widened, operation, update.decrement, update.location);
		resolve_bit_field(*e, field, unit);
		return write_bit_field(field, unit, to_unit(field, next, operation));
	}
	return apply_bit_field_update(field, unit, update);
}

bool FunctionLowering::resolve_bit_field(const syntax::Expr& target, BitField& field, Place& unit)
{
	const syntax::Expr* e = WithoutParentheses(&target);
	switch (e->kind)
	{
	case syntax::ExprKind::Binary:
	{
		const syntax::BinaryExpr& comma = static_cast<const syntax::BinaryExpr&>(*e);
		if (comma.op != TokenKind::OP_COMMA)
			break;
		discard(*comma.left);
		return resolve_bit_field(*comma.right, field, unit);
	}
	case syntax::ExprKind::Conditional:
		return false;
	case syntax::ExprKind::Assign:
	case syntax::ExprKind::Unary:
	{
		// An assignment or prefix increment designates its operand's
		// field once it is updated.
		BitFieldUpdate inner = BitFieldUpdate();
		const syntax::Expr* operand = nullptr;
		if (e->kind == syntax::ExprKind::Unary)
		{
			const syntax::UnaryExpr& step = static_cast<const syntax::UnaryExpr&>(*e);
			if (step.op != TokenKind::OP_INC && step.op != TokenKind::OP_DEC)
				break;
			inner.kind = BitFieldUpdate::Kind::Increment;
			inner.decrement = step.op == TokenKind::OP_DEC;
			inner.prefix = true;
			inner.location = e->location;
			operand = step.operand;
		}
		else
		{
			const syntax::BinaryExpr& assign = static_cast<const syntax::BinaryExpr&>(*e);
			inner = assignment_update(assign);
			operand = assign.left;
		}
		if (!resolve_bit_field(*operand, field, unit))
			return false;
		apply_bit_field_update(field, unit, inner);
		return true;
	}
	default:
	{
		bool found = false;
		const syntax::MemberExpr* access = bit_field_access(*e, found);
		if (!found)
			break;
		field = bit_field(designated_bit_field(*e), e->location);
		unit = bit_field_unit(access, *e, field);
		return true;
	}
	}
	throw LoweringError(e->location, "a bit-field glvalue outside the supported forms");
}

RValue FunctionLowering::apply_bit_field_update(const BitField& field, const Place& unit, BitFieldUpdate& update)
{
	const sema::Type* type = field.variable->type;
	update.type = type;
	switch (update.kind)
	{
	case BitFieldUpdate::Kind::Assign:
		return write_bit_field(field, unit, to_unit(field, fit(update_operand(update), type), type));
	case BitFieldUpdate::Kind::Compound:
	{
		const RValue old = read_bit_field(field, unit);
		const RValue right = update_operand(update);
		const sema::Type* unqualified = type->unqualified;
		const RValue widened = convert(old, unqualified, update.operation, ConversionContext::CompoundAssignment);
		RValue result = arithmetic_op(update.op, update.operation, widened, right);
		result = to_unit(field,
			fit(convert(result, update.operation, unqualified, ConversionContext::CompoundAssignment), unqualified),
			unqualified);
		return write_bit_field(field, unit, result);
	}
	default:
	{
		// Stepped in the type the field's value promotes to (5.3.2p1: as
		// E1 += 1), then written back.
		const RValue old = read_bit_field(field, unit);
		const sema::Type* operation = field_operation_type(field);
		const RValue widened = operation == type->unqualified
			? old
			: convert(old, type->unqualified, operation, ConversionContext::CompoundAssignment);
		const RValue next = stepped(widened, operation, update.decrement, update.location);
		const RValue written = write_bit_field(field, unit, to_unit(field, next, operation));
		return update.prefix ? written : old;
	}
	}
}

RValue FunctionLowering::apply_object_update(const syntax::Expr& target, BitFieldUpdate& update)
{
	// A conditional's operand that is no bit-field, updated as an object.
	const sema::Type* type = target.facts->type;
	const lowir::Type storage = lowering_.storage_type(type);
	update.type = type;
	const Place place = glvalue(target);
	switch (update.kind)
	{
	case BitFieldUpdate::Kind::Assign:
	{
		const RValue value = fit(update_operand(update), type);
		store(storage, value, place);
		return value;
	}
	case BitFieldUpdate::Kind::Compound:
	{
		const sema::Type* unqualified = type->unqualified;
		const RValue old = load(storage, place);
		const RValue right = update_operand(update);
		RValue result = arithmetic_op(update.op, update.operation,
			convert(old, unqualified, update.operation, ConversionContext::CompoundAssignment), right);
		result = fit(
			convert(result, update.operation, unqualified, ConversionContext::CompoundAssignment), unqualified);
		store(storage, result, place);
		return result;
	}
	default:
	{
		const RValue old = load(storage, place);
		const RValue next = stepped(old, type, update.decrement, update.location);
		store(storage, next, place);
		return update.prefix ? next : old;
	}
	}
}

} // namespace lower
