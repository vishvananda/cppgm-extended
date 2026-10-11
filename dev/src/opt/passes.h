#pragma once

#include "opt/function_ir.h"
#include "opt/optimizer.h"

// The optimizer's function passes (opt/optimizer.h). Each takes the body
// of one function definition, reports whether it changed anything, and
// leaves a valid body; analyses are rebuilt by the pass that needs them.
namespace opt {

struct FunctionContext
{
	FunctionContext(lowir::Program& p, lowir::SymbolId s, OptimizerStats& st, int l)
		: program(p), symbol(s), body(*p.functions[p.symbols[s].detail].body), stats(st), level(l)
	{}

	lowir::Program& program;
	lowir::SymbolId symbol;
	lowir::FunctionBody& body;
	OptimizerStats& stats;
	int level;
	bool verify = false;               // OptimizerOptions::verify_passes
	const char* verify_dump = nullptr; // OptimizerOptions::verify_dump, or null
	// The CFG and dominator-tree facts, shared by the passes until one
	// changes the graph (terminators, blocks, handler openers): passes that
	// only rewrite values keep them; the others invalidate them.
	Cfg cfg;
	bool cfg_valid = false;
	bool cfg_light = false; // edges and order current, dominators not (ensure_cfg_light)
	Cfg& ensure_cfg()
	{
		if (!cfg_valid)
		{
			if (cfg_light)
				complete_cfg(cfg);
			else
				build_cfg(program, body, cfg);
			cfg_valid = true;
			cfg_light = false;
		}
		return cfg;
	}
	// The edges, flags and reverse postorder only, for a pass that reads no
	// dominator facts (CFG cleanup rebuilds after every step that changes
	// the graph).
	Cfg& ensure_cfg_light()
	{
		if (!cfg_valid && !cfg_light)
		{
			build_cfg_light(program, body, cfg);
			cfg_light = true;
		}
		return cfg;
	}
	// The dominator tree (built with the CFG's dominance intervals).
	const DomChildren& ensure_dom() { return ensure_cfg().tree; }
	void invalidate_cfg() { cfg_valid = cfg_light = false; }
	// Per symbol: a definition this optimization proved cannot unwind
	// (internal fact, re-derived from the same LowIR by any consumer).
	const std::vector<uint8_t>* nounwind = nullptr;
	// Per symbol: an internal global the unit never writes (find_unwritten_globals).
	const std::vector<uint8_t>* unwritten = nullptr;
	// The body's version: bumped by every pass that reports a change (and
	// by any other edit the driver makes), so a pass that left the body as
	// it was at this version is not run again until something changes,
	// its report repeated (run_pass in optimizer.cpp): each pass's result
	// depends on the body alone.
	uint32_t version = 0;
	struct Settled
	{
		bool (*pass)(FunctionContext&);
		uint32_t version;
		bool reported; // what it returned
	};
	std::vector<Settled> settled;
	// A pass that reports a change only for the driver's sake, the body as
	// it was (promote_slots naming slots nothing accesses: another round
	// follows), names itself here: run_pass keeps the version.
	bool (*kept_body)(FunctionContext&) = nullptr;
	// The pass run_pass is running (a pass another one calls sees its
	// caller here, and the version is then not yet the body's).
	bool (*running)(FunctionContext&) = nullptr;
	// A version whose body restore_definition_order found in order.
	uint32_t ordered_version = ~0u;
	// Scratch shared by passes (sized on demand).
	std::vector<uint32_t> passed; // restore_definition_order's per-instruction stamps
	uint32_t passed_stamp = 0;
	std::vector<uint32_t> uses;
	std::vector<lowir::BlockId> block_of;
	// `block_of` and `uses` as a driver-run pass found them on entry, kept
	// for the next pass at the same version (and arena sizes).
	uint32_t block_of_version = ~0u, uses_version = ~0u;
	size_t block_of_size = 0, uses_size = 0;
	bool left_ordered = false; // the last restore_definition_order left the body in order
	const std::vector<lowir::BlockId>& entry_block_of(bool (*self)(FunctionContext&), bool fresh = true)
	{
		const bool share = fresh && running == self;
		if (!share || block_of_version != version || block_of_size != body.instructions.size())
		{
			map_instruction_blocks(body, block_of);
			block_of_version = share ? version : ~0u;
			block_of_size = body.instructions.size();
		}
		return block_of;
	}
	const std::vector<uint32_t>& entry_uses(bool (*self)(FunctionContext&), bool fresh = true)
	{
		const bool share = fresh && running == self;
		if (!share || uses_version != version || uses_size != body.temps.size())
		{
			count_uses(body, uses);
			uses_version = share ? version : ~0u;
			uses_size = body.temps.size();
		}
		return uses;
	}
	void invalidate_block_of() { block_of_version = ~0u; }
	void invalidate_uses() { uses_version = ~0u; }
};

// Folding, algebraic identities, copy propagation and dominator-scoped
// reuse of pure expressions (opt/value_simplify.cpp). O(instructions)
// per run with hashed expression keys.
bool simplify_values(FunctionContext& context);

// Removes unused pure instructions, nonvolatile loads and effect-free
// calls (readnone/readonly, unwind=no, not noreturn) by a use-count
// worklist. O(instructions + operands).
bool remove_dead_code(FunctionContext& context);

// Sparse conditional constant propagation over SSA and executable edges
// (Wegman and Zadeck), then rewrites constants, folds decided branches and
// drops phi inputs of dead edges. O(instructions + edges) lattice steps.
bool propagate_constants(FunctionContext& context);

// Control-flow cleanup: decided branches and switches, unreachable blocks,
// jump-only blocks, straight-line merges, empty diamonds, conditional edges
// into `unreachable`. Preserves handler targets. O(blocks + edges) per
// round, at most kCfgRounds rounds.
bool cleanup_cfg(FunctionContext& context);

// Memory forwarding over the dominator tree: a nonvolatile load reuses
// the value an earlier load read or a store wrote at the same address and
// type when no write that may alias intervenes (facts pass only to a block
// whose one predecessor is its immediate dominator). O(instructions x
// live facts), at most 64 facts.
bool forward_memory(FunctionContext& context);

// Adjacent nonvolatile scalar load/store pairs copying a contiguous range
// between storage that cannot overlap (distinct slots or globals, noalias
// parameters) merge into one copyobj. One scan, O(instructions).
bool coalesce_copies(FunctionContext& context);

// O2+, at the end of a function's pipeline (opt/copy_coalescing.cpp): the
// slot each copied return copies whole into the indirect result becomes
// that result (named return value) when its address only reaches memory
// operations and calls' indirect results, and the result is used only in
// returning blocks after the slot's last use there. O(instructions).
bool name_returned_slot(FunctionContext& context);

// Jump threading: a branch on the value its block's only predecessor just
// tested takes the known side; a predecessor whose phi inputs decide a
// small block's branch (whose values nothing else uses) jumps to the
// decided successor. O(blocks + edges) plus bounded per-edge evaluation.
bool thread_jumps(FunctionContext& context);

// Loop removal (opt/loop_passes.cpp): a loop with one exit, tested as
// `iv != bound` on an induction variable, is bypassed when its body has
// no effect and nothing outside reads its values but the tested one (the
// bound at the exit); a top-tested loop storing one byte pattern through
// a pointer induction becomes a memset call in its preheader. One rewrite
// per CFG, at most four. O(blocks + edges + instructions) each.
bool remove_loops(FunctionContext& context);

// Loop counters that never wrap marked IF_NO_WRAP for the selector (O2+,
// last, after every transform: opt/loop_passes.cpp).
bool mark_no_wrap_counters(FunctionContext& context);

// Loop-invariant loads (O2+, after all inlining: opt/loop_passes.cpp): a
// load of an address the loop does not change, from memory nothing in the
// loop may write, moves to the preheader with its address steps.
bool hoist_loop_loads(FunctionContext& context);

// Loop rotation (O3, last, opt/loop_rotation.cpp): a header that only tests
// is copied into the latches and stays as the guard; its values go
// through slots for promote_slots to join again.
bool rotate_loops(FunctionContext& context);

// Fill loops of a run-time value whose bytes repeat call memset for long
// runs (O2+, after all inlining: opt/loop_passes.cpp).
bool version_fill_loops(FunctionContext& context);

// Switch formation (O2+, opt/switch_formation.cpp): a branch on a value
// against a literal and the blocks after it that only test that value,
// reached only from among them, become one switch on it (each outside
// target taking the values its edges carried, the largest set the
// default) when that has fewer runs of consecutive values per target than
// the tests it replaces. O(region blocks x ranges) per head.
bool form_switches(FunctionContext& context);

// O2+, after the scalar rounds (opt/select_formation.cpp): a branch to a
// diamond or triangle whose arms only compute (at most two speculatable
// integer instructions each, no load, division or floating point) becomes
// a jump, the arms' instructions hoisted before it and the merge's
// differing phi inputs (at most two) `select`s on the branch condition.
// O(blocks + merge phis) per round.
bool form_selects(FunctionContext& context);

// O2+, the last pass: a compare used only as branch conditions, also in
// other blocks than its own, is recomputed before each such branch (one
// used once moves there; opt/jump_threading.cpp).
bool split_branch_compares(FunctionContext& context);
// O2+: a branch on an expanded equality memcmp against a literal tests its
// chunks one after the other (jump_threading.cpp).
bool split_or_branches(FunctionContext& context);
// O2+: a branch on a compare of a phi gives each predecessor its own
// compare of its incoming value (jump_threading.cpp).
bool specialize_phi_compares(FunctionContext& context);

// O3 full unrolling of one counted loop per function: at most four
// passes through a straight-line body, at most 64 cloned instructions.
bool unroll_loops(FunctionContext& context);

// Promotes scalar slots accessed only by direct nonvolatile loads and
// stores of their type to SSA values: phis at iterated dominance
// frontiers (never in handler blocks), renaming over the dominator tree.
// O(instructions + edges + inserted phis).
bool promote_slots(FunctionContext& context);

// Scalar replacement of address-taken slots: a slot whose address only
// reaches nonvolatile loads, stores, copyobj and zeroinit at constant
// offsets, or a call's indirect_result argument (the call fills a
// temporary, copied field by field after it), splits into one slot per
// typed field (agreeing accesses, fillers across copied spans, at most 16
// fields); copies and zeroing expand per field (object slots; an
// address-taken scalar slot keeps its accesses).
// The new slots are then promoted. O(instructions + fields).
bool split_aggregate_slots(FunctionContext& context);

// 128-bit integers as pairs of 64-bit halves (O2+, opt/wide_integers.cpp):
// logic, addition, subtraction, shifts, extensions, truncations, compares,
// selects, phis, loads and stores of i128 work on i64 halves, and i128
// slots accessed whole become two i64 slots; other uses keep i128 values
// put together from the halves. O(instructions).
bool expand_wide_integers(FunctionContext& context);

// Whether an instruction may transfer to an active handler, using the
// proven nonthrowing direct callees.
bool can_unwind(const FunctionContext& context, const lowir::Instruction& inst);

// Removes protected regions whose normal-path instructions cannot unwind
// (their openers and the eh_ends popping them); handlers become
// unreachable. Active-region stacks propagate once over ordinary edges;
// conflicting stacks keep every region. O(instructions + edges).
bool strip_dead_regions(FunctionContext& context);

// Whether the body has no EH instruction and nothing that can unwind.
bool proves_nounwind(const FunctionContext& context);

// Reorders blocks into reverse postorder when an ordinary use precedes its
// definition in layout (after reuse across blocks); whether it did.
bool restore_definition_order(FunctionContext& context);

} // namespace opt
