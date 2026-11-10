#pragma once
#include "opt/passes.h"
namespace opt {
// Facts from the existing value walk. Queries never reparse or rescan a body.
class IntegerPatternFacts
{
public:
	virtual lowir::Value resolve(lowir::Value value) = 0;
	virtual uint64_t possible_bits(lowir::Value value, lowir::Type type, unsigned depth = 0) = 0;

protected:
	~IntegerPatternFacts() = default;
};
bool simplify_integer_arithmetic(lowir::FunctionBody& body, const std::vector<uint32_t>& uses,
	IntegerPatternFacts& facts, lowir::BlockId block, lowir::InstId& previous, lowir::InstId instruction);
bool simplify_integer_conversion(lowir::FunctionBody& body, const std::vector<uint32_t>& uses,
	IntegerPatternFacts& facts, lowir::Instruction& instruction);
bool simplify_pointer_distance(lowir::FunctionBody& body, IntegerPatternFacts& facts, lowir::BlockId block,
	lowir::InstId& previous, lowir::InstId instruction);
} // namespace opt
