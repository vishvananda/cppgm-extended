#include "native/select/mir_cleanup.h"

#include "native/select/block_placement.h"

namespace native {

namespace {

// The selector reserves r10/r11 for instruction-local work. After the
// dividend has reached rax, one may test its upper half with the divisor's
// without disturbing either operand. Keep full-address scratch dependencies.
Reg divide_scratch(const Inst& inst)
{
	if ((inst.op != Op::Div && inst.op != Op::Idiv) || inst.type != lowir::TypeKind::I64 || inst.nops != 1)
		return kNoReg;
	const Operand& divisor = inst.ops[0];
	if (divisor.kind != OperandKind::Reg && divisor.kind != OperandKind::Mem)
		return kNoReg;
	for (Reg scratch : { R10, R11 })
		if (!divisor.uses_reg(scratch))
			return scratch;
	return kNoReg;
}

Inst operation(Op op, lowir::TypeKind type, Operand a, Operand b = Operand())
{
	Inst inst;
	inst.op = op;
	inst.type = type;
	inst.nops = b.kind == OperandKind::None ? 1 : 2;
	inst.ops[0] = a;
	inst.ops[1] = b;
	return inst;
}

} // namespace

// A 32-bit unsigned divide gives the full 64-bit result when both operands
// fit, including a zero divisor's hardware trap. The cold path retains the
// original operation for every other input. Unlike an IR narrowing guess,
// this guard never discards high operand bits. A signed divide takes the
// same path: operands that fit are not negative, so the quotient and
// remainder are the unsigned ones, and the cqo before it has cleared rdx.
void narrow_variable_divides(Function& function)
{
	bool candidate = false;
	for (const Inst& inst : function.insts)
		candidate = candidate || divide_scratch(inst) != kNoReg;
	if (!candidate)
		return;
	std::vector<Inst> original;
	original.swap(function.insts);
	const uint32_t count = static_cast<uint32_t>(function.blocks.size());
	std::vector<uint32_t> order;
	order.reserve(count);
	function.insts.reserve(original.size());
	for (uint32_t b = 0; b < count; ++b)
	{
		const Block block = function.blocks[b];
		uint32_t current = b;
		function.blocks[current].first = static_cast<uint32_t>(function.insts.size());
		order.push_back(current);
		for (uint32_t i = block.first; i < block.first + block.count; ++i)
		{
			const Inst divide = original[i];
			const Reg scratch = divide_scratch(divide);
			if (scratch == kNoReg)
			{
				function.insts.push_back(divide);
				continue;
			}
			const uint32_t fast = static_cast<uint32_t>(function.blocks.size()), slow = fast + 1, join = fast + 2;
			Block added = block;
			added.name = added.edge_from = added.edge_to = 0;
			added.landing = added.fallen_into = false;
			function.blocks.insert(function.blocks.end(), 3, added);
			const auto emit = [&](Inst inst) {
				inst.location = divide.location;
				function.insts.push_back(inst);
			};
			const auto begin = [&](uint32_t next) {
				function.blocks[current].count = static_cast<uint32_t>(function.insts.size()) -
					function.blocks[current].first;
				current = next;
				function.blocks[current].first = static_cast<uint32_t>(function.insts.size());
				order.push_back(current);
			};
			const Operand work = Operand::make_reg(scratch);
			emit(operation(Op::Mov, lowir::TypeKind::I64, work, Operand::make_reg(RAX)));
			emit(operation(Op::Or, lowir::TypeKind::I64, work, divide.ops[0]));
			emit(operation(Op::Shr, lowir::TypeKind::I64, work, Operand::make_imm(32)));
			Inst branch = operation(Op::Jcc, lowir::TypeKind::Void, Operand::make_label(slow));
			branch.sub = CC_NE;
			emit(branch);
			emit(operation(Op::Jmp, lowir::TypeKind::Void, Operand::make_label(fast)));
			begin(fast);
			Inst word = divide;
			word.op = Op::Div;
			word.type = lowir::TypeKind::I32;
			emit(word);
			emit(operation(Op::Jmp, lowir::TypeKind::Void, Operand::make_label(join)));
			begin(slow);
			emit(divide);
			emit(operation(Op::Jmp, lowir::TypeKind::Void, Operand::make_label(join)));
			begin(join);
		}
		function.blocks[current].count = static_cast<uint32_t>(function.insts.size()) - function.blocks[current].first;
	}
	// This shared remapper includes jump tables, addressed entries and EH
	// landing/select labels. Existing block identities still name their heads.
	reorder_blocks(function, order);
	function.placement_valid = false;
}

} // namespace native
