#include "opt/inliner.h"
#include "opt/interprocedural.h"
#include "opt/scalar_fold.h"

#include "lowir/builder.h"
#include "opt/loops.h"

namespace opt {

using namespace lowir;

namespace {

// Ordinary wave: the per-caller growth budget and the trivial leaf every
// budget admits (a callee's own charge is priced in admit).
const uint32_t kCallerBudget = 1536;
// Additional call-containing copies in an already large caller, outside
// its loops. Small helpers retain ordinary admission: losing a setter or
// accessor there can cost every activation of a hot wrapper.
const uint32_t kOutsideLoopBudget = 40;
// Loop sites get a fourfold allowance for small full callees and
// bounded partial copies.
const uint32_t kLoopSiteScale = 4;
const uint32_t kTrivialLeaf = 4;
// Single-call (definition-removing) inlining; from O2 a callee twice that
// size and its caller twice the budget (a lambda's instantiation of a
// large dispatcher called once: pa33's O1 fixtures pin the O1 caps).
const uint32_t kOnceCap = 512;
const uint32_t kOnceCallerBudget = 1024;
const uint32_t kOnceO2Scale = 2;
// Folded copies of EH-bearing callees given constant arguments: the
// callee's largest size, the copy's caps (hinted larger), the per-caller
// budget of copied instructions and the failures one callee may cost a wave.
const uint32_t kFoldedCalleeCap = 256;
const uint32_t kFoldedCap = 40;
const uint32_t kFoldedHintCap = 80;
const uint32_t kFoldedCallerBudget = 2048;
const uint32_t kFoldedFailures = 16;
// A small original wrapper: one this size with a constant result folds its
// caller's guard, and a larger callee full of calls stays out of a large
// caller's code outside loops (kOutsideLoopBudget).
const uint32_t kWrapperCap = 32;
// Partial inlining (O2+, late wave): the copy's caps (a hinted callee's
// larger), doubled at a loop site.
const uint32_t kPartialCap = 24;
const uint32_t kPartialHintCap = 40;
// A partial copy of a callee with at most two callers.
const uint32_t kPartialFewCap = 48;
// A callee whose hot code allocates (a growth path) gets only a partial
// copy this small: a test or two in front of the work.
const uint32_t kPartialAllocatingCap = 8;
// So does a callee on a call cycle (not the caller itself): its early
// return (`if (p == nullptr) return nullptr;` in front of a tree walk)
// goes inline at each site of the cycle at the cost of a few instructions.
const uint32_t kPartialRecursiveCap = 8;
// Recursive inlining (O2+, late wave): a caller's call to itself takes a
// copy of the caller's body as the wave found it, to kRecursiveDepth
// levels (fewer when the copies would pass kRecursiveTotal instructions),
// while that body has at most kRecursiveCap instructions, no EH region and,
// without a loop, at least kRecursiveTreeCalls calls to itself (a tree walk
// recurses from about every other activation; a lone self call usually
// sits behind an early return, and a copy would only lengthen the common
// path's prologue); the deepest copies' calls get the partial copies. A
// body with a loop and one call to itself, as tail recursion often leaves,
// takes kRecursiveChainDepth levels: the copies are nested loops.
const uint32_t kRecursiveCap = 64;
const uint32_t kRecursiveTotal = 320;
const uint32_t kRecursiveDepth = 2;
const uint32_t kRecursiveChainDepth = 8;
const uint32_t kRecursiveTreeCalls = 2;
// A caller from this size is large: no site copies it into another caller
// except as a single-call candidate.
const uint32_t kSettledSize = 320;
struct EntryCost
{
	uint32_t parameter = kNoId;
	Type type;
	ComparePredicate predicate = ComparePredicate::Ne;
	Constant literal = {};
	bool parameter_left = true;
	uint32_t hot[2] = {}, unpriced[2] = {};
};

struct Shape
{
	EntryCost entry_cost;

	bool constant_body = false;
	uint64_t constant_parameters = 0;
	uint32_t cost = 0, hot_cost = 0, unpriced = 0, hot_unpriced = 0;
	uint32_t size = 0;
	uint32_t hot = 0;      // instructions outside cold blocks (every path from them
						   // ends in noreturn)
	bool hot_call = false; // a call outside cold blocks
	uint32_t blocks = 0;
	uint32_t returns = 0;
	bool eh = false;
	bool catches = false; // a catch or filter clause, a selector or exception
						  // read, or a throw
	bool guarded = false; // catches only in terminate guards (guard_landing)
	bool call = false;
	uint32_t calls = 0; // call instructions
	// Current calls into the wave's component, refreshed with this shape.
	uint32_t self_calls = 0, component_calls = 0;
	bool loop = false;
	bool loop_dispatch = false; // an indirect call outside cold blocks inside one
								// of its natural loops
	bool unsafe = false;
	bool unwinds = false;
	bool fast_return = false;     // a call-free path from the entry reaches a return
	bool allocates = false;       // hot code calls an allocation function (a growth path)
	bool constant_result = false; // every normal return is the same integer literal
};

uint32_t call_site_cost(const Instruction& call)
{
	return 6 + call.extra_count;
}

// Conservative scalar estimate. Unknown/bulk operations are unpriced rather
// than silently treated as one scalar instruction.
uint32_t inline_instruction_cost(const Instruction& inst)
{
	switch (inst.op)
	{
	case Opcode::Return:
	case Opcode::Unreachable:
	case Opcode::Copy:
		return 0;
	case Opcode::Jump:
	case Opcode::Branch:
	case Opcode::Phi:
		return 1;
	case Opcode::Call:
		return call_site_cost(inst);
	case Opcode::Index:
	{
		const uint64_t n = storage_bytes(inst.type);
		return inst.ops[1].is(ValueKind::Integer) || n == 1 || n == 2 || n == 4 || n == 8 ? 1 : 3;
	}
	case Opcode::Switch:
		return 2 + inst.extra_count;
	case Opcode::Vector:
		switch (lowir::shape_of(static_cast<VectorOp>(inst.sub)))
		{
		case VectorShape::Binary:
		case VectorShape::BinaryImmediate:
		case VectorShape::UnaryImmediate:
			return 2;
		case VectorShape::Blend:
		case VectorShape::Test:
			return 3;
		case VectorShape::FromScalar:
			return static_cast<VectorOp>(inst.sub) == VectorOp::Fromf32 ? 3 : 1;
		case VectorShape::ToScalar:
		case VectorShape::Zero:
		case VectorShape::Unary:
		case VectorShape::Mask:
			return 1;
		}
		return UINT32_MAX;

	case Opcode::CopyObject:
	case Opcode::ZeroInit:
		if (inst.immediate < 0 || inst.immediate > 64)
			return UINT32_MAX;
		return uint32_t((inst.immediate + 7) / 8) * (inst.op == Opcode::CopyObject ? 2 : 1) +
			(inst.immediate != 0 ? 1 : 0);

	default:
		break;
	}

	if (inst.type.kind() == TypeKind::V128)
		return inst.op == Opcode::Load || inst.op == Opcode::Store ? 1 : UINT32_MAX;
	if (inst.type.kind() == TypeKind::F32 || inst.type.kind() == TypeKind::F64)
	{
		switch (inst.op)
		{
		case Opcode::Const:
		case Opcode::Load:
		case Opcode::Store:
			return 1;
		case Opcode::Compare:
			return 2;
		case Opcode::Unary:
			return 2;
		case Opcode::Binary:
			switch (static_cast<BinaryOp>(inst.sub))
			{
			case BinaryOp::Add:
			case BinaryOp::Sub:
			case BinaryOp::Mul:
				return 1;
			case BinaryOp::Div:
				return 8;
			default:
				return UINT32_MAX;
			}
		case Opcode::Convert:
		{
			const Type from = convert_source_type(inst);
			return from.kind() == TypeKind::F32 || from.kind() == TypeKind::F64 ? 1
				: from.is_integer()                                             ? 6
																				: UINT32_MAX;
		}
		case Opcode::Select:
			return 3;
		default:
			return UINT32_MAX;
		}
	}
	if (inst.type.kind() == TypeKind::I128)
	{
		switch (inst.op)
		{
		case Opcode::Const:
			return 2;
		case Opcode::Load:
		case Opcode::Store:
			return 2;
		case Opcode::Compare:
			return 4;
		case Opcode::Convert:
			return convert_source_type(inst).is_integer() &&
					(static_cast<ConvertOp>(inst.sub) == ConvertOp::ZExt ||
						static_cast<ConvertOp>(inst.sub) == ConvertOp::SExt)
				? 2
				: UINT32_MAX;
		case Opcode::Unary:
			return 3;
		case Opcode::Binary:
			switch (static_cast<BinaryOp>(inst.sub))
			{
			case BinaryOp::And:
			case BinaryOp::Or:
			case BinaryOp::Xor:
				return 2;
			case BinaryOp::Add:
			case BinaryOp::Sub:
				return 3;
			case BinaryOp::Shl:
			case BinaryOp::Shr:
			case BinaryOp::UShr:
				return 4;
			case BinaryOp::Mul:
				return 8;
			default:
				return 24;
			}
		default:
			return UINT32_MAX;
		}
	}
	if (!(inst.type.is_pointer() || (inst.type.is_integer() && inst.type.kind() != TypeKind::I128)))
		return UINT32_MAX;
	switch (inst.op)
	{
	case Opcode::Const:
	case Opcode::Addr:
	case Opcode::Load:
	case Opcode::Store:
	case Opcode::Compare:
		return 1;
	case Opcode::Unary:
		return inst.sub <= static_cast<uint8_t>(UnaryOp::BitNot) ? 1 : 3;
	case Opcode::Binary:
		return inst.sub == static_cast<uint8_t>(BinaryOp::Div) || inst.sub == static_cast<uint8_t>(BinaryOp::Mod) ||
				inst.sub == static_cast<uint8_t>(BinaryOp::UDiv) || inst.sub == static_cast<uint8_t>(BinaryOp::UMod)
			? 8
			: 1;
	case Opcode::Convert:
	{
		const Type from = convert_source_type(inst);
		if (static_cast<ConvertOp>(inst.sub) == ConvertOp::Trunc && (inst.flags & IF_EXACT) != 0)
			return 0;
		if (from.is_pointer() && (inst.type.is_pointer() || inst.type.kind() == TypeKind::I64))
			return 0;
		if (inst.type.is_pointer() && (from.kind() == TypeKind::I64))
			return 0;
		return from.is_integer() ? 1 : from.kind() == TypeKind::F32 || from.kind() == TypeKind::F64 ? 4 : UINT32_MAX;
	}
	case Opcode::Select:
		return 3;
	default:
		return UINT32_MAX;
	}
}

bool cheap_shape_instruction(const Instruction& inst)
{
	return inst.op == Opcode::Copy || inst.op == Opcode::Addr ||
		(inst.op == Opcode::Index && inst.ops[1].is(ValueKind::Integer));
}

uint32_t addressed_slot(const FunctionBody& body, Value v);

bool phi_type(Type type)
{
	return type.is_pointer() || type.kind() == TypeKind::F32 || type.kind() == TypeKind::F64 ||
		(type.is_integer() && type.kind() != TypeKind::I128);
}

// Whether block `b` is a terminate guard's landing, as a noexcept function's
// is: a catch-all, the exception read, a call that neither returns nor
// unwinds, and `unreachable`. Nothing it catches leaves it.
bool guard_landing(const Program& program, const FunctionBody& body, BlockId b)
{
	const Instruction* inst[4] = {};
	InstId i = body.blocks[b].first;
	for (unsigned k = 0; k < 4; ++k)
	{
		if (i == kNoId)
			return false;
		inst[k] = &body.instructions[i];
		i = inst[k]->next;
	}
	return i == kNoId && inst[0]->op == Opcode::EhCatchAll && inst[1]->op == Opcode::Exception &&
		inst[2]->op == Opcode::Call && inst[3]->op == Opcode::Unreachable && program.may_end_block(*inst[2]) &&
		!may_unwind(program, *inst[2]);
}

// One summary for an entry predicate directly comparing a parameter with a
// literal. Call sites only evaluate that predicate; no callee walk or clone.
template<class Price>
void summarize_entry_cost(const FunctionBody& body, const Cfg& cfg, const std::vector<uint8_t>& cold,
	const std::vector<uint8_t>& rare, const Price& price, EntryCost& result)
{
	const BlockId entry = body.layout[0];
	if (cfg.pred_count(entry) != 0 || body.blocks[entry].last == kNoId)
		return;
	const Instruction& branch = body.instructions[body.blocks[entry].last];
	if (branch.op != Opcode::Branch || !branch.ops[0].is(ValueKind::Temp) || branch.ops[1] == branch.ops[2])
		return;
	const TempId condition = branch.ops[0].index;
	EntryCost candidate;
	if (condition < body.parameter_count && body.temps[condition].type.is_integer())
	{
		candidate.parameter = condition;
		candidate.type = body.temps[condition].type;
		candidate.literal = integer_constant(candidate.type, Literal{ 0, 0 });
	}
	else
	{
		const InstId def = body.temps[condition].def;
		if (def == kNoId)
			return;
		const Instruction& compare = body.instructions[def];
		if (compare.op != Opcode::Compare || !compare.type.is_integer())
			return;
		for (unsigned side = 0; side != 2; ++side)
			if (compare.ops[side].is(ValueKind::Temp) && compare.ops[side].index < body.parameter_count &&
				literal_constant(body, compare.ops[1 - side], candidate.literal))
			{
				candidate.parameter = compare.ops[side].index;
				candidate.type = compare.type;
				candidate.predicate = static_cast<ComparePredicate>(compare.sub);
				candidate.parameter_left = side == 0;
				break;
			}
		if (candidate.parameter == kNoId)
			return;
	}
	// Two reachability bits traverse each edge at most twice, reusing the
	// CFG already constructed for the callee's shape. Backedges to entry
	// were rejected above, so they cannot change the chosen entry edge.
	std::vector<uint8_t> reachable(cfg.block_count, 0);
	std::vector<BlockId> work;
	reachable[entry] = 3;
	for (unsigned side = 0; side != 2; ++side)
	{
		const BlockId target = branch.ops[side == 0 ? 2 : 1].index;
		reachable[target] |= uint8_t(1u << side);
		work.push_back(target);
	}
	while (!work.empty())
	{
		const BlockId block = work.back();
		work.pop_back();
		for (uint32_t k = cfg.succ_first[block]; k != cfg.succ_first[block + 1]; ++k)
		{
			const BlockId next = cfg.succs[k];
			const uint8_t bits = reachable[next] | reachable[block];
			if (bits != reachable[next])
			{
				reachable[next] = bits;
				work.push_back(next);
			}
		}
	}
	uint32_t returns[2] = {};
	for (BlockId block : cfg.rpo)
	{
		if (reachable[block] == 0 || cold[block])
			continue;
		uint32_t cost = 0, unknown = 0, exits = 0;
		for (InstId i = body.blocks[block].first; i != kNoId; i = body.instructions[i].next)
		{
			const uint32_t part = price(body.instructions[i]);
			if (part == UINT32_MAX)
				++unknown;
			else
				cost += part;
			exits += body.instructions[i].op == Opcode::Return ? 1 : 0;
		}
		if (!rare.empty() && rare[block])
			cost = cost / 4 + (cost % 4 != 0);
		for (unsigned side = 0; side != 2; ++side)
			if ((reachable[block] & (1u << side)) != 0)
			{
				candidate.hot[side] += cost;
				candidate.unpriced[side] += unknown;
				returns[side] += exits;
			}
	}
	for (unsigned side = 0; side != 2; ++side)
		if (returns[side] > 1)
			candidate.hot[side] += 2 * (returns[side] - 1);
	result = candidate;
}

void call_cost_after_constants(
	const Shape& shape, const FunctionBody& caller, const Instruction& call, uint32_t& hot, uint32_t& unpriced)
{
	hot = shape.hot_cost;
	unpriced = shape.hot_unpriced;
	const EntryCost& entry = shape.entry_cost;
	if (entry.parameter == kNoId || entry.parameter >= call.extra_count)
		return;
	Constant argument, condition;
	if (!literal_constant(caller, caller.operands[call.extra_first + entry.parameter], argument))
		return;
	if (!fold_compare(entry.predicate, entry.type, entry.parameter_left ? argument : entry.literal,
			entry.parameter_left ? entry.literal : argument, condition))
		return;
	const unsigned side = condition.bits.low != 0 ? 1 : 0;
	hot = entry.hot[side];
	unpriced = entry.unpriced[side];
}

template<bool source_required = false>
Shape shape_of(
	const Program& program, SymbolId symbol, const std::vector<uint8_t>& nounwind, const CallGraph* graph = nullptr)
{
	Shape shape;
	const uint32_t component = graph != nullptr ? graph->component[graph->function_of(symbol)] : kNoId;
	const Function& function = program.functions[program.symbols[symbol].detail];
	const FunctionBody& body = *function.body;
	const Signature& sig = program.signatures[function.signature];
	// An ellipsis alone needs no callee frame. Only operations that observe
	// its argument area prevent substitution (checked below).
	shape.unsafe = body.layout.empty() || !body.code_labels.empty();
	shape.constant_body = !source_required && body.layout.size() == 1 && body.parameter_count <= 64;
	bool unwinding = false, unguarded = false;
	bool has_predictions = false;
	bool same_result = !source_required && sig.result.is_integer();
	uint32_t result_literal = kNoId;
	uint32_t cheap = 0;
	std::vector<TempId> address_candidates;
	for (BlockId b : body.layout)
	{
		++shape.blocks;
		bool guard = false;
		for (InstId i = body.blocks[b].first; i != kNoId; i = body.instructions[i].next)
		{
			const Instruction& inst = body.instructions[i];
			++shape.size;
			if (!source_required && inst.result != kNoId)
			{
				bool candidate = inst.op == Opcode::Addr && inst.ops[0].is(ValueKind::Slot);
				if (inst.op == Opcode::Index && inst.ops[1].is(ValueKind::Integer) &&
					(inst.ops[0].is(ValueKind::Slot) ||
						(inst.ops[0].is(ValueKind::Temp) && body.temps[inst.ops[0].index].type.is_pointer())))
				{
					const int64_t k = static_cast<int64_t>(body.literals[inst.ops[1].index].low);
					const uint64_t n = storage_bytes(inst.type);
					candidate = n <= 1024 && k >= -(int64_t(1) << 20) && k <= (int64_t(1) << 20);
				}
				if (candidate)
					address_candidates.push_back(inst.result);
			}

			has_predictions = has_predictions ||
				(inst.op == Opcode::Branch && inst.sub != static_cast<uint8_t>(BranchPrediction::None));
			if (shape.constant_body)
			{
				switch (inst.op)
				{
				case Opcode::Const:
				case Opcode::Copy:
				case Opcode::Unary:
				case Opcode::Binary:
				case Opcode::Compare:
				case Opcode::Convert:
				case Opcode::Select:
				case Opcode::Return:
					break;
				default:
					shape.constant_body = false;
					break;
				}
				if (shape.constant_body)
					for_each_operand(body, inst, [&](const Value& v) {
						if (v.is(ValueKind::Slot))
							shape.constant_body = false;
						if (v.is(ValueKind::Temp) && v.index < body.parameter_count)
							shape.constant_parameters |= uint64_t(1) << v.index;
					});
			}

			if (!source_required)
			{
				const uint32_t cost = inline_instruction_cost(inst);
				if (cost == UINT32_MAX)
					++shape.unpriced;
				else
					shape.cost += cost;
			}
			shape.eh = shape.eh || is_eh_instruction(inst.op);
			const bool catching = inst.op == Opcode::EhCatch || inst.op == Opcode::EhFilter ||
				inst.op == Opcode::EhCatchAll || inst.op == Opcode::ExceptionSelector || inst.op == Opcode::Exception ||
				inst.op == Opcode::Throw;
			if (catching && i == body.blocks[b].first)
				guard = guard_landing(program, body, b);
			unguarded = unguarded || (catching && !guard);
			shape.catches = shape.catches || catching;
			shape.call = shape.call || inst.op == Opcode::Call;
			shape.calls += inst.op == Opcode::Call ? 1 : 0;
			if (graph != nullptr && inst.op == Opcode::Call && inst.ops[0].is(ValueKind::Symbol))
			{
				const uint32_t target = graph->function_of(inst.ops[0].index);
				shape.self_calls += inst.ops[0].index == symbol ? 1 : 0;
				shape.component_calls += target != kNoId && graph->component[target] == component ? 1 : 0;
			}
			shape.returns += inst.op == Opcode::Return ? 1 : 0;
			if (inst.op == Opcode::Return && same_result)
			{
				const Value value = inst.ops[0];
				if (value.kind != ValueKind::Integer)
					same_result = false;
				else
				{
					if (result_literal != kNoId)
					{
						const Literal& first = body.literals[result_literal];
						const Literal& next = body.literals[value.index];
						same_result = first.low == next.low && first.high == next.high;
					}
					result_literal = value.index;
				}
			}
			unwinding = unwinding ||
				(!source_required &&
					(may_unwind(program, inst) &&
						!(inst.op == Opcode::Call && inst.ops[0].is(ValueKind::Symbol) &&
							nounwind[inst.ops[0].index])));
			if (inst.op == Opcode::VaStart || inst.op == Opcode::VaArg ||
				(!source_required && is_stack_instruction(inst.op)) || (b == body.layout[0] && inst.op == Opcode::Phi))
				shape.unsafe = true;
		}
	}

	std::vector<uint8_t> folded_addresses;
	if (!address_candidates.empty())
	{
		folded_addresses.assign(body.temps.size(), 0);
		for (TempId t : address_candidates)
			folded_addresses[t] = 1;
		for (BlockId b : body.layout)
			for (InstId i = body.blocks[b].first; i != kNoId; i = body.instructions[i].next)
			{
				const Instruction& inst = body.instructions[i];
				for_each_operand(body, inst, [&](const Value& v) {
					if (!v.is(ValueKind::Temp) || folded_addresses[v.index] == 0)
						return;
					const bool address = (inst.op == Opcode::Load && v == inst.ops[0]) ||
						(inst.op == Opcode::Store && v == inst.ops[1] && !(v == inst.ops[0]));
					folded_addresses[v.index] = address && folded_addresses[v.index] == 1 ? 2 : 3;
				});
			}
		for (TempId t : address_candidates)
		{
			folded_addresses[t] = folded_addresses[t] == 2 ? 1 : 0;
			shape.cost -= folded_addresses[t];
		}
	}
	auto priced_instruction = [&](const Instruction& inst) {
		return inst.result != kNoId && !folded_addresses.empty() && folded_addresses[inst.result] != 0
			? uint32_t(0)
			: inline_instruction_cost(inst);
	};
	if (shape.returns > 1)
		shape.cost += 2 * (shape.returns - 1);
	shape.constant_result = same_result && shape.returns != 0;
	shape.unwinds = unwinding && sig.boundary.unwind != ir_model::CUM_NO && !nounwind[symbol];
	shape.guarded = shape.catches && !unguarded;
	shape.hot = shape.size;
	shape.hot_cost = shape.cost;
	shape.hot_unpriced = shape.unpriced;
	shape.hot_call = shape.call;
	// Required substitution has no profitability policy: loops, cold paths
	// and dominators cannot change its admission. Keep only cloning safety.
	if (source_required)
		return shape;
	if (shape.blocks > 1)
	{
		static Cfg scratch; // (reused for its capacity: every build overwrites what
							// it fills)
		Cfg& cfg = scratch;
		build_cfg_light(program, body, cfg); // dominators only for find_loops
		for (BlockId b : cfg.rpo)
			for (uint32_t k = cfg.succ_first[b]; k < cfg.succ_first[b + 1] && !shape.loop; ++k)
				if (cfg.reachable(cfg.succs[k]) && cfg.rpo_index[cfg.succs[k]] <= cfg.rpo_index[b])
					shape.loop = true;
		// Cold blocks: a noreturn call ends them, or every successor is
		// cold (reverse postorder backwards: successors first, loops stay
		// hot).
		std::vector<uint8_t> cold(cfg.block_count, 0);
		for (uint32_t r = static_cast<uint32_t>(cfg.rpo.size()); r-- > 0;)
		{
			const BlockId b = cfg.rpo[r];
			bool ends = false;
			for (InstId i = body.blocks[b].first; i != kNoId && !ends; i = body.instructions[i].next)
				ends = body.instructions[i].op == Opcode::Call && program.may_end_block(body.instructions[i]);
			bool all = cfg.succ_count(b) > 0;
			for (uint32_t k = cfg.succ_first[b]; k < cfg.succ_first[b + 1]; ++k)
				all = all && cold[cfg.succs[k]] && cfg.rpo_index[cfg.succs[k]] > r;
			cold[b] = (ends || all) && b != body.layout[0];
			if (cold[b])
				for (InstId i = body.blocks[b].first; i != kNoId; i = body.instructions[i].next)
				{
					--shape.hot;
					const uint32_t cost = priced_instruction(body.instructions[i]);
					if (cost == UINT32_MAX)
						--shape.hot_unpriced;
					else
						shape.hot_cost -= cost;
				}
		}

		// Reuse the callee CFG. A unique predicted edge starts a rare
		// region; forward joins inherit rarity only from all predecessors.
		// Keep one quarter of its known static cost for copied code size.
		// Terminal cold blocks were already removed from hot_cost above.
		std::vector<uint8_t> rare;
		if (has_predictions)
		{
			rare = cold;
			for (BlockId b : cfg.rpo)
			{
				if (b == body.layout[0])
					continue;
				bool inherited = cfg.pred_count(b) != 0;
				for (uint32_t k = cfg.pred_first[b]; k < cfg.pred_first[b + 1] && inherited; ++k)
					inherited = rare[cfg.preds[k]] && cfg.rpo_index[cfg.preds[k]] < cfg.rpo_index[b];
				if (inherited)
					rare[b] = 1;
				if (cfg.pred_count(b) == 1)
				{
					const BlockId from = cfg.preds[cfg.pred_first[b]];
					const InstId last = body.blocks[from].last;
					if (last != kNoId)
					{
						const Instruction& branch = body.instructions[last];
						if (branch.op == Opcode::Branch && branch.ops[1].index != branch.ops[2].index)
						{
							const BranchPrediction prediction = static_cast<BranchPrediction>(branch.sub);
							if ((prediction == BranchPrediction::Unlikely && branch.ops[1].index == b) ||
								(prediction == BranchPrediction::Likely && branch.ops[2].index == b))
								rare[b] = 1;
						}
					}
				}
				if (rare[b] && !cold[b])
				{
					uint32_t cost = 0;
					for (InstId i = body.blocks[b].first; i != kNoId; i = body.instructions[i].next)
					{
						const uint32_t part = priced_instruction(body.instructions[i]);
						if (part != UINT32_MAX)
							cost += part;
					}
					shape.hot_cost -= cost - (cost / 4 + (cost % 4 != 0));
				}
			}
		}
		summarize_entry_cost(body, cfg, cold, rare, priced_instruction, shape.entry_cost);
		shape.hot_call = false;
		for (BlockId b : cfg.rpo)
			for (InstId i = body.blocks[b].first; i != kNoId && !cold[b]; i = body.instructions[i].next)
			{
				const Instruction& inst = body.instructions[i];
				cheap += !shape.eh && cheap_shape_instruction(inst) ? 1 : 0;
				shape.hot_call = shape.hot_call || inst.op == Opcode::Call;
				shape.allocates = shape.allocates ||
					(inst.op == Opcode::Call && inst.ops[0].is(ValueKind::Symbol) &&
						program.symbols[inst.ops[0].index].metadata.role == ir_model::SR_ALLOCATE_MEMORY);
			}
		// The call-free region: blocks reached from the entry through
		// blocks without calls.
		std::vector<uint8_t> region(cfg.block_count, 0);
		std::vector<BlockId> work;
		auto calls = [&](BlockId b) {
			for (InstId i = body.blocks[b].first; i != kNoId; i = body.instructions[i].next)
				if (body.instructions[i].op == Opcode::Call)
					return true;
			return false;
		};
		if (!calls(body.layout[0]))
		{
			region[body.layout[0]] = 1;
			work.push_back(body.layout[0]);
		}
		while (!work.empty())
		{
			const BlockId b = work.back();
			work.pop_back();
			for (InstId i = body.blocks[b].first; i != kNoId; i = body.instructions[i].next)
				shape.fast_return = shape.fast_return || body.instructions[i].op == Opcode::Return;
			for (uint32_t k = cfg.succ_first[b]; k < cfg.succ_first[b + 1]; ++k)
			{
				const BlockId s = cfg.succs[k];
				if (!region[s] && cfg.reachable(s) && !calls(s))
				{
					region[s] = 1;
					work.push_back(s);
				}
			}
		}
	}
	if (shape.blocks == 1)
		for (InstId i = body.blocks[body.layout[0]].first; i != kNoId; i = body.instructions[i].next)
			cheap += !shape.eh && cheap_shape_instruction(body.instructions[i]) ? 1 : 0;
	if (!shape.eh && !shape.hot_call)
		shape.hot -= cheap / 2;
	return shape;
}

// A result phis cannot carry (an object, i128 or f80): the value every
// return names when they agree, else a fresh merge slot each return block
// fills (copyobj for an object, a store otherwise) and the continuation
// reads (the slot as the object value, or a load).
// An object result whose one use is a whole copy starting the
// continuation (`copyobj %result, d` right after the call): each return
// copies into d itself, which nothing reads or writes in between, and the
// copy goes (no merge slot copied again: the copies of a small object built
// field by field read back across the merge with wider loads). Returns the
// copy's instruction, or kNoId.
InstId result_copy(const FunctionBody& cb, BlockId cont, TempId result, Type type)
{
	const InstId first = cb.blocks[cont].first;
	if (first == kNoId)
		return kNoId;
	const Instruction& copy = cb.instructions[first];
	if (copy.op != Opcode::CopyObject || (copy.flags & IF_VOLATILE) != 0 || !(copy.ops[0] == Value::temp(result)) ||
		copy.ops[1] == Value::temp(result) || copy.immediate != static_cast<int64_t>(type.bytes))
		return kNoId;
	uint32_t uses = 0;
	for (const Instruction& inst : cb.instructions)
		for_each_operand(cb, inst, [&](const Value& v) { uses += v == Value::temp(result) ? 1 : 0; });
	return uses == 1 ? first : kNoId;
}

// Keep a two-word inline result in scalar SSA across its return edges.
// Recreating an i128 merge slot after aggregate promotion hides both words
// from the following simplification and inliner waves.
bool merge_wide_returns(
	FunctionBody& body, TempId result, const std::vector<std::pair<BlockId, Value>>& returns, BlockId cont)
{
	for (const auto& r : returns)
		if (!r.second.is(ValueKind::Temp) && !r.second.is(ValueKind::Integer) && !r.second.is(ValueKind::Slot))
			return false;
	const Type word = Type::scalar(TypeKind::I64), wide = Type::scalar(TypeKind::I128);
	std::vector<Value> pairs[2];
	BlockId block = cont;
	InstId prev = kNoId;
	const auto emit = [&](Instruction inst) {
		const InstId id = append_instruction(body, inst);
		body.instructions[id].result = append_temp(body, inst.type, id);
		insert_after(body, block, prev, id);
		prev = id;
		return Value::temp(body.instructions[id].result);
	};
	const auto convert = [&](ConvertOp op, Type to, Type from, Value v) {
		Instruction inst = make_instruction(Opcode::Convert, to);
		inst.sub = static_cast<uint8_t>(op);
		set_convert_source_type(inst, from);
		inst.ops[0] = v;
		return emit(inst);
	};
	const auto binary = [&](BinaryOp op, Value a, Value b) {
		Instruction inst = make_instruction(Opcode::Binary, wide);
		inst.sub = static_cast<uint8_t>(op);
		inst.ops[0] = a;
		inst.ops[1] = b;
		return emit(inst);
	};
	const Value shift = integer_value(body, Literal{ 64, 0 });
	for (const auto& r : returns)
	{
		block = r.first;
		prev = previous_in_block(body, block, body.blocks[block].last);
		Value parts[2];
		Value value = r.second;
		if (value.is(ValueKind::Integer))
		{
			const Literal literal = body.literals[value.index];
			parts[0] = integer_value(body, integer_literal(static_cast<int64_t>(literal.low)));
			parts[1] = integer_value(body, integer_literal(static_cast<int64_t>(literal.high)));
		}
		else
		{
			if (value.is(ValueKind::Slot))
			{
				Instruction load = make_instruction(Opcode::Load, wide);
				load.ops[0] = value;
				value = emit(load);
			}
			parts[0] = convert(ConvertOp::Trunc, word, wide, value);
			parts[1] = convert(ConvertOp::Trunc, word, wide, binary(BinaryOp::UShr, value, shift));
		}
		for (unsigned half = 0; half < 2; ++half)
		{
			pairs[half].push_back(Value::block(block));
			pairs[half].push_back(parts[half]);
		}
	}
	block = cont;
	prev = kNoId;
	Value words[2];
	for (unsigned half = 0; half < 2; ++half)
	{
		Instruction phi = make_instruction(Opcode::Phi, word);
		phi.extra_first = append_operands(body, pairs[half].data(), static_cast<uint32_t>(pairs[half].size()));
		phi.extra_count = static_cast<uint32_t>(pairs[half].size());
		words[half] = emit(phi);
	}
	const Value lo = convert(ConvertOp::ZExt, wide, word, words[0]);
	const Value hi = convert(ConvertOp::ZExt, wide, word, words[1]);
	const Value merged = binary(BinaryOp::Or, lo, binary(BinaryOp::Shl, hi, shift));
	// Reuse the call's result identity. The temporary made by emit is dead.
	body.instructions[prev].result = result;
	body.temps[result].def = prev;
	body.temps[merged.index].def = kNoId;
	return true;
}

void merge_returns(FunctionBody& cb, TempId result, Type type, const std::vector<std::pair<BlockId, Value>>& returns,
	std::vector<Value>& repl)
{
	bool same = true;
	for (const std::pair<BlockId, Value>& r : returns)
		same = same && r.second == returns[0].second;
	if (repl.size() <= result)
		repl.resize(cb.temps.size(), Value::none());
	if (same)
	{
		repl[result] = returns[0].second;
		return;
	}
	const BlockId cont = cb.instructions[cb.blocks[returns[0].first].last].ops[0].index;
	if (type.kind() == TypeKind::I128 && merge_wide_returns(cb, result, returns, cont))
		return;
	const InstId into = type.is_object() ? result_copy(cb, cont, result, type) : kNoId;
	const SlotId merge = static_cast<SlotId>(cb.slots.size());
	if (into == kNoId)
		cb.slots.push_back(Slot{ 0, type.is_object() ? SlotKind::Retmergeobj : SlotKind::Retmerge, 0, 0, type });
	for (const std::pair<BlockId, Value>& r : returns)
	{
		const InstId jump = cb.blocks[r.first].last;
		const InstId before = previous_in_block(cb, r.first, jump);
		InstId at = before;
		if (type.is_object() && into != kNoId)
		{
			Instruction copy = cb.instructions[into];
			copy.ops[0] = r.second;
			copy.next = kNoId;
			insert_after(cb, r.first, at, append_instruction(cb, copy));
		}
		else if (type.is_object())
		{
			Instruction addr = make_instruction(Opcode::Addr, Type::scalar(TypeKind::Ptr));
			addr.ops[0] = Value::slot(merge);
			const InstId a = append_instruction(cb, addr);
			cb.instructions[a].result = append_temp(cb, Type::scalar(TypeKind::Ptr), a);
			insert_after(cb, r.first, at, a);
			Instruction copy = make_instruction(Opcode::CopyObject, Type::scalar(TypeKind::Void));
			copy.ops[0] = r.second;
			copy.ops[1] = Value::temp(cb.instructions[a].result);
			copy.immediate = static_cast<int64_t>(type.bytes);
			copy.sub = static_cast<uint8_t>(type.align_log2);
			const InstId c = append_instruction(cb, copy);
			insert_after(cb, r.first, a, c);
		}
		else
		{
			Instruction store = make_instruction(Opcode::Store, type);
			store.ops[0] = r.second;
			store.ops[1] = Value::slot(merge);
			insert_after(cb, r.first, at, append_instruction(cb, store));
		}
	}
	if (type.is_object())
	{
		if (into != kNoId)
			unlink_after(cb, cont, kNoId, into);
		else
			repl[result] = Value::slot(merge);
		return;
	}
	// The continuation (every return's jump target) loads the merge.
	Instruction load = make_instruction(Opcode::Load, type);
	load.ops[0] = Value::slot(merge);
	load.result = result;
	const InstId l = append_instruction(cb, load);
	cb.temps[result].def = l;
	insert_after(cb, cont, kNoId, l);
}

// Partial inlining (O2+, the late wave): the callee copied without the
// code behind its cut blocks, where the copy calls the callee instead
// (from its entry, with the same arguments). That is the same when every
// path from the entry to a cut block is free of side effects (clean). A
// block holding a call is cut at its shallowest clean dominator below the
// entry that reaches no return without a call (or stays in the copy): the
// copy keeps the returning fast paths and none of a slow path's prefix.
// A copy over `cap` is not made. (Cutting at the shallowest clean
// dominator instead, to copy the first test only, mostly copied a rarely
// taken call-free path and kept the call on the common one: ParserState::at
// copied its replayed-token read, and every call read the window.) A copy
// with a loop is not made either: the call behind a cut repeats the loop.
struct PartialCut
{
	std::vector<uint8_t> copy, cut;
	uint32_t size = 0; // copied instructions, and a call and a jump per cut block
	uint32_t cuts = 0;
};

// The copied region from the cut blocks: `cut` marks every cut, and ends
// with the reached ones only.
void partial_region(const FunctionBody& kb, const Cfg& cfg, PartialCut& out, bool& returns)
{
	const BlockId entry = kb.layout[0];
	out.copy.assign(cfg.block_count, 0);
	out.size = out.cuts = 0;
	returns = false;
	std::vector<BlockId> work(1, entry);
	out.copy[entry] = 1;
	while (!work.empty())
	{
		const BlockId b = work.back();
		work.pop_back();
		for (InstId i = kb.blocks[b].first; i != kNoId; i = kb.instructions[i].next)
		{
			++out.size;
			returns = returns || kb.instructions[i].op == Opcode::Return;
		}
		for (uint32_t k = cfg.succ_first[b]; k < cfg.succ_first[b + 1]; ++k)
		{
			const BlockId s = cfg.succs[k];
			if (out.copy[s] || out.cut[s] == 2)
				continue;
			if (out.cut[s])
			{
				out.cut[s] = 2;
				++out.cuts;
				out.size += 2;
				continue;
			}
			out.copy[s] = 1;
			work.push_back(s);
		}
	}
	for (uint8_t& c : out.cut)
		c = c == 2 ? 1 : 0;
}

bool partial_cut(const Program& program, const FunctionBody& kb, PartialCut& out, uint32_t cap)
{
	static Cfg scratch; // (reused for its capacity: every build overwrites what
						// it fills)
	Cfg& cfg = scratch;
	build_cfg(program, kb, cfg);
	const uint32_t n = cfg.block_count;
	std::vector<uint8_t> side(n, 0), calls(n, 0), fast(n, 0);
	for (BlockId b : kb.layout)
	{
		bool returns = false;
		for (InstId i = kb.blocks[b].first; i != kNoId; i = kb.instructions[i].next)
		{
			const Instruction& inst = kb.instructions[i];
			calls[b] = calls[b] || inst.op == Opcode::Call;
			returns = returns || inst.op == Opcode::Return;
			if (!is_pure(inst) && !is_terminator(inst.op) &&
				!(inst.op == Opcode::Load && (inst.flags & IF_VOLATILE) == 0))
				side[b] = 1;
		}
		// (A call block's return follows its call: no fast path.)
		fast[b] = returns && !calls[b];
	}
	// Clean blocks: the greatest solution, forward.
	std::vector<uint8_t> clean(n, 1);
	for (bool changed = true; changed;)
	{
		changed = false;
		for (BlockId b : cfg.rpo)
		{
			bool all = true;
			for (uint32_t k = cfg.pred_first[b]; k < cfg.pred_first[b + 1]; ++k)
				all = all && (!cfg.reachable(cfg.preds[k]) || (clean[cfg.preds[k]] && !side[cfg.preds[k]]));
			if (!all && clean[b])
			{
				clean[b] = 0;
				changed = true;
			}
		}
	}
	// Fast blocks reach a return through call-free blocks: backward.
	for (bool changed = true; changed;)
	{
		changed = false;
		for (size_t r = cfg.rpo.size(); r-- > 0;)
		{
			const BlockId b = cfg.rpo[r];
			if (fast[b] || calls[b])
				continue;
			for (uint32_t k = cfg.succ_first[b]; k < cfg.succ_first[b + 1] && !fast[b]; ++k)
				fast[b] = fast[cfg.succs[k]];
			changed = changed || fast[b];
		}
	}
	const BlockId entry = kb.layout[0];
	bool returns = false;
	out.cut.assign(n, 0);
	for (BlockId c : cfg.rpo)
	{
		if (!calls[c])
			continue;
		BlockId at = kNoId;
		for (BlockId d = c; d != kNoId && d != entry; d = cfg.idom[d])
			if (clean[d] && !fast[d] && (cfg.flags[d] & BF_HANDLER) == 0)
				at = d;
		if (at != kNoId)
			out.cut[at] = 1;
	}
	partial_region(kb, cfg, out, returns);
	if (out.cuts == 0 || !returns || out.size > cap)
		return false;
	// A loop in the copy would run again in the call behind a cut.
	std::vector<uint32_t> order(n, 0);
	for (uint32_t r = 0; r < cfg.rpo.size(); ++r)
		order[cfg.rpo[r]] = r;
	for (BlockId b : cfg.rpo)
		for (uint32_t k = cfg.succ_first[b]; out.copy[b] && k < cfg.succ_first[b + 1]; ++k)
			if (out.copy[cfg.succs[k]] && order[cfg.succs[k]] <= order[b])
				return false;
	return true;
}

// An already-simplified body that only returns has no frame or local work
// to clone. Arguments were evaluated before the call; keep their effects.
bool inline_return(
	FunctionBody& body, BlockId block, InstId prev, InstId call_id, const FunctionBody& callee, InstId& tail)
{
	if (callee.layout.size() != 1)
		return false;
	const Instruction& returned = callee.instructions[callee.blocks[callee.layout[0]].first];
	if (returned.op != Opcode::Return)
		return false;
	const Instruction call = body.instructions[call_id];
	if (call.result == kNoId)
	{
		unlink_after(body, block, prev, call_id);
		tail = prev;
		return true;
	}
	Value value = returned.ops[0];
	if (!phi_type(call.type))
		return false;
	if (value.is(ValueKind::Temp))
	{
		if (value.index >= callee.parameter_count || callee.temps[value.index].type != call.type)
			return false;
		value = body.operands[call.extra_first + value.index];
		if (!value.is(ValueKind::Temp) && !value.is(ValueKind::Integer) && !value.is(ValueKind::Float))
			return false;
	}
	else if (value.is(ValueKind::Integer) || value.is(ValueKind::Float))
		value = literal_value(body, value.kind, value.float_kind, callee.literals[value.index]);
	else
		return false;
	Instruction copy = make_instruction(Opcode::Copy, call.type);
	copy.ops[0] = value;
	copy.result = call.result;
	copy.location = call.location;
	copy.next = call.next;
	body.instructions[call_id] = copy;
	tail = call_id;
	return true;
}

// Copies callee `kb` into `cb` in place of the call `call_id` (in `block`,
// after `prev`): the block is split at the call, the callee's blocks go
// between the two halves (in `layout_next`, the caller's layout as a
// list by block), returns jump to the continuation and merge the result in
// a phi there (or, for a value phis cannot carry, its one return's value
// replaces the result: recorded in `repl`, applied once per caller).
InstId clone_call(Program& program, SymbolId caller, SymbolId source, FunctionBody& cb, BlockId block, InstId prev,
	InstId call_id, const FunctionBody& kb, std::vector<BlockId>& layout_next, std::vector<Value>& repl,
	const PartialCut* partial = nullptr, bool straight = false)
{
	const Instruction call = cb.instructions[call_id];
	const Instruction& last = kb.instructions[kb.blocks[kb.layout.back()].last];
	const bool flat = straight && partial == nullptr && kb.layout.size() == 1 && last.op == Opcode::Return &&
		(call.result == kNoId ||
			(phi_type(call.type) && !last.ops[0].is(ValueKind::Slot) && !last.ops[0].is(ValueKind::Symbol) &&
				(!last.ops[0].is(ValueKind::Temp) || kb.temps[last.ops[0].index].type == call.type)));
	if (!flat && layout_next.empty())
	{
		layout_next.assign(cb.blocks.size(), kNoId);
		for (size_t k = 0; k + 1 < cb.layout.size(); ++k)
			layout_next[cb.layout[k]] = cb.layout[k + 1];
	}
	const InstId old_last = cb.blocks[block].last;
	const BlockId cont = flat ? block : append_block(cb, BlockKind::InlineCont);
	if (!flat)
	{
		cb.blocks[cont].first = call.next;
		cb.blocks[cont].last = call.next == kNoId ? kNoId : cb.blocks[block].last;
	}
	cb.blocks[block].last = prev;
	if (prev == kNoId)
		cb.blocks[block].first = kNoId;
	else
		cb.instructions[prev].next = kNoId;
	cb.instructions[call_id].next = kNoId;
	if (!flat && cb.blocks[cont].last != kNoId)
		for_each_successor(
			cb, cb.instructions[cb.blocks[cont].last], [&](BlockId s) { rename_phi_incoming(cb, s, block, cont); });
	else if (!flat)
	{
		const Instruction unreachable = make_instruction(Opcode::Unreachable, Type::scalar(TypeKind::Void));
		const InstId id = append_instruction(cb, unreachable);
		insert_after(cb, cont, kNoId, id);
	}
	// Temporaries, slots and blocks of the copy; an argument of another
	// type than its parameter (compatible: the same width) is retyped at
	// the end of the call's block.
	static std::vector<Value> temps;
	temps.assign(kb.temps.size(), Value::none());
	const Signature& entry = program.signatures[program.functions[program.symbols[source].detail].signature];
	const Parameter* params = program.parameters_of(entry);
	const Signature& calling = program.signatures[program.functions[program.symbols[caller].detail].signature];
	const Parameter* inputs = program.parameters_of(calling);
	for (TempId t = 0; t < kb.parameter_count; ++t)
	{
		temps[t] = cb.operands[call.extra_first + t];
		bool nonnull = params[t].object_bytes != 0 && params[t].type.is_pointer() && !temps[t].is(ValueKind::Slot);
		if (nonnull && temps[t].is(ValueKind::Temp))
		{
			const TempId argument = temps[t].index;
			const InstId id = cb.temps[argument].def;
			const Instruction* origin = id == kNoId ? nullptr : &cb.instructions[id];
			if ((argument < cb.parameter_count && inputs[argument].object_bytes != 0) ||
				(origin != nullptr &&
					((origin->op == Opcode::Copy && (origin->flags & IF_NONNULL) != 0) ||
						(origin->op == Opcode::Addr && origin->ops[0].is(ValueKind::Slot)))))
				nonnull = false;
		}
		if (!nonnull && (!temps[t].is(ValueKind::Temp) || cb.temps[temps[t].index].type == kb.temps[t].type))
			continue;
		Instruction retype = make_instruction(Opcode::Copy, kb.temps[t].type);
		retype.flags = nonnull ? IF_NONNULL : 0;
		retype.ops[0] = temps[t];
		retype.location = call.location;
		const InstId id = append_instruction(cb, retype);
		cb.instructions[id].result = append_temp(cb, kb.temps[t].type, id);
		insert_after(cb, block, cb.blocks[block].last, id);
		temps[t] = Value::temp(cb.instructions[id].result);
	}
	// (A partial copy maps the copied blocks' results only: their uses are
	// defined there or are parameters, and a caller copying itself would
	// otherwise double its temporaries at each of its call sites.)
	// (A named one's copy prints as an inlined copy of its name.)
	const auto copy_temp = [&](TempId t) {
		const Temp copied = kb.temps[t]; // (kb may be cb: appending moves it)
		const TempId made = append_temp(cb, copied.type, kNoId);
		cb.temps[made].name = copied.name;
		cb.temps[made].kind = copied.name != 0 ? TempKind::Inlined : TempKind::Plain;
		temps[t] = Value::temp(made);
	};
	if (partial == nullptr && !straight)
		for (TempId t = kb.parameter_count; t < kb.temps.size(); ++t)
			copy_temp(t);
	else
		for (BlockId b : kb.layout)
			if (partial == nullptr || partial->copy[b])
				for (InstId i = kb.blocks[b].first; i != kNoId; i = kb.instructions[i].next)
					if (kb.instructions[i].result != kNoId)
						copy_temp(kb.instructions[i].result);
	static std::vector<uint32_t> slots;
	slots.assign(kb.slots.size(), kNoId);
	static std::vector<uint8_t> named;
	named.clear();
	if (partial != nullptr || straight)
	{
		named.assign(kb.slots.size(), 0);
		for (BlockId b : kb.layout)
			if (partial == nullptr || partial->copy[b])
				for (InstId i = kb.blocks[b].first; i != kNoId; i = kb.instructions[i].next)
					for_each_operand(kb, kb.instructions[i], [&](const Value& v) {
						if (v.is(ValueKind::Slot))
							named[v.index] = 1;
					});
	}
	for (SlotId s = 0; s < kb.slots.size(); ++s)
	{
		if ((partial != nullptr || straight) && !named[s])
			continue;
		slots[s] = static_cast<uint32_t>(cb.slots.size());
		// A copy keeps a generated name's kind (a shadow's or range-for
		// variable's with its name: they count in the caller); a named
		// slot's prints as an inlined copy of that name, a parameter's as an
		// inlined one.
		Slot slot = kb.slots[s];
		if (slot.kind == SlotKind::Plain || slot.kind == SlotKind::Parameter)
		{
			slot.name = slot.kind == SlotKind::Plain ? slot.name : 0;
			slot.kind = SlotKind::Inlined;
			slot.index = 0;
		}
		cb.slots.push_back(slot);
	}
	static std::vector<BlockId> blocks;
	blocks.assign(kb.blocks.size(), kNoId);
	for (BlockId b : kb.layout)
		if (partial == nullptr || partial->copy[b] || partial->cut[b])
		{
			// A copy keeps a generated block's kind; the entry's prints as
			// the inlined entry, a named block's as a copy of that name.
			const Block copied = kb.blocks[b]; // (kb may be cb: appending moves it)
			blocks[b] = flat ? block
							 : append_block(cb,
								   copied.kind != BlockKind::Plain ? copied.kind
									   : b == kb.layout[0]         ? BlockKind::InlineEntry
																   : BlockKind::InlineCopy);
			if (copied.kind == BlockKind::Plain && b != kb.layout[0])
				cb.blocks[blocks[b]].name = copied.name;
		}
	static std::vector<uint32_t> locations;
	locations.assign(kb.locations.size() + 1, 0);
	auto map_value = [&](Value v) {
		switch (v.kind)
		{
		case ValueKind::Temp:
			return temps[v.index];
		case ValueKind::Slot:
			return Value::slot(slots[v.index]);
		case ValueKind::Block:
			return Value::block(blocks[v.index]);
		case ValueKind::Integer:
		case ValueKind::Float:
			return literal_value(cb, v.kind, v.float_kind, kb.literals[v.index]);
		default:
			return v;
		}
	};
	static std::vector<std::pair<BlockId, Value>> returns;
	returns.clear();
	static std::vector<Value> extra;
	extra.clear();
	for (BlockId b : kb.layout)
	{
		const BlockId nb = blocks[b];
		if (nb == kNoId)
			continue;
		if (partial != nullptr && partial->cut[b])
		{
			// The callee called from its entry, the result merged as a return.
			Instruction again = call;
			again.next = kNoId;
			const std::vector<Value> args(
				cb.operands.begin() + call.extra_first, cb.operands.begin() + call.extra_first + call.extra_count);
			again.extra_first = args.empty() ? 0 : append_operands(cb, args.data(), call.extra_count);
			const InstId id = append_copy(cb, cb, call_id, again);
			if (call.result != kNoId)
			{
				cb.instructions[id].result = append_temp(cb, cb.temps[call.result].type, id);
				returns.push_back(std::make_pair(nb, Value::temp(cb.instructions[id].result)));
			}
			else
				returns.push_back(std::make_pair(nb, Value::none()));
			insert_after(cb, nb, kNoId, id);
			Instruction leave = make_instruction(Opcode::Jump, Type::scalar(TypeKind::Void));
			leave.ops[0] = Value::block(cont);
			leave.location = call.location;
			insert_after(cb, nb, id, append_instruction(cb, leave));
			continue;
		}
		for (InstId i = kb.blocks[b].first; i != kNoId; i = kb.instructions[i].next)
		{
			Instruction inst = kb.instructions[i];
			for (Value& v : inst.ops)
				v = map_value(v);
			extra.clear();
			for (uint32_t k = 0; k < inst.extra_count; ++k)
			{
				// A partial copy's phi keeps the copied predecessors.
				if (inst.op == Opcode::Phi && partial != nullptr && k % 2 == 0 &&
					!partial->copy[kb.operands[inst.extra_first + k].index])
				{
					++k;
					continue;
				}
				extra.push_back(map_value(kb.operands[inst.extra_first + k]));
			}
			inst.extra_count = static_cast<uint32_t>(extra.size());
			inst.extra_first = inst.extra_count != 0 ? append_operands(cb, extra.data(), inst.extra_count) : 0;
			if (inst.location != 0)
			{
				if (locations[inst.location] == 0)
				{
					lowir::DebugLocation location = kb.locations[inst.location - 1];
					// Inlined instructions remain inside the caller's lexical scope.
					location.scope = call.location != 0 ? cb.locations[call.location - 1].scope : 0;
					cb.locations.push_back(location);
					locations[inst.location] = static_cast<uint32_t>(cb.locations.size());
				}
				inst.location = locations[inst.location];
			}
			if (flat && inst.op == Opcode::Return)
			{
				if (call.result == kNoId)
					break;
				inst.op = Opcode::Copy;
				inst.type = call.type;
				inst.result = kNoId;
				const InstId id = append_copy(cb, kb, i, inst);
				cb.instructions[id].result = call.result;
				cb.temps[call.result].def = id;
				insert_after(cb, nb, cb.blocks[nb].last, id);
				break;
			}
			if (inst.op == Opcode::Return)
			{
				returns.push_back(std::make_pair(nb, inst.ops[0]));
				inst.op = Opcode::Jump;
				inst.type = Type::scalar(TypeKind::Void);
				inst.ops[0] = Value::block(cont);
			}
			if (inst.result != kNoId)
				inst.result = temps[inst.result].index;
			const InstId id = append_copy(cb, kb, i, inst);
			if (inst.result != kNoId)
				cb.temps[inst.result].def = id;
			insert_after(cb, nb, cb.blocks[nb].last, id);
		}
	}
	if (flat)
	{
		const InstId tail = cb.blocks[block].last;
		if (tail == kNoId)
			cb.blocks[block].first = call.next;
		else
			cb.instructions[tail].next = call.next;
		cb.blocks[block].last = call.next == kNoId ? tail : old_last;
		return tail;
	}
	// The call becomes a jump into the copy.
	Instruction jump = make_instruction(Opcode::Jump, Type::scalar(TypeKind::Void));
	jump.ops[0] = Value::block(blocks[kb.layout[0]]);
	jump.location = call.location;
	const InstId jump_id = append_instruction(cb, jump);
	insert_after(cb, block, cb.blocks[block].last, jump_id);
	// Layout: the copy's blocks after `block`, then the continuation.
	layout_next.resize(cb.blocks.size(), kNoId);
	BlockId tail = block;
	const BlockId after = layout_next[block];
	for (BlockId k : kb.layout)
	{
		if (blocks[k] == kNoId)
			continue;
		layout_next[tail] = blocks[k];
		tail = blocks[k];
	}
	layout_next[tail] = cont;
	layout_next[cont] = after;
	if (call.result != kNoId)
	{
		const Type type = cb.temps[call.result].type;
		// A return of a same-width value of another type (`return i64 %p`
		// for a ptr %p) is a retype: a copy in its block before the jump.
		for (std::pair<BlockId, Value>& r : returns)
		{
			// A returned symbol is its address, a slot its value: phis take
			// temporaries and literals only.
			if (phi_type(type) && (r.second.is(ValueKind::Symbol) || r.second.is(ValueKind::Slot)))
			{
				const bool slot = r.second.is(ValueKind::Slot);
				Instruction take = make_instruction(
					slot ? Opcode::Load : Opcode::Addr, slot ? type : Type::scalar(TypeKind::Ptr));
				take.ops[0] = r.second;
				const InstId t = append_instruction(cb, take);
				cb.instructions[t].result = append_temp(cb, take.type, t);
				insert_after(cb, r.first, previous_in_block(cb, r.first, cb.blocks[r.first].last), t);
				r.second = Value::temp(cb.instructions[t].result);
			}
			if (!r.second.is(ValueKind::Temp) || cb.temps[r.second.index].type == type)
				continue;
			Instruction copy = make_instruction(Opcode::Copy, type);
			copy.ops[0] = r.second;
			const InstId c = append_instruction(cb, copy);
			cb.instructions[c].result = append_temp(cb, type, c);
			insert_after(cb, r.first, previous_in_block(cb, r.first, cb.blocks[r.first].last), c);
			r.second = Value::temp(cb.instructions[c].result);
		}
		if (phi_type(type))
		{
			std::vector<Value> pairs;
			for (const std::pair<BlockId, Value>& r : returns)
			{
				pairs.push_back(Value::block(r.first));
				pairs.push_back(phi_input(cb, type, r.second));
			}
			Instruction phi = make_instruction(Opcode::Phi, type);
			phi.result = call.result;
			if (pairs.empty())
			{
				// The callee never returns: the result is never read.
				phi.op = Opcode::Copy;
				Literal zero = { 0, 0 };
				phi.ops[0] = type.is_float() ? literal_value(cb, ValueKind::Float, type.kind(), zero)
											 : integer_value(cb, zero);
			}
			else
			{
				phi.extra_first = append_operands(cb, pairs.data(), static_cast<uint32_t>(pairs.size()));
				phi.extra_count = static_cast<uint32_t>(pairs.size());
			}
			const InstId id = append_instruction(cb, phi);
			cb.temps[call.result].def = id;
			insert_after(cb, cont, kNoId, id);
		}
		else if (!returns.empty())
			merge_returns(cb, call.result, type, returns, repl);
	}
	(void)program;
	return kNoId;
}

class InlineWaveRunner
{
public:
	InlineWaveRunner(Program& program, const OptimizerOptions& options, OptimizerStats& stats, InlineWave wave,
		InlineBudgets& budgets, const std::vector<uint32_t>& original_size, const std::vector<uint8_t>& nounwind,
		const std::function<void(SymbolId)>& simplify, bool source_required = false)
		: program_(program), options_(options), stats_(stats), wave_(wave), budgets_(budgets),
		  original_size_(original_size), nounwind_(nounwind), simplify_(simplify), source_required_(source_required)
	{}

	void run();

private:
	bool compatible(const Instruction& call, SymbolId callee) const;
	bool non_growing(const Shape& shape, const Instruction& call) const;
	// The charge of inlining `callee` here, or false when the policy keeps
	// the call.
	bool cleanup_welcome(const Shape& shape, bool handler_code, bool loop_site) const;
	bool admit(
		uint32_t callee, const Instruction& call, bool handler_code, bool loop_site, uint32_t& charge, bool& once);
	bool admit_partial(uint32_t g, const Instruction& call, bool handler_code, bool loop_site, PartialCut& partial,
		uint32_t& charge, const FunctionBody* source);
	bool process(SymbolId caller);
	void settle(lowir::FunctionBody& body, const std::vector<BlockId>& layout_next, const std::vector<Value>& repl);
	// A private copy of EH-bearing callee `g` with the call's constant
	// arguments substituted, simplified; its symbol when it lost every EH
	// instruction and fits, else kNoId (the copy is released).
	SymbolId folded_copy(const FunctionBody& caller, const Instruction& call, uint32_t g, bool handler_code);
	void release(SymbolId copy);

	Program& program_;
	const OptimizerOptions& options_;
	OptimizerStats& stats_;
	InlineWave wave_;
	InlineBudgets& budgets_;
	const std::vector<uint32_t>& original_size_;
	const std::vector<uint8_t>& nounwind_;
	const std::function<void(SymbolId)>& simplify_;
	bool source_required_;
	bool caller_cfg_changed_ = false;
	CallGraph graph_;
	std::vector<Shape> shapes_;
	uint32_t caller_ = kNoId;
	// The caller has a loop (its own or an inlined one).
	bool caller_loop_ = false;
	// A callee's loop moves into any caller. (The O0 back end gave a
	// function at most seven cross-block registers and none to one with
	// EH regions, so a second loop nest, or a loop in a function with
	// regions, ran from memory; the O1+ allocator gives every cross-block
	// value a register by weight, native/select/register_allocation.cpp.)
	bool loop_welcome(const Shape&) const { return true; }
	uint64_t budget_ = 0;
	uint64_t outside_loop_budget_ = 0;
	uint64_t once_budget_ = 0;
	uint64_t folded_budget_ = 0;
	std::vector<uint32_t> folded_failures_; // per function index, this wave
	uint32_t folded_names_ = 0;
	// Recursive inlining: the caller's body as the wave found it (the copy
	// at its calls to itself; also the partial copies' source), its size,
	// the copies' remaining budget and the depth they may reach.
	std::unique_ptr<FunctionBody> original_;
	uint32_t original_count_ = 0;
	uint32_t recursive_budget_ = 0;
	uint32_t recursive_depth_ = 0;
};

bool InlineWaveRunner::non_growing(const Shape& shape, const Instruction& call) const
{
	if (shape.unpriced == 0 && shape.cost <= call_site_cost(call))
		return true;
	if (!shape.constant_body)
		return false;
	const FunctionBody& body = *program_.functions[program_.symbols[graph_.functions[caller_]].detail].body;
	uint64_t mask = shape.constant_parameters;
	for (uint32_t p = 0; mask != 0; ++p, mask >>= 1)
		if ((mask & 1) != 0 && (p >= call.extra_count || !body.operands[call.extra_first + p].is_literal()))
			return false;
	return true;
}

bool InlineWaveRunner::compatible(const Instruction& call, SymbolId callee) const
{
	const Function& function = program_.functions[program_.symbols[callee].detail];
	const Signature& sig = program_.signatures[function.signature];
	const bool variadic = sig.boundary.arity == ir_model::CAM_VARIADIC;
	if (call.extra_count < sig.parameter_count || (!variadic && call.extra_count != sig.parameter_count) ||
		call.type != sig.result)
		return false;
	if ((call.flags & IF_SIGNATURE) != 0)
	{
		const Signature& stated = program_.signatures[static_cast<SignatureId>(call.immediate)];
		if (stated.parameter_count != sig.parameter_count || stated.result != sig.result)
			return false;
		for (uint32_t p = 0; p < sig.parameter_count; ++p)
			if (program_.parameters_of(stated)[p].type != program_.parameters_of(sig)[p].type)
				return false;
	}
	// The copy reads each parameter as its argument. A by_address
	// parameter given a value (the call materializes it) or an object
	// parameter given a global (the call snapshots it) stays a call.
	const FunctionBody& caller = *program_.functions[program_.symbols[graph_.functions[caller_]].detail].body;
	for (uint32_t p = 0; p < sig.parameter_count; ++p)
	{
		const Parameter& parameter = program_.parameters_of(sig)[p];
		const Value arg = caller.operands[call.extra_first + p];
		if (parameter.pass != ir_model::PPM_DIRECT &&
			!(arg.is(ValueKind::Symbol) || (arg.is(ValueKind::Temp) && caller.temps[arg.index].type.is_pointer())))
			return false;
		if (parameter.type.is_object() && arg.is(ValueKind::Symbol))
			return false;
		// An object slot whose address another argument carries: the
		// callee could write it before reading its copy.
		if (parameter.type.is_object() && arg.is(ValueKind::Slot))
			for (uint32_t q = 0; q < sig.parameter_count; ++q)
				if (q != p && addressed_slot(caller, caller.operands[call.extra_first + q]) == arg.index)
					return false;
		// A temporary of another type than its parameter (an unsigned
		// bit-field's i32 for a u32): of the same width it is retyped by a
		// copy in the clone (the call would hand the callee the bits in
		// the parameter's convention), otherwise the call stays.
		if (arg.is(ValueKind::Temp))
		{
			const Type have = caller.temps[arg.index].type;
			if (have != parameter.type &&
				(have.is_object() || parameter.type.is_object() || have.is_float() != parameter.type.is_float() ||
					storage_bytes(have) != storage_bytes(parameter.type)))
				return false;
		}
	}
	return true;
}

// A callee whose only catches are terminate guards (a noexcept function
// that calls what may throw) may go inline at -O2 and above outside handler
// code when it cannot unwind: its guard catches all that its copy raises,
// whatever the caller's records hold.
//
// A cleanup-only body, or a guarded operation with a call-free normal
// return, may go inline outside every path reachable from handler code.
// Do not grow an exception dispatcher that has no such normal fast path:
// moving its handlers expands the caller's unwind program on every throw.
// The caller may unwind; copying into handler code could overwrite its
// still-current exception. Existing size and growth budgets still apply.
bool InlineWaveRunner::cleanup_welcome(const Shape& shape, bool handler_code, bool loop_site) const
{
	if (handler_code || options_.level < 2 || (wave_ != InlineWave::Late && !loop_site) ||
		(shape.catches && !shape.fast_return))
		return false;
	const Symbol& symbol = program_.symbols[graph_.functions[caller_]];
	const Signature& sig = program_.signatures[program_.functions[symbol.detail].signature];
	return sig.boundary.unwind != ir_model::CUM_NO;
}

bool InlineWaveRunner::admit(
	uint32_t g, const Instruction& call, bool handler_code, bool loop_site, uint32_t& charge, bool& once)
{
	const SymbolId callee = graph_.functions[g];
	const SymbolMetadata& m = program_.symbols[callee].metadata;
	const Shape& shape = shapes_[g];
	once = false;
	if (source_required_ && (m.flags & SF_FORCE_INLINE) == 0)
		return false;
	// A loop can absorb a loop-free dispatcher or forwarder on its call
	// cycle. Keep recursive workers out of the copy: no self call or second
	// component edge, and the ordinary profitability/budget rules still apply.
	const bool collapse = graph_.recursive[g] && !source_required_ && options_.level >= 2 && loop_site &&
		graph_.component[g] == graph_.component[caller_] && !shape.loop && !shape.eh && shape.self_calls == 0 &&
		shape.component_calls == 1;
	if ((graph_.recursive[g] && !collapse) || g == caller_ || (m.flags & SF_NO_INLINE) != 0 || shape.unsafe ||
		(!source_required_ &&
			((m.flags & SF_REPLACEABLE) != 0 ||
				(shape.eh && !cleanup_welcome(shape, handler_code, loop_site) &&
					!(shape.guarded && !shape.unwinds && !handler_code && options_.level >= 2)) ||
				(handler_code && shape.unwinds))))
		return false;
	if ((m.flags & SF_FORCE_INLINE) != 0)
	{
		charge = 0;
		return true;
	}
	const uint32_t removed = call_site_cost(call);
	if (non_growing(shape, call))
	{
		charge = 0;
		return true;
	}
	if (wave_ != InlineWave::Once)
	{
		const bool hint = (m.flags & SF_INLINE_HINT) != 0;
		const uint32_t allowance = hint ? 65 : 45;
		uint32_t hot, unpriced;
		const FunctionBody& caller_body = *program_.functions[program_.symbols[graph_.functions[caller_]].detail].body;
		call_cost_after_constants(shape, caller_body, call, hot, unpriced);
		charge = hot > removed ? hot - removed : 0;
		if (unpriced == 0 && charge <= allowance + (loop_site ? removed : 0) && charge <= budget_)
			return true;
		if (wave_ == InlineWave::Late)
			return false;
	}
	const uint32_t size = shape.hot;
	const bool single = discardable(program_, callee) && graph_.calls_in[g] == 1 && !graph_.observed[g];
	const uint32_t requested_once_cap = options_.limits.once_cap != 0
		? options_.limits.once_cap
		: kOnceCap * (options_.level >= 2 ? kOnceO2Scale : 1);
	const uint32_t once_cap = m.binding == ir_model::SBM_WEAK ? std::min(requested_once_cap, uint32_t(128))
															  : requested_once_cap;
	const bool sole = options_.level >= 2 && m.binding == ir_model::SBM_INTERNAL && graph_.callers_in[g] == 1 &&
		!graph_.observed[g] && discardable(program_, callee) && size * graph_.calls_in[g] <= once_cap;
	if (!(single || sole) || size > once_cap || size > once_budget_ || size > budgets_.once_unit)
		return false;
	charge = size;
	once = true;
	return true;
}

// A partial copy of `g` at `call` (partial_cut): from O2, for a callee the
// late wave keeps out of line, taking scalar arguments directly and
// returning nothing or a scalar. A caller's call to itself qualifies: the
// copy holds the clean prefix and calls the callee at the cuts, so it never
// recurses itself, and the copies are not revisited in the wave. (Other
// recursive callees get only a copy within kPartialRecursiveCap: larger
// copies at every site of a call cycle, the parser's, take the budgets the
// ordinary callees of those sites need.)
// `source`, when given, is the body cut and copied (the caller's original
// under recursive inlining: the live body's cuts would sit below the
// copies).
bool InlineWaveRunner::admit_partial(uint32_t g, const Instruction& call, bool handler_code, bool loop_site,
	PartialCut& partial, uint32_t& charge, const FunctionBody* source)
{
	const SymbolId callee = graph_.functions[g];
	const SymbolMetadata& m = program_.symbols[callee].metadata;
	const Shape& shape = shapes_[g];
	const bool cycle = graph_.recursive[g] && g != caller_;
	if (options_.level < 2 || (m.flags & (SF_NO_INLINE | SF_REPLACEABLE)) != 0 || shape.unsafe || shape.eh ||
		(handler_code && shape.unwinds) || (call.result != kNoId && !phi_type(call.type)))
		return false;
	const Function& function = program_.functions[program_.symbols[callee].detail];
	const Signature& sig = program_.signatures[function.signature];
	for (uint32_t p = 0; p < sig.parameter_count; ++p)
	{
		// (An addressed argument, unchanged on the copied clean paths, is
		// passed on as it came.)
		const Parameter& parameter = program_.parameters_of(sig)[p];
		if (parameter.pass == ir_model::PPM_INDIRECT_RESULT || parameter.type.is_object())
			return false;
	}
	const bool hint = (m.flags & (SF_INLINE_HINT | SF_FORCE_INLINE)) != 0;
	const uint32_t cap = cycle ? kPartialRecursiveCap
							   : (graph_.calls_in[g] <= 2 ? kPartialFewCap
										 : hint           ? kPartialHintCap
														  : kPartialCap) *
			(loop_site ? kLoopSiteScale : 1);
	if (!partial_cut(program_, source != nullptr ? *source : *function.body, partial, cap) || partial.size > cap ||
		(shape.allocates && partial.size > kPartialAllocatingCap))
		return false;
	charge = partial.size;
	return charge <= budget_;
}

// The slot an address names (through addr, index and pointer copies), or
// kNoId.
uint32_t addressed_slot(const FunctionBody& body, Value v)
{
	for (unsigned step = 0; step < 8 && v.is(ValueKind::Temp); ++step)
	{
		if (v.index < body.parameter_count || body.temps[v.index].def == kNoId)
			return kNoId;
		const Instruction& def = body.instructions[body.temps[v.index].def];
		if (def.op == Opcode::Addr && def.ops[0].is(ValueKind::Slot))
			return def.ops[0].index;
		if (def.op != Opcode::Index && !(def.op == Opcode::Copy && def.type.is_pointer()))
			return kNoId;
		v = def.ops[0];
	}
	return kNoId;
}

// A constant argument: an integer literal, or the address of a readonly
// global (a symbol, or a temporary its `addr` defines).
bool constant_argument(const Program& program, const FunctionBody& body, Value v, Value& out)
{
	if (v.is(ValueKind::Integer))
	{
		out = v;
		return true;
	}
	if (v.is(ValueKind::Temp) && v.index >= body.parameter_count && body.temps[v.index].def != kNoId &&
		body.instructions[body.temps[v.index].def].op == Opcode::Addr)
		v = body.instructions[body.temps[v.index].def].ops[0];
	if (!v.is(ValueKind::Symbol))
		return false;
	const Symbol& symbol = program.symbols[v.index];
	if (symbol.kind != SymbolKind::GlobalDefinition || symbol.metadata.storage != ir_model::GSM_READONLY ||
		(symbol.metadata.flags & SF_REPLACEABLE) != 0)
		return false;
	out = v;
	return true;
}

void InlineWaveRunner::release(SymbolId copy)
{
	Symbol& symbol = program_.symbols[copy];
	program_.functions[symbol.detail].body.reset();
	symbol.kind = SymbolKind::FunctionDeclaration;
}

SymbolId InlineWaveRunner::folded_copy(
	const FunctionBody& caller, const Instruction& call, uint32_t g, bool handler_code)
{
	const SymbolId callee = graph_.functions[g];
	// By value: making the copy's symbol may move the symbol table.
	const SymbolMetadata metadata = program_.symbols[callee].metadata;
	const uint32_t detail = program_.symbols[callee].detail;
	const Shape& shape = shapes_[g];
	const bool hint = (metadata.flags & (SF_INLINE_HINT | SF_FORCE_INLINE)) != 0;
	if (!shape.eh || shape.size > kFoldedCalleeCap || graph_.recursive[g] || shape.unsafe ||
		(metadata.flags & (SF_NO_INLINE | SF_REPLACEABLE)) != 0 || (shape.loop && (!hint || !loop_welcome(shape))) ||
		folded_failures_[g] >= kFoldedFailures || folded_budget_ == 0)
		return kNoId;
	std::vector<std::pair<uint32_t, Value>> constants;
	// Only named parameters have parameter temporaries. An unnamed argument's
	// position could otherwise replace an unrelated instruction's result.
	const uint32_t parameters = program_.signatures[program_.functions[detail].signature].parameter_count;
	for (uint32_t k = 0; k < parameters; ++k)
	{
		Value c;
		if (constant_argument(program_, caller, caller.operands[call.extra_first + k], c))
			constants.push_back(std::make_pair(k, c));
	}
	if (constants.empty())
		return kNoId;
	// The copy: an internal definition outside the program's order.
	std::string name;
	NameId id = 0;
	do
	{
		name = "__folded_copy" + std::to_string(folded_names_++);
		id = program_.names.find(name.data(), static_cast<uint32_t>(name.size()));
	} while (id != 0 && program_.find_symbol(id) != kNoId);
	const SymbolId copy = program_.symbol_for(program_.names.intern(name.data(), static_cast<uint32_t>(name.size())));
	const Function& source = program_.functions[detail];
	Function function;
	function.signature = source.signature;
	function.location = source.location;
	function.body.reset(new FunctionBody(*source.body));
	program_.symbols[copy].kind = SymbolKind::FunctionDefinition;
	program_.symbols[copy].detail = static_cast<uint32_t>(program_.functions.size());
	program_.symbols[copy].metadata = metadata;
	program_.symbols[copy].metadata.binding = ir_model::SBM_INTERNAL;
	program_.symbols[copy].metadata.object = 0;
	program_.symbols[copy].metadata.flags &= static_cast<uint8_t>(~SF_OBJECT_ROOT);
	program_.functions.push_back(std::move(function));
	FunctionBody& body = *program_.functions[program_.symbols[copy].detail].body;
	for (const std::pair<uint32_t, Value>& c : constants)
	{
		const Value value = c.second.is(ValueKind::Integer) ? integer_value(body, caller.literals[c.second.index])
															: c.second;
		for (BlockId b : body.layout)
			for (InstId i = body.blocks[b].first; i != kNoId; i = body.instructions[i].next)
			{
				Instruction& inst = body.instructions[i];
				uint32_t position = 0;
				for_each_operand(body, inst, [&](Value& v) {
					const bool callee_slot = inst.op == Opcode::Call && position++ == 0;
					if (v.is(ValueKind::Temp) && v.index == c.first && !(callee_slot && value.is_literal()))
						v = value;
				});
			}
	}
	simplify_(copy);
	const Shape folded = shape_of(program_, copy, nounwind_);
	const uint32_t cap = hint ? kFoldedHintCap : kFoldedCap;
	if (folded.eh || folded.unsafe || folded.size > cap || folded.size > folded_budget_ ||
		(handler_code && folded.unwinds))
	{
		release(copy);
		++folded_failures_[g];
		return kNoId;
	}
	folded_budget_ -= folded.size;
	return copy;
}

bool InlineWaveRunner::process(SymbolId caller)
{
	caller_ = graph_.index_of[caller];
	caller_cfg_changed_ = false;
	caller_loop_ = shapes_[caller_].loop;
	const uint32_t default_budget = options_.limits.caller_budget != 0 ? options_.limits.caller_budget : kCallerBudget;
	budget_ = default_budget;
	outside_loop_budget_ = kOutsideLoopBudget;
	once_budget_ = options_.limits.once_caller_budget != 0
		? options_.limits.once_caller_budget
		: kOnceCallerBudget * (options_.level >= 2 ? kOnceO2Scale : 1);
	folded_budget_ = kFoldedCallerBudget;
	FunctionBody& body = *program_.functions[program_.symbols[caller].detail].body;
	static Cfg scratch; // (reused for its capacity: every build overwrites what
						// it fills)
	Cfg& cfg = scratch;
	// Dominators only for handler code and the loops (find_loops).
	if (!source_required_)
		build_cfg_light(program_, body, cfg);
	if (!source_required_ && caller_loop_)
		complete_cfg(cfg);
	// Any ordinary path from a landing may still carry its exception.
	// A merge with a normal path is not enough to prove otherwise.
	const size_t policy_blocks = source_required_ ? 0 : body.blocks.size();
	std::vector<uint8_t> handler(policy_blocks, 0);
	std::vector<BlockId> work;
	if (!source_required_)
	{
		for (BlockId b : cfg.handlers)
			if (!handler[b])
			{
				handler[b] = 1;
				work.push_back(b);
			}
		for (size_t k = 0; k < work.size(); ++k)
		{
			const BlockId b = work[k];
			for (uint32_t e = cfg.succ_first[b]; e < cfg.succ_first[b + 1]; ++e)
			{
				const BlockId next = cfg.succs[e];
				if (!handler[next])
				{
					handler[next] = 1;
					work.push_back(next);
				}
			}
		}
	}
	work.clear();
	// Cold blocks: every path from them ends in a noreturn call or an
	// unreachable (O2+: only trivial and forced callees go inline there;
	// new blocks inherit their split block's answer).
	std::vector<uint8_t> cold(policy_blocks, 0);
	if (options_.level >= 2)
		for (uint32_t r = static_cast<uint32_t>(cfg.rpo.size()); r-- > 0;)
		{
			const BlockId b = cfg.rpo[r];
			bool ends = false;
			for (InstId i = body.blocks[b].first; i != kNoId && !ends; i = body.instructions[i].next)
				ends = (body.instructions[i].op == Opcode::Call && program_.may_end_block(body.instructions[i])) ||
					body.instructions[i].op == Opcode::Unreachable;
			bool all = cfg.succ_count(b) > 0;
			for (uint32_t k = cfg.succ_first[b]; k < cfg.succ_first[b + 1]; ++k)
				all = all && cold[cfg.succs[k]] && cfg.rpo_index[cfg.succs[k]] > r;
			cold[b] = (ends || all) && b != body.layout[0];
		}
	// Predicted rare edges guide profitability, just as terminal paths do.
	// A unique incoming edge identifies the prediction entering a region;
	// a join inherits it only when all forward predecessors are cold.
	if (!source_required_ && options_.level >= 2)
		for (BlockId b : cfg.rpo)
		{
			if (b == body.layout[0])
				continue;
			bool inherited = cfg.pred_count(b) != 0;
			for (uint32_t k = cfg.pred_first[b]; k < cfg.pred_first[b + 1] && inherited; ++k)
				inherited = cold[cfg.preds[k]] && cfg.rpo_index[cfg.preds[k]] < cfg.rpo_index[b];
			if (inherited)
				cold[b] = 1;
			if (cfg.pred_count(b) != 1)
				continue;
			const BlockId from = cfg.preds[cfg.pred_first[b]];
			const InstId last = body.blocks[from].last;
			if (last == kNoId)
				continue;
			const Instruction& branch = body.instructions[last];
			if (branch.op != Opcode::Branch || branch.ops[1].index == branch.ops[2].index)
				continue;
			const BranchPrediction prediction = static_cast<BranchPrediction>(branch.sub);
			if ((prediction == BranchPrediction::Unlikely && branch.ops[1].index == b) ||
				(prediction == BranchPrediction::Likely && branch.ops[2].index == b))
				cold[b] = 1;
		}
	// Loop blocks (new blocks inherit their split block's answer).
	std::vector<uint8_t> in_loop(policy_blocks, 0);
	if (caller_loop_)
	{
		LoopForest forest;
		find_loops(body, cfg, forest);
		for (BlockId b : cfg.rpo)
			in_loop[b] = forest.innermost[b] != kNoId;
	}
	work.assign(body.layout.rbegin(), body.layout.rend());
	std::vector<BlockId> layout_next(source_required_ ? 0 : body.blocks.size(), kNoId);
	if (!source_required_)
		for (size_t k = 0; k + 1 < body.layout.size(); ++k)
			layout_next[body.layout[k]] = body.layout[k + 1];
	std::vector<Value> repl;
	bool changed = false;
	// Recursive inlining (kRecursiveCap): the body before any copy, and the
	// depth the total budget allows for the copies' self calls (each level
	// multiplies them).
	original_.reset();
	original_count_ = recursive_budget_ = recursive_depth_ = 0;
	if (wave_ == InlineWave::Late && options_.level >= 2 && graph_.recursive[caller_] && !shapes_[caller_].eh &&
		!shapes_[caller_].unsafe && shapes_[caller_].size <= kRecursiveCap)
	{
		const uint64_t self_calls = shapes_[caller_].self_calls;
		// (A loop's one self call, what is left of a tail-recursive function,
		// is a chain: its copies nest loops.)
		const bool chain = shapes_[caller_].loop && self_calls == 1;
		const uint32_t depth_cap = chain ? kRecursiveChainDepth : kRecursiveDepth;
		uint64_t copies = 0, level = 1;
		for (uint32_t d = 1; d <= depth_cap && (chain || (self_calls >= kRecursiveTreeCalls && !shapes_[caller_].loop));
			++d)
		{
			level *= self_calls;
			copies += level;
			if (copies * shapes_[caller_].size > kRecursiveTotal)
				break;
			recursive_depth_ = d;
		}
		if (recursive_depth_ != 0)
		{
			original_.reset(new FunctionBody(body));
			original_count_ = shapes_[caller_].size;
			recursive_budget_ = kRecursiveTotal;
		}
	}
	// A block's recursive copy level (new blocks are one deeper than their
	// split block; the continuation stays).
	std::vector<uint8_t> depth(policy_blocks, 0);
	while (!work.empty())
	{
		const BlockId b = work.back();
		work.pop_back();
		InstId prev = kNoId;
		for (InstId i = body.blocks[b].first, next = kNoId; i != kNoId; i = next)
		{
			next = body.instructions[i].next;
			InstId before_call = prev;
			prev = i;
			const Instruction& inst = body.instructions[i];
			if (inst.op != Opcode::Call || !inst.ops[0].is(ValueKind::Symbol))
				continue;
			// Its arguments as an immediate replacement would leave them.
			if (!repl.empty())
				for_each_operand(body, body.instructions[i], [&](Value& v) {
					while (v.is(ValueKind::Temp) && v.index < repl.size() && !repl[v.index].is(ValueKind::None))
						v = repl[v.index];
				});
			const SymbolId callee = inst.ops[0].index;
			if ((source_required_ || !options_.optional_inlining) &&
				(program_.symbols[callee].metadata.flags & SF_FORCE_INLINE) == 0)
				continue;
			if (source_required_)
				materialize_call_arguments(program_, body, b, before_call, i);
			const uint32_t g = graph_.function_of(callee);
			uint32_t charge = 0;
			bool once = false;
			if (g == kNoId || !compatible(body.instructions[i], callee))
				continue;
			SymbolId copy = kNoId;
			PartialCut partial;
			bool partly = false, fully = false;
			const bool cold_site = !source_required_ && cold[b] != 0 && shapes_[g].size > kTrivialLeaf &&
				!non_growing(shapes_[g], body.instructions[i]) &&
				(program_.symbols[callee].metadata.flags & SF_FORCE_INLINE) == 0;
			if (cold_site ||
				!admit(g, body.instructions[i], !source_required_ && handler[b] != 0,
					!source_required_ && in_loop[b] != 0, charge, once))
			{
				if (source_required_)
					continue;
				// Recursive inlining: a call of the caller to itself takes a
				// copy of the original to the admitted depth; below it, the
				// late wave copies the callee's call-free part (from the
				// original too). At such a call without an original the
				// live body is the callee: the pending edits go in first
				// (its layout and operands are then current for the cut
				// and the copy).
				if (wave_ == InlineWave::Late && g == caller_ && original_ && depth[b] < recursive_depth_ &&
					original_count_ <= recursive_budget_ && original_count_ <= budget_ &&
					!(handler[b] != 0 && shapes_[g].unwinds))
				{
					fully = true;
					charge = original_count_;
				}
				else
				{
					if (wave_ == InlineWave::Late && g == caller_ && !original_ && changed)
						settle(body, layout_next, repl);
					partly = wave_ == InlineWave::Late &&
						admit_partial(g, body.instructions[i], handler[b] != 0, in_loop[b] != 0, partial, charge,
							g == caller_ ? original_.get() : nullptr);
				}
				// An EH-bearing callee may still fold away with the call's
				// constants (the ordinary wave only).
				if (!partly && !fully && wave_ != InlineWave::Ordinary)
					continue;
				// Audit mode validates the whole program while simplifying
				// the copy: the caller's pending edits go in first.
				if (!partly && !fully)
				{
					if (options_.verify_passes && changed)
						settle(body, layout_next, repl);
					copy = folded_copy(body, body.instructions[i], g, handler[b] != 0);
					if (copy == kNoId)
						continue;
					charge = 0;
				}
			}
			const SymbolId source = copy != kNoId ? copy : callee;
			// A copy of the caller into itself reads the original, or a
			// snapshot: clone_call appends to the caller while it reads the
			// callee.
			std::unique_ptr<FunctionBody> snapshot;
			if (partly && g == caller_ && !original_)
				snapshot.reset(new FunctionBody(body));
			const FunctionBody& kb = g == caller_ && original_ ? *original_
				: snapshot                                     ? *snapshot
						   : *program_.functions[program_.symbols[source].detail].body;
			const size_t before = body.blocks.size();
			const uint64_t size = copy != kNoId ? instruction_count(kb) : partly ? partial.size : shapes_[g].size;
			// The unit's growth budgets (the single-call policy has its own).
			const bool forced = copy == kNoId && (program_.symbols[callee].metadata.flags & SF_FORCE_INLINE) != 0;
			uint64_t& unit = forced ? budgets_.force_unit : budgets_.growth_unit;
			// (A trivial leaf takes no more room than its call.)
			const bool free_leaf = !partly && copy == kNoId && non_growing(shapes_[g], body.instructions[i]);
			// Preserve loop/handler sites, small and call-free helpers,
			// mandatory/constant copies and definition-removing inlining.
			// Inlining a small original wrapper with a constant result can
			// remove its caller's guard after the wrapper absorbs helpers.
			// The actual expanded copy still pays the ordinary unit budget.
			const bool folds_result = body.instructions[i].result != kNoId && shapes_[g].constant_result &&
				callee < original_size_.size() && original_size_[callee] != 0 && original_size_[callee] <= kWrapperCap;
			const bool outside_loop = options_.level >= 2 && options_.limits.caller_budget == 0 && !source_required_ &&
				!once && !forced && !free_leaf && !folds_result && copy == kNoId && !fully &&
				shapes_[g].size > kWrapperCap && shapes_[g].hot_call && shapes_[caller_].size >= kSettledSize &&
				in_loop[b] == 0 && handler[b] == 0;
			if (outside_loop && size > outside_loop_budget_)
				continue;
			if (!once && !free_leaf && size > unit)
			{
				if (copy != kNoId)
					release(copy);
				continue;
			}
			if (!once && !free_leaf)
				unit -= size;
			if (outside_loop)
				outside_loop_budget_ -= size;
			const bool loop = copy != kNoId ? shape_of(program_, copy, nounwind_).loop : shapes_[g].loop;
			InstId tail = kNoId;
			if (!source_required_ || !inline_return(body, b, before_call, i, kb, tail))
				tail = clone_call(program_, caller, source, body, b, before_call, i, kb, layout_next, repl,
					partly ? &partial : nullptr, source_required_ || !fully);
			if (copy != kNoId)
				release(copy);
			if (!source_required_)
			{
				handler.resize(body.blocks.size(), handler[b]);
				in_loop.resize(body.blocks.size(), in_loop[b]);
				cold.resize(body.blocks.size(), cold[b]);
				depth.resize(body.blocks.size(), static_cast<uint8_t>(depth[b] + 1));
				if (body.blocks.size() != before)
					depth[before] = depth[b];
			}
			if (once)
			{
				once_budget_ -= charge;
				budgets_.once_unit -= charge;
			}
			else
				budget_ = budget_ >= charge ? budget_ - charge : 0;
			if (fully)
				recursive_budget_ -= original_count_;
			else if (!partly)
				--graph_.calls_in[g];
			caller_loop_ = caller_loop_ || loop;
			++stats_.calls_inlined;
			stats_.inlined_instructions += size;
			changed = true;
			if (body.blocks.size() == before)
			{
				prev = tail;
				continue;
			}
			caller_cfg_changed_ = true;
			// Continue in the continuation (created first by clone_call); a
			// recursive copy's own blocks first, for the next level.
			work.push_back(static_cast<BlockId>(before));
			if (fully)
				for (size_t k = body.blocks.size(); k-- > before + 1;)
					work.push_back(static_cast<BlockId>(k));
			break;
		}
	}
	if (!changed)
		return false;
	if (!source_required_ || caller_cfg_changed_)
		settle(body, layout_next, repl);
	return true;
}

// The caller's layout from its list, and the results that became a
// returned value or a merge slot replaced. Once per caller (and, in audit
// mode, before a folded copy validates the whole program).
void InlineWaveRunner::settle(
	FunctionBody& body, const std::vector<BlockId>& layout_next, const std::vector<Value>& repl)
{
	std::vector<BlockId> layout;
	layout.reserve(body.blocks.size());
	for (BlockId b = body.layout.empty() ? kNoId : body.layout[0]; b != kNoId; b = layout_next[b])
		layout.push_back(b);
	body.layout.swap(layout);
	if (repl.empty())
		return;
	auto resolve = [&](Value v) {
		while (v.is(ValueKind::Temp) && v.index < repl.size() && !repl[v.index].is(ValueKind::None))
			v = repl[v.index];
		return v;
	};
	for (BlockId b : body.layout)
		for (InstId i = body.blocks[b].first; i != kNoId; i = body.instructions[i].next)
			for_each_operand(body, body.instructions[i], [&](Value& v) { v = resolve(v); });
}

void InlineWaveRunner::run()
{
	// Required substitutions see the live call graph. A constant-false
	// template branch can contain a self call that is never executed;
	// retaining that edge would incorrectly classify its body as recursive.
	if (source_required_)
		for (const SymbolId symbol : program_.order)
			if (program_.symbols[symbol].kind == SymbolKind::FunctionDefinition &&
				(program_.symbols[symbol].metadata.flags & SF_FORCE_INLINE) != 0)
				simplify_(symbol);
	build_call_graph(program_, graph_, source_required_);
	shapes_.resize(graph_.functions.size());
	if (!source_required_)
	{
		folded_failures_.assign(graph_.functions.size(), 0);
	}
	for (uint32_t f = 0; f < graph_.functions.size(); ++f)
		if (!source_required_ || graph_.calls_in[f] != 0)
		{
			shapes_[f] = source_required_
				? shape_of<true>(program_, graph_.functions[f], nounwind_)
				: shape_of(program_, graph_.functions[f], nounwind_, graph_.recursive[f] ? &graph_ : nullptr);
		}
	for (uint32_t f : graph_.bottom_up)
	{
		const SymbolId caller = graph_.functions[f];
		if (source_required_ && graph_.callee_first[f] == graph_.callee_first[f + 1])
			continue;
		// A this-adjusting thunk is kept as lowered, so its body keeps the
		// forwarding shape the object back end encodes as `this` adjusted
		// and a jump (every one in a host object, as GCC emits them,
		// smaller than a copy of the target and as fast; a variadic
		// function's always: a call cannot forward the variable arguments).
		// For course objects, which compile the others as functions, the
		// target is inlined instead (one call per dispatch, not two). The
		// object paths recognize thunks from the optimized bodies alike.
		if (this_adjusting_thunk(program_, caller) &&
			(!options_.inline_thunk_targets ||
				program_.signatures[program_.functions[program_.symbols[caller].detail].signature].boundary.arity ==
					ir_model::CAM_VARIADIC))
			continue;
		const bool changed = process(caller);
		if (changed || (!source_required_ && wave_ == InlineWave::Ordinary))
		{
			// Straight-line copies preserve the prepared body's CFG and slot
			// facts. Only a block-splicing copy needs cleanup again.
			if (!source_required_ || caller_cfg_changed_)
				simplify_(caller);
			if (!source_required_ || (program_.symbols[caller].metadata.flags & SF_FORCE_INLINE) != 0)
				shapes_[f] = source_required_
					? shape_of<true>(program_, caller, nounwind_)
					: shape_of(program_, caller, nounwind_, graph_.recursive[f] ? &graph_ : nullptr);
			if (!source_required_)
				infer_optimized_readonly(program_, caller);
		}
	}
}

} // namespace

void run_inline_wave(Program& program, const OptimizerOptions& options, OptimizerStats& stats, InlineWave wave,
	InlineBudgets& budgets, const std::vector<uint32_t>& original_size, const std::vector<uint8_t>& nounwind,
	const std::function<void(SymbolId)>& simplify)
{
	InlineWaveRunner runner(program, options, stats, wave, budgets, original_size, nounwind, simplify);
	runner.run();
}

void run_source_inline_wave(Program& program, const std::function<void(SymbolId)>& simplify)
{
	OptimizerOptions options;
	options.level = 0;
	OptimizerStats stats;
	InlineBudgets budgets;
	budgets.force_unit = UINT64_MAX;
	std::vector<uint32_t> original;
	std::vector<uint8_t> nounwind(program.symbols.size(), 0);

	InlineWaveRunner runner(program, options, stats, InlineWave::Ordinary, budgets, original, nounwind, simplify, true);
	runner.run();
}

} // namespace opt
