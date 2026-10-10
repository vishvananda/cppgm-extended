#include "lower/program_lowering.h"

#include <algorithm>

#include "lowir/literals.h"
#include "sema/analyzer.h"
#include "sema/constant_values.h"

// Constant initialization from evaluated values (3.6.2p2, PA16): the
// value the analysis's constant evaluation gave a variable's
// initialization (sema/constant_values.h) stated as LowIR data: scalars
// as items, address constants as symbol plus offset, class objects and
// arrays as their subobjects' items at their offsets (a polymorphic
// object's table pointer first), zero bytes elsewhere.
namespace lower {

namespace {

lowir::Constant IntegerItem(uint64_t bits)
{
	lowir::Constant constant;
	constant.kind = lowir::ValueKind::Integer;
	constant.float_kind = lowir::TypeKind::Void;
	constant.bits = lowir::integer_literal(static_cast<int64_t>(bits));
	return constant;
}

} // namespace

lowir::Literal IntegralLiteral(const sema::Type* type, uint64_t bits)
{
	lowir::Literal literal = lowir::integer_literal(static_cast<int64_t>(bits));
	sema::IntegralKind kind;
	if (type != nullptr && sema::IntegralKindOf(type, kind) && kind.bits > 64 && !kind.is_signed)
		literal.high = 0;
	return literal;
}

const sema::ConstValue* ProgramLowering::initialization_value(
	const sema::VariableEntity* variable, const syntax::Initializer* initializer)
{
	// The value the variable's initialization has when it is a constant
	// expression: evaluated by the analysis (constexpr objects, const
	// integral ones, references), else probed now (3.6.2p2's constant
	// initialization of any object).
	if (const sema::ConstValue* evaluated = analyzer_->evaluated_value(variable))
		return evaluated;
	if (initializer == nullptr && (variable->constructor == nullptr || variable->constructor->trivial))
		return nullptr;
	return analyzer_->constant_initialization(variable, initializer);
}

void ProgramLowering::demand_constructions(const syntax::Expr* expr, uint32_t depth)
{
	// The constructors a constant-initialized object's initializer names
	// (class prvalues, braced elements, converting constructors) are still
	// emitted, as its own constructor is (the course's form).
	while (expr != nullptr && expr->kind == syntax::ExprKind::Paren)
		expr = static_cast<const syntax::ParenExpr*>(expr)->inner;
	if (expr == nullptr || expr->facts == nullptr || depth > 256)
		return;
	const sema::ExprFacts* facts = expr->facts;
	if (facts->entity != nullptr && facts->entity->kind == sema::EntityKind::Function &&
		static_cast<const sema::FunctionEntity*>(facts->entity)->constructor &&
		!static_cast<const sema::FunctionEntity*>(facts->entity)->trivial)
		demand(static_cast<const sema::FunctionEntity*>(facts->entity));
	const sema::Conversion* conversion = facts->conversion;
	if (conversion != nullptr && conversion->constructor != nullptr && conversion->constructor->constructor &&
		!conversion->constructor->trivial)
		demand(conversion->constructor);
	switch (expr->kind)
	{
	case syntax::ExprKind::BracedInit:
		for (const syntax::Expr* element : static_cast<const syntax::BracedInitList*>(expr)->elements)
			demand_constructions(element, depth + 1);
		break;
	case syntax::ExprKind::FunctionalCast:
	{
		const syntax::FunctionalCastExpr* cast = static_cast<const syntax::FunctionalCastExpr*>(expr);
		for (const syntax::Expr* operand : cast->braced != nullptr ? cast->braced->elements : cast->arguments)
			demand_constructions(operand, depth + 1);
		break;
	}
	case syntax::ExprKind::Cast:
		demand_constructions(static_cast<const syntax::CastExpr*>(expr)->operand, depth + 1);
		break;
	default:
		break;
	}
}

void ProgramLowering::demand_initializer_calls(const syntax::Expr* expr, uint32_t depth)
{
	// The functions a constant-initialized reference's initializer odr-uses
	// (3.2p2) in either arm of its conditional, defined as its dynamic
	// initialization would define them (see read_through_storage).
	while (expr != nullptr && expr->kind == syntax::ExprKind::Paren)
		expr = static_cast<const syntax::ParenExpr*>(expr)->inner;
	if (expr == nullptr || expr->facts == nullptr || depth > 256)
		return;
	const sema::ExprFacts* facts = expr->facts;
	if (facts->entity != nullptr && facts->entity->kind == sema::EntityKind::Function &&
		(facts->flags & (sema::kDirectCall | sema::kOperatorCall | sema::kUserConverted)) != 0 &&
		!static_cast<const sema::FunctionEntity*>(facts->entity)->trivial)
		demand(static_cast<const sema::FunctionEntity*>(facts->entity));
	for (const sema::Conversion* conversion = facts->conversion; conversion != nullptr; conversion = conversion->after)
	{
		if (conversion->constructor != nullptr && !conversion->constructor->trivial)
			demand(conversion->constructor);
	}
	switch (expr->kind)
	{
	case syntax::ExprKind::Conditional:
	{
		const syntax::ConditionalExpr* choice = static_cast<const syntax::ConditionalExpr*>(expr);
		demand_initializer_calls(choice->condition, depth + 1);
		demand_initializer_calls(choice->then_expr, depth + 1);
		demand_initializer_calls(choice->else_expr, depth + 1);
		break;
	}
	case syntax::ExprKind::Binary:
	case syntax::ExprKind::Assign:
		demand_initializer_calls(static_cast<const syntax::BinaryExpr*>(expr)->left, depth + 1);
		demand_initializer_calls(static_cast<const syntax::BinaryExpr*>(expr)->right, depth + 1);
		break;
	case syntax::ExprKind::Unary:
		demand_initializer_calls(static_cast<const syntax::UnaryExpr*>(expr)->operand, depth + 1);
		break;
	case syntax::ExprKind::Cast:
		demand_initializer_calls(static_cast<const syntax::CastExpr*>(expr)->operand, depth + 1);
		break;
	case syntax::ExprKind::Member:
		demand_initializer_calls(static_cast<const syntax::MemberExpr*>(expr)->base, depth + 1);
		break;
	case syntax::ExprKind::Call:
	{
		const syntax::CallExpr* call = static_cast<const syntax::CallExpr*>(expr);
		demand_initializer_calls(call->callee, depth + 1);
		for (const syntax::Expr* argument : call->arguments)
			demand_initializer_calls(argument, depth + 1);
		break;
	}
	default:
		break;
	}
}

void ProgramLowering::demand_constructions(const syntax::Initializer* initializer)
{
	if (initializer == nullptr)
		return;
	if (initializer->expression != nullptr)
		demand_constructions(initializer->expression, 0);
	for (const syntax::Expr* argument : initializer->arguments)
		demand_constructions(argument, 0);
}

bool ProgramLowering::constant_item(
	const sema::Type* type, const sema::ConstValue& value, lowir::DataItem& item, uint32_t location)
{
	// One scalar's item: a value-initialization's arithmetic zero as its
	// type's zero, like any other evaluated element (the course's form),
	// another zero as zero bytes, an address constant as symbol plus offset.
	item = lowir::DataItem();
	item.type = type->is_reference() ? lowir::Type::scalar(lowir::TypeKind::Ptr) : storage_type(type);
	if ((value.flags & sema::kValueZeroed) != 0 && value.kind == sema::ValueKind::Floating && item.type.is_float())
	{
		item.kind = lowir::DataItemKind::Scalar;
		item.constant.kind = lowir::ValueKind::Float;
		item.constant.float_kind = item.type.kind();
		item.constant.bits = lowir::float_literal(item.type.kind(), 0);
		return true;
	}
	if (((value.flags & sema::kValueZeroed) != 0 &&
			(value.kind != sema::ValueKind::Integer || !item.type.is_integer())) ||
		value.kind == sema::ValueKind::Indeterminate)
	{
		item.kind = lowir::DataItemKind::Zero;
		item.amount = static_cast<int64_t>(size_of(type, location));
		return true;
	}
	switch (value.kind)
	{
	case sema::ValueKind::Integer:
		item.kind = lowir::DataItemKind::Scalar;
		item.constant = IntegerItem(value.bits);
		item.constant.bits = IntegralLiteral(type, value.bits);
		if (sema::IsWideIntegral(type))
			item.constant.bits.high = value.high;
		return item.type.is_integer();
	case sema::ValueKind::Floating:
		item.kind = lowir::DataItemKind::Scalar;
		if (SoftFloatOf(type) == SoftFloat::BFloat)
		{
			// A bfloat16 value (the evaluation holds it exactly): its
			// encoding as an integer of its size, as static_item states it.
			StaticValue floating = StaticValue();
			floating.kind = StaticValue::Kind::Floating;
			floating.floating = value.floating;
			StaticValue encoded = StaticValue();
			item.type = lowir::Type::scalar(lowir::TypeKind::U16);
			item.constant = IntegerItem(0);
			if (!SoftStaticValue(floating, nullptr, SoftFloat::BFloat, encoded))
				return false;
			item.constant = IntegerItem(encoded.bits);
			return true;
		}
		item.constant.kind = lowir::ValueKind::Float;
		item.constant.float_kind = item.type.kind();
		item.constant.bits = lowir::float_literal(item.type.kind(), value.floating);
		return item.type.is_float();
	case sema::ValueKind::Pointer:
		switch (value.root)
		{
		case sema::RootKind::Null:
			item.kind = lowir::DataItemKind::Zero;
			item.amount = 8;
			return true;
		case sema::RootKind::Variable:
		{
			// A block-scope static is its function's global (6.7p4), made
			// where its declaration was lowered.
			const sema::VariableEntity* variable = static_cast<const sema::VariableEntity*>(value.address.entity);
			if (variable->is_static && variable->owner != nullptr &&
				(variable->owner->kind == sema::ScopeKind::Block || variable->owner->kind == sema::ScopeKind::Function))
			{
				const uint32_t local = local_static_index_.find(variable);
				if (local == PointerMap::kMissing)
					return false;
				item.address = local_statics_[local].symbol;
				break;
			}
			item.address = variable_symbol(variable);
			break;
		}
		case sema::RootKind::Literal:
			item.address = string_literal(*static_cast<const syntax::LiteralExpr*>(value.address.entity));
			break;
		case sema::RootKind::Temporary:
			item.address = constant_temporary(static_cast<const sema::ConstValue*>(value.address.entity), location);
			if (item.address == lowir::kNoId)
				return false;
			break;
		case sema::RootKind::Function:
		{
			const sema::FunctionEntity* function = static_cast<const sema::FunctionEntity*>(value.address.entity);
			demand(function);
			item.address = function_symbol(function);
			break;
		}
		case sema::RootKind::TypeInfo:
			item.address = type_information(static_cast<const sema::Type*>(value.address.entity));
			break;
		default:
			return false;
		}
		item.kind = lowir::DataItemKind::Address;
		item.type = lowir::Type::scalar(lowir::TypeKind::Ptr);
		item.amount = value.address.offset;
		return true;
	default:
		return false;
	}
}

namespace {

// A data item's bytes and alignment as the object writer places it.
uint64_t DataItemBytes(const lowir::DataItem& item, uint64_t& align)
{
	align = 1;
	if (item.kind == lowir::DataItemKind::Zero)
		return static_cast<uint64_t>(item.amount);
	if (item.kind == lowir::DataItemKind::Address || item.type.kind() == lowir::TypeKind::Ptr)
	{
		align = 8;
		return 8;
	}
	if (item.type.kind() == lowir::TypeKind::Obj)
	{
		align = uint64_t(1) << item.type.align_log2;
		return item.type.bytes;
	}
	const unsigned bits = lowir::integer_bits(item.type.kind());
	const unsigned rank = lowir::float_rank(item.type.kind());
	const uint64_t size = bits != 0 ? (bits <= 8 ? 1 : bits / 8) : rank == 1 ? 4 : rank == 2 ? 8 : 16;
	align = size;
	return size;
}

} // namespace

bool ProgramLowering::holds_data_member_pointer(const sema::Type* type)
{
	// A pointer to data member, an array of them, or a class with one in a
	// base or member (a union's first member only, 8.5p5): its zero-
	// initialization writes the null value -1 there. Memoized per class.
	type = type->canonical;
	while (type->kind == sema::TypeKind::Array)
		type = type->element;
	const sema::Type* bare = type->unqualified;
	if (bare->kind == sema::TypeKind::MemberPointer)
		return bare->element->kind != sema::TypeKind::Function;
	const sema::ClassEntity* cls = bare->kind == sema::TypeKind::Class ? sema::ClassOf(bare) : nullptr;
	if (cls == nullptr || !cls->complete)
		return false;
	const uint32_t known = data_member_pointer_classes_.find(cls);
	if (known != PointerMap::kMissing)
		return known == 2;
	bool holds = false;
	for (const sema::BaseClass* base = cls->bases; base != nullptr && !holds; base = base->next)
		holds = holds_data_member_pointer(static_cast<const sema::ClassEntity*>(base->base)->type);
	for (const sema::DataMember* member = cls->members; member != nullptr && !holds; member = member->next)
	{
		const sema::Type* member_type = member->variable->type;
		holds = !member->bit_field && !member_type->is_reference() && holds_data_member_pointer(member_type);
		if (cls->key == TokenKind::KW_UNION)
			break;
	}
	data_member_pointer_classes_.set(cls, holds ? 2 : 1);
	return holds;
}

void ProgramLowering::null_fill(
	const sema::Type* type, uint64_t base, uint64_t lo, uint64_t hi, bool complete, StaticData& out)
{
	// Bytes [lo, hi) of a zero-initialized object of `type` at `base`: zero
	// items, -1 for each pointer to data member wholly inside (8.5p5); an
	// array's run of whole elements as the first's items, then copies of
	// them (LowIR states every item).
	if (lo >= hi)
		return;
	if (!holds_data_member_pointer(type))
		return append_zero(hi - lo, out);
	type = type->canonical;
	if (type->unqualified->kind == sema::TypeKind::MemberPointer)
	{
		if (lo != base || hi != base + 8)
			return append_zero(hi - lo, out);
		lowir::DataItem null = lowir::DataItem();
		null.kind = lowir::DataItemKind::Scalar;
		null.type = lowir::Type::scalar(lowir::TypeKind::I64);
		null.constant = IntegerItem(static_cast<uint64_t>(-1));
		out.items.push_back(null);
		return;
	}
	if (type->kind == sema::TypeKind::Array)
	{
		const uint64_t size = size_of(type->element, 0);
		uint64_t i = (lo - base) / size;
		const uint64_t whole = (hi - base) / size;
		if ((lo - base) % size != 0)
		{
			const uint64_t end = base + (i + 1) * size;
			null_fill(type->element, base + i * size, lo, end < hi ? end : hi, true, out);
			++i;
		}
		if (i < whole)
		{
			const uint64_t at = base + i * size;
			const size_t first = out.items.size();
			null_fill(type->element, at, at, at + size, true, out);
			const size_t last = out.items.size();
			out.items.reserve(last + (last - first) * (whole - i - 1));
			for (uint64_t copy = i + 1; copy < whole; ++copy)
				for (size_t k = first; k < last; ++k)
					out.items.push_back(out.items[k]);
			i = whole;
		}
		if (base + i * size < hi)
			null_fill(type->element, base + i * size, base + i * size, hi, true, out);
		return;
	}
	// A class: its subobjects holding one in offset order (non-virtual
	// bases, members, a complete object's virtual bases; a union's first
	// member), the bytes between them zero.
	const sema::ClassEntity* cls = sema::ClassOf(type);
	const NullSpan span = null_pieces(cls, complete);
	// The first piece reaching past `lo` (pieces before it end at or
	// before it), up to the first starting at or past `hi`.
	uint32_t first = span.first;
	for (uint32_t count = span.count; count > 0;)
	{
		const uint32_t half = count / 2;
		if (null_piece_store_[first + half].reach <= lo - base)
		{
			first += half + 1;
			count -= half + 1;
		}
		else
			count = half;
	}
	uint64_t at = lo;
	for (uint32_t k = first; k < span.first + span.count; ++k)
	{
		const NullPiece piece = null_piece_store_[k];
		const uint64_t offset = base + piece.offset;
		if (offset >= hi)
			break;
		const uint64_t from = offset > at ? offset : at;
		const uint64_t end = base + piece.end;
		const uint64_t to = end < hi ? end : hi;
		if (from >= to)
			continue;
		append_zero(from - at, out);
		null_fill(piece.type, offset, from, to, piece.complete, out);
		at = to;
	}
	append_zero(hi - at, out);
}

NullSpan ProgramLowering::null_pieces(const sema::ClassEntity* cls, bool complete)
{
	// A class's subobjects holding a pointer to data member, in offset
	// order (non-virtual bases, members, a complete object's virtual bases;
	// a union's first member), offsets from the class's; each one's end
	// and the largest end so far. Made once per class and completeness in
	// a unit (null_fill visits a class once per zero range).
	const void* key = reinterpret_cast<const char*>(cls) + (complete ? 1 : 0);
	const uint32_t known = null_piece_spans_.find(key);
	if (known != PointerMap::kMissing)
		return null_spans_[known];
	size_of(cls->type, 0);
	const uint32_t first = static_cast<uint32_t>(null_piece_store_.size());
	auto add = [&](uint64_t offset, const sema::Type* type, bool whole) {
		if (holds_data_member_pointer(type))
			null_piece_store_.push_back(NullPiece{ offset, type, whole, offset + size_of(type, 0), 0 });
	};
	for (const sema::BaseClass* edge = cls->bases; edge != nullptr; edge = edge->next)
		if (!edge->is_virtual)
			add(edge->offset, static_cast<const sema::ClassEntity*>(edge->base)->type, false);
	for (const sema::DataMember* member = cls->members; member != nullptr; member = member->next)
	{
		if (!member->bit_field && !member->variable->type->is_reference())
			add(member->offset / 8, member->variable->type, true);
		if (cls->key == TokenKind::KW_UNION)
			break;
	}
	for (uint32_t i = 0; complete && i < sema::VirtualBaseCount(cls); ++i)
	{
		const sema::BaseClass* edge = sema::VirtualBases(cls)[i].base;
		add(edge->offset, static_cast<const sema::ClassEntity*>(edge->base)->type, false);
	}
	std::stable_sort(null_piece_store_.begin() + first, null_piece_store_.end(),
		[](const NullPiece& a, const NullPiece& b) { return a.offset < b.offset; });
	uint64_t reach = 0;
	for (size_t k = first; k < null_piece_store_.size(); ++k)
	{
		reach = null_piece_store_[k].end > reach ? null_piece_store_[k].end : reach;
		null_piece_store_[k].reach = reach;
	}
	const NullSpan span = NullSpan{ first, static_cast<uint32_t>(null_piece_store_.size()) - first };
	null_piece_spans_.set(key, static_cast<uint32_t>(null_spans_.size()));
	null_spans_.push_back(span);
	return span;
}

void ProgramLowering::null_member_pointers(const sema::Type* type, StaticData& data)
{
	// A statically initialized object's null pointers to data members: each
	// one its data leaves as zero bytes becomes -1, its zero items rebuilt
	// (null_fill). Data without a zero item stays.
	if (!holds_data_member_pointer(type))
		return;
	bool zero = false;
	for (const lowir::DataItem& item : data.items)
		zero = zero || item.kind == lowir::DataItemKind::Zero;
	if (!zero)
		return;
	StaticData items;
	items.items.reserve(data.items.size() + 8);
	uint64_t at = 0;
	for (const lowir::DataItem& item : data.items)
	{
		// A typed item sits at its alignment (native/image/global_data).
		uint64_t align = 1;
		const uint64_t size = DataItemBytes(item, align);
		if (item.kind != lowir::DataItemKind::Zero)
		{
			at = (at + align - 1) & ~(align - 1);
			items.items.push_back(item);
		}
		else
			null_fill(type, 0, at, at + size, true, items);
		at += size;
	}
	data.items.swap(items.items);
}

uint32_t ProgramLowering::member_pointer_items(const sema::ConstValue& value, lowir::DataItem* items, uint32_t location)
{
	// A pointer to member's words (lower/member_pointer_lowering.cpp): a
	// data member's offset (null_data_member when null); a member function's
	// address, or one plus its table slot's offset for a virtual one, then
	// the adjustment, each an i64 (a null one's two zeros: an i128 item would
	// be aligned to 16, moving the members after it). Returns the item count.
	const sema::Entity* member = static_cast<const sema::Entity*>(value.address.entity);
	const bool function = value.type->element->kind == sema::TypeKind::Function;
	lowir::DataItem scalar = lowir::DataItem();
	scalar.kind = lowir::DataItemKind::Scalar;
	scalar.type = lowir::Type::scalar(lowir::TypeKind::I64);
	scalar.constant = IntegerItem(0);
	if (member == nullptr)
	{
		items[0] = scalar;
		items[1] = scalar;
		if (!function)
			items[0].constant = IntegerItem(static_cast<uint64_t>(null_data_member()));
		return function ? 2 : 1;
	}
	if (!function)
	{
		scalar.constant = IntegerItem(member_home_offset(static_cast<const sema::VariableEntity*>(member), location) +
			static_cast<uint64_t>(value.address.offset));
		items[0] = scalar;
		return 1;
	}
	const sema::FunctionEntity* target = static_cast<const sema::FunctionEntity*>(member);
	items[0] = scalar;
	if (target->is_virtual)
		items[0].constant = IntegerItem(1 + static_cast<uint64_t>(target->vtable_slot) * 8);
	else
	{
		demand(target);
		items[0].kind = lowir::DataItemKind::Address;
		items[0].type = lowir::Type::scalar(lowir::TypeKind::Ptr);
		items[0].address = function_symbol(target);
		items[0].amount = 0;
	}
	items[1] = scalar;
	items[1].constant = IntegerItem(static_cast<uint64_t>(value.address.offset));
	return 2;
}

void ProgramLowering::place_tables(const sema::ClassEntity* cls, uint64_t offset, std::vector<StaticPlacement>& out)
{
	// The class's table group defined first (it may define its bases'
	// groups, growing segment_offsets_).
	vtable_symbol(cls);
	const sema::VtableGroup* group = cls->virtuals->group;
	const uint64_t* offsets = group != nullptr ? segment_offsets(cls) : nullptr;
	const uint32_t count = group != nullptr ? group->segment_count : 1;
	for (uint32_t i = 0; i < count; ++i)
	{
		if (i != 0 && group->segments[i].shared)
			continue;
		const TableAddress address = table_address(cls, i);
		lowir::DataItem table = lowir::DataItem();
		table.kind = lowir::DataItemKind::Address;
		table.type = lowir::Type::scalar(lowir::TypeKind::Ptr);
		table.address = address.symbol;
		table.amount = address.addend;
		out.push_back(StaticPlacement{ offset + (i != 0 ? offsets[i] : 0), 8, table });
	}
}

bool ProgramLowering::constant_placements(const sema::Type* type, const sema::ConstValue& value, uint64_t offset,
	std::vector<StaticPlacement>& out, std::vector<uint8_t>& bits, uint32_t location, uint32_t depth, bool table_placed)
{
	// The items of a value at their byte offsets; a bit-field's bits in
	// `bits` (the object's bytes), its units' bytes items of their own
	// (marked by amount -1). `table_placed`: a derived class's table
	// pointer already fills this object's (a primary base's, Itanium 2.4).
	if (depth > 256)
		return false;
	if (type->kind == sema::TypeKind::Array)
	{
		const uint64_t size = size_of(type->element, location);
		for (uint32_t i = 0; i < value.count; ++i)
		{
			if (!constant_placements(
					type->element, value.elements[i], offset + i * size, out, bits, location, depth + 1, false))
				return false;
		}
		return true;
	}
	const sema::ClassEntity* cls = type->is_reference() ? nullptr : sema::ClassOf(type);
	if (!type->is_reference() && type->unqualified->kind == sema::TypeKind::Complex)
	{
		// A GNU complex value: its two parts (sema/constant_complex.cpp),
		// real then imaginary.
		if (value.kind != sema::ValueKind::Array || value.count != 2)
			return value.kind == sema::ValueKind::Indeterminate;
		const sema::Type* part = type->unqualified->element;
		const uint64_t size = size_of(part, location);
		return constant_placements(part, value.elements[0], offset, out, bits, location, depth + 1, false) &&
			constant_placements(part, value.elements[1], offset + size, out, bits, location, depth + 1, false);
	}
	if (!type->is_reference() && sema::LaneVector(type) && value.kind == sema::ValueKind::Array)
	{
		// A GNU vector value: its lanes in order (sema::LaneVector).
		const sema::Type* lane = type->unqualified->element;
		const uint64_t size = size_of(lane, location);
		if (value.count != type->unqualified->bound)
			return false;
		for (uint32_t i = 0; i < value.count; ++i)
		{
			if (!constant_placements(lane, value.elements[i], offset + i * size, out, bits, location, depth + 1, false))
				return false;
		}
		return true;
	}
	if (value.kind == sema::ValueKind::MemberPointer)
	{
		lowir::DataItem items[2];
		const uint32_t count = member_pointer_items(value, items, location);
		for (uint32_t i = 0; i < count; ++i)
			out.push_back(StaticPlacement{ offset + 8 * i, 8, items[i] });
		return true;
	}
	if (cls == nullptr)
	{
		lowir::DataItem item;
		if (!constant_item(type, value, item, location))
			return false;
		if (item.kind != lowir::DataItemKind::Zero)
			out.push_back(StaticPlacement{ offset, type->is_reference() ? 8 : size_of(type, location), item });
		return true;
	}
	if (value.kind != sema::ValueKind::Class)
		return value.kind == sema::ValueKind::Indeterminate;
	size_of(cls->type, location);
	if (cls->polymorphic)
	{
		// Its table pointers: at its start (the primary chain's bases share
		// the slot, 10.3, Itanium 2.4), and a table group's other segments'
		// at their subobjects (the complete object's tables, which its
		// bases' then do not place).
		if (cls->virtuals == nullptr)
			return false;
		if (!table_placed)
			place_tables(cls, offset, out);
	}
	uint32_t index = 0;
	for (const sema::BaseClass* base = cls->bases; base != nullptr; base = base->next, ++index)
	{
		if (base->is_virtual)
			return false;
		const sema::ClassEntity* base_class = static_cast<const sema::ClassEntity*>(base->base);
		const bool primary = cls->polymorphic && base_class->polymorphic &&
			(base->offset == 0 || cls->virtuals->group != nullptr);
		if (!constant_placements(base_class->type, value.elements[index], offset + base->offset, out, bits, location,
				depth + 1, primary))
			return false;
	}
	const bool is_union = cls->key == TokenKind::KW_UNION;
	for (const sema::DataMember* member = cls->members; member != nullptr; member = member->next)
	{
		if (member->variable->kind != sema::EntityKind::Variable ||
			(member->bit_field && member->variable->name == nullptr))
			continue;
		if (is_union && value.extra != member->ordinal + 1)
			continue;
		const sema::VariableEntity* variable = static_cast<const sema::VariableEntity*>(member->variable);
		const sema::ConstValue& element = value.elements[index + member->ordinal];
		if (member->bit_field)
		{
			if (element.kind != sema::ValueKind::Integer)
				continue;
			// A 128-bit field's bits past 64 are its value's high half (9.6p1:
			// a field of a type of at most 64 bits has no value bits there).
			const uint64_t bit = offset * 8 + member_bit_offset(variable, location);
			for (uint64_t k = 0; k < member->width && k < 128; ++k)
			{
				const size_t byte = static_cast<size_t>((bit + k) / 8);
				if (byte >= bits.size())
					return false;
				const uint64_t word = k < 64 ? element.bits >> k : element.high >> (k - 64);
				bits[byte] |= static_cast<uint8_t>((word & 1u) << ((bit + k) % 8));
			}
			// The field's storage unit, as one unsigned item of its width
			// (its value assembled from `bits`, see constant_data); a shared
			// or split unit is written at run time.
			if (member->unit_shared || member->unit_split != 0 || member->unit_bytes > 8)
				return false;
			lowir::DataItem unit = lowir::DataItem();
			unit.kind = lowir::DataItemKind::Scalar;
			unit.type = lowir::Type::scalar(member->unit_bytes == 1 ? lowir::TypeKind::U8
					: member->unit_bytes == 2                       ? lowir::TypeKind::U16
					: member->unit_bytes == 4                       ? lowir::TypeKind::U32
																	: lowir::TypeKind::I64);
			unit.amount = -1;
			out.push_back(StaticPlacement{ offset + member->unit_offset, member->unit_bytes, unit });
			continue;
		}
		if (!constant_placements(variable->type, element, offset + member_offset(variable, location), out, bits,
				location, depth + 1, false))
			return false;
	}
	return true;
}

bool ProgramLowering::scalar_array_data(
	const sema::Type* type, const sema::ConstValue& value, StaticData& out, uint32_t location, uint32_t depth)
{
	// An array of scalars: its elements' items, already in offset order.
	if (value.kind == sema::ValueKind::MemberPointer)
	{
		lowir::DataItem items[2];
		const uint32_t count = member_pointer_items(value, items, location);
		for (uint32_t i = 0; i < count; ++i)
			append_item(items[i], out);
		return true;
	}
	if (type->kind != sema::TypeKind::Array &&
		(type->unqualified->kind == sema::TypeKind::Complex || sema::LaneVector(type)) &&
		value.kind == sema::ValueKind::Array)
	{
		// A GNU complex element: its two parts, real then imaginary; a
		// vector element: its lanes (see constant_placements).
		const sema::Type* part = type->unqualified->element;
		const uint64_t count = type->unqualified->kind == sema::TypeKind::Complex ? 2 : type->unqualified->bound;
		if (value.count != count)
			return false;
		for (uint32_t i = 0; i < value.count; ++i)
		{
			if (!scalar_array_data(part, value.elements[i], out, location, depth + 1))
				return false;
		}
		return true;
	}
	if (type->kind != sema::TypeKind::Array)
	{
		lowir::DataItem item;
		if (!constant_item(type, value, item, location))
			return false;
		append_item(item, out);
		return true;
	}
	if (value.kind != sema::ValueKind::Array || value.count != type->bound || depth > 256)
		return false;
	for (uint32_t i = 0; i < value.count; ++i)
	{
		if (!scalar_array_data(type->element, value.elements[i], out, location, depth + 1))
			return false;
	}
	return true;
}

bool ProgramLowering::constant_data(
	const sema::Type* type, const sema::ConstValue& value, StaticData& out, uint32_t location)
{
	// The object's bytes in offset order: its items, zero bytes between.
	const sema::Type* element = type;
	while (element->kind == sema::TypeKind::Array)
		element = element->element;
	if (type->kind == sema::TypeKind::Array && !element->is_reference() && sema::ClassOf(element) == nullptr)
		return scalar_array_data(type, value, out, location, 0);
	const uint64_t size = size_of(type, location);
	std::vector<StaticPlacement> placements;
	std::vector<uint8_t> bits(static_cast<size_t>(size), 0);
	if (!constant_placements(type, value, 0, placements, bits, location, 0, false))
		return false;
	std::stable_sort(placements.begin(), placements.end(),
		[](const StaticPlacement& a, const StaticPlacement& b) { return a.offset < b.offset; });
	uint64_t position = 0;
	for (StaticPlacement& placement : placements)
	{
		const bool bit_byte = placement.item.kind == lowir::DataItemKind::Scalar && placement.item.amount == -1;
		if (placement.offset < position)
		{
			// A bit-field unit's byte another field of the unit placed.
			if (bit_byte)
				continue;
			return false;
		}
		if (bit_byte)
		{
			uint64_t unit = 0;
			for (uint64_t i = placement.size; i-- > 0;)
				unit = unit << 8 | bits[static_cast<size_t>(placement.offset + i)];
			placement.item.amount = 0;
			placement.item.constant = IntegerItem(unit);
		}
		append_zero(placement.offset - position, out);
		append_item(placement.item, out);
		position = placement.offset + placement.size;
	}
	if (size > position)
		append_zero(size - position, out);
	return true;
}

lowir::SymbolId ProgramLowering::constant_temporary(const sema::ConstValue* temporary, uint32_t location)
{
	// The temporary a constant-initialized reference is bound to (12.2p5,
	// 3.6.2p2): an internal global of its constant value, one per
	// temporary; kNoId when the value has no LowIR statement.
	const uint32_t found = constant_temporaries_.find(temporary);
	if (found != PointerMap::kMissing)
		return found;
	const sema::Type* type = temporary->type;
	lowir::Global global = lowir::Global();
	if (type->kind == sema::TypeKind::Array || type->unqualified->kind == sema::TypeKind::Class)
	{
		StaticData data;
		if (!constant_data(type, *temporary, data, location))
			return lowir::kNoId;
		global.init = lowir::GlobalInit::Data;
		global.first_item = static_cast<uint32_t>(program_.data_items.size());
		global.item_count = static_cast<uint32_t>(data.items.size());
		program_.data_items.insert(program_.data_items.end(), data.items.begin(), data.items.end());
	}
	else
	{
		global.typed = true;
		global.type = storage_type(type);
		if (!constant_global(type, *temporary, global, location))
			return lowir::kNoId;
	}
	const lowir::SymbolId symbol = new_symbol("__ref_tmp__" + std::to_string(++static_temporaries_));
	program_.declare_global(symbol, lowir::SymbolKind::GlobalDefinition, global);
	program_.symbols[symbol].metadata.binding = ir_model::SBM_INTERNAL;
	name_temporary(symbol);
	++counters_.globals;
	constant_temporaries_.set(temporary, symbol);
	return symbol;
}

bool ProgramLowering::constant_global(
	const sema::Type* type, const sema::ConstValue& value, lowir::Global& global, uint32_t location)
{
	// A scalar global's initializer: its constant, or an address constant;
	// an initialized null pointer is the constant 0 (the course's form).
	lowir::DataItem item;
	if (!constant_item(type, value, item, location))
		return false;
	switch (item.kind)
	{
	case lowir::DataItemKind::Scalar:
		global.init = lowir::GlobalInit::Constant;
		global.constant = item.constant;
		// A bfloat16 value: its encoding's integer type.
		if (SoftFloatOf(type) != SoftFloat::None)
			global.type = item.type;
		return true;
	case lowir::DataItemKind::Address:
		global.init = lowir::GlobalInit::Address;
		global.address = item.address;
		global.addend = item.amount;
		return true;
	default:
		if (value.kind == sema::ValueKind::Pointer && value.root == sema::RootKind::Null)
		{
			global.init = lowir::GlobalInit::Constant;
			global.constant = IntegerItem(0);
			return true;
		}
		return false;
	}
}

} // namespace lower
