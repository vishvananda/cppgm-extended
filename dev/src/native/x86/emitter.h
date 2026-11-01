#pragma once

#include "native/x86/code_location.h"

#include <cstdint>
#include <vector>

#include "native/image/image.h"
#include "native/mir/mir.h"
#include "native/x86/assembler.h"
#include "support/telemetry.h"

// Native emission of one MIR function into the image's code section:
// prologue and shared epilogue from the final frame facts, each MIR
// instruction (pseudo instructions expanded with the reserved scratch
// registers r10/r11 and xmm14/xmm15), local labels, and a second pass
// that encodes every branch whose rel8 form provably fits.
namespace native {

// Image symbols of the runtime the emitted code refers to.
struct RuntimeSymbols
{
	uint32_t eh_dispatch = 0;
	uint32_t eh_uncaught = 0;
	uint32_t eh_top = 0;
	uint32_t eh_payload = 0;
	// The selector of the landing being dispatched, and the routine one
	// catch clause calls (match the clause's type, then set the selector).
	uint32_t eh_selector = 0;
	uint32_t eh_catch = 0;
	// The host model's _Unwind_Resume.
	uint32_t unwind_resume = 0;
	// i128 division (libgcc's names): __divti3, __udivti3, __modti3,
	// __umodti3, by WideOp::Div - Div.
	uint32_t wide_division[4] = { 0, 0, 0, 0 };
	// i128/floating conversions (libgcc's names, kFloatConversionSymbols):
	// to floating, then from floating; signed, then unsigned; f32, f64, f80.
	uint32_t float_conversion[12] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
};

// What a host unwind table and LSDA describe of the last function encoded
// in the host EH model, as offsets from its start: the frame setup and
// teardown, the potentially throwing calls (each with its landing, 1 +
// its index in EhTables::landings, or 0, and the stack argument bytes
// it pushed) in address order, and the landing pads.
struct UnwindFacts
{
	struct Save
	{
		uint32_t end; // after the store
		Reg reg;
		int32_t offset; // [rbp + offset]
	};
	struct CallSite
	{
		uint32_t begin;
		uint32_t end;
		uint32_t landing;
		uint32_t args;
	};
	uint32_t push_end = 0;  // after `push rbp`
	uint32_t frame_end = 0; // after `mov rbp, rsp`
	// Function::omit_frame_pointer: the CFA stays RSP-based; `sub rsp,
	// reserve` (ending at reserve_end, absent when reserve is 0) precedes
	// the saves' pushes.
	bool frameless = false;
	uint32_t reserve = 0;
	uint32_t reserve_end = 0;
	std::vector<Save> saves;
	struct Epilogue
	{
		uint32_t leave_end;  // after its `leave` (frameless: after the stack is released)
		uint32_t ret_end;    // after its `ret`
		uint32_t pop_end[6]; // frameless: after each pop, in pop order
	};
	uint32_t leave_end = 0;        // after the shared epilogue's `leave`
	uint32_t ret_end = 0;          // after its `ret`; the resume terminal follows
	uint32_t pop_end[6] = {};      // frameless: after the shared epilogue's pops
	std::vector<Epilogue> returns; // duplicated epilogues before it, in order
	// A shrink-wrapped function's prologues (each sets the frame up from
	// the entry's state), in address order; its code before the first has
	// no frame.
	struct Prologue
	{
		uint32_t start;
		uint32_t reserve_end;
		uint32_t save_end[6];
	};
	std::vector<Prologue> prologues;
	// Frameless: after each `sub rsp, n` or `add rsp, n` of a call's
	// argument area (or a block's start that closes one a noreturn call
	// left open), the argument bytes below the body's RSP.
	struct Adjust
	{
		uint32_t end;
		uint32_t args;
	};
	std::vector<Adjust> adjusts;
	uint32_t size = 0;
	std::vector<CallSite> calls;
	std::vector<uint32_t> pads;
};

struct EmitStats
{
	telemetry::Counter code_bytes = 0;
	telemetry::Counter returns = 0;
	telemetry::Counter tail_jumps = 0;
	telemetry::Counter short_branches = 0;
	telemetry::Counter long_branches = 0;
	telemetry::Counter fixups = 0;
	telemetry::Counter scratch_carried_reloads = 0;
};

class FunctionEmitter
{
public:
	FunctionEmitter(Image& image, const RuntimeSymbols& runtime, EmitStats& stats);

	// Encodes `function` at the end of the code section and defines its
	// symbol there.
	void emit(const Function& function);
	// The exception model of the functions emitted from now on; the host
	// model records each function's unwind facts.
	void set_eh_model(EhModel model) { eh_model_ = model; }
	const UnwindFacts& unwind() const { return unwind_; }
	const std::vector<CodeLocation>& locations() const { return locations_; }

private:
	std::vector<CodeLocation> locations_;
	void code_location(const Function& function, const Inst& inst);
	void encode(const Function& function);
	void prologue(const Function& function);
	void frame_reserve(const Function& function);
	void epilogue(const Function& function);
	void epilogue_body(const Function& function, uint32_t* pop_end);
	RM frame_rm(int32_t disp) const;
	void return_jump(const Function& function, bool last);
	void tail_jump(const Function& function, const Inst& inst);
	void ret(const Function& function, const Inst& inst, bool last);
	void argument_area(const Inst& inst);
	void shift(const Inst& inst);
	void instruction(const Function& function, const Inst& inst, uint32_t index, bool last_in_function);
	void branch(Cond cond, bool conditional, uint32_t label);
	RM rm(const Operand& op) const;
	Reg scratch(const Inst& inst, Reg avoid = kNoReg) const;
	void move_imm(Reg dst, int64_t value, bool keep_flags);
	void alu(unsigned digit, const Inst& inst);
	void compare(const Inst& inst);
	void multiply(const Inst& inst);
	void store(const Inst& inst);
	void mov(const Operand& a, const Operand& b, bool keep_flags);
	void float_move(const Inst& inst);
	void float_arith(const Inst& inst);
	void vector_op(const Inst& inst);
	void vector_special(const Inst& inst);
	void vector_move(Reg d, const Operand& src);
	Reg vector_operands(const Inst& inst);
	void float_compare(const Operand& a, const Operand& b, lowir::TypeKind type);
	void float_setcc(const Inst& inst);
	void convert(const Inst& inst);
	void x87_load(const Operand& op, lowir::TypeKind type);
	void x87_result(const Inst& inst);
	void copy_bytes(const Inst& inst);
	unsigned copy_chunk_limit(const Operand& src, uint32_t bytes) const;
	void zero_bytes(const Inst& inst);
	void wide(const Inst& inst);
	void wide_float_convert(const Inst& inst);
	void wide_compare(const Inst& inst);
	void wide_half(Reg dst, const Operand& op, unsigned half);
	void wide_alu(unsigned digit, Reg dst, const Operand& op, unsigned half);
	void cmpxchg16b(const Inst& inst);
	void variadic_argument(const Inst& inst);
	void va_arg_object(const Inst& inst);
	void va_copy_bytes(const RM& home, int32_t source, uint32_t offset, uint32_t bytes);
	void eh(const Inst& inst);
	void call(const Inst& inst);
	void bit_test(const Inst& inst);
	uint32_t strlen_prefix();
	void resume_terminal(const Function& function);
	void jump_table(const Inst& inst);
	void emit_tables(const Function& function);
	void address_into(Reg dst, const Operand& op);
	RM constant_rm(const Operand& fimm);
	RM scratch_rm() const;

	Image& image_;
	const RuntimeSymbols& runtime_;
	EmitStats& stats_;
	Assembler asm_;
	const Function* function_ = nullptr;
	std::vector<uint32_t> labels_;
	uint32_t epilogue_ = 0;
	uint64_t base_ = 0;
	EhModel eh_model_ = EhModel::Course;
	UnwindFacts unwind_;
	uint32_t terminal_ = 0;      // the host resume terminal's label
	bool dynamic_stack_ = false; // RSP moves (stack_alloc, pushed EH records)
	bool omit_fp_ = false;       // Function::omit_frame_pointer
	// Without a frame pointer, a function making no call needs no 16-byte
	// alignment: its frame sits 8 bytes higher (no padding for the missing
	// saved RBP), which moves the incoming arguments, not the slots, by
	// incoming_shift_ relative to RSP.
	int32_t incoming_shift_ = 0;
	// Without a frame pointer, the bytes of a call's argument area RSP is
	// lowered by at the current instruction (`sub rsp, n` before the call,
	// `add rsp, n` after it, in one block): frame operands lie that much
	// farther from RSP.
	int32_t rsp_shift_ = 0;
	bool terminal_used_ = false;
	// Branch relaxation: per MIR instruction index, whether its branch is
	// encoded short; the first pass records the long encoding's offsets.
	std::vector<uint8_t> short_;
	std::vector<int32_t> branch_offset_;
	std::vector<uint32_t> branch_label_;
	std::vector<uint8_t> branch_conditional_; // per branch: a jcc (rel8 saves 4 bytes; a jmp 3)
	std::vector<int32_t> relax_target_;       // per branch: its target's first-encode position
	std::vector<int64_t> relax_saved_;        // per branch: bytes the short branches before it save
	bool relax_pass_ = false;
	uint32_t current_ = 0;
	uint32_t block_first_ = 0; // the block being encoded: its first instruction
	uint32_t inst_index_ = 0;  // and the instruction being encoded
	// Carried reloads: compiler-created frame homes whose value a register
	// still holds since its store in the current block.
	struct Carry
	{
		int32_t offset;
		Reg reg;
	};
	std::vector<Carry> carries_;
	std::vector<int32_t> temp_homes_;
	Reg pending_copy_ = kNoReg;
	std::vector<uint32_t> next_emitted_; // first block at or after b that emits code
	std::vector<uint32_t> chain_end_;    // jump threading: where b's jump chain ends
	std::vector<uint32_t> chain_;
	// The jump tables of the function being encoded, emitted after its code.
	struct PendingTable
	{
		uint32_t label;
		uint32_t first;
		uint32_t count;
	};
	std::vector<PendingTable> tables_;
	bool carried_reload(const Inst& inst);
	void track_carries(const Function& function, uint32_t index, uint32_t end);
	void drop_carries(Reg reg, int32_t offset);
};

} // namespace native
