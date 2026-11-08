// Retain immediate vector operations through source-required inlining. The
// ordinary LowIR helper also supplies the existing runtime-selector semantics.
#include "lower/source_inlining.h"

#include "lower/program_lowering.h"
#include "opt/passes.h"
#include "opt/scalar_fold.h"

namespace lower {

lowir::SymbolId SourceInlining::vector_function(lowir::VectorOp operation) const
{
	for (const auto& entry : vectors_)
		if (entry.second == operation)
			return entry.first;
	return lowir::kNoId;
}

lowir::SymbolId SourceInlining::add_vector_function(lowir::SymbolId symbol, lowir::VectorOp operation)
{
	lowir::Signature signature = lowir::Signature();
	signature.result = lowir::Type::scalar(lowir::TypeKind::V128);
	signature.first_parameter = static_cast<uint32_t>(program_.parameters.size());
	signature.parameter_count = 3;
	signature.boundary.effects = ir_model::CFXM_READNONE;
	signature.boundary.unwind = ir_model::CUM_NO;
	for (uint32_t i = 0; i < 3; ++i)
	{
		lowir::Parameter parameter = lowir::Parameter();
		const char* name = i == 0 ? "first" : i == 1 ? "second" : "selector";
		parameter.name = program_.names.intern(name, i == 0 ? 5 : i == 1 ? 6 : 8);
		parameter.type = lowir::Type::scalar(i < 2 ? lowir::TypeKind::V128 : lowir::TypeKind::I32);
		program_.parameters.push_back(parameter);
	}
	program_.declare_function(symbol, lowir::SymbolKind::FunctionDeclaration, program_.add_signature(signature));
	program_.symbols[symbol].metadata.binding = ir_model::SBM_INTERNAL;
	vectors_.push_back(std::make_pair(symbol, operation));
	return symbol;
}

bool SourceInlining::vector_call(const lowir::Instruction& instruction, lowir::VectorOp* operation) const
{
	if (instruction.op != lowir::Opcode::Call || !instruction.ops[0].is(lowir::ValueKind::Symbol))
		return false;
	for (const auto& entry : vectors_)
		if (entry.first == instruction.ops[0].index)
		{
			if (operation != nullptr)
				*operation = entry.second;
			return true;
		}
	return false;
}

void SourceInlining::select_vectors()
{
	for (const lowir::SymbolId symbol : deferred_)
	{
		lowir::FunctionBody& original = *program_.functions[program_.symbols[symbol].detail].body;
		bool pending = false;
		for (const lowir::BlockId block : original.layout)
			for (lowir::InstId i = original.blocks[block].first; i != lowir::kNoId; i = original.instructions[i].next)
				pending = pending || vector_call(original.instructions[i]);
		if (!pending)
			continue;
		// Prove selector values without changing named O0/debug storage. IDs of
		// surviving calls are stable across these scalar simplifications.
		lowir::FunctionBody proof = original;
		opt::OptimizerStats stats;
		opt::FunctionContext context(program_, symbol, proof, stats, 0);
		opt::promote_slots(context);
		opt::propagate_constants(context);
		opt::simplify_values(context);
		for (const lowir::BlockId block : original.layout)
			for (lowir::InstId i = original.blocks[block].first; i != lowir::kNoId; i = original.instructions[i].next)
			{
				lowir::Instruction& instruction = original.instructions[i];
				lowir::VectorOp operation;
				const lowir::Instruction& known = proof.instructions[i];
				if (!vector_call(instruction, &operation) || !vector_call(known))
					continue;
				lowir::Constant selector;
				if (!opt::literal_constant(proof, proof.operands[known.extra_first + 2], selector) ||
					selector.kind != lowir::ValueKind::Integer)
					continue;
				const uint64_t count = selector.bits.low;
				if (operation >= lowir::VectorOp::Dpps && operation <= lowir::VectorOp::Roundsd &&
					(count > 255 || selector.bits.high != 0))
					throw LoweringError(0, "a vector selector outside the unsigned-byte range after substitution");
				const bool zero = (operation == lowir::VectorOp::Palignr && count >= 32) ||
					((operation == lowir::VectorOp::Pslldq || operation == lowir::VectorOp::Psrldq) && count >= 16);
				const lowir::Value first = original.operands[instruction.extra_first];
				const lowir::Value second = original.operands[instruction.extra_first + 1];
				instruction.op = lowir::Opcode::Vector;
				instruction.sub = static_cast<uint16_t>(zero ? lowir::VectorOp::Pzero : operation);
				instruction.ops[0] = zero ? lowir::Value::none() : first;
				instruction.ops[1] = !zero && lowir::shape_of(operation) == lowir::VectorShape::BinaryImmediate
					? second
					: lowir::Value::none();
				instruction.ops[2] = zero ? lowir::Value::none()
										  : opt::integer_value(original, lowir::Literal{ count & 255, 0 });
				instruction.immediate = 0;
				instruction.extra_first = instruction.extra_count = instruction.flags = 0;
			}
	}
}

lowir::SymbolId ProgramLowering::source_vector_function(lowir::VectorOp operation)
{
	if (!source_inlining_)
		source_inlining_.reset(new SourceInlining(program_));
	const lowir::SymbolId known = source_inlining_->vector_function(operation);
	if (known != lowir::kNoId)
		return known;
	const std::string name = std::string("__cppgm_vector_") + lowir::spelling(operation);
	return source_inlining_->add_vector_function(new_symbol(name.c_str()), operation);
}

} // namespace lower
