#pragma once

#include "lowir/program.h"
#include "support/flat_map.h"

namespace lower {

// Optional source-attribute work. Ordinary definitions keep streaming; only
// an annotated body or one that calls it is retained until declarations finish.
class SourceInlining
{
public:
	explicit SourceInlining(lowir::Program& program) : program_(program) {}
	void mark(lowir::SymbolId symbol, lowir::SymbolId target);
	bool defer(lowir::SymbolId symbol);
	void run();
	lowir::SymbolId immediate_guard(lowir::SymbolId symbol);
	lowir::SymbolId immediate_guard_id() const { return immediate_guard_; }
	const std::vector<lowir::SymbolId>& deferred() const { return deferred_; }

private:
	struct Hash
	{
		size_t operator()(uint32_t value) const { return value * 0x9e3779b9u; }
	};
	bool immediate_call(const lowir::Instruction& inst) const;
	bool direct_immediates(const lowir::FunctionBody& body) const;
	void validate_immediates();
	void remove_immediates(lowir::FunctionBody& body);
	lowir::Program& program_;
	lowir::SymbolId immediate_guard_ = lowir::kNoId;
	bool aliases_ = false;
	FlatMap<lowir::SymbolId, lowir::SymbolId, Hash> required_;
	std::vector<lowir::SymbolId> annotated_;
	std::vector<lowir::SymbolId> deferred_;
};

} // namespace lower
