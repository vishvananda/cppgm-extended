#include "native/select/function_selector.h"
#include "native/select/for_each_inst.h"
#include "native/select/host_eh.h"

#include <string>
#include <algorithm>
#include <functional>
#include <utility>

#include "native/backend_error.h"

// The host EH model's static view of the course's handler records (their
// dynamic semantics: native/runtime/runtime.h). Each block entry has one
// record stack, so each potentially throwing call has one protecting
// record -- the innermost record whose landing is not running -- whose
// landing pad the call's LSDA entry names, and each landing one action
// chain.
//
// A state is a node of a trie of records (parent, landing, kind, active),
// interned so that equal stacks are equal nodes.
// - eh_try/eh_cleanup ^L push an inactive record; L's entry state is the
//   stack with that record active (every push of L must give the same).
// - a bare eh_cleanup makes an active try record on top a cleanup record;
//   eh_end pops the top record; an `exception` read pops an active try
//   record on top.
// - resume drops the active records on top: an inactive record left in
//   the frame receives the exception at its landing (a jump past its pad:
//   the frame slots still hold the exception and selector), otherwise the
//   function's resume terminal continues unwinding.
// Control-flow merges must agree on the records that protect calls (pa26
// README 7). Paths may still differ in records whose landings are running
// -- landings forward to a shared cleanup suffix, another landing's block
// or a catch dispatch -- which no region or resume reaches: the merged
// state is a tail node over their common stack, with the range of the
// number of such records and the kind of the top one, which eh_end and
// `exception` reads resolve when every path agrees on the result and
// resume drops.
//
// Paths also carry the set of selector values the landing dispatch can
// see (a bit mask): a pad leaves its landing's clause selectors (0 too
// without a catch-all), a selector compare's branch edges split the set
// and an edge no selector takes is not followed -- the frontend's no-match
// path after a catch-all, whose regions need not balance.
//
// The tables (chains, type table, selector translation): host_eh_tables.cpp.
namespace native {

using lowir::Instruction;
using lowir::kNoId;
using lowir::Opcode;
using lowir::TypeKind;
using lowir::ValueKind;

namespace {

using host_eh::bare_cleanup;
using host_eh::clause;
using host_eh::selector_bit;

// Trie keys: a record's landing below bit 60, its kind and activity in
// bits 60 and 61; a tail's key sets bit 62.
const uint32_t kMaxLanding = 1u << 28;
const uint64_t kTailKey = static_cast<uint64_t>(1) << 62;

// Functions of at most this many blocks reach their states' fixed point
// without the reverse postorder.
const uint32_t kSmallFunctionBlocks = 32;

// The kind of a tail's top record: none, try, cleanup, either.
enum : uint8_t
{
	kTopNone,
	kTopTry,
	kTopCleanup,
	kTopMixed
};

uint8_t combine_top(uint8_t a, uint8_t b)
{
	return a == kTopNone ? b : (b == kTopNone || a == b ? a : kTopMixed);
}

} // namespace

uint32_t FunctionSelector::eh_node(uint32_t parent, uint32_t landing, bool cleanup, bool active)
{
	if (landing >= kMaxLanding)
		eh_fail("too many blocks");
	const uint64_t key = parent | (static_cast<uint64_t>(landing) << 32) | (static_cast<uint64_t>(cleanup) << 60) |
		(static_cast<uint64_t>(active) << 61);
	if (const uint32_t* known = eh_node_index_.find(key))
		return *known;
	const uint32_t id = static_cast<uint32_t>(eh_nodes_.size());
	const EhNode& below = eh_nodes_[parent];
	EhNode node;
	node.parent = parent;
	node.landing = landing;
	node.depth = below.depth + 1;
	node.cleanup = cleanup;
	node.active = active;
	node.inside = parent != 0 && (below.active || below.inside);
	node.tail_lo = node.tail_hi = 0;
	node.tail_top = kTopNone;
	node.protect = active ? below.protect : id;
	node.chain = 0;
	node.chain_count = kNoId;
	eh_nodes_.push_back(node);
	eh_node_index_.insert(key, id);
	return id;
}

// `base` with lo..hi active records of the frame on top, whose top is of
// kind `top` (kTop*); `base` itself without any.
uint32_t FunctionSelector::eh_tail(uint32_t base, uint32_t lo, uint32_t hi, uint8_t top)
{
	if (hi == 0)
		return base;
	if (hi > 255)
		eh_fail("too many landing records merge");
	const uint64_t key = base | (static_cast<uint64_t>(lo) << 32) | (static_cast<uint64_t>(hi) << 40) |
		(static_cast<uint64_t>(top) << 48) | kTailKey;
	if (const uint32_t* known = eh_node_index_.find(key))
		return *known;
	const uint32_t id = static_cast<uint32_t>(eh_nodes_.size());
	const EhNode& below = eh_nodes_[base];
	EhNode node;
	node.parent = base;
	node.landing = kNoId;
	node.depth = below.depth + 1;
	node.cleanup = false;
	node.active = true;
	node.inside = true;
	node.tail_lo = static_cast<uint8_t>(lo);
	node.tail_hi = static_cast<uint8_t>(hi);
	node.tail_top = top;
	node.protect = below.protect;
	node.chain = 0;
	node.chain_count = kNoId;
	eh_nodes_.push_back(node);
	eh_node_index_.insert(key, id);
	return id;
}

// The state two paths' states merge to (their join: states only widen).
// Position by position from the top: records of one landing merge into
// one record; records of different landings that are both active (their
// landings running: no call's region and no resume reaches them) become a
// tail over the merged rest. The protecting records must agree (pa26
// README 7). One landing's record may be inactive on one path and an
// active cleanup record on the other: a landing's no-match path forwards
// to an enclosing try's dispatch with that try's record left installed.
// Both paths are inside its landing: the record protects nothing (nor does
// a handler's own try in C++) and the paths' pops agree.
uint32_t FunctionSelector::eh_merge(uint32_t a, uint32_t b)
{
	if (a == b)
		return a;
	uint32_t base[2] = { a, b };
	uint32_t lo[2] = { 0, 0 };
	uint32_t hi[2] = { 0, 0 };
	uint8_t top[2] = { kTopNone, kTopNone };
	uint32_t climbed[2] = { 0, 0 };
	uint8_t first[2] = { kTopNone, kTopNone };
	for (unsigned s = 0; s < 2; ++s)
	{
		const EhNode& node = eh_nodes_[base[s]];
		if (node.tail_hi == 0)
			continue;
		lo[s] = node.tail_lo;
		hi[s] = node.tail_hi;
		top[s] = node.tail_top;
		base[s] = node.parent;
	}
	auto climb = [&](unsigned s) {
		const EhNode& node = eh_nodes_[base[s]];
		if (base[s] == 0 || !node.active || node.tail_hi != 0)
			eh_fail("control-flow paths with different EH regions merge");
		if (climbed[s]++ == 0)
			first[s] = node.cleanup ? kTopCleanup : kTopTry;
		base[s] = node.parent;
	};
	while (eh_nodes_[base[0]].depth > eh_nodes_[base[1]].depth)
		climb(0);
	while (eh_nodes_[base[1]].depth > eh_nodes_[base[0]].depth)
		climb(1);
	while (base[0] != base[1])
	{
		const EhNode x = eh_nodes_[base[0]];
		const EhNode y = eh_nodes_[base[1]];
		if (x.landing == y.landing && x.tail_hi == 0 && y.tail_hi == 0)
		{
			// Both active with different kinds: part of the tail.
			bool cleanup = x.cleanup;
			bool active = x.active;
			bool position = x.cleanup == y.cleanup;
			if (x.active != y.active)
			{
				if (!(x.active ? x.cleanup : y.cleanup))
					eh_fail("control-flow paths with different EH regions merge");
				cleanup = active = position = true;
			}
			else if (!position && !x.active)
				eh_fail("control-flow paths with different EH regions merge");
			if (position)
			{
				base[0] = base[1] = eh_node(eh_merge(x.parent, y.parent), x.landing, cleanup, active);
				break;
			}
		}
		climb(0);
		climb(1);
	}
	uint8_t merged_top = kTopNone;
	for (unsigned s = 0; s < 2; ++s)
	{
		uint8_t t = hi[s] > 0 ? top[s] : kTopNone;
		if (climbed[s] > 0 && lo[s] == 0)
			t = combine_top(t, first[s]);
		merged_top = combine_top(merged_top, t);
		lo[s] += climbed[s];
		hi[s] += climbed[s];
	}
	return eh_tail(base[0], lo[0] < lo[1] ? lo[0] : lo[1], hi[0] > hi[1] ? hi[0] : hi[1], merged_top);
}

void FunctionSelector::eh_enter(uint32_t block, uint32_t state, uint64_t mask, std::vector<uint32_t>& work)
{
	uint32_t& in = eh_in_[block];
	if (in == kNoId)
	{
		in = state;
		eh_mask_in_[block] = mask;
		work.push_back(block);
		return;
	}
	const uint32_t merged = eh_merge(in, state);
	if (merged != in || (eh_mask_in_[block] | mask) != eh_mask_in_[block])
	{
		in = merged;
		eh_mask_in_[block] |= mask;
		work.push_back(block);
	}
}

// The selectors a landing's pad can leave: its catch clauses', and 0
// unless one of them is a catch-all (the first chain entry the
// personality matches is then always one of the landing's).
uint64_t FunctionSelector::eh_landing_mask(uint32_t block)
{
	const lowir::FunctionBody& body = *body_;
	uint64_t mask = 1;
	bool catch_all = false;
	for (lowir::InstId i = body.blocks[blocks_[block].id].first; i != kNoId; i = body.instructions[i].next)
	{
		const Instruction& inst = body.instructions[i];
		if (!clause(inst))
			break;
		if (inst.op == Opcode::EhCatch || inst.op == Opcode::EhCatchAll || inst.op == Opcode::EhFilter)
			mask |= selector_bit(inst.immediate);
		catch_all = catch_all || inst.op == Opcode::EhCatchAll;
	}
	return catch_all ? mask & ~static_cast<uint64_t>(1) : mask;
}

// The selector a jump into landing block `b` leaves, as the course runtime
// matching its clauses again would: when its first catch clause is a
// catch-all (a terminate guard another landing forwards to), that
// clause's selector unless a selector is set; 0 when the landing has no
// catch clause first or is no landing.
int64_t FunctionSelector::eh_entry_selector(uint32_t b) const
{
	if (!blocks_[b].handler)
		return 0;
	const lowir::FunctionBody& body = *body_;
	for (lowir::InstId i = body.blocks[blocks_[b].id].first; i != kNoId; i = body.instructions[i].next)
	{
		const Instruction& inst = body.instructions[i];
		if (!clause(inst))
			return 0;
		if (inst.op == Opcode::EhCatchAll)
			return inst.immediate;
		if (inst.op == Opcode::EhCatch)
			return 0;
	}
	return 0;
}

// The selectors an edge into block `target` carries from a path with
// `mask`: a catch-all-first landing sets the selector no clause set.
uint64_t FunctionSelector::eh_edge_mask(uint32_t target, uint64_t mask) const
{
	const int64_t entry = eh_entry_selector(target);
	return entry != 0 && (mask & 1) ? (mask & ~static_cast<uint64_t>(1)) | selector_bit(entry) : mask;
}

// Whether a conditional edge into block `b` needs an edge block of its
// own in the host model: `b` is a catch-all-first landing, whose selector
// the edge sets as a jump into it does (any edge into a landing is valid
// LowIR: the course runtime runs the clause on entry).
bool FunctionSelector::eh_entry_edge(uint32_t b) const
{
	return eh_model_ == EhModel::Host && eh_entry_selector(b) != 0;
}

// A branch on `selector == k` (or !=) of the block's selector read
// `selector`: the masks of its true and false edges.
void FunctionSelector::eh_refine(const Instruction& branch, uint32_t selector, uint64_t& yes, uint64_t& no) const
{
	const lowir::FunctionBody& body = *body_;
	const lowir::Value condition = branch.ops[0];
	if (selector == kNoId || condition.kind != ValueKind::Temp || body.temps[condition.index].def == kNoId)
		return;
	const Instruction& compare = body.instructions[body.temps[condition.index].def];
	if (compare.op != Opcode::Compare)
		return;
	const lowir::ComparePredicate predicate = static_cast<lowir::ComparePredicate>(compare.sub);
	if (predicate != lowir::ComparePredicate::Eq && predicate != lowir::ComparePredicate::Ne)
		return;
	const lowir::Value a = compare.ops[0];
	const lowir::Value b = compare.ops[1];
	const lowir::Value constant = a.kind == ValueKind::Temp && a.index == selector
		? b
		: (b.kind == ValueKind::Temp && b.index == selector ? a : lowir::Value::none());
	if (constant.kind != ValueKind::Integer)
		return;
	const int64_t k = static_cast<int64_t>(body.literals[constant.index].low);
	uint64_t equal = yes & selector_bit(k);
	uint64_t unequal = k >= 0 && k <= 62 ? yes & ~selector_bit(k) : yes;
	if (predicate == lowir::ComparePredicate::Ne)
		std::swap(equal, unequal);
	yes = equal;
	no = unequal;
}

// One block's transfer from its entry state and selector mask. The first
// walks propagate both to successors and landings (not along a selector
// branch edge no selector takes: the no-match path after a catch-all);
// the recording walk, over the final states, notes each call's landing
// and each resume's target.
void FunctionSelector::eh_walk(uint32_t b, bool record)
{
	const lowir::FunctionBody& body = *body_;
	std::vector<uint32_t>& work = block_insts_;
	uint32_t node = eh_in_[b];
	uint64_t mask = eh_mask_in_[b];
	uint32_t selector = kNoId;
	bool leading = eh_landing_node_[b] != kNoId;
	for (lowir::InstId i = body.blocks[blocks_[b].id].first; i != kNoId; i = body.instructions[i].next)
	{
		const Instruction& inst = body.instructions[i];
		if (!clause(inst))
			leading = false;
		else if (!leading && inst.op != Opcode::EhCleanup)
		{
			// A clause after a landing's leading ones (inlined terminate
			// forwarding): a catch-all sets the selector no clause set.
			if (inst.op != Opcode::EhCatchAll)
				eh_fail("a typed catch clause outside the leading clauses of a landing");
			eh_mid_clauses_ = true;
			if (record)
				eh_sites_[i] = 1;
			if (mask & 1)
				mask = (mask & ~static_cast<uint64_t>(1)) | selector_bit(inst.immediate);
			selector = kNoId;
		}
		const EhNode current = eh_nodes_[node];
		const bool tail = current.tail_hi != 0;
		// The tail's base: whether its top is an active try record, which
		// paths with no tail record would take for their own.
		const EhNode& base = eh_nodes_[tail ? current.parent : node];
		const bool base_try = (tail ? current.parent : node) != 0 && base.active && !base.cleanup && base.tail_hi == 0;
		switch (inst.op)
		{
		case Opcode::EhTry:
		case Opcode::EhCleanup:
		{
			if (inst.ops[0].kind != ValueKind::Block)
			{
				if (!tail)
				{
					if (base_try)
						node = eh_node(current.parent, current.landing, true, true);
				}
				else if (current.tail_lo == 0 || current.tail_top == kTopMixed)
					eh_fail("a cleanup clause where merged landing records differ");
				else if (current.tail_top == kTopTry)
					node = eh_tail(current.parent, current.tail_lo, current.tail_hi, kTopCleanup);
				break;
			}
			const uint32_t landing = layout_of_[inst.ops[0].index];
			const bool cleanup = inst.op == Opcode::EhCleanup;
			node = eh_node(node, landing, cleanup, false);
			// The propagating walks keep the latest record (states only
			// widen); the recording walk checks every push agrees.
			if (record)
			{
				if (eh_landing_node_[landing] != node)
					eh_fail("a landing installed under different enclosing regions");
				break;
			}
			eh_landing_node_[landing] = node;
			eh_enter(landing, eh_node(eh_nodes_[node].parent, landing, cleanup, true), eh_landing_mask(landing), work);
			break;
		}
		case Opcode::EhEnd:
			if (!tail && node == 0)
				eh_fail("eh_end without an open region of the function");
			if (tail && current.tail_lo == 0)
				eh_fail("eh_end where merged landing records differ");
			node = tail ? eh_tail(current.parent, current.tail_lo - 1u, current.tail_hi - 1u, kTopMixed)
						: current.parent;
			break;
		case Opcode::Exception:
			if (!tail)
			{
				if (base_try)
					node = current.parent;
				break;
			}
			if ((current.tail_lo == 0 && base_try) || current.tail_top == kTopMixed)
				eh_fail("an exception read where merged landing records differ");
			if (current.tail_top == kTopTry)
				node = eh_tail(
					current.parent, current.tail_lo == 0 ? 0 : current.tail_lo - 1u, current.tail_hi - 1u, kTopMixed);
			break;
		case Opcode::ExceptionSelector:
			selector = inst.result;
			break;
		case Opcode::Call:
		{
			if (!record)
				break;
			const lowir::Signature* sig = program_.call_boundary(inst);
			if (sig != nullptr && sig->boundary.unwind == ir_model::CUM_NO)
				break;
			// The landing block (eh_prune makes it the landing's index).
			const uint32_t protect = current.protect;
			eh_sites_[i] = protect == 0 ? 0 : 1 + eh_nodes_[protect].landing;
			break;
		}
		case Opcode::Jump:
			// A jump to a block that only resumes (landings share one) is a
			// resume of the jumping path's own state; a jump into a
			// catch-all-first landing sets the selector no clause set.
			if (!eh_resume_only(layout_of_[inst.ops[0].index]))
			{
				const uint32_t target = layout_of_[inst.ops[0].index];
				if (!record)
					eh_enter(target, node, eh_edge_mask(target, mask), work);
				break;
			}
			if (record && current.protect == 0)
				eh_sites_[i] = kResumeTerminal;
			// fall through
		case Opcode::Resume:
		{
			const uint32_t protect = current.protect;
			if (protect == 0)
				return;
			const EhNode target = eh_nodes_[protect];
			// Records a running landing opened and left installed (no active
			// record on top): the chain the personality matched for that
			// landing included none of them, so none may catch. A target
			// below the running landing's own record is in its chain (or in
			// the chain of the landing whose resume entered it, below which
			// the target is too).
			if (protect == node)
				for (uint32_t k = node; k != 0 && !eh_nodes_[k].active; k = eh_nodes_[k].parent)
					if (eh_nodes_[k].inside && (eh_landing_mask(eh_nodes_[k].landing) & ~static_cast<uint64_t>(1)) != 0)
						eh_fail("resume into a catching region opened while handling the exception");
			if (record)
				eh_sites_[i] = 1 + target.landing;
			else
				eh_enter(target.landing, eh_node(target.parent, target.landing, target.cleanup, true),
					eh_landing_mask(target.landing), work);
			return;
		}
		case Opcode::Throw:
			eh_fail("a throw instruction (host objects throw through the eh_throw runtime role)");
		case Opcode::Branch:
		{
			if (record)
				break;
			uint64_t yes = mask;
			uint64_t no = mask;
			eh_refine(inst, selector, yes, no);
			const uint32_t on_true = layout_of_[inst.ops[1].index];
			const uint32_t on_false = layout_of_[inst.ops[2].index];
			if (yes != 0)
				eh_enter(on_true, node, eh_edge_mask(on_true, yes), work);
			if (no != 0)
				eh_enter(on_false, node, eh_edge_mask(on_false, no), work);
			break;
		}
		case Opcode::Switch:
			if (!record)
				for (uint32_t k = succ_first_[b]; k < succ_first_[b + 1]; ++k)
					eh_enter(succs_[k], node, eh_edge_mask(succs_[k], mask), work);
			break;
		default:
			break;
		}
	}
}

// Whether block `b` holds only a resume.
bool FunctionSelector::eh_resume_only(uint32_t b) const
{
	const lowir::Block& block = body_->blocks[blocks_[b].id];
	return block.first != kNoId && block.first == block.last && body_->instructions[block.first].op == Opcode::Resume;
}

void FunctionSelector::eh_fail(const char* what) const
{
	throw BackendError(
		std::string("host EH: ") + what + " (in @" + program_.spelling(program_.symbols[symbol_].name) + ")");
}

void FunctionSelector::analyze_host_eh()
{
	const lowir::FunctionBody& body = *body_;
	const uint32_t n = static_cast<uint32_t>(blocks_.size());
	eh_sites_.assign(body.instructions.size(), 0);
	eh_landing_node_.assign(n, kNoId);
	eh_landing_index_.assign(n, kNoId);
	pad_mir_.assign(n, kNoId);
	select_mir_.assign(n, kNoId);
	eh_resumed_.assign(n, 0);
	bool any = false;
	bool reads = false;
	for_each_inst([&](uint32_t id, uint32_t) {
		const Opcode op = body.instructions[id].op;
		const bool read = op == Opcode::Exception || op == Opcode::ExceptionSelector || op == Opcode::Resume;
		any = any || read || (op >= Opcode::EhTry && op <= Opcode::EhEnd) || op == Opcode::Throw;
		reads = reads || read;
	});
	if (!any)
		return;
	eh_nodes_.clear();
	eh_node_index_.clear(n);
	EhNode empty;
	empty.parent = 0;
	empty.landing = kNoId;
	empty.cleanup = false;
	empty.active = false;
	empty.inside = false;
	empty.depth = 0;
	empty.tail_lo = empty.tail_hi = 0;
	empty.tail_top = kTopNone;
	empty.protect = 0;
	empty.chain = 0;
	empty.chain_count = 0;
	eh_nodes_.push_back(empty);
	eh_in_.assign(n, kNoId);
	eh_mask_in_.assign(n, 0);
	eh_mid_clauses_ = false;
	// Entry states to a fixed point: a block is walked again when a merge
	// widens its state (a lower base, a wider tail, more selectors), a
	// bounded number of times. Blocks are taken in reverse postorder, so a
	// join has merged what its forward predecessors bring before the blocks
	// after it are walked (only back edges and resumes widen a walked
	// block's state); a small function's blocks simply last in, first out
	// (their walks again are as few as the order would cost).
	eh_in_[0] = 0;
	eh_mask_in_[0] = ~static_cast<uint64_t>(0);
	if (n <= kSmallFunctionBlocks)
	{
		std::vector<uint32_t>& work = eh_work_list_;
		work.assign(1, 0);
		while (!work.empty())
		{
			const uint32_t b = work.back();
			work.pop_back();
			block_insts_.clear();
			eh_walk(b, false);
			++stats_.eh_walks;
			for (size_t k = 0; k < block_insts_.size(); ++k)
				work.push_back(block_insts_[k]);
		}
	}
	else
	{
		eh_order();
		std::vector<uint64_t>& heap = eh_heap_;
		std::vector<uint8_t>& listed = eh_listed_;
		listed.assign(n, 0);
		heap.assign(1, eh_rank_[0]);
		listed[0] = 1;
		while (!heap.empty())
		{
			std::pop_heap(heap.begin(), heap.end(), std::greater<uint64_t>());
			const uint32_t b = static_cast<uint32_t>(heap.back());
			heap.pop_back();
			listed[b] = 0;
			block_insts_.clear();
			eh_walk(b, false);
			++stats_.eh_walks;
			for (size_t k = 0; k < block_insts_.size(); ++k)
			{
				const uint32_t s = block_insts_[k];
				if (listed[s])
					continue;
				listed[s] = 1;
				heap.push_back(eh_rank_[s]);
				std::push_heap(heap.begin(), heap.end(), std::greater<uint64_t>());
			}
		}
	}
	stats_.eh_states += eh_nodes_.size();
	for (uint32_t b = 0; b < n; ++b)
		if (blocks_[b].reachable && eh_in_[b] != kNoId)
			eh_walk(b, true);
	uint32_t landings = 0;
	eh_prune(landings, reads);
	if (landings == 0 && !reads)
		return;
	if (landings == 0)
		eh_fail("an exception read or resume outside every landing");
	eh_build_tables(landings);
}

// Each block's walking priority (eh_rank_: its reverse postorder number
// over the successor and landing edges in the high half, the block in the
// low half; blocks the entry does not reach last).
void FunctionSelector::eh_order()
{
	const uint32_t n = static_cast<uint32_t>(blocks_.size());
	// The landing edges (each eh_try/eh_cleanup ^L) build_cfg found.
	const std::vector<uint32_t>& first = handler_first_;
	const std::vector<uint32_t>& edges = handlers_;
	// An iterative depth-first walk; `next` is each open block's next edge
	// (successors, then landing edges).
	std::vector<uint64_t>& rank = eh_rank_;
	rank.assign(n, 0);
	std::vector<uint32_t>& next = eh_dfs_next_;
	next.assign(n, 0);
	std::vector<uint8_t>& seen = eh_listed_;
	seen.assign(n, 0);
	std::vector<uint32_t>& stack = eh_work_list_;
	stack.assign(1, 0);
	seen[0] = 1;
	uint32_t finished = 0;
	while (!stack.empty())
	{
		const uint32_t b = stack.back();
		const uint32_t succs = succ_first_[b + 1] - succ_first_[b];
		const uint32_t k = next[b]++;
		if (k < succs + (first[b + 1] - first[b]))
		{
			const uint32_t s = k < succs ? succs_[succ_first_[b] + k] : edges[first[b] + (k - succs)];
			if (!seen[s])
			{
				seen[s] = 1;
				stack.push_back(s);
			}
			continue;
		}
		stack.pop_back();
		rank[b] = static_cast<uint64_t>(n - 1 - finished++) << 32 | b;
	}
	for (uint32_t b = 0; b < n; ++b)
		if (!seen[b])
			rank[b] = static_cast<uint64_t>(n) << 32 | b;
}

// The blocks control reaches in the host model -- from the entry along
// successor edges, to each potentially throwing call's landing (the
// personality enters a pad only from an LSDA call site) and to each
// resume's local target -- and the landings that keep a table entry: a
// reached call names them (they keep a pad) or a reached resume enters
// them. A landing of a region without a potentially throwing call is
// never entered: neither it nor the blocks only it reaches are selected.
// Numbers those landings and makes each call's site its landing's index;
// whether a reached block reads the exception.
void FunctionSelector::eh_prune(uint32_t& landings, bool& reads)
{
	const lowir::FunctionBody& body = *body_;
	const uint32_t n = static_cast<uint32_t>(blocks_.size());
	std::vector<uint8_t>& live = eh_live_;
	live.assign(n, 0);
	eh_resumed_.assign(n, 0);
	std::vector<uint32_t>& work = eh_work_list_;
	work.assign(1, 0);
	live[0] = 1;
	reads = false;
	auto reach = [&](uint32_t s, uint8_t how) {
		if (live[s] == 0)
			work.push_back(s);
		if (live[s] < how)
			live[s] = how;
	};
	while (!work.empty())
	{
		const uint32_t b = work.back();
		work.pop_back();
		for (uint32_t k = succ_first_[b]; k < succ_first_[b + 1]; ++k)
			reach(succs_[k], 1);
		for (lowir::InstId i = body.blocks[blocks_[b].id].first; i != kNoId; i = body.instructions[i].next)
		{
			const uint32_t site = eh_sites_[i];
			switch (body.instructions[i].op)
			{
			case Opcode::Exception:
			case Opcode::ExceptionSelector:
				reads = true;
				break;
			case Opcode::Call:
				if (site != 0)
					reach(site - 1, 2);
				break;
			case Opcode::Resume:
				reads = true;
				// fall through
			case Opcode::Jump:
				if (site != 0 && site != kResumeTerminal)
				{
					eh_resumed_[site - 1] = 1;
					reach(site - 1, 1);
				}
				break;
			default:
				break;
			}
		}
	}
	bool pruned = false;
	for (uint32_t b = 0; b < n; ++b)
	{
		if (live[b] != 0 || !blocks_[b].reachable)
			continue;
		blocks_[b].reachable = false;
		pruned = true;
		for (lowir::InstId i = body.blocks[blocks_[b].id].first; i != kNoId; i = body.instructions[i].next)
			marks_[i] &= ~IM_REACHABLE;
	}
	if (pruned)
		reachable_preds();
	for (uint32_t b = 0; b < n; ++b)
		if (eh_landing_node_[b] != kNoId && eh_in_[b] != kNoId && (live[b] == 2 || eh_resumed_[b] != 0))
			eh_landing_index_[b] = landings++;
	for (uint32_t b = 0; b < n; ++b)
		if (live[b] != 0)
			for (lowir::InstId i = body.blocks[blocks_[b].id].first; i != kNoId; i = body.instructions[i].next)
				if (body.instructions[i].op == Opcode::Call && eh_sites_[i] != 0)
					eh_sites_[i] = 1 + eh_landing_index_[eh_sites_[i] - 1];
}

} // namespace native
