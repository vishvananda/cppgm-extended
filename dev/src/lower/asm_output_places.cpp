#include "lower/function_lowering.h"

#include "sema/expression_facts.h"

namespace lower {

RValue FunctionLowering::capture_asm_designator(const syntax::Expr& target, BitFieldUpdate& update)
{
	BitFieldUpdate inner = BitFieldUpdate();
	const syntax::Expr* operand = nullptr;
	if (target.kind == syntax::ExprKind::Assign)
	{
		const syntax::BinaryExpr& assignment = static_cast<const syntax::BinaryExpr&>(target);
		inner = assignment_update(assignment);
		operand = assignment.left;
		inner.type = operand->facts->type;
		update_operand(inner);
	}
	else
	{
		const syntax::UnaryExpr& step = static_cast<const syntax::UnaryExpr&>(target);
		inner.kind = BitFieldUpdate::Kind::Increment;
		inner.decrement = step.op == TokenKind::OP_DEC;
		inner.prefix = true;
		inner.location = target.location;
		operand = step.operand;
	}
	update.capture->pending.push_back(inner);
	const RValue value = update_bit_field(*operand, update);
	update.capture->pending.pop_back();
	return value;
}

RValue FunctionLowering::capture_asm_case(
	AsmOutputCapture& capture, const BitField* field, const Place& place, const sema::Type* type)
{
	for (size_t i = capture.pending.size(); i != 0; --i)
	{
		BitFieldUpdate& pending = capture.pending[i - 1];
		if (field != nullptr)
			apply_bit_field_update(*field, place, pending);
		else
			apply_object_update_at(type, place, pending);
	}
	AsmOutputCase target = AsmOutputCase();
	if (field != nullptr)
		target.field = *field;
	target.place = place;
	target.place.has_known = false;
	target.type = lowering_.storage_type(type->unqualified);
	target.bit_field = field != nullptr;
	Place pointer = Place();
	pointer.storage = capture.pointer_slot;
	store(program_.pointer_type(), RValue{ address_of(place), program_.pointer_type() }, pointer);
	Place selector = Place();
	selector.storage = capture.selector_slot;
	store(lowir::Type::scalar(lowir::TypeKind::I32),
		integer(lowir::Type::scalar(lowir::TypeKind::I32), capture.cases.size()), selector);
	capture.cases.push_back(target);
	if (!capture.read)
		return capture.empty;
	return field != nullptr ? read_bit_field(*field, target.place) : load(target.type, target.place);
}

void FunctionLowering::write_asm_case(const AsmOutputCase& target, lowir::Value address, const RValue& value)
{
	Place destination = target.place;
	destination.storage = address;
	if (target.bit_field)
		write_bit_field(
			target.field, destination, to_unit(target.field, value, target.field.variable->type->unqualified));
	else
		store(target.type, value, destination);
}

void FunctionLowering::finish_asm_output(const AsmOutputCapture& capture)
{
	Place result = Place();
	result.storage = capture.result_slot;
	const RValue value = load(capture.empty.type, result);
	Place pointer = Place();
	pointer.storage = capture.pointer_slot;
	const lowir::Value address = load(program_.pointer_type(), pointer).value;
	if (capture.cases.size() == 1)
	{
		write_asm_case(capture.cases[0], address, value);
		return;
	}
	Place selector = Place();
	selector.storage = capture.selector_slot;
	const RValue selected = load(lowir::Type::scalar(lowir::TypeKind::I32), selector);
	const lowir::BlockId end = new_block(lowir::BlockKind::CondEnd);
	std::vector<lowir::BlockId> blocks;
	std::vector<lowir::Value> cases;
	for (size_t i = 0; i < capture.cases.size(); ++i)
	{
		blocks.push_back(new_block(lowir::BlockKind::SwitchCase));
		cases.push_back(integer(selected.type, i).value);
		cases.push_back(lowir::Value::block(blocks.back()));
	}
	lowir::Instruction dispatch = lowir::make_instruction(
		lowir::Opcode::Switch, lowir::Type::scalar(lowir::TypeKind::Void));
	dispatch.ops[0] = selected.value;
	dispatch.ops[1] = lowir::Value::block(blocks[0]);
	dispatch.extra_first = builder_.add_operands(cases.data(), static_cast<uint32_t>(cases.size()));
	dispatch.extra_count = static_cast<uint32_t>(cases.size());
	terminate(dispatch);
	for (size_t i = 0; i < blocks.size(); ++i)
	{
		place(blocks[i]);
		write_asm_case(capture.cases[i], address, value);
		jump(end);
	}
	place(end);
}

} // namespace lower
