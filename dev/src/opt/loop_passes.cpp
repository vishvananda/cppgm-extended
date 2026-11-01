#include <cstring>

#include "lowir/builder.h"
#include "opt/loops.h"
#include "opt/passes.h"
#include "opt/scalar_fold.h"

namespace opt {

using namespace lowir;

namespace {

// An induction variable: a header phi entering from outside with `init`
// and from the single latch with `next` = phi stepped by a constant
// (`index T phi, c` for a pointer, `add`/`sub` for an integer).
struct Induction
{
	InstId phi = kNoId;
	TempId value = kNoId;
	Value init;
	TempId next = kNoId;
	int64_t step_bytes = 0; // pointer step in bytes, or integer step
	bool pointer = false;
};

// The loop's single exit: a branch whose taken side leaves the loop exactly
// when `iv == bound` (a `ne` comparison continuing, or `eq` leaving).
struct ExitTest
{
	BlockId exiting = kNoId;
	BlockId exit = kNoId;
	Value compared; // the induction value tested (the phi or its next)
	Value bound;
	Value exit_value;   // the tested value when the loop leaves (the bound, or a counted literal)
	int64_t trips = -1; // exits at the trips-th test (0-based) of a counted loop; -1 unknown
};

class LoopPass
{
public:
	explicit LoopPass(FunctionContext& context) : c_(context), body_(context.body), cfg_(context.cfg) {}

	bool run(bool fresh);
	bool unroll();
	bool mark_no_wrap();
	bool hoist_loads();
	bool version_fills();

private:
	bool try_unroll(uint32_t loop);
	bool try_unroll_search(uint32_t loop);
	bool unroll_search_shape(uint32_t loop, int64_t trips, TempId iv, std::vector<uint8_t>& member,
		std::vector<uint8_t>& owned, BlockId& through);
	void add_exit_phis(const std::vector<uint8_t>& member, const std::vector<uint8_t>& owned, BlockId exit);
	bool invariant(uint32_t loop, Value v) const;
	bool induction(uint32_t loop, InstId phi, Induction& out) const;
	void add_narrow_accumulators(uint32_t loop, std::vector<Induction>& ivs) const;
	bool exit_test(uint32_t loop, ExitTest& out, std::vector<Induction>& ivs) const;
	bool effect_free(uint32_t loop) const;
	bool fill_pattern(uint32_t loop, const Induction& iv, InstId& store, InstId& load) const;
	bool copy_pattern(
		uint32_t loop, const std::vector<Induction>& ivs, const ExitTest& test, uint32_t& src, uint32_t& dst) const;
	bool used_outside_only(uint32_t loop, const Value& a, const Value& b) const;
	void emit_copy(uint32_t loop, const Induction& src, const Induction& dst, const ExitTest& test);
	SymbolId builtin_symbol(const char* name, const char* object, const Type* types, uint32_t count);
	bool builtin_available(const char* name, const char* object) const;
	bool private_values(uint32_t loop, const ExitTest& test, const std::vector<Induction>& ivs) const;
	bool final_values(uint32_t loop, const ExitTest& test, const std::vector<Induction>& ivs);
	bool bypass(uint32_t loop, const ExitTest& test);
	void emit_fill(uint32_t loop, const Induction& iv, const ExitTest& test, InstId store, InstId load);
	SymbolId fill_symbol();
	bool fill_available() const;
	bool wide_fill_pattern(uint32_t loop, const ExitTest& test, const Induction& iv, InstId& store) const;
	bool widen_fill(uint32_t loop, const ExitTest& test, const std::vector<Induction>& ivs);
	bool try_loop(uint32_t loop);
	bool indexed_stores(uint32_t loop, const ExitTest& test, const std::vector<Induction>& ivs, bool rewrite);
	bool make_preheader(uint32_t loop);
	bool count_to_pointer(uint32_t loop);
	bool hoist_stored_load(uint32_t loop);
	bool hoist_from(uint32_t loop, bool apply);
	bool entry_objects_kept(uint32_t loop, uint32_t parameter);
	bool innermost(uint32_t loop) const
	{
		for (const Loop& l : forest_.loops)
			if (l.parent == loop)
				return false;
		return true;
	}

	FunctionContext& c_;
	FunctionBody& body_;
	Cfg& cfg_;
	LoopForest forest_;
	std::vector<uint32_t> hoist_uses_; // use counts for hoist_from (empty: not counted)
	// Per block of cfg_: the by-address parameters (bit k: parameter k)
	// whose objects are known live at its end (entry_objects_kept; empty:
	// not computed for this graph).
	std::vector<uint64_t> live_objects_;
};

bool LoopPass::invariant(uint32_t loop, Value v) const
{
	if (!v.is(ValueKind::Temp) || v.index < body_.parameter_count)
		return true;
	const InstId def = body_.temps[v.index].def;
	if (def != kNoId && def >= c_.block_of.size())
		return true; // made by this pass outside the loop
	return def != kNoId && c_.block_of[def] != kNoId && !in_loop(forest_, loop, c_.block_of[def]);
}

bool LoopPass::induction(uint32_t loop, InstId id, Induction& out) const
{
	const Loop& l = forest_.loops[loop];
	const Instruction& phi = body_.instructions[id];
	if (phi.op != Opcode::Phi || phi.extra_count != 4 || l.latches.size() != 1)
		return false;
	out.phi = id;
	out.value = phi.result;
	out.init = phi_incoming(body_, phi, l.preheader != kNoId ? l.preheader : kNoId);
	const Value next = phi_incoming(body_, phi, l.latches[0]);
	for (uint32_t k = 0; k < 4; k += 2)
		if (body_.operands[phi.extra_first + k].index != l.latches[0])
			out.init = body_.operands[phi.extra_first + k + 1];
	if (!next.is(ValueKind::Temp) || !invariant(loop, out.init))
		return false;
	const InstId def = body_.temps[next.index].def;
	if (def == kNoId)
		return false;
	const Instruction& step = body_.instructions[def];
	if (!(step.ops[0].is(ValueKind::Temp) && step.ops[0].index == phi.result) || !step.ops[1].is(ValueKind::Integer))
		return false;
	const int64_t c = static_cast<int64_t>(body_.literals[step.ops[1].index].low);
	out.next = next.index;
	if (step.op == Opcode::Index && phi.type.is_pointer())
	{
		out.pointer = true;
		out.step_bytes = c * static_cast<int64_t>(storage_bytes(step.type));
		return out.step_bytes != 0;
	}
	if (step.op == Opcode::Binary && step.type == phi.type && phi.type.is_integer() &&
		(static_cast<BinaryOp>(step.sub) == BinaryOp::Add || static_cast<BinaryOp>(step.sub) == BinaryOp::Sub))
	{
		const Literal bits = normalize_integer(step.type, body_.literals[step.ops[1].index]);
		const int64_t signed_c = static_cast<int64_t>(bits.low);
		out.step_bytes = static_cast<BinaryOp>(step.sub) == BinaryOp::Add ? signed_c : -signed_c;
		return out.step_bytes != 0;
	}
	return false;
}

// A narrow accumulator stepping through its promotion, `next = trunc T (add
// (ext r), c)`, steps by c in T whichever extension it took. For the exit
// values only: it may wrap, which the other readers of an induction exclude.
void LoopPass::add_narrow_accumulators(uint32_t loop, std::vector<Induction>& ivs) const
{
	const Loop& l = forest_.loops[loop];
	if (l.latches.size() != 1)
		return;
	const auto convert_of = [&](Value v, bool trunc) -> const Instruction* {
		if (!v.is(ValueKind::Temp) || body_.temps[v.index].def == kNoId)
			return nullptr;
		const Instruction& def = body_.instructions[body_.temps[v.index].def];
		if (def.op != Opcode::Convert)
			return nullptr;
		const ConvertOp op = static_cast<ConvertOp>(def.sub);
		return (trunc ? op == ConvertOp::Trunc : op == ConvertOp::SExt || op == ConvertOp::ZExt) ? &def : nullptr;
	};
	for (InstId i = body_.blocks[l.header].first; i != kNoId && body_.instructions[i].op == Opcode::Phi;
		i = body_.instructions[i].next)
	{
		const Instruction& phi = body_.instructions[i];
		if (phi.extra_count != 4 || !phi.type.is_integer())
			continue;
		Induction iv;
		iv.phi = i;
		iv.value = phi.result;
		const Value next = phi_incoming(body_, phi, l.latches[0]);
		for (uint32_t k = 0; k < 4; k += 2)
			if (body_.operands[phi.extra_first + k].index != l.latches[0])
				iv.init = body_.operands[phi.extra_first + k + 1];
		const Instruction* narrow = convert_of(next, true);
		if (narrow == nullptr || narrow->type != phi.type || !invariant(loop, iv.init) ||
			!narrow->ops[0].is(ValueKind::Temp))
			continue;
		const Instruction& sum = body_.instructions[body_.temps[narrow->ops[0].index].def];
		if (sum.op != Opcode::Binary || !sum.ops[1].is(ValueKind::Integer) ||
			(static_cast<BinaryOp>(sum.sub) != BinaryOp::Add && static_cast<BinaryOp>(sum.sub) != BinaryOp::Sub))
			continue;
		const Instruction* wide = convert_of(sum.ops[0], false);
		if (wide == nullptr || convert_source_type(*wide) != phi.type || !wide->ops[0].is(ValueKind::Temp) ||
			wide->ops[0].index != phi.result)
			continue;
		const int64_t c = static_cast<int64_t>(normalize_integer(phi.type, body_.literals[sum.ops[1].index]).low);
		iv.next = next.index;
		iv.step_bytes = static_cast<BinaryOp>(sum.sub) == BinaryOp::Add ? c : -c;
		if (iv.step_bytes != 0)
			ivs.push_back(iv);
	}
}

// The test number (0-based) at which a counted loop leaves: the induction
// starts at a literal and steps by a literal, the bound is a literal, and
// the tested values move from continuing to leaving without leaving the
// comparison's integer range (no wrap). `next` tests the stepped value.
bool counted_trips(Type type, ComparePredicate pred, bool continue_on_true, __int128 init, __int128 step,
	__int128 bound, bool next, int64_t& trips, __int128& last)
{
	const unsigned bits = integer_bits(type.kind());
	const bool unsigned_order = pred == ComparePredicate::Ult || pred == ComparePredicate::Ule ||
		pred == ComparePredicate::Ugt || pred == ComparePredicate::Uge;
	if (bits == 0 || bits > 64)
		return false;
	const __int128 lo = unsigned_order ? 0 : -(static_cast<__int128>(1) << (bits - 1));
	const __int128 hi = unsigned_order ? (static_cast<__int128>(1) << bits) - 1
									   : (static_cast<__int128>(1) << (bits - 1)) - 1;
	auto in_range = [&](__int128 v) { return v >= lo && v <= hi; };
	// Literals read in the comparison's order.
	auto order = [&](__int128 v) {
		const __int128 mask = (static_cast<__int128>(1) << bits) - 1;
		v &= mask;
		if (!unsigned_order && v > hi)
			v -= mask + 1;
		return v;
	};
	init = order(init);
	bound = order(bound);
	auto holds = [&](__int128 x) {
		bool r;
		switch (pred)
		{
		case ComparePredicate::Eq:
			r = x == bound;
			break;
		case ComparePredicate::Ne:
			r = x != bound;
			break;
		case ComparePredicate::Lt:
		case ComparePredicate::Ult:
			r = x < bound;
			break;
		case ComparePredicate::Le:
		case ComparePredicate::Ule:
			r = x <= bound;
			break;
		case ComparePredicate::Gt:
		case ComparePredicate::Ugt:
			r = x > bound;
			break;
		default:
			r = x >= bound;
			break;
		}
		return r == continue_on_true;
	};
	const __int128 first = init + (next ? step : 0);
	if (!in_range(first))
		return false;
	if (!holds(first))
	{
		trips = 0;
		last = first;
		return true;
	}
	// The first k with !holds(first + k*step), found by the side the bound
	// lies on; any candidate is checked exactly.
	__int128 k;
	const __int128 gap = bound - first;
	if (step > 0 ? gap < 0 : gap > 0)
		k = 1; // already past the bound: the next value decides
	else
		k = gap / step;
	for (__int128 probe = k > 1 ? k - 1 : 1; probe <= k + 2; ++probe)
	{
		const __int128 x = first + probe * step;
		if (!in_range(x))
			return false;
		if (!holds(x))
		{
			// Every earlier value continued (monotone between first and x).
			for (__int128 q = 1; q < probe && q < 3; ++q)
				if (!holds(first + q * step))
					return false;
			if (probe > 0x7fffffff)
				return false;
			trips = static_cast<int64_t>(probe);
			last = x;
			return true;
		}
	}
	return false;
}

// One exit edge, branching on `cmp ne|eq` of an induction value against
// an invariant. A pointer induction may step by any constant (it moves
// within its object until it meets the bound); an integer one steps by
// one, so it meets any bound.
bool LoopPass::exit_test(uint32_t loop, ExitTest& out, std::vector<Induction>& ivs) const
{
	const Loop& l = forest_.loops[loop];
	uint32_t exits = 0;
	for (BlockId b : l.blocks)
		for (uint32_t k = cfg_.succ_first[b]; k < cfg_.succ_first[b + 1]; ++k)
			if (!in_loop(forest_, loop, cfg_.succs[k]))
			{
				++exits;
				out.exiting = b;
				out.exit = cfg_.succs[k];
			}
	if (exits != 1 || (cfg_.flags[out.exit] & BF_HANDLER) != 0)
		return false;
	for (InstId i = body_.blocks[l.header].first; i != kNoId && body_.instructions[i].op == Opcode::Phi;
		i = body_.instructions[i].next)
	{
		Induction iv;
		if (induction(loop, i, iv))
			ivs.push_back(iv);
	}
	const Instruction& term = body_.instructions[body_.blocks[out.exiting].last];
	if (term.op != Opcode::Branch || !term.ops[0].is(ValueKind::Temp) || term.ops[1].index == term.ops[2].index)
		return false;
	const InstId def = body_.temps[term.ops[0].index].def;
	if (def == kNoId || body_.instructions[def].op != Opcode::Compare)
		return false;
	const Instruction& cmp = body_.instructions[def];
	const bool leave_on_true = term.ops[1].index == out.exit;
	for (const Induction& iv : ivs)
		for (unsigned side = 0; side < 2; ++side)
		{
			const Value v = cmp.ops[side], other = cmp.ops[1 - side];
			if (!v.is(ValueKind::Temp) || (v.index != iv.value && v.index != iv.next) || !invariant(loop, other))
				continue;
			ComparePredicate pred = side == 0 ? static_cast<ComparePredicate>(cmp.sub)
											  : swapped(static_cast<ComparePredicate>(cmp.sub));
			// Unsigned, `x > 0` is `x != 0` and `x <= 0` is `x == 0` (a count
			// down to zero).
			if (other.is(ValueKind::Integer) && body_.literals[other.index].low == 0 &&
				body_.literals[other.index].high == 0)
			{
				if (pred == ComparePredicate::Ugt)
					pred = ComparePredicate::Ne;
				else if (pred == ComparePredicate::Ule)
					pred = ComparePredicate::Eq;
			}
			out.compared = v;
			out.bound = other;
			out.exit_value = other;
			out.trips = -1;
			// A counted loop: literal start, step and bound.
			if (!iv.pointer && iv.init.is(ValueKind::Integer) && other.is(ValueKind::Integer) && cmp.type.is_integer())
			{
				const Literal a = normalize_integer(cmp.type, body_.literals[iv.init.index]);
				const Literal b = normalize_integer(cmp.type, body_.literals[other.index]);
				__int128 last;
				int64_t trips;
				if (counted_trips(cmp.type, pred, !leave_on_true, static_cast<__int128>(static_cast<int64_t>(a.low)),
						iv.step_bytes, static_cast<__int128>(static_cast<int64_t>(b.low)), v.index == iv.next, trips,
						last))
				{
					out.trips = trips;
					const Literal bits = { static_cast<uint64_t>(last), static_cast<uint64_t>(last >> 64) };
					out.exit_value = integer_value(body_, normalize_integer(cmp.type, bits));
					return true;
				}
			}
			// Otherwise an equality exit the induction must meet: a pointer
			// (moving within its object) or an integer stepping by one.
			const bool equality = (pred == ComparePredicate::Ne && !leave_on_true) ||
				(pred == ComparePredicate::Eq && leave_on_true);
			if (equality && (iv.pointer || iv.step_bytes == 1 || iv.step_bytes == -1))
				return true;
		}
	return false;
}

// Only phis, nontrapping pure values and control flow.
bool LoopPass::effect_free(uint32_t loop) const
{
	for (BlockId b : forest_.loops[loop].blocks)
		for (InstId i = body_.blocks[b].first; i != kNoId; i = body_.instructions[i].next)
		{
			const Instruction& inst = body_.instructions[i];
			if (inst.op == Opcode::Jump || inst.op == Opcode::Branch)
				continue;
			if (!is_pure(inst))
				return false;
			if (inst.op == Opcode::Binary)
			{
				const BinaryOp op = static_cast<BinaryOp>(inst.sub);
				if (op == BinaryOp::Div || op == BinaryOp::Mod || op == BinaryOp::UDiv || op == BinaryOp::UMod)
					return false;
			}
		}
	return true;
}

// Values the loop defines are used outside it only as the tested
// induction value at the exit (where it equals the bound).
bool LoopPass::private_values(uint32_t loop, const ExitTest& test, const std::vector<Induction>& ivs) const
{
	(void)ivs;
	bool ok = true;
	for (BlockId b : body_.layout)
	{
		if (in_loop(forest_, loop, b))
			continue;
		for (InstId i = body_.blocks[b].first; i != kNoId && ok; i = body_.instructions[i].next)
		{
			const Instruction& inst = body_.instructions[i];
			for_each_operand(body_, inst, [&](const Value& v) {
				if (!v.is(ValueKind::Temp) || invariant(loop, v))
					return;
				if (!(v == test.compared))
					ok = false;
			});
		}
	}
	return ok;
}

// A loop tested at its header on an induction phi it leaves when the phi
// meets the bound (a counted trip number, an integer stepping by one, or a
// pointer, which moves within its object until it meets the bound), whose
// values read after it are header inductions: the preheader computes the
// trip count N and each such induction's exit value init + N * step (in
// its own type's arithmetic, as the loop's steps wrap), and the reads
// after the loop take them. Then only the tested value is read after it
// (bypass gives it the bound).
bool LoopPass::final_values(uint32_t loop, const ExitTest& test, const std::vector<Induction>& ivs)
{
	const Loop& l = forest_.loops[loop];
	if (l.preheader == kNoId || test.exiting != l.header || !test.compared.is(ValueKind::Temp))
		return false;
	const Induction* tested = nullptr;
	for (const Induction& iv : ivs)
		if (iv.value == test.compared.index)
			tested = &iv;
	if (tested == nullptr ||
		(test.trips < 0 && !tested->pointer && tested->step_bytes != 1 && tested->step_bytes != -1))
		return false;
	auto fits = [](Type t) {
		return t.is_pointer() || (t.is_integer() && t.kind() != TypeKind::I128 && t.kind() != TypeKind::I1);
	};
	if (!fits(body_.temps[tested->value].type))
		return false;
	// The inductions read after the loop (the tested one included).
	std::vector<uint8_t> read(ivs.size(), 0);
	bool ok = true;
	for (BlockId b : body_.layout)
	{
		if (in_loop(forest_, loop, b))
			continue;
		for (InstId i = body_.blocks[b].first; i != kNoId && ok; i = body_.instructions[i].next)
			for_each_operand(body_, body_.instructions[i], [&](const Value& v) {
				if (!v.is(ValueKind::Temp) || invariant(loop, v))
					return;
				bool found = false;
				for (size_t k = 0; k < ivs.size(); ++k)
					if (ivs[k].value == v.index)
					{
						read[k] = 1;
						found = true;
					}
				ok = ok && found;
			});
	}
	if (!ok)
		return false;
	for (size_t k = 0; k < ivs.size(); ++k)
		if (read[k] && !fits(body_.temps[ivs[k].value].type))
			return false;
	// The trip count, as i64, in the preheader.
	const BlockId pre = l.preheader;
	const InstId term = body_.blocks[pre].last;
	InstId at = previous_in_block(body_, pre, term);
	const uint32_t location = body_.instructions[term].location;
	auto emit = [&](Instruction inst) {
		inst.location = location;
		const InstId id = append_instruction(body_, inst);
		body_.instructions[id].result = append_temp(body_, inst.type, id);
		insert_after(body_, pre, at, id);
		at = id;
		return Value::temp(body_.instructions[id].result);
	};
	auto binary = [&](BinaryOp op, Type type, Value a, Value b) {
		Instruction inst = make_instruction(Opcode::Binary, type);
		inst.sub = static_cast<uint8_t>(op);
		inst.ops[0] = a;
		inst.ops[1] = b;
		return emit(inst);
	};
	auto literal = [&](int64_t v) { return integer_value(body_, integer_literal(v)); };
	const Type i64 = Type::scalar(TypeKind::I64);
	Value trips;
	if (test.trips >= 0)
		trips = literal(test.trips);
	else if (tested->pointer)
	{
		const Value bytes = binary(BinaryOp::Sub, Type::scalar(TypeKind::Ptr), test.bound, tested->init);
		Instruction count = make_instruction(Opcode::Binary, i64);
		count.sub = static_cast<uint8_t>(BinaryOp::Div);
		count.flags = IF_EXACT;
		count.ops[0] = bytes;
		count.ops[1] = literal(tested->step_bytes);
		trips = emit(count);
	}
	else
	{
		const Type type = body_.temps[tested->value].type;
		const Value diff = tested->step_bytes == 1 ? binary(BinaryOp::Sub, type, test.bound, tested->init)
												   : binary(BinaryOp::Sub, type, tested->init, test.bound);
		trips = diff;
		if (type.kind() != TypeKind::I64)
		{
			Instruction widen = make_instruction(Opcode::Convert, i64);
			widen.sub = static_cast<uint8_t>(ConvertOp::ZExt);
			widen.ops[0] = diff;
			set_convert_source_type(widen, type);
			trips = emit(widen);
		}
	}
	// Each read induction's exit value.
	std::vector<Value> exit(ivs.size(), Value::none());
	for (size_t k = 0; k < ivs.size(); ++k)
	{
		if (!read[k])
			continue;
		const Induction& iv = ivs[k];
		if (&iv == tested)
		{
			exit[k] = test.exit_value;
			continue;
		}
		const Type type = body_.temps[iv.value].type;
		if (iv.pointer)
		{
			const Value bytes = binary(BinaryOp::Mul, i64, trips, literal(iv.step_bytes));
			Instruction step = make_instruction(Opcode::Index, Type::scalar(TypeKind::I8));
			step.ops[0] = iv.init;
			step.ops[1] = bytes;
			exit[k] = emit(step);
			continue;
		}
		Value count = trips;
		if (type.kind() != TypeKind::I64)
		{
			Instruction narrow = make_instruction(Opcode::Convert, type);
			narrow.sub = static_cast<uint8_t>(ConvertOp::Trunc);
			narrow.ops[0] = trips;
			set_convert_source_type(narrow, i64);
			count = emit(narrow);
		}
		const Value product = binary(BinaryOp::Mul, type, count, literal(iv.step_bytes));
		exit[k] = binary(BinaryOp::Add, type, iv.init, product);
	}
	for (BlockId b : body_.layout)
	{
		if (in_loop(forest_, loop, b))
			continue;
		for (InstId i = body_.blocks[b].first; i != kNoId; i = body_.instructions[i].next)
			for_each_operand(body_, body_.instructions[i], [&](Value& v) {
				if (!v.is(ValueKind::Temp) || v.index == test.compared.index)
					return;
				for (size_t k = 0; k < ivs.size(); ++k)
					if (read[k] && ivs[k].value == v.index)
						v = exit[k];
			});
	}
	map_instruction_blocks(body_, c_.block_of);
	c_.invalidate_block_of();
	return true;
}

// Every entry into the loop goes to the exit instead; the exit's phis take
// the value they took from the exiting edge (the bound for the tested
// induction value) for each former entry.
bool LoopPass::bypass(uint32_t loop, const ExitTest& test)
{
	const Loop& l = forest_.loops[loop];
	std::vector<BlockId> entries;
	for (uint32_t k = cfg_.pred_first[l.header]; k < cfg_.pred_first[l.header + 1]; ++k)
		if (!in_loop(forest_, loop, cfg_.preds[k]))
			entries.push_back(cfg_.preds[k]);
	auto exit_value = [&](Value v) { return v == test.compared ? test.exit_value : v; };
	for (BlockId p : entries)
	{
		// An entry that already reaches the exit with other phi inputs goes
		// through a new jump-only block on its loop edge.
		bool conflict = false;
		for (InstId i = body_.blocks[test.exit].first; i != kNoId && body_.instructions[i].op == Opcode::Phi;
			i = body_.instructions[i].next)
		{
			const Instruction& phi = body_.instructions[i];
			const Value existing = phi_incoming(body_, phi, p);
			conflict = conflict ||
				(!existing.is(ValueKind::None) && !(existing == exit_value(phi_incoming(body_, phi, test.exiting))));
		}
		BlockId from = p;
		if (conflict)
		{
			from = append_block(body_, BlockKind::LoopExit);
			Instruction jump = make_instruction(Opcode::Jump, Type::scalar(TypeKind::Void));
			jump.ops[0] = Value::block(test.exit);
			insert_after(body_, from, kNoId, append_instruction(body_, jump));
			std::vector<BlockId> layout;
			for (BlockId b : body_.layout)
			{
				layout.push_back(b);
				if (b == p)
					layout.push_back(from);
			}
			body_.layout.swap(layout);
			for_each_successor(body_, body_.instructions[body_.blocks[p].last], [&](BlockId& s) {
				if (s == l.header)
					s = from;
			});
		}
		else
			for_each_successor(body_, body_.instructions[body_.blocks[p].last], [&](BlockId& s) {
				if (s == l.header)
					s = test.exit;
			});
		for (InstId i = body_.blocks[test.exit].first; i != kNoId && body_.instructions[i].op == Opcode::Phi;
			i = body_.instructions[i].next)
		{
			Instruction& phi = body_.instructions[i];
			if (!phi_incoming(body_, phi, from).is(ValueKind::None))
				continue;
			std::vector<Value> pairs(
				body_.operands.begin() + phi.extra_first, body_.operands.begin() + phi.extra_first + phi.extra_count);
			pairs.push_back(Value::block(from));
			pairs.push_back(exit_value(phi_incoming(body_, phi, test.exiting)));
			phi.extra_first = append_operands(body_, pairs.data(), static_cast<uint32_t>(pairs.size()));
			phi.extra_count = static_cast<uint32_t>(pairs.size());
		}
	}
	remove_phi_incoming(body_, test.exit, test.exiting);
	// The loop's blocks are unreachable now: they leave the layout (their
	// successors outside the loop are the exit, already repaired).
	std::vector<BlockId> layout;
	for (BlockId b : body_.layout)
		if (!in_loop(forest_, loop, b))
			layout.push_back(b);
	body_.layout.swap(layout);
	// Other uses after the loop read the tested value: the bound.
	if (!invariant(loop, test.compared))
		for (BlockId b : body_.layout)
			if (!in_loop(forest_, loop, b))
				for (InstId i = body_.blocks[b].first; i != kNoId; i = body_.instructions[i].next)
					for_each_operand(body_, body_.instructions[i], [&](Value& v) {
						if (v == test.compared)
							v = test.exit_value;
					});
	return true;
}

SymbolId LoopPass::fill_symbol()
{
	const Type types[3] = { Type::scalar(TypeKind::Ptr), Type::scalar(TypeKind::I32), Type::scalar(TypeKind::I64) };
	return builtin_symbol("__cppgm_builtin_memset", "cppgm_builtin_memset", types, 3);
}

// A runtime builtin `name` (the back end's `object`) taking `types` and
// returning a pointer, declared on first use (kNoId when the name is taken
// by something else).
SymbolId LoopPass::builtin_symbol(const char* name, const char* object, const Type* types, uint32_t count)
{
	if (!builtin_available(name, object))
		return kNoId;
	Program& program = c_.program;
	const NameId id = program.names.intern(name, static_cast<uint32_t>(std::strlen(name)));
	const SymbolId symbol = program.symbol_for(id);
	if (program.symbols[symbol].kind != SymbolKind::Unresolved)
		return is_function_kind(program.symbols[symbol].kind) ? symbol : kNoId;
	Signature sig;
	sig.first_parameter = static_cast<uint32_t>(program.parameters.size());
	sig.parameter_count = count;
	sig.result = Type::scalar(TypeKind::Ptr);
	sig.boundary.unwind = ir_model::CUM_NO;
	for (uint32_t k = 0; k < count; ++k)
	{
		Parameter p = Parameter();
		p.name = 0;
		p.type = types[k];
		p.pass = ir_model::PPM_DIRECT;
		p.object_bytes = 0;
		program.parameters.push_back(p);
	}
	program.declare_function(symbol, SymbolKind::FunctionDeclaration, program.add_signature(sig));
	SymbolMetadata& m = program.symbols[symbol].metadata;
	m.binding = ir_model::SBM_STRONG;
	m.object = program.names.intern(object, static_cast<uint32_t>(std::strlen(object)));
	return symbol;
}

// O2+: a copy loop, `store T v, q` of `v = load T p` (each once per pass,
// v read by nothing else) with p and q pointer inductions stepped by T's
// size, tested at its header on p against an invariant bound: it copies
// bound - p0 bytes forward from p0 to q0 (src and dst index ivs).
bool LoopPass::copy_pattern(
	uint32_t loop, const std::vector<Induction>& ivs, const ExitTest& test, uint32_t& src, uint32_t& dst) const
{
	InstId store = kNoId, load = kNoId;
	for (BlockId b : forest_.loops[loop].blocks)
		for (InstId i = body_.blocks[b].first; i != kNoId; i = body_.instructions[i].next)
		{
			const Instruction& inst = body_.instructions[i];
			if (inst.op == Opcode::Store || inst.op == Opcode::Load)
			{
				InstId& slot = inst.op == Opcode::Store ? store : load;
				if (slot != kNoId || (inst.flags & IF_VOLATILE) != 0)
					return false;
				slot = i;
			}
			else if (!is_pure(inst) && inst.op != Opcode::Jump && inst.op != Opcode::Branch)
				return false;
		}
	if (store == kNoId || load == kNoId)
		return false;
	const Instruction& st = body_.instructions[store];
	const Instruction& ld = body_.instructions[load];
	if (!st.ops[0].is(ValueKind::Temp) || st.ops[0].index != ld.result || !(st.type == ld.type) ||
		!(st.type.is_integer() || st.type.is_pointer()) || storage_bytes(st.type) == 0)
		return false;
	const Loop& l = forest_.loops[loop];
	for (const InstId x : { load, store })
		if (c_.block_of[x] == l.header || !cfg_.dominates(c_.block_of[x], l.latches[0]))
			return false;
	// The loaded value feeds the store alone.
	uint32_t reads = 0;
	for (BlockId b : forest_.loops[loop].blocks)
		for (InstId i = body_.blocks[b].first; i != kNoId; i = body_.instructions[i].next)
			for_each_operand(body_, body_.instructions[i], [&](const Value& v) {
				if (v.is(ValueKind::Temp) && v.index == ld.result)
					++reads;
			});
	if (reads != 1)
		return false;
	// The addresses are inductions (through copies).
	auto induction_of = [&](Value address) -> uint32_t {
		for (unsigned step = 0; step < 4 && address.is(ValueKind::Temp); ++step)
		{
			for (uint32_t k = 0; k < ivs.size(); ++k)
				if (ivs[k].value == address.index)
					return k;
			const InstId def = body_.temps[address.index].def;
			if (def == kNoId || body_.instructions[def].op != Opcode::Copy)
				return kNoId;
			address = body_.instructions[def].ops[0];
		}
		return kNoId;
	};
	src = induction_of(ld.ops[0]);
	dst = induction_of(st.ops[1]);
	const int64_t size = static_cast<int64_t>(storage_bytes(st.type));
	return src != kNoId && dst != kNoId && src != dst && ivs[src].pointer && ivs[dst].pointer &&
		ivs[src].step_bytes == size && ivs[dst].step_bytes == size && test.compared.is(ValueKind::Temp) &&
		test.compared.index == ivs[src].value;
}

// Values the loop defines are used outside it only as a or b.
bool LoopPass::used_outside_only(uint32_t loop, const Value& a, const Value& b) const
{
	bool ok = true;
	for (BlockId bl : body_.layout)
	{
		if (in_loop(forest_, loop, bl))
			continue;
		for (InstId i = body_.blocks[bl].first; i != kNoId && ok; i = body_.instructions[i].next)
			for_each_operand(body_, body_.instructions[i], [&](const Value& v) {
				if (v.is(ValueKind::Temp) && !invariant(loop, v) && !(v == a) && !(v == b))
					ok = false;
			});
	}
	return ok;
}

// The preheader branches to a new block calling memmove(q0, p0, n) and
// then the exit, unless q0 lies strictly inside [p0, p0 + n) (as unsigned
// q0 - p0 < n): there the loop runs as before (repeating bytes it already
// wrote). Either way p leaves as the bound and q as q0 + n.
void LoopPass::emit_copy(uint32_t loop, const Induction& src, const Induction& dst, const ExitTest& test)
{
	const Loop& l = forest_.loops[loop];
	const BlockId pre = l.preheader;
	InstId at = previous_in_block(body_, pre, body_.blocks[pre].last);
	auto emit_in = [&](BlockId b, InstId& after, Instruction inst, Type result) {
		const InstId id = append_instruction(body_, inst);
		if (!result.is_void())
			body_.instructions[id].result = append_temp(body_, result, id);
		insert_after(body_, b, after, id);
		after = id;
		return result.is_void() ? Value::none() : Value::temp(body_.instructions[id].result);
	};
	const Type i64 = Type::scalar(TypeKind::I64), ptr = Type::scalar(TypeKind::Ptr), i8 = Type::scalar(TypeKind::I8);
	auto as_i64 = [&](Value v) {
		Instruction copy = make_instruction(Opcode::Copy, i64);
		copy.ops[0] = v;
		return emit_in(pre, at, copy, i64);
	};
	Instruction sub = make_instruction(Opcode::Binary, i64);
	sub.sub = static_cast<uint8_t>(BinaryOp::Sub);
	sub.ops[0] = as_i64(test.bound);
	sub.ops[1] = as_i64(src.init);
	const Value bytes = emit_in(pre, at, sub, i64);
	Instruction end = make_instruction(Opcode::Index, i8);
	end.ops[0] = dst.init;
	end.ops[1] = bytes;
	const Value dst_end = emit_in(pre, at, end, ptr);
	Instruction distance = make_instruction(Opcode::Binary, i64);
	distance.sub = static_cast<uint8_t>(BinaryOp::Sub);
	distance.ops[0] = as_i64(dst.init);
	distance.ops[1] = as_i64(src.init);
	Instruction apart = make_instruction(Opcode::Compare, i64);
	apart.sub = static_cast<uint8_t>(ComparePredicate::Uge);
	apart.ops[0] = emit_in(pre, at, distance, i64);
	apart.ops[1] = bytes;
	const Value whole = emit_in(pre, at, apart, i64);
	// The copy block.
	const BlockId copy = append_block(body_, BlockKind::LoopCopy);
	InstId in_copy = kNoId;
	const Type types[3] = { ptr, ptr, i64 };
	Instruction call = make_instruction(Opcode::Call, ptr);
	call.ops[0] = Value::symbol(builtin_symbol("__cppgm_builtin_memmove", "cppgm_builtin_memmove", types, 3));
	const Value args[3] = { dst.init, src.init, bytes };
	call.extra_first = append_operands(body_, args, 3);
	call.extra_count = 3;
	emit_in(copy, in_copy, call, ptr);
	Instruction jump = make_instruction(Opcode::Jump, Type::scalar(TypeKind::Void));
	jump.ops[0] = Value::block(test.exit);
	emit_in(copy, in_copy, jump, Type::scalar(TypeKind::Void));
	Instruction& branch = body_.instructions[body_.blocks[pre].last];
	branch.op = Opcode::Branch;
	branch.ops[0] = whole;
	branch.ops[1] = Value::block(copy);
	branch.ops[2] = Value::block(l.header);
	std::vector<BlockId> layout;
	for (BlockId b : body_.layout)
	{
		layout.push_back(b);
		if (b == pre)
			layout.push_back(copy);
	}
	body_.layout.swap(layout);
	// After the loop: p is the bound, q is q0 + n (on both ways in).
	const Value src_value = Value::temp(src.value), dst_value = Value::temp(dst.value);
	auto leaving = [&](Value v) { return v == src_value ? test.bound : v == dst_value ? dst_end : v; };
	for (InstId i = body_.blocks[test.exit].first; i != kNoId && body_.instructions[i].op == Opcode::Phi;
		i = body_.instructions[i].next)
	{
		Instruction& phi = body_.instructions[i];
		std::vector<Value> pairs(
			body_.operands.begin() + phi.extra_first, body_.operands.begin() + phi.extra_first + phi.extra_count);
		pairs.push_back(Value::block(copy));
		pairs.push_back(phi_input(body_, phi.type, leaving(phi_incoming(body_, phi, test.exiting))));
		phi.extra_first = append_operands(body_, pairs.data(), static_cast<uint32_t>(pairs.size()));
		phi.extra_count = static_cast<uint32_t>(pairs.size());
	}
	for (BlockId b : body_.layout)
		if (!in_loop(forest_, loop, b) && b != copy)
			for (InstId i = body_.blocks[b].first; i != kNoId; i = body_.instructions[i].next)
			{
				Instruction& inst = body_.instructions[i];
				if (inst.op == Opcode::Phi && b == test.exit)
				{
					// The exiting edge keeps the loop's values.
					for (uint32_t k = 0; k + 1 < inst.extra_count; k += 2)
						if (!(body_.operands[inst.extra_first + k] == Value::block(test.exiting)))
							body_.operands[inst.extra_first + k + 1] = leaving(
								body_.operands[inst.extra_first + k + 1]);
					continue;
				}
				for_each_operand(body_, inst, [&](Value& v) { v = leaving(v); });
			}
}

// stepped by the store's size: `store T v, p` where v is a literal of
// identical bytes, or a byte loop-invariant, or a byte the loop loads from
// an invariant address (rewritten only with the value it holds).
bool LoopPass::fill_pattern(uint32_t loop, const Induction& iv, InstId& store, InstId& load) const
{
	store = load = kNoId;
	for (BlockId b : forest_.loops[loop].blocks)
		for (InstId i = body_.blocks[b].first; i != kNoId; i = body_.instructions[i].next)
		{
			const Instruction& inst = body_.instructions[i];
			if (inst.op == Opcode::Store)
			{
				if (store != kNoId || (inst.flags & IF_VOLATILE) != 0)
					return false;
				store = i;
			}
			else if (inst.op == Opcode::Load)
			{
				if (load != kNoId || (inst.flags & IF_VOLATILE) != 0 || !invariant(loop, inst.ops[0]))
					return false;
				load = i;
			}
			else if (!is_pure(inst) && inst.op != Opcode::Jump && inst.op != Opcode::Branch)
				return false;
		}
	if (store == kNoId || !iv.pointer)
		return false;
	// The store runs once per pass: after the header's test (which also
	// runs when the loop leaves) and on every path to the latch.
	const Loop& l = forest_.loops[loop];
	const BlockId at = c_.block_of[store];
	if (at == l.header || !cfg_.dominates(at, l.latches[0]))
		return false;
	const Instruction st = body_.instructions[store];
	const uint64_t size = storage_bytes(st.type);
	if (!st.type.is_integer() || size == 0 || iv.step_bytes != static_cast<int64_t>(size))
		return false;
	// The address is the induction value (through copies).
	Value address = st.ops[1];
	for (unsigned step = 0; step < 4 && address.is(ValueKind::Temp) && address.index != iv.value; ++step)
	{
		const InstId def = body_.temps[address.index].def;
		if (def == kNoId || body_.instructions[def].op != Opcode::Copy)
			return false;
		address = body_.instructions[def].ops[0];
	}
	if (!(address.is(ValueKind::Temp) && address.index == iv.value))
		return false;
	const Value v = st.ops[0];
	if (v.is(ValueKind::Integer))
	{
		const Literal bits = normalize_integer(st.type, body_.literals[v.index]);
		const uint64_t byte = bits.low & 0xff;
		for (uint64_t k = 1; k < size; ++k)
			if (((bits.low >> (8 * k)) & 0xff) != byte)
				return false;
		return load == kNoId;
	}
	if (size != 1 || !v.is(ValueKind::Temp))
		return false;
	if (load != kNoId)
		return body_.instructions[load].result == v.index && body_.instructions[load].type == st.type;
	return invariant(loop, v);
}

// The preheader calls memset(init, byte, bound - init) for a fill loop.
void LoopPass::emit_fill(uint32_t loop, const Induction& iv, const ExitTest& test, InstId store, InstId load)
{
	const Loop& l = forest_.loops[loop];
	const BlockId pre = l.preheader;
	InstId at = previous_in_block(body_, pre, body_.blocks[pre].last);
	auto emit = [&](Instruction inst, Type result) {
		const InstId id = append_instruction(body_, inst);
		if (!result.is_void())
			body_.instructions[id].result = append_temp(body_, result, id);
		insert_after(body_, pre, at, id);
		at = id;
		return Value::temp(body_.instructions[id].result);
	};
	const Type i64 = Type::scalar(TypeKind::I64), ptr = Type::scalar(TypeKind::Ptr);
	// The byte count: the bound's own `index i8 init, n`, or bound - init.
	Value bytes = Value::none();
	if (test.bound.is(ValueKind::Temp) && body_.temps[test.bound.index].def != kNoId)
	{
		const Instruction& def = body_.instructions[body_.temps[test.bound.index].def];
		if (def.op == Opcode::Index && def.ops[0] == iv.init && storage_bytes(def.type) == 1)
			bytes = def.ops[1];
	}
	if (bytes.is(ValueKind::None))
	{
		Instruction end = make_instruction(Opcode::Copy, i64), begin = make_instruction(Opcode::Copy, i64);
		end.ops[0] = test.bound;
		begin.ops[0] = iv.init;
		Instruction sub = make_instruction(Opcode::Binary, i64);
		sub.sub = static_cast<uint8_t>(BinaryOp::Sub);
		sub.ops[0] = emit(end, i64);
		sub.ops[1] = emit(begin, i64);
		bytes = emit(sub, i64);
	}
	const Instruction st = body_.instructions[store];
	Value byte;
	if (st.ops[0].is(ValueKind::Integer))
		byte = integer_value(body_, integer_literal(static_cast<int64_t>(body_.literals[st.ops[0].index].low & 0xff)));
	else
	{
		Value v = st.ops[0];
		if (load != kNoId)
		{
			Instruction hoisted = make_instruction(Opcode::Load, st.type);
			hoisted.ops[0] = body_.instructions[load].ops[0];
			v = emit(hoisted, st.type);
		}
		Instruction widen = make_instruction(Opcode::Convert, Type::scalar(TypeKind::I32));
		widen.sub = static_cast<uint8_t>(ConvertOp::ZExt);
		set_convert_source_type(widen, st.type);
		widen.ops[0] = v;
		byte = emit(widen, Type::scalar(TypeKind::I32));
	}
	const SymbolId memset = fill_symbol();
	Instruction call = make_instruction(Opcode::Call, ptr);
	call.ops[0] = Value::symbol(memset);
	const Value args[3] = { iv.init, byte, bytes };
	call.extra_first = append_operands(body_, args, 3);
	call.extra_count = 3;
	call.location = st.location;
	emit(call, ptr);
}

// O2+: a fill loop memset cannot write -- `store T v, p` of an invariant
// 2-, 4- or 8-byte value through a pointer induction stepped by T's size,
// tested at the header on the induction against an invariant end, the
// induction the header's only phi -- writes the value replicated to 64
// bits, 16 bytes per pass, over the range's whole 16-byte chunks in a loop
// placed before it; the original loop starts after them (at most 15 bytes
// remain). The pass per chunk is two stores, one step and one test.
bool LoopPass::wide_fill_pattern(uint32_t loop, const ExitTest& test, const Induction& iv, InstId& store) const
{
	const Loop& l = forest_.loops[loop];
	if (c_.level < 2 || l.preheader == kNoId || test.exiting != l.header || !iv.pointer ||
		!(test.compared.is(ValueKind::Temp) && test.compared.index == iv.value) ||
		(iv.step_bytes != 2 && iv.step_bytes != 4 && iv.step_bytes != 8))
		return false;
	uint32_t phis = 0;
	for (InstId i = body_.blocks[l.header].first; i != kNoId && body_.instructions[i].op == Opcode::Phi;
		i = body_.instructions[i].next)
		++phis;
	if (phis != 1)
		return false;
	// Not the tail of a widened loop: its start is `index init, bytes & -16`.
	if (iv.init.is(ValueKind::Temp) && body_.temps[iv.init.index].def != kNoId)
	{
		const Instruction& start = body_.instructions[body_.temps[iv.init.index].def];
		if (start.op == Opcode::Index && start.ops[1].is(ValueKind::Temp) &&
			body_.temps[start.ops[1].index].def != kNoId)
		{
			const Instruction& chunk = body_.instructions[body_.temps[start.ops[1].index].def];
			if (chunk.op == Opcode::Binary && static_cast<BinaryOp>(chunk.sub) == BinaryOp::And &&
				chunk.ops[1].is(ValueKind::Integer) &&
				body_.literals[chunk.ops[1].index].low == static_cast<uint64_t>(-16))
				return false;
		}
	}
	store = kNoId;
	for (BlockId b : l.blocks)
		for (InstId i = body_.blocks[b].first; i != kNoId; i = body_.instructions[i].next)
		{
			const Instruction& inst = body_.instructions[i];
			if (inst.op == Opcode::Store)
			{
				if (store != kNoId || (inst.flags & IF_VOLATILE) != 0)
					return false;
				store = i;
			}
			else if (!is_pure(inst) && inst.op != Opcode::Jump && inst.op != Opcode::Branch)
				return false;
		}
	if (store == kNoId)
		return false;
	// The store runs once per pass, after the header's test.
	const BlockId at = c_.block_of[store];
	if (at == l.header || !cfg_.dominates(at, l.latches[0]))
		return false;
	const Instruction& st = body_.instructions[store];
	if (!(st.type.is_integer() || st.type.is_pointer()) || st.type.kind() == TypeKind::I128 ||
		storage_bytes(st.type) != static_cast<uint64_t>(iv.step_bytes))
		return false;
	Value address = st.ops[1];
	for (unsigned step = 0; step < 4 && address.is(ValueKind::Temp) && address.index != iv.value; ++step)
	{
		const InstId def = body_.temps[address.index].def;
		if (def == kNoId || body_.instructions[def].op != Opcode::Copy)
			return false;
		address = body_.instructions[def].ops[0];
	}
	if (!(address.is(ValueKind::Temp) && address.index == iv.value))
		return false;
	const Value v = st.ops[0];
	return v.is(ValueKind::Integer) || (v.is(ValueKind::Temp) && invariant(loop, v));
}

bool LoopPass::widen_fill(uint32_t loop, const ExitTest& test, const std::vector<Induction>& ivs)
{
	const Loop& l = forest_.loops[loop];
	const Induction* iv = nullptr;
	InstId store = kNoId;
	for (const Induction& candidate : ivs)
		if (wide_fill_pattern(loop, test, candidate, store))
		{
			iv = &candidate;
			break;
		}
	if (iv == nullptr)
		return false;
	const Instruction st = body_.instructions[store];
	const uint64_t size = static_cast<uint64_t>(iv->step_bytes);
	const Type i64 = Type::scalar(TypeKind::I64), ptr = Type::scalar(TypeKind::Ptr);
	const BlockId pre = l.preheader, header = l.header;
	const BlockId wide = append_block(body_, BlockKind::WideLoop), tail = append_block(body_, BlockKind::WideTail);
	auto emit = [&](BlockId b, InstId& after, Instruction inst, Type result) {
		inst.location = st.location;
		const InstId id = append_instruction(body_, inst);
		if (!result.is_void())
			body_.instructions[id].result = append_temp(body_, result, id);
		insert_after(body_, b, after, id);
		after = id;
		return Value::temp(body_.instructions[id].result);
	};
	auto binary = [&](BinaryOp op, Value a, Value b) {
		Instruction inst = make_instruction(Opcode::Binary, i64);
		inst.sub = static_cast<uint8_t>(op);
		inst.ops[0] = a;
		inst.ops[1] = b;
		return inst;
	};
	auto index = [&](Value base, Value bytes) {
		Instruction inst = make_instruction(Opcode::Index, Type::scalar(TypeKind::I8));
		inst.ops[0] = base;
		inst.ops[1] = bytes;
		return inst;
	};
	auto differs = [&](Value a, Value b) {
		Instruction inst = make_instruction(Opcode::Compare, ptr);
		inst.sub = static_cast<uint8_t>(ComparePredicate::Ne);
		inst.ops[0] = a;
		inst.ops[1] = b;
		return inst;
	};
	// The preheader: the chunks' end, the replicated value, the entry test.
	InstId at = previous_in_block(body_, pre, body_.blocks[pre].last);
	Instruction end = make_instruction(Opcode::Copy, i64), begin = make_instruction(Opcode::Copy, i64);
	end.ops[0] = test.bound;
	begin.ops[0] = iv->init;
	// (Sequenced: an argument list's evaluation order is the host's.)
	const Value begin_bytes = emit(pre, at, begin, i64);
	const Value end_bytes = emit(pre, at, end, i64);
	const Value bytes = emit(pre, at, binary(BinaryOp::Sub, end_bytes, begin_bytes), i64);
	const Value chunk_bytes = emit(
		pre, at, binary(BinaryOp::And, bytes, integer_value(body_, integer_literal(-16))), i64);
	const Value end16 = emit(pre, at, index(iv->init, chunk_bytes), ptr);
	const uint64_t replicator = size == 8 ? 1 : size == 4 ? 0x0000000100000001ull : 0x0001000100010001ull;
	Value v64;
	if (st.ops[0].is(ValueKind::Integer))
	{
		const Literal bits = normalize_integer(st.type, body_.literals[st.ops[0].index]);
		const uint64_t mask = size == 8 ? ~0ull : (1ull << (8 * size)) - 1;
		v64 = integer_value(body_, integer_literal(static_cast<int64_t>((bits.low & mask) * replicator)));
	}
	else if (size == 8)
	{
		Instruction copy = make_instruction(Opcode::Copy, i64);
		copy.ops[0] = st.ops[0];
		v64 = st.type.is_pointer() ? emit(pre, at, copy, i64) : st.ops[0];
	}
	else
	{
		Instruction widen = make_instruction(Opcode::Convert, i64);
		widen.sub = static_cast<uint8_t>(ConvertOp::ZExt);
		set_convert_source_type(widen, st.type);
		widen.ops[0] = st.ops[0];
		const Value wide_value = emit(pre, at, widen, i64);
		const Value replicate = integer_value(body_, integer_literal(static_cast<int64_t>(replicator)));
		v64 = emit(pre, at, binary(BinaryOp::Mul, wide_value, replicate), i64);
	}
	const Value enter = emit(pre, at, differs(iv->init, end16), i64);
	Instruction& entry = body_.instructions[body_.blocks[pre].last];
	entry.op = Opcode::Branch;
	entry.type = Type::scalar(TypeKind::Void);
	entry.ops[0] = enter;
	entry.ops[1] = Value::block(wide);
	entry.ops[2] = Value::block(tail);
	// The wide loop: q = phi(init, q + 16); two stores; the test.
	InstId w = kNoId;
	Instruction phi = make_instruction(Opcode::Phi, ptr);
	const InstId phi_id = append_instruction(body_, phi);
	const Value q = Value::temp(body_.instructions[phi_id].result = append_temp(body_, ptr, phi_id));
	insert_after(body_, wide, w, phi_id);
	w = phi_id;
	Instruction first = make_instruction(Opcode::Store, i64), second = make_instruction(Opcode::Store, i64);
	first.ops[0] = v64;
	first.ops[1] = q;
	emit(wide, w, first, Type::scalar(TypeKind::Void));
	second.ops[0] = v64;
	second.ops[1] = emit(wide, w, index(q, integer_value(body_, integer_literal(8))), ptr);
	emit(wide, w, second, Type::scalar(TypeKind::Void));
	const Value q2 = emit(wide, w, index(q, integer_value(body_, integer_literal(16))), ptr);
	const Value again = emit(wide, w, differs(q2, end16), i64);
	Instruction loop_branch = make_instruction(Opcode::Branch, Type::scalar(TypeKind::Void));
	loop_branch.ops[0] = again;
	loop_branch.ops[1] = Value::block(wide);
	loop_branch.ops[2] = Value::block(tail);
	emit(wide, w, loop_branch, Type::scalar(TypeKind::Void));
	const Value pairs[4] = { Value::block(pre), iv->init, Value::block(wide), q2 };
	body_.instructions[phi_id].extra_first = append_operands(body_, pairs, 4);
	body_.instructions[phi_id].extra_count = 4;
	// The tail's preheader, and the original loop starting after the chunks.
	InstId t = kNoId;
	Instruction jump = make_instruction(Opcode::Jump, Type::scalar(TypeKind::Void));
	jump.ops[0] = Value::block(header);
	emit(tail, t, jump, Type::scalar(TypeKind::Void));
	rename_phi_incoming(body_, header, pre, tail);
	Instruction& start = body_.instructions[iv->phi];
	for (uint32_t k = 0; k < start.extra_count; k += 2)
		if (body_.operands[start.extra_first + k] == Value::block(tail))
			body_.operands[start.extra_first + k + 1] = end16;
	std::vector<BlockId> layout;
	for (BlockId b : body_.layout)
	{
		layout.push_back(b);
		if (b == pre)
		{
			layout.push_back(wide);
			layout.push_back(tail);
		}
	}
	body_.layout.swap(layout);
	map_instruction_blocks(body_, c_.block_of);
	c_.invalidate_block_of();
	++c_.stats.loops_removed;
	return true;
}

// O2+, after all inlining (finish_function): widen_fill's chunk loop of a
// value only known at run time whose bytes all repeat (0, ~0u: most of an
// out-of-line `vector::assign`'s) fills at least kMemsetChunkBytes of
// chunks with one memset instead. Its preheader's entry test goes to a
// length test, then a test of the value's bytes, then the memset (which
// continues at the tail like the loop); either test failing runs the loop.
// (Before inlining the larger fills would push `vector`'s fill constructors
// past their callers' caps.)
const uint64_t kMemsetChunkBytes = 128;

bool LoopPass::version_fills()
{
	bool changed = false;
	// (Each versioning inserts three blocks before its loop: the bound is
	// the live layout, so the loops behind it are still reached.)
	for (size_t k = 0; k < body_.layout.size(); ++k)
	{
		// The chunk loop, exactly as widen_fill emits it: q = phi(init, q2);
		// store v64, q; store v64, q + 8; q2 = q + 16; branch q2 != end16.
		const BlockId wide = body_.layout[k];
		InstId i = body_.blocks[wide].first;
		const Instruction* seq[7];
		unsigned n = 0;
		for (; i != kNoId && n < 7; i = body_.instructions[i].next)
			seq[n++] = &body_.instructions[i];
		if (n != 7 || i != kNoId || seq[0]->op != Opcode::Phi || seq[0]->extra_count != 4 ||
			seq[1]->op != Opcode::Store || seq[2]->op != Opcode::Index || seq[3]->op != Opcode::Store ||
			seq[4]->op != Opcode::Index || seq[5]->op != Opcode::Compare || seq[6]->op != Opcode::Branch)
			continue;
		const Instruction& phi = *seq[0];
		const Value q = Value::temp(phi.result);
		const Value v64 = seq[1]->ops[0];
		auto literal = [&](Value v, uint64_t want) {
			return v.is(ValueKind::Integer) && body_.literals[v.index].low == want;
		};
		if (!v64.is(ValueKind::Temp) || seq[1]->type.kind() != TypeKind::I64 || !(seq[1]->ops[1] == q) ||
			!(seq[2]->ops[0] == q) || !literal(seq[2]->ops[1], 8) || !(seq[3]->ops[0] == v64) ||
			!(seq[3]->ops[1] == Value::temp(seq[2]->result)) || !(seq[4]->ops[0] == q) ||
			!literal(seq[4]->ops[1], 16) || static_cast<ComparePredicate>(seq[5]->sub) != ComparePredicate::Ne ||
			!(seq[6]->ops[0] == Value::temp(seq[5]->result)) || !(seq[6]->ops[1] == Value::block(wide)))
			continue;
		// (Either operand order: the compares may be canonicalized.)
		const unsigned bound_at = seq[5]->ops[0] == Value::temp(seq[4]->result) ? 1 : 0;
		const Value end16 = seq[5]->ops[bound_at];
		if (!(seq[5]->ops[1 - bound_at] == Value::temp(seq[4]->result)))
			continue;
		const Value* pairs = &body_.operands[phi.extra_first];
		const unsigned from = pairs[0] == Value::block(wide) ? 2 : 0;
		if (!(pairs[2 - from] == Value::block(wide)) || !(pairs[3 - from] == Value::temp(seq[4]->result)) ||
			!pairs[from].is(ValueKind::Block))
			continue;
		const BlockId pre = pairs[from].index, tail = seq[6]->ops[2].index;
		const Value init = pairs[from + 1];
		// The preheader's entry test, init != end16 with end16 = init + (bytes & -16).
		const Instruction& entry = body_.instructions[body_.blocks[pre].last];
		if (!end16.is(ValueKind::Temp) || body_.temps[end16.index].def == kNoId || entry.op != Opcode::Branch ||
			!(entry.ops[1] == Value::block(wide)) || !(entry.ops[2] == Value::block(tail)) ||
			!entry.ops[0].is(ValueKind::Temp) || body_.temps[entry.ops[0].index].def == kNoId)
			continue;
		const Instruction& enter = body_.instructions[body_.temps[entry.ops[0].index].def];
		const Instruction& start = body_.instructions[body_.temps[end16.index].def];
		if (enter.op != Opcode::Compare || static_cast<ComparePredicate>(enter.sub) != ComparePredicate::Ne ||
			!((enter.ops[0] == init && enter.ops[1] == end16) || (enter.ops[0] == end16 && enter.ops[1] == init)) ||
			start.op != Opcode::Index || !(start.ops[0] == init) || !start.ops[1].is(ValueKind::Temp) ||
			body_.temps[start.ops[1].index].def == kNoId)
			continue;
		const Instruction& chunk = body_.instructions[body_.temps[start.ops[1].index].def];
		if (chunk.op != Opcode::Binary || static_cast<BinaryOp>(chunk.sub) != BinaryOp::And ||
			!literal(chunk.ops[1], static_cast<uint64_t>(-16)))
			continue;
		// The tail (often merged into the original loop's header): the memset
		// block joins it carrying what the preheader and the loop both carry.
		std::vector<std::pair<InstId, Value>> joined;
		bool agree = true;
		for (InstId p = body_.blocks[tail].first; p != kNoId && body_.instructions[p].op == Opcode::Phi && agree;
			p = body_.instructions[p].next)
		{
			const Instruction& join = body_.instructions[p];
			Value from_pre = Value::none(), from_wide = Value::none();
			for (uint32_t e = 0; e < join.extra_count; e += 2)
			{
				const Value label = body_.operands[join.extra_first + e];
				if (label == Value::block(pre))
					from_pre = body_.operands[join.extra_first + e + 1];
				else if (label == Value::block(wide))
					from_wide = body_.operands[join.extra_first + e + 1];
			}
			agree = !from_pre.is(ValueKind::None) && from_pre == from_wide;
			joined.push_back(std::make_pair(p, from_pre));
		}
		if (!agree)
			continue;
		const SymbolId memset = fill_symbol();
		if (memset == kNoId)
			return changed;
		// (Captured: appending instructions moves them.)
		const Value chunk_bytes = start.ops[1], q2 = Value::temp(seq[4]->result);
		const Type i64 = Type::scalar(TypeKind::I64), ptr = Type::scalar(TypeKind::Ptr);
		const Type none = Type::scalar(TypeKind::Void);
		const BlockId sized = append_block(body_, BlockKind::FillSized),
					  same = append_block(body_, BlockKind::FillSame), fill = append_block(body_, BlockKind::FillFast);
		auto emit = [&](BlockId b, InstId& after, Instruction inst, Type result) {
			inst.location = body_.instructions[body_.blocks[wide].first].location;
			const InstId id = append_instruction(body_, inst);
			if (!result.is_void())
				body_.instructions[id].result = append_temp(body_, result, id);
			insert_after(body_, b, after, id);
			after = id;
			return result.is_void() ? Value::none() : Value::temp(body_.instructions[id].result);
		};
		auto binary = [&](BinaryOp op, Value a, int64_t b) {
			Instruction inst = make_instruction(Opcode::Binary, i64);
			inst.sub = static_cast<uint8_t>(op);
			inst.ops[0] = a;
			inst.ops[1] = integer_value(body_, integer_literal(b));
			return inst;
		};
		auto compare = [&](ComparePredicate p, Value a, Value b) {
			Instruction inst = make_instruction(Opcode::Compare, i64);
			inst.sub = static_cast<uint8_t>(p);
			inst.ops[0] = a;
			inst.ops[1] = b;
			return inst;
		};
		auto branch = [&](BlockId b, InstId& after, Value cond, BlockId taken, BlockId other) {
			Instruction inst = make_instruction(Opcode::Branch, none);
			inst.ops[0] = cond;
			inst.ops[1] = Value::block(taken);
			inst.ops[2] = Value::block(other);
			emit(b, after, inst, none);
		};
		InstId s = kNoId, u = kNoId, f = kNoId;
		const Value longer = emit(sized, s,
			compare(ComparePredicate::Uge, chunk_bytes,
				integer_value(body_, integer_literal(static_cast<int64_t>(kMemsetChunkBytes)))),
			i64);
		branch(sized, s, longer, same, wide);
		const Value low = emit(same, u, binary(BinaryOp::And, v64, 0xff), i64);
		const Value spread = emit(same, u, binary(BinaryOp::Mul, low, 0x0101010101010101ll), i64);
		branch(same, u, emit(same, u, compare(ComparePredicate::Eq, spread, v64), i64), fill, wide);
		Instruction narrow = make_instruction(Opcode::Convert, Type::scalar(TypeKind::I32));
		narrow.sub = static_cast<uint8_t>(ConvertOp::Trunc);
		set_convert_source_type(narrow, i64);
		narrow.ops[0] = low;
		const Value byte = emit(fill, f, narrow, Type::scalar(TypeKind::I32));
		Instruction call = make_instruction(Opcode::Call, ptr);
		call.ops[0] = Value::symbol(memset);
		const Value args[3] = { init, byte, chunk_bytes };
		call.extra_first = append_operands(body_, args, 3);
		call.extra_count = 3;
		emit(fill, f, call, ptr);
		Instruction done = make_instruction(Opcode::Jump, none);
		done.ops[0] = Value::block(tail);
		emit(fill, f, done, none);
		// The loop entered from either test.
		body_.instructions[body_.blocks[pre].last].ops[1] = Value::block(sized);
		Instruction& loop_phi = body_.instructions[body_.blocks[wide].first];
		const Value entered[6] = { Value::block(sized), init, Value::block(same), init, Value::block(wide), q2 };
		loop_phi.extra_first = append_operands(body_, entered, 6);
		loop_phi.extra_count = 6;
		for (const std::pair<InstId, Value>& j : joined)
		{
			Instruction& join = body_.instructions[j.first];
			std::vector<Value> pairs(body_.operands.begin() + join.extra_first,
				body_.operands.begin() + join.extra_first + join.extra_count);
			pairs.insert(pairs.end(), { Value::block(fill), j.second });
			join.extra_first = append_operands(body_, pairs.data(), static_cast<uint32_t>(pairs.size()));
			join.extra_count = static_cast<uint32_t>(pairs.size());
		}
		std::vector<BlockId> layout;
		for (BlockId b : body_.layout)
		{
			if (b == wide)
				layout.insert(layout.end(), { sized, same, fill });
			layout.push_back(b);
		}
		body_.layout.swap(layout);
		changed = true;
	}
	if (changed)
		map_instruction_blocks(body_, c_.block_of);
	c_.invalidate_block_of();
	return changed;
}

bool LoopPass::try_loop(uint32_t loop)
{
	const Loop& l = forest_.loops[loop];
	if (l.latches.size() != 1)
		return false;
	ExitTest test;
	std::vector<Induction> ivs;
	if (!exit_test(loop, test, ivs))
		return false;
	if (!private_values(loop, test, ivs))
	{
		// An effect-free loop whose header inductions are read after it:
		// their exit values from the trip count.
		if (effect_free(loop))
		{
			std::vector<Induction> read = ivs;
			add_narrow_accumulators(loop, read);
			if (final_values(loop, test, read))
			{
				if (!bypass(loop, test))
					return false;
				++c_.stats.loops_removed;
				return true;
			}
		}
		// A copy loop's destination may be read after it.
		uint32_t src, dst;
		if (c_.level >= 2 && l.preheader != kNoId && test.exiting == l.header &&
			builtin_available("__cppgm_builtin_memmove", "cppgm_builtin_memmove") &&
			copy_pattern(loop, ivs, test, src, dst) &&
			used_outside_only(loop, Value::temp(ivs[src].value), Value::temp(ivs[dst].value)))
		{
			emit_copy(loop, ivs[src], ivs[dst], test);
			++c_.stats.loops_removed;
			return true;
		}
		return widen_fill(loop, test, ivs);
	}
	if (effect_free(loop))
	{
		if (!bypass(loop, test))
			return false;
		++c_.stats.loops_removed;
		return true;
	}
	// Stores over a counted range go before the loop (a loop entered from
	// a branching block gets a preheader first; the next round rewrites
	// it).
	if (c_.level >= 2 && indexed_stores(loop, test, ivs, false))
	{
		if (l.preheader == kNoId)
			return make_preheader(loop);
		indexed_stores(loop, test, ivs, true);
		if (!bypass(loop, test))
			return false;
		++c_.stats.loops_removed;
		return true;
	}
	// A fill loop is tested at its header on the induction itself (so it
	// stores at init, init + size, ... up to the bound), and needs the
	// preheader for its call.
	if (l.preheader == kNoId || test.exiting != l.header)
		return false;
	if (fill_available())
		for (const Induction& iv : ivs)
		{
			InstId store, load;
			if (test.compared.index != iv.value || iv.step_bytes < 0 || !fill_pattern(loop, iv, store, load))
				continue;
			emit_fill(loop, iv, test, store, load);
			if (!bypass(loop, test))
				return false;
			++c_.stats.loops_removed;
			return true;
		}
	uint32_t src, dst;
	if (c_.level >= 2 && builtin_available("__cppgm_builtin_memmove", "cppgm_builtin_memmove") &&
		copy_pattern(loop, ivs, test, src, dst))
	{
		emit_copy(loop, ivs[src], ivs[dst], test);
		++c_.stats.loops_removed;
		return true;
	}
	return widen_fill(loop, test, ivs);
}

// O2+: a counted loop tested at its header on an integer induction from a
// literal stepping by one, at most kIndexedTrips passes, whose effects are
// stores, once per pass, to `index T base, iv` (base invariant, T an
// integer of the store's width) of a literal or of `load T index T src,
// iv` read by nothing else, src a slot or global no store of the loop
// writes: the preheader writes each store's whole range instead -- a
// zeroinit for zero, eight-byte pieces of a literal's repeated bytes, a
// copyobj from the source's range -- and the caller bypasses the loop.
// At most kIndexedBytes per store.
const int64_t kIndexedTrips = 64;
const uint64_t kIndexedBytes = 256;
const uint32_t kIndexedStores = 4;

bool LoopPass::indexed_stores(uint32_t loop, const ExitTest& test, const std::vector<Induction>& ivs, bool rewrite)
{
	const Loop& l = forest_.loops[loop];
	if (test.trips <= 0 || test.trips > kIndexedTrips || test.exiting != l.header || (rewrite && l.preheader == kNoId))
		return false;
	const Induction* iv = nullptr;
	for (const Induction& candidate : ivs)
		if (test.compared.is(ValueKind::Temp) && test.compared.index == candidate.value && !candidate.pointer &&
			candidate.step_bytes == 1 && candidate.init.is(ValueKind::Integer))
			iv = &candidate;
	if (iv == nullptr)
		return false;
	const int64_t first = static_cast<int64_t>(body_.literals[iv->init.index].low);
	if (first < 0 || first > kIndexedTrips)
		return false;
	count_uses(body_, c_.uses);
	c_.invalidate_uses();
	// The induction itself, or a copy or extension of it.
	auto is_iv = [&](Value v) {
		for (unsigned step = 0; step < 3 && v.is(ValueKind::Temp); ++step)
		{
			if (v.index == iv->value)
				return true;
			const InstId def = body_.temps[v.index].def;
			if (def == kNoId)
				return false;
			const Instruction& d = body_.instructions[def];
			const bool widen = d.op == Opcode::Convert &&
				(static_cast<ConvertOp>(d.sub) == ConvertOp::ZExt || static_cast<ConvertOp>(d.sub) == ConvertOp::SExt);
			if (d.op != Opcode::Copy && !widen)
				return false;
			v = d.ops[0];
		}
		return false;
	};
	// The slot or global an address names directly, else none.
	auto object_of = [&](Value v) -> Value {
		if (v.is(ValueKind::Temp) && body_.temps[v.index].def != kNoId)
		{
			const Instruction& d = body_.instructions[body_.temps[v.index].def];
			if (d.op == Opcode::Addr)
				return d.ops[0];
		}
		return v.is(ValueKind::Slot) || v.is(ValueKind::Symbol) ? v : Value::none();
	};
	// The base of `index T base, iv` of element width `bytes`: invariant,
	// or an object's address taken in the loop.
	auto element_base = [&](Value address, uint64_t bytes, Value& base) {
		if (!address.is(ValueKind::Temp) || body_.temps[address.index].def == kNoId)
			return false;
		const Instruction& d = body_.instructions[body_.temps[address.index].def];
		if (d.op != Opcode::Index || storage_bytes(d.type) != bytes || !is_iv(d.ops[1]) ||
			(!invariant(loop, d.ops[0]) && object_of(d.ops[0]).is(ValueKind::None)))
			return false;
		base = d.ops[0];
		return true;
	};
	struct Fill
	{
		InstId store;
		Value base;
		Value source; // a copy's source base, else none
		uint64_t bytes;
	};
	std::vector<Fill> fills;
	std::vector<InstId> loads;
	for (BlockId b : l.blocks)
		for (InstId i = body_.blocks[b].first; i != kNoId; i = body_.instructions[i].next)
		{
			const Instruction& inst = body_.instructions[i];
			if (inst.op == Opcode::Store)
			{
				const uint64_t bytes = storage_bytes(inst.type);
				Fill f = { i, Value::none(), Value::none(), bytes };
				if ((inst.flags & IF_VOLATILE) != 0 || !inst.type.is_integer() || bytes == 0 || bytes > 8 ||
					b == l.header || !cfg_.dominates(b, l.latches[0]) || !element_base(inst.ops[1], bytes, f.base))
					return false;
				if (!inst.ops[0].is(ValueKind::Integer))
				{
					// A copy: the value is a load of the source's element.
					const Value v = inst.ops[0];
					if (!v.is(ValueKind::Temp) || body_.temps[v.index].def == kNoId)
						return false;
					const Instruction& load = body_.instructions[body_.temps[v.index].def];
					if (load.op != Opcode::Load || (load.flags & IF_VOLATILE) != 0 || load.type != inst.type ||
						c_.uses[v.index] != 1 || !element_base(load.ops[0], bytes, f.source) ||
						object_of(f.source).is(ValueKind::None))
						return false;
				}
				fills.push_back(f);
			}
			else if (inst.op == Opcode::Load)
				loads.push_back(i);
			else if (!is_pure(inst) && inst.op != Opcode::Jump && inst.op != Opcode::Branch)
				return false;
		}
	if (fills.empty())
		return false;
	// Every load feeds a copy; no store writes a copy's source object, and
	// each destination is a distinct object or pointer.
	for (InstId i : loads)
	{
		bool feeds = false;
		for (const Fill& f : fills)
			feeds = feeds ||
				(body_.instructions[f.store].ops[0].is(ValueKind::Temp) &&
					body_.temps[body_.instructions[f.store].ops[0].index].def == i);
		if (!feeds)
			return false;
	}
	for (const Fill& f : fills)
	{
		if (static_cast<uint64_t>(test.trips) * f.bytes > kIndexedBytes)
			return false;
		for (const Fill& g : fills)
		{
			if (&f != &g && f.base == g.base)
				return false;
			if (!g.source.is(ValueKind::None))
			{
				const Value dst = object_of(f.base), src = object_of(g.source);
				if (dst.is(ValueKind::None) || dst == src)
					return false;
			}
		}
	}
	if (!rewrite)
		return true;
	const BlockId pre = l.preheader;
	InstId at = previous_in_block(body_, pre, body_.blocks[pre].last);
	auto emit = [&](Instruction inst, Type result) {
		inst.location = body_.instructions[fills[0].store].location;
		const InstId id = append_instruction(body_, inst);
		if (!result.is_void())
			body_.instructions[id].result = append_temp(body_, result, id);
		insert_after(body_, pre, at, id);
		at = id;
		return result.is_void() ? Value::none() : Value::temp(body_.instructions[id].result);
	};
	const Type ptr = Type::scalar(TypeKind::Ptr), i8 = Type::scalar(TypeKind::I8);
	// `index T base, first` for an element of `bytes` (an address the loop
	// takes taken again here).
	auto start = [&](Value base, uint64_t bytes) {
		if (!invariant(loop, base))
		{
			Instruction addr = make_instruction(Opcode::Addr, ptr);
			addr.ops[0] = object_of(base);
			base = emit(addr, ptr);
		}
		if (first == 0)
			return base;
		Instruction index = make_instruction(Opcode::Index, i8);
		index.ops[0] = base;
		index.ops[1] = integer_value(body_, integer_literal(first * static_cast<int64_t>(bytes)));
		return emit(index, ptr);
	};
	for (const Fill& f : fills)
	{
		const Instruction st = body_.instructions[f.store];
		const uint64_t total = static_cast<uint64_t>(test.trips) * f.bytes;
		const Value dst = start(f.base, f.bytes);
		if (!f.source.is(ValueKind::None))
		{
			Instruction copy = make_instruction(Opcode::CopyObject, Type::scalar(TypeKind::Void));
			copy.ops[0] = start(f.source, f.bytes);
			copy.ops[1] = dst;
			copy.immediate = static_cast<int64_t>(total);
			emit(copy, Type::scalar(TypeKind::Void));
			continue;
		}
		const Literal bits = normalize_integer(st.type, body_.literals[st.ops[0].index]);
		if (bits.low == 0)
		{
			Instruction zero = make_instruction(Opcode::ZeroInit, Type::scalar(TypeKind::Void));
			zero.ops[0] = dst;
			zero.immediate = static_cast<int64_t>(total);
			emit(zero, Type::scalar(TypeKind::Void));
			continue;
		}
		// The element's bytes repeated through eight bytes, then the
		// remainder in elements.
		uint64_t pattern = 0;
		const uint64_t mask = f.bytes == 8 ? ~static_cast<uint64_t>(0)
										   : (static_cast<uint64_t>(1) << (8 * f.bytes)) - 1;
		for (uint64_t k = 0; k < 8; k += f.bytes)
			pattern |= (bits.low & mask) << (8 * k);
		for (uint64_t offset = 0; offset < total; offset += offset + 8 <= total ? 8 : f.bytes)
		{
			const bool wide = offset + 8 <= total;
			Value address = dst;
			if (offset != 0)
			{
				Instruction index = make_instruction(Opcode::Index, i8);
				index.ops[0] = dst;
				index.ops[1] = integer_value(body_, integer_literal(static_cast<int64_t>(offset)));
				address = emit(index, ptr);
			}
			Instruction store = make_instruction(Opcode::Store, wide ? Type::scalar(TypeKind::I64) : st.type);
			store.ops[0] = wide ? integer_value(body_, Literal{ pattern, 0 }) : st.ops[0];
			store.ops[1] = address;
			emit(store, Type::scalar(TypeKind::Void));
		}
	}
	return true;
}

// A jump-only block on the loop's one entry edge (false when several
// blocks outside enter it).
bool LoopPass::make_preheader(uint32_t loop)
{
	const Loop& l = forest_.loops[loop];
	BlockId entry = kNoId;
	for (uint32_t k = cfg_.pred_first[l.header]; k < cfg_.pred_first[l.header + 1]; ++k)
	{
		const BlockId p = cfg_.preds[k];
		if (in_loop(forest_, loop, p))
			continue;
		if (entry != kNoId && entry != p)
			return false;
		entry = p;
	}
	if (entry == kNoId || (cfg_.flags[entry] & BF_HANDLER) != 0)
		return false;
	const BlockId pre = append_block(body_, BlockKind::LoopPreheader);
	Instruction jump = make_instruction(Opcode::Jump, Type::scalar(TypeKind::Void));
	jump.ops[0] = Value::block(l.header);
	insert_after(body_, pre, kNoId, append_instruction(body_, jump));
	std::vector<BlockId> layout;
	for (BlockId b : body_.layout)
	{
		if (b == l.header)
			layout.push_back(pre);
		layout.push_back(b);
	}
	body_.layout.swap(layout);
	for_each_successor(body_, body_.instructions[body_.blocks[entry].last], [&](BlockId& t) {
		if (t == l.header)
			t = pre;
	});
	rename_phi_incoming(body_, l.header, entry, pre);
	return true;
}

bool LoopPass::builtin_available(const char* name, const char* object) const
{
	const Program& program = c_.program;
	const NameId id = program.names.find(name, static_cast<uint32_t>(std::strlen(name)));
	const NameId runtime = program.names.find(object, static_cast<uint32_t>(std::strlen(object)));
	const SymbolId symbol = id != 0 ? program.find_symbol(id) : kNoId;
	if (symbol != kNoId && program.symbols[symbol].kind != SymbolKind::Unresolved &&
		(!is_function_kind(program.symbols[symbol].kind) || runtime == 0 ||
			program.symbols[symbol].metadata.object != runtime))
		return false;
	return true;
}

bool LoopPass::fill_available() const
{
	return builtin_available("__cppgm_builtin_memset", "cppgm_builtin_memset");
}

// O3 full unrolling, within the budgets below: a counted loop tested at its
// header that leaves after at most kUnrollTrips passes through a
// straight-line body (a chain of jump-ended blocks back to the header)
// becomes those passes in one block, then the header's final test values,
// and the exit. Values the loop's header defines read their final copies
// outside it. At most kUnrollBody cloned instructions.
const int64_t kUnrollTrips = 4;
const uint32_t kUnrollBody = 64;

bool LoopPass::try_unroll(uint32_t loop)
{
	const Loop& l = forest_.loops[loop];
	if (l.latches.size() != 1 || l.preheader == kNoId)
		return false;
	ExitTest test;
	std::vector<Induction> ivs;
	if (!exit_test(loop, test, ivs) || test.trips < 0 || test.trips > kUnrollTrips || test.exiting != l.header)
		return false;
	// The body chain: header -> b1 -> ... -> latch -> header, jumps only.
	std::vector<BlockId> chain;
	const Instruction& head_term = body_.instructions[body_.blocks[l.header].last];
	BlockId at = head_term.ops[1].index == test.exit ? head_term.ops[2].index : head_term.ops[1].index;
	while (at != l.header)
	{
		if (!in_loop(forest_, loop, at) || chain.size() > l.blocks.size() || cfg_.pred_count(at) != 1)
			return false;
		const Instruction& term = body_.instructions[body_.blocks[at].last];
		if (term.op != Opcode::Jump)
			return false;
		chain.push_back(at);
		at = term.ops[0].index;
	}
	if (chain.size() + 1 != l.blocks.size())
		return false;
	uint32_t size = 0;
	for (BlockId b : l.blocks)
		for (InstId i = body_.blocks[b].first; i != kNoId; i = body_.instructions[i].next)
		{
			const Instruction& inst = body_.instructions[i];
			if (is_eh_instruction(inst.op) || inst.op == Opcode::StackAlloc || inst.op == Opcode::VaStart)
				return false;
			size += inst.op == Opcode::Phi || is_terminator(inst.op) ? 0 : 1;
		}
	if (static_cast<uint64_t>(size) * static_cast<uint64_t>(test.trips + 1) > kUnrollBody)
		return false;
	// Clone pass by pass into one new block.
	const BlockId unrolled = append_block(body_, BlockKind::Unrolled);
	std::vector<Value> map(body_.temps.size(), Value::none());
	auto mapped = [&](Value v) {
		return v.is(ValueKind::Temp) && v.index < map.size() && !map[v.index].is(ValueKind::None) ? map[v.index] : v;
	};
	std::vector<std::pair<TempId, Value>> phis; // header phi, latch input
	for (InstId i = body_.blocks[l.header].first; i != kNoId && body_.instructions[i].op == Opcode::Phi;
		i = body_.instructions[i].next)
	{
		const Instruction& phi = body_.instructions[i];
		Value init = Value::none(), back = Value::none();
		for (uint32_t k = 0; k < phi.extra_count; k += 2)
			(body_.operands[phi.extra_first + k].index == l.latches[0] ? back : init) =
				body_.operands[phi.extra_first + k + 1];
		map[phi.result] = init;
		phis.push_back(std::make_pair(phi.result, back));
	}
	auto copy_block = [&](BlockId b) {
		for (InstId i = body_.blocks[b].first; i != kNoId; i = body_.instructions[i].next)
		{
			Instruction inst = body_.instructions[i];
			if (inst.op == Opcode::Phi || is_terminator(inst.op))
				continue;
			for (Value& v : inst.ops)
				v = mapped(v);
			std::vector<Value> extra;
			for (uint32_t k = 0; k < inst.extra_count; ++k)
				extra.push_back(mapped(body_.operands[inst.extra_first + k]));
			inst.extra_first = inst.extra_count != 0 ? append_operands(body_, extra.data(), inst.extra_count) : 0;
			const TempId old_result = inst.result;
			const InstId id = append_copy(body_, body_, i, inst);
			if (old_result != kNoId)
			{
				const TempId fresh = append_temp(body_, body_.temps[old_result].type, id);
				body_.instructions[id].result = fresh;
				map.resize(body_.temps.size(), Value::none());
				map[old_result] = Value::temp(fresh);
			}
			insert_after(body_, unrolled, body_.blocks[unrolled].last, id);
		}
	};
	for (int64_t pass = 0; pass < test.trips; ++pass)
	{
		copy_block(l.header);
		for (BlockId b : chain)
			copy_block(b);
		std::vector<Value> next;
		for (const std::pair<TempId, Value>& p : phis)
			next.push_back(mapped(p.second));
		for (size_t k = 0; k < phis.size(); ++k)
			map[phis[k].first] = next[k];
	}
	copy_block(l.header); // the final, leaving test
	Instruction jump = make_instruction(Opcode::Jump, Type::scalar(TypeKind::Void));
	jump.ops[0] = Value::block(test.exit);
	insert_after(body_, unrolled, body_.blocks[unrolled].last, append_instruction(body_, jump));
	// Entry, exit phis, and uses of header values outside the loop.
	for_each_successor(body_, body_.instructions[body_.blocks[l.preheader].last], [&](BlockId& s) {
		if (s == l.header)
			s = unrolled;
	});
	rename_phi_incoming(body_, test.exit, l.header, unrolled);
	for (BlockId b : body_.layout)
		if (!in_loop(forest_, loop, b))
			for (InstId i = body_.blocks[b].first; i != kNoId; i = body_.instructions[i].next)
				for_each_operand(body_, body_.instructions[i], [&](Value& v) {
					if (v.is(ValueKind::Temp) && !invariant(loop, v))
						v = mapped(v);
				});
	// The loop's own blocks are unreachable now: they leave the layout.
	std::vector<BlockId> layout;
	for (BlockId b : body_.layout)
	{
		if (b == l.header)
			layout.push_back(unrolled);
		if (!in_loop(forest_, loop, b))
			layout.push_back(b);
	}
	body_.layout.swap(layout);
	++c_.stats.loops_unrolled;
	return true;
}

// O3 full unrolling of a branching body: a counted loop tested at its
// header, its body branching freely and leaving early. Each pass is a copy
// of the loop's blocks entered by the previous pass's back edge (the first
// by the preheader; a final copy of the header leaves), where the induction
// is a literal. A table search (the loop reads a readonly global, whose
// loads then fold) may run kSearchTrips passes within kSearchBody copied
// instructions; any other loop with a call-free pass (the three-operand
// loop of an instruction walk, say; a body calling on every pass is
// mostly the callee's work, and the copies only grow the code)
// kShortTrips passes within kUnrollBranchBody, counted as copied: the
// loop's instructions per pass and the header's once more (the ELF
// writer's put64 loops: a byte stored per pass, its shift then a
// constant, the growth call off that path). A block
// outside the loop reading a loop value is a private exit (one
// predecessor, in the loop; no phi; a return or unreachable): copied with
// each pass. Exit phis take each exiting copy's value.
const int64_t kSearchTrips = 16;
// Loops unrolled (or given a preheader to unroll) per run, each found
// again after the previous change.
const unsigned kUnrollRounds = 16;
const uint32_t kSearchBody = 256;
const uint32_t kUnrollBranchBody = 160;
const int64_t kShortTrips = 8;

// Whether `v` is computed inside the loop (`member` blocks) from the
// induction `iv` and literals alone, within `depth` steps.
bool from_induction(const FunctionBody& body, const std::vector<BlockId>& block_of, const std::vector<uint8_t>& member,
	TempId iv, Value v, unsigned depth)
{
	if (v.is_literal())
		return true;
	if (!v.is(ValueKind::Temp) || depth == 0)
		return false;
	if (v.index == iv)
		return true;
	const InstId def = v.index >= body.parameter_count ? body.temps[v.index].def : kNoId;
	if (def == kNoId || def >= block_of.size() || block_of[def] == kNoId || !member[block_of[def]])
		return false;
	const Instruction& d = body.instructions[def];
	if (!(d.op == Opcode::Compare || d.op == Opcode::Binary || d.op == Opcode::Unary || d.op == Opcode::Convert ||
			d.op == Opcode::Copy))
		return false;
	for (const Value& o : d.ops)
		if (!o.is(ValueKind::None) && !from_induction(body, block_of, member, iv, o, depth - 1))
			return false;
	return true;
}

// The shape try_unroll_search copies: the loop's blocks (`member`: no
// nested loop or handler, no handler successor) and its private exits
// (`owned`), within the budget of its kind.
bool LoopPass::unroll_search_shape(uint32_t loop, int64_t trips, TempId iv, std::vector<uint8_t>& member,
	std::vector<uint8_t>& owned, BlockId& through)
{
	const Loop& l = forest_.loops[loop];
	member.assign(body_.blocks.size(), 0);
	for (BlockId b : l.blocks)
	{
		if (forest_.innermost[b] != loop || (cfg_.flags[b] & BF_HANDLER) != 0)
			return false;
		member[b] = 1;
	}
	bool table = false;
	uint32_t size = 0;
	std::vector<uint8_t> calling(body_.blocks.size(), 0);
	auto readonly_base = [&](Value v) {
		for (unsigned step = 0; step < 8 && v.is(ValueKind::Temp) && v.index >= body_.parameter_count; ++step)
		{
			const InstId def = body_.temps[v.index].def;
			if (def == kNoId)
				return false;
			const Instruction& d = body_.instructions[def];
			if (d.op == Opcode::Addr && d.ops[0].is(ValueKind::Symbol))
				return constant_contents(c_.program, d.ops[0].index, c_.unwritten);
			if (d.op != Opcode::Index && d.op != Opcode::Copy)
				return false;
			v = d.ops[0];
		}
		return false;
	};
	// An access of a frame slot (a local array the copies would index by
	// literals, which slot splitting then turns into one value per element).
	auto slot_base = [&](Value v) {
		for (unsigned step = 0; step < 8 && v.is(ValueKind::Temp) && v.index >= body_.parameter_count; ++step)
		{
			const InstId def = body_.temps[v.index].def;
			if (def == kNoId)
				return false;
			const Instruction& d = body_.instructions[def];
			if (d.op == Opcode::Addr)
				return d.ops[0].is(ValueKind::Slot);
			if (d.op != Opcode::Index && d.op != Opcode::Copy)
				return false;
			v = d.ops[0];
		}
		return v.is(ValueKind::Slot);
	};
	bool slots = false;
	const InstId head_test = body_.blocks[l.header].last;
	for (BlockId b : l.blocks)
		for (InstId i = body_.blocks[b].first; i != kNoId; i = body_.instructions[i].next)
		{
			const Instruction& inst = body_.instructions[i];
			if (is_eh_instruction(inst.op) || inst.op == Opcode::StackAlloc || inst.op == Opcode::VaStart)
				return false;
			slots = slots || (inst.op == Opcode::Load && slot_base(inst.ops[0])) ||
				(inst.op == Opcode::Store && slot_base(inst.ops[1]));
			size += inst.op == Opcode::Phi || is_terminator(inst.op) ? 0 : 1;
			calling[b] = calling[b] || inst.op == Opcode::Call;
			table = table || (inst.op == Opcode::Load && (inst.flags & IF_VOLATILE) == 0 && readonly_base(inst.ops[0]));
			// (A test of the induction and literals alone, `mask & (1 << r)`,
			// is a constant in each copy too.)
			table = table ||
				(inst.op == Opcode::Branch && i != head_test && iv != kNoId &&
					from_induction(body_, c_.block_of, member, iv, inst.ops[0], 6));
		}
	// A pass without a call: some path from the header back to the latch
	// through blocks that call nothing (a push_back's reallocation lies
	// beside such a path; a body calling on every pass has none).
	auto call_free_pass = [&]() {
		std::vector<uint8_t> seen(body_.blocks.size(), 0);
		std::vector<BlockId> stack;
		if (calling[l.header])
			return false;
		seen[l.header] = 1;
		stack.push_back(l.header);
		while (!stack.empty())
		{
			const BlockId b = stack.back();
			stack.pop_back();
			if (b == l.latches[0])
				return true;
			for (uint32_t k = cfg_.succ_first[b]; k < cfg_.succ_first[b + 1]; ++k)
			{
				const BlockId s = cfg_.succs[k];
				if (member[s] && !seen[s] && !calling[s])
				{
					seen[s] = 1;
					stack.push_back(s);
				}
			}
		}
		return false;
	};
	// A body that calls nothing and reads and writes no frame slot runs
	// kSearchTrips passes within kSearchBody too (g++ peels such loops
	// completely up to 16 passes: the per-register loops over an
	// allocator's state). A local array's elements would each become a
	// value live across the code around the copies.
	bool call_free = !slots;
	for (BlockId b : l.blocks)
		call_free = call_free && !calling[b];
	const bool long_pass = table || call_free;
	if (!table && (trips > (call_free ? kSearchTrips : kShortTrips) || !call_free_pass()))
		return false;
	// Outside readers of loop values: private exits, or exit phis (a phi
	// reading the value over an edge from the loop). Any other reader -- an
	// ordinary instruction, or a phi reading over an edge from outside (an
	// outer header's, through the outer latch) -- is reached through the
	// loop's one exit block `through`, where add_exit_phis gives it an exit
	// phi to read; with several exit blocks there is no such place.
	owned.assign(body_.blocks.size(), 0);
	through = kNoId;
	auto loop_value = [&](const Value& v) {
		if (!v.is(ValueKind::Temp) || v.index < body_.parameter_count)
			return false;
		const InstId def = body_.temps[v.index].def;
		return def != kNoId && def < c_.block_of.size() && c_.block_of[def] != kNoId && member[c_.block_of[def]];
	};
	bool live_through = false;
	for (BlockId b : body_.layout)
	{
		if (member[b])
			continue;
		bool reads = false, phi_reads = false;
		for (InstId i = body_.blocks[b].first; i != kNoId && !reads; i = body_.instructions[i].next)
		{
			const Instruction& inst = body_.instructions[i];
			if (inst.op == Opcode::Phi)
			{
				for (uint32_t k = 0; k < inst.extra_count; k += 2)
					phi_reads = phi_reads ||
						(!member[body_.operands[inst.extra_first + k].index] &&
							loop_value(body_.operands[inst.extra_first + k + 1]));
				continue;
			}
			for_each_operand(body_, inst, [&](const Value& v) { reads = reads || loop_value(v); });
		}
		if (!reads && !phi_reads)
			continue;
		const InstId last = body_.blocks[b].last;
		const InstId first = body_.blocks[b].first;
		if (reads && cfg_.pred_count(b) == 1 && member[cfg_.preds[cfg_.pred_first[b]]] && cfg_.succ_count(b) == 0 &&
			last != kNoId && body_.instructions[first].op != Opcode::Phi && (cfg_.flags[b] & BF_HANDLER) == 0 &&
			(body_.instructions[last].op == Opcode::Return || body_.instructions[last].op == Opcode::Unreachable))
		{
			owned[b] = 1;
			for (InstId i = first; i != kNoId; i = body_.instructions[i].next)
				size += is_terminator(body_.instructions[i].op) ? 0 : 1;
			continue;
		}
		live_through = true;
	}
	if (live_through)
	{
		for (BlockId b : l.blocks)
			for (uint32_t k = cfg_.succ_first[b]; k < cfg_.succ_first[b + 1]; ++k)
			{
				const BlockId s = cfg_.succs[k];
				if (member[s] || owned[s])
					continue;
				if (through != kNoId && through != s)
					return false;
				through = s;
			}
		if (through == kNoId || (cfg_.flags[through] & BF_HANDLER) != 0)
			return false;
	}
	uint32_t header_size = 0;
	for (InstId i = body_.blocks[l.header].first; i != kNoId; i = body_.instructions[i].next)
		header_size += body_.instructions[i].op == Opcode::Phi || is_terminator(body_.instructions[i].op) ? 0 : 1;
	const uint64_t cloned = static_cast<uint64_t>(size) * static_cast<uint64_t>(trips) + (table ? size : header_size);
	if (cloned > (long_pass ? kSearchBody : kUnrollBranchBody))
		return false;
	for (BlockId b : l.blocks)
		for (uint32_t k = cfg_.succ_first[b]; k < cfg_.succ_first[b + 1]; ++k)
			if (!member[cfg_.succs[k]] && (cfg_.flags[cfg_.succs[k]] & BF_HANDLER) != 0)
				return false;
	return true;
}

// Each loop value read outside the loop other than by an exit phi (a
// phi of `exit`, or of a private exit's successor -- there is none) gets a
// phi at `exit` taking it from every member predecessor (the value
// dominates `exit`, so each of them), and itself from any other
// predecessor (one `exit` dominates); the outside readers read that phi.
void LoopPass::add_exit_phis(const std::vector<uint8_t>& member, const std::vector<uint8_t>& owned, BlockId exit)
{
	std::vector<Value> exit_value(body_.temps.size(), Value::none());
	auto reader = [&](const Instruction& inst, uint32_t k) {
		// Whether operand k of an outside instruction reads a loop value
		// other than as an exit phi: a phi's input over an edge from outside.
		if (inst.op == Opcode::Phi)
			return k % 2 == 1 && !member[body_.operands[inst.extra_first + k - 1].index];
		return true;
	};
	auto loop_value = [&](const Value& v) {
		if (!v.is(ValueKind::Temp) || v.index < body_.parameter_count)
			return false;
		const InstId def = body_.temps[v.index].def;
		return def != kNoId && def < c_.block_of.size() && c_.block_of[def] != kNoId && member[c_.block_of[def]];
	};
	// Every operand of an outside instruction reading a loop value, as
	// (instruction, operand) with operand 0-2 the fixed ones and 3 + k the
	// k-th extra one: the phis are made after the walk, as making one may
	// move the instruction and operand arrays.
	std::vector<std::pair<InstId, uint32_t>> reads;
	for (BlockId b : body_.layout)
	{
		if (member[b] || owned[b])
			continue;
		for (InstId i = body_.blocks[b].first; i != kNoId; i = body_.instructions[i].next)
		{
			const Instruction& inst = body_.instructions[i];
			if (inst.op == Opcode::Phi)
			{
				for (uint32_t k = 1; k < inst.extra_count; k += 2)
					if (loop_value(body_.operands[inst.extra_first + k]) && reader(inst, k))
						reads.push_back(std::make_pair(i, 3 + k));
				continue;
			}
			for (uint32_t k = 0; k < 3; ++k)
				if (loop_value(inst.ops[k]))
					reads.push_back(std::make_pair(i, k));
			for (uint32_t k = 0; k < inst.extra_count; ++k)
				if (loop_value(body_.operands[inst.extra_first + k]))
					reads.push_back(std::make_pair(i, 3 + k));
		}
	}
	auto operand = [&](const std::pair<InstId, uint32_t>& r) -> Value& {
		Instruction& inst = body_.instructions[r.first];
		return r.second < 3 ? inst.ops[r.second] : body_.operands[inst.extra_first + r.second - 3];
	};
	for (const std::pair<InstId, uint32_t>& r : reads)
	{
		const Value v = operand(r);
		if (!exit_value[v.index].is(ValueKind::None))
			continue;
		const Type type = body_.temps[v.index].type;
		Instruction phi = make_instruction(Opcode::Phi, type);
		const InstId id = append_instruction(body_, phi);
		const TempId result = append_temp(body_, type, id);
		body_.instructions[id].result = result;
		std::vector<Value> pairs;
		for (uint32_t p = cfg_.pred_first[exit]; p < cfg_.pred_first[exit + 1]; ++p)
		{
			pairs.push_back(Value::block(cfg_.preds[p]));
			pairs.push_back(member[cfg_.preds[p]] ? v : Value::temp(result));
		}
		const uint32_t first = append_operands(body_, pairs.data(), static_cast<uint32_t>(pairs.size()));
		body_.instructions[id].extra_first = first;
		body_.instructions[id].extra_count = static_cast<uint32_t>(pairs.size());
		body_.instructions[id].location = body_.instructions[body_.temps[v.index].def].location;
		insert_after(body_, exit, kNoId, id);
		exit_value[v.index] = Value::temp(result);
	}
	for (const std::pair<InstId, uint32_t>& r : reads)
	{
		Value& v = operand(r);
		v = exit_value[v.index];
	}
}

bool LoopPass::try_unroll_search(uint32_t loop)
{
	const Loop& l = forest_.loops[loop];
	if (l.latches.size() != 1)
		return false;
	// The header's counted test.
	const Instruction& head_term = body_.instructions[body_.blocks[l.header].last];
	if (head_term.op != Opcode::Branch || !head_term.ops[0].is(ValueKind::Temp) ||
		in_loop(forest_, loop, head_term.ops[1].index) == in_loop(forest_, loop, head_term.ops[2].index))
		return false;
	const InstId cmp_id = body_.temps[head_term.ops[0].index].def;
	if (cmp_id == kNoId || body_.instructions[cmp_id].op != Opcode::Compare || c_.block_of[cmp_id] != l.header)
		return false;
	const Instruction& cmp = body_.instructions[cmp_id];
	const bool leave_on_true = !in_loop(forest_, loop, head_term.ops[1].index);
	int64_t trips = -1;
	TempId iv_temp = kNoId;
	for (InstId i = body_.blocks[l.header].first; i != kNoId && body_.instructions[i].op == Opcode::Phi && trips < 0;
		i = body_.instructions[i].next)
	{
		Induction iv;
		if (!induction(loop, i, iv) || iv.pointer || !iv.init.is(ValueKind::Integer) || !cmp.type.is_integer())
			continue;
		for (unsigned side = 0; side < 2 && trips < 0; ++side)
		{
			const Value v = cmp.ops[side], other = cmp.ops[1 - side];
			if (!v.is(ValueKind::Temp) || (v.index != iv.value && v.index != iv.next) || !other.is(ValueKind::Integer))
				continue;
			const ComparePredicate pred = side == 0 ? static_cast<ComparePredicate>(cmp.sub)
													: swapped(static_cast<ComparePredicate>(cmp.sub));
			const Literal a = normalize_integer(cmp.type, body_.literals[iv.init.index]);
			const Literal b = normalize_integer(cmp.type, body_.literals[other.index]);
			__int128 last;
			int64_t n;
			if (counted_trips(cmp.type, pred, !leave_on_true, static_cast<__int128>(static_cast<int64_t>(a.low)),
					iv.step_bytes, static_cast<__int128>(static_cast<int64_t>(b.low)), v.index == iv.next, n, last))
			{
				trips = n;
				iv_temp = iv.value;
			}
		}
	}
	if (trips < 1 || trips > kSearchTrips)
		return false;
	std::vector<uint8_t> member, owned;
	BlockId through;
	if (!unroll_search_shape(loop, trips, iv_temp, member, owned, through))
		return false;
	// A loop entered from a branching block gets its preheader first (the
	// caller finds the loops again and comes back).
	if (l.preheader == kNoId)
		return make_preheader(loop);
	if (through != kNoId)
		add_exit_phis(member, owned, through);
	// The copies: blocks and temporaries per pass, in reverse postorder
	// (a value's definition is copied before its uses: only the header
	// is entered by a back edge).
	std::vector<BlockId> order;
	for (BlockId b : cfg_.rpo)
		if (member[b] || owned[b])
			order.push_back(b);
	std::vector<std::vector<BlockId>> blocks(static_cast<size_t>(trips) + 1);
	std::vector<Value> map(body_.temps.size(), Value::none());
	std::vector<std::pair<TempId, Value>> phis; // header phi, latch input
	for (InstId i = body_.blocks[l.header].first; i != kNoId && body_.instructions[i].op == Opcode::Phi;
		i = body_.instructions[i].next)
	{
		const Instruction& phi = body_.instructions[i];
		Value init = Value::none(), back = Value::none();
		for (uint32_t k = 0; k < phi.extra_count; k += 2)
			(body_.operands[phi.extra_first + k].index == l.latches[0] ? back : init) =
				body_.operands[phi.extra_first + k + 1];
		map[phi.result] = init;
		phis.push_back(std::make_pair(phi.result, back));
	}
	auto mapped = [&](Value v) {
		return v.is(ValueKind::Temp) && v.index < map.size() && !map[v.index].is(ValueKind::None) ? map[v.index] : v;
	};
	std::vector<BlockId> copied; // in layout order: pass by pass
	for (int64_t pass = 0; pass <= trips; ++pass)
	{
		std::vector<BlockId>& mine = blocks[static_cast<size_t>(pass)];
		mine.assign(body_.blocks.size(), kNoId);
		// The last pass is the header's final test, and the private exits
		// it may leave to.
		for (BlockId b : body_.layout)
			if ((member[b] || owned[b]) && (pass < trips || b == l.header || owned[b]))
			{
				mine[b] = append_block(
					body_, body_.blocks[b].kind != BlockKind::Plain ? body_.blocks[b].kind : BlockKind::Unrolled);
				copied.push_back(mine[b]);
			}
	}
	const BlockId dead = append_block(body_, BlockKind::Dead);
	insert_after(body_, dead, kNoId,
		append_instruction(body_, make_instruction(Opcode::Unreachable, Type::scalar(TypeKind::Void))));
	copied.push_back(dead);
	// Exit phis' new pairs, per outside block (the loop's own pairs go).
	std::vector<std::vector<Value>> exit_pairs(body_.blocks.size());
	std::vector<BlockId> exit_blocks;
	for (int64_t pass = 0; pass <= trips; ++pass)
	{
		const std::vector<BlockId>& mine = blocks[static_cast<size_t>(pass)];
		auto target = [&](BlockId t) {
			if (t == l.header)
				return pass < trips ? blocks[static_cast<size_t>(pass) + 1][l.header] : dead;
			if (member[t] || owned[t])
				return mine[t] != kNoId ? mine[t] : dead;
			return t;
		};
		for (BlockId b : order)
		{
			if (mine[b] == kNoId)
				continue;
			for (InstId i = body_.blocks[b].first; i != kNoId; i = body_.instructions[i].next)
			{
				Instruction inst = body_.instructions[i];
				if (inst.op == Opcode::Phi && b == l.header)
					continue; // the header's phis read the previous pass (map)
				for (Value& v : inst.ops)
					v = v.is(ValueKind::Block) ? Value::block(target(v.index)) : mapped(v);
				std::vector<Value> extra;
				for (uint32_t k = 0; k < inst.extra_count; ++k)
				{
					const Value v = body_.operands[inst.extra_first + k];
					// A phi's predecessors are copies of this pass.
					extra.push_back(v.is(ValueKind::Block)
							? Value::block(inst.op == Opcode::Phi ? mine[v.index] : target(v.index))
							: mapped(v));
				}
				inst.extra_first = inst.extra_count != 0 ? append_operands(body_, extra.data(), inst.extra_count) : 0;
				const TempId old_result = inst.result;
				const InstId id = append_copy(body_, body_, i, inst);
				if (old_result != kNoId)
				{
					const TempId fresh = append_temp(body_, body_.temps[old_result].type, id);
					body_.instructions[id].result = fresh;
					map.resize(body_.temps.size(), Value::none());
					map[old_result] = Value::temp(fresh);
				}
				insert_after(body_, mine[b], body_.blocks[mine[b]].last, id);
			}
			if (!member[b])
				continue;
			// Edges leaving the loop: the target's phis take this copy.
			for_each_successor(body_, body_.instructions[body_.blocks[b].last], [&](BlockId t) {
				if (member[t] || owned[t])
					return;
				for (InstId i = body_.blocks[t].first; i != kNoId && body_.instructions[i].op == Opcode::Phi;
					i = body_.instructions[i].next)
				{
					if (exit_pairs[t].empty())
						exit_blocks.push_back(t);
					exit_pairs[t].push_back(Value::make(ValueKind::Integer, i)); // marks the phi (by instruction)
					exit_pairs[t].push_back(Value::block(mine[b]));
					exit_pairs[t].push_back(mapped(phi_incoming(body_, body_.instructions[i], b)));
				}
			});
		}
		// The next pass's header phis read this pass's latch values.
		std::vector<Value> next;
		for (const std::pair<TempId, Value>& p : phis)
			next.push_back(mapped(p.second));
		if (pass < trips)
			for (size_t k = 0; k < phis.size(); ++k)
				map[phis[k].first] = next[k];
	}
	// Exit phis: the loop's pairs replaced by the copies'.
	for (const BlockId t : exit_blocks)
	{
		const std::vector<Value>& add = exit_pairs[t];
		for (InstId i = body_.blocks[t].first; i != kNoId && body_.instructions[i].op == Opcode::Phi;
			i = body_.instructions[i].next)
		{
			Instruction& phi = body_.instructions[i];
			std::vector<Value> pairs;
			for (uint32_t k = 0; k < phi.extra_count; k += 2)
				if (!member[body_.operands[phi.extra_first + k].index])
				{
					pairs.push_back(body_.operands[phi.extra_first + k]);
					pairs.push_back(body_.operands[phi.extra_first + k + 1]);
				}
			for (size_t k = 0; k + 2 < add.size() + 1; k += 3)
				if (add[k].index == i)
				{
					pairs.push_back(add[k + 1]);
					pairs.push_back(add[k + 2]);
				}
			phi.extra_first = append_operands(body_, pairs.data(), static_cast<uint32_t>(pairs.size()));
			phi.extra_count = static_cast<uint32_t>(pairs.size());
		}
	}
	// Entry, and the layout: the copies where the loop was.
	for_each_successor(body_, body_.instructions[body_.blocks[l.preheader].last], [&](BlockId& t) {
		if (t == l.header)
			t = blocks[0][l.header];
	});
	std::vector<BlockId> layout;
	for (BlockId b : body_.layout)
	{
		if (b == l.header)
			layout.insert(layout.end(), copied.begin(), copied.end());
		if (!member[b] && !owned[b])
			layout.push_back(b);
	}
	body_.layout.swap(layout);
	++c_.stats.loops_unrolled;
	return true;
}

bool LoopPass::unroll()
{
	c_.ensure_cfg();
	find_loops(body_, cfg_, forest_);
	c_.entry_block_of(unroll_loops);
	bool any = false;
	for (unsigned round = 0; round < kUnrollRounds; ++round)
	{
		bool one = false;
		for (uint32_t k = static_cast<uint32_t>(forest_.loops.size()); k-- > 0 && !one;)
			one = try_unroll(k) || (c_.level >= 3 && try_unroll_search(k));
		if (!one)
			break;
		any = true;
		// The copies changed the graph: the loops are found again.
		c_.invalidate_cfg();
		c_.ensure_cfg();
		find_loops(body_, cfg_, forest_);
		map_instruction_blocks(body_, c_.block_of);
		c_.invalidate_block_of();
	}
	return any;
}

const uint32_t kLoopTries = 16;

// O2+: a loop counting an i64 down to zero by ones (its header branching on
// `c` or `ne|eq c, 0`) beside a pointer induction tests the pointer against
// its end instead -- init + n * step, formed in the preheader -- when the
// counter has no other use: it goes (std::uninitialized_fill_n's
// `for (; n > 0; --n, ++p)`, which then compares the pointer once per pass).
bool LoopPass::count_to_pointer(uint32_t loop)
{
	const Loop& l = forest_.loops[loop];
	if (c_.level < 2 || l.latches.size() != 1 || l.preheader == kNoId)
		return false;
	const BlockId h = l.header;
	const InstId t = body_.blocks[h].last;
	if (t == kNoId)
		return false;
	Instruction& term = body_.instructions[t];
	if (term.op != Opcode::Branch || !term.ops[0].is(ValueKind::Temp) || term.ops[1].index == term.ops[2].index ||
		in_loop(forest_, loop, term.ops[1].index) == in_loop(forest_, loop, term.ops[2].index))
		return false;
	// The tested counter and whether the true side continues with it nonzero.
	Value c = term.ops[0];
	InstId compare = kNoId;
	bool nonzero_true = true;
	if (c.index >= body_.parameter_count && body_.temps[c.index].def != kNoId &&
		body_.instructions[body_.temps[c.index].def].op == Opcode::Compare)
	{
		compare = body_.temps[c.index].def;
		const Instruction& cmp = body_.instructions[compare];
		const ComparePredicate pred = static_cast<ComparePredicate>(cmp.sub);
		Value a = cmp.ops[0], k = cmp.ops[1];
		if (!a.is(ValueKind::Temp))
			std::swap(a, k);
		if (!k.is(ValueKind::Integer) || body_.literals[k.index].low != 0 || body_.literals[k.index].high != 0 ||
			(pred != ComparePredicate::Ne && pred != ComparePredicate::Eq) || !a.is(ValueKind::Temp))
			return false;
		nonzero_true = pred == ComparePredicate::Ne;
		c = a;
	}
	if (!body_.temps[c.index].type.is_integer() || body_.temps[c.index].type.kind() != TypeKind::I64)
		return false;
	Induction counter, pointer;
	bool have_counter = false, have_pointer = false;
	for (InstId i = body_.blocks[h].first; i != kNoId && body_.instructions[i].op == Opcode::Phi;
		i = body_.instructions[i].next)
	{
		Induction iv;
		if (!induction(loop, i, iv))
			continue;
		if (!iv.pointer && iv.value == c.index && iv.step_bytes == -1)
		{
			counter = iv;
			have_counter = true;
		}
		else if (iv.pointer && !have_pointer)
		{
			pointer = iv;
			have_pointer = true;
		}
	}
	if (!have_counter || !have_pointer || (nonzero_true != in_loop(forest_, loop, term.ops[1].index)))
		return false;
	// The counter is read only by the test and its step; the step only by
	// the counter's phi.
	uint32_t counter_uses = 0, next_uses = 0;
	for (BlockId b : body_.layout)
		for (InstId i = body_.blocks[b].first; i != kNoId; i = body_.instructions[i].next)
			for_each_operand(body_, body_.instructions[i], [&](const Value& v) {
				if (v.is(ValueKind::Temp) && v.index == counter.value)
					++counter_uses;
				if (v.is(ValueKind::Temp) && v.index == counter.next)
					++next_uses;
			});
	if (counter_uses != 2 || next_uses != 1)
		return false;
	// The end in the preheader, the pointer's test in the header.
	const BlockId pre = l.preheader;
	InstId at = previous_in_block(body_, pre, body_.blocks[pre].last);
	auto emit = [&](BlockId b, InstId& after, Instruction inst, Type result) {
		inst.location = term.location;
		const InstId id = append_instruction(body_, inst);
		body_.instructions[id].result = append_temp(body_, result, id);
		insert_after(body_, b, after, id);
		after = id;
		return Value::temp(body_.instructions[id].result);
	};
	const Type i64 = Type::scalar(TypeKind::I64), ptr = Type::scalar(TypeKind::Ptr);
	Instruction scale = make_instruction(Opcode::Binary, i64);
	scale.sub = static_cast<uint8_t>(BinaryOp::Mul);
	scale.ops[0] = counter.init;
	scale.ops[1] = integer_value(body_, integer_literal(pointer.step_bytes));
	const Value bytes = emit(pre, at, scale, i64);
	Instruction end = make_instruction(Opcode::Index, Type::scalar(TypeKind::I8));
	end.ops[0] = pointer.init;
	end.ops[1] = bytes;
	const Value bound = emit(pre, at, end, ptr);
	InstId before = previous_in_block(body_, h, t);
	Instruction test = make_instruction(Opcode::Compare, ptr);
	test.sub = static_cast<uint8_t>(ComparePredicate::Ne);
	test.ops[0] = Value::temp(pointer.value);
	test.ops[1] = bound;
	const Value differs = emit(h, before, test, i64);
	body_.instructions[t].ops[0] = differs;
	if (!nonzero_true)
		std::swap(body_.instructions[t].ops[1], body_.instructions[t].ops[2]);
	(void)compare;
	map_instruction_blocks(body_, c_.block_of);
	c_.invalidate_block_of();
	++c_.stats.loops_removed;
	return true;
}

// O2+: a loop whose one store writes, every pass, the value its one load
// just read from an invariant address reads it once in the preheader: a
// store aliasing that address writes what it holds (same type and width),
// so every pass reads the same value. Only an address that may always be
// read (a slot, a global, a by-address parameter, a constant field offset
// within one of them), since the preheader runs even when the loop does not
// (std::fill_n's `*first = value` through a reference).
bool LoopPass::hoist_stored_load(uint32_t loop)
{
	const Loop& l = forest_.loops[loop];
	if (c_.level < 2 || l.latches.size() != 1 || l.preheader == kNoId)
		return false;
	InstId load = kNoId, store = kNoId;
	for (BlockId b : l.blocks)
		for (InstId i = body_.blocks[b].first; i != kNoId; i = body_.instructions[i].next)
		{
			const Instruction& inst = body_.instructions[i];
			if (inst.op == Opcode::Load)
			{
				if (load != kNoId)
					return false;
				load = i;
			}
			else if (inst.op == Opcode::Store)
			{
				if (store != kNoId)
					return false;
				store = i;
			}
			else if (!is_pure(inst) && inst.op != Opcode::Jump && inst.op != Opcode::Branch)
				return false;
		}
	if (load == kNoId || store == kNoId)
		return false;
	const Instruction& ld = body_.instructions[load];
	const Instruction& st = body_.instructions[store];
	if ((ld.flags & IF_VOLATILE) != 0 || (st.flags & IF_VOLATILE) != 0 || !(st.ops[0] == Value::temp(ld.result)) ||
		ld.type != st.type || !(ld.type.is_integer() || ld.type.is_pointer()) || ld.type.kind() == TypeKind::I128 ||
		!invariant(loop, ld.ops[0]))
		return false;
	// The load runs before the store in every pass.
	const BlockId lb = c_.block_of[load], sb = c_.block_of[store];
	if (lb == sb)
	{
		bool before = false;
		for (InstId i = body_.blocks[lb].first; i != kNoId && i != store; i = body_.instructions[i].next)
			before = before || i == load;
		if (!before)
			return false;
	}
	else if (!cfg_.dominates(lb, sb))
		return false;
	// An address that may always be read.
	Value a = ld.ops[0];
	uint64_t offset = 0;
	for (unsigned step = 0; step < 4 && a.is(ValueKind::Temp) && a.index >= body_.parameter_count; ++step)
	{
		const InstId d = body_.temps[a.index].def;
		if (d == kNoId)
			return false;
		const Instruction& def = body_.instructions[d];
		if (def.op == Opcode::Addr && (def.ops[0].is(ValueKind::Slot) || def.ops[0].is(ValueKind::Symbol)))
		{
			a = def.ops[0];
			break;
		}
		if (def.op != Opcode::Index || !def.ops[1].is(ValueKind::Integer) || body_.literals[def.ops[1].index].high != 0)
			return false;
		offset += body_.literals[def.ops[1].index].low * storage_bytes(def.type);
		a = def.ops[0];
	}
	bool readable = a.is(ValueKind::Slot) || a.is(ValueKind::Symbol);
	if (a.is(ValueKind::Temp) && a.index < body_.parameter_count)
	{
		const Function& function = c_.program.functions[c_.program.symbols[c_.symbol].detail];
		const Signature& sig = c_.program.signatures[function.signature];
		const Parameter& param = c_.program.parameters_of(sig)[a.index];
		readable = param.pass == ir_model::PPM_BY_ADDRESS && offset + storage_bytes(ld.type) <= param.object_bytes &&
			entry_objects_kept(loop, a.index);
	}
	if (!readable)
		return false;
	unlink_after(body_, lb, previous_in_block(body_, lb, load), load);
	insert_after(body_, l.preheader, previous_in_block(body_, l.preheader, body_.blocks[l.preheader].last), load);
	body_.instructions[load].flags &= static_cast<uint8_t>(~IF_TYPED); // it may run where it did not
	map_instruction_blocks(body_, c_.block_of);
	c_.invalidate_block_of();
	++c_.stats.loops_removed;
	return true;
}

bool LoopPass::run(bool fresh)
{
	c_.ensure_cfg();
	find_loops(body_, cfg_, forest_);
	if (forest_.loops.empty())
		return false;
	c_.entry_block_of(remove_loops, fresh);
	// What each loop holds (its nested loops included), counted once: only
	// a loop without effects (bypass), with one store, at most one load
	// and no other effect (fill), or at O2+ with a few stores and their
	// loads (indexed_stores) is examined, at most kLoopTries of them per
	// run (each examination scans the body).
	const uint32_t n = static_cast<uint32_t>(forest_.loops.size());
	std::vector<uint32_t> stores(n, 0), loads(n, 0), others(n, 0), traps(n, 0);
	for (BlockId b : body_.layout)
	{
		const uint32_t x = b < forest_.innermost.size() ? forest_.innermost[b] : kNoId;
		if (x == kNoId)
			continue;
		for (InstId i = body_.blocks[b].first; i != kNoId; i = body_.instructions[i].next)
		{
			const Instruction& inst = body_.instructions[i];
			if (inst.op == Opcode::Store)
				++stores[x];
			else if (inst.op == Opcode::Load)
				++loads[x];
			else if (inst.op == Opcode::Binary &&
				(static_cast<BinaryOp>(inst.sub) == BinaryOp::Div || static_cast<BinaryOp>(inst.sub) == BinaryOp::Mod ||
					static_cast<BinaryOp>(inst.sub) == BinaryOp::UDiv ||
					static_cast<BinaryOp>(inst.sub) == BinaryOp::UMod))
				++traps[x];
			else if (!is_pure(inst) && inst.op != Opcode::Jump && inst.op != Opcode::Branch)
				++others[x];
		}
	}
	for (uint32_t k = n; k-- > 0;)
	{
		const uint32_t p = forest_.loops[k].parent;
		if (p == kNoId)
			continue;
		stores[p] += stores[k];
		loads[p] += loads[k];
		others[p] += others[k];
		traps[p] += traps[k];
	}
	// Innermost loops first (later in the list); one rewrite per run,
	// since a bypass changes the CFG the other loops were found in.
	for (uint32_t k = n; k-- > 0;)
		if (others[k] == 0 && stores[k] != 0 && count_to_pointer(k))
			return true;
	for (uint32_t k = n; k-- > 0;)
		if (others[k] == 0 && stores[k] == 1 && loads[k] == 1 && hoist_stored_load(k))
			return true;
	uint32_t tries = 0;
	for (uint32_t k = n; k-- > 0 && tries < kLoopTries;)
	{
		const bool bypassable = stores[k] == 0 && loads[k] == 0 && others[k] == 0 && traps[k] == 0;
		const bool fill = stores[k] == 1 && loads[k] <= 1 && others[k] == 0;
		const bool indexed = c_.level >= 2 && stores[k] > 1 && stores[k] <= kIndexedStores && loads[k] <= stores[k] &&
			others[k] == 0 && traps[k] == 0;
		if (!bypassable && !fill && !indexed)
			continue;
		++tries;
		if (try_loop(k))
			return true;
	}
	return false;
}

} // namespace

// Loop counters whose increment never wraps (IF_NO_WRAP): a narrow integer
// induction stepped by one where a branch leaves the loop unless the
// counter (tested in the header) or its next value (tested in the latch,
// from a literal start below the type's maximum: the body runs once
// before the test) is below a loop-invariant bound of its type and
// signedness: the increment then never passes the bound. The selector
// keeps its 64-bit sum as the canonical form (an i32 index sign-extended,
// a u8 register counter zero-extended, renormalized on every pass
// before).
namespace {

// A typed access (IF_TYPED) of a pointer and one of a non-pointer wider
// than a byte name different objects unless their addresses agree (as in
// memory forwarding).
bool typed_kinds_apart(Type a, Type b)
{
	auto wide = [](Type t) {
		const TypeKind k = t.kind();
		return k != TypeKind::I1 && k != TypeKind::I8 && k != TypeKind::U8 && k != TypeKind::Void && !t.is_object();
	};
	return a.is_pointer() != b.is_pointer() && wide(a) && wide(b);
}

// The registers values live across a loop may take before a hoisted load
// (which holds one across the loop) would push one into the frame: the
// loop's live-in values and header phis count against it.
const uint32_t kLoopRegisters = 10;

// A write of the loop: its location and access.
struct LoopWrite
{
	Located at;
	uint64_t bytes;
	Type type;
	bool typed;
};

} // namespace

// Whether by-address parameter `parameter`'s object, readable on entry,
// is still readable where `loop` is entered: on every path from the entry
// to the loop's entry edges, no call that may write memory (and so free
// it: munmap, operator delete) runs after the entry or after the last
// access through the parameter (a load or store within its object_bytes:
// the object was live then, as a whole). Handlers count as reached after
// such a call. A forward dataflow over cfg_ of the first 64 parameters'
// facts, once per graph.
bool LoopPass::entry_objects_kept(uint32_t loop, uint32_t parameter)
{
	if (parameter >= 64)
		return false;
	const uint32_t n = cfg_.block_count;
	if (live_objects_.size() != n)
	{
		const Function& function = c_.program.functions[c_.program.symbols[c_.symbol].detail];
		const Signature& sig = c_.program.signatures[function.signature];
		const uint32_t count = std::min<uint32_t>(std::min(body_.parameter_count, sig.parameter_count), 64);
		// Per block: whether it calls (its input then counts for nothing),
		// and the parameters it accesses after its last call.
		std::vector<uint8_t> calls(n, 0);
		std::vector<uint64_t> accessed(n, 0);
		for (BlockId b : cfg_.rpo)
			for (InstId i = body_.blocks[b].first; i != kNoId; i = body_.instructions[i].next)
			{
				const Instruction& inst = body_.instructions[i];
				if (inst.op == Opcode::Call && call_may_write_memory(c_.program, inst))
				{
					calls[b] = 1;
					accessed[b] = 0;
					continue;
				}
				if (inst.op != Opcode::Load && inst.op != Opcode::Store)
					continue;
				const Located at = locate(body_, inst.ops[inst.op == Opcode::Load ? 0 : 1]);
				if (!at.root.is(ValueKind::Temp) || at.root.index >= count || !at.known || at.offset < 0)
					continue;
				const Parameter& param = c_.program.parameters_of(sig)[at.root.index];
				if (param.pass == ir_model::PPM_BY_ADDRESS && static_cast<uint64_t>(at.offset) < param.object_bytes)
					accessed[b] |= uint64_t(1) << at.root.index;
			}
		const uint64_t all = count == 64 ? ~uint64_t(0) : (uint64_t(1) << count) - 1;
		live_objects_.assign(n, all);
		const BlockId entry = cfg_.rpo.empty() ? kNoId : cfg_.rpo[0];
		for (bool changed = true; changed;)
		{
			changed = false;
			for (BlockId b : cfg_.rpo)
			{
				uint64_t in = b == entry || ((cfg_.flags[b] & BF_HANDLER) == 0 && cfg_.pred_count(b) != 0) ? all : 0;
				for (uint32_t k = cfg_.pred_first[b]; k < cfg_.pred_first[b + 1] && in != 0; ++k)
					in &= live_objects_[cfg_.preds[k]];
				const uint64_t out = (calls[b] ? 0 : in) | accessed[b];
				if (out != live_objects_[b])
				{
					live_objects_[b] = out;
					changed = true;
				}
			}
		}
	}
	const Loop& l = forest_.loops[loop];
	for (uint32_t k = cfg_.pred_first[l.header]; k < cfg_.pred_first[l.header + 1]; ++k)
	{
		const BlockId p = cfg_.preds[k];
		if (!in_loop(forest_, loop, p) && !(live_objects_[p] >> parameter & 1))
			return false;
	}
	return true;
}

// Whether a load at `at` (bytes, type, typed) may read what `w` writes.
static bool may_overlap(const LoopWrite& w, const Located& at, uint64_t bytes, Type type, bool typed)
{
	const bool named_a = w.at.root.is(ValueKind::Slot) || w.at.root.is(ValueKind::Symbol);
	const bool named_b = at.root.is(ValueKind::Slot) || at.root.is(ValueKind::Symbol);
	if (w.at.root == at.root)
		return !w.at.known || !at.known ||
			(w.at.offset < at.offset + static_cast<int64_t>(bytes) &&
				at.offset < w.at.offset + static_cast<int64_t>(w.bytes));
	if (named_a && named_b)
		return false; // distinct slots and globals
	return !(w.typed && typed && typed_kinds_apart(w.type, type));
}

// Moves the loads of `loop` that read loop-invariant memory to its
// preheader (see hoist_loop_loads): each load's address steps inside the
// loop (index, addr, pointer copies) go first. A load moves when it runs
// whenever the loop is entered (in the header, before any call) or reads
// memory that may always be read (a slot, a global, a parameter's object).
// Without `apply`: whether some load would move (no preheader needed).
bool LoopPass::hoist_from(uint32_t loop, bool apply)
{
	const Loop& l = forest_.loops[loop];
	if (apply && l.preheader == kNoId)
		return false;
	std::vector<LoopWrite> writes;
	for (BlockId b : l.blocks)
		for (InstId i = body_.blocks[b].first; i != kNoId; i = body_.instructions[i].next)
		{
			const Instruction& inst = body_.instructions[i];
			if (inst.op == Opcode::Store)
				writes.push_back(LoopWrite{ locate(body_, inst.ops[1]), storage_bytes(inst.type), inst.type,
					(inst.flags & IF_TYPED) != 0 && (inst.flags & IF_VOLATILE) == 0 });
			else if (inst.op == Opcode::CopyObject || inst.op == Opcode::ZeroInit)
				writes.push_back(LoopWrite{ locate(body_, inst.op == Opcode::CopyObject ? inst.ops[1] : inst.ops[0]),
					static_cast<uint64_t>(inst.immediate), Type::scalar(TypeKind::Void), false });
			else if (may_write_memory(c_.program, inst) || (inst.op == Opcode::Load && (inst.flags & IF_VOLATILE) != 0))
				return false; // a call, an atomic, a volatile access: memory may change anywhere
		}
	const Function& function = c_.program.functions[c_.program.symbols[c_.symbol].detail];
	const Signature& sig = c_.program.signatures[function.signature];
	// Address steps inside the loop whose operands are invariant.
	auto movable_step = [&](InstId d) {
		const Instruction& def = body_.instructions[d];
		if (!(def.op == Opcode::Index || def.op == Opcode::Addr || (def.op == Opcode::Copy && def.type.is_pointer())))
			return false;
		bool ok = true;
		for (const Value& v : def.ops)
			ok = ok && invariant(loop, v);
		return ok;
	};
	// Values live into the loop (defined outside, used inside) and its
	// header phis: the registers already held across it.
	uint32_t live_in = 0;
	{
		std::vector<uint8_t> seen(body_.temps.size(), 0);
		for (BlockId b : l.blocks)
			for (InstId i = body_.blocks[b].first; i != kNoId; i = body_.instructions[i].next)
			{
				const Instruction& inst = body_.instructions[i];
				if (b == l.header && inst.op == Opcode::Phi)
					++live_in;
				for_each_operand(body_, inst, [&](const Value& v) {
					if (v.is(ValueKind::Temp) && v.index < seen.size() && !seen[v.index] && invariant(loop, v))
					{
						seen[v.index] = 1;
						++live_in;
					}
				});
			}
	}
	const uint32_t limit = live_in < kLoopRegisters ? kLoopRegisters - live_in : 0;
	bool changed = false;
	uint32_t moved = 0;
	// The header's instructions before its first call run on every entry.
	bool header_clear = true;
	for (bool progress = true; progress && moved < limit;)
	{
		progress = false;
		for (BlockId b : l.blocks)
		{
			header_clear = b == l.header;
			for (InstId i = body_.blocks[b].first; i != kNoId;)
			{
				const Instruction& inst = body_.instructions[i];
				const InstId next = inst.next;
				if (inst.op == Opcode::Call || may_unwind(c_.program, inst))
					header_clear = false;
				if (inst.op != Opcode::Load || (inst.flags & IF_VOLATILE) != 0 || inst.type.is_object() ||
					inst.result == kNoId)
				{
					i = next;
					continue;
				}
				// The address: invariant, or steps inside the loop from an
				// invariant base.
				std::vector<InstId> steps;
				Value a = inst.ops[0];
				while (!invariant(loop, a) && a.is(ValueKind::Temp) && body_.temps[a.index].def != kNoId &&
					movable_step(body_.temps[a.index].def) && steps.size() < 4)
				{
					steps.push_back(body_.temps[a.index].def);
					a = body_.instructions[steps.back()].ops[0];
				}
				if (!invariant(loop, a))
				{
					i = next;
					continue;
				}
				const Located at = locate(body_, inst.ops[0]);
				const uint64_t bytes = storage_bytes(inst.type);
				const bool typed = (inst.flags & IF_TYPED) != 0;
				bool clobbered = false;
				for (const LoopWrite& w : writes)
					clobbered = clobbered || may_overlap(w, at, bytes, inst.type, typed);
				// Run on every entry, the load keeps its facts; moved only
				// because its memory may always be read, it runs where it did
				// not, and `typed` may not hold there (lowir.md "Instruction Facts").
				const bool on_entry = header_clear && b == l.header;
				bool readable = on_entry;
				if (!readable && at.known && (at.root.is(ValueKind::Slot) || at.root.is(ValueKind::Symbol)))
					readable = true;
				if (!readable && at.known && at.offset >= 0 && at.root.is(ValueKind::Temp) &&
					at.root.index < body_.parameter_count && at.root.index < sig.parameter_count)
				{
					const Parameter& param = c_.program.parameters_of(sig)[at.root.index];
					readable = param.object_bytes != 0 &&
						static_cast<uint64_t>(at.offset) + bytes <= param.object_bytes &&
						entry_objects_kept(loop, at.root.index);
				}
				if (clobbered || !readable || moved >= limit)
				{
					i = next;
					continue;
				}
				if (!apply)
					return true;
				++moved;
				// Steps base first, then the load, before the preheader's jump.
				const BlockId pre = l.preheader;
				for (size_t k = steps.size(); k-- > 0;)
				{
					const InstId s = steps[k];
					const BlockId sb = c_.block_of[s];
					unlink_after(body_, sb, previous_in_block(body_, sb, s), s);
					insert_after(body_, pre, previous_in_block(body_, pre, body_.blocks[pre].last), s);
					c_.block_of[s] = pre;
					c_.invalidate_block_of();
				}
				unlink_after(body_, b, previous_in_block(body_, b, i), i);
				insert_after(body_, pre, previous_in_block(body_, pre, body_.blocks[pre].last), i);
				if (!on_entry)
					body_.instructions[i].flags &= static_cast<uint8_t>(~IF_TYPED);
				c_.block_of[i] = pre;
				c_.invalidate_block_of();
				changed = progress = true;
				i = next;
			}
		}
	}
	// Arithmetic on invariant values (the loads above may supply them): an
	// integer or pointer Binary other than a division, or an extension or
	// truncation, whose operands are all invariant moves to the preheader
	// when one of its operand temps is read nowhere else. That operand no
	// longer lives through the loop and the result does instead (a
	// `size()` bound's subtraction and shift). An integer conversion to f32
	// or f64 moves whatever else reads its operand: it cannot trap, and even
	// spilled its result is cheaper to reload than to convert. Compares and
	// address steps stay: they fuse with their branch or access.
	// (Counted once per function: moving instructions changes no count.)
	if (hoist_uses_.empty())
		count_uses(body_, hoist_uses_);
	const std::vector<uint32_t>& uses = hoist_uses_;
	for (bool progress = true; progress;)
	{
		progress = false;
		for (BlockId b : l.blocks)
			for (InstId i = body_.blocks[b].first; i != kNoId;)
			{
				const Instruction& inst = body_.instructions[i];
				const InstId next = inst.next;
				const BinaryOp bop = static_cast<BinaryOp>(inst.sub);
				const ConvertOp cop = static_cast<ConvertOp>(inst.sub);
				const bool arithmetic = inst.result != kNoId && (inst.type.is_integer() || inst.type.is_pointer()) &&
					inst.type.kind() != TypeKind::I128 &&
					((inst.op == Opcode::Binary && bop != BinaryOp::Div && bop != BinaryOp::Mod &&
						 bop != BinaryOp::UDiv && bop != BinaryOp::UMod) ||
						(inst.op == Opcode::Convert && convert_source_type(inst).is_integer() &&
							(cop == ConvertOp::ZExt || cop == ConvertOp::SExt || cop == ConvertOp::Trunc)));
				const bool to_float = inst.op == Opcode::Convert &&
					(cop == ConvertOp::SIToFP || cop == ConvertOp::UIToFP) &&
					(inst.type.kind() == TypeKind::F32 || inst.type.kind() == TypeKind::F64) &&
					convert_source_type(inst).kind() != TypeKind::I128;
				bool all = arithmetic || to_float, dying = to_float;
				for (unsigned k = 0; k < 3 && all; ++k)
				{
					const Value v = inst.ops[k];
					all = invariant(loop, v);
					if (!v.is(ValueKind::Temp))
						continue;
					uint32_t here = 0;
					for (const Value& w : inst.ops)
						here += w == v ? 1 : 0;
					dying = dying || uses[v.index] == here;
				}
				if (!all || !dying)
				{
					i = next;
					continue;
				}
				if (!apply)
					return true;
				const BlockId pre = l.preheader;
				unlink_after(body_, b, previous_in_block(body_, b, i), i);
				insert_after(body_, pre, previous_in_block(body_, pre, body_.blocks[pre].last), i);
				// It may now run where it did not: its facts may not hold.
				body_.instructions[i].flags &= static_cast<uint8_t>(~(IF_NO_WRAP | IF_EXACT));
				c_.block_of[i] = pre;
				c_.invalidate_block_of();
				changed = progress = true;
				i = next;
			}
	}
	return changed;
}

// Innermost loops first: a load hoisted into an inner loop's preheader is
// a candidate of the loop around it. Loops get a preheader first.
bool LoopPass::hoist_loads()
{
	c_.ensure_cfg();
	find_loops(body_, cfg_, forest_);
	if (forest_.loops.empty())
		return false;
	c_.entry_block_of(hoist_loop_loads);
	// A preheader for each loop with a load to move (each one made changes
	// the graph: the loops are found again).
	bool made = false;
	for (unsigned round = 0; round < 16; ++round)
	{
		bool one = false;
		for (uint32_t loop = 0; loop < forest_.loops.size() && !one; ++loop)
			one = forest_.loops[loop].preheader == kNoId && hoist_from(loop, false) && make_preheader(loop);
		if (!one)
			break;
		made = true;
		c_.invalidate_cfg();
		c_.ensure_cfg();
		live_objects_.clear();
		find_loops(body_, cfg_, forest_);
		map_instruction_blocks(body_, c_.block_of);
		c_.invalidate_block_of();
	}
	bool changed = made;
	for (uint32_t loop = static_cast<uint32_t>(forest_.loops.size()); loop-- > 0;)
		changed = hoist_from(loop, true) || changed;
	return changed;
}

bool LoopPass::mark_no_wrap()
{
	c_.ensure_cfg();
	find_loops(body_, cfg_, forest_);
	bool any = false;
	for (uint32_t loop = 0; loop < forest_.loops.size(); ++loop)
	{
		const Loop& l = forest_.loops[loop];
		if (l.latches.size() != 1)
			continue;
		for (InstId i = body_.blocks[l.header].first; i != kNoId && body_.instructions[i].op == Opcode::Phi;
			i = body_.instructions[i].next)
		{
			const Type type = body_.instructions[i].type;
			const TypeKind kind = type.kind();
			if (!(kind == TypeKind::I8 || kind == TypeKind::U8 || kind == TypeKind::I16 || kind == TypeKind::U16 ||
					kind == TypeKind::I32))
				continue;
			Induction iv;
			if (!induction(loop, i, iv) || iv.pointer || iv.step_bytes != 1)
				continue;
			const InstId step = body_.temps[iv.next].def;
			if (body_.instructions[step].op != Opcode::Binary ||
				static_cast<BinaryOp>(body_.instructions[step].sub) != BinaryOp::Add)
				continue;
			const bool is_signed = kind == TypeKind::I8 || kind == TypeKind::I16 || kind == TypeKind::I32;
			// (A latch test sees the start's increment first.)
			bool start_below_max = false;
			if (iv.init.is(ValueKind::Integer))
			{
				const unsigned bits = static_cast<unsigned>(8 * storage_bytes(type)) - (is_signed ? 1 : 0);
				const Literal start = normalize_integer(type, body_.literals[iv.init.index]);
				start_below_max = static_cast<int64_t>(start.low) < static_cast<int64_t>((1ull << bits) - 1);
			}
			// The guard: the header's branch on the counter (the header's
			// other successor leaving), or the latch's on the next value.
			// The header's guards only a step past it: one in the header
			// runs before its test, on the leaving pass too.
			bool step_in_header = false;
			for (InstId k = body_.blocks[l.header].first; k != kNoId && !step_in_header; k = body_.instructions[k].next)
				step_in_header = k == step;
			bool guarded = false;
			for (unsigned which = 0; which < 2 && !guarded; ++which)
			{
				if ((which == 0 && step_in_header) || (which == 1 && !start_below_max))
					continue;
				const BlockId b = which == 0 ? l.header : l.latches[0];
				const Instruction& term = body_.instructions[body_.blocks[b].last];
				if (term.op != Opcode::Branch || !term.ops[0].is(ValueKind::Temp) ||
					body_.temps[term.ops[0].index].def == kNoId)
					continue;
				const Instruction& cmp = body_.instructions[body_.temps[term.ops[0].index].def];
				if (cmp.op != Opcode::Compare || cmp.type != type)
					continue;
				const bool in1 = in_loop(forest_, loop, term.ops[1].index),
						   in2 = in_loop(forest_, loop, term.ops[2].index);
				if (in1 == in2)
					continue;
				// Continuing on true (the in-loop target first) or on false.
				const bool continue_on_true = in1;
				const TempId tested = which == 0 ? iv.value : iv.next;
				for (unsigned side = 0; side < 2 && !guarded; ++side)
				{
					if (!(cmp.ops[side].is(ValueKind::Temp) && cmp.ops[side].index == tested) ||
						!invariant(loop, cmp.ops[1 - side]))
						continue;
					ComparePredicate pred = side == 0 ? static_cast<ComparePredicate>(cmp.sub)
													  : swapped(static_cast<ComparePredicate>(cmp.sub));
					if (!continue_on_true)
						pred = inverted(pred);
					guarded = pred == (is_signed ? ComparePredicate::Lt : ComparePredicate::Ult);
				}
			}
			if (!guarded)
				continue;
			body_.instructions[step].flags |= IF_NO_WRAP;
			any = true;
		}
	}
	return any;
}

bool hoist_loop_loads(FunctionContext& context)
{
	LoopPass pass(context);
	return pass.hoist_loads();
}

bool version_fill_loops(FunctionContext& context)
{
	LoopPass pass(context);
	const bool changed = pass.version_fills();
	if (changed)
		context.invalidate_cfg();
	return changed;
}

bool mark_no_wrap_counters(FunctionContext& context)
{
	LoopPass pass(context);
	return pass.mark_no_wrap();
}

bool unroll_loops(FunctionContext& context)
{
	// At most kUnrollRounds loops, one at a time, each found again after
	// the previous change (LoopPass::unroll).
	LoopPass pass(context);
	const bool changed = pass.unroll();
	if (changed)
		context.invalidate_cfg();
	return changed;
}

bool remove_loops(FunctionContext& context)
{
	bool any = false;
	for (unsigned round = 0; round < 8; ++round)
	{
		LoopPass pass(context);
		if (!pass.run(round == 0))
			break;
		context.invalidate_cfg();
		any = true;
	}
	return any;
}

} // namespace opt
