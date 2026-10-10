#include "opt/inliner.h"

#include "lowir/builder.h"
#include "opt/loops.h"

namespace opt {

using namespace lowir;

namespace {

// Ordinary wave: callee size caps (a call-free single-block body; other
// call-free bodies; a body whose hot code calls; a hinted one), the
// per-caller growth budget and the trivial leaf every budget admits.
const uint32_t kOrdinaryLeafCap = 40;
const uint32_t kOrdinaryCap = 24;
const uint32_t kOrdinaryCallCap = 6;
const uint32_t kHintCap = 48;
const uint32_t kCallerBudget = 1536;
// A call inside one of the caller's loops runs once per iteration: its
// callee's caps double.
const uint32_t kLoopSiteScale = 2;
const uint32_t kTrivialLeaf = 4;
// Single-call (definition-removing) inlining; from O2 a callee twice that
// size and its caller twice the budget (a lambda's instantiation of a
// large dispatcher called once: pa32's O1 fixtures pin the O1 caps).
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
// Fast-path admission (multi-block, loop-free callees whose calls lie off
// a call-free path from the entry to a return): that path's size cap and
// the whole body's (hinted larger), both doubled at a loop site.
const uint32_t kFastPathCap = 24;
const uint32_t kFastPathHintCap = 48;
const uint32_t kFastPathTotalCap = 96;
const uint32_t kFastPathTotalHintCap = 160;
// A fast-path wrapper (a body this small with a call-free path from the
// entry to a return) keeps callees larger than kSlowSiteCap out of its
// slow paths, so it stays small enough to inline (vector::push_back and
// _M_realloc_append).
const uint32_t kWrapperCap = 32;
const uint32_t kSlowSiteCap = 16;
// From O2, a discardable body up to this size whose path to a return
// calls only callees every wave copies whole (forced, trivial leaves:
// emplace_back's construct and forward) keeps callees larger than
// kWrapperCap out of its slow paths.
const uint32_t kLeafWrapperCap = 48;
// A callee whose loop calls through a pointer the site passes as a known
// function is copied up to this size (the call turns direct).
const uint32_t kDispatchCap = 96;
// A loop around one call inlined whole (admit).
const uint32_t kTinyLoopCap = 12;
// Late wave: a call-free single-block body, any other body, a hinted one.
const uint32_t kLateLeafCap = 40;
const uint32_t kLateCap = 6;
const uint32_t kLateHintCap = 48;
// From O2, a body without calls outside cold blocks: its cap in both
// waves, also when it holds a loop and has several callers (O1 keeps such
// a loop out of line).
const uint32_t kCallFreeCap = 48;
// Partial inlining (O2+, late wave): the copy's caps (a hinted callee's
// larger), doubled at a loop site.
const uint32_t kPartialCap = 24;
const uint32_t kPartialHintCap = 40;
// A partial copy of a callee with at most two callers.
const uint32_t kPartialFewCap = 48;
// A callee whose hot code allocates (a growth path) gets only a partial
// copy this small: a test or two in front of the work.
const uint32_t kPartialAllocatingCap = 8;
// Recursive inlining (O2+, late wave): a caller's call to itself takes a
// copy of the caller's body as the wave found it, to kRecursiveDepth
// levels (fewer when the copies would pass kRecursiveTotal instructions),
// while that body has at most kRecursiveCap instructions, no loop, no EH
// region and at least kRecursiveTreeCalls calls to itself (a tree walk
// recurses from about every other activation; a lone self call usually
// sits behind an early return, and a copy would only lengthen the common
// path's prologue); the deepest copies' calls get the partial copies.
const uint32_t kRecursiveCap = 64;
const uint32_t kRecursiveTotal = 320;
const uint32_t kRecursiveDepth = 2;
const uint32_t kRecursiveTreeCalls = 2;
// From O2, a caller this unit never copies into another (no call site
// here, or larger than any cap admits it at a loop site, kFastPathTotal-
// HintCap doubled, and not a single-call candidate) takes hinted and
// call-free callees with at most kSettledCalls call sites up to half again
// their caps: its growth costs no other copy, and few copies are made.
// (Callees full of calls keep their caps: pa32 pins them.)
const uint32_t kSettledSize = 2 * kFastPathTotalHintCap;
const uint32_t kSettledScaleNum = 3;
const uint32_t kSettledScaleDen = 2;
const uint32_t kSettledCalls = 3;
const uint32_t kSettledFewCalls = 2;
const uint32_t kSettledFewCap = 128;
const uint32_t kSettledCallShare = 8;
const uint32_t kSettledSmallCaller = 128;
// A callee grown past its caps by its own copies is judged by its input
// shape at this many loop sites per caller (admit).
const uint32_t kGrownCopies = 2;

struct Shape
{
	uint32_t size = 0;
	uint32_t hot = 0;      // instructions outside cold blocks (every path from them ends in noreturn)
	bool hot_call = false; // a call outside cold blocks
	uint32_t blocks = 0;
	uint32_t returns = 0;
	bool eh = false;
	bool call = false;
	uint32_t calls = 0; // call instructions
	bool loop = false;
	bool loop_dispatch = false; // an indirect call outside cold blocks inside one of its natural loops
	bool unsafe = false;
	bool unwinds = false;
	uint32_t fast = 0;        // instructions of the blocks a call-free path from the entry reaches
	bool fast_return = false; // one of them returns
	bool allocates = false;   // hot code calls an allocation function (a growth path)
};

uint32_t addressed_slot(const FunctionBody& body, Value v);

// A callee with a short call-free path to a return around a slower calling
// path. (Not a growth path: vector::_M_realloc_append returns without a
// call on a path no call takes, a zero length with no old buffer.)
bool fast_path_of(const Shape& shape, bool hint, uint32_t scale)
{
	return shape.blocks > 1 && shape.hot_call && !shape.loop && shape.fast_return && !shape.allocates &&
		shape.fast <= (hint ? kFastPathHintCap : kFastPathCap) * scale &&
		shape.hot <= (hint ? kFastPathTotalHintCap : kFastPathTotalCap) * scale;
}

bool phi_type(Type type)
{
	return type.is_pointer() || type.kind() == TypeKind::F32 || type.kind() == TypeKind::F64 ||
		(type.is_integer() && type.kind() != TypeKind::I128);
}

Shape shape_of(const Program& program, SymbolId symbol, const std::vector<uint8_t>& nounwind)
{
	Shape shape;
	const Function& function = program.functions[program.symbols[symbol].detail];
	const FunctionBody& body = *function.body;
	const Signature& sig = program.signatures[function.signature];
	shape.unsafe = sig.boundary.arity == ir_model::CAM_VARIADIC || body.layout.empty();
	bool unwinding = false;
	for (BlockId b : body.layout)
	{
		++shape.blocks;
		for (InstId i = body.blocks[b].first; i != kNoId; i = body.instructions[i].next)
		{
			const Instruction& inst = body.instructions[i];
			++shape.size;
			shape.eh = shape.eh || is_eh_instruction(inst.op);
			shape.call = shape.call || inst.op == Opcode::Call;
			shape.calls += inst.op == Opcode::Call ? 1 : 0;
			shape.returns += inst.op == Opcode::Return ? 1 : 0;
			unwinding = unwinding ||
				(may_unwind(program, inst) &&
					!(inst.op == Opcode::Call && inst.ops[0].is(ValueKind::Symbol) && nounwind[inst.ops[0].index]));
			if (inst.op == Opcode::VaStart || inst.op == Opcode::VaArg || inst.op == Opcode::StackAlloc ||
				(b == body.layout[0] && inst.op == Opcode::Phi))
				shape.unsafe = true;
		}
	}
	shape.unwinds = unwinding && sig.boundary.unwind != ir_model::CUM_NO && !nounwind[symbol];
	shape.hot = shape.size;
	shape.hot_call = shape.call;
	if (shape.blocks > 1)
	{
		static Cfg scratch; // (reused for its capacity: every build overwrites what it fills)
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
					--shape.hot;
		}
		shape.hot_call = false;
		for (BlockId b : cfg.rpo)
			for (InstId i = body.blocks[b].first; i != kNoId && !cold[b]; i = body.instructions[i].next)
			{
				const Instruction& inst = body.instructions[i];
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
			{
				++shape.fast;
				shape.fast_return = shape.fast_return || body.instructions[i].op == Opcode::Return;
			}
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
		// Loops find_loops does not report (holding handler blocks, deep
		// nests) count as dispatch-free.
		if (shape.loop && shape.hot_call)
		{
			LoopForest forest;
			complete_cfg(cfg);
			find_loops(body, cfg, forest);
			for (BlockId b : cfg.rpo)
				for (InstId i = body.blocks[b].first; i != kNoId && !cold[b] && forest.innermost[b] != kNoId;
					i = body.instructions[i].next)
					shape.loop_dispatch = shape.loop_dispatch ||
						(body.instructions[i].op == Opcode::Call && !body.instructions[i].ops[0].is(ValueKind::Symbol));
		}
	}
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
// When that copy is over `cap`, the cut is the shallowest clean dominator
// (the copy holds the first test only).
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
	static Cfg scratch; // (reused for its capacity: every build overwrites what it fills)
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
	for (int keep_fast = 1; keep_fast >= 0; --keep_fast)
	{
		out.cut.assign(n, 0);
		for (BlockId c : cfg.rpo)
		{
			if (!calls[c])
				continue;
			BlockId at = kNoId;
			for (BlockId d = c; d != kNoId && d != entry; d = cfg.idom[d])
				if (clean[d] && (!keep_fast || !fast[d]) && (cfg.flags[d] & BF_HANDLER) == 0)
					at = d;
			if (at != kNoId)
				out.cut[at] = 1;
		}
		partial_region(kb, cfg, out, returns);
		if (out.size <= cap)
			break;
	}
	return out.cuts != 0 && returns;
}

// Copies callee `kb` into `cb` in place of the call `call_id` (in `block`,
// after `prev`): the block is split at the call, the callee's blocks go
// between the two halves (in `layout_next`, the caller's layout as a
// list by block), returns jump to the continuation and merge the result in
// a phi there (or, for a value phis cannot carry, its one return's value
// replaces the result: recorded in `repl`, applied once per caller).
void clone_call(Program& program, FunctionBody& cb, BlockId block, InstId prev, InstId call_id, const FunctionBody& kb,
	std::vector<BlockId>& layout_next, std::vector<Value>& repl, const PartialCut* partial = nullptr)
{
	const Instruction call = cb.instructions[call_id];
	const BlockId cont = append_block(cb, BlockKind::InlineCont);
	cb.blocks[cont].first = call.next;
	cb.blocks[cont].last = call.next == kNoId ? kNoId : cb.blocks[block].last;
	cb.blocks[block].last = prev;
	if (prev == kNoId)
		cb.blocks[block].first = kNoId;
	else
		cb.instructions[prev].next = kNoId;
	cb.instructions[call_id].next = kNoId;
	if (cb.blocks[cont].last != kNoId)
		for_each_successor(
			cb, cb.instructions[cb.blocks[cont].last], [&](BlockId s) { rename_phi_incoming(cb, s, block, cont); });
	else
	{
		const Instruction unreachable = make_instruction(Opcode::Unreachable, Type::scalar(TypeKind::Void));
		const InstId id = append_instruction(cb, unreachable);
		insert_after(cb, cont, kNoId, id);
	}
	// Temporaries, slots and blocks of the copy; an argument of another
	// type than its parameter (compatible: the same width) is retyped at
	// the end of the call's block.
	std::vector<Value> temps(kb.temps.size(), Value::none());
	for (TempId t = 0; t < kb.parameter_count; ++t)
	{
		temps[t] = cb.operands[call.extra_first + t];
		if (!temps[t].is(ValueKind::Temp) || cb.temps[temps[t].index].type == kb.temps[t].type)
			continue;
		Instruction retype = make_instruction(Opcode::Copy, kb.temps[t].type);
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
	if (partial == nullptr)
		for (TempId t = kb.parameter_count; t < kb.temps.size(); ++t)
			copy_temp(t);
	else
		for (BlockId b : kb.layout)
			if (partial->copy[b])
				for (InstId i = kb.blocks[b].first; i != kNoId; i = kb.instructions[i].next)
					if (kb.instructions[i].result != kNoId)
						copy_temp(kb.instructions[i].result);
	std::vector<uint32_t> slots(kb.slots.size(), kNoId);
	std::vector<uint8_t> named;
	if (partial != nullptr)
	{
		named.assign(kb.slots.size(), 0);
		for (BlockId b : kb.layout)
			if (partial->copy[b])
				for (InstId i = kb.blocks[b].first; i != kNoId; i = kb.instructions[i].next)
					for_each_operand(kb, kb.instructions[i], [&](const Value& v) {
						if (v.is(ValueKind::Slot))
							named[v.index] = 1;
					});
	}
	for (SlotId s = 0; s < kb.slots.size(); ++s)
	{
		if (partial != nullptr && !named[s])
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
	std::vector<BlockId> blocks(kb.blocks.size(), kNoId);
	for (BlockId b : kb.layout)
		if (partial == nullptr || partial->copy[b] || partial->cut[b])
		{
			// A copy keeps a generated block's kind; the entry's prints as
			// the inlined entry, a named block's as a copy of that name.
			const Block copied = kb.blocks[b]; // (kb may be cb: appending moves it)
			blocks[b] = append_block(cb,
				copied.kind != BlockKind::Plain ? copied.kind
					: b == kb.layout[0]         ? BlockKind::InlineEntry
												: BlockKind::InlineCopy);
			if (copied.kind == BlockKind::Plain && b != kb.layout[0])
				cb.blocks[blocks[b]].name = copied.name;
		}
	std::vector<uint32_t> locations(kb.locations.size() + 1, 0);
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
	std::vector<std::pair<BlockId, Value>> returns;
	std::vector<Value> extra;
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
					cb.locations.push_back(kb.locations[inst.location - 1]);
					locations[inst.location] = static_cast<uint32_t>(cb.locations.size());
				}
				inst.location = locations[inst.location];
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
}

class InlineWaveRunner
{
public:
	InlineWaveRunner(Program& program, const OptimizerOptions& options, OptimizerStats& stats, InlineWave wave,
		InlineBudgets& budgets, const std::vector<uint32_t>& original_size, const std::vector<uint8_t>& nounwind,
		const std::function<void(SymbolId)>& simplify)
		: program_(program), options_(options), stats_(stats), wave_(wave), budgets_(budgets),
		  original_size_(original_size), nounwind_(nounwind), simplify_(simplify)
	{}

	void run();

private:
	bool compatible(const Instruction& call, SymbolId callee) const;
	// The charge of inlining `callee` here, or false when the policy keeps
	// the call.
	bool admit(uint32_t callee, bool handler_code, bool loop_site, uint32_t& charge, bool& once, bool dispatch = false);
	bool leaf_return_path(const FunctionBody& body, const Cfg& cfg) const;
	bool known_target(const Instruction& call) const;
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
	CallGraph graph_;
	std::vector<Shape> shapes_;
	std::vector<Shape> input_shapes_;     // the ordinary wave's input (every function before its own copies)
	std::vector<uint32_t> grown_copies_;  // per function index: copies judged by the input shape, this caller
	std::vector<uint32_t> grown_touched_; // the indexes grown_copies_ counts for this caller (reset per caller)
	uint32_t caller_ = kNoId;
	// The caller has a loop (its own or an inlined one) or EH regions.
	bool caller_loop_ = false;
	bool caller_eh_ = false;
	bool caller_wrapper_ = false;      // a fast-path wrapper (kWrapperCap)
	bool caller_leaf_wrapper_ = false; // kLeafWrapperCap
	bool caller_settled_ = false;      // never copied into another (kSettledSize)
	// A callee's loop moves into any caller. (The O0 back end gave a
	// function at most seven cross-block registers and none to one with
	// EH regions, so a second loop nest, or a loop in a function with
	// regions, ran from memory; the O1+ allocator gives every cross-block
	// value a register by weight, native/select/register_allocation.cpp.)
	bool loop_welcome(const Shape&) const { return true; }
	uint64_t budget_ = 0;
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

bool InlineWaveRunner::compatible(const Instruction& call, SymbolId callee) const
{
	const Function& function = program_.functions[program_.symbols[callee].detail];
	const Signature& sig = program_.signatures[function.signature];
	if (call.extra_count != sig.parameter_count || call.type != sig.result)
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

// Whether call passes a known function: its symbol, or the address of a
// slot of the caller whose one write stores one (a comparator object).
bool InlineWaveRunner::known_target(const Instruction& call) const
{
	const FunctionBody& body = *program_.functions[program_.symbols[graph_.functions[caller_]].detail].body;
	auto function_symbol = [&](Value v) {
		if (v.is(ValueKind::Temp) && body.temps[v.index].def != kNoId &&
			body.instructions[body.temps[v.index].def].op == Opcode::Addr)
			v = body.instructions[body.temps[v.index].def].ops[0];
		return v.is(ValueKind::Symbol) && is_function_kind(program_.symbols[v.index].kind);
	};
	for (uint32_t k = 0; k < call.extra_count; ++k)
	{
		const Value v = body.operands[call.extra_first + k];
		if (function_symbol(v))
			return true;
		if (!v.is(ValueKind::Temp) || body.temps[v.index].def == kNoId)
			continue;
		const Instruction& def = body.instructions[body.temps[v.index].def];
		if (def.op != Opcode::Addr || !def.ops[0].is(ValueKind::Slot))
			continue;
		const SlotId slot = def.ops[0].index;
		uint32_t writes = 0;
		bool stores_function = false;
		for (BlockId b : body.layout)
			for (InstId i = body.blocks[b].first; i != kNoId; i = body.instructions[i].next)
			{
				const Instruction& inst = body.instructions[i];
				const bool writer = inst.op == Opcode::Store || inst.op == Opcode::CopyObject ||
					inst.op == Opcode::ZeroInit;
				const Value target = inst.op == Opcode::Store || inst.op == Opcode::CopyObject ? inst.ops[1]
																							   : inst.ops[0];
				if (!writer)
					continue;
				const bool names = target == Value::slot(slot) ||
					(target.is(ValueKind::Temp) && body.temps[target.index].def != kNoId &&
						body.instructions[body.temps[target.index].def].op == Opcode::Addr &&
						body.instructions[body.temps[target.index].def].ops[0] == Value::slot(slot));
				if (!names)
					continue;
				++writes;
				stores_function = inst.op == Opcode::Store && function_symbol(inst.ops[0]);
			}
		if (writes == 1 && stores_function)
			return true;
	}
	return false;
}

bool InlineWaveRunner::admit(uint32_t g, bool handler_code, bool loop_site, uint32_t& charge, bool& once, bool dispatch)
{
	const SymbolId callee = graph_.functions[g];
	const SymbolMetadata& m = program_.symbols[callee].metadata;
	const Shape& shape = shapes_[g];
	once = false;
	if (graph_.recursive[g] || g == caller_ || (m.flags & (SF_NO_INLINE | SF_REPLACEABLE)) != 0 || shape.unsafe ||
		shape.eh || (handler_code && shape.unwinds))
		return false;
	const bool hint = (m.flags & (SF_INLINE_HINT | SF_FORCE_INLINE)) != 0;
	// Cold code is copied but priced as free: it runs at most once, on the
	// way to a noreturn call.
	const uint32_t size = shape.hot;
	const uint32_t original = callee < original_size_.size() ? original_size_[callee] : size;
	const bool single = discardable(program_, callee) && graph_.calls_in[g] == 1 && !graph_.observed[g];
	if ((m.flags & SF_FORCE_INLINE) != 0)
	{
		charge = 0;
		return true;
	}
	if (shape.loop && !loop_welcome(shape))
		return false;
	// A loop calling through a pointer the site passes as a known function
	// (std::push_heap's comparator) goes inline: the call turns direct.
	if (dispatch && shape.loop_dispatch && options_.level >= 2 && size <= kDispatchCap && size <= budget_)
	{
		charge = size;
		return true;
	}
	if ((caller_wrapper_ && size > kSlowSiteCap) || (caller_leaf_wrapper_ && size > kWrapperCap))
		return false;
	// O2+: a tiny loop around one call (a skip loop: take tokens until one
	// is not whitespace) goes inline: the loop stays, a call level goes.
	if (options_.level >= 2 && shape.loop && shape.calls == 1 && size <= kTinyLoopCap && size <= budget_)
	{
		charge = size;
		return true;
	}
	const uint32_t scale = loop_site ? kLoopSiteScale : 1;
	const bool fast_path = fast_path_of(shape, hint, scale);
	const bool settled = caller_settled_ && graph_.calls_in[g] <= kSettledCalls;
	const uint32_t settled_num = settled ? kSettledScaleNum : 1, settled_den = settled ? kSettledScaleDen : 1;
	if (wave_ == InlineWave::Late)
	{
		const bool leaf = shape.blocks == 1 && shape.returns == 1 && !shape.call;
		const bool call_free = options_.level >= 2 && !shape.hot_call;
		const uint32_t hint_cap = options_.limits.hint_late_cap != 0 ? options_.limits.hint_late_cap : kLateHintCap;
		const uint32_t cap = (leaf && !call_free         ? kLateLeafCap
									 : hint || call_free ? (hint ? hint_cap : kCallFreeCap) * settled_num / settled_den
														 : kLateCap) *
			(loop_site ? kLoopSiteScale : 1);
		charge = size;
		return (size <= cap || fast_path) && charge <= budget_;
	}
	if (wave_ == InlineWave::Ordinary)
	{
		// The ordinary policy on a shape: 1 a trivial leaf (free), 2 under
		// its cap or a fast path, 0 neither.
		auto fits = [&](const Shape& s) -> int {
			if (s.hot <= kTrivialLeaf && !s.call)
				return 1;
			const bool call_free = options_.level >= 2 && !s.hot_call;
			const bool shared_loop = shape.loop && graph_.calls_in[g] > 1 && !hint &&
				(!call_free || size > kCallFreeCap);
			const bool straight = s.blocks == 1 && s.returns == 1 && !s.hot_call;
			const uint32_t cap = (hint || call_free   ? (hint ? kHintCap : kCallFreeCap) * settled_num / settled_den
										 : straight   ? kOrdinaryLeafCap
										 : s.hot_call ? kOrdinaryCallCap
													  : kOrdinaryCap) *
				(loop_site ? kLoopSiteScale : 1);
			return !shared_loop && (s.hot <= cap || fast_path_of(s, hint, scale)) ? 2 : 0;
		};
		int fit = fits(shape);
		// Callees go first, so a callee may stand here grown past its
		// caps only by copies of its own callees. At a loop site, where
		// the call costs most, the policy reads the sizes and paths it had
		// on input (not when it absorbed a growth path: that stays out of
		// loops as it is; at most kGrownCopies sites per caller), and the
		// charge prices the copy as it is: absorbing a callee cannot evict
		// a function from its callers' loops.
		if (fit == 0 && loop_site && !shape.allocates && g < input_shapes_.size() && input_shapes_[g].hot < shape.hot &&
			grown_copies_[g] < kGrownCopies)
		{
			const Shape& input = input_shapes_[g];
			Shape judged = shape;
			judged.size = input.size;
			judged.hot = input.hot;
			judged.hot_call = input.hot_call;
			judged.blocks = input.blocks;
			judged.returns = input.returns;
			judged.call = input.call;
			judged.calls = input.calls;
			judged.fast = input.fast;
			judged.fast_return = input.fast_return;
			fit = fits(judged);
			if (fit != 0 && grown_copies_[g]++ == 0)
				grown_touched_.push_back(g);
		}
		if (fit == 1)
		{
			charge = 0;
			return true;
		}
		if (fit == 2)
		{
			charge = original > size ? original : size;
			if (charge <= budget_)
				return true;
		}
	}
	// The single-call policy (both the ordinary and the once wave): one
	// call site; from O2 also one calling function holding every site of
	// an internal callee (a lambda's operator() called from the loops of
	// the for_each it is passed to) when the copies together fit the cap.
	// The definition goes either way.
	const uint32_t once_cap = options_.limits.once_cap != 0 ? options_.limits.once_cap
															: kOnceCap * (options_.level >= 2 ? kOnceO2Scale : 1);
	const bool sole = options_.level >= 2 && m.binding == ir_model::SBM_INTERNAL && graph_.callers_in[g] == 1 &&
		!graph_.observed[g] && discardable(program_, callee) && size * graph_.calls_in[g] <= once_cap;
	if (!(single || sole) || size > once_cap || size > once_budget_ || size > budgets_.once_unit)
	{
		// A settled caller copies a callee with at most kSettledFewCalls
		// call sites up to kSettledFewCap (the callee stays: at most that
		// many copies), unless calls make much of it (pa32 pins those).
		if (caller_settled_ && wave_ != InlineWave::Once && graph_.calls_in[g] <= kSettledFewCalls &&
			size <= (shapes_[caller_].size <= kSettledSmallCaller ? 2 * kSettledFewCap : kSettledFewCap) &&
			size <= budget_ && shape.calls * kSettledCallShare <= shape.size)
		{
			charge = size;
			return true;
		}
		return false;
	}
	charge = size;
	once = true;
	return true;
}

// A partial copy of `g` at `call` (partial_cut): from O2, for a callee the
// late wave keeps out of line, taking scalar arguments directly and
// returning nothing or a scalar. A caller's call to itself qualifies: the
// copy holds the clean prefix and calls the callee at the cuts, so it never
// recurses itself, and the copies are not revisited in the wave. (Other
// recursive callees stay out: copies at every site of a call cycle, the
// parser's, take the budgets the ordinary callees of those sites need.)
// `source`, when given, is the body cut and copied (the caller's original
// under recursive inlining: the live body's cuts would sit below the
// copies).
bool InlineWaveRunner::admit_partial(uint32_t g, const Instruction& call, bool handler_code, bool loop_site,
	PartialCut& partial, uint32_t& charge, const FunctionBody* source)
{
	const SymbolId callee = graph_.functions[g];
	const SymbolMetadata& m = program_.symbols[callee].metadata;
	const Shape& shape = shapes_[g];
	if (options_.level < 2 || (graph_.recursive[g] && g != caller_) ||
		(m.flags & (SF_NO_INLINE | SF_REPLACEABLE)) != 0 || shape.unsafe || shape.eh ||
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
	const uint32_t cap = (graph_.calls_in[g] <= 2 ? kPartialFewCap
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
	for (uint32_t k = 0; k < call.extra_count; ++k)
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

// Whether a path from the entry reaches a return through blocks whose
// calls all go to callees every wave copies whole (forced, or trivial
// call-free leaves).
bool InlineWaveRunner::leaf_return_path(const FunctionBody& body, const Cfg& cfg) const
{
	auto leaf_calls = [&](BlockId b) {
		for (InstId i = body.blocks[b].first; i != kNoId; i = body.instructions[i].next)
		{
			const Instruction& inst = body.instructions[i];
			if (inst.op != Opcode::Call)
				continue;
			if (!inst.ops[0].is(ValueKind::Symbol) || graph_.index_of[inst.ops[0].index] == kNoId)
				return false;
			const uint32_t g = graph_.index_of[inst.ops[0].index];
			const bool forced = (program_.symbols[inst.ops[0].index].metadata.flags & SF_FORCE_INLINE) != 0;
			if (!forced && (shapes_[g].size > kTrivialLeaf || shapes_[g].call))
				return false;
		}
		return true;
	};
	std::vector<uint8_t> seen(cfg.block_count, 0);
	std::vector<BlockId> work;
	if (body.layout.empty() || !leaf_calls(body.layout[0]))
		return false;
	seen[body.layout[0]] = 1;
	work.push_back(body.layout[0]);
	while (!work.empty())
	{
		const BlockId b = work.back();
		work.pop_back();
		if (body.instructions[body.blocks[b].last].op == Opcode::Return)
			return true;
		for (uint32_t k = cfg.succ_first[b]; k < cfg.succ_first[b + 1]; ++k)
		{
			const BlockId s = cfg.succs[k];
			if (!seen[s] && cfg.reachable(s) && leaf_calls(s))
			{
				seen[s] = 1;
				work.push_back(s);
			}
		}
	}
	return false;
}

bool InlineWaveRunner::process(SymbolId caller)
{
	caller_ = graph_.index_of[caller];
	caller_loop_ = shapes_[caller_].loop;
	caller_eh_ = shapes_[caller_].eh;
	caller_wrapper_ = shapes_[caller_].fast_return && shapes_[caller_].hot <= kWrapperCap;
	caller_settled_ = options_.level >= 2 &&
		(graph_.calls_in[caller_] == 0 ||
			(shapes_[caller_].size > kSettledSize &&
				!(discardable(program_, caller) && graph_.calls_in[caller_] == 1)));
	const uint32_t default_budget = options_.limits.caller_budget != 0 ? options_.limits.caller_budget : kCallerBudget;
	budget_ = default_budget;
	once_budget_ = options_.limits.once_caller_budget != 0
		? options_.limits.once_caller_budget
		: kOnceCallerBudget * (options_.level >= 2 ? kOnceO2Scale : 1);
	folded_budget_ = kFoldedCallerBudget;
	for (uint32_t g : grown_touched_)
		grown_copies_[g] = 0;
	grown_touched_.clear();
	FunctionBody& body = *program_.functions[program_.symbols[caller].detail].body;
	static Cfg scratch; // (reused for its capacity: every build overwrites what it fills)
	Cfg& cfg = scratch;
	// Dominators only for handler code and the loops (find_loops).
	build_cfg_light(program_, body, cfg);
	if (!cfg.handlers.empty() || caller_loop_)
		complete_cfg(cfg);
	caller_leaf_wrapper_ = !caller_wrapper_ && options_.level >= 2 && shapes_[caller_].hot <= kLeafWrapperCap &&
		discardable(program_, caller) && leaf_return_path(body, cfg);
	// Handler code: blocks dominated by a handler target (new blocks
	// inherit their split block's answer); none without a handler.
	std::vector<uint8_t> handler(body.blocks.size(), 0);
	if (!cfg.handlers.empty())
		for (BlockId b : cfg.rpo)
			handler[b] = (cfg.flags[b] & BF_HANDLER) != 0 || (cfg.idom[b] != kNoId && handler[cfg.idom[b]]);
	// Cold blocks: every path from them ends in a noreturn call or an
	// unreachable (O2+: only trivial and forced callees go inline there;
	// new blocks inherit their split block's answer).
	std::vector<uint8_t> cold(body.blocks.size(), 0);
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
	// Loop blocks (new blocks inherit their split block's answer).
	std::vector<uint8_t> in_loop(body.blocks.size(), 0);
	if (caller_loop_)
	{
		LoopForest forest;
		find_loops(body, cfg, forest);
		for (BlockId b : cfg.rpo)
			in_loop[b] = forest.innermost[b] != kNoId;
	}
	std::vector<BlockId> work(body.layout.rbegin(), body.layout.rend());
	std::vector<BlockId> layout_next(body.blocks.size(), kNoId);
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
		!shapes_[caller_].unsafe && !shapes_[caller_].loop && shapes_[caller_].size <= kRecursiveCap)
	{
		uint64_t self_calls = 0;
		for (BlockId b : body.layout)
			for (InstId i = body.blocks[b].first; i != kNoId; i = body.instructions[i].next)
				self_calls += body.instructions[i].op == Opcode::Call &&
					body.instructions[i].ops[0].is(ValueKind::Symbol) && body.instructions[i].ops[0].index == caller;
		uint64_t copies = 0, level = 1;
		for (uint32_t d = 1; d <= kRecursiveDepth && self_calls >= kRecursiveTreeCalls; ++d)
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
	std::vector<uint8_t> depth(body.blocks.size(), 0);
	while (!work.empty())
	{
		const BlockId b = work.back();
		work.pop_back();
		InstId prev = kNoId;
		for (InstId i = body.blocks[b].first; i != kNoId; prev = i, i = body.instructions[i].next)
		{
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
			const uint32_t g = graph_.index_of[callee];
			uint32_t charge = 0;
			bool once = false;
			if (g == kNoId || !compatible(inst, callee))
				continue;
			SymbolId copy = kNoId;
			PartialCut partial;
			bool partly = false, fully = false;
			const bool dispatch = shapes_[g].loop_dispatch && known_target(inst);
			const bool cold_site = cold[b] != 0 && shapes_[g].size > kTrivialLeaf &&
				(program_.symbols[callee].metadata.flags & SF_FORCE_INLINE) == 0;
			if (cold_site || !admit(g, handler[b] != 0, in_loop[b] != 0, charge, once, dispatch))
			{
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
			if (!once && size > unit)
			{
				if (copy != kNoId)
					release(copy);
				continue;
			}
			if (!once)
				unit -= size;
			const bool loop = copy != kNoId ? shape_of(program_, copy, nounwind_).loop : shapes_[g].loop;
			clone_call(program_, body, b, prev, i, kb, layout_next, repl, partly ? &partial : nullptr);
			if (copy != kNoId)
				release(copy);
			handler.resize(body.blocks.size(), handler[b]);
			in_loop.resize(body.blocks.size(), in_loop[b]);
			cold.resize(body.blocks.size(), cold[b]);
			depth.resize(body.blocks.size(), static_cast<uint8_t>(depth[b] + 1));
			depth[before] = depth[b];
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
	build_call_graph(program_, graph_);
	shapes_.resize(graph_.functions.size());
	folded_failures_.assign(graph_.functions.size(), 0);
	grown_copies_.assign(graph_.functions.size(), 0);
	grown_touched_.clear();
	for (uint32_t f = 0; f < graph_.functions.size(); ++f)
		shapes_[f] = shape_of(program_, graph_.functions[f], nounwind_);
	if (wave_ == InlineWave::Ordinary)
		input_shapes_ = shapes_;
	for (uint32_t f : graph_.bottom_up)
	{
		const SymbolId caller = graph_.functions[f];
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
		if (changed || wave_ == InlineWave::Ordinary)
		{
			simplify_(caller);
			shapes_[f] = shape_of(program_, caller, nounwind_);
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

} // namespace opt
