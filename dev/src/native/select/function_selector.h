#pragma once

#include <cstdint>
#include <map>
#include <vector>

#include "lowir/program.h"
#include "native/mir/mir.h"
#include "native/select/call_abi.h"
#include "native/select/selector.h"
#include "native/select/selector_facts.h"
#include "native/select/selector_state.h"

// The per-function selector (its data in selector_state.h). One
// FunctionSelector is reused for every function of a program. The work is
// split by phase:
//   analysis.cpp     CFG, slot promotion, aliases, uses, folding decisions
//   allocation.cpp   clobber masks, register choice, frame homes
//   operands.cpp     operand resolution, scratch loads, parallel moves
//   emit_values.cpp  value-producing instructions
//   emit_divide.cpp  integer division and remainder (constant divisors)
//   emit_calls.cpp   calls, parameters, returns, variadic state
//   emit_control.cpp terminators, phi edge transfers, EH, block driver
//   host_eh.cpp      the host EH model's static handler records
//   host_eh_tables.cpp its chains, type table and selector translation
//   branch_layout.cpp  O1+: jump threading and fall-through branches in the MIR
//   global_allocation.cpp  O0: function-level registers (hulls)
//   register_allocation.cpp  O1+: function-level registers (live ranges)
namespace native {

class FunctionSelector : private SelectorState
{
public:
	FunctionSelector(const lowir::Program& program, SelectStats& stats);

	void select(lowir::SymbolId symbol, Function& out);
	void set_eh_model(EhModel model) { eh_model_ = model; }
	void set_optimization_level(int level) { level_ = level; }

	// ---- analysis.cpp
	void reset(lowir::SymbolId symbol, Function& out);
	void build_cfg();
	void reachable_preds();
	void analyze_slots();
	void resolve_aliases();
	void find_scaled_offsets();
	void count_uses();
	void eliminate_dead();
	void place_uses();
	void find_addresses();
	void find_folds();
	void find_rmw();
	void find_byte_booleans();
	void extend_lifetimes();
	lowir::Value resolve(lowir::Value value) const;
	lowir::Type type_of(lowir::Value value) const;
	const lowir::Literal& literal(lowir::Value value) const;
	lowir::Value add_literal(const lowir::Literal& bits);
	bool alias_compatible(lowir::Type to, lowir::Type from) const;
	bool known_boolean(uint32_t temp, unsigned depth = 0) const;
	lowir::TypeKind compare_width(const lowir::Instruction& compare) const;
	bool floating_inst(const lowir::Instruction& inst) const;
	bool sunk_compare(uint32_t inst) const;
	int32_t read_pos(uint32_t inst) const;
	bool live(uint32_t inst) const { return (marks_[inst] & (IM_DEAD | IM_REACHABLE)) == IM_REACHABLE; }
	template<typename F>
	void for_each_inst(F f);
	void note_use(lowir::Value value, uint32_t inst, uint32_t block, int32_t pos, bool value_use);
	bool address_expr(uint32_t inst, AddrExpr& expr) const;

	// ---- allocation.cpp
	void inst_clobbers(uint32_t inst, uint32_t& clobber, uint32_t& early) const;
	bool compare_scratch(const lowir::Instruction& inst) const;
	void prepass_block(uint32_t block);
	void compute_clobbers();
	void retained_uses(uint32_t block, const int32_t* first_clobber, const int32_t* first_early);

	// ---- register_allocation.cpp (O1+ function-level registers)
	void allocate_globals_precise();
	bool exceptional_edges();
	void loop_depths();
	void block_weights();
	void collect_uses(const std::vector<int32_t>& index);
	void live_ranges();
	void assign_registers();
	void install_splits(
		const std::vector<std::pair<uint32_t, uint32_t>>& split, std::vector<std::pair<uint32_t, uint32_t>>& homed);
	Reg choose_register(uint32_t c, uint32_t first, uint32_t past, Reg web, Reg tied);
	void emission_facts();
	void hint_chain(lowir::Value v, uint32_t block, Reg reg);
	bool computed_from(uint32_t g, uint32_t from) const;

	// ---- global_allocation.cpp
	void allocate_globals();
	void global_hulls(std::vector<uint32_t>& candidates);
	uint32_t range_clobbers(int32_t first, int32_t last) const;
	void layout_slots();
	Reg allocate(uint32_t temp, uint32_t exclude, Reg prefer0 = kNoReg, Reg prefer1 = kNoReg);
	void bind(uint32_t temp, Reg reg);
	void release(Reg reg);
	// Counts an allocation that found the class's registers held.
	void note_busy(uint32_t pool);
	void release_dying(int32_t pos);
	void retire_dying();
	int32_t frame_alloc(uint64_t bytes, uint32_t align);
	int32_t home_of(uint32_t temp);
	void add_binding(BindingKind kind, lowir::NameId name, int32_t offset, lowir::Type type);
	bool dies_here(uint32_t temp) const;
	bool hull_ends(uint32_t temp, int32_t pos) const;
	void note_hull_last_uses(uint32_t block);
	void finish_frame();

	// ---- operands.cpp
	SymbolAccess symbol_access(uint32_t symbol) const;
	Operand symbol_operand(uint32_t symbol, int32_t addend) const;
	Operand symbol_memory(uint32_t symbol, int32_t addend, Reg scratch);
	Operand reg_operand(lowir::Value value, lowir::Type type, Reg scratch);
	Operand rm_operand(lowir::Value value, lowir::Type type, Reg scratch);
	Operand address_operand(lowir::Value storage, Reg scratch);
	Operand lazy_operand(const AddrExpr& expr, Reg scratch, Reg spare = kNoReg);
	Reg temp_reg_or_load(uint32_t temp, Reg scratch);
	Operand temp_location(uint32_t temp, Reg scratch = kSelectScratch);
	Operand imm_for(lowir::Value value, lowir::Type type) const;
	bool fits_imm32(const Operand& op) const;
	void load_into(Reg dst, const Operand& src, lowir::TypeKind type);
	void materialize_address(Reg dst, const AddrExpr& expr, bool exclusive = false);
	void parallel_moves(std::vector<Move>& moves);
	void emit_move(const Move& move);

	// ---- emit_values.cpp
	void emit_inst(uint32_t inst);
	void emit_copy(const lowir::Instruction& inst);
	void emit_addr(const lowir::Instruction& inst);
	void emit_load(const lowir::Instruction& inst);
	void emit_store(const lowir::Instruction& inst);
	void emit_index(const lowir::Instruction& inst);
	void emit_unary(const lowir::Instruction& inst);
	void emit_binary(const lowir::Instruction& inst);
	void emit_float_binary(const lowir::Instruction& inst);
	void emit_divide(const lowir::Instruction& inst);
	DivideForm divide_form(const lowir::Instruction& inst, int64_t& divisor) const;
	void emit_divide_unsigned_shift(const lowir::Instruction& inst, uint64_t divisor);
	void emit_divide_hardware(const lowir::Instruction& inst);
	Reg divide_dividend(const lowir::Instruction& inst);
	void emit_divide_constant(const lowir::Instruction& inst, DivideForm form, int64_t divisor);
	void emit_signed_shift_divide(Reg x, Reg d, int64_t divisor, bool remainder);
	void emit_exact_divide(Reg x, Reg d, int64_t divisor);
	Reg emit_magic_remainder(Reg x, Reg q, int64_t divisor);
	Reg emit_unsigned_magic_divide(Reg x, int64_t divisor, bool remainder);
	Reg emit_signed_magic_divide(Reg x, int64_t divisor, bool remainder);
	void emit_shift(const lowir::Instruction& inst);
	void emit_compare(const lowir::Instruction& inst);
	void emit_select(const lowir::Instruction& inst);
	void emit_convert(const lowir::Instruction& inst);
	void emit_int_convert(const lowir::Instruction& inst);
	void emit_atomic(const lowir::Instruction& inst);
	void emit_bulk(const lowir::Instruction& inst);
	void emit_wide(const lowir::Instruction& inst);
	void normalize(Reg reg, lowir::TypeKind type);
	Reg result_reg(const lowir::Instruction& inst, Reg prefer0 = kNoReg, Reg prefer1 = kNoReg, uint32_t exclude = 0);
	void finish_result(const lowir::Instruction& inst, Reg reg);
	void store_result_memory(const lowir::Instruction& inst, const Operand& src, lowir::TypeKind type);
	Operand memory_result(const lowir::Instruction& inst);
	Reg dying_reg(lowir::Value value) const;

	// ---- emit_calls.cpp
	void note_parameter_classes();
	void emit_entry();
	void fold_entry_extensions(size_t first);
	void narrow_params_observed(std::vector<uint8_t>& observed);
	bool small_mask(const lowir::Instruction& inst) const;
	void stage_call_arguments(const lowir::Instruction& inst, const lowir::Signature* sig,
		const lowir::Parameter* params, const CallLayout& layout, uint32_t& arg_mask,
		std::vector<std::pair<Reg, uint32_t>>& late, uint32_t& late_mask);
	void emit_call(uint32_t inst_id);
	void emit_return(const lowir::Instruction& inst);
	void emit_va_start(const lowir::Instruction& inst);
	void emit_va_arg_object(const lowir::Instruction& inst, uint8_t classes, const Operand& list);
	void classify_signature(const lowir::Signature& sig, CallLayout& layout);
	uint8_t argument_classes(uint32_t inst, uint32_t argument, const lowir::Signature* sig) const;
	void classify_arguments(uint32_t inst, const lowir::Signature* sig, CallLayout& layout);
	const CallLayout& emission_layout(uint32_t inst, const lowir::Signature* sig, bool typed);

	// ---- emit_control.cpp
	void emit_blocks();
	void emit_block(uint32_t block);
	void enter_block(uint32_t block);
	void retain_parameters(uint32_t block);
	Reg incoming_reg(uint32_t param) const;
	void emit_terminator(uint32_t inst);
	void emit_branch(const lowir::Instruction& inst);
	void emit_resume(uint32_t site);
	void emit_switch(const lowir::Instruction& inst);
	bool emit_clustered_switch(
		const lowir::Instruction& inst, const Operand& selector, lowir::TypeKind kind, uint32_t fallback);
	void emit_switch_tests(uint32_t tree, uint32_t first, uint32_t past, int64_t lo, int64_t hi);
	void emit_compare_jump(const lowir::Instruction& compare, uint32_t on_true, uint32_t on_false);
	Cond compare_flags(const lowir::Instruction& compare);
	void emit_edge_transfers(uint32_t from, uint32_t to);
	uint32_t edge_target(lowir::BlockId to);
	void emit_edge_blocks(uint32_t from);
	void emit_eh(uint32_t id);

	// ---- host_eh.cpp
	void analyze_host_eh();
	// Throws BackendError: host EH structure outside the supported subset.
	[[noreturn]] void eh_fail(const char* what) const;
	void eh_walk(uint32_t block, bool record);
	void eh_enter(uint32_t block, uint32_t state, uint64_t mask, std::vector<uint32_t>& work);
	uint64_t eh_landing_mask(uint32_t block);
	int64_t eh_entry_selector(uint32_t block) const;
	uint64_t eh_edge_mask(uint32_t target, uint64_t mask) const;
	bool eh_entry_edge(uint32_t block) const;
	void eh_refine(const lowir::Instruction& branch, uint32_t selector, uint64_t& yes, uint64_t& no) const;
	uint32_t eh_node(uint32_t parent, uint32_t landing, bool cleanup, bool active);
	uint32_t eh_tail(uint32_t base, uint32_t lo, uint32_t hi, uint8_t top);
	uint32_t eh_merge(uint32_t a, uint32_t b);
	bool eh_resume_only(uint32_t block) const;
	void eh_order();
	void eh_prune(uint32_t& landings, bool& reads);

	// ---- host_eh_tables.cpp
	void eh_build_tables(uint32_t landings);
	void eh_choose_indexes();
	uint32_t eh_clause_index(const lowir::Instruction& inst);
	int32_t eh_filter_value(lowir::InstId id, const lowir::Instruction& inst);
	void eh_landing_clauses(uint32_t block, std::vector<int32_t>& filters, bool& catch_all);
	void eh_build_chain(uint32_t node);
	bool eh_unmatched_work(uint32_t block);
	void eh_build_landing(uint32_t block, uint32_t index);
	bool eh_selects(uint32_t block, uint32_t index) const;
	void emit_landing_pad(uint32_t block);

	// ---- branch_layout.cpp
	// Returns the instructions left in the blocks.
	uint32_t layout_branches();

	// ---- MIR construction
	Inst& emit(Op op, lowir::TypeKind type = lowir::TypeKind::Void);
	Inst& emit1(Op op, lowir::TypeKind type, const Operand& a);
	Inst& emit2(Op op, lowir::TypeKind type, const Operand& a, const Operand& b);
	Inst& emit3(Op op, lowir::TypeKind type, const Operand& a, const Operand& b, const Operand& c);
	void begin_mir_block(lowir::NameId name, uint32_t edge_from, uint32_t edge_to);
	void end_mir_block();

	const lowir::Program& program_;
	SelectStats& stats_;
};

} // namespace native
