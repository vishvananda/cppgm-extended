#include "lower/function_lowering.h"

#include "sema/conversions.h"

// Run-time type queries: typeid (5.2.8) designates a type
// information object, the dynamic type's read from the object's virtual
// table (Itanium 2.5.2: the slot before the function addresses); a
// comparison of two typeid results compares the objects' addresses; a
// checked dynamic_cast (5.2.7p8) calls the runtime's __dynamic_cast. The
// course's forms: a dynamic query of an object reached through a pointer
// tests the address for null first (a null one calls the runtime's
// failure, which does not return); a reference operand is not tested,
// since a reference cannot be null; and a cast's result goes through a
// slot of its own.
namespace lower {

namespace {

const syntax::Expr* WithoutParentheses(const syntax::Expr* expr)
{
	while (expr->kind == syntax::ExprKind::Paren)
		expr = static_cast<const syntax::ParenExpr*>(expr)->inner;
	return expr;
}

// The base subobjects a hint's walk may visit; past it the cast has no
// hint (-1, always valid).
const uint32_t kHintWork = 1u << 16;

// A hint from a base's public paths: their count, whether one is through a
// virtual base, the offset of the (last) one.
int64_t HintOf(uint32_t public_count, bool virtual_found, uint64_t found_offset)
{
	if (public_count == 0)
		return -2;
	if (public_count == 1 && !virtual_found)
		return static_cast<int64_t>(found_offset);
	return virtual_found ? -1 : -3;
}

} // namespace

int64_t ProgramLowering::dynamic_cast_hint(
	const sema::ClassEntity* base, const sema::ClassEntity* derived, uint32_t location)
{
	// Itanium 2.9.7's src2dst hint for a cast from `base` to `derived`: the
	// offset of `base` when it is a unique public non-virtual base, -2 when
	// it is no public base (unrelated, or reached through non-public bases
	// only), -3 when it is a public base several times but never through a
	// virtual base, else -1 (a path through a virtual base: no static
	// offset; the walk follows every base-specifier path from `derived`, so
	// a shared virtual base's paths each count). Memoized per pair; a
	// second cast to the class the last walk was for takes every base's
	// facts in one walk, which then answers the casts to that class (unless
	// the walk passed its limit, where each pair's own walk decides).
	if (derived->bases == nullptr)
		return hint_walk(base, derived, location);
	const ClassPair key = ClassPair{ derived, base };
	if (const int64_t* known = cast_hints_.find(key))
		return *known;
	int64_t hint = 0;
	if (hint_class_ == derived && !hint_facts_taken_)
	{
		hint_walk(nullptr, derived, location);
		hint_facts_taken_ = true;
	}
	if (hint_class_ == derived && !hint_overflow_)
		hint = walked_hint(base);
	else
	{
		if (hint_class_ != derived)
		{
			hint_class_ = derived;
			hint_facts_taken_ = false;
			hint_overflow_ = false;
		}
		hint = hint_walk(base, derived, location);
	}
	cast_hints_.insert(key, hint);
	return hint;
}

int64_t ProgramLowering::hint_walk(const sema::ClassEntity* base, const sema::ClassEntity* derived, uint32_t location)
{
	// Each base-specifier path from `derived`, to `base` (where a path
	// ends), or to every class (null `base`: each class's public paths'
	// facts in hint_facts_, and hint_overflow_ when the walk passes its
	// limit; it visits every path a walk to one class does).
	std::vector<HintStep>& stack = hint_stack_;
	stack.assign(1, HintStep{ derived, 0, true, false });
	if (base == nullptr)
	{
		hint_index_.clear();
		hint_facts_.clear();
	}
	uint32_t public_count = 0, work = 0;
	bool virtual_found = false;
	uint64_t found_offset = 0;
	while (!stack.empty())
	{
		const HintStep step = stack.back();
		stack.pop_back();
		if (++work > kHintWork)
		{
			if (base == nullptr)
				hint_overflow_ = true;
			return -1;
		}
		if (step.cls == base)
		{
			if (step.public_path)
			{
				++public_count;
				virtual_found = virtual_found || step.virtual_path;
				found_offset = step.offset;
			}
			continue;
		}
		if (base == nullptr && step.public_path)
		{
			const uint32_t known = hint_index_.find(step.cls);
			if (known == PointerMap::kMissing)
			{
				hint_index_.set(step.cls, static_cast<uint32_t>(hint_facts_.size()));
				hint_facts_.push_back(HintFacts{ 1, step.virtual_path, step.offset });
			}
			else
			{
				HintFacts& facts = hint_facts_[known];
				++facts.public_count;
				facts.virtual_path = facts.virtual_path || step.virtual_path;
				facts.offset = step.offset;
			}
		}
		size_of(step.cls->type, location);
		for (const sema::BaseClass* next = step.cls->bases; next != nullptr; next = next->next)
		{
			const sema::ClassEntity* cls = static_cast<const sema::ClassEntity*>(next->base);
			stack.push_back(HintStep{ cls, step.offset + next->offset,
				step.public_path && next->access == sema::Access::Public, step.virtual_path || next->is_virtual });
		}
	}
	return HintOf(public_count, virtual_found, found_offset);
}

int64_t ProgramLowering::walked_hint(const sema::ClassEntity* base) const
{
	const uint32_t known = hint_index_.find(base);
	if (known == PointerMap::kMissing)
		return HintOf(0, false, 0);
	const HintFacts& facts = hint_facts_[known];
	return HintOf(facts.public_count, facts.virtual_path, facts.offset);
}

lowir::Value FunctionLowering::dynamic_type_information(lowir::Value object, RuntimeFunction failure, bool test_null)
{
	// A null address fails (when the operand can be null); otherwise the
	// table's type information pointer.
	const lowir::Type pointer = lowir::Type::scalar(lowir::TypeKind::Ptr);
	if (test_null)
	{
		RValue address;
		address.value = object;
		address.type = pointer;
		const lowir::BlockId fail = new_block(lowir::BlockKind::TypeidFail);
		const lowir::BlockId scan = new_block(lowir::BlockKind::TypeidScan);
		branch(compare(lowir::ComparePredicate::Eq, pointer, address, integer(pointer, 0)), fail, scan);
		place(fail);
		runtime_failure(failure);
		place(scan);
	}
	Place vptr = Place();
	vptr.storage = object;
	const RValue table = load(pointer, vptr);
	Place slot = Place();
	slot.storage = index(lowir::Type::scalar(lowir::TypeKind::I8), 0, table.value,
		integer(lowir::Type::scalar(lowir::TypeKind::I64), static_cast<uint64_t>(-8)))
					   .value;
	return load(pointer, slot).value;
}

void FunctionLowering::runtime_failure(RuntimeFunction failure)
{
	// The runtime's bad_typeid or bad_cast throw, which does not return:
	// the course ends the block with the function's (zero) return.
	++lowering_.counters().instructions;
	builder_.call(lowir::Type::scalar(lowir::TypeKind::Void), lowir::Value::symbol(lowering_.runtime_function(failure)),
		nullptr, 0, lowir::kNoId, 0);
	zero_return();
}

Place FunctionLowering::typeid_place(const syntax::TypeTraitExpr& expr)
{
	// 5.2.8p2: an evaluated operand's object's dynamic type; else the
	// static type's object. The address is tested for null only when the
	// operand applies unary * to a pointer, the one form p2 makes throw
	// bad_typeid; a reference or a named object cannot be null.
	Place place = Place();
	if ((expr.facts->flags & sema::kDynamicType) == 0)
	{
		place.storage = lowir::Value::symbol(lowering_.type_information(expr.facts->target));
		return place;
	}
	const syntax::Expr* operand = WithoutParentheses(expr.operand);
	const bool through_pointer = operand->kind == syntax::ExprKind::Unary &&
		static_cast<const syntax::UnaryExpr*>(operand)->op == TokenKind::OP_STAR &&
		(operand->facts == nullptr || (operand->facts->flags & sema::kOperatorCall) == 0);
	const lowir::Value object = address(*expr.operand);
	if (!lowering_.host_objects())
		lowering_.class_table(sema::ClassOf(operand->facts->type));
	place.storage = dynamic_type_information(object, RuntimeFunction::BadTypeid, through_pointer);
	return place;
}

RValue FunctionLowering::type_identity(const syntax::BinaryExpr& expr)
{
	// The course's std::type_info equality: the same object.
	const lowir::Type pointer = lowir::Type::scalar(lowir::TypeKind::Ptr);
	RValue left;
	left.value = address(*expr.left);
	left.type = pointer;
	RValue right;
	right.value = address(*expr.right);
	right.type = pointer;
	return compare(
		expr.op == TokenKind::OP_EQ ? lowir::ComparePredicate::Eq : lowir::ComparePredicate::Ne, pointer, left, right);
}

lowir::Value FunctionLowering::runtime_cast(
	lowir::Value object, const sema::ClassEntity* from, const sema::ClassEntity* to, bool reference, uint32_t location)
{
	// 5.2.7p8 through the runtime: null stays null (a pointer's), else the
	// search's result, kept in the cast's slot; a reference's failed search
	// calls the runtime's bad_cast (p9).
	const lowir::Type pointer = lowir::Type::scalar(lowir::TypeKind::Ptr);
	Place result = Place();
	result.storage = lowir::Value::slot(generated_slot(lowir::SlotKind::DynCast, pointer));
	store(pointer, integer(pointer, 0), result);
	RValue address;
	address.value = object;
	address.type = pointer;
	const lowir::BlockId scan = new_block(lowir::BlockKind::DynCastScan);
	const lowir::BlockId end = new_block(lowir::BlockKind::DynCastEnd);
	branch(compare(lowir::ComparePredicate::Eq, pointer, address, integer(pointer, 0)), end, scan);
	place(scan);
	lowir::Value arguments[4];
	arguments[0] = object;
	arguments[1] = address_of_symbol(lowering_.class_information(from, true));
	arguments[2] = address_of_symbol(lowering_.class_information(to, true));
	arguments[3] = integer(lowir::Type::scalar(lowir::TypeKind::I64),
		static_cast<uint64_t>(lowering_.dynamic_cast_hint(from, to, location)))
					   .value;
	++lowering_.counters().instructions;
	RValue found;
	found.value = lowir::Value::temp(builder_.call(pointer,
		lowir::Value::symbol(lowering_.runtime_function(RuntimeFunction::DynamicCast)), arguments, 4, lowir::kNoId, 0));
	found.type = pointer;
	store(pointer, found, result);
	if (reference)
	{
		const lowir::BlockId fail = new_block(lowir::BlockKind::DynCastFail);
		const lowir::BlockId success = new_block(lowir::BlockKind::DynCastFound);
		branch(compare(lowir::ComparePredicate::Eq, pointer, found, integer(pointer, 0)), fail, success);
		place(fail);
		runtime_failure(RuntimeFunction::BadCast);
		place(success);
		jump(end);
		// The course's unreachable block after the success path.
		place(new_block(lowir::BlockKind::Block));
	}
	jump(end);
	place(end);
	return load(pointer, result).value;
}

lowir::Value FunctionLowering::address_of_symbol(lowir::SymbolId symbol)
{
	Place place = Place();
	place.storage = lowir::Value::symbol(symbol);
	return address_of(place);
}

lowir::Value FunctionLowering::most_derived_object(lowir::Value object)
{
	// 5.2.7p7: null stays null, else the object's address moved by its
	// table's offset-to-top (Itanium 2.5.2), kept in the cast's slot.
	// This operation needs no RTTI search or void type-information object.
	const lowir::Type pointer = lowir::Type::scalar(lowir::TypeKind::Ptr);
	const lowir::Type i64 = lowir::Type::scalar(lowir::TypeKind::I64);
	Place result = Place();
	result.storage = lowir::Value::slot(generated_slot(lowir::SlotKind::DynCast, pointer));
	store(pointer, integer(pointer, 0), result);
	RValue address;
	address.value = object;
	address.type = pointer;
	const lowir::BlockId scan = new_block(lowir::BlockKind::DynCastScan);
	const lowir::BlockId end = new_block(lowir::BlockKind::DynCastEnd);
	branch(compare(lowir::ComparePredicate::Eq, pointer, address, integer(pointer, 0)), end, scan);
	place(scan);
	Place vptr = Place();
	vptr.storage = object;
	Place top = Place();
	top.storage = index(lowir::Type::scalar(lowir::TypeKind::I8), 0, load(pointer, vptr).value,
		integer(i64, static_cast<uint64_t>(-16)))
					  .value;
	store(pointer, index(lowir::Type::scalar(lowir::TypeKind::I8), 0, object, load(i64, top)), result);
	jump(end);
	place(end);
	return load(pointer, result).value;
}

RValue FunctionLowering::dynamic_cast_pointer(const syntax::CastExpr& expr)
{
	RValue value;
	value.type = lowir::Type::scalar(lowir::TypeKind::Ptr);
	if (sema::IsVoid(expr.facts->type->element))
	{
		value.value = most_derived_object(operand(*expr.operand, ConversionContext::Cast).value);
		return value;
	}
	const sema::ClassEntity* from = sema::ClassOf(WithoutParentheses(expr.operand)->facts->type->unqualified->element);
	const sema::ClassEntity* to = sema::ClassOf(expr.facts->type->element);
	value.value = runtime_cast(operand(*expr.operand, ConversionContext::Cast).value, from, to, false, expr.location);
	return value;
}

Place FunctionLowering::dynamic_cast_reference(const syntax::CastExpr& expr)
{
	const sema::ClassEntity* from = sema::ClassOf(WithoutParentheses(expr.operand)->facts->type);
	const sema::ClassEntity* to = sema::ClassOf(expr.facts->type);
	Place place = Place();
	place.storage = runtime_cast(address(*expr.operand), from, to, true, expr.location);
	return place;
}

} // namespace lower
