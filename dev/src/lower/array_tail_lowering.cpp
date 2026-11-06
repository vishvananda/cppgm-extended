#include "lower/function_lowering.h"

#include "sema/aggregates.h"

namespace lower {

namespace {

// A bulk zero must not replace an observable scalar access, a reference
// binding, or a constructor. Walk the type, never the array's extent.
bool PlainZeroStorage(const sema::Type* type, unsigned depth)
{
	if (depth > 64 || type->is_reference() || (type->cv & (sema::kVolatile | sema::kAtomic)) != 0)
		return false;
	if (type->kind == sema::TypeKind::Array)
		return PlainZeroStorage(type->element, depth + 1);
	const sema::ClassEntity* cls = sema::ClassOf(type);
	if (cls == nullptr)
		return true;
	for (const sema::BaseClass* base = cls->bases; base != nullptr; base = base->next)
	{
		if (!PlainZeroStorage(base->base->type, depth + 1))
			return false;
	}
	for (const sema::DataMember* member = cls->members; member != nullptr; member = member->next)
	{
		if (!PlainZeroStorage(member->variable->type, depth + 1))
			return false;
	}
	return true;
}

uint64_t LeafCount(const sema::Type* type)
{
	uint64_t count = 1;
	while (type->kind == sema::TypeKind::Array)
	{
		count *= type->bound;
		type = type->element;
	}
	return count;
}

} // namespace

void FunctionLowering::elements_made(ElementCount& count, uint64_t amount)
{
	const lowir::Type i64 = lowir::Type::scalar(lowir::TypeKind::I64);
	Place slot = Place();
	slot.storage = lowir::Value::slot(count.slot);
	if (count.repeated)
		store(i64, binary(lowir::BinaryOp::Add, i64, load(i64, slot), integer(i64, amount)), slot);
	else
	{
		count.made += amount;
		if (count.made < count.total)
			store(i64, integer(i64, count.made), slot);
	}
}

void FunctionLowering::omitted_array_tail(const sema::Type* type, lowir::Value base, uint64_t first, uint32_t location)
{
	// The explicit prefix has finished. A zero span or one repeated typed
	// initialization handles its omitted suffix, including nested arrays.
	const sema::Type* element = type->element;
	const uint64_t count = type->bound - first;
	const uint64_t size = lowering_.size_of(element, location);
	const lowir::Value address = structured_element(base, element, first, location, false);
	const sema::Type* tail = lowering_.graph().types.array(element, count);
	ElementCount local = ElementCount();
	ElementCount* const saved_count = element_count_;
	const size_t saved_members = aggregate_members_;
	const bool saved_regions = member_regions_;
	size_t counted = SIZE_MAX;
	if (element_count_ == nullptr && aggregate_members_ != SIZE_MAX && !static_initializer_ && needs_destruction(tail))
	{
		// A nonthrowing tail can still precede a throwing member. Represent
		// its completed objects once, rather than retaining one loop address.
		counted = push_counted(tail, address, local);
		element_count_ = &local;
		aggregate_members_ = SIZE_MAX;
	}
	if (lowering_.zero_value_initialized(element, 0) && PlainZeroStorage(element, 0))
	{
		// A packed enclosing member may supply only byte alignment. Null
		// data-member pointers are patched through the existing ABI walker.
		zero_bytes(address, count * size, 1);
		null_data_member_pointers(tail, address, 0, true);
		if (element_count_ != nullptr)
			elements_made(*element_count_, LeafCount(tail));
	}
	else
		omitted_array_loop(tail, address, location);
	if (counted != SIZE_MAX)
	{
		element_count_ = saved_count;
		aggregate_members_ = saved_members;
		member_regions_ = saved_regions;
		end_counted(counted);
		cleanups_.push_back(Cleanup{ address, tail, nullptr, lowir::kNoId, lowir::kNoId, lowir::kNoId, false });
		cleanups_.back().member = true;
	}
}

void FunctionLowering::omitted_array_loop(const sema::Type* type, lowir::Value base, uint32_t location)
{
	ElementCount* const count = element_count_;
	const uint64_t made = count != nullptr ? count->made : 0;
	const bool repeated = count != nullptr && count->repeated;
	if (count != nullptr)
		count->repeated = true;
	const lowir::Type i64 = lowir::Type::scalar(lowir::TypeKind::I64);
	Place cursor = Place();
	cursor.storage = lowir::Value::slot(generated_slot(lowir::SlotKind::ConstructorArrayIndex, i64));
	store(i64, integer(i64, 0), cursor);
	close_region();
	const lowir::BlockId condition = new_block(lowir::BlockKind::ConstructorArrayCond);
	const lowir::BlockId body = new_block(lowir::BlockKind::ConstructorArrayBody);
	const lowir::BlockId end = new_block(lowir::BlockKind::ConstructorArrayEnd);
	jump(condition);
	place(condition);
	const RValue current = load(i64, cursor);
	branch(compare(lowir::ComparePredicate::Ult, i64, current, integer(i64, type->bound)), body, end);
	place(body);
	const RValue offset = binary(
		lowir::BinaryOp::Mul, i64, current, integer(i64, lowering_.size_of(type->element, location)));
	const lowir::Value address =
		index(lowir::Type::scalar(lowir::TypeKind::I8), ir_model::IPK_ARRAY_ELEMENT, base, offset).value;
	uint32_t none = 0;
	structured_item(type->element, address, nullptr, none, location, false, true);
	close_region();
	store(i64, binary(lowir::BinaryOp::Add, i64, current, integer(i64, 1)), cursor);
	jump(condition);
	place(end);
	if (count != nullptr)
	{
		count->repeated = repeated;
		count->made = made + (repeated ? 0 : LeafCount(type));
	}
}

} // namespace lower
