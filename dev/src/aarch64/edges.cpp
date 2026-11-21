#include "aarch64/selector.h"
#include "aarch64/error.h"
#include "machine/parallel_register_moves.h"

#include <algorithm>
namespace aarch64 {
using namespace lowir;

namespace {
// A scalar may move among either register bank and its frame home.
// Register locations name all their bits, even when a move uses fewer.
constexpr uint32_t simd_base = 32, frame_location = 64;
constexpr unsigned limit = 32;
bool simd(uint32_t loc)
{
	return loc >= simd_base && loc < frame_location;
}
}

struct Selector::EdgeMoves
{
	machine::ScalarTransfer moves[limit];
	Value sources[limit];
	TempId results[limit];
	unsigned count = 0;
};

// The moves of an edge's phis among registers and frame homes. False when
// a value needs the snapshot path: plan_phis reserves the snapshot area
// for exactly those edges.
bool Selector::edge_moves(BlockId from, BlockId to, EdgeMoves& edge)
{
	auto& moves = edge.moves;
	auto& sources = edge.sources;
	auto& results = edge.results;
	unsigned& count = edge.count;
	const auto location = [&](TempId t) {
		return vectors_[t] != kNoId  ? simd_base + vectors_[t]
			: registers_[t] != kNoId ? registers_[t]
			: homes_[t] != kNoId     ? frame_location + homes_[t]
									 : kNoId;
	};
	// The interval allocator gives full-width vector spills their own homes.
	// Aliased call-result storage still uses the snapshot path.
	const auto vector_home_known = [&](TempId t) {
		return homes_[t] != kNoId && (call_result_slots_.empty() || call_result_slots_[t] == kNoId);
	};
	for (InstId i = body_.blocks[to].first; i != kNoId && body_.instructions[i].op == Opcode::Phi;
		i = body_.instructions[i].next)
	{
		const auto& phi = body_.instructions[i];
		const Value source = phis_.incoming(body_, i, from);
		if (same_home(phi.result, source))
			continue;
		if (count == limit ||
			!(source.is(ValueKind::Temp) || source.is(ValueKind::Integer) || source.is(ValueKind::Float)))
			return false;
		const uint32_t dst = location(phi.result);
		const bool literal = !source.is(ValueKind::Temp) || rematerialized(source.index);
		const uint32_t src = literal ? machine::kTransferLiteral : location(source.index);
		if (dst == kNoId || (!literal && src == kNoId))
			return false;
		if (memory_value(phi.type) &&
			(phi.type.kind() != TypeKind::V128 || (!simd(dst) && !vector_home_known(phi.result)) ||
				(src != machine::kTransferLiteral && !simd(src) && !vector_home_known(source.index))))
			return false;
		for (unsigned i = 0; i < count; ++i)
			if (moves[i].destination == dst)
				return false;
		sources[count] = source;
		results[count] = phi.result;
		moves[count++] = { dst, src };
	}
	return true;
}

bool Selector::parallel_edge(BlockId from, BlockId to)
{
	EdgeMoves edge;
	if (!edge_moves(from, to, edge))
		return false;
	auto& moves = edge.moves;
	auto& sources = edge.sources;
	auto& results = edge.results;
	const unsigned count = edge.count;
	const auto read = [&](unsigned reg, uint32_t loc, unsigned bytes) {
		if (loc >= frame_location)
			access(false, reg, frame_base(), loc - frame_location, bytes);
		else if (bytes == 4)
			out_.emit(Op::OrrW, reg, 31, loc);
		else
			move(reg, loc);
	};
	uint32_t scratch = 0;
	return machine::parallel_scalar_transfers<limit>(
		moves, count,
		[&](unsigned i) {
			const uint32_t dst = moves[i].destination;
			const bool vector = simd(dst);
			scratch = vector ? simd_base + 17 : 15;
			if (vector)
				vector_move(17, dst - simd_base);
			else if (body_.temps[results[i]].type.kind() == TypeKind::V128)
			{
				vector_home(false, 17, results[i]);
				scratch = simd_base + 17;
			}
			else
				read(15, dst, 8);
		},
		[&](unsigned i, uint32_t src) {
			const uint32_t dst = moves[i].destination;
			const Type type = body_.temps[results[i]].type;
			if (src == machine::kTransferSaved)
				src = scratch;
			if (type.kind() == TypeKind::V128)
			{
				const unsigned target = simd(dst) ? dst - simd_base : 16;
				save_vector(simd(src) ? src - simd_base : vector_value(sources[i], target), results[i]);
				return;
			}
			if (simd(dst))
			{
				const unsigned target = dst - simd_base;
				if (src == machine::kTransferLiteral)
					vector_move(target,
						type.is_vector() ? vector_value(sources[i], target) : fp_operand(sources[i], type, target));
				else if (simd(src))
					vector_move(target, src - simd_base);
				else if (src < simd_base)
					fp_move(true, target, src, type);
				else
					fp_home(false, target, sources[i].index);
				return;
			}
			const unsigned reg = dst < simd_base ? dst : 9;
			if (src == machine::kTransferLiteral)
				value(reg, sources[i], type);
			else if (simd(src))
			{
				if (dst >= frame_location)
				{
					fp_home(true, src - simd_base, results[i]);
					return;
				}
				fp_move(false, src - simd_base, reg, type);
			}
			else if (dst >= frame_location && src < simd_base && src != 15)
			{
				access(true, src, frame_base(), dst - frame_location, 8);
				return;
			}
			else
				read(reg, src, type.kind() == TypeKind::F32 ? 4 : 8);
			if (dst >= frame_location)
				access(true, reg, frame_base(), dst - frame_location, 8);
		});
}

void Selector::copy_edge(BlockId from, BlockId to)
{
	if (level_ && parallel_edge(from, to))
		return;
	// Snapshot all sources before committing any destination of this parallel copy.
	uint32_t offset = phi_area_;
	const InstId first = body_.blocks[to].first;
	if (level_ && !std::binary_search(frame_phi_blocks_.begin(), frame_phi_blocks_.end(), to))
		throw AArch64Error("AArch64 phi transfer needs a snapshot area the frame did not reserve");
	for (InstId i = first; i != kNoId && body_.instructions[i].op == Opcode::Phi; i = body_.instructions[i].next)
	{
		const Type type = body_.instructions[i].type;
		const auto source = phis_.incoming(body_, i, from);
		if (memory_value(type))
		{
			frame_address(10, offset);
			copy_value(10, source, type);
		}
		else
		{
			const auto reg = operand_register(source, type, 10);
			access(true, reg, frame_base(), offset, 8);
		}
		offset += (storage_bytes(type) + 7) & ~uint64_t(7);
	}
	offset = phi_area_;
	for (InstId i = first; i != kNoId && body_.instructions[i].op == Opcode::Phi; i = body_.instructions[i].next)
	{
		const auto& phi = body_.instructions[i];
		if (memory_value(phi.type))
		{
			frame_address(11, offset);
			if (phi.type.kind() == TypeKind::V128)
			{
				out_.emit(Op::LdrQ, 16, 11);
				save_vector(16, phi.result);
			}
			else
			{
				home_address(10, phi.result);
				copy_bytes(10, 11, storage_bytes(phi.type));
			}
		}
		else
		{
			const auto dest = registers_[phi.result] == kNoId ? 10 : registers_[phi.result];
			access(false, dest, frame_base(), offset, 8);
			// Scalar SSA values already have their type's canonical high bits.
			const auto source = phis_.incoming(body_, i, from);
			save(dest, phi.result, source.is(ValueKind::Temp) || source.is(ValueKind::Integer));
		}
		offset += (storage_bytes(phi.type) + 7) & ~uint64_t(7);
	}
}

bool Selector::same_home(TempId dest, Value source) const
{
	if (!source.is(ValueKind::Temp))
		return false;
	return registers_[dest] != kNoId ? registers_[source.index] == registers_[dest]
		: vectors_[dest] != kNoId    ? vectors_[source.index] == vectors_[dest]
									 : homes_[dest] != kNoId && homes_[dest] == homes_[source.index];
}

bool Selector::needs_snapshot(BlockId from, BlockId to)
{
	// An acyclic ordered transfer may include scalar frame homes. A later
	// source must see the value from before every destination write.
	uint32_t written[2] = {};
	uint32_t offsets[16], count = 0;
	for (auto i = body_.blocks[to].first; i != kNoId && body_.instructions[i].op == Opcode::Phi;
		i = body_.instructions[i].next)
	{
		const auto& phi = body_.instructions[i];
		const auto source = phis_.incoming(body_, i, from);
		if (same_home(phi.result, source))
			continue;
		if ((memory_value(phi.type) && phi.type.kind() != TypeKind::V128) ||
			!(source.is(ValueKind::Temp) || source.is(ValueKind::Integer) || source.is(ValueKind::Float)))
			return true;
		if (source.is(ValueKind::Temp))
		{
			const bool simd = vectors_[source.index] != kNoId;
			const auto reg = simd ? vectors_[source.index] : registers_[source.index];
			if (reg != kNoId ? (written[simd] & (1u << reg)) != 0
							 : std::find(offsets, offsets + count, homes_[source.index]) != offsets + count)
				return true;
		}
		const bool simd = vectors_[phi.result] != kNoId;
		const auto reg = simd ? vectors_[phi.result] : registers_[phi.result];
		if (reg != kNoId)
			written[simd] |= 1u << reg;
		else if (count < 16)
			offsets[count++] = homes_[phi.result];
		else
			return true;
	}
	return false;
}

uint64_t Selector::plan_phis()
{
	uint64_t maximum = 0;
	for (auto block : body_.layout)
	{
		const auto entry = body_.blocks[block].first;
		if (entry == kNoId || body_.instructions[entry].op != Opcode::Phi)
			continue;
		const auto& first = body_.instructions[entry];
		if (shared_entry_moves(block))
			continue;
		bool snapshot = !level_;
		if (level_)
			for (uint32_t k = 0; k < first.extra_count; k += 2)
			{
				const auto from = body_.operands[first.extra_first + k].index;
				if (needs_snapshot(from, block))
				{
					// Register moves make most such transfers; only the
					// rest read and write the snapshot area.
					snapshot_edges_.emplace_back(block, from);
					EdgeMoves moves;
					snapshot |= !edge_moves(from, block, moves);
				}
			}
		if (!snapshot)
			continue;
		frame_phi_blocks_.push_back(block);
		uint64_t bytes = 0;
		for (auto i = entry; i != kNoId && body_.instructions[i].op == Opcode::Phi; i = body_.instructions[i].next)
			bytes += (storage_bytes(body_.instructions[i].type) + 7) & ~uint64_t(7);
		maximum = std::max(maximum, bytes);
	}
	if (level_)
		for (auto block : body_.layout)
			if (shared_exit_moves(block))
			{
				if (exit_moves_.empty())
					exit_moves_.assign(body_.blocks.size(), 0);
				exit_moves_[block] = 1;
			}
	std::sort(snapshot_edges_.begin(), snapshot_edges_.end());
	std::sort(frame_phi_blocks_.begin(), frame_phi_blocks_.end());
	return maximum;
}

// When both edges of a two-way branch make the same register moves, or put
// the same integer literal in a general register, the block makes them once
// before its branch (a move keeps the flags), and neither edge needs code.
bool Selector::shared_exit_moves(BlockId block)
{
	const auto& term = body_.instructions[body_.blocks[block].last];
	if (term.op != Opcode::Branch || term.ops[1].index == term.ops[2].index)
		return false;
	const BlockId targets[2] = { term.ops[1].index, term.ops[2].index };
	for (const BlockId to : targets)
	{
		const auto first = body_.blocks[to].first;
		if (first == kNoId || body_.instructions[first].op != Opcode::Phi ||
			(!entry_moves_.empty() && entry_moves_[to] != kNoId) ||
			(out_.exceptions && out_.exceptions->entry_selectors[to]))
			return false;
	}
	EdgeMoves yes, no;
	if (!edge_moves(block, targets[0], yes) || !edge_moves(block, targets[1], no) || !yes.count ||
		yes.count != no.count)
		return false;
	const auto literal = [&](const EdgeMoves& edge, unsigned i) {
		return edge.moves[i].source == machine::kTransferLiteral && edge.sources[i].is(ValueKind::Integer) &&
			edge.moves[i].destination < simd_base;
	};
	// Each move of the true edge pairs with its own move of the false edge.
	static_assert(limit <= 32, "a bit for each edge move");
	uint32_t paired = 0;
	for (unsigned i = 0; i < yes.count; ++i)
	{
		const auto& move = yes.moves[i];
		const auto kind = body_.temps[yes.results[i]].type.kind();
		const bool constant = literal(yes, i);
		if (move.destination >= frame_location || (!constant && move.source >= frame_location) ||
			memory_value(body_.temps[yes.results[i]].type))
			return false;
		const auto same_literal = [&](unsigned k) {
			const auto& mine = body_.literals[yes.sources[i].index];
			const auto& theirs = body_.literals[no.sources[k].index];
			return literal(no, k) && mine.low == theirs.low && mine.high == theirs.high;
		};
		bool same = false;
		for (unsigned k = 0; k < no.count && !same; ++k)
			if (!(paired & (1u << k)) && no.moves[k].destination == move.destination &&
				no.moves[k].source == move.source && body_.temps[no.results[k]].type.kind() == kind &&
				(!constant || same_literal(k)))
			{
				paired |= 1u << k;
				same = true;
			}
		if (!same)
			return false;
	}
	return true;
}

// The moves both edges of `block` make, before its branch on `operand`
// (a condition code for B.cond). False, and the edges make their own,
// when a move would write the tested register or come between a pending
// bit test's AND and its branch.
bool Selector::exit_moves(BlockId block, BlockId to, Op test, unsigned operand)
{
	if (exit_moves_.empty() || !exit_moves_[block])
		return false;
	EdgeMoves moves;
	bool clear = edge_moves(block, to, moves);
	if (test != Op::BCond && clear && !out_.code.empty())
	{
		const auto& last = out_.code.back();
		clear = !((last.op == Op::AndImmW || last.op == Op::AndImm) && last.rd == operand);
		for (unsigned i = 0; clear && i < moves.count; ++i)
			clear = moves.moves[i].destination != operand;
	}
	exit_moves_[block] = clear;
	if (clear)
		parallel_edge(block, to);
	exit_done_ = clear ? block : kNoId;
	return clear;
}

// When every edge into a block moves the same registers, the block makes the
// moves once at its top. Its incoming edges then need no code or blocks.
bool Selector::shared_entry_moves(BlockId block)
{
	const auto& first = body_.instructions[body_.blocks[block].first];
	if (!level_ || first.extra_count < 4 ||
		(out_.exceptions &&
			(out_.exceptions->entry_selectors[block] ||
				out_.exceptions->analysis.eh_landing_index_[out_.exceptions->layout_of[block]] != kNoId)))
		return false;
	const auto location = [&](TempId t) -> uint32_t { return vectors_[t] != kNoId ? 32 + vectors_[t] : registers_[t]; };
	const BlockId from = body_.operands[first.extra_first].index;
	uint32_t sources = 0;
	unsigned moves = 0;
	for (auto i = body_.blocks[block].first; i != kNoId && body_.instructions[i].op == Opcode::Phi;
		i = body_.instructions[i].next)
	{
		const auto& phi = body_.instructions[i];
		const Value source = phis_.incoming(body_, i, from);
		const bool none = same_home(phi.result, source);
		const bool movable = source.is(ValueKind::Temp) && !rematerialized(source.index) && !memory_value(phi.type) &&
			location(source.index) != kNoId && location(phi.result) != kNoId;
		if (!none && (!movable || ++moves == 32))
			return false;
		for (uint32_t k = 2; k < first.extra_count; k += 2)
		{
			const Value other = phis_.incoming(body_, i, body_.operands[first.extra_first + k].index);
			if (none ? !same_home(phi.result, other)
					 : !other.is(ValueKind::Temp) || rematerialized(other.index) ||
						location(other.index) != location(source.index))
				return false;
		}
		if (!none && registers_[source.index] != kNoId)
			sources |= 1u << registers_[source.index];
	}
	if (!moves)
		return false;
	if (entry_moves_.empty())
		entry_moves_.assign(body_.blocks.size(), kNoId);
	entry_moves_[block] = from;
	if (!entry_regs_.empty())
		entry_regs_[block] |= sources;
	return true;
}

void Selector::entry_moves(BlockId block)
{
	if (!entry_moves_.empty() && entry_moves_[block] != kNoId)
		parallel_edge(entry_moves_[block], block);
}

// A block of phis and a jump whose edges all need no code is not emitted.
// Its label is its successor's, so its predecessors branch there directly.
void Selector::plan_forwarding(const std::vector<BlockId>& layout)
{
	if (!level_ || !body_.code_labels.empty())
		return;
	for (size_t k = 1; k < layout.size(); ++k)
	{
		const BlockId block = layout[k];
		const auto last = body_.blocks[block].last;
		if (last == kNoId || body_.instructions[last].op != Opcode::Jump)
			continue;
		const BlockId to = body_.instructions[last].ops[0].index;
		if (to == block || (!entry_moves_.empty() && entry_moves_[block] != kNoId))
			continue;
		// The block that enters a loop builds the constants the loop holds.
		if (std::any_of(held_constants_.begin(), held_constants_.end(),
				[&](const HeldConstant& held) { return held.block == block; }))
			continue;
		if (out_.exceptions)
		{
			const auto& plan = *out_.exceptions;
			const auto b = plan.layout_of[block];
			if (plan.entry_selectors[block] || plan.analysis.eh_landing_index_[b] != kNoId ||
				!plan.analysis.blocks_[b].reachable)
				continue;
		}
		const auto first = body_.blocks[block].first;
		bool empty = empty_edge(block, to);
		for (auto i = first; empty && i != last; i = body_.instructions[i].next)
			empty = body_.instructions[i].op == Opcode::Phi;
		const auto& phi = body_.instructions[first];
		for (uint32_t p = 0; empty && phi.op == Opcode::Phi && p < phi.extra_count; p += 2)
			empty = empty_edge(body_.operands[phi.extra_first + p].index, block);
		if (!empty)
			continue;
		if (forwarded_.empty())
			forwarded_.assign(body_.blocks.size(), kNoId);
		// An empty loop of forwarding blocks keeps its last jump.
		auto end = to;
		while (forwarded(end) && end != block)
			end = forwarded_[end];
		if (end != block)
			forwarded_[block] = to;
	}
}

BlockId Selector::next_emitted(const std::vector<BlockId>& layout, size_t index) const
{
	while (++index < layout.size())
		if (!forwarded(layout[index]))
			return layout[index];
	return kNoId;
}

void Selector::bind_forwarded()
{
	for (BlockId block = 0; block < forwarded_.size(); ++block)
		if (forwarded_[block] != kNoId)
		{
			auto to = forwarded_[block];
			while (forwarded(to))
				to = forwarded_[to];
			out_.labels[block] = out_.labels[to];
		}
}

void Selector::register_edge(BlockId from, BlockId to)
{
	for (auto i = body_.blocks[to].first; i != kNoId && body_.instructions[i].op == Opcode::Phi;
		i = body_.instructions[i].next)
	{
		const auto& phi = body_.instructions[i];
		const auto source = phis_.incoming(body_, i, from);
		if (same_home(phi.result, source))
			continue;
		if (phi.type.kind() == TypeKind::V128)
			save_vector(vector_value(source, 16), phi.result);
		else if (vectors_[phi.result] != kNoId)
			vector_move(vectors_[phi.result], fp_operand(source, phi.type, vectors_[phi.result]));
		else if (registers_[phi.result] != kNoId)
			value(registers_[phi.result], source, phi.type);
		else
		{
			const auto reg = operand_register(source, phi.type, 9);
			access(true, reg, frame_base(), homes_[phi.result], 8);
		}
	}
}

bool Selector::empty_edge(BlockId block, BlockId to)
{
	const auto first = body_.blocks[to].first;
	if (out_.exceptions && out_.exceptions->entry_selectors[to])
		return false;
	if ((!entry_moves_.empty() && entry_moves_[to] != kNoId) || (!exit_moves_.empty() && exit_moves_[block]))
		return true;
	for (auto i = first; i != kNoId && body_.instructions[i].op == Opcode::Phi; i = body_.instructions[i].next)
	{
		const auto& phi = body_.instructions[i];
		const auto source = phis_.incoming(body_, i, block);
		if (!level_ || !same_home(phi.result, source))
			return false;
	}
	return true;
}

// A short register-only backedge copy can run before its branch when the
// destinations are all dead on the other edge. MOV preserves the flags.
bool Selector::backedge_moves(BlockId from, BlockId to, BlockId other, unsigned condition)
{
	if (entry_regs_.empty() || out_.labels[to] == kNoId || exit_done_ == from || !empty_edge(from, other) ||
		condition == 15 || (out_.exceptions && out_.exceptions->entry_selectors[to]) ||
		(!entry_moves_.empty() && entry_moves_[to] != kNoId))
		return false;
	machine::ScalarTransfer moves[2];
	unsigned count = 0;
	for (InstId i = body_.blocks[to].first; i != kNoId && body_.instructions[i].op == Opcode::Phi;
		i = body_.instructions[i].next)
	{
		const auto& phi = body_.instructions[i];
		const Value incoming = phis_.incoming(body_, i, from);
		if (same_home(phi.result, incoming))
			continue;
		if (count == 2 || !incoming.is(ValueKind::Temp))
			return false;
		const auto dst = registers_[phi.result], src = registers_[incoming.index];
		if (dst == kNoId || src == kNoId || dst == condition || (entry_regs_[other] & (1u << dst)))
			return false;
		moves[count++] = { dst, src };
	}
	return count &&
		machine::parallel_scalar_transfers(
			moves, count, [&](unsigned i) { move(15, moves[i].destination); },
			[&](unsigned i, uint32_t src) { move(moves[i].destination, src == machine::kTransferSaved ? 15 : src); });
}

void Selector::conditional_edges(BlockId block, const lowir::Instruction& branch, Op test, unsigned operand)
{
	const BlockId yes = branch.ops[1].index, no = branch.ops[2].index;
	const auto inverse = test == Op::Cbz ? Op::Cbnz
		: test == Op::Cbnz               ? Op::Cbz
		: test == Op::CbzW               ? Op::CbnzW
		: test == Op::CbnzW              ? Op::CbzW
										 : Op::BCond;
	const auto inverse_operand = test == Op::BCond ? operand ^ 1 : operand;
	exit_moves(block, yes, test, operand);
	if (level_ && (no == fallthrough_ || yes == fallthrough_))
	{
		const bool taken = no == fallthrough_;
		const BlockId target = taken ? yes : no;
		if (backedge_moves(block, target, fallthrough_, test == Op::BCond ? kNoId : operand))
		{
			out_.reference(taken ? inverse : test, Fixup::Block, target, taken ? inverse_operand : operand);
			return;
		}
	}
	else if (level_ >= 2 && !block_weights_.empty())
	{
		// When neither destination follows this block, send the conditional
		// branch directly to a hot header and leave the extra jump on exit.
		const bool taken = block_weights_[yes] > block_weights_[no];
		const BlockId hot = taken ? yes : no, cold = taken ? no : yes;
		if (out_.labels[hot] != kNoId && block_weights_[hot] / 2 > block_weights_[cold] &&
			(empty_edge(block, hot) || backedge_moves(block, hot, cold, test == Op::BCond ? kNoId : operand)))
		{
			out_.reference(taken ? inverse : test, Fixup::Block, hot, taken ? inverse_operand : operand);
			edge(block, cold, true);
			return;
		}
	}
	if (no == fallthrough_ && empty_edge(block, yes))
	{
		out_.reference(inverse, Fixup::Block, yes, inverse_operand);
		edge(block, no, true);
		return;
	}
	// When neither edge needs code, the jump goes on the less frequent one:
	// by weight, or else, when both go back, the edge that does not go to
	// the nearer of the two, which is the inner loop's header. Placement's
	// estimates, where there are some, leave this to orient_branches.
	const bool both_behind = out_.labels[yes] != kNoId && out_.labels[no] != kNoId;
	if (level_ && branch_estimates_.empty() && yes != fallthrough_ && no != fallthrough_ &&
		(block_weights_.empty() ? both_behind && out_.labels[yes] > out_.labels[no]
								: block_weights_[yes] > block_weights_[no]) &&
		empty_edge(block, yes) && empty_edge(block, no))
	{
		out_.reference(inverse, Fixup::Block, yes, inverse_operand);
		edge(block, no, true);
		return;
	}
	// The moves of a colder edge beside an empty one that falls through go
	// out of line too, as placement prices them.
	if (no == fallthrough_ && defer_edge(block, yes, no, inverse, inverse_operand))
		return;
	if (level_ && empty_edge(block, no))
	{
		out_.reference(test, Fixup::Block, no, operand);
		edge(block, yes, true);
		return;
	}
	// The branch goes to the empty edge's destination, and the other edge
	// makes its moves and jumps.
	if (level_ && yes != fallthrough_ && empty_edge(block, yes))
	{
		out_.reference(inverse, Fixup::Block, yes, inverse_operand);
		edge(block, no, true);
		return;
	}
	if (defer_edge(
			block, yes, no, yes == fallthrough_ ? test : inverse, yes == fallthrough_ ? operand : inverse_operand))
		return;
	const uint32_t other = out_.labels.size();
	out_.labels.push_back(kNoId);
	const bool yes_next = yes == fallthrough_;
	out_.reference(yes_next ? inverse : test, Fixup::Block, other, yes_next ? inverse_operand : operand);
	edge(block, yes_next ? no : yes);
	out_.bind(other);
	edge(block, yes_next ? yes : no, true);
}

// When the edge that falls through runs more often than the other, by
// placement's estimates, the branch goes to the other edge's moves, made
// out of line after the next code that does not fall through
// (flush_deferred_edges). `test` and `operand` take the other edge.
bool Selector::defer_edge(BlockId block, BlockId yes, BlockId no, Op test, unsigned operand)
{
	if (level_ < 2 || out_.exceptions || branch_estimates_.empty() || (yes != fallthrough_ && no != fallthrough_))
		return false;
	const auto& estimate = branch_estimates_[block];
	const BlockId away = yes == fallthrough_ ? no : yes;
	const uint64_t next_edge = yes == fallthrough_ ? estimate.to_yes : estimate.to_no;
	const uint64_t moved = yes == fallthrough_ ? estimate.to_no : estimate.to_yes;
	if (estimate.yes != yes || estimate.no != no ||
		!(next_edge > moved || (next_edge == moved && shared_move(block, away))))
		return false;
	const uint32_t label = out_.labels.size();
	out_.labels.push_back(kNoId);
	out_.reference(test, Fixup::Block, label, operand);
	deferred_edges_.push_back({ block, away, label });
	edge(block, fallthrough_, true);
	return true;
}

// Whether the edge makes one register move that another edge into `to`
// makes alone: share_edge_moves sends a branch to its deferred copy to the
// other edge's where that falls into `to`.
bool Selector::shared_move(BlockId from, BlockId to)
{
	const auto first = body_.blocks[to].first;
	if (first == kNoId || body_.instructions[first].op != Opcode::Phi || empty_edge(from, to))
		return false;
	EdgeMoves mine;
	if (!edge_moves(from, to, mine) || mine.count != 1 || mine.moves[0].destination >= simd_base ||
		mine.moves[0].source >= simd_base || !body_.temps[mine.results[0]].type.is_integer())
		return false;
	const auto& phi = body_.instructions[first];
	for (uint32_t k = 0; k < phi.extra_count; k += 2)
	{
		const BlockId other = body_.operands[phi.extra_first + k].index;
		EdgeMoves theirs;
		if (other != from && edge_moves(other, to, theirs) && theirs.count == 1 &&
			theirs.moves[0].destination == mine.moves[0].destination && theirs.moves[0].source == mine.moves[0].source)
			return true;
	}
	return false;
}

// The deferred edges' moves and jumps, where no code falls through. The
// last falls into the block that follows when it goes there, and a label
// bound at the end, which expects that block, moves past them.
void Selector::flush_deferred_edges()
{
	const auto end = static_cast<uint32_t>(out_.code.size());
	std::vector<uint32_t> ending;
	if (out_.last_bound == end)
		for (uint32_t label = 0; label < out_.labels.size(); ++label)
			if (out_.labels[label] == end)
				ending.push_back(label);
	for (size_t k = 0; k < deferred_edges_.size(); ++k)
	{
		out_.bind(deferred_edges_[k].label);
		edge(deferred_edges_[k].from, deferred_edges_[k].to, k + 1 == deferred_edges_.size());
	}
	for (auto label : ending)
		out_.bind(label);
	deferred_edges_.clear();
}

void Selector::edge(BlockId from, BlockId to, bool last)
{
	if ((entry_moves_.empty() || entry_moves_[to] == kNoId) && exit_done_ != from)
	{
		if (!level_ || std::binary_search(snapshot_edges_.begin(), snapshot_edges_.end(), std::make_pair(to, from)))
			copy_edge(from, to);
		else
			register_edge(from, to);
	}
	if (out_.exceptions && out_.exceptions->entry_selectors[to] != 0)
	{
		constant(9, out_.exceptions->entry_selectors[to]);
		access(true, 9, frame_base(), exception_slot_ + 8, 8);
	}
	if (!last || to != fallthrough_)
		out_.reference(Op::B, Fixup::Block, to);
}

} // namespace aarch64
