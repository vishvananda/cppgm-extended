#include "native/select/function_selector.h"

#include "lowir/assembly.h"
#include "native/backend_error.h"

namespace native {
namespace {

struct Capture
{
	int32_t address = 0;
	int32_t value = 0;
};

bool floating(lowir::Type type)
{
	return type.is_float() || type.is_vector();
}

} // namespace

const std::shared_ptr<const AssemblyPlan>& FunctionSelector::assembly_plan(const lowir::Instruction& inst) const
{
	const uint32_t index = static_cast<uint32_t>(&inst - body_->instructions.data());
	const uint64_t key = (uint64_t(symbol_) << 32) | index;
	std::shared_ptr<const AssemblyPlan>& plan = assembly_plans_[key];
	if (!plan)
	{
		std::vector<lowir::Value> arguments(inst.extra_count);
		for (uint32_t i = 0; i < inst.extra_count; ++i)
			arguments[i] = resolve(body_->operands[inst.extra_first + i]);
		plan = bind_assembly(*program_.assembly, static_cast<uint32_t>(inst.immediate), *body_, arguments);
	}
	return plan;
}

void FunctionSelector::emit_assembly(const lowir::Instruction& inst)
{
	using namespace asm_model;
	using lowir::TypeKind;
	const std::shared_ptr<const AssemblyPlan>& plan = assembly_plan(inst);
	const lowir::AssemblyModule& module = *program_.assembly;
	const lowir::AssemblyTemplate& definition = module.templates[plan->definition];
	std::vector<Capture> captures(definition.operand_count);
	uint32_t initialized = 0;
	Operand initial[32];
	moves_.clear();
	for (uint32_t i = 0; i < definition.operand_count; ++i)
	{
		const lowir::AssemblyOperand& operand = module.operands[definition.first_operand + i];
		const bool memory = plan->locations[i] == AL_MEMORY;
		const bool immediate = plan->locations[i] == AL_IMMEDIATE;
		if (!immediate && (operand.input != lowir::kNoId || memory))
		{
			const uint32_t input = memory && !operand.input_storage ? operand.output : operand.input;
			const lowir::Value value = resolve(body_->operands[inst.extra_first + input]);
			const lowir::Type input_type = memory || operand.input_storage ? lowir::Type::scalar(TypeKind::Ptr)
																		   : operand.type;
			Operand source = memory_value_operand(value, input_type);
			if (operand.input_storage && !memory)
			{
				// Capture a register alternative's value before setting up any
				// assembly registers. The address expression has already run.
				load_into(R10, source, TypeKind::Ptr);
				const Reg scratch = floating(operand.type) ? XMM15 : R11;
				Inst& load = emit2(floating(operand.type) ? Op::FMov : Op::Load, operand.type.kind(),
					Operand::make_reg(scratch), Operand::make_mem(R10, 0));
				if ((operand.flags & AO_VOLATILE) != 0)
					load.flags |= MF_VOLATILE;
				source = Operand::make_frame(
					frame_alloc(operand.type.is_vector() ? 16 : register_bytes(operand.type.kind()),
						lowir::data_alignment(operand.type)));
				emit2(floating(operand.type) ? Op::FMov : Op::Store, operand.type.kind(), source,
					Operand::make_reg(scratch));
			}
			const Reg reg = plan->registers[i];
			if ((initialized & reg_bit(reg)) != 0 && !(initial[reg] == source))
				throw BackendError("assembly matching operands supply distinct input values to one register");
			if ((initialized & reg_bit(reg)) == 0)
			{
				Move move;
				move.dst = Operand::make_reg(reg);
				move.src = source;
				move.type = memory ? TypeKind::Ptr : operand.type.kind();
				moves_.push_back(move);
				initial[reg] = source;
				initialized |= reg_bit(reg);
			}
		}
		if (operand.output != lowir::kNoId && !memory)
		{
			// Addresses are captured before any assembly writes. Their SSA
			// registers therefore have ordinary input lifetimes; neither a
			// clobber nor an overlapping output can destroy a later address.
			Capture& capture = captures[i];
			capture.address = frame_alloc(8, 8);
			capture.value = frame_alloc(operand.type.is_vector() ? 16 : register_bytes(operand.type.kind()),
				operand.type.is_vector() ? 16 : lowir::data_alignment(operand.type));
			Move move;
			move.dst = Operand::make_frame(capture.address);
			const lowir::Value address = resolve(body_->operands[inst.extra_first + operand.output]);
			move.src = memory_value_operand(address, lowir::Type::scalar(TypeKind::Ptr));
			move.type = TypeKind::Ptr;
			moves_.push_back(move);
		}
	}
	parallel_moves(moves_);
	if (!out_->assembly)
		out_->assembly.reset(new AssemblySites());
	const uint32_t site = static_cast<uint32_t>(out_->assembly->plans.size());
	out_->assembly->plans.push_back(plan);
	Inst& effect = emit1(Op::Asm, TypeKind::Void, Operand::make_imm(site));
	effect.aux = plan->reads;
	effect.aux2 = plan->writes;
	effect.sub = static_cast<uint8_t>(((definition.state_read & AS_CONDITION_FLAGS) != 0 ? 1 : 0) |
		((definition.state_write & AS_CONDITION_FLAGS) != 0 ? 2 : 0));
	written_ |= plan->writes;
	used_callee_saved_ |= plan->writes & kCalleeSavedGprs;
	// First capture every physical result without changing flags or using
	// scratch registers. Only then may flag capture or output addressing
	// reuse a register that held another result.
	for (uint32_t i = 0; i < definition.operand_count; ++i)
	{
		const lowir::AssemblyOperand& operand = module.operands[definition.first_operand + i];
		if (operand.output != lowir::kNoId && (operand.flags & AO_FLAG) == 0 && plan->locations[i] != AL_MEMORY)
			emit2(floating(operand.type) ? Op::FMov : Op::Store, operand.type.kind(),
				Operand::make_frame(captures[i].value), Operand::make_reg(plan->registers[i]));
	}
	for (uint32_t i = 0; i < definition.operand_count; ++i)
	{
		const lowir::AssemblyOperand& operand = module.operands[definition.first_operand + i];
		if (operand.output == lowir::kNoId || (operand.flags & AO_FLAG) == 0)
			continue;
		emit1(Op::Setcc, TypeKind::I64, Operand::make_reg(R11)).sub = operand.condition;
		emit2(Op::Movzx, TypeKind::I64, Operand::make_reg(R11), Operand::make_reg(R11));
		emit2(Op::Store, operand.type.kind(), Operand::make_frame(captures[i].value), Operand::make_reg(R11));
	}
	// The frontend's operand order determines assignment order. Aliased
	// places see the final assignment, after every result was captured.
	for (uint32_t i = 0; i < definition.operand_count; ++i)
	{
		const lowir::AssemblyOperand& operand = module.operands[definition.first_operand + i];
		if (operand.output == lowir::kNoId || plan->locations[i] == AL_MEMORY)
			continue;
		load_into(R10, Operand::make_frame(captures[i].address), TypeKind::Ptr);
		const Reg value = floating(operand.type) ? XMM15 : R11;
		load_into(value, Operand::make_frame(captures[i].value), operand.type.kind());
		Inst& store = emit2(floating(operand.type) ? Op::FMov : Op::Store, operand.type.kind(),
			Operand::make_mem(R10, 0), Operand::make_reg(value));
		if ((operand.flags & AO_VOLATILE) != 0)
			store.flags |= MF_VOLATILE;
	}
	for (uint32_t mask = plan->writes; mask != 0; mask &= mask - 1)
		release(static_cast<Reg>(__builtin_ctz(mask)));
}

} // namespace native
