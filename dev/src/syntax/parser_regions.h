#pragma once

#include <cstdint>

#include "support/stack_budget.h"
#include "syntax/parser_state.h"

// Scoped regions of a parse: nesting levels, tentative alternatives and
// speculation boundaries (see ParserState).
namespace syntax {

// Marks one level of syntactic nesting (an expression operand, a
// statement, a declaration, a declarator, a template argument list, a
// braced list). When the parser's recursion has used its stack budget
// (half the process stack limit), the translation unit is rejected instead
// of overflowing the stack; frame sizes differ between constructs, so the
// budget is measured in bytes rather than levels. Typical limits are
// thousands of levels, far above C++11 Annex B's minimums.
class NestingGuard
{
public:
	explicit NestingGuard(ParserState& parser) : parser_(parser)
	{
		char marker;
		if (stack_bytes_used(parser.stack_base_, &marker) > parser.stack_budget_)
			throw ParseError(parser.location(), "constructs nested too deeply");
		telemetry::note_peak(parser_.peak_nesting_, ++parser_.nesting_);
	}
	~NestingGuard() { --parser_.nesting_; }
	NestingGuard(const NestingGuard&) = delete;
	NestingGuard& operator=(const NestingGuard&) = delete;

private:
	ParserState& parser_;
};

// Opens a tentative region; unless committed, the parser is restored when
// it ends.
class Tentative
{
public:
	explicit Tentative(ParserState& parser) : parser_(parser), checkpoint_(parser.save()), open_(true) {}
	~Tentative()
	{
		if (open_)
			parser_.restore(checkpoint_);
	}
	Tentative(const Tentative&) = delete;
	Tentative& operator=(const Tentative&) = delete;

	// Keeps what the region parsed; later calls do nothing.
	void commit()
	{
		if (!open_)
			return;
		open_ = false;
		parser_.commit(checkpoint_);
	}
	bool committed() const { return !open_; }
	void abandon()
	{
		parser_.restore(checkpoint_);
		open_ = false;
	}

private:
	ParserState& parser_;
	ParserState::Checkpoint checkpoint_;
	bool open_;
};

// Starts a region whose parsing no enclosing speculation can retry
// piecemeal: a class body (its members and deferred bodies are parsed with
// the class) or a deferred lambda body. Inside, lambda bodies are parsed
// directly again, and a failed speculative template argument list
// abandons enclosing lists only up to the boundary.
class SpeculationBoundary
{
public:
	explicit SpeculationBoundary(ParserState& parser)
		: parser_(parser), speculation_(parser.speculation_), open_lists_base_(parser.open_lists_base_)
	{
		parser.speculation_ = 0;
		parser.open_lists_base_ = parser.open_lists_.size();
	}
	~SpeculationBoundary()
	{
		parser_.speculation_ = speculation_;
		parser_.open_lists_base_ = open_lists_base_;
	}
	SpeculationBoundary(const SpeculationBoundary&) = delete;
	SpeculationBoundary& operator=(const SpeculationBoundary&) = delete;

private:
	ParserState& parser_;
	uint32_t speculation_;
	size_t open_lists_base_;
};

} // namespace syntax
