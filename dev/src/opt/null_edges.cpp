#include "lowir/builder.h"
#include "opt/passes.h"
#include "opt/scalar_fold.h"

namespace opt {

using namespace lowir;

namespace {

// Instructions examined after a block's phis.
const uint32_t kNullUseScan = 32;
// Definitions a predicted branch's test is traced back through.
const unsigned kExpectTrace = 8;

// Whether `inst` always completes: control reaches the instruction after it.
bool completes(const Instruction& inst)
{
	switch (inst.op)
	{
	case Opcode::Const:
	case Opcode::Copy:
	case Opcode::Addr:
	case Opcode::Load:
	case Opcode::Index:
	case Opcode::Unary:
	case Opcode::Binary:
	case Opcode::Compare:
	case Opcode::Select:
	case Opcode::Convert:
	case Opcode::Vector:
	case Opcode::Store:
	case Opcode::CopyObject:
	case Opcode::ZeroInit:
		return true;
	default:
		return false;
	}
}

// Whether `inst` has no defined behavior when `pointer` is null: a non-null
// refinement of it, a nonvolatile access through it (no object lives at a
// null pointer or a step from one), or a call that passes it where the
// callee states an object.
bool requires_object(const Program& program, const FunctionBody& body, const Instruction& inst, Value pointer)
{
	switch (inst.op)
	{
	case Opcode::Copy:
		return (inst.flags & IF_NONNULL) != 0 && inst.ops[0] == pointer;
	case Opcode::Load:
		return (inst.flags & IF_VOLATILE) == 0 && accessed_pointer(body, inst.ops[0]) == pointer;
	case Opcode::Store:
		return (inst.flags & IF_VOLATILE) == 0 && accessed_pointer(body, inst.ops[1]) == pointer;
	case Opcode::Call:
	{
		const Signature* callee = program.call_boundary(inst);
		if (callee == nullptr)
			return false;
		const Parameter* parameters = program.parameters_of(*callee);
		for (uint32_t k = 0; k < inst.extra_count && k < callee->parameter_count; ++k)
			if (body.operands[inst.extra_first + k] == pointer && parameters[k].object_bytes != 0 &&
				parameters[k].type.is_pointer())
				return true;
		return false;
	}
	default:
		return false;
	}
}

// Whether `block`, entered with the phi `pointer` null, has no defined
// behavior: an instruction that requires its object runs before any that
// may not complete.
bool requires_object(const Program& program, const FunctionBody& body, BlockId block, Value pointer)
{
	uint32_t seen = 0;
	for (InstId i = body.blocks[block].first; i != kNoId && seen < kNullUseScan; i = body.instructions[i].next)
	{
		const Instruction& inst = body.instructions[i];
		if (inst.op == Opcode::Phi)
			continue;
		++seen;
		if (requires_object(program, body, inst, pointer))
			return true;
		if (!completes(inst))
			return false;
	}
	return false;
}

bool IsZero(const FunctionBody& body, Value value)
{
	Constant constant;
	return literal_constant(body, value, constant) && constant.kind == ValueKind::Integer &&
		(constant.bits.low | constant.bits.high) == 0;
}

bool IntegerOrPointer(const FunctionBody& body, Value value)
{
	return value.is(ValueKind::Temp) &&
		(body.temps[value.index].type.is_integer() || body.temps[value.index].type.is_pointer());
}

// The value whose truth a branch tests: its condition traced back through
// copies and conversions of an integer or pointer and through comparisons
// with zero (an `eq` inverting the truth expected of it). `bits` is the
// narrowest truncation after the last comparison, which decides a
// constant's truth there (a cast to bool truncates a truth value).
Value TestedValue(const FunctionBody& body, Value value, bool& expected, unsigned& bits)
{
	bits = 128;
	for (unsigned depth = 0; depth < kExpectTrace && IntegerOrPointer(body, value); ++depth)
	{
		const InstId def = body.temps[value.index].def;
		if (def == kNoId)
			break;
		const Instruction& inst = body.instructions[def];
		const ComparePredicate predicate = static_cast<ComparePredicate>(inst.sub);
		const ConvertOp conversion = static_cast<ConvertOp>(inst.sub);
		if (inst.op == Opcode::Copy && IntegerOrPointer(body, inst.ops[0]))
			value = inst.ops[0];
		else if (inst.op == Opcode::Convert && IntegerOrPointer(body, inst.ops[0]) &&
			(conversion == ConvertOp::ZExt || conversion == ConvertOp::SExt || conversion == ConvertOp::Trunc))
		{
			if (conversion == ConvertOp::Trunc && integer_bits(inst.type.kind()) < bits)
				bits = integer_bits(inst.type.kind());
			value = inst.ops[0];
		}
		else if (inst.op == Opcode::Compare &&
			(predicate == ComparePredicate::Eq || predicate == ComparePredicate::Ne) &&
			(IsZero(body, inst.ops[0]) || IsZero(body, inst.ops[1])))
		{
			expected = expected == (predicate == ComparePredicate::Ne);
			bits = 128;
			value = IsZero(body, inst.ops[1]) ? inst.ops[0] : inst.ops[1];
		}
		else
			break;
	}
	return value;
}

// Whether a constant reaching the traced value is nonzero at the test.
bool TruthAt(const Constant& constant, unsigned bits)
{
	const uint64_t low = bits >= 64 ? constant.bits.low : constant.bits.low & ((uint64_t(1) << bits) - 1);
	const uint64_t high = bits >= 128 ? constant.bits.high
		: bits > 64                   ? constant.bits.high & ((uint64_t(1) << (bits - 64)) - 1)
									  : 0;
	return (low | high) != 0;
}

// The two-way branch that decides whether control goes from `from` to
// `to`, and the block holding it: from's own, or the one of from's single
// predecessor when from only jumps on. `target` is the side it takes.
BlockId DecidingBranch(const FunctionBody& body, const Cfg& cfg, BlockId from, BlockId to, BlockId& target)
{
	for (unsigned step = 0; step < 2; ++step)
	{
		const InstId t = body.blocks[from].last;
		if (t == kNoId)
			return kNoId;
		const Instruction& term = body.instructions[t];
		if (term.op == Opcode::Branch && term.ops[1].index != term.ops[2].index &&
			(term.ops[1].index == to || term.ops[2].index == to))
		{
			target = to;
			return from;
		}
		if (step != 0 || term.op != Opcode::Jump || cfg.pred_count(from) != 1)
			return kNoId;
		to = from;
		from = cfg.preds[cfg.pred_first[from]];
	}
	return kNoId;
}

} // namespace

// O1+: an edge that brings a null pointer to a phi whose block cannot run
// with it null is never taken (`static_cast<Derived*>(base)->member`: the
// cast's null test, which a null base alone passes). A jump along it becomes
// `unreachable`, a branch a jump to its other target; the cleanup removes
// the test.
bool drop_null_edges(FunctionContext& c)
{
	FunctionBody& body = c.body;
	c.ensure_cfg();
	const Cfg& cfg = c.cfg;
	bool changed = false;
	std::vector<BlockId> dead;
	for (BlockId block : body.layout)
	{
		const InstId first = body.blocks[block].first;
		if (first == kNoId || body.instructions[first].op != Opcode::Phi || !cfg.reachable(block) ||
			(cfg.flags[block] & BF_HANDLER) != 0)
			continue;
		dead.clear();
		for (InstId i = first; i != kNoId && body.instructions[i].op == Opcode::Phi; i = body.instructions[i].next)
		{
			const Instruction& phi = body.instructions[i];
			if (!phi.type.is_pointer() || phi.extra_count < 4)
				continue;
			bool null = false, required = false;
			for (uint32_t k = 0; k < phi.extra_count; k += 2)
			{
				const BlockId pred = body.operands[phi.extra_first + k].index;
				Constant zero;
				if (pred == block || !literal_constant(body, body.operands[phi.extra_first + k + 1], zero) ||
					zero.kind != ValueKind::Integer || (zero.bits.low | zero.bits.high) != 0)
					continue;
				if (!null)
					required = requires_object(c.program, body, block, Value::temp(phi.result));
				null = true;
				if (!required)
					break;
				bool listed = false;
				for (BlockId d : dead)
					listed = listed || d == pred;
				if (!listed)
					dead.push_back(pred);
			}
		}
		for (BlockId pred : dead)
		{
			const InstId t = body.blocks[pred].last;
			if (t == kNoId || !cfg.reachable(pred))
				continue;
			Instruction& term = body.instructions[t];
			if (term.op == Opcode::Jump)
				term.op = Opcode::Unreachable;
			else if (term.op == Opcode::Branch && term.ops[1].index != term.ops[2].index)
			{
				term.op = Opcode::Jump;
				term.sub = 0;
				term.flags = 0;
				term.type = Type::scalar(TypeKind::Void);
				term.ops[1] = term.ops[1].index == block ? term.ops[2] : term.ops[1];
			}
			else
				continue;
			term.ops[0] = term.op == Opcode::Jump ? term.ops[1] : Value::none();
			term.ops[1] = term.ops[2] = Value::none();
			remove_phi_incoming(body, block, pred);
			changed = true;
		}
	}
	if (changed)
		c.invalidate_cfg();
	return changed;
}

// O1+: a predicted branch expects the value it tests to be nonzero or zero.
// When that value is a phi of its block, an edge that brings the phi a
// constant of the other truth is rarely taken, and the unpredicted branch
// that chooses the edge is predicted away from it, as LLVM lowers
// llvm.expect of a phi. A branch so predicted predicts the branches before
// it in turn: each operand of an `&&` or `||` chain whose value an
// expectation tests (`!` and casts to bool included) is predicted to
// continue the chain. Predictions change no edge.
bool predict_constant_edges(FunctionContext& c)
{
	FunctionBody& body = c.body;
	std::vector<BlockId> work;
	for (BlockId b : body.layout)
	{
		const InstId t = body.blocks[b].last;
		if (t != kNoId && body.instructions[t].op == Opcode::Branch &&
			static_cast<BranchPrediction>(body.instructions[t].sub) != BranchPrediction::None)
			work.push_back(b);
	}
	if (work.empty())
		return false;
	const Cfg& cfg = c.ensure_cfg_light();
	bool changed = false;
	while (!work.empty())
	{
		const BlockId b = work.back();
		work.pop_back();
		const Instruction& branch = body.instructions[body.blocks[b].last];
		bool expected = static_cast<BranchPrediction>(branch.sub) == BranchPrediction::Likely;
		unsigned bits = 128;
		const Value tested = TestedValue(body, branch.ops[0], expected, bits);
		InstId phi = body.blocks[b].first;
		while (phi != kNoId && body.instructions[phi].op == Opcode::Phi &&
			!(tested.is(ValueKind::Temp) && body.instructions[phi].result == tested.index))
			phi = body.instructions[phi].next;
		if (phi == kNoId || body.instructions[phi].op != Opcode::Phi || !cfg.reachable(b))
			continue;
		for (uint32_t k = 0; k < body.instructions[phi].extra_count; k += 2)
		{
			const uint32_t at = body.instructions[phi].extra_first + k;
			Constant constant;
			if (!literal_constant(body, body.operands[at + 1], constant) || constant.kind != ValueKind::Integer ||
				TruthAt(constant, bits) == expected)
				continue;
			BlockId target = kNoId;
			const BlockId from = DecidingBranch(body, cfg, body.operands[at].index, b, target);
			Instruction* decider = from == kNoId ? nullptr : &body.instructions[body.blocks[from].last];
			if (decider == nullptr || static_cast<BranchPrediction>(decider->sub) != BranchPrediction::None)
				continue;
			decider->sub = static_cast<uint8_t>(
				decider->ops[1].index == target ? BranchPrediction::Unlikely : BranchPrediction::Likely);
			work.push_back(from);
			changed = true;
		}
	}
	return changed;
}

} // namespace opt
