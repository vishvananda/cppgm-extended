#include "opt/optimizer.h"

#include <chrono>
#include <cstdio>
#include <cstdlib>
#include <sstream>

#include "lowir/builder.h"
#include "lowir/text_writer.h"
#include "lowir/validator.h"

#include "opt/inliner.h"
#include "opt/bit_recurrences.h"
#include "opt/interprocedural.h"
#include "opt/passes.h"

namespace opt {

using namespace lowir;

namespace {

// Scalar rounds per function: each runs propagation, simplification,
// control-flow cleanup and dead-code removal once (each linear); the loop
// stops at the first round that changes nothing.
const unsigned kScalarRounds = 4;
// Select formation rounds per function (nested merges convert inside out).
const unsigned kHoistRounds = 4;
const unsigned kSelectRounds = 3;

void count_program(const Program& program, uint64_t& instructions, uint64_t& blocks)
{
	instructions = blocks = 0;
	for (const Function& function : program.functions)
	{
		if (function.body == nullptr)
			continue;
		instructions += instruction_count(*function.body);
		blocks += function.body->layout.size();
	}
}

void scalar_rounds(FunctionContext& context);

// A scalar temporary passed to a by_address (or indirect_result) parameter
// stands for storage holding its value (lowir.md: such an argument denotes
// addressable storage), which the back end makes for the call. Propagation
// would replace the temporary by a literal, which the call then passes as
// the pointer itself, so before any pass each such argument becomes the
// storage it stands for: a slot of the temporary's type, stored just
// before the call, whose address the call passes. Likewise, scalar slot
// value operands become explicit loads before SSA and memory analyses.
void materialize_storage_arguments(Program& program, FunctionBody& body)
{
	for (BlockId b : body.layout)
		for (InstId i = body.blocks[b].first, prev = kNoId; i != kNoId; prev = i, i = body.instructions[i].next)
		{
			if (body.instructions[i].op == Opcode::Asm)
			{
				const uint32_t count = body.instructions[i].extra_count;
				const uint32_t first = body.instructions[i].extra_first;
				for (uint32_t k = 0; k < count; ++k)
				{
					const Value value = body.operands[first + k];
					if (value.is(ValueKind::Slot))
						body.operands[first + k] = materialize_slot_read(
							body, b, prev, value, body.slots[value.index].type, body.instructions[i].location);
				}
			}
			else if (body.instructions[i].op == Opcode::Call)
				materialize_call_arguments(program, body, b, prev, i);
			else if (body.instructions[i].ops[0].is(ValueKind::Slot) ||
				body.instructions[i].ops[1].is(ValueKind::Slot) || body.instructions[i].ops[2].is(ValueKind::Slot))
				materialize_slot_values(body, b, prev, i);
		}
}

void materialize_storage_arguments(Program& program)
{
	for (const Function& function : program.functions)
		if (function.body != nullptr)
			materialize_storage_arguments(program, *function.body);
}

// A switch compares its cases at its selector's type (the back end's
// rule), which a literal replacing the selector would lose: the type is
// recorded in the instruction for the passes that fold it (text spells
// none; a literal selector compares as i64).
void record_switch_types(FunctionBody& body)
{
	for (BlockId b : body.layout)
	{
		const InstId t = body.blocks[b].last;
		if (t == kNoId)
			continue;
		Instruction& term = body.instructions[t];
		if (term.op == Opcode::Switch && term.type.is_void() && term.ops[0].is(ValueKind::Temp))
			term.type = body.temps[term.ops[0].index].type;
	}
}

// OptimizerOptions::verify_passes: the whole program is validated after
// every pass of every function; the first pass leaving it invalid is
// named on standard error and the process stops (exit status 3). With a
// dump path (--verify-passes=<path>) the program is written there instead
// and the run continues; that mode also reports a use of a temporary whose
// definition is no longer linked into the layout, which the validator
// accepts.
bool unlinked_definition_used(const FunctionContext& context)
{
	const FunctionBody& body = context.body;
	std::vector<BlockId> block_of;
	map_instruction_blocks(body, block_of);
	bool found = false;
	for (BlockId b : body.layout)
		for (InstId i = body.blocks[b].first; i != kNoId && !found; i = body.instructions[i].next)
			for_each_operand(body, body.instructions[i], [&](const Value& v) {
				if (v.is(ValueKind::Temp) && v.index >= body.parameter_count)
				{
					const InstId def = body.temps[v.index].def;
					found = found || def == kNoId || def >= block_of.size() || block_of[def] == kNoId;
				}
			});
	return found;
}

void verify(const FunctionContext& context, const char* pass)
{
	if (!context.verify)
		return;
	// Names print uniquely once a body is compacted (compact_body), not
	// between passes: the structure and types are checked here.
	lowir::Diagnostic diagnostic;
	const char* dump = context.verify_dump;
	if (lowir::validate_constructed_program(context.program, diagnostic, false))
	{
		if (dump == nullptr || !unlinked_definition_used(context))
			return;
		std::fprintf(stderr, "lowiropt verify: unlinked definition used after %s in @%s\n", pass,
			context.program.spelling(context.program.symbols[context.symbol].name));
		lowir::write_program_file(context.program, dump);
		return;
	}
	std::fprintf(stderr, "lowiropt verify: invalid after %s in @%s: %s\n", pass,
		context.program.spelling(context.program.symbols[context.symbol].name),
		lowir::describe(context.program, diagnostic).c_str());
	if (dump != nullptr)
	{
		lowir::write_program_file(context.program, dump);
		return;
	}
	std::exit(3);
}

// OptimizerOptions::verify_passes: the instruction blocks and use counts a
// pass left for the next one at this version (FunctionContext::entry_block_of,
// entry_uses) must be what a fresh computation gives; else the pass named
// edited the body without reporting it, and the process stops (exit
// status 3).
void verify_kept_analyses(const FunctionContext& context, const char* pass)
{
	const FunctionBody& body = context.body;
	bool stale = false;
	if (context.block_of_version == context.version && context.block_of_size == body.instructions.size())
	{
		std::vector<BlockId> fresh;
		map_instruction_blocks(body, fresh);
		stale = fresh != context.block_of;
	}
	if (context.uses_version == context.version && context.uses_size == body.temps.size())
	{
		std::vector<uint32_t> fresh;
		count_uses(body, fresh);
		stale = stale || fresh != context.uses;
	}
	if (!stale)
		return;
	std::fprintf(stderr, "lowiropt verify: stale block map or use counts after %s in @%s\n", pass,
		context.program.spelling(context.program.symbols[context.symbol].name));
	std::exit(3);
}

void enable_verify(FunctionContext& context, const OptimizerOptions& options)
{
	context.verify = options.verify_passes;
	context.verify_dump = options.verify_dump.empty() ? nullptr : options.verify_dump.c_str();
}

// A pass, unless it found nothing to do at the body's current version (it
// would find nothing again: the version moves with every change).
bool run_pass(FunctionContext& context, bool (*pass)(FunctionContext&), const char* name)
{
	for (const FunctionContext::Settled& s : context.settled)
		if (s.pass == pass && s.version == context.version)
			return s.reported;
	context.kept_body = nullptr;
	context.running = pass;
	context.left_ordered = false;
	const bool was_ordered = context.ordered_version == context.version;
	const bool changed = pass(context);
	context.running = nullptr;
	verify(context, name);
	if (changed && context.kept_body != pass)
	{
		++context.version;
		// Known in order: the pass ended with restore_definition_order, or
		// only removed instructions or replaced operands with literals.
		if (context.left_ordered || (was_ordered && (pass == remove_dead_code || pass == propagate_constants)))
			context.ordered_version = context.version;
		return true;
	}
	if (context.verify)
		verify_kept_analyses(context, name);
	for (FunctionContext::Settled& s : context.settled)
		if (s.pass == pass)
		{
			s.version = context.version;
			s.reported = changed;
			return changed;
		}
	context.settled.push_back(FunctionContext::Settled{ pass, context.version, changed });
	return changed;
}

#define RUN_PASS(pass) run_pass(context, pass, #pass)

// A function context's containers, kept from one function to the next
// for their capacity: every CFG array is rebuilt before it is read (the
// context starts with no CFG), the kept analyses start invalid, and the
// definition-order stamps keep rising.
struct ContextPool
{
	Cfg cfg;
	std::vector<uint32_t> passed, uses;
	uint32_t passed_stamp = 0;
	std::vector<BlockId> block_of;
};

class PooledContext
{
public:
	PooledContext(FunctionContext& context, ContextPool& pool) : context_(context), pool_(pool) { exchange(); }
	~PooledContext() { exchange(); }

private:
	void exchange()
	{
		std::swap(context_.cfg, pool_.cfg);
		context_.passed.swap(pool_.passed);
		std::swap(context_.passed_stamp, pool_.passed_stamp);
		context_.uses.swap(pool_.uses);
		context_.block_of.swap(pool_.block_of);
	}
	FunctionContext& context_;
	ContextPool& pool_;
};

ContextPool& context_pool()
{
	static ContextPool pool;
	return pool;
}

// One function's local pipeline: scalar rounds, slot promotion, and
// scalar rounds again over what promotion exposed; the body is then
// compacted so a caller copies only live code.
void simplify_function(Program& program, SymbolId symbol, OptimizerStats& stats, const OptimizerOptions& options,
	std::vector<uint8_t>& nounwind, const std::vector<uint8_t>& unwritten, uint32_t& return_table_names)
{
	const int level = options.level;
	const bool addressed = !program.functions[program.symbols[symbol].detail].body->code_labels.empty();
	FunctionContext context(program, symbol, stats, level);
	const PooledContext pooled(context, context_pool());
	context.nounwind = &nounwind;
	context.unwritten = &unwritten;
	context.return_table_names = &return_table_names;
	enable_verify(context, options);
	verify(context, "inlining");
	record_switch_types(context.body);
	if (addressed)
	{
		// Preserve every code entry. These passes neither clone entries nor
		// require copies on an indirect edge; CFG cleanup retains addressed blocks.
		context.level = 1;
		for (unsigned round = 0; round < kScalarRounds; ++round)
		{
			context.level = level;
			bool changed = RUN_PASS(split_aggregate_slots);
			// Scalar/value passes at O2 preserve addressed entries; reserve
			// graph-forming O3 rewrites for the ordinary pipeline.
			context.level = level >= 2 ? 2 : 1;
			changed = RUN_PASS(promote_slots) || changed;
			changed = RUN_PASS(propagate_constants) || changed;
			changed = RUN_PASS(simplify_values) || changed;
			changed = RUN_PASS(forward_memory) || changed;
			changed = RUN_PASS(remove_dead_code) || changed;
			changed = RUN_PASS(cleanup_cfg) || changed;
			if (!changed)
				break;
		}
		compact_body(program, context.body);
		return;
	}

	// Scalar slots first: the lowering keeps locals in slots, so scalar
	// rounds see their values once promoted. An object slot splits best
	// after a round has simplified the copies that fill it, so a body with
	// one has its round first. A split exposes fields whose copies may
	// split another slot, and a promotion addresses that only flowed
	// through a slot (once a round has propagated them or removed the uses
	// that kept it): another attempt follows each round while slots remain.
	// At least one round of scalar passes runs; a later one only after
	// another change.
	bool object_slots = false;
	for (const Slot& slot : context.body.slots)
		object_slots = object_slots || slot.type.is_object();
	if (object_slots)
		scalar_rounds(context);
	for (unsigned round = object_slots ? 1 : 0; round < 4; ++round)
	{
		bool changed = RUN_PASS(split_aggregate_slots);
		if (changed)
			RUN_PASS(remove_dead_code);
		changed = RUN_PASS(promote_slots) || changed;
		if (!changed && round != 0)
			break;
		scalar_rounds(context);
		if (context.body.slots.empty())
			break;
	}
	bool again = RUN_PASS(strip_dead_regions);
	again = RUN_PASS(coalesce_copies) || again;
	if (again)
	{
		// A slot a forwarded copy left with field accesses only splits now.
		if (!context.body.slots.empty() && RUN_PASS(split_aggregate_slots))
			RUN_PASS(remove_dead_code);
		if (!context.body.slots.empty())
			RUN_PASS(promote_slots);
		scalar_rounds(context);
	}
	if (level >= 2 && RUN_PASS(eliminate_tail_recursion))
		scalar_rounds(context);
	again = RUN_PASS(remove_loops);
	if (level >= 3 && RUN_PASS(unroll_loops))
		again = true;
	if (level >= 2 && RUN_PASS(form_switches))
		again = true;
	if (again)
		scalar_rounds(context);
	if (nounwind.size() <= symbol)
		nounwind.resize(program.symbols.size(), 0); // a folded copy made after the start
	nounwind[symbol] = proves_nounwind(context) ? 1 : 0;
	if (level >= 2 && RUN_PASS(name_returned_slot))
		scalar_rounds(context);
	if (level >= 2)
	{
		RUN_PASS(split_or_branches);
		RUN_PASS(specialize_phi_compares);
		RUN_PASS(split_branch_compares);
	}
	if (RUN_PASS(form_phi_result_tables))
		scalar_rounds(context);
	if (level >= 2 && RUN_PASS(form_return_tables))
		scalar_rounds(context);
	compact_body(program, context.body);
}

// O2+, after all inlining (a select is smaller than the branches it
// replaces, and an i128 operation than its halves, which the inliner's
// sizes would see): i128 values as halves, then selects, each round's
// cleanup dropping the arms and merging what the next round may convert,
// then their compares split beside them.
// A loop is a back edge of the reverse postorder. Without one, the loop passes
// each build a forest to find nothing.
bool has_loop(FunctionContext& context)
{
	Cfg& cfg = context.ensure_cfg_light();
	std::vector<uint32_t> order(cfg.block_count, 0);
	for (uint32_t r = 0; r < cfg.rpo.size(); ++r)
		order[cfg.rpo[r]] = r;
	for (BlockId b : cfg.rpo)
		for (uint32_t k = cfg.succ_first[b]; k < cfg.succ_first[b + 1]; ++k)
			if (order[cfg.succs[k]] <= order[b])
				return true;
	return false;
}

bool calls_known_memory(const FunctionContext& context)
{
	const Program& program = context.program;
	const FunctionBody& body = context.body;
	for (BlockId b : body.layout)
		for (InstId i = body.blocks[b].first; i != kNoId; i = body.instructions[i].next)
		{
			const Instruction& inst = body.instructions[i];
			if (inst.op == Opcode::Call && inst.ops[0].is(ValueKind::Symbol))
			{
				const SymbolId s = inst.ops[0].index;
				if ((s < program.read_only_functions.size() && program.read_only_functions[s] != 0) ||
					(s < program.function_writes.size() && program.function_writes[s].bytes != 0))
					return true;
			}
		}
	return false;
}

// The loop passes of the final pass.
// Code motion out of loops: a load through a pointer whose arithmetic just
// left the loop follows it, then the arithmetic on what it loads.
bool hoist_loops(FunctionContext& context)
{
	bool changed = RUN_PASS(hoist_loop_loads);
	for (unsigned round = 0; round < kHoistRounds && RUN_PASS(hoist_loop_invariants); ++round)
	{
		changed = true;
		if (!RUN_PASS(hoist_loop_loads))
			break;
	}
	return changed;
}

bool finish_loops(FunctionContext& context, int level)
{
	bool changed = false;
	changed = RUN_PASS(hoist_loop_loads) || changed;
	changed = hoist_loops(context) || changed;
	changed = RUN_PASS(version_fill_loops) || changed;
	if (level >= 3 && RUN_PASS(merge_loop_latches))
	{
		RUN_PASS(remove_loops);
		RUN_PASS(remove_dead_code);
		changed = true;
	}
	// (Last: the counters' facts hold for the instructions as they stand.)
	changed = RUN_PASS(mark_no_wrap_counters) || changed;
	// O3: rotated loops, what each pass loads anew and what the latch just
	// read meeting at the new header, where the preheader may load it once.
	if (level >= 3 && RUN_PASS(merge_loop_entries))
		RUN_PASS(simplify_values);
	if (level >= 3 && RUN_PASS(rotate_loops))
	{
		RUN_PASS(promote_slots);
		// (Latches now test what their headers read. A guard of literals goes.)
		RUN_PASS(fold_nonzero_tests);
		RUN_PASS(simplify_values);
		RUN_PASS(cleanup_cfg);
		RUN_PASS(forward_memory);
		RUN_PASS(remove_dead_code);
		hoist_loops(context);
		changed = true;
	}
	// O3: a loop per value of a test it need not repeat, then what each copy's
	// straight code leaves.
	if (level >= 3 && RUN_PASS(unswitch_loops))
	{
		RUN_PASS(promote_slots);
		RUN_PASS(cleanup_cfg);
		RUN_PASS(simplify_values);
		RUN_PASS(forward_memory);
		RUN_PASS(remove_dead_code);
		RUN_PASS(cleanup_cfg);
		RUN_PASS(hoist_loop_invariants);
		hoist_loops(context);
		changed = true;
	}
	// O3: what each pass of a loop stores stays in a register; the exit's
	// store then reaches the loop around it.
	if (level >= 3 && RUN_PASS(promote_loop_memory))
	{
		RUN_PASS(promote_slots);
		RUN_PASS(forward_memory);
		RUN_PASS(remove_dead_code);
		RUN_PASS(cleanup_cfg);
		changed = true;
	}
	// O3: swapped nests, then what their new inner loops leave behind.
	if (level >= 3 && RUN_PASS(interchange_loops))
	{
		// (A load's address may be arithmetic the swap made invariant.)
		RUN_PASS(hoist_loop_invariants);
		hoist_loops(context);
		changed = true;
	}
	if (level >= 3 && RUN_PASS(cache_loop_loads))
	{
		RUN_PASS(remove_dead_code);
		RUN_PASS(simplify_values);
		changed = true;
	}
	// O3: loops in lanes, then what their guards and exits leave.
	if (level >= 3 && RUN_PASS(vectorize_loops))
	{
		RUN_PASS(simplify_values);
		RUN_PASS(cleanup_cfg);
		RUN_PASS(remove_dead_code);
		changed = true;
	}
	// O2: products of a counter, after everything that reads a loop's shape.
	if (RUN_PASS(reduce_strength))
	{
		RUN_PASS(remove_dead_code);
		changed = true;
	}
	if (RUN_PASS(widen_counters))
	{
		RUN_PASS(remove_dead_code);
		changed = true;
	}
	if (RUN_PASS(sink_counter_steps))
		changed = true;
	return changed;
}

// Whether an instruction of `body` names a symbol `marked` marks.
bool names_symbol(const FunctionBody& body, const std::vector<uint8_t>& marked)
{
	for (BlockId b : body.layout)
		for (InstId i = body.blocks[b].first; i != kNoId; i = body.instructions[i].next)
		{
			bool found = false;
			for_each_operand(body, body.instructions[i], [&](const Value& v) {
				found = found || (v.is(ValueKind::Symbol) && v.index < marked.size() && marked[v.index] != 0);
			});
			if (found)
				return true;
		}
	return false;
}

void finish_function(Program& program, SymbolId symbol, OptimizerStats& stats, const OptimizerOptions& options,
	std::vector<uint8_t>& nounwind, std::vector<uint8_t>& unwritten, const std::vector<uint8_t>* moved,
	BitTablePool& bit_tables)
{
	if (!program.functions[program.symbols[symbol].detail].body->code_labels.empty())
		return;
	FunctionContext context(program, symbol, stats, 2);
	const PooledContext pooled(context, context_pool());
	context.nounwind = &nounwind;
	context.unwritten = &unwritten;
	context.bit_tables = options.level >= 3 ? &bit_tables : nullptr;
	enable_verify(context, options);
	// What a call's newly proved memory footprint no longer hides, and the
	// loads of globals the late static initialization gave data (and the
	// addresses of the stores it removed).
	bool changed = calls_known_memory(context) && RUN_PASS(forward_memory);
	if (moved != nullptr && names_symbol(context.body, *moved))
	{
		RUN_PASS(simplify_values);
		changed = true;
	}
	if (changed)
		RUN_PASS(remove_dead_code);
	changed = (options.has_gnu_sincos && RUN_PASS(combine_sin_cos)) || changed;
	changed = (options.level >= 2 && RUN_PASS(guard_sqrt_calls)) || changed;
	// i128 values as halves once the inliner has sized its callees whole
	// (a variable shift is a dozen i64 instructions): the halves' slots
	// promoted and a round over what that exposes.
	if (RUN_PASS(expand_wide_integers))
	{
		RUN_PASS(promote_slots);
		scalar_rounds(context);
		changed = true;
	}
	for (unsigned round = 0; round < kSelectRounds && RUN_PASS(form_selects); ++round)
	{
		RUN_PASS(cleanup_cfg);
		changed = true;
	}
	const bool had_loops = has_loop(context);
	if (had_loops)
		changed = finish_loops(context, options.level) || changed;
	if (options.level >= 3 && RUN_PASS(fold_bit_recurrences))
	{
		nounwind.resize(program.symbols.size(), 0);
		unwritten.resize(program.symbols.size(), 0);
		scalar_rounds(context);
		// Newly introduced lookup addresses can now leave their loops.
		RUN_PASS(hoist_loop_invariants);
		changed = true;
	}
	if (options.level >= 3 && RUN_PASS(pack_block_lanes))
	{
		RUN_PASS(simplify_values);
		RUN_PASS(remove_dead_code);
		// Packing creates lane splats after ordinary loop motion has run.
		if (had_loops)
			RUN_PASS(hoist_loop_invariants);
		changed = true;
	}
	changed = RUN_PASS(sink_values) || changed;
	changed = RUN_PASS(schedule_integer_values) || changed;
	changed = RUN_PASS(hold_loop_literals) || changed;
	changed = RUN_PASS(issue_long_latency) || changed;
	if (!changed)
		return;
	RUN_PASS(split_branch_compares);
	compact_body(program, context.body);
}

void scalar_rounds(FunctionContext& context)
{
	// A one-block body has no control flow to propagate along or clean.
	if (context.body.layout.size() == 1)
	{
		for (unsigned round = 0; round < kScalarRounds; ++round)
		{
			bool changed = RUN_PASS(simplify_values);
			changed = RUN_PASS(forward_memory) || changed;
			changed = RUN_PASS(remove_dead_code) || changed;
			if (!changed)
				break;
		}
		// Code after a noreturn call still goes.
		if (RUN_PASS(cleanup_cfg) && context.body.layout.size() == 1)
			RUN_PASS(remove_dead_code);
		if (context.body.layout.size() == 1)
			return;
	}
	for (unsigned round = 0; round < kScalarRounds; ++round)
	{
		bool changed = RUN_PASS(propagate_constants);
		changed = RUN_PASS(cleanup_cfg) || changed;
		changed = RUN_PASS(simplify_values) || changed;
		changed = RUN_PASS(forward_memory) || changed;
		changed = RUN_PASS(remove_dead_code) || changed;
		changed = RUN_PASS(thread_jumps) || changed;
		changed = RUN_PASS(cleanup_cfg) || changed;
		if (!changed)
			break;
	}
	if (context.ordered_version != context.version)
	{
		if (restore_definition_order(context))
			++context.version;
		else
			context.ordered_version = context.version;
	}
	verify(context, "restore_definition_order");
}

} // namespace

void materialize_call_arguments(Program& program, FunctionBody& body, BlockId b, InstId& prev, InstId call_id)
{
	const Signature* sig = program.call_boundary(body.instructions[call_id]);
	if (sig == nullptr)
		return;
	const Parameter* params = program.parameters_of(*sig);
	for (uint32_t k = 0; k < body.instructions[call_id].extra_count; ++k)
	{
		const Value v = body.operands[body.instructions[call_id].extra_first + k];
		if (v.is(ValueKind::Slot) && !body.slots[v.index].type.is_object() &&
			(k >= sig->parameter_count || params[k].pass == ir_model::PPM_DIRECT))
		{
			const Type type = body.slots[v.index].type;
			const uint32_t location = body.instructions[call_id].location;
			const Value loaded = materialize_slot_read(body, b, prev, v, type, location);
			body.operands[body.instructions[call_id].extra_first + k] = loaded;
			continue;
		}
		if (k >= sig->parameter_count || params[k].pass == ir_model::PPM_DIRECT || !v.is(ValueKind::Temp))
			continue;
		const Type type = body.temps[v.index].type;
		if (type.is_pointer() || type.is_object())
			continue;
		const uint32_t location = body.instructions[call_id].location;
		const SlotId slot = static_cast<SlotId>(body.slots.size());
		body.slots.push_back(Slot{ 0, SlotKind::Refarg, 0, 0, type });
		Instruction addr = make_instruction(Opcode::Addr, Type::scalar(TypeKind::Ptr));
		addr.ops[0] = Value::slot(slot);
		addr.location = location;
		const InstId a = append_instruction(body, addr);
		body.instructions[a].result = append_temp(body, Type::scalar(TypeKind::Ptr), a);
		insert_after(body, b, prev, a);
		Instruction store = make_instruction(Opcode::Store, type);
		store.ops[0] = v;
		store.ops[1] = Value::temp(body.instructions[a].result);
		store.location = location;
		const InstId st = append_instruction(body, store);
		insert_after(body, b, a, st);
		prev = st;
		body.operands[body.instructions[call_id].extra_first + k] = Value::temp(body.instructions[a].result);
	}
}

void inline_source_required(Program& program)
{
	OptimizerStats stats;
	for (SymbolId symbol : program.order)
		if (program.symbols[symbol].kind == SymbolKind::FunctionDefinition &&
			(program.symbols[symbol].metadata.flags & SF_FORCE_INLINE) != 0 &&
			program.functions[program.symbols[symbol].detail].body != nullptr)
		{
			FunctionBody& body = *program.functions[program.symbols[symbol].detail].body;
			bool allocates = false;
			for (BlockId block : body.layout)
				for (InstId i = body.blocks[block].first; i != kNoId; i = body.instructions[i].next)
					allocates = allocates || body.instructions[i].op == Opcode::StackAlloc;
			if (allocates && body.code_labels.empty())
			{
				FunctionContext context(program, symbol, stats, 0);
				const PooledContext pooled(context, context_pool());
				propagate_constants(context);
				materialize_fixed_allocations(program, body);
				scope_dynamic_allocations(body);
			}
		}
	const std::function<void(SymbolId)> simplify = [&](SymbolId symbol) {
		if (!program.functions[program.symbols[symbol].detail].body->code_labels.empty())
			return;
		FunctionContext context(program, symbol, stats, 0);
		const PooledContext pooled(context, context_pool());
		// Preserve named storage in full debug bodies.
		if (program.debug == nullptr && (program.symbols[symbol].metadata.flags & SF_FORCE_INLINE) != 0)
		{
			materialize_storage_arguments(program, context.body);
			promote_slots(context);
			remove_dead_code(context);
			strip_dead_regions(context);
		}
		// Fold template-dependent constant conditions before required
		// substitution classifies recursion. Ordinary O0 bodies stay as lowered.
		if ((program.symbols[symbol].metadata.flags & SF_FORCE_INLINE) != 0)
			propagate_constants(context);
		// Inlining turns a return after a noreturn call into a spurious
		// continuation edge. Remove it before EH stacks meet at that join.
		cleanup_cfg(context);
	};
	run_source_inline_wave(program, simplify);
}

bool parse_inline_limit(const std::string& text, InlineLimits& limits)
{
	const size_t equals = text.find('=');
	if (equals == std::string::npos || equals + 1 >= text.size())
		return false;
	const std::string name = text.substr(0, equals);
	const std::string value = text.substr(equals + 1);
	for (char c : value)
		if (c < '0' || c > '9')
			return false;
	const unsigned long parsed = std::strtoul(value.c_str(), nullptr, 10);
	if (parsed == 0 || parsed > 0xffffffffu)
		return false;
	const uint32_t n = static_cast<uint32_t>(parsed);
	if (name == "caller-budget")
		limits.caller_budget = n;
	else if (name == "once-cap")
		limits.once_cap = n;
	else if (name == "once-caller-budget")
		limits.once_caller_budget = n;
	else if (name == "hint-late-cap")
		limits.hint_late_cap = n;
	else
		return false;
	return true;
}

void optimize_program(Program& program, const OptimizerOptions& options, OptimizerStats* stats_out)
{
	OptimizerStats local;
	OptimizerStats& stats = stats_out != nullptr ? *stats_out : local;
	const std::chrono::steady_clock::time_point start = std::chrono::steady_clock::now();
	count_program(program, stats.instructions_in, stats.blocks_in);
	if (options.level > 0)
	{
		program.read_only_functions.clear();
		program.function_writes.clear();
		find_allocation_functions(program);
		std::vector<uint32_t> original(program.symbols.size(), 0);
		for (SymbolId s = 0; s < program.symbols.size(); ++s)
		{
			const Symbol& symbol = program.symbols[s];
			if (symbol.kind == SymbolKind::FunctionDefinition && program.functions[symbol.detail].body != nullptr)
			{
				original[s] = static_cast<uint32_t>(instruction_count(*program.functions[symbol.detail].body));
				++stats.functions;
			}
		}
		materialize_storage_arguments(program);
		const int level = options.level;
		std::vector<uint8_t> nounwind(program.symbols.size(), 0);
		std::vector<uint8_t> unwritten, moved;
		initialize_statically(program, unwritten);
		uint32_t return_table_names = 0;
		const std::function<void(SymbolId)> simplify = [&](SymbolId s) {
			simplify_function(program, s, stats, options, nounwind, unwritten, return_table_names);
		};
		// Pipeline-wide growth: each inlining policy copies at most the
		// unit's input size (at least 10,240 instructions) in total, the
		// ordinary policies twice that (template layers, as in tramp3d, are
		// mostly small accessors: past the budget each stays a call, and a
		// unit that runs out spends five times the instructions).
		InlineBudgets budgets;
		budgets.once_unit = stats.instructions_in > 10240 ? stats.instructions_in : 10240;
		budgets.force_unit = budgets.once_unit;
		budgets.growth_unit = 2 * budgets.once_unit;
		run_inline_wave(program, options, stats, InlineWave::Ordinary, budgets, original, nounwind, simplify);
		prune_unreachable(program, stats);
		if (level >= 2)
			speculate_virtual_calls(program, stats);
		if (level >= 2 && copy_for_callees(program, stats))
		{
			original.resize(program.symbols.size(), 0);
			nounwind.resize(program.symbols.size(), 0);
		}
		run_inline_wave(program, options, stats, InlineWave::Once, budgets, original, nounwind, simplify);
		prune_unreachable(program, stats);
		run_inline_wave(program, options, stats, InlineWave::Late, budgets, original, nounwind, simplify);
		if (level >= 2)
			forward_calls(program, stats);
		prune_unreachable(program, stats);
		if (level >= 2 && specialize_arguments(program, stats, simplify))
			prune_unreachable(program, stats);
		if (level >= 2)
			infer_function_memory(program);
		// The waves inlined the constructors the init function calls: what
		// they store is now stores the final pass's loads can read.
		const bool moved_late = level >= 2 && initialize_statically(program, unwritten, &moved);
		BitTablePool bit_tables;
		for (SymbolId s = 0; level >= 2 && s < program.symbols.size(); ++s)
			if (program.symbols[s].kind == SymbolKind::FunctionDefinition &&
				program.functions[program.symbols[s].detail].body != nullptr)
				finish_function(
					program, s, stats, options, nounwind, unwritten, moved_late ? &moved : nullptr, bit_tables);
	}

	if (options.level == 1)
		for (SymbolId symbol : program.order)
			if (program.symbols[symbol].kind == SymbolKind::FunctionDefinition &&
				program.functions[program.symbols[symbol].detail].body != nullptr &&
				program.functions[program.symbols[symbol].detail].body->code_labels.empty())
			{
				FunctionContext context(program, symbol, stats, options.level);
				const PooledContext pooled(context, context_pool());
				enable_verify(context, options);
				bool changed = false;
				for (unsigned round = 0; round < kSelectRounds && RUN_PASS(form_selects); ++round)
				{
					RUN_PASS(cleanup_cfg);
					changed = true;
				}
				if (changed)
					scalar_rounds(context);
				changed = (has_loop(context) && RUN_PASS(sink_counter_steps)) || changed;
				if (changed)
					compact_body(program, context.body);
			}
	count_program(program, stats.instructions_out, stats.blocks_out);
	stats.optimize_ms = std::chrono::duration<double, std::milli>(std::chrono::steady_clock::now() - start).count();
}

std::string describe_stats(const OptimizerStats& s)
{
	std::ostringstream out;
#if CPPGM_TELEMETRY_ENABLED
	out << "pa33_opt_stats functions=" << s.functions << " instructions_in=" << s.instructions_in
		<< " instructions_out=" << s.instructions_out << " blocks_in=" << s.blocks_in << " blocks_out=" << s.blocks_out
		<< " folded=" << s.folded << " propagated=" << s.propagated << " reused=" << s.reused << " dead=" << s.dead
		<< " branches_folded=" << s.branches_folded << " blocks_removed=" << s.blocks_removed
		<< " blocks_merged=" << s.blocks_merged << " slots_promoted=" << s.slots_promoted
		<< " phis_inserted=" << s.phis_inserted << " calls_inlined=" << s.calls_inlined
		<< " calls_forwarded=" << s.calls_forwarded << " inlined_instructions=" << s.inlined_instructions
		<< " functions_pruned=" << s.functions_pruned << " loads_reused=" << s.loads_reused
		<< " stores_removed=" << s.stores_removed << " loops_removed=" << s.loops_removed
		<< " loops_unrolled=" << s.loops_unrolled << " bit_recurrences_folded=" << s.bit_recurrences_folded
		<< " bit_table_bytes=" << s.bit_table_bytes << " invariants_hoisted=" << s.invariants_hoisted
		<< " stores_sunk=" << s.stores_sunk << " loops_vectorized=" << s.loops_vectorized
		<< " loops_interchanged=" << s.loops_interchanged << " loops_unswitched=" << s.loops_unswitched
		<< " counters_shifted=" << s.counters_shifted << " counters_widened=" << s.counters_widened
		<< " multiplies_reduced=" << s.multiplies_reduced << " tail_calls_removed=" << s.tail_calls_removed
		<< " math_calls_shared=" << s.math_calls_shared << " specialized=" << s.specialized
		<< " calls_speculated=" << s.calls_speculated << " optimize_ms=" << s.optimize_ms;
#else
	(void)s;
#endif
	return out.str();
}

} // namespace opt
