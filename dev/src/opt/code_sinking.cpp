#include "opt/integer_patterns.h"
#include "opt/loops.h"
#include "opt/passes.h"

#include <algorithm>
#include <vector>

namespace opt {

using namespace lowir;

namespace {

// The most dominator-tree steps one question may walk (past it, the value stays).
constexpr unsigned kWalk = 128;

// Pure instructions move down to the block nearest their readers that lies
// in no deeper loop, so a path that reads nothing computes nothing and a
// step meets the test of what it computes. Blocks are visited after every
// block they dominate, so a reader's move is known when its operands are
// placed: one walk moves a whole chain.
class CodeSinking
{
public:
	explicit CodeSinking(FunctionContext& context) : c_(context), body_(context.body), cfg_(context.cfg) {}

	bool run();

private:
	struct Move
	{
		InstId inst;
		BlockId to;
		uint32_t position;
	};

	BlockId common(BlockId a, BlockId b) const;
	void read(Value v, BlockId b);
	uint32_t depth(BlockId b) const;
	BlockId place(BlockId from, BlockId lowest) const;
	void settle(BlockId b);
	void unlink_moved();
	void insert(size_t first, size_t last);

	FunctionContext& c_;
	FunctionBody& body_;
	Cfg& cfg_;
	LoopForest forest_;
	std::vector<uint32_t> loop_depth_;
	std::vector<BlockId> lowest_; // per temp: the nearest block every reader lies under
	std::vector<Move> moves_;
	std::vector<uint8_t> moved_;       // per instruction
	std::vector<uint32_t> first_read_; // per temp, in the target at hand
	std::vector<InstId> order_;
};

// (`kNoId` when the walk is too long, which pins the value.)
BlockId CodeSinking::common(BlockId a, BlockId b) const
{
	for (unsigned step = 0; step < kWalk; ++step)
	{
		if (cfg_.dominates(a, b))
			return a;
		if (cfg_.dominates(b, a))
			return b;
		if (cfg_.rpo_index[a] > cfg_.rpo_index[b])
			a = cfg_.idom[a];
		else
			b = cfg_.idom[b];
	}
	return kNoId;
}

void CodeSinking::read(Value v, BlockId b)
{
	if (v.kind != ValueKind::Temp)
		return;
	BlockId& lowest = lowest_[v.index];
	lowest = lowest == kNoId ? b : common(lowest, b);
	if (lowest == kNoId)
		lowest = cfg_.rpo[0];
}

uint32_t CodeSinking::depth(BlockId b) const
{
	return forest_.innermost[b] == kNoId ? 0 : loop_depth_[forest_.innermost[b]];
}

BlockId CodeSinking::place(BlockId from, BlockId lowest) const
{
	if (!cfg_.dominates(from, lowest))
		return from;
	BlockId best = from;
	uint32_t best_depth = depth(from);
	bool found = false;
	unsigned step = 0;
	for (BlockId x = lowest; x != from; x = cfg_.idom[x])
	{
		if (++step > kWalk)
			return from;
		if ((cfg_.flags[x] & BF_HANDLER) != 0 || forest_.dropped[x] != 0)
			continue;
		if (found ? depth(x) < best_depth : depth(x) <= best_depth)
		{
			best = x;
			best_depth = depth(x);
			found = true;
		}
	}
	return best;
}

void CodeSinking::settle(BlockId b)
{
	order_.clear();
	for (InstId i = body_.blocks[b].first; i != kNoId; i = body_.instructions[i].next)
		order_.push_back(i);
	for (size_t k = order_.size(); k-- > 0;)
	{
		const Instruction& inst = body_.instructions[order_[k]];
		if (inst.op == Opcode::Phi)
			continue;
		BlockId at = b;
		if (is_pure(inst) && inst.result != kNoId && lowest_[inst.result] != kNoId && lowest_[inst.result] != b)
			at = place(b, lowest_[inst.result]);
		if (at != b)
		{
			moves_.push_back(Move{ order_[k], at, 0 });
			moved_[order_[k]] = 1;
		}
		for_each_operand(body_, inst, [&](const Value& v) { read(v, at); });
	}
}

void CodeSinking::unlink_moved()
{
	for (BlockId b : cfg_.rpo)
	{
		InstId prev = kNoId;
		for (InstId i = body_.blocks[b].first; i != kNoId;)
		{
			const InstId next = body_.instructions[i].next;
			if (moved_[i] != 0)
				unlink_after(body_, b, prev, i);
			else
				prev = i;
			i = next;
		}
	}
}

// moves_[first, last) enter one block, each before its first reader there
// (the end, for a phi's read): last-placed readers come first in the list.
void CodeSinking::insert(size_t first, size_t last)
{
	const BlockId to = moves_[first].to;
	order_.clear();
	for (InstId i = body_.blocks[to].first; i != kNoId; i = body_.instructions[i].next)
		order_.push_back(i);
	const uint32_t end = static_cast<uint32_t>(order_.size() - 1);
	for (size_t m = first; m < last; ++m)
		first_read_[body_.instructions[moves_[m].inst].result] = end;
	for (uint32_t k = end + 1; k-- > 0;)
	{
		const Instruction& inst = body_.instructions[order_[k]];
		if (inst.op == Opcode::Phi)
			continue;
		for_each_operand(body_, inst, [&](const Value& v) {
			if (v.kind == ValueKind::Temp && first_read_[v.index] != kNoId)
				first_read_[v.index] = k;
		});
	}
	// (A moved reader's own place bounds the values it reads.)
	for (size_t m = first; m < last; ++m)
	{
		const Instruction& inst = body_.instructions[moves_[m].inst];
		const uint32_t position = first_read_[inst.result];
		moves_[m].position = position;
		for_each_operand(body_, inst, [&](const Value& v) {
			if (v.kind == ValueKind::Temp && first_read_[v.index] != kNoId)
				first_read_[v.index] = std::min(first_read_[v.index], position);
		});
	}
	// Each goes right after what stood before its position: the moves are
	// in reverse program order, so the earliest ends up first of those there.
	for (size_t m = first; m < last; ++m)
	{
		const uint32_t position = moves_[m].position;
		insert_after(body_, to, position == 0 ? kNoId : order_[position - 1], moves_[m].inst);
		first_read_[body_.instructions[moves_[m].inst].result] = kNoId;
	}
}

bool CodeSinking::run()
{
	if (body_.layout.size() == 1)
		return false;
	c_.ensure_cfg();
	if (cfg_.rpo.size() != body_.layout.size())
		return false;
	find_loops(body_, cfg_, forest_);
	if (forest_.partial)
		return false;
	loop_depth_.assign(forest_.loops.size(), 1);
	for (uint32_t l = 0; l < forest_.loops.size(); ++l)
		if (forest_.loops[l].parent != kNoId)
			loop_depth_[l] = loop_depth_[forest_.loops[l].parent] + 1;
	lowest_.assign(body_.temps.size(), kNoId);
	moved_.assign(body_.instructions.size(), 0);
	moves_.clear();
	for (BlockId b : cfg_.rpo)
		for (InstId i = body_.blocks[b].first; i != kNoId; i = body_.instructions[i].next)
		{
			const Instruction& inst = body_.instructions[i];
			if (inst.op != Opcode::Phi)
				break;
			for (uint32_t k = 0; k + 1 < inst.extra_count; k += 2)
				read(body_.operands[inst.extra_first + k + 1], body_.operands[inst.extra_first + k].index);
		}
	for (size_t k = cfg_.rpo.size(); k-- > 0;)
		settle(cfg_.rpo[k]);
	if (moves_.empty())
		return false;
	unlink_moved();
	first_read_.assign(body_.temps.size(), kNoId);
	std::stable_sort(moves_.begin(), moves_.end(), [](const Move& a, const Move& b) { return a.to < b.to; });
	for (size_t first = 0; first < moves_.size();)
	{
		size_t last = first + 1;
		while (last < moves_.size() && moves_[last].to == moves_[first].to)
			++last;
		insert(first, last);
		first = last;
	}
	// (A signed division by a power of two that now stands under a test of
	// its operand's sign is a shift.)
	for (const Move& move : moves_)
		unsign_tested_division(c_, move.to, body_.instructions[move.inst]);
	c_.invalidate_block_of();
	restore_definition_order(c_);
	return true;
}

// A scalar integer computation with no effect or trap. Consuming a dying
// loaded operand early replaces its live range by the result's, and may let
// the next computation consume two such ranges. Counters stay where the
// loop passes put them, beside their tests.
bool integer_step(const Instruction& inst)
{
	if ((!inst.type.is_integer() && !inst.type.is_pointer()) || storage_bytes(inst.type) > 8)
		return false;
	if (inst.op == Opcode::Binary)
	{
		const BinaryOp op = static_cast<BinaryOp>(inst.sub);
		return op != BinaryOp::Div && op != BinaryOp::Mod && op != BinaryOp::UDiv && op != BinaryOp::UMod;
	}
	if (inst.op == Opcode::Unary)
		return true;
	if (inst.op != Opcode::Convert)
		return false;
	const Type from = convert_source_type(inst);
	return (from.is_integer() || from.is_pointer()) && storage_bytes(from) <= 8;
}

class IntegerSchedule
{
public:
	explicit IntegerSchedule(FunctionContext& context)
		: body_(context.body), uses_(context.entry_uses(schedule_integer_values)),
		  block_(context.entry_block_of(schedule_integer_values)), anchor_(body_.instructions.size(), 0),
		  loaded_(body_.temps.size(), 0)
	{}

	bool run();

private:
	bool loaded(Value v, BlockId b) const;
	bool dying(const Instruction& inst, BlockId b) const;
	bool order(BlockId b);
	FunctionBody& body_;
	const std::vector<uint32_t>& uses_;
	const std::vector<BlockId>& block_;
	std::vector<uint32_t> anchor_, first_, last_, next_;
	std::vector<uint8_t> loaded_;
	std::vector<InstId> seq_, out_;
};

bool IntegerSchedule::loaded(Value v, BlockId b) const
{
	if (!v.is(ValueKind::Temp) || !loaded_[v.index])
		return false;
	const InstId def = body_.temps[v.index].def;
	return def != kNoId && block_[def] == b;
}

bool IntegerSchedule::dying(const Instruction& inst, BlockId b) const
{
	for (Value v : inst.ops)
		if (loaded(v, b))
		{
			uint32_t here = 0;
			for (Value w : inst.ops)
				here += v == w ? 1 : 0;
			if (uses_[v.index] == here)
				return true;
		}
	return false;
}

// Bucket each instruction after its latest operand or barrier, retaining
// original order within a bucket. One forward walk computes every anchor;
// dependencies already assigned to the same bucket precede their readers.
bool IntegerSchedule::order(BlockId b)
{
	seq_.clear();
	for (InstId i = body_.blocks[b].first; i != kNoId; i = body_.instructions[i].next)
		seq_.push_back(i);
	if (seq_.size() < 3)
		return false;
	first_.assign(seq_.size(), kNoId);
	last_.assign(seq_.size(), kNoId);
	next_.assign(seq_.size(), kNoId);
	uint32_t barrier = 0;
	for (uint32_t p = 0; p < seq_.size(); ++p)
	{
		const InstId i = seq_[p];
		const Instruction& inst = body_.instructions[i];
		const bool integer = integer_step(inst);
		if (inst.result != kNoId)
		{
			bool from_load = inst.op == Opcode::Load && (inst.flags & IF_VOLATILE) == 0 &&
				(inst.type.is_integer() || inst.type.is_pointer()) && storage_bytes(inst.type) <= 8;
			for (Value v : inst.ops)
				from_load = from_load || (integer && loaded(v, b));
			loaded_[inst.result] = from_load ? 1 : 0;
		}
		if (inst.op == Opcode::Phi ||
			(!(inst.op == Opcode::Load && (inst.flags & IF_VOLATILE) == 0) &&
				(!is_pure(inst) || (inst.op == Opcode::Binary && inst.type.is_integer() && !integer))))
			barrier = p;
		uint32_t at = p;
		if (integer && dying(inst, b))
		{
			at = barrier;
			for_each_operand(body_, inst, [&](const Value& v) {
				if (!v.is(ValueKind::Temp))
					return;
				const InstId def = body_.temps[v.index].def;
				if (def != kNoId && block_[def] == b)
					at = std::max(at, anchor_[def]);
			});
		}
		anchor_[i] = at;
		if (first_[at] == kNoId)
			first_[at] = p;
		else
			next_[last_[at]] = p;
		last_[at] = p;
	}
	out_.clear();
	for (uint32_t p : first_)
		for (; p != kNoId; p = next_[p])
			out_.push_back(seq_[p]);
	if (out_ == seq_)
		return false;
	for (size_t k = 0; k < out_.size(); ++k)
		body_.instructions[out_[k]].next = k + 1 == out_.size() ? kNoId : out_[k + 1];
	body_.blocks[b].first = out_.front();
	body_.blocks[b].last = out_.back();
	return true;
}

bool IntegerSchedule::run()
{
	bool changed = false;
	for (BlockId b : body_.layout)
		changed = order(b) || changed;
	return changed;
}

// The most blocks walked from a step's block, and instructions read in
// them, for a later read of its counter; the most phi inputs of a function
// read for the edges that read it.
constexpr unsigned kStepWalk = 32;
constexpr unsigned kStepScan = 256;
constexpr size_t kStepPhiReads = 1u << 16;

// O2+, late: a counter's step stands after the last read of the counter.
// `y = x + literal` moves down its block, to before its own first reader
// there or the test that ends the block, when x is read between the two
// places, itself or as an address formed from it by literal offsets, and
// is live nowhere after the block: x's register then takes y, and no copy
// carries y into x's phi.
class StepPlacement
{
public:
	explicit StepPlacement(FunctionContext& context) : c_(context), body_(context.body) {}

	bool run();

private:
	// What the block at hand reads of one temp: its reads there, those that
	// form an offset or an address from it, the value it is formed from
	// (plus one; zero: none), the positions (plus one) of its last read, or
	// its parts', and its first, and the steps taken from it.
	struct Reads
	{
		uint32_t count, parts, root, last, first, steps;
	};
	// The operand an instruction forms an offset or an address from (none:
	// it forms neither).
	static Value formed_from(const Instruction& inst)
	{
		if (inst.op == Opcode::Binary)
			return stepped(inst);
		if (inst.op != Opcode::Index)
			return Value::none();
		if (inst.ops[1].is(ValueKind::Temp))
			return inst.ops[1];
		return inst.ops[0].is(ValueKind::Temp) && inst.ops[1].is(ValueKind::Integer) ? inst.ops[0] : Value::none();
	}
	// The temp a binary instruction adds a literal to or takes one from.
	static Value stepped(const Instruction& inst)
	{
		return inst.type.is_integer() && storage_bytes(inst.type) <= 8 &&
				(static_cast<BinaryOp>(inst.sub) == BinaryOp::Add ||
					static_cast<BinaryOp>(inst.sub) == BinaryOp::Sub) &&
				inst.ops[0].is(ValueKind::Temp) && inst.ops[1].is(ValueKind::Integer)
			? inst.ops[0]
			: Value::none();
	}
	// Whether the block's accesses hold `inst`'s result in their addresses:
	// an offset every reader forms an address from, or an address, read in
	// this block alone.
	bool folded(const Instruction& inst) const
	{
		const Reads& r = at_[inst.result];
		return r.count == (*uses_)[inst.result] && (inst.op == Opcode::Index || r.parts == r.count);
	}
	void touch(TempId t)
	{
		if (at_[t].count == 0 && at_[t].root == 0 && at_[t].last == 0)
			touched_.push_back(t);
	}
	bool dies_in(TempId x, BlockId b);
	bool place(BlockId b);

	FunctionContext& c_;
	FunctionBody& body_;
	// The entry analyses, read at the first block that has a step (the
	// moves change neither: an instruction keeps its block).
	const std::vector<uint32_t>* uses_ = nullptr;
	const std::vector<BlockId>* block_ = nullptr;
	std::vector<Reads> at_;
	std::vector<TempId> touched_;
	std::vector<InstId> seq_, out_;
	std::vector<std::pair<uint32_t, uint32_t>> moves_; // (the position to stand before, the step's position)
	std::vector<uint8_t> moved_;
	// Every phi input of the function that is a temp, with its edge's source.
	std::vector<std::pair<TempId, BlockId>> phi_reads_;
	bool phi_reads_built_ = false, phi_reads_whole_ = false;
	std::vector<uint32_t> seen_;
	uint32_t stamp_ = 0;
	std::vector<BlockId> work_;
};

// Whether x is live on no path out of b: no block that a path from b reaches
// before it enters x's block again reads x, and no phi reads it on an edge
// from b or from such a block.
bool StepPlacement::dies_in(TempId x, BlockId b)
{
	if ((*uses_)[x] == at_[x].count)
		return true;
	if (!phi_reads_built_)
	{
		phi_reads_built_ = true;
		for (BlockId block : body_.layout)
			for (InstId i = body_.blocks[block].first; i != kNoId && body_.instructions[i].op == Opcode::Phi;
				i = body_.instructions[i].next)
			{
				const Instruction& phi = body_.instructions[i];
				for (uint32_t k = 0; k + 1 < phi.extra_count && phi_reads_.size() < kStepPhiReads; k += 2)
				{
					const Value v = body_.operands[phi.extra_first + k + 1];
					if (v.is(ValueKind::Temp))
						phi_reads_.push_back(std::make_pair(v.index, body_.operands[phi.extra_first + k].index));
				}
			}
		phi_reads_whole_ = phi_reads_.size() < kStepPhiReads;
		std::sort(phi_reads_.begin(), phi_reads_.end());
	}
	const InstId def = x < body_.parameter_count ? kNoId : body_.temps[x].def;
	if (!phi_reads_whole_ || def == kNoId)
		return false;
	if (block_ == nullptr)
		block_ = &c_.entry_block_of(place_steps);
	const Cfg& cfg = c_.ensure_cfg_light();
	const BlockId home = (*block_)[def];
	if (seen_.size() < body_.blocks.size())
		seen_.assign(body_.blocks.size(), 0);
	++stamp_;
	work_.clear();
	const auto enter = [&](BlockId from) {
		for (uint32_t s = cfg.succ_first[from]; s < cfg.succ_first[from + 1]; ++s)
			if (cfg.succs[s] != home && seen_[cfg.succs[s]] != stamp_)
			{
				seen_[cfg.succs[s]] = stamp_;
				work_.push_back(cfg.succs[s]);
			}
	};
	enter(b);
	unsigned walked = 0, scanned = 0;
	while (!work_.empty())
	{
		const BlockId next = work_.back();
		work_.pop_back();
		if (++walked > kStepWalk)
			return false;
		for (InstId i = body_.blocks[next].first; i != kNoId; i = body_.instructions[i].next)
		{
			const Instruction& inst = body_.instructions[i];
			if (inst.op == Opcode::Phi)
				continue;
			bool reads = ++scanned > kStepScan;
			for_each_operand(
				body_, inst, [&](const Value& v) { reads = reads || (v.is(ValueKind::Temp) && v.index == x); });
			if (reads)
				return false;
		}
		enter(next);
	}
	for (auto e = std::lower_bound(phi_reads_.begin(), phi_reads_.end(), std::make_pair(x, static_cast<BlockId>(0)));
		e != phi_reads_.end() && e->first == x; ++e)
		if (e->second == b || seen_[e->second] == stamp_)
			return false;
	return true;
}

// Places the steps of block `b`, whose instructions after its phis `seq_`
// holds.
bool StepPlacement::place(BlockId b)
{
	if (uses_ == nullptr)
	{
		uses_ = &c_.entry_uses(place_steps);
		at_.resize(body_.temps.size());
	}
	for (TempId t : touched_)
		at_[t] = Reads();
	touched_.clear();
	for (uint32_t p = 0; p < seq_.size(); ++p)
	{
		const Instruction& inst = body_.instructions[seq_[p]];
		const Value from = formed_from(inst);
		for_each_operand(body_, inst, [&](const Value& v) {
			if (!v.is(ValueKind::Temp))
				return;
			touch(v.index);
			Reads& r = at_[v.index];
			++r.count;
			if (r.first == 0)
				r.first = p + 1;
			if (v == from)
				++r.parts;
		});
	}
	// Each value's last read, a folded offset's and address's reads among them.
	for (uint32_t p = 0; p < seq_.size(); ++p)
	{
		const Instruction& inst = body_.instructions[seq_[p]];
		const Value from = formed_from(inst);
		const bool holds = !from.is(ValueKind::None) && inst.result != kNoId && folded(inst);
		for_each_operand(body_, inst, [&](const Value& v) {
			if (!v.is(ValueKind::Temp) || (holds && v == from))
				return;
			at_[at_[v.index].root != 0 ? at_[v.index].root - 1 : v.index].last = p + 1;
		});
		if (holds)
		{
			touch(inst.result);
			at_[inst.result].root = at_[from.index].root != 0 ? at_[from.index].root : from.index + 1;
		}
		else if (inst.op == Opcode::Binary && !from.is(ValueKind::None))
			++at_[from.index].steps;
	}
	// The place a step may move down to: the test that ends the block.
	uint32_t end = static_cast<uint32_t>(seq_.size()) - 1;
	const Instruction& term = body_.instructions[seq_[end]];
	if (term.op == Opcode::Branch && term.ops[0].is(ValueKind::Temp) &&
		body_.instructions[seq_[end - 1]].result == term.ops[0].index &&
		body_.instructions[seq_[end - 1]].op == Opcode::Compare)
		--end;
	moves_.clear();
	for (uint32_t p = 0; p + 1 < end; ++p)
	{
		const Instruction& inst = body_.instructions[seq_[p]];
		if (inst.op != Opcode::Binary || inst.result == kNoId || stepped(inst).is(ValueKind::None) || folded(inst))
			continue;
		const Reads& x = at_[inst.ops[0].index];
		const uint32_t to = at_[inst.result].first != 0 ? at_[inst.result].first - 1 : end;
		// (Of two steps of one counter only one can take its register: they
		// stay as the loop passes left them.)
		if (x.root != 0 || x.steps != 1 || x.last <= p + 1 || x.last > to || to > end || !dies_in(inst.ops[0].index, b))
			continue;
		moves_.push_back(std::make_pair(to, p));
	}
	if (moves_.empty())
		return false;
	std::sort(moves_.begin(), moves_.end());
	moved_.assign(seq_.size(), 0);
	for (const std::pair<uint32_t, uint32_t>& move : moves_)
		moved_[move.second] = 1;
	out_.clear();
	size_t m = 0;
	for (uint32_t p = 0; p < seq_.size(); ++p)
	{
		for (; m < moves_.size() && moves_[m].first == p; ++m)
			out_.push_back(seq_[moves_[m].second]);
		if (!moved_[p])
			out_.push_back(seq_[p]);
	}
	// (The block's phis keep their places before the rest.)
	InstId prev = kNoId;
	for (InstId i = body_.blocks[b].first; i != kNoId && body_.instructions[i].op == Opcode::Phi;
		i = body_.instructions[i].next)
		prev = i;
	for (InstId i : out_)
	{
		if (prev == kNoId)
			body_.blocks[b].first = i;
		else
			body_.instructions[prev].next = i;
		prev = i;
	}
	body_.instructions[prev].next = kNoId;
	body_.blocks[b].last = prev;
	return true;
}

bool StepPlacement::run()
{
	bool changed = false;
	for (BlockId b : body_.layout)
	{
		// (Most blocks have no step: they are read this once.)
		seq_.clear();
		bool any = false;
		for (InstId i = body_.blocks[b].first; i != kNoId; i = body_.instructions[i].next)
		{
			const Instruction& inst = body_.instructions[i];
			if (inst.op == Opcode::Phi)
				continue;
			any = any || (inst.op == Opcode::Binary && !stepped(inst).is(ValueKind::None));
			seq_.push_back(i);
		}
		if (any && seq_.size() >= 3)
			changed = place(b) || changed;
	}
	return changed;
}

} // namespace

bool sink_values(FunctionContext& context)
{
	CodeSinking pass(context);
	return pass.run();
}

bool schedule_integer_values(FunctionContext& context)
{
	if (context.body.temps.size() < 3)
		return false;
	IntegerSchedule pass(context);
	return pass.run();
}

bool place_steps(FunctionContext& context)
{
	if (context.body.temps.size() < 3)
		return false;
	StepPlacement pass(context);
	return pass.run();
}

} // namespace opt
