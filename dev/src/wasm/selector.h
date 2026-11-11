#pragma once

#include "opt/loops.h"
#include "wasm/module.h"
#include "wasm/control.h"

namespace wasm {

class Selector
{
public:
	Selector(const lowir::Program& program, const Module& module, lowir::SymbolId symbol, Function& output);
	void run();

private:
	void value(lowir::Value value, lowir::Type type);
	void address(lowir::Value value);
	void constant(lowir::Literal bits, lowir::Type type);
	void instruction(const lowir::Instruction& inst);
	void unary(const lowir::Instruction& inst);
	void binary(const lowir::Instruction& inst);
	void compare(const lowir::Instruction& inst);
	void convert(const lowir::Instruction& inst);
	void normalize(lowir::Type type);
	void integer_view(lowir::Type type, bool signed_view);
	void condition(lowir::Value operand);
	void edge(lowir::BlockId from, lowir::BlockId to);
	void phi_copies(lowir::BlockId from, lowir::BlockId to);
	void block(lowir::BlockId block);
	void restore_stack();
	void dispatch();
	void simple();
	bool structured();
	void region(uint32_t region);
	void open_label(Op op, lowir::BlockId target);
	void close_label();
	Op memory_op(lowir::Type type, bool store) const;
	lowir::Type operand_type(lowir::Value operand, lowir::Type fallback) const;
	uint32_t add_local(ValueType type);

	const lowir::Program& program_;
	const Module& module_;
	const lowir::FunctionBody& body_;
	Function& out_;
	std::vector<uint32_t> slots_;
	uint32_t frame_ = lowir::kNoId;
	uint32_t saved_stack_ = lowir::kNoId;
	uint32_t pc_ = lowir::kNoId;
	uint32_t nesting_ = 0;
	uint32_t dispatch_depth_ = 0;
	uint32_t current_block_ = 0;
	ControlPlan control_;
	struct Label
	{
		lowir::BlockId target;
		uint32_t depth;
	};
	std::vector<Label> labels_;
};

} // namespace wasm
