#include "opt/passes.h"
#include "support/flat_map.h"

namespace opt {

using namespace lowir;

bool can_unwind(const FunctionContext& context, const Instruction& inst)
{
	if (!may_unwind(context.program, inst))
		return false;
	// A direct callee this traversal proved nonthrowing.
	return !(inst.op == Opcode::Call && inst.ops[0].is(ValueKind::Symbol) && context.nounwind != nullptr &&
		inst.ops[0].index < context.nounwind->size() && (*context.nounwind)[inst.ops[0].index] != 0);
}

namespace {

// A persistent stack of active regions: node 0 is empty; node k > 0 is
// region `region` on top of node `below`.
struct StackNode
{
	uint32_t region;
	uint32_t below;
};

class RegionStripper
{
public:
	explicit RegionStripper(FunctionContext& context) : c_(context), body_(context.body) {}

	bool run();

private:
	uint32_t push(uint32_t stack, uint32_t region);
	bool propagate();
	bool empty_cleanup(InstId opener) const;

	FunctionContext& c_;
	FunctionBody& body_;
	std::vector<StackNode> nodes_;
	struct KeyHash
	{
		size_t operator()(uint64_t k) const { return static_cast<size_t>(k * 0x9e3779b97f4a7c15ull); }
	};
	FlatMap<uint64_t, uint32_t, KeyHash> interned_; // (region, below) -> node
	std::vector<uint8_t> marked_;                   // per node: every region of its stack is unsafe
	std::vector<InstId> opener_;                    // per region: its eh_try/eh_cleanup
	std::vector<uint32_t> region_of_;               // per instruction: region it opens
	std::vector<uint32_t> entry_;                   // per block: stack at entry (kNoId unvisited)
	std::vector<uint8_t> normal_;                   // per block: reachable over ordinary edges
	std::vector<uint8_t> unsafe_;                   // per region
	std::vector<uint32_t> popped_;                  // per eh_end in normal code: region it pops
};

uint32_t RegionStripper::push(uint32_t stack, uint32_t region)
{
	const uint64_t key = (static_cast<uint64_t>(region) << 32) | stack;
	if (const uint32_t* found = interned_.find(key))
		return *found;
	nodes_.push_back(StackNode{ region, stack });
	marked_.push_back(0);
	const uint32_t node = static_cast<uint32_t>(nodes_.size() - 1);
	interned_.insert(key, node);
	return node;
}

// The active-region stack over ordinary edges from the entry. False when
// two paths reach a block with different stacks or a pop has nothing to
// pop: the function keeps all its regions.
bool RegionStripper::propagate()
{
	Cfg& cfg = c_.cfg;
	entry_.assign(cfg.block_count, kNoId);
	normal_.assign(cfg.block_count, 0);
	popped_.assign(body_.instructions.size(), kNoId);
	std::vector<BlockId> work(1, body_.layout[0]);
	entry_[body_.layout[0]] = 0;
	normal_[body_.layout[0]] = 1;
	while (!work.empty())
	{
		const BlockId b = work.back();
		work.pop_back();
		uint32_t stack = entry_[b];
		for (InstId i = body_.blocks[b].first; i != kNoId; i = body_.instructions[i].next)
		{
			const Instruction& inst = body_.instructions[i];
			if ((inst.op == Opcode::EhTry || inst.op == Opcode::EhCleanup) && region_of_[i] != kNoId)
				stack = push(stack, region_of_[i]);
			else if (inst.op == Opcode::EhEnd)
			{
				if (stack == 0)
					return false;
				popped_[i] = nodes_[stack].region;
				stack = nodes_[stack].below;
			}
			else if (can_unwind(c_, inst))
			{
				// The handler that catches it may resume to its parents:
				// every active region can be entered (a marked node's
				// stack is already).
				for (uint32_t s = stack; s != 0 && !marked_[s]; s = nodes_[s].below)
				{
					unsafe_[nodes_[s].region] = 1;
					marked_[s] = 1;
				}
			}
			else if (is_eh_instruction(inst.op))
			{
				// Catch clauses, exception reads and bare cleanups in normal
				// code: not a shape this analysis models.
				return false;
			}
		}
		for (uint32_t k = cfg.succ_first[b]; k < cfg.succ_first[b + 1]; ++k)
		{
			const BlockId s = cfg.succs[k];
			if (entry_[s] == kNoId)
			{
				entry_[s] = stack;
				normal_[s] = 1;
				work.push_back(s);
			}
			else if (entry_[s] != stack)
				return false;
		}
	}
	return true;
}

// Whether `opener` is an eh_cleanup whose handler holds exactly an eh_end
// and a resume.
bool RegionStripper::empty_cleanup(InstId opener) const
{
	const Instruction& inst = body_.instructions[opener];
	if (inst.op != Opcode::EhCleanup || !inst.ops[0].is(ValueKind::Block))
		return false;
	const InstId first = body_.blocks[inst.ops[0].index].first;
	if (first == kNoId || body_.instructions[first].op != Opcode::EhEnd)
		return false;
	const InstId second = body_.instructions[first].next;
	return second != kNoId && body_.instructions[second].op == Opcode::Resume &&
		body_.instructions[second].next == kNoId;
}

// A protected region none of whose normal-path instructions can unwind
// never enters its handler: its eh_try/eh_cleanup and the eh_ends that pop
// it on normal paths go, and the handler becomes unreachable.
bool RegionStripper::run()
{
	region_of_.assign(body_.instructions.size(), kNoId);
	opener_.clear();
	for (BlockId b : body_.layout)
		for (InstId i = body_.blocks[b].first; i != kNoId; i = body_.instructions[i].next)
		{
			const Instruction& inst = body_.instructions[i];
			if ((inst.op == Opcode::EhTry || inst.op == Opcode::EhCleanup) && inst.ops[0].is(ValueKind::Block))
			{
				region_of_[i] = static_cast<uint32_t>(opener_.size());
				opener_.push_back(i);
			}
		}
	if (opener_.empty())
		return false;
	c_.ensure_cfg();
	nodes_.assign(1, StackNode{ kNoId, 0 });
	marked_.assign(1, 1);
	interned_.clear();
	unsafe_.assign(opener_.size(), 0);
	if (!propagate())
		return false;
	// Openers outside normal code (in handlers) stay. From O2 a cleanup
	// whose handler only pops the region and resumes does nothing on the
	// way out (a member with a destructor that proved trivial): it goes
	// whether or not the region can unwind, and unwinding continues to
	// the enclosing handler as the resume did. O1 keeps the shape.
	const bool strip_empty = c_.level >= 2;
	std::vector<uint8_t> strip(opener_.size(), 0);
	bool any = false;
	for (BlockId b : body_.layout)
		if (normal_[b])
			for (InstId i = body_.blocks[b].first; i != kNoId; i = body_.instructions[i].next)
				if (region_of_[i] != kNoId && (!unsafe_[region_of_[i]] || (strip_empty && empty_cleanup(i))))
				{
					strip[region_of_[i]] = 1;
					any = true;
				}
	if (!any)
		return false;
	for (BlockId b : body_.layout)
	{
		if (!normal_[b])
			continue;
		InstId prev = kNoId;
		for (InstId i = body_.blocks[b].first; i != kNoId;)
		{
			const InstId next = body_.instructions[i].next;
			const bool opener = region_of_[i] != kNoId && strip[region_of_[i]];
			const bool closer = popped_[i] != kNoId && strip[popped_[i]];
			if (opener || closer)
				unlink_after(body_, b, prev, i);
			else
				prev = i;
			i = next;
		}
	}
	return true;
}

} // namespace

bool strip_dead_regions(FunctionContext& context)
{
	RegionStripper stripper(context);
	const bool changed = stripper.run();
	if (changed)
		context.invalidate_cfg(); // handler edges went with the openers
	return changed;
}

bool proves_nounwind(const FunctionContext& context)
{
	const SymbolMetadata& m = context.program.symbols[context.symbol].metadata;
	if ((m.flags & SF_REPLACEABLE) != 0)
		return false;
	const FunctionBody& body = context.body;
	for (BlockId b : body.layout)
		for (InstId i = body.blocks[b].first; i != kNoId; i = body.instructions[i].next)
		{
			const Instruction& inst = body.instructions[i];
			if (is_eh_instruction(inst.op) || can_unwind(context, inst))
				return false;
		}
	return true;
}

} // namespace opt
