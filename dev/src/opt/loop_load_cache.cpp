#include "lowir/builder.h"
#include "opt/loops.h"
#include "opt/passes.h"

#include <vector>

namespace opt {

using namespace lowir;

namespace {

// Carry a loaded boundary beside a conditionally updated pointer. Unlike
// invariant-load motion, this follows a small graph of pointer phis: unchanged
// edges keep the boundary, and changed edges reuse a load already on that path.
// Bound both the graph and the number of rewrites; no speculative search.
constexpr size_t kPhis = 4;
constexpr size_t kEdges = 16;
constexpr unsigned kRewrites = 16;

struct CacheEdge
{
	BlockId pred;
	Value pointer;
	InstId load;
	uint32_t node;
};

struct CacheNode
{
	InstId phi;
	BlockId block;
	Value cached;
	std::vector<CacheEdge> edges;
};

class LoopLoadCache
{
public:
	explicit LoopLoadCache(FunctionContext& context) : c_(context), body_(context.body) {}
	bool run();

private:
	bool eligible(const Loop& loop) const;
	bool plan(uint32_t loop, InstId index);
	uint32_t node(uint32_t loop, InstId phi);
	InstId edge_load(BlockId pred, Value pointer) const;
	Value emit(BlockId block, Instruction inst);
	Value boundary(BlockId block, Value loaded);
	bool apply(uint32_t loop);

	FunctionContext& c_;
	FunctionBody& body_;
	LoopForest forest_;
	std::vector<CacheNode> nodes_;
	InstId load_ = kNoId, index_ = kNoId;
	BlockId entry_ = kNoId;
	size_t edges_ = 0;
};

bool LoopLoadCache::eligible(const Loop& loop) const
{
	bool store = false;
	const Type ptr = Type::scalar(TypeKind::Ptr);
	for (BlockId b : loop.blocks)
		for (InstId i = body_.blocks[b].first; i != kNoId; i = body_.instructions[i].next)
		{
			const Instruction& inst = body_.instructions[i];
			if ((inst.flags & IF_VOLATILE) != 0)
				return false;
			if (inst.op == Opcode::Store)
			{
				if ((inst.flags & IF_TYPED) == 0 || !typed_types_apart(inst.type, ptr))
					return false;
				store = true;
			}
			else if (inst.op != Opcode::Load && !is_pure(inst) && inst.op != Opcode::Jump && inst.op != Opcode::Branch)
				return false;
		}
	return store;
}

InstId LoopLoadCache::edge_load(BlockId pred, Value pointer) const
{
	for (InstId i = body_.blocks[pred].first; i != kNoId; i = body_.instructions[i].next)
	{
		const Instruction& inst = body_.instructions[i];
		if (inst.op == Opcode::Load && inst.type.is_pointer() && inst.ops[0] == pointer &&
			(inst.flags & (IF_TYPED | IF_VOLATILE)) == IF_TYPED)
			return i;
	}
	return kNoId;
}

uint32_t LoopLoadCache::node(uint32_t loop, InstId phi)
{
	for (uint32_t n = 0; n < nodes_.size(); ++n)
		if (nodes_[n].phi == phi)
			return n;
	if (nodes_.size() == kPhis)
		return kNoId;
	const uint32_t n = static_cast<uint32_t>(nodes_.size());
	const BlockId block = c_.block_of[phi];
	const Instruction& inst = body_.instructions[phi];
	if (!inst.type.is_pointer() || block == kNoId || !in_loop(forest_, loop, block))
		return kNoId;
	nodes_.push_back(CacheNode{ phi, block, Value::none(), {} });
	for (uint32_t k = 0; k < inst.extra_count; k += 2)
	{
		if (++edges_ > kEdges)
			return kNoId;
		const BlockId pred = body_.operands[inst.extra_first + k].index;
		const Value pointer = body_.operands[inst.extra_first + k + 1];
		CacheEdge edge{ pred, pointer, kNoId, kNoId };
		if (!in_loop(forest_, loop, pred))
		{
			if (n != 0 || pred != entry_)
				return kNoId;
		}
		else
		{
			const InstId def = pointer.is(ValueKind::Temp) ? body_.temps[pointer.index].def : kNoId;
			if (def != kNoId && body_.instructions[def].op == Opcode::Phi)
			{
				edge.node = node(loop, def);
				if (edge.node == kNoId)
					return kNoId;
			}
			else
			{
				edge.load = edge_load(pred, pointer);
				if (edge.load == kNoId)
					return kNoId;
			}
		}
		nodes_[n].edges.push_back(edge);
	}
	return n;
}

bool LoopLoadCache::plan(uint32_t loop, InstId index)
{
	const Loop& l = forest_.loops[loop];
	const Instruction& ix = body_.instructions[index];
	if (ix.op != Opcode::Index || !ix.ops[1].is(ValueKind::Integer) || !ix.ops[0].is(ValueKind::Temp) ||
		ix.result == kNoId || c_.uses[ix.result] == 0)
		return false;
	const InstId load = body_.temps[ix.ops[0].index].def;
	if (load == kNoId || c_.block_of[load] != l.header || c_.uses[ix.ops[0].index] != 1)
		return false;
	const Instruction& ld = body_.instructions[load];
	if (ld.op != Opcode::Load || !ld.type.is_pointer() || (ld.flags & (IF_TYPED | IF_VOLATILE)) != IF_TYPED ||
		!ld.ops[0].is(ValueKind::Temp))
		return false;
	const InstId phi = body_.temps[ld.ops[0].index].def;
	if (phi == kNoId || c_.block_of[phi] != l.header || body_.instructions[phi].op != Opcode::Phi)
		return false;
	entry_ = kNoId;
	for (uint32_t k = c_.cfg.pred_first[l.header]; k < c_.cfg.pred_first[l.header + 1]; ++k)
	{
		const BlockId pred = c_.cfg.preds[k];
		if (!in_loop(forest_, loop, pred))
		{
			if (entry_ != kNoId || (c_.cfg.flags[pred] & BF_EH) != 0)
				return false;
			entry_ = pred;
		}
	}
	if (entry_ == kNoId || !eligible(l))
		return false;
	nodes_.clear();
	edges_ = 0;
	load_ = load;
	index_ = index;
	return node(loop, phi) == 0;
}

Value LoopLoadCache::emit(BlockId block, Instruction inst)
{
	const InstId id = append_instruction(body_, inst);
	body_.instructions[id].result = append_temp(body_, Type::scalar(TypeKind::Ptr), id);
	const InstId before = previous_in_block(body_, block, body_.blocks[block].last);
	insert_after(body_, block, before, id);
	return Value::temp(body_.instructions[id].result);
}

Value LoopLoadCache::boundary(BlockId block, Value loaded)
{
	const Instruction original = body_.instructions[index_];
	Instruction ix = make_instruction(Opcode::Index, original.type);
	ix.location = original.location;
	ix.ops[0] = loaded;
	ix.ops[1] = original.ops[1];
	return emit(block, ix);
}

bool LoopLoadCache::apply(uint32_t loop)
{
	BlockId pre = forest_.loops[loop].preheader;
	if (pre == kNoId)
	{
		// Split the taken entry edge: a zero-trip guard must not acquire a read.
		if (!make_loop_preheader(body_, c_.cfg, forest_, loop))
			return false;
		pre = static_cast<BlockId>(body_.blocks.size() - 1);
	}
	for (CacheNode& n : nodes_)
	{
		Instruction phi = make_instruction(Opcode::Phi, Type::scalar(TypeKind::Ptr));
		phi.location = body_.instructions[n.phi].location;
		const InstId id = append_instruction(body_, phi);
		body_.instructions[id].result = append_temp(body_, phi.type, id);
		n.cached = Value::temp(body_.instructions[id].result);
		insert_after(body_, n.block, kNoId, id);
	}
	for (const CacheNode& n : nodes_)
	{
		std::vector<Value> incoming;
		for (const CacheEdge& edge : n.edges)
		{
			BlockId pred = edge.pred;
			Value value;
			if (pred == entry_)
			{
				pred = pre;
				Instruction ld = body_.instructions[load_];
				ld.ops[0] = edge.pointer;
				ld.flags = 0;
				value = boundary(pred, emit(pred, ld));
			}
			else if (edge.node != kNoId)
				value = nodes_[edge.node].cached;
			else
				value = boundary(pred, Value::temp(body_.instructions[edge.load].result));
			incoming.push_back(Value::block(pred));
			incoming.push_back(value);
		}
		const uint32_t first = append_operands(body_, incoming.data(), static_cast<uint32_t>(incoming.size()));
		Instruction& phi = body_.instructions[body_.temps[n.cached.index].def];
		phi.extra_first = first;
		phi.extra_count = static_cast<uint32_t>(incoming.size());
	}
	const TempId result = body_.instructions[index_].result;
	replace_all_uses(body_, result, nodes_[0].cached);
	c_.invalidate_cfg();
	c_.invalidate_block_of();
	c_.invalidate_uses();
	++c_.stats.invariants_hoisted;
	return true;
}

bool LoopLoadCache::run()
{
	bool changed = false;
	for (unsigned round = 0; round < kRewrites; ++round)
	{
		c_.ensure_cfg();
		find_loops(body_, c_.cfg, forest_);
		c_.entry_block_of(cache_loop_loads, round == 0);
		c_.entry_uses(cache_loop_loads, round == 0);
		bool rewritten = false;
		for (uint32_t l = static_cast<uint32_t>(forest_.loops.size()); l-- > 0 && !rewritten;)
		{
			const Loop& loop = forest_.loops[l];
			for (InstId i = body_.blocks[loop.header].first; i != kNoId && !rewritten; i = body_.instructions[i].next)
				if (plan(l, i))
					rewritten = apply(l);
		}
		if (!rewritten)
			break;
		changed = true;
	}
	return changed;
}

} // namespace

bool cache_loop_loads(FunctionContext& context)
{
	return LoopLoadCache(context).run();
}

} // namespace opt
