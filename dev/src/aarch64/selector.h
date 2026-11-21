#pragma once

#include "aarch64/mir.h"
#include "aarch64/abi.h"
#include "aarch64/shrink_wrap.h"
#include "machine/phi_inputs.h"

namespace opt {
struct Cfg;
}

namespace aarch64 {

struct LiveRange;

// The condition code that holds for an integer comparison predicate.
unsigned integer_condition(unsigned predicate);

// The multiplier constant_division builds for a division by a literal, and
// the divisor a remainder also needs (0 for a quotient); false when that
// selection does not apply.
bool division_constants(
	const lowir::Instruction& inst, const lowir::FunctionBody& body, uint64_t& multiplier, uint64_t& divisor);

// A constant built in a register at the start of the block that enters a
// loop and held through the loop.
struct HeldConstant
{
	lowir::BlockId block;
	unsigned reg;
	uint64_t value;
};

class Selector
{
public:
	Selector(const lowir::Program& program, lowir::SymbolId symbol, Function& out, int level, bool pic,
		const std::vector<uint32_t>& leaf_registers, bool minimize_size, bool contract_float);
	void run();

private:
	void plan();
	struct EntryGuard
	{
		unsigned reg = lowir::kNoId, bytes = 0, offset = 0, bits = 0, literal = 0;
		bool equal = false;
	};
	unsigned incoming_scalar(lowir::Value param) const;
	bool entry_guard(lowir::BlockId entry, EntryGuard& guard) const;
	lowir::BlockId early_return();
	lowir::InstId branch_on_result(const lowir::Instruction& inst, lowir::BlockId block);
	lowir::InstId projected_call(const lowir::Instruction& inst);
	std::vector<uint32_t> slot_mentions_;
	uint32_t result_reg_ = lowir::kNoId;
	void plan_slots();
	std::vector<lowir::BlockId> order_blocks();
	std::vector<uint64_t> block_frequency_;        // estimated executions by block, from order_blocks
	std::vector<BranchEstimate> branch_estimates_; // by block, from order_blocks
	std::vector<uint64_t> block_weights_;
	void prepare_exceptions();
	void landing(lowir::BlockId block);
	void resume(uint32_t site);
	void record_call(const lowir::Instruction& inst);
	void exception_value(const lowir::Instruction& inst);
	uint32_t site(const lowir::Instruction& inst) const;
	struct CallRegisters
	{
		uint8_t allowed = 0xff;
		unsigned char preferred = 8;
	};
	std::vector<CallRegisters> call_registers() const;
	void allocate();
	void call_result_homes();
	void spill_homes(const std::vector<LiveRange>& ranges);
	void parameter_homes(opt::Cfg& cfg);
	void register_objects();
	bool register_access(const lowir::Instruction& inst);
	std::vector<uint8_t> object_words_, parameter_words_, word_accesses_;
	unsigned vector_value(lowir::Value value, unsigned scratch);
	void save_vector(unsigned reg, lowir::TempId result);
	void vector_home(bool store, unsigned reg, lowir::TempId temp);
	void vector_instruction(const lowir::Instruction& inst);
	void vector_minmax(lowir::VectorOp op, unsigned dest, unsigned left, unsigned right);
	void vector_shift(lowir::VectorOp op, unsigned dest, unsigned source, uint64_t count);
	lowir::InstId vector_combination(const lowir::Instruction& inst);
	lowir::InstId scalar_splat(const lowir::Instruction& inst);
	lowir::InstId vector_load(const lowir::Instruction& load);
	lowir::InstId vector_store(const lowir::Instruction& extract);
	lowir::InstId vector_extension(const lowir::Instruction& first);
	lowir::InstId shift_accumulate(const lowir::Instruction& shift);
	lowir::InstId halving_add(const lowir::Instruction& sum);
	bool half_range(lowir::Value v, unsigned width) const;
	lowir::InstId combine_float_pair(const lowir::Instruction& inst);
	lowir::InstId lane_bound(const lowir::Instruction& merge);
	int extended_subscript(
		const lowir::Instruction& convert, lowir::Value& offset, uint64_t& scale, lowir::InstId& at) const;
	bool low_lane_only(const lowir::Instruction& made) const;
	void scalar_bound(bool maximum, bool wide, unsigned dest, unsigned lhs, unsigned rhs, bool number);
	lowir::InstId scalar_lane_binary(const lowir::Instruction& inst);
	lowir::InstId packed_multiply_accumulate(
		const lowir::Instruction& inst, const lowir::Instruction* splat = nullptr, unsigned lane_field = 0);
	lowir::InstId packed_lane_multiply(const lowir::Instruction& inst);
	lowir::InstId horizontal_double_sum(const lowir::Instruction& inst);
	lowir::InstId horizontal_word_sum(const lowir::Instruction& inst);
	void vector_shuffle(const lowir::Instruction& inst, unsigned source);
	void vector_move(unsigned to, unsigned from);
	void pair(lowir::Value src, unsigned lo, unsigned hi);
	void save_pair(lowir::TempId dest, unsigned lo, unsigned hi);
	void wide_binary(const lowir::Instruction& inst);
	void wide_shift(lowir::BinaryOp op);
	lowir::InstId wide_high_product(const lowir::Instruction& first);
	lowir::InstId widening_product(const lowir::Instruction& first);
	lowir::InstId bitfield(const lowir::Instruction& first);
	lowir::InstId inserted_field(const lowir::Instruction& first);
	bool wide(const lowir::Instruction& inst);
	void save_variadic_registers();
	void emit_va_start(const lowir::Instruction& inst);
	void stack_va_arg(const lowir::Instruction& inst, const Argument& arg, bool aligned);
	void emit_va_arg(const lowir::Instruction& inst);
	void read_variadic_value(const lowir::Instruction& inst, const Argument& arg, bool stacked);
	unsigned frame_base() const { return dynamic_stack_ ? 28 : 31; }
	void base_address(unsigned reg, unsigned base, uint32_t offset, unsigned alignment = 1);
	void frame_address(unsigned reg, uint32_t offset) { base_address(reg, frame_base(), offset); }
	void result_address(unsigned reg);
	void home_address(unsigned reg, lowir::TempId temp);
	void stack_alloc(const lowir::Instruction& inst);
	void copy_piece(unsigned dst, unsigned src, unsigned offset, unsigned bytes);
	void copy_staged(unsigned dst, unsigned src, unsigned bytes);
	void copy_run(unsigned dst, unsigned src, uint64_t bytes, bool backward);
	void copy_bytes(unsigned dst, unsigned src, uint64_t size);
	void copy_value(unsigned dst, lowir::Value src, lowir::Type type);
	struct CallLayout
	{
		std::vector<Argument> args;
		std::vector<lowir::Type> types;
		std::vector<unsigned> copies;
		unsigned size;
		bool parallel = false;
		bool tail = false;
		lowir::BlockId block = lowir::kNoId;
		uint16_t saved_args = 0;
		uint32_t preserved_callers = 0;
		uint32_t saved_simd = 0;
	};
	void pass_parallel(const lowir::Instruction& inst, const CallLayout& layout);
	bool tail_compatible(const lowir::Instruction& inst, const CallLayout& layout) const;
	CallLayout call_layout(const lowir::Instruction& inst) const;
	void parameter(lowir::TempId id, const Argument& arg);
	void pass(const Argument& arg, lowir::Value src, lowir::Type type, unsigned copy_offset);
	void transfer(bool load, const Argument& arg, lowir::Value object, lowir::Type type);
	void float_instruction(const lowir::Instruction& inst);
	void float_compare(const lowir::Instruction& inst);
	unsigned float_flags(const lowir::Instruction& inst);
	bool float_consumer(const lowir::Instruction& inst, lowir::BlockId block);
	bool increment_choice(
		const lowir::Instruction& cmp, const lowir::Instruction& add, const lowir::Instruction& select);
	lowir::InstId added_condition(const lowir::Instruction& cmp);
	lowir::InstId contracted_float_sum(const lowir::Instruction& inst);
	lowir::InstId magnitude(const lowir::Instruction& first);
	lowir::InstId cleared_sign(const lowir::Instruction& bits) const;
	void float_choice(const lowir::Instruction& inst, unsigned condition);
	unsigned fp_operand(lowir::Value operand, lowir::Type type, unsigned scratch);
	void fp_home(bool store, unsigned reg, lowir::TempId temp);
	void fp_stack_access(bool store, unsigned reg, uint32_t offset, bool single);
	void save_fp(unsigned reg, lowir::TempId temp);
	void fp_move(bool to_fp, unsigned fp, unsigned gp, lowir::Type type);
	bool floating(const lowir::Instruction& inst) const;
	uint32_t reserve(uint64_t size, uint32_t alignment);
	void stack_adjust(bool subtract);
	void saved_registers(bool store);
	void save_bank(bool store, bool simd, const std::vector<std::pair<unsigned, uint32_t>>& regs, size_t first);
	void constant(unsigned reg, uint64_t bits);
	void move(unsigned to, unsigned from);
	void extend(unsigned reg, unsigned bits, bool sign);
	void scalar_access(const lowir::Instruction& inst, const lowir::Instruction* extension = nullptr);
	lowir::InstId signed_access(const lowir::Instruction& load);
	unsigned zero_or_register(lowir::Value source, lowir::Type type, unsigned scratch);
	void access(bool store, unsigned reg, unsigned base, uint32_t offset, unsigned bytes);
	void darwin_tls_address(unsigned reg, lowir::SymbolId symbol);
	void address(unsigned reg, lowir::Value value);
	unsigned address_register(lowir::Value operand, unsigned scratch);
	void value(unsigned reg, lowir::Value value, lowir::Type type);
	void save(unsigned reg, lowir::TempId temp, bool normalized = false);
	void find_loose_values();
	bool loose(lowir::TempId temp) const { return !loose_.empty() && loose_[temp]; }
	void find_chosen_increments();
	bool chosen_increment(lowir::Value v) const
	{
		return v.is(lowir::ValueKind::Temp) && !increments_.empty() && increments_[v.index];
	}
	lowir::Type type_of(lowir::Value v, lowir::Type fallback) const;
	bool same_home(lowir::TempId dest, lowir::Value source) const;
	bool needs_snapshot(lowir::BlockId from, lowir::BlockId to);
	uint64_t plan_phis();
	void register_edge(lowir::BlockId from, lowir::BlockId to);
	struct EdgeMoves;
	bool edge_moves(lowir::BlockId from, lowir::BlockId to, EdgeMoves& edge);
	bool parallel_edge(lowir::BlockId from, lowir::BlockId to);
	void copy_edge(lowir::BlockId from, lowir::BlockId to);
	void edge(lowir::BlockId from, lowir::BlockId to, bool last = false);
	bool shared_entry_moves(lowir::BlockId block);
	bool shared_exit_moves(lowir::BlockId block);
	bool exit_moves(lowir::BlockId block, lowir::BlockId to, Op test, unsigned operand);
	bool defer_edge(lowir::BlockId block, lowir::BlockId yes, lowir::BlockId no, Op test, unsigned operand);
	void flush_deferred_edges();
	bool shared_move(lowir::BlockId from, lowir::BlockId to);
	void entry_moves(lowir::BlockId block);
	void plan_forwarding(const std::vector<lowir::BlockId>& layout);
	lowir::BlockId next_emitted(const std::vector<lowir::BlockId>& layout, size_t index) const;
	bool forwarded(lowir::BlockId block) const { return !forwarded_.empty() && forwarded_[block] != lowir::kNoId; }
	void bind_forwarded();
	void select(const lowir::Instruction& inst, lowir::BlockId block);
	void atomic(const lowir::Instruction& inst);
	void switch_edges(const lowir::Instruction& inst, lowir::BlockId block);
	struct SwitchSearch;
	bool search_switch(SwitchSearch& search, lowir::BlockId from, lowir::Type type);
	void search_cases(const SwitchSearch& search, uint32_t first, uint32_t past, int64_t lo, int64_t hi, bool compared);
	void compare_key(unsigned dest, unsigned input, uint64_t amount, bool narrow);
	unsigned condition_register(lowir::Value operand, unsigned scratch);
	bool word_condition(lowir::Value operand) const;
	unsigned operand_register(lowir::Value operand, lowir::Type type, unsigned scratch);
	void narrow_left(lowir::Value src, lowir::Type type, lowir::TempId result, unsigned width, unsigned count);
	bool constant_quotient(const lowir::Instruction& inst, uint64_t divisor, unsigned bits, unsigned lhs, unsigned dst);
	bool constant_division(const lowir::Instruction& inst, uint64_t raw, unsigned bits, unsigned lhs, unsigned dst);
	void build_held_constants(lowir::BlockId block);
	std::vector<HeldConstant> held_constants_;
	// By division: the multiplier's register | the divisor's << 8 (255: none).
	std::vector<std::pair<lowir::InstId, uint32_t>> held_divisions_;
	bool simple_binary(const lowir::Instruction& inst);
	lowir::InstId arithmetic_operand(const lowir::Instruction& producer);
	void integer_unary(const lowir::Instruction& inst);
	void binary(const lowir::Instruction& inst);
	void compare(const lowir::Instruction& inst, bool flags_ready = false);
	void compare_flags(const lowir::Instruction& inst);
	lowir::InstId truncated_operand(const lowir::Instruction& trunc, lowir::BlockId block);
	lowir::InstId rotated_operand(const lowir::Instruction& first);
	lowir::InstId shifted_operand(const lowir::Instruction& shift, lowir::BlockId block);
	bool emit_scalar_pair(
		const lowir::Instruction& low, const lowir::Instruction& high, lowir::Value base, uint64_t offset);
	lowir::InstId paired_access(const lowir::Instruction& low);
	lowir::InstId paired_fields(const lowir::Instruction& first);
	lowir::InstId paired_store(const lowir::Instruction& index);
	lowir::InstId address_sequence(lowir::InstId id);
	bool page_offset(lowir::Value pointer, int64_t displacement, unsigned bytes) const;
	lowir::InstId global_access(const lowir::Instruction& addr);
	bool vector_page_access(const lowir::Instruction& inst);
	void choose(const lowir::Instruction& inst, unsigned condition);
	lowir::InstId bit_branch(
		const lowir::Instruction& inst, lowir::BlockId block, const lowir::Instruction* shift = nullptr);
	lowir::InstId narrow_range_branch(const lowir::Instruction& sub, lowir::BlockId block);
	lowir::InstId finish_condition(lowir::InstId id, unsigned condition, lowir::BlockId block);
	void set_condition(lowir::TempId result, unsigned condition);
	lowir::InstId compare_pair(const lowir::Instruction& first, lowir::BlockId block);
	bool compare_consumer(const lowir::Instruction& inst, lowir::BlockId block, bool flags_ready = false);
	lowir::InstId compare_select_pair(const lowir::Instruction& inst);
	lowir::InstId compare_before_increment(const lowir::Instruction& inst, lowir::BlockId block);
	bool backedge_moves(lowir::BlockId from, lowir::BlockId to, lowir::BlockId other, unsigned condition);
	bool empty_edge(lowir::BlockId block, lowir::BlockId to);
	void conditional_edges(lowir::BlockId block, const lowir::Instruction& branch, Op test, unsigned operand);
	void conditional_bit_edges(
		lowir::BlockId block, const lowir::Instruction& branch, Op test, unsigned source, unsigned bit, unsigned width);
	void call(const lowir::Instruction& inst);
	void emit_call(const lowir::Instruction& inst, const CallLayout& layout, bool projected = false);
	std::vector<uint32_t> argument_spills_, simd_call_spills_, remat_slots_;
	void protect_simd_at_calls(unsigned reg, uint64_t calls);
	uint64_t cold_crossings(const LiveRange& range, const std::vector<std::pair<uint32_t, uint32_t>>& segments,
		const std::vector<uint32_t>& calls, uint64_t budget) const;
	bool rematerialize_frame_address(lowir::TempId id);
	uint32_t spill_home(uint32_t offset);
	bool rematerialized(lowir::TempId id) const { return !remat_slots_.empty() && remat_slots_[id] != lowir::kNoId; }
	void epilogue(const lowir::Instruction& inst);
	bool load_returns_directly(const lowir::Instruction& load) const;
	void finish(lowir::SymbolId tail = lowir::kNoId, unsigned indirect = 31);

	const lowir::Program& program_;
	lowir::SymbolId symbol_;
	std::vector<uint32_t> selectors_;
	uint32_t exception_slot_ = lowir::kNoId;
	const lowir::FunctionBody& body_;
	const lowir::Signature& signature_;
	Function& out_;
	int level_;
	bool pic_, minimize_size_, contract_float_;
	const std::vector<uint32_t>& leaf_registers_;
	bool dynamic_stack_ = false, stack_pair_ = false;
	lowir::BlockId fallthrough_ = lowir::kNoId;
	uint32_t outgoing_ = 0;
	std::vector<uint32_t> slots_, homes_, registers_, vectors_, uses_, call_result_slots_;
	std::vector<std::pair<unsigned, uint32_t>> preserved_, preserved_fp_;
	machine::PhiInputs phis_;
	std::vector<uint32_t> entry_regs_, aliases_;
	std::vector<BitTest> bit_tests_;
	std::vector<lowir::InstId> entry_elided_;
	std::vector<uint8_t> loose_;       // narrow values held with undefined high bits (narrow.cpp)
	std::vector<uint8_t> spill_homes_; // by 8-byte frame unit: a scalar temp's spill home
	std::vector<uint8_t> increments_;  // increments made by the selection reading them (integer.cpp)
	std::vector<std::pair<lowir::BlockId, lowir::BlockId>> snapshot_edges_;
	std::vector<lowir::BlockId> frame_phi_blocks_;
	std::vector<lowir::BlockId> entry_moves_, forwarded_;
	std::vector<uint8_t> exit_moves_; // by block: its branch's edges make the same moves before it
	lowir::BlockId exit_done_ = lowir::kNoId;
	struct DeferredEdge
	{
		lowir::BlockId from, to;
		uint32_t label;
	};
	std::vector<DeferredEdge> deferred_edges_;
	std::vector<CallLayout> calls_;
	std::vector<uint32_t> call_indices_;
	Argument result_ = {};
	Arguments named_;
	uint32_t tls_vectors_ = lowir::kNoId;
	uint32_t frame_pointer_ = lowir::kNoId;
	uint32_t variadic_area_ = lowir::kNoId;
	uint32_t result_slot_ = lowir::kNoId;
	uint32_t result_pointer_ = lowir::kNoId;
	uint32_t frame_ = 0, phi_area_ = 0;
	FrameCode frame_code_;
};

} // namespace aarch64
