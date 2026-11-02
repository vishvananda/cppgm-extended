#include "lowir/builder.h"
#include "opt/loops.h"
#include "opt/passes.h"

#include <vector>

namespace opt {

using namespace lowir;

namespace {

// The most literals one loop keeps in registers: the rest stay in memory.
constexpr size_t kHeld = 8;
constexpr size_t kHeldAddresses = 4;

// A loop without calls on its usual paths reads its floating literals from registers: each is
// copied once before the outermost such loop, and a register costs no load
// per iteration. (A value the allocator cannot keep is read from its frame
// home, as the literal would be from read-only data.) Zero is cheaper to
// clear than to keep.
class LoopLiterals
{
public:
	explicit LoopLiterals(FunctionContext& context) : c_(context), body_(context.body), cfg_(context.cfg) {}

	bool run();

private:
	struct Held
	{
		Value literal;
		Value temp;
	};

	bool rare(BlockId b) const;
	bool has_call(const Loop& loop) const;
	bool reads_literal(const Loop& loop) const;
	bool same_literal(Value a, Value b) const;
	bool hold(uint32_t loop);
	bool hold_addresses(uint32_t loop);

	FunctionContext& c_;
	FunctionBody& body_;
	Cfg& cfg_;
	LoopForest forest_;
	std::vector<Held> held_;
};

bool held_kind(Value v)
{
	return v.is(ValueKind::Float) && (v.float_kind == TypeKind::F32 || v.float_kind == TypeKind::F64);
}

// Whether a branch leads to b only on a floating value that is not a number,
// or on a negative one under a square root: a rare path, around whose call
// the allocator saves what the loop keeps.
bool LoopLiterals::rare(BlockId b) const
{
	if (cfg_.pred_count(b) != 1)
		return false;
	const Instruction& branch = body_.instructions[body_.blocks[cfg_.preds[cfg_.pred_first[b]]].last];
	if (branch.op != Opcode::Branch || !branch.ops[0].is(ValueKind::Temp) || !(branch.ops[1].index == b))
		return false;
	const InstId def = body_.temps[branch.ops[0].index].def;
	if (def == kNoId)
		return false;
	const Instruction& test = body_.instructions[def];
	if (test.op != Opcode::Compare || !test.type.is_float())
		return false;
	const ComparePredicate pred = static_cast<ComparePredicate>(test.sub);
	return (pred == ComparePredicate::Ne && test.ops[0] == test.ops[1]) ||
		(pred == ComparePredicate::Lt && test.ops[1].is(ValueKind::Float) &&
			body_.literals[test.ops[1].index].low == 0);
}

bool LoopLiterals::has_call(const Loop& loop) const
{
	for (BlockId b : loop.blocks)
		for (InstId i = body_.blocks[b].first; i != kNoId; i = body_.instructions[i].next)
			if (body_.instructions[i].op == Opcode::Call && !rare(b))
				return true;
	return false;
}

bool LoopLiterals::same_literal(Value a, Value b) const
{
	const Literal& x = body_.literals[a.index];
	const Literal& y = body_.literals[b.index];
	return x.low == y.low && x.high == y.high && a.float_kind == b.float_kind;
}

bool LoopLiterals::reads_literal(const Loop& loop) const
{
	bool found = false;
	for (BlockId b : loop.blocks)
		for (InstId i = body_.blocks[b].first; i != kNoId && !found; i = body_.instructions[i].next)
		{
			const Instruction& inst = body_.instructions[i];
			if (inst.op != Opcode::Store)
				for_each_operand(body_, inst,
					[&](const Value& v) { found = found || (held_kind(v) && body_.literals[v.index].low != 0); });
		}
	return found;
}

bool LoopLiterals::hold(uint32_t loop)
{
	const Loop& l = forest_.loops[loop];
	const BlockId pre = l.preheader;
	InstId tail = previous_in_block(body_, pre, body_.blocks[pre].last);
	held_.clear();
	bool changed = false;
	for (BlockId b : l.blocks)
		for (InstId i = body_.blocks[b].first; i != kNoId; i = body_.instructions[i].next)
		{
			if (body_.instructions[i].op == Opcode::Store)
				continue;
			// (Appending below moves the instructions and the operands.)
			const uint32_t extra = body_.instructions[i].extra_count;
			for (uint32_t k = 0; k < 3 + extra; ++k)
			{
				const uint32_t first = body_.instructions[i].extra_first;
				const Value v = k < 3 ? body_.instructions[i].ops[k] : body_.operands[first + k - 3];
				if (!held_kind(v) || body_.literals[v.index].low == 0)
					continue;
				// (A phi's input from outside is read once.)
				if (k >= 3 && body_.instructions[i].op == Opcode::Phi &&
					!in_loop(forest_, loop, body_.operands[first + k - 4].index))
					continue;
				Value temp = Value::none();
				for (const Held& h : held_)
					if (same_literal(h.literal, v))
						temp = h.temp;
				if (temp.is(ValueKind::None))
				{
					if (held_.size() == kHeld)
						continue;
					const Type type = Type::scalar(v.float_kind);
					Instruction copy = make_instruction(Opcode::Copy, type);
					copy.ops[0] = v;
					const InstId id = append_instruction(body_, copy);
					const TempId result = append_temp(body_, type, id);
					body_.instructions[id].result = result;
					insert_after(body_, pre, tail, id);
					tail = id;
					temp = Value::temp(result);
					held_.push_back(Held{ v, temp });
				}
				if (k < 3)
					body_.instructions[i].ops[k] = temp;
				else
					body_.operands[first + k - 3] = temp;
				changed = true;
			}
		}
	return changed;
}

// A compare in a loop reads a symbol's address, at most kHeldAddresses of
// them, from an `addr` in the block that dominates the outermost loop's
// header: a preheader, or the test that enters the loop. Read in place, the
// address takes a `lea` per pass. A general register survives a call, so a
// loop with calls holds them too.
bool LoopLiterals::hold_addresses(uint32_t loop)
{
	const Loop& l = forest_.loops[loop];
	const BlockId pre = cfg_.idom[l.header];
	InstId tail = previous_in_block(body_, pre, body_.blocks[pre].last);
	held_.clear();
	bool changed = false;
	for (BlockId b : l.blocks)
		for (InstId i = body_.blocks[b].first; i != kNoId; i = body_.instructions[i].next)
			for (unsigned k = 0; k < 2 && body_.instructions[i].op == Opcode::Compare; ++k)
			{
				const Value v = body_.instructions[i].ops[k];
				if (!v.is(ValueKind::Symbol))
					continue;
				Value temp = Value::none();
				for (const Held& h : held_)
					if (h.literal == v)
						temp = h.temp;
				if (temp.is(ValueKind::None))
				{
					if (held_.size() == kHeldAddresses)
						continue;
					const Type type = Type::scalar(TypeKind::Ptr);
					Instruction addr = make_instruction(Opcode::Addr, type);
					addr.ops[0] = v;
					const InstId id = append_instruction(body_, addr);
					const TempId result = append_temp(body_, type, id);
					body_.instructions[id].result = result;
					insert_after(body_, pre, tail, id);
					tail = id;
					temp = Value::temp(result);
					held_.push_back(Held{ v, temp });
				}
				body_.instructions[i].ops[k] = temp;
				changed = true;
			}
	return changed;
}

bool LoopLiterals::run()
{
	if (body_.layout.size() == 1)
		return false;
	c_.ensure_cfg();
	find_loops(body_, cfg_, forest_);
	std::vector<uint8_t> held(forest_.loops.size(), 0);
	bool changed = false;
	for (uint32_t loop = 0; loop < forest_.loops.size(); ++loop)
	{
		const uint32_t parent = forest_.loops[loop].parent;
		if (parent != kNoId && held[parent] != 0)
		{
			held[loop] = 1;
			continue;
		}
		if (has_call(forest_.loops[loop]))
			continue;
		held[loop] = 1;
		if (forest_.loops[loop].preheader != kNoId && reads_literal(forest_.loops[loop]))
			changed = hold(loop) || changed;
	}
	std::vector<uint8_t> addressed(forest_.loops.size(), 0);
	for (uint32_t loop = 0; loop < forest_.loops.size(); ++loop)
	{
		const uint32_t parent = forest_.loops[loop].parent;
		if (parent != kNoId && addressed[parent] != 0)
		{
			addressed[loop] = 1;
			continue;
		}
		addressed[loop] = cfg_.idom[forest_.loops[loop].header] != kNoId;
		if (addressed[loop] != 0)
			changed = hold_addresses(loop) || changed;
	}
	if (changed)
		c_.invalidate_block_of();
	return changed;
}

} // namespace

bool hold_loop_literals(FunctionContext& context)
{
	LoopLiterals pass(context);
	return pass.run();
}

} // namespace opt
