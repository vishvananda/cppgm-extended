#include "wasm/mir.h"

namespace wasm {

namespace {

bool local_operation(Op op)
{
	return op == Op::LocalGet || op == Op::LocalSet || op == Op::LocalTee;
}

void count_reads(const Function& function, std::vector<uint32_t>& reads)
{
	reads.assign(function.locals.size(), 0);
	for (const Instruction& inst : function.instructions)
		if (inst.op == Op::LocalGet)
			++reads[function.operands[inst.operands.first]];
}

} // namespace

void cleanup_locals(Function& function, bool optimize)
{
	std::vector<uint32_t> uses;
	if (optimize)
	{
		count_reads(function, uses);
		size_t kept = 0;
		for (size_t i = 0; i < function.instructions.size(); ++i)
		{
			Instruction inst = function.instructions[i];
			if (inst.op == Op::LocalSet && i + 1 < function.instructions.size())
			{
				const Instruction& next = function.instructions[i + 1];
				const uint64_t id = function.operands[inst.operands.first];
				if (next.op == Op::LocalGet && function.operands[next.operands.first] == id)
				{
					++i;
					if (uses[id] == 1)
						continue;
					inst.op = Op::LocalTee;
				}
			}
			function.instructions[kept++] = inst;
		}
		function.instructions.resize(kept);
		count_reads(function, uses);
		kept = 0;
		for (Instruction inst : function.instructions)
		{
			if ((inst.op == Op::LocalSet || inst.op == Op::LocalTee) &&
				uses[function.operands[inst.operands.first]] == 0)
			{
				if (inst.op == Op::LocalTee)
					continue;
				inst.op = Op::Drop;
				inst.operands.count = 0;
			}
			function.instructions[kept++] = inst;
		}
		function.instructions.resize(kept);
	}
	// Parameters retain their public ABI indices, including unused parameters.
	// Ordinary locals need no allocation if no surviving instruction uses them.
	uses.assign(function.locals.size(), 0);
	for (const Instruction& inst : function.instructions)
		if (local_operation(inst.op))
			uses[function.operands[inst.operands.first]] = 1;
	uint32_t next = function.parameter_count;
	for (uint32_t i = 0; i < function.locals.size(); ++i)
		if (i < function.parameter_count)
			uses[i] = i;
		else if (uses[i] != 0)
		{
			uses[i] = next;
			function.locals[next++] = function.locals[i];
		}
	function.locals.resize(next);
	for (const Instruction& inst : function.instructions)
		if (local_operation(inst.op))
		{
			auto& operand = function.operands[inst.operands.first];
			operand = uses[operand];
		}
}

} // namespace wasm
