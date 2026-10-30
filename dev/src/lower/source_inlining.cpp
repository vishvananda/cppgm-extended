#include "lower/source_inlining.h"

#include <algorithm>

#include "lower/program_lowering.h"
#include "opt/inliner.h"
#include "opt/passes.h"
#include "sema/analyzer.h"

namespace lower {

void SourceInlining::mark(lowir::SymbolId symbol, lowir::SymbolId target)
{
	if (required_.find(symbol) == nullptr)
	{
		required_.insert(symbol, target);
		annotated_.push_back(symbol);
		aliases_ = aliases_ || symbol != target;
	}
}

bool SourceInlining::defer(lowir::SymbolId symbol)
{
	if (required_.find(symbol) != nullptr)
	{
		deferred_.push_back(symbol);
		return true;
	}
	const lowir::FunctionBody& body = *program_.functions[program_.symbols[symbol].detail].body;
	for (const lowir::BlockId block : body.layout)
		for (lowir::InstId i = body.blocks[block].first; i != lowir::kNoId; i = body.instructions[i].next)
		{
			const lowir::Instruction& inst = body.instructions[i];
			if (inst.op == lowir::Opcode::Call && inst.ops[0].is(lowir::ValueKind::Symbol) &&
				(required_.find(inst.ops[0].index) != nullptr || immediate_call(inst)))
			{
				deferred_.push_back(symbol);
				return true;
			}
		}
	return false;
}

void SourceInlining::run()
{
	std::vector<uint8_t> flags;
	flags.reserve(annotated_.size());
	for (const lowir::SymbolId symbol : annotated_)
	{
		flags.push_back(program_.symbols[symbol].metadata.flags);
		program_.symbols[symbol].metadata.flags |= lowir::SF_FORCE_INLINE;
	}
	// A nonvirtual destructor base entry aliases its complete entry. Use
	// that already-known body, rather than searching ABI spellings.
	if (aliases_)
		for (const lowir::SymbolId symbol : deferred_)
		{
			lowir::FunctionBody& body = *program_.functions[program_.symbols[symbol].detail].body;
			for (const lowir::BlockId block : body.layout)
				for (lowir::InstId i = body.blocks[block].first; i != lowir::kNoId; i = body.instructions[i].next)
				{
					lowir::Instruction& inst = body.instructions[i];
					if (inst.op == lowir::Opcode::Call && inst.ops[0].is(lowir::ValueKind::Symbol))
						if (const lowir::SymbolId* target = required_.find(inst.ops[0].index))
							inst.ops[0] = lowir::Value::symbol(*target);
				}
		}
	opt::inline_source_required(program_);
	lowir::SymbolId incomplete = lowir::kNoId;
	for (const lowir::SymbolId symbol : deferred_)
	{
		const lowir::FunctionBody& body = *program_.functions[program_.symbols[symbol].detail].body;
		for (const lowir::BlockId block : body.layout)
			for (lowir::InstId i = body.blocks[block].first; i != lowir::kNoId; i = body.instructions[i].next)
			{
				const lowir::Instruction& inst = body.instructions[i];
				if (inst.op == lowir::Opcode::Call && inst.ops[0].is(lowir::ValueKind::Symbol) &&
					(program_.symbols[inst.ops[0].index].metadata.flags & lowir::SF_FORCE_INLINE) != 0)
					incomplete = inst.ops[0].index;
			}
	}
	if (immediate_guard_ != lowir::kNoId)
		validate_immediates();
	for (size_t i = 0; i < annotated_.size(); ++i)
		program_.symbols[annotated_[i]].metadata.flags = flags[i];
	if (incomplete != lowir::kNoId)
		throw LoweringError(0,
			(std::string("an always_inline call of ") + program_.spelling(program_.symbols[incomplete].name) +
				" whose body cannot be substituted")
				.c_str());
}

lowir::SymbolId SourceInlining::immediate_guard(lowir::SymbolId symbol)
{
	if (immediate_guard_ != lowir::kNoId)
		return immediate_guard_;
	// A transient typed call keeps the requirement through inlining and dead
	// code handling. Its symbol id, never its spelling, identifies the check.
	immediate_guard_ = symbol;
	lowir::Signature signature = lowir::Signature();
	signature.first_parameter = static_cast<uint32_t>(program_.parameters.size());
	signature.parameter_count = 1;
	signature.result = lowir::Type::scalar(lowir::TypeKind::Void);
	signature.boundary.unwind = ir_model::CUM_NO;
	lowir::Parameter parameter = lowir::Parameter();
	parameter.type = lowir::Type::scalar(lowir::TypeKind::I32);
	program_.parameters.push_back(parameter);
	program_.declare_function(
		immediate_guard_, lowir::SymbolKind::FunctionDeclaration, program_.add_signature(signature));
	return immediate_guard_;
}

bool SourceInlining::immediate_call(const lowir::Instruction& inst) const
{
	return inst.op == lowir::Opcode::Call && inst.ops[0].is(lowir::ValueKind::Symbol) &&
		inst.ops[0].index == immediate_guard_;
}

void SourceInlining::remove_immediates(lowir::FunctionBody& body)
{
	for (const lowir::BlockId block : body.layout)
	{
		lowir::InstId previous = lowir::kNoId;
		for (lowir::InstId i = body.blocks[block].first; i != lowir::kNoId;)
		{
			const lowir::InstId next = body.instructions[i].next;
			if (immediate_call(body.instructions[i]))
			{
				if (previous == lowir::kNoId)
					body.blocks[block].first = next;
				else
					body.instructions[previous].next = next;
				if (body.blocks[block].last == i)
					body.blocks[block].last = previous;
			}
			else
				previous = i;
			i = next;
		}
	}
}

bool SourceInlining::direct_immediates(const lowir::FunctionBody& body) const
{
	// A single forward scan proves copies and same-block scalar locals. Epochs
	// invalidate memory facts in O(1); unknown forms keep the full analysis.
	std::vector<uint8_t> constants(body.temps.size(), 0);
	std::vector<uint64_t> slots(body.slots.size(), 0);
	uint64_t epoch = 0;
	const auto constant = [&](lowir::Value value) {
		return value.is(lowir::ValueKind::Integer) || (value.is(lowir::ValueKind::Temp) && constants[value.index] != 0);
	};
	for (const lowir::BlockId block : body.layout)
	{
		++epoch;
		for (lowir::InstId i = body.blocks[block].first; i != lowir::kNoId; i = body.instructions[i].next)
		{
			const lowir::Instruction& inst = body.instructions[i];
			bool known = false;
			switch (inst.op)
			{
			case lowir::Opcode::Const:
			case lowir::Opcode::Copy:
				known = inst.type.is_integer() && constant(inst.ops[0]);
				break;
			case lowir::Opcode::Convert:
				known = inst.type.is_integer() && lowir::convert_source_type(inst).is_integer() &&
					constant(inst.ops[0]);
				break;
			case lowir::Opcode::Load:
				known = (inst.flags & lowir::IF_VOLATILE) == 0 && inst.ops[0].is(lowir::ValueKind::Slot) &&
					inst.type.is_integer() && inst.type == body.slots[inst.ops[0].index].type &&
					slots[inst.ops[0].index] == epoch;
				break;
			case lowir::Opcode::Store:
				if (inst.ops[1].is(lowir::ValueKind::Slot))
					slots[inst.ops[1].index] = (inst.flags & lowir::IF_VOLATILE) == 0 && inst.type.is_integer() &&
							inst.type == body.slots[inst.ops[1].index].type && constant(inst.ops[0])
						? epoch
						: 0;
				else
					++epoch;
				break;
			case lowir::Opcode::Call:
				if (immediate_call(inst))
				{
					if (!constant(body.operands[inst.extra_first]))
						return false;
				}
				else
					++epoch;
				break;
			case lowir::Opcode::ZeroInit:
			case lowir::Opcode::CopyObject:
			{
				const lowir::Value target = inst.ops[inst.op == lowir::Opcode::ZeroInit ? 0 : 1];
				if (target.is(lowir::ValueKind::Slot))
					slots[target.index] = 0;
				else
					++epoch;
				break;
			}
			case lowir::Opcode::Addr:
			case lowir::Opcode::Index:
			case lowir::Opcode::Unary:
			case lowir::Opcode::Binary:
			case lowir::Opcode::Compare:
			case lowir::Opcode::Select:
			case lowir::Opcode::Phi:
			case lowir::Opcode::Jump:
			case lowir::Opcode::Branch:
			case lowir::Opcode::Switch:
			case lowir::Opcode::Return:
				break;
			default:
				++epoch;
				break;
			}
			if (inst.result != lowir::kNoId)
				constants[inst.result] = known ? 1 : 0;
		}
	}
	return true;
}

void SourceInlining::validate_immediates()
{
	for (const lowir::SymbolId symbol : deferred_)
	{
		lowir::FunctionBody& original = *program_.functions[program_.symbols[symbol].detail].body;
		bool checked = false, unresolved = false;
		for (const lowir::BlockId block : original.layout)
			for (lowir::InstId i = original.blocks[block].first; i != lowir::kNoId; i = original.instructions[i].next)
			{
				const lowir::Instruction& inst = original.instructions[i];
				if (immediate_call(inst))
				{
					checked = true;
					unresolved = unresolved || !original.operands[inst.extra_first].is(lowir::ValueKind::Integer);
				}
			}
		if (!checked)
			continue;
		// Annotated wrapper definitions may keep their formal parameters. Every
		// actual ordinary caller has already received the substituted checks.
		if (unresolved && (program_.symbols[symbol].metadata.flags & lowir::SF_FORCE_INLINE) == 0 &&
			!direct_immediates(original))
		{
			// Analyze a private copy so required diagnostics never change O0 code
			// or the user's named debug storage.
			lowir::FunctionBody body = original;
			opt::OptimizerStats stats;
			opt::FunctionContext context(program_, symbol, body, stats, 0);
			opt::promote_slots(context);
			opt::propagate_constants(context);
			opt::simplify_values(context);
			for (const lowir::BlockId block : body.layout)
				for (lowir::InstId i = body.blocks[block].first; i != lowir::kNoId; i = body.instructions[i].next)
				{
					const lowir::Instruction& inst = body.instructions[i];
					if (immediate_call(inst) && !body.operands[inst.extra_first].is(lowir::ValueKind::Integer))
						throw LoweringError(
							0, "a packed lane shuffle selector is not constant after mandatory inlining");
				}
		}
		remove_immediates(original);
	}
	// The transient declaration is never serialized or exposed as an object
	// symbol; all references have been removed.
	program_.order.erase(
		std::remove(program_.order.begin(), program_.order.end(), immediate_guard_), program_.order.end());
}

lowir::SymbolId ProgramLowering::source_immediate_guard()
{
	if (!source_inlining_)
		source_inlining_.reset(new SourceInlining(program_));
	const lowir::SymbolId known = source_inlining_->immediate_guard_id();
	if (known != lowir::kNoId)
		return known;
	return source_inlining_->immediate_guard(new_symbol("__cppgm_source_immediate"));
}

void ProgramLowering::mark_source_inline(const sema::FunctionEntity* function, lowir::SymbolId symbol)
{
	if (!host_objects_ || function->no_inline)
		return;
	const sema::FunctionEntity* pattern = function->instantiated ? sema::PatternOf(function) : nullptr;
	if (!function->always_inline && (function->primary == nullptr || !function->primary->always_inline) &&
		(pattern == nullptr || !pattern->always_inline))
		return;
	if (!source_inlining_)
		source_inlining_.reset(new SourceInlining(program_));
	lowir::SymbolId target = symbol;
	if (function->destructor && !function->is_virtual && base_symbols_.find(function) == symbol &&
		!HiddenBaseParameters(function, EntryVariant::Base))
		target = function_symbol(function);
	source_inlining_->mark(symbol, target);
}

void ProgramLowering::finish_source_inlining()
{
	std::unique_ptr<SourceInlining> work = std::move(source_inlining_);
	work->run();
	for (const lowir::SymbolId symbol : work->deferred())
		release_definition(symbol);
}

} // namespace lower
