#pragma once
#include "opt/passes.h"
namespace opt {
// Facts from the existing value walk. Queries never reparse or rescan a body.
class IntegerPatternFacts
{
public:
	virtual lowir::Value resolve(lowir::Value value) = 0;
	virtual uint64_t possible_bits(lowir::Value value, lowir::Type type, unsigned depth = 0) = 0;
	virtual uint64_t returned_power_bits(lowir::SymbolId symbol, lowir::Type type) = 0;

protected:
	~IntegerPatternFacts() = default;
};
// Union of literal single-bit values on every normal return, or zero when
// not proved. Bounded typed SSA walk; callers memoize it for their value walk.
uint64_t infer_return_power_bits(const lowir::Program& program, lowir::SymbolId symbol);
bool simplify_integer_arithmetic(lowir::FunctionBody& body, const std::vector<uint32_t>& uses,
	IntegerPatternFacts& facts, lowir::BlockId block, lowir::InstId& previous, lowir::InstId instruction);
bool simplify_integer_conversion(lowir::FunctionBody& body, const std::vector<uint32_t>& uses,
	IntegerPatternFacts& facts, lowir::Instruction& instruction);
// A compare of a successor (`x + 1 > y`), of an advanced pointer with its
// base (`p + n == p`) or of an offset value with a count that reaches the
// type's greatest value (`x - a < n`), rewritten in place to compare x or n.
bool simplify_integer_compare(lowir::FunctionBody& body, const std::vector<uint32_t>& uses, IntegerPatternFacts& facts,
	lowir::Instruction& instruction);
// An `or` that writes a bit-field at its storage unit's top back, stepped by
// a literal, over the unit's other bits: the unit plus the step at the
// field's place.
bool simplify_field_step(lowir::FunctionBody& body, const std::vector<uint32_t>& uses, IntegerPatternFacts& facts,
	lowir::Instruction& instruction);
// Whether x and y are one value scaled alike plus literals: steps and
// scalings by literals in `type` (of 64 bits or fewer) lead both to it.
// `difference` is then x - y, wrapping as the type does.
bool affine_difference(const lowir::FunctionBody& body, IntegerPatternFacts& facts, lowir::Value x, lowir::Value y,
	lowir::Type type, uint64_t& difference);
// The byte distance of a pointer difference `inst` of two element addresses
// of one root whose 64-bit indexes have an affine_difference.
bool index_difference(
	const lowir::FunctionBody& body, IntegerPatternFacts& facts, const lowir::Instruction& inst, int64_t& bytes);
bool simplify_pointer_distance(lowir::FunctionBody& body, IntegerPatternFacts& facts, lowir::BlockId block,
	lowir::InstId& previous, lowir::InstId instruction);
bool simplify_floor_correction(
	FunctionContext& context, lowir::BlockId block, lowir::InstId& previous, lowir::InstId instruction);
// Two truncating divisions by positive literals, rewritten in place as one
// by their product when it is a value of the type: x / a / b is x / (a * b).
// An unsigned quotient below the sign bit (of an unsigned right shift by
// k >= 1 or division by a >= 2) divided by b is an unsigned quotient:
// (x >> k) / b is x / (b << k).
bool merge_divisions(lowir::FunctionBody& body, lowir::Instruction& instruction);
// Whether the test of a branch edge that every path into `block` takes shows
// the signed value `v` at least zero: v itself above a literal of at least
// -1, or the value v is one less than above a literal of at least zero (a
// bounded walk up the dominator tree, which the context's graph must hold).
bool tested_not_negative(const FunctionContext& context, lowir::BlockId block, lowir::Value v);
// A signed division or remainder by a literal power of two whose operand
// tested_not_negative shows at least zero in `block`: rewritten in place as
// the shift or the mask.
bool unsign_tested_division(FunctionContext& context, lowir::BlockId block, lowir::Instruction& instruction);
// The caller has identified a truncation or right shift of an i128 pack.
bool simplify_packed_wide_part(lowir::FunctionBody& body, lowir::Instruction& instruction, bool truncation);
// Follow one side of a packed OR when the other contributes none of the
// observed bits. Same-width signedness copies preserve those bits.
lowir::Value projected_or_input(
	const lowir::FunctionBody& body, IntegerPatternFacts& facts, lowir::Value input, lowir::Type type, uint64_t kept);

} // namespace opt
