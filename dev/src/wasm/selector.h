#pragma once

#include "opt/loops.h"
#include "wasm/module.h"
#include "wasm/control.h"
#include "machine/switch_plan.h"
#include "machine/call_arguments.h"
#include "machine/phi_inputs.h"

namespace wasm {

struct ExceptionPlan;

class Selector
{
public:
	Selector(const lowir::Program& program, const Module& module, lowir::SymbolId symbol, Function& output,
		const ExceptionPlan* exceptions = nullptr);
	void run();

private:
	void value(lowir::Value value, lowir::Type type);
	void address(lowir::Value value);
	uint32_t memory_address(lowir::Value value);
	void constant(lowir::Literal bits, lowir::Type type);
	void instruction(const lowir::Instruction& inst);
	void atomic_instruction(const lowir::Instruction& inst);
	void call(const lowir::Instruction& inst);
	void call_body(const lowir::Instruction& inst, const lowir::Signature& signature);
	void exception_call(const lowir::Instruction& inst, const lowir::Signature* signature, uint32_t landing);
	void exception_select(uint32_t landing);
	void exception_throw(const lowir::Instruction& inst);
	void exception_value(lowir::Type type);
	void exception_match(uint32_t landing);
	void exception_snapshot(uint32_t landing, bool restore);
	void restore_exception_context();
	void exception_resume(uint32_t site);
	bool exception_instruction(const lowir::Instruction& inst);
	void exception_address(uint32_t symbol, int64_t offset = 0);
	const VariadicPlan& variadic_plan(const lowir::Instruction& inst, const lowir::Signature& signature);
	void pack_variadic(const lowir::Instruction& inst, const lowir::Signature& signature);
	void variadic_start(const lowir::Instruction& inst);
	void variadic_argument(const lowir::Instruction& inst);
	bool builtin_call(const lowir::Instruction& inst, const lowir::Signature& signature);
	void return_value(const lowir::Instruction& inst);
	bool object_instruction(const lowir::Instruction& inst);
	bool wide_instruction(const lowir::Instruction& inst);
	void wide_part(lowir::Value value, unsigned part);
	void store_wide(lowir::Value destination);
	void wide_binary(const lowir::Instruction& inst);
	void wide_compare(const lowir::Instruction& inst);
	void wide_shift(const lowir::Instruction& inst);
	void wide_convert(const lowir::Instruction& inst);
	void prepare_locals();
	void plan_wide_locals();
	void prepare_frame();
	void frame_address(uint32_t offset);
	void copy_object(lowir::Value source, lowir::Value destination, uint64_t bytes);
	void unary(const lowir::Instruction& inst);
	void binary(const lowir::Instruction& inst);
	void compare(const lowir::Instruction& inst);
	void convert(const lowir::Instruction& inst);
	void normalize(lowir::Type type);
	void vector_instruction(const lowir::Instruction& inst);
	void vector_constant(lowir::Literal bits);
	void vector_shuffle(lowir::Value first, lowir::Value second, const uint8_t* lanes);
	void vector_permute(const lowir::Instruction& inst);
	void vector_scalar(const lowir::Instruction& inst);
	void vector_float(const lowir::Instruction& inst);
	void vector_shift(const lowir::Instruction& inst);
	void vector_compare(const lowir::Instruction& inst);
	void vector_blend_test(const lowir::Instruction& inst);
	void vector_convert(const lowir::Instruction& inst);
	void vector_horizontal(const lowir::Instruction& inst);
	void vector_integer_math(const lowir::Instruction& inst);
	void vector_shuffle_stack(const uint8_t* lanes);
	void vector_reciprocal(const lowir::Instruction& inst);
	void vector_minpos(const lowir::Instruction& inst);
	void vector_carryless(const lowir::Instruction& inst);
	void vector_dot(const lowir::Instruction& inst);
	uint32_t vector_immediate(const lowir::Instruction& inst) const;
	void integer_view(lowir::Type type, bool signed_view);
	void condition(lowir::Value operand);
	void edge(lowir::BlockId from, lowir::BlockId to);
	void phi_copies(lowir::BlockId from, lowir::BlockId to);
	void block(lowir::BlockId block);
	void restore_stack();
	bool select_switch(const lowir::Instruction& inst);
	void switch_tree(const std::vector<machine::CaseCluster>& clusters, uint32_t first, uint32_t past,
		uint32_t selector, lowir::BlockId fallback);
	void switch_table(const std::vector<std::pair<int64_t, uint32_t>>& cases, const std::vector<uint32_t>& targets,
		uint32_t selector, lowir::BlockId fallback);
	void dispatch();
	void simple();
	bool structured();
	void region(uint32_t region);
	void region_child(uint32_t child);
	void dispatch_region(uint32_t region);
	bool branch_label(uint32_t target);
	void open_label(Op op, lowir::BlockId target);
	void close_label();
	Op memory_op(lowir::Type type, bool store) const;
	lowir::Type operand_type(lowir::Value operand, lowir::Type fallback) const;
	uint32_t add_local(ValueType type);

	const lowir::Program& program_;
	const Module& module_;
	const lowir::FunctionBody& body_;
	const lowir::Signature& signature_;
	const Boundary& boundary_;
	Function& out_;
	const ExceptionPlan* exceptions_;
	uint32_t exception_pointer_ = lowir::kNoId;
	uint32_t exception_high_ = lowir::kNoId;
	uint32_t exception_reference_ = lowir::kNoId;
	uint32_t exception_filter_ = lowir::kNoId;
	uint32_t exception_selector_ = lowir::kNoId;
	uint32_t exception_stack_ = lowir::kNoId;
	std::vector<uint32_t> exception_contexts_;
	lowir::InstId current_inst_ = lowir::kNoId;
	std::vector<uint32_t> slots_;
	std::vector<uint32_t> locals_;
	std::vector<uint32_t> wide_locals_;
	std::vector<uint32_t> objects_;
	std::vector<uint32_t> incoming_;
	std::vector<uint32_t> call_areas_;
	std::vector<uint32_t> variadic_areas_;
	std::vector<uint32_t> variadic_copies_;
	std::vector<lowir::Type> argument_types_;
	machine::CallPlanCache<VariadicPlan> variadic_plans_;
	uint32_t wide_low_ = lowir::kNoId;
	uint32_t wide_high_ = lowir::kNoId;
	uint32_t variadic_parameter_ = lowir::kNoId;
	uint32_t hidden_result_ = lowir::kNoId;
	uint32_t frame_ = lowir::kNoId;
	uint32_t saved_stack_ = lowir::kNoId;
	uint32_t pc_ = lowir::kNoId;
	uint32_t nesting_ = 0;
	uint32_t dispatch_depth_ = 0;
	uint32_t current_block_ = 0;
	ControlPlan control_;
	machine::PhiInputs phi_inputs_;
	struct Label
	{
		lowir::BlockId target;
		uint32_t depth;
	};
	std::vector<Label> labels_;
	std::vector<Label> dispatch_labels_;
	std::vector<uint32_t> region_locals_;
};

} // namespace wasm
