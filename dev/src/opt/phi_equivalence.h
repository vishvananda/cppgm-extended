#pragma once
#include "opt/passes.h"
namespace opt {
// Reusable predecessor scratch for a bounded proof of equal cyclic phi graphs.
class PhiComparison
{
public:
	bool equal(FunctionContext& context, const lowir::Instruction& a, const lowir::Instruction& b,
		const std::vector<lowir::Value>& replacements);

private:
	std::vector<lowir::Value> pred_input_;
	std::vector<uint32_t> pred_stamp_;
	uint32_t stamp_ = 0;
};
uint64_t phi_signature(
	const lowir::FunctionBody& body, const std::vector<lowir::BlockId>& block_of, const lowir::Instruction& phi);
// A phi whose every input is a retyping copy from one other type of the same
// width (a pointer's bits as i64, an integer in the other signedness), a
// zero or the phi itself becomes that copy of a phi of the copies' sources;
// one whose every input steps a pointer by the same constant (and is no
// loop's pointer) becomes that step from a phi of the pointers.
// Inputs are read through `replacements`, which grows with `uses` for the
// new phis; a new phi precedes the block's other phis.
bool retype_copy_phis(FunctionContext& context, lowir::BlockId block, std::vector<lowir::Value>& replacements,
	std::vector<uint32_t>& uses);
// A phi whose every input is one integer conversion (an extension or a
// truncation from one type) of the input another phi of the block takes from
// that predecessor, or a constant that conversion takes that phi's constant
// to, becomes the conversion of the other phi.
bool convert_matching_phis(FunctionContext& context, lowir::BlockId block, std::vector<lowir::Value>& replacements,
	std::vector<uint32_t>& uses);
// A phi whose every input is one chain of integer operations with literals
// (a few steps) on the input another phi of the block takes from that
// predecessor, or the constant the chain takes that phi's constant to,
// becomes the chain on the other phi: a value kept one step behind another
// (a heap's hole behind its parent) then needs no register of its own that
// the back edge would trade with the other's. New temps grow `replacements`
// and `uses`.
bool derive_matching_phis(FunctionContext& context, lowir::BlockId block, std::vector<lowir::Value>& replacements,
	std::vector<uint32_t>& uses);
// A diamond: `head` branches on c to two arms with it as their one
// predecessor, which jump to one merge they are the only predecessors of.
struct Diamond
{
	lowir::BlockId head = lowir::kNoId, on_true = lowir::kNoId, on_false = lowir::kNoId, merge = lowir::kNoId;
	lowir::Value condition;
};
bool diamond_at(const lowir::FunctionBody& body, const Cfg& cfg, lowir::BlockId head, Diamond& d);
// No instruction of the block writes memory.
bool write_free(const lowir::Program& program, const lowir::FunctionBody& body, lowir::BlockId block);
// The write-free blocks of two back-to-back diamonds (their arms and the
// first merge).
struct QuietBlocks
{
	const std::vector<lowir::BlockId>& block_of;
	lowir::BlockId blocks[5];
	bool holds(lowir::InstId inst) const
	{
		const lowir::BlockId b = inst < block_of.size() ? block_of[inst] : lowir::kNoId;
		for (lowir::BlockId q : blocks)
			if (b != lowir::kNoId && b == q)
				return true;
		return false;
	}
};
// Whether a and b compute the same value: equal operands, or the same pure
// operation or nonvolatile load over operands that are (depth-bounded). A
// load counts only inside `quiet`, so no write can separate the two; a load
// before the first diamond may follow a write.
bool same_computation(
	const lowir::FunctionBody& body, lowir::Value a, lowir::Value b, unsigned depth, const QuietBlocks& quiet);
uint32_t phi_count(const lowir::FunctionBody& body, lowir::BlockId block);
} // namespace opt
