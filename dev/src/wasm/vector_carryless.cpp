#include "wasm/selector.h"

namespace wasm {

using namespace lowir;

void Selector::vector_carryless(const lowir::Instruction& inst)
{
	const unsigned control = vector_immediate(inst);
	const uint32_t a = add_local(ValueType::I64), b = add_local(ValueType::I64);
	const uint32_t low = add_local(ValueType::I64), high = add_local(ValueType::I64), shift = add_local(ValueType::I64);
	for (unsigned i = 0; i < 2; ++i)
	{
		value(inst.ops[i], Type::scalar(TypeKind::V128));
		out_.emit(Op::I64x2ExtractLane, { (control >> (4 * i)) & 1 });
		out_.emit(Op::LocalSet, { i == 0 ? a : b });
	}
	out_.emit(Op::I64Const, { 0 });
	out_.emit(Op::LocalSet, { low });
	out_.emit(Op::I64Const, { 0 });
	out_.emit(Op::LocalSet, { high });
	out_.emit(Op::Block, { block_type(ValueType::Void) });
	out_.emit(Op::Loop, { block_type(ValueType::Void) });
	nesting_ += 2;
	out_.emit(Op::LocalGet, { b });
	out_.emit(Op::I64Eqz);
	out_.emit(Op::BrIf, { 1 });
	out_.emit(Op::LocalGet, { b });
	out_.emit(Op::I64Ctz);
	out_.emit(Op::LocalTee, { shift });
	out_.emit(Op::I64Eqz);
	out_.emit(Op::I32Eqz);
	out_.emit(Op::If, { block_type(ValueType::Void) });
	++nesting_;
	out_.emit(Op::LocalGet, { high });
	out_.emit(Op::LocalGet, { a });
	out_.emit(Op::I64Const, { 64 });
	out_.emit(Op::LocalGet, { shift });
	out_.emit(Op::I64Sub);
	out_.emit(Op::I64ShrU);
	out_.emit(Op::I64Xor);
	out_.emit(Op::LocalSet, { high });
	out_.emit(Op::End);
	--nesting_;
	out_.emit(Op::LocalGet, { low });
	out_.emit(Op::LocalGet, { a });
	out_.emit(Op::LocalGet, { shift });
	out_.emit(Op::I64Shl);
	out_.emit(Op::I64Xor);
	out_.emit(Op::LocalSet, { low });
	out_.emit(Op::LocalGet, { b });
	out_.emit(Op::LocalGet, { b });
	out_.emit(Op::I64Const, { 1 });
	out_.emit(Op::I64Sub);
	out_.emit(Op::I64And);
	out_.emit(Op::LocalSet, { b });
	out_.emit(Op::Br, { 0 });
	out_.emit(Op::End);
	out_.emit(Op::End);
	nesting_ -= 2;
	vector_constant(Literal{ 0, 0 });
	out_.emit(Op::LocalGet, { low });
	out_.emit(Op::I64x2ReplaceLane, { 0 });
	out_.emit(Op::LocalGet, { high });
	out_.emit(Op::I64x2ReplaceLane, { 1 });
}

} // namespace wasm
