#include "lower/source_inlining.h"

#include "lower/program_lowering.h"
#include "opt/inliner.h"
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
				required_.find(inst.ops[0].index) != nullptr)
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
	for (size_t i = 0; i < annotated_.size(); ++i)
		program_.symbols[annotated_[i]].metadata.flags = flags[i];
	if (incomplete != lowir::kNoId)
		throw LoweringError(0,
			(std::string("an always_inline call of ") + program_.spelling(program_.symbols[incomplete].name) +
				" whose body cannot be substituted")
				.c_str());
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
