#pragma once

#include <cstdint>
#include <vector>

#include "lowir/program.h"
#include "machine/exception_analysis.h"
#include "native/mir/mir.h"
#include "native/select/allocation_structures.h"
#include "native/select/call_abi.h"
#include "native/select/frame_layout.h"
#include "native/select/loop_split.h"
#include "native/select/selector_facts.h"
#include "support/flat_map.h"

// The data of the per-function selector (function_selector.h holds its
// phases). Vectors are cleared, not freed, between functions.
namespace native {

struct WordKeyHash
{
	size_t operator()(uint64_t key) const { return static_cast<size_t>(key); }
	size_t operator()(uint32_t key) const { return key; }
};

struct SelectorState
{
	const lowir::FunctionBody* body_ = nullptr;
	const lowir::Signature* signature_ = nullptr;
	lowir::SymbolId symbol_ = lowir::kNoId;
	Function* out_ = nullptr;

	std::vector<BlockInfo> blocks_;    // by layout position
	std::vector<uint32_t> layout_of_;  // block id -> layout position
	std::vector<uint32_t> succ_first_; // per block: its first successor
	std::vector<uint32_t> succs_;      // distinct successors (layout)
	std::vector<EdgeBlock> edges_;
	SwitchLowering switches_;                                   // switches lowered as tests, their deferred subtrees
	std::vector<uint32_t> edge_first_;                          // per block: its first edge block
	std::vector<uint32_t> edge_mir_;                            // per successor of the current block: its edge block
	std::vector<uint32_t> wide_phi_first_;                      // wide phis: start of their sorted pairs
	std::vector<std::pair<uint32_t, uint32_t>> wide_phi_pairs_; // (predecessor id, operand index)
	std::vector<TempInfo> temps_;
	std::vector<WordPair> wide_parts_;
	std::vector<WordPair> incoming_words_;
	std::vector<uint32_t> incoming_origin_;
	std::vector<lowir::Value> alias_;
	std::vector<uint8_t> fits_; // O1+ per temp: low bits its non-negative value fits in (64: unknown)
	std::vector<AddrExpr> addrs_;
	std::vector<int32_t> pos_;         // instruction -> position
	std::vector<uint32_t> inst_block_; // instruction -> layout position
	std::vector<uint8_t> marks_;
	std::vector<uint8_t> slot_state_;
	std::vector<lowir::Value> slot_value_;
	std::vector<uint32_t> slot_store_;
	std::vector<int32_t> slot_offset_;
	std::vector<lowir::Literal> extra_literals_;
	std::vector<uint32_t> block_insts_; // scratch: one block's instructions
	CallLayout entry_layout_;
	CallLayoutCache call_layouts_; // the calls' emission-time layouts (emission_layout)
	std::vector<Move> moves_;
	// Parallel-move resolution (operands.cpp): per move the pending moves
	// reading its destination, done flags, the ready heap, memory
	// destinations and sources by location, and each register's readers.
	std::vector<uint32_t> pm_readers_;
	std::vector<uint8_t> pm_done_;
	std::vector<uint32_t> pm_ready_;
	std::vector<uint32_t> pm_mem_dsts_;
	std::vector<uint32_t> pm_mem_srcs_;
	std::vector<uint32_t> pm_reg_readers_;
	std::vector<uint32_t> pm_written_;
	std::vector<Move> stack_constants_; // constant stack arguments of a call
	std::vector<lowir::Type> arg_types_;
	std::vector<uint8_t> arg_classes_; // host: their lowir::ObjectClassBits

	// Emission state.
	uint32_t owner_[32];
	uint32_t occupied_ = 0;
	uint32_t pinned_ = 0;
	uint32_t used_callee_saved_ = 0;
	uint32_t frame_size_ = 0;
	std::vector<FrameObject> frame_objects_; // frame_alloc's, for order_frame
	bool frame_pinned_ = false;              // an address formed outside its frame object
	uint32_t block_ = 0;
	int32_t position_ = 0;
	uint32_t location_ = 0;
	int32_t sret_home_ = 0;
	int32_t va_save_area_ = 0;
	uint32_t named_gprs_ = 0;
	uint32_t named_xmms_ = 0;
	uint32_t named_stack_ = 0;
	bool needs_scratch_area_ = false;
	// Registers reserved by actual frame and ABI requirements.
	uint32_t excluded_ = 0;
	std::vector<uint32_t> dying_; // freed this instruction
	// Clobber facts per instruction id (allocation.cpp).
	std::vector<uint32_t> clobber_;
	std::vector<uint32_t> early_;
	bool has_eh_ = false;
	bool has_stack_alloc_ = false;
	// Target-independent handler/action facts and reused analysis storage.
	EhModel eh_model_ = EhModel::Course;
	bool position_independent_ = false;
	int level_ = 0;
	machine::ExceptionAnalysis eh_;
	std::vector<uint32_t> pad_mir_, select_mir_;
	// Global allocation (global_allocation.cpp): linear positions, hulls,
	// the registers each block reserves for cross-block values.
	std::vector<int32_t> block_base_;
	std::vector<uint32_t> reserved_;
	std::vector<int32_t> hull_start_;
	std::vector<int32_t> hull_end_;
	// Clobbers per linear position and their range-OR structure.
	std::vector<uint32_t> clobber_at_;
	std::vector<uint32_t> clobber_prefix_;
	std::vector<uint32_t> clobber_suffix_;
	std::vector<uint32_t> chunk_or_; // sparse table over chunk ORs, level-major
	uint32_t chunk_count_ = 0;
	std::vector<uint32_t> preds_first_;
	std::vector<uint32_t> preds_;
	std::vector<uint8_t> narrow_observed_; // host: per parameter, read beyond its width
	std::vector<uint64_t> narrow_from_;    // host: per temp, the parameters its upper bits come from
	std::vector<uint8_t> byte_only_;       // O1+: per Boolean temp, every reader reads its low byte only
	std::vector<uint8_t> low_read_;        // O1+: per narrow temp, no reader reads above its width
	std::vector<uint8_t> scale_of_;        // O1+: per byte offset, the scale its indexes apply (find_scaled_offsets)
	std::vector<int32_t> shift_of_;        // O1+: per offset aliasing x, what it adds to x (empty: none)
	std::vector<uint8_t> rotate_;          // O1+: per or/xor/add result, its rotation's left count (find_rmw; 0: none)
	std::vector<uint8_t> high_product_;    // O1+: per i64 trunc, 1/2: the high half of a zext/sext product
	std::vector<uint32_t> handler_first_;  // per block: the landings its eh_try/eh_cleanup name
	std::vector<uint32_t> handlers_;
	std::vector<uint32_t> global_temps_;        // values with a hull register
	std::vector<uint32_t> block_globals_first_; // per block: hull values to bind
	std::vector<uint32_t> block_globals_;
	bool exit_block_ = false; // the current block has no successor
	// Parameter retention: per (block, integer argument register) the last
	// use of the parameter it carries before the register is clobbered; per
	// block the parameters whose incoming register is still intact at its end.
	std::vector<int32_t> param_last_;
	std::vector<uint32_t> avail_out_;
	std::vector<uint8_t> emitted_;
	uint32_t avail_in_ = 0;
	uint32_t written_ = 0;
	// O1+ function-level registers (register_allocation.cpp): the
	// emission facts (per block the assigned values present, CSR in
	// block_globals_first_; the registers of the values defined in each
	// block; those bound on entry to the current one) and the allocator's
	// slots, clobbers, exceptional predecessors, loop depths, candidates'
	// uses, ranges and weights, and pressure.
	struct GlobalEntry
	{
		uint32_t temp;
		int32_t release; // position after which the block frees it (INT32_MAX: never)
		bool live_in;    // bound on entry (else at its definition)
	};
	bool precise_globals_ = false;
	// O1+, host model, RSP fixed past the prologue (Function::omit_frame_pointer):
	// RBP is a sixth callee-saved register.
	bool rbp_free_ = false;
	// A live frame_address instruction requires the actual RBP frame.
	bool frame_addressed_ = false;
	std::vector<Operand> pm_src_at_; // parallel_moves: each move's source before redirection
	std::vector<GlobalEntry> block_entries_;
	std::vector<uint32_t> block_def_gregs_;
	uint32_t block_gregs_ = 0;
	std::vector<int32_t> ra_base_;
	std::vector<uint32_t> ra_clobber_at_;
	std::vector<uint32_t> ra_early_at_;
	RangeOr ra_clobber_;
	RangeOr ra_early_;
	PressureTree ra_gpr_pressure_;
	PressureTree ra_xmm_pressure_;
	std::vector<uint32_t> ra_exc_first_;
	std::vector<uint32_t> ra_exc_preds_;
	bool loop_facts_ready_ = false; // the current function's CFG has not changed
	std::vector<uint32_t> ra_depth_;
	std::vector<uint8_t> ra_cold_; // blocks that only lead to a throw, resume, noreturn call or cold call
	std::vector<uint32_t> ra_candidates_;
	std::vector<uint32_t> ra_use_first_;
	std::vector<std::pair<uint32_t, int32_t>> ra_uses_;
	std::vector<uint32_t> ra_range_first_;
	std::vector<uint32_t> ra_range_block_;
	std::vector<std::pair<int32_t, int32_t>> ra_ranges_;
	std::vector<uint64_t> ra_freq_;
	std::vector<uint8_t> ra_looped_; // per candidate: defined or used inside a loop
	std::vector<Reg> ra_use_hint_;   // per candidate: a return's or call argument's register
	// Per register parameter: (temp, the last slot its incoming register holds it, that register).
	struct HeldParameter
	{
		uint32_t temp;
		int32_t last;
		Reg reg;
	};
	std::vector<HeldParameter> ra_held_;
	LoopSplitter loop_splitter_;         // the natural loops; spilling around them
	std::vector<SplitMove> split_moves_; // by (from, to): the split values' edge transfers
	SplitHomes split_homes_;
	// Branch layout (branch_layout.cpp): per MIR block its chain end, then
	// its new index; scratch: one jump chain.
	std::vector<uint32_t> layout_map_;
	std::vector<uint64_t> layout_frequency_; // layout_branches: block_frequencies
	std::vector<uint32_t> layout_chain_;
	std::vector<uint8_t> layout_live_; // layout_branches: blocks the entry and the pads reach
};

} // namespace native
