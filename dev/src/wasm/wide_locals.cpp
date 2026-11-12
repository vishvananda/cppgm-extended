#include "wasm/selector.h"

namespace wasm {

using namespace lowir;

void Selector::plan_wide_locals()
{
	wide_locals_.assign(body_.temps.size(), kNoId);
	std::vector<uint8_t> memory(body_.temps.size(), 0);
	for (BlockId block : body_.layout)
		for (InstId i = body_.blocks[block].first; i != kNoId; i = body_.instructions[i].next)
		{
			const auto& inst = body_.instructions[i];
			bool scalar = false;
			switch (inst.op)
			{
			case Opcode::Const:
			case Opcode::Copy:
			case Opcode::Binary:
			case Opcode::Unary:
			case Opcode::Compare:
				scalar = inst.type.kind() == TypeKind::I128;
				break;
			case Opcode::Convert:
			case Opcode::Phi:
			case Opcode::Return:
			case Opcode::Branch:
			case Opcode::Store:
				scalar = true;
				break;
			case Opcode::Call:
			{
				if (inst.result != kNoId)
					memory[inst.result] = 1; // hidden result storage is part of the ABI
				const auto* signature = program_.call_boundary(inst);
				if (signature != nullptr)
					for (uint32_t p = 0; p < inst.extra_count; ++p)
					{
						const Value v = body_.operands[inst.extra_first + p];
						if (v.is(ValueKind::Temp) && p < signature->parameter_count &&
							program_.parameters_of(*signature)[p].type.kind() != TypeKind::I128)
							memory[v.index] = 1;
					}
				scalar = signature != nullptr;
				break;
			}
			default:
				break;
			}
			if (!scalar)
			{
				if (inst.result != kNoId)
					memory[inst.result] = 1;
				opt::for_each_operand(body_, inst, [&](Value v) {
					if (v.is(ValueKind::Temp))
						memory[v.index] = 1;
				});
			}
		}
	for (uint32_t i = 0; i < body_.temps.size(); ++i)
		if (body_.temps[i].type.kind() == TypeKind::I128 && !memory[i])
			wide_locals_[i] = 0;
}

} // namespace wasm
