#include "lowir/builder.h"
#include "opt/passes.h"
#include "opt/scalar_fold.h"

namespace opt {

using namespace lowir;

namespace {

// Instructions examined after a block's phis.
const uint32_t kNullUseScan = 32;

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

} // namespace opt
