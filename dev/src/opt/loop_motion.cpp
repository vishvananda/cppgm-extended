#include "lowir/builder.h"
#include "opt/loops.h"
#include "opt/passes.h"
#include "opt/scalar_fold.h"

namespace opt {

using namespace lowir;

namespace {

// Loop-invariant code motion: pure instructions whose operands the loop does
// not compute move to its preheader.
class LoopMotion
{
public:
	explicit LoopMotion(FunctionContext& context) : c_(context), body_(context.body), cfg_(context.cfg) {}

	bool run();

private:
	bool invariant(uint32_t loop, Value v) const;
	void note_register_uses();
	bool literal_index(const Instruction& inst) const;
	bool speculable(const Instruction& inst) const;
	bool worth_moving(const Instruction& inst, bool calls) const;
	bool sweep(uint32_t loop, bool apply);

	FunctionContext& c_;
	FunctionBody& body_;
	Cfg& cfg_;
	LoopForest forest_;
	std::vector<uint8_t> in_register_; // per temp: some use needs it in a register
};

bool LoopMotion::invariant(uint32_t loop, Value v) const
{
	if (!v.is(ValueKind::Temp) || v.index < body_.parameter_count)
		return true;
	const InstId def = body_.temps[v.index].def;
	return def != kNoId && c_.block_of[def] != kNoId && !in_loop(forest_, loop, c_.block_of[def]);
}

bool LoopMotion::literal_index(const Instruction& inst) const
{
	return inst.op == Opcode::Index && inst.ops[1].is(ValueKind::Integer);
}

// An address that only reaches loads and stores, through literal steps, fits
// in their operands: in a register of its own it saves nothing.
void LoopMotion::note_register_uses()
{
	in_register_.assign(body_.temps.size(), 0);
	for (BlockId b : body_.layout)
		for (InstId i = body_.blocks[b].first; i != kNoId; i = body_.instructions[i].next)
		{
			const Instruction& inst = body_.instructions[i];
			uint32_t position = 0;
			for_each_operand(body_, inst, [&](const Value& v) {
				const uint32_t at = position++;
				const bool address = (inst.op == Opcode::Load && at == 0) || (inst.op == Opcode::Store && at == 1) ||
					(literal_index(inst) && at == 0);
				if (v.is(ValueKind::Temp) && !address)
					in_register_[v.index] = 1;
			});
		}
	// (Layout order has definitions first but for phis: two walks back reach
	// the addresses a literal step's use needs.)
	for (unsigned walk = 0; walk < 2; ++walk)
		for (size_t at = body_.layout.size(); at-- > 0;)
			for (InstId i = body_.blocks[body_.layout[at]].first; i != kNoId; i = body_.instructions[i].next)
			{
				const Instruction& inst = body_.instructions[i];
				if (literal_index(inst) && in_register_[inst.result] && inst.ops[0].is(ValueKind::Temp))
					in_register_[inst.ops[0].index] = 1;
			}
}

// Whether `inst` may run on a path that would not have reached it: integer
// division traps unless a literal divisor rules that out.
bool LoopMotion::speculable(const Instruction& inst) const
{
	if (inst.op != Opcode::Binary || !inst.type.is_integer())
		return true;
	const BinaryOp op = static_cast<BinaryOp>(inst.sub);
	if (op != BinaryOp::Div && op != BinaryOp::Mod && op != BinaryOp::UDiv && op != BinaryOp::UMod)
		return true;
	if (!inst.ops[1].is(ValueKind::Integer))
		return false;
	const Literal divisor = normalize_integer(inst.type, body_.literals[inst.ops[1].index]);
	const bool is_signed = op == BinaryOp::Div || op == BinaryOp::Mod;
	return (divisor.low != 0 || divisor.high != 0) && !(is_signed && static_cast<int64_t>(divisor.low) == -1);
}

// A loop with a call would carry the value across it in memory or in one of
// the few registers calls keep: only a costly computation pays for that.
bool LoopMotion::worth_moving(const Instruction& inst, bool calls) const
{
	if (inst.type.kind() == TypeKind::I128 ||
		(inst.op == Opcode::Convert && convert_source_type(inst).kind() == TypeKind::I128))
		return false; // (a call of the runtime's)
	// Virtual locals survive calls without spilling. Their address arithmetic
	// is explicit too, rather than folded into a load/store operand.
	calls = calls && !c_.costs.locals_survive_calls;
	switch (inst.op)
	{
	case Opcode::Addr:
		return !calls && (!c_.costs.scaled_memory_operands || in_register_[inst.result] != 0);
	case Opcode::Index:
		return !calls && (!c_.costs.scaled_memory_operands || !literal_index(inst) || in_register_[inst.result] != 0);
	case Opcode::Binary:
	{
		const BinaryOp op = static_cast<BinaryOp>(inst.sub);
		return !calls || op == BinaryOp::Mul || op == BinaryOp::Div || op == BinaryOp::Mod || op == BinaryOp::UDiv ||
			op == BinaryOp::UMod;
	}
	case Opcode::Compare:
	case Opcode::Copy:
	case Opcode::Unary:
	case Opcode::Select:
	case Opcode::Convert:
	case Opcode::Vector:
		return !calls;
	default:
		return false;
	}
}

// Moves what the loop's blocks hold to its preheader, repeating while a move
// frees another. Without `apply`: whether anything would move.
bool LoopMotion::sweep(uint32_t loop, bool apply)
{
	const Loop& l = forest_.loops[loop];
	if (apply && l.preheader == kNoId)
		return false;
	bool calls = false;
	for (BlockId b : l.blocks)
		for (InstId i = body_.blocks[b].first; i != kNoId && !calls; i = body_.instructions[i].next)
			calls = body_.instructions[i].op == Opcode::Call || body_.instructions[i].op == Opcode::Asm;
	const BlockId pre = l.preheader;
	InstId tail = apply ? previous_in_block(body_, pre, body_.blocks[pre].last) : kNoId;
	bool any = false;
	for (bool moved = true; moved;)
	{
		moved = false;
		for (BlockId b : l.blocks)
		{
			// Everything before a point of the header runs whenever the
			// preheader does (the header's instructions up to the first that
			// may leave).
			bool certain = b == l.header;
			InstId prev = kNoId;
			for (InstId i = body_.blocks[b].first; i != kNoId;)
			{
				const Instruction& inst = body_.instructions[i];
				const InstId next = inst.next;
				bool move = is_pure(inst) && inst.op != Opcode::Phi && !inst.type.is_object() &&
					inst.type.kind() != TypeKind::F80 && worth_moving(inst, calls) && (certain || speculable(inst));
				if (move)
					for_each_operand(body_, inst, [&](const Value& v) { move = move && invariant(loop, v); });
				if (!move)
				{
					certain = certain && (is_pure(inst) || inst.op == Opcode::Load || inst.op == Opcode::Store);
					prev = i;
					i = next;
					continue;
				}
				if (!apply)
					return true;
				unlink_after(body_, b, prev, i);
				insert_after(body_, pre, tail, i);
				if (!certain)
				{
					body_.instructions[i].flags &= static_cast<uint8_t>(~(IF_NO_WRAP | IF_EXACT | IF_NONNULL));
					if (body_.instructions[i].op == Opcode::Index)
						body_.instructions[i].set_index_bounds(IndexBounds::None);
				}
				tail = i;
				c_.block_of[i] = pre;
				++c_.stats.invariants_hoisted;
				moved = any = true;
				i = next;
			}
		}
	}
	return any;
}

bool LoopMotion::run()
{
	c_.ensure_cfg();
	find_loops(body_, cfg_, forest_);
	if (forest_.loops.empty())
		return false;
	c_.entry_block_of(hoist_loop_invariants);
	note_register_uses();
	// A preheader for each loop with something to move, then the loops are
	// found again. (A loop's preheader changes only its header's entry edge.)
	bool changed = false;
	for (unsigned round = 0; round < 16; ++round)
	{
		bool one = false;
		for (uint32_t loop = 0; loop < forest_.loops.size(); ++loop)
			one = (forest_.loops[loop].preheader == kNoId && sweep(loop, false) &&
					  make_loop_preheader(body_, cfg_, forest_, loop)) ||
				one;
		if (!one)
			break;
		changed = true;
		c_.invalidate_cfg();
		c_.ensure_cfg();
		find_loops(body_, cfg_, forest_);
		map_instruction_blocks(body_, c_.block_of);
		c_.invalidate_block_of();
	}
	// Innermost first: what moves into an inner preheader may leave the loop
	// around it too.
	for (uint32_t loop = static_cast<uint32_t>(forest_.loops.size()); loop-- > 0;)
		changed = sweep(loop, true) || changed;
	return changed;
}

} // namespace

bool hoist_loop_invariants(FunctionContext& context)
{
	LoopMotion pass(context);
	return pass.run();
}

} // namespace opt
