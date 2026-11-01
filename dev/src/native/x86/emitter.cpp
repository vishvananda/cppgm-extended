#include "native/x86/emitter.h"

#include <algorithm>
#include <cstring>

#include "native/mir/type_kinds.h"
#include "native/select/bulk_policy.h"
#include "support/internal_error.h"

// MIR -> x86-64. Integer and control instructions encode directly; the
// pseudo instructions (copy/zero bytes, f80, i128, conversions with no
// single SSE form, EH and variadic state) expand here using the reserved
// scratch registers, never a register that can hold a live value.
namespace native {

using lowir::TypeKind;

namespace {

bool signed_load(TypeKind kind)
{
	return kind == TypeKind::I8 || kind == TypeKind::I16 || kind == TypeKind::I32 || kind == TypeKind::I64 ||
		kind == TypeKind::Ptr || kind == TypeKind::I128;
}

const Reg kX14 = static_cast<Reg>(XMM0 + 14);
const Reg kX15 = static_cast<Reg>(XMM0 + 15);

// An immediate narrowed to its instruction's encoding: refused rather than
// truncated when bits would be lost (the selector materializes those).
int64_t narrowed(int64_t value, unsigned bytes)
{
	if (!fits_operation(value, bytes))
		throw InternalError("immediate does not fit its instruction encoding");
	return value;
}

} // namespace

FunctionEmitter::FunctionEmitter(Image& image, const RuntimeSymbols& runtime, EmitStats& stats)
	: image_(image), runtime_(runtime), stats_(stats), asm_(image.text())
{}

RM FunctionEmitter::rm(const Operand& op) const
{
	switch (op.kind)
	{
	case OperandKind::Reg:
		return RM::r(op.reg);
	case OperandKind::Mem:
		if (op.symbol != lowir::kNoId)
			return RM::rip(op.symbol, op.disp);
		if (op.reg == kFrame && in_aligned_region(*function_, op.disp))
			return RM::m(R12, op.disp - function_->aligned_disp, op.index, op.scale);
		if (op.reg == kFrame)
			return omit_fp_ ? RM::m(RSP,
								  op.disp + static_cast<int32_t>(function_->stack_size) -
									  (op.disp >= 0 ? incoming_shift_ : 0) + rsp_shift_,
								  op.index, op.scale)
							: RM::m(RBP, op.disp, op.index, op.scale);
		return RM::m(op.reg, op.disp, op.index, op.scale);
	default:
		throw InternalError("operand is not a register or memory location");
	}
}

RM FunctionEmitter::scratch_rm() const
{
	return frame_rm(function_->scratch_offset);
}

// [rbp + disp], RSP-based without a frame pointer (Function::omit_frame_pointer).
RM FunctionEmitter::frame_rm(int32_t disp) const
{
	return omit_fp_
		? RM::m(
			  RSP, disp + static_cast<int32_t>(function_->stack_size) - (disp >= 0 ? incoming_shift_ : 0) + rsp_shift_)
		: RM::m(RBP, disp);
}

Reg FunctionEmitter::scratch(const Inst& inst, Reg avoid) const
{
	const Reg candidates[2] = { R11, R10 };
	for (unsigned c = 0; c < 2; ++c)
	{
		bool used = candidates[c] == avoid;
		for (unsigned k = 0; k < inst.nops && !used; ++k)
			used = inst.ops[k].uses_reg(candidates[c]);
		if (!used)
			return candidates[c];
	}
	throw InternalError("no emitter scratch register");
}

void FunctionEmitter::move_imm(Reg dst, int64_t value, bool keep_flags)
{
	if (value == 0 && keep_flags)
	{
		asm_.op_plus(0, false, 0xb8, dst);
		asm_.imm(0, 4);
		return;
	}
	asm_.mov_ri(dst, value);
}

RM FunctionEmitter::constant_rm(const Operand& fimm)
{
	uint8_t bytes[16];
	std::memset(bytes, 0, sizeof bytes);
	uint32_t size = 8;
	if (fimm.type == TypeKind::F32)
	{
		size = 4;
		std::memcpy(bytes, &fimm.imm, 4);
	}
	else if (fimm.type == TypeKind::F80)
	{
		size = 16;
		std::memcpy(bytes, &fimm.imm, 8);
		std::memcpy(bytes + 8, &fimm.imm_high, 2);
	}
	else
		std::memcpy(bytes, &fimm.imm, 8);
	return RM::rip(image_.constant(bytes, size, size), 0);
}

void FunctionEmitter::address_into(Reg dst, const Operand& op)
{
	if (op.kind == OperandKind::Reg)
		asm_.mov_rr(dst, op.reg);
	else
		asm_.lea(dst, rm(op));
}

// ---------------------------------------------------------------------------
// Function

const unsigned kRelaxPasses = 3;

void FunctionEmitter::emit(const Function& function)
{
	function_ = &function;
	std::vector<uint8_t>& text = image_.text();
	while (text.size() % 16 != 0)
		text.push_back(0xcc);
	base_ = text.size();
	image_.define(function.symbol, Section::Text, base_);
	short_.clear();
	branch_offset_.clear();
	branch_label_.clear();
	branch_conditional_.clear();
	relax_pass_ = false;
	encode(function);
	// Later passes: every branch that fits rel8 with the branches still
	// long still fits once more of them shrink, so a short one stays short;
	// each pass shortens what the previous pass's offsets admit (O1+: up to
	// kRelaxPasses; -O0 keeps one). Branch length is the only encoding that
	// depends on position (the tables' padding follows the code), so a
	// pass's offsets are the first encode's less the bytes the branches
	// already short before them save: the passes decide from those, and
	// one encode emits their choice.
	const size_t branches = branch_offset_.size();
	short_.assign(branches, 0);
	relax_target_.resize(branches);
	for (size_t i = 0; i < branches; ++i)
		relax_target_[i] = asm_.label_position(branch_label_[i]);
	relax_saved_.assign(branches + 1, 0);
	const unsigned passes = function.branches_laid_out ? kRelaxPasses : 1;
	bool shortened = false;
	for (unsigned pass = 0; pass < passes; ++pass)
	{
		for (size_t i = 0; i < branches; ++i)
			relax_saved_[i + 1] = relax_saved_[i] + (short_[i] ? (branch_conditional_[i] ? 4 : 3) : 0);
		bool any = false;
		for (size_t i = 0; i < branches; ++i)
		{
			const int32_t target = relax_target_[i];
			if (short_[i] || target < 0)
				continue;
			// (Branches at or past the target do not move it.)
			const size_t before = static_cast<size_t>(
				std::lower_bound(branch_offset_.begin(), branch_offset_.end(), target) - branch_offset_.begin());
			const int64_t offset = branch_offset_[i] - relax_saved_[i];
			if (fits_i8(static_cast<int64_t>(target) - relax_saved_[before] - (offset + 2)))
			{
				short_[i] = 1;
				any = true;
			}
		}
		if (!any)
			break;
		shortened = true;
	}
	if (shortened)
	{
		text.resize(base_);
		asm_.fixups().clear();
		relax_pass_ = true;
		encode(function);
	}
	if (!asm_.resolve_labels())
		throw InternalError("branch out of range");
	image_.add_code_fixups(asm_.fixups(), 0);
	stats_.fixups += asm_.fixups().size();
	asm_.fixups().clear();
	stats_.code_bytes += text.size() - base_;
	for (size_t i = 0; i < short_.size(); ++i)
		++(short_[i] ? stats_.short_branches : stats_.long_branches);
}

// A reload of a compiler-created frame home (never address-taken) from the
// store that defined it in the same block may be satisfied from a register
// still carrying the stored value: the store's own source while nothing
// redefines it, or the emitter scratch r11, copied at the store, when a
// bounded lookahead finds the reload with r11 untouched in between. Windows
// end at block boundaries and at every operation the tracking does not
// model (calls, bulk memory, EH, floating point, symbol accesses,
// emitter-scratch uses); overlapping windows share r11 only one at a time.
namespace {

const unsigned kCarryWindow = 64;

// Whether an instruction past insts[index] reads the flags before any
// sets them (moves, extensions and loads between a compare and its cmov).
bool flags_read_after(const Function& function, uint32_t index)
{
	for (uint32_t k = index + 1; k < function.insts.size(); ++k)
		switch (function.insts[k].op)
		{
		case Op::Mov:
		case Op::Load:
		case Op::Store:
		case Op::Lea:
		case Op::Movzx:
		case Op::Sext:
		case Op::Nop:
			break;
		case Op::Zext:
			// A bit's is an `and`, which sets them.
			if (function.insts[k].type == TypeKind::I1)
				return false;
			break;
		case Op::Cmov:
		case Op::Setcc:
		case Op::Jcc:
			return true;
		default:
			return false;
		}
	return false;
}

bool plain_inst(const Inst& inst)
{
	switch (inst.op)
	{
	case Op::Mov:
	case Op::Load:
	case Op::Lea:
	case Op::Add:
	case Op::Sub:
	case Op::And:
	case Op::Or:
	case Op::Xor:
	case Op::Imul:
	case Op::Neg:
	case Op::Not:
	case Op::Shl:
	case Op::Shr:
	case Op::Sar:
	case Op::Rol:
	case Op::Ror:
	case Op::Setcc:
	case Op::Movzx:
	case Op::Sext:
	case Op::Zext:
	case Op::Cmp:
	case Op::Store:
	case Op::Nop:
		break;
	default:
		return false;
	}
	for (unsigned k = 0; k < inst.nops; ++k)
	{
		const Operand& op = inst.ops[k];
		if ((op.kind == OperandKind::Mem && op.symbol != lowir::kNoId) || op.kind == OperandKind::Symbol ||
			(op.kind == OperandKind::Imm && !fits_i32(op.imm)) || (op.kind == OperandKind::Reg && is_xmm(op.reg)))
			return false;
	}
	return true;
}

// The GPR a plain instruction writes, or kNoReg.
Reg written_gpr(const Inst& inst)
{
	if (inst.op == Op::Store || inst.op == Op::Cmp || inst.op == Op::Test || inst.op == Op::Bt ||
		inst.ops[0].kind != OperandKind::Reg)
		return kNoReg;
	return inst.ops[0].reg;
}

// Whether the encoding calls out: a call, a runtime routine (i128
// division, course catch clauses) or the host resume terminal.
bool calls_out(const Function& function)
{
	for (const Inst& inst : function.insts)
		if (inst.op == Op::Call || inst.op == Op::EhCatch || inst.op == Op::Resume || inst.op == Op::Throw ||
			(inst.op == Op::Wide &&
				(static_cast<WideOp>(inst.sub) == WideOp::Div || static_cast<WideOp>(inst.sub) == WideOp::UDiv ||
					static_cast<WideOp>(inst.sub) == WideOp::Mod || static_cast<WideOp>(inst.sub) == WideOp::UMod)) ||
			(inst.op == Op::Convert && (inst.type == lowir::TypeKind::I128 || inst.type2 == lowir::TypeKind::I128)))
			return true;
	return false;
}

// Whether no instruction addresses the frame (a frame object, a home, the
// scratch area or the stack through RSP): the reservation below the saves
// then serves only the calls' alignment.
bool frameless_body(const Function& function)
{
	if (function.scratch_bytes != 0 || function.aligned_base != 0)
		return false;
	for (const Inst& inst : function.insts)
		for (unsigned k = 0; k < inst.nops; ++k)
		{
			const Operand& op = inst.ops[k];
			if (op.kind == OperandKind::Mem && op.symbol == lowir::kNoId &&
				(op.reg == kFrame || op.reg == RSP || op.index == RSP))
				return false;
		}
	return true;
}

bool frame_slot(const Operand& op, int32_t& offset)
{
	if (op.kind != OperandKind::Mem || op.reg != kFrame || op.index != kNoReg || op.symbol != lowir::kNoId)
		return false;
	offset = op.disp;
	return true;
}

bool reload_of(const Inst& inst, int32_t offset)
{
	int32_t at = 0;
	return inst.op == Op::Load && (inst.type == TypeKind::I64 || inst.type == TypeKind::Ptr) &&
		frame_slot(inst.ops[1], at) && at == offset;
}

} // namespace

bool FunctionEmitter::carried_reload(const Inst& inst)
{
	int32_t offset = 0;
	if (inst.op != Op::Load || (inst.type != TypeKind::I64 && inst.type != TypeKind::Ptr) ||
		!frame_slot(inst.ops[1], offset))
		return false;
	for (size_t k = 0; k < carries_.size(); ++k)
	{
		if (carries_[k].offset != offset)
			continue;
		asm_.mov_rr(inst.ops[0].reg, carries_[k].reg);
		if (!relax_pass_)
			++stats_.scratch_carried_reloads;
		return true;
	}
	return false;
}

void FunctionEmitter::drop_carries(Reg reg, int32_t offset)
{
	for (size_t k = 0; k < carries_.size();)
	{
		if (carries_[k].reg == reg || carries_[k].offset == offset)
		{
			carries_[k] = carries_.back();
			carries_.pop_back();
		}
		else
			++k;
	}
}

// Called before `insts[index]` is encoded; `end` bounds its block.
void FunctionEmitter::track_carries(const Function& function, uint32_t index, uint32_t end)
{
	const Inst& inst = function.insts[index];
	if (!plain_inst(inst))
	{
		carries_.clear();
		return;
	}
	int32_t offset = 0;
	const bool home_store = inst.op == Op::Store && (inst.type == TypeKind::I64 || inst.type == TypeKind::Ptr) &&
		inst.ops[1].kind == OperandKind::Reg && frame_slot(inst.ops[0], offset) &&
		std::binary_search(temp_homes_.begin(), temp_homes_.end(), offset);
	if (!home_store)
	{
		int32_t stored = 0;
		const bool store = inst.op == Op::Store && frame_slot(inst.ops[0], stored);
		drop_carries(written_gpr(inst), store ? stored : INT32_MIN);
		return;
	}
	const Reg source = inst.ops[1].reg;
	drop_carries(kNoReg, offset);
	bool source_written = false;
	bool scratch_used = source == R11;
	for (size_t k = 0; k < carries_.size(); ++k)
		scratch_used = scratch_used || carries_[k].reg == R11;
	const uint32_t limit = index + kCarryWindow < end ? index + kCarryWindow : end;
	for (uint32_t j = index + 1; j < limit; ++j)
	{
		const Inst& next = function.insts[j];
		if (reload_of(next, offset))
		{
			Carry carry;
			carry.offset = offset;
			carry.reg = source;
			if (source_written)
			{
				if (scratch_used)
					return;
				carry.reg = R11;
				pending_copy_ = source;
			}
			carries_.push_back(carry);
			return;
		}
		int32_t stored = 0;
		if (!plain_inst(next) || (next.op == Op::Store && frame_slot(next.ops[0], stored) && stored == offset))
			return;
		if (written_gpr(next) == source)
			source_written = true;
		for (unsigned k = 0; k < next.nops; ++k)
			scratch_used = scratch_used || next.ops[k].uses_reg(R11);
	}
}

void FunctionEmitter::code_location(const Function& function, const Inst& inst)
{
	const lowir::DebugLocation& here = function.locations[inst.location - 1];
	const uint32_t offset = static_cast<uint32_t>(asm_.size() - base_);
	if (!locations_.empty())
	{
		CodeLocation& last = locations_.back();
		if (last.offset == offset)
		{
			last.location = here;
			return;
		}
		if (last.location.file == here.file && last.location.line == here.line && last.location.column == here.column &&
			last.location.scope == here.scope)
			return;
	}
	locations_.push_back(CodeLocation{ offset, here });
}

void FunctionEmitter::encode(const Function& function)
{
	locations_.clear();
	asm_.reset_labels();
	labels_.assign(function.blocks.size(), 0);
	for (size_t b = 0; b < function.blocks.size(); ++b)
		labels_[b] = asm_.new_label();
	epilogue_ = asm_.new_label();
	terminal_ = asm_.new_label();
	terminal_used_ = false;
	tables_.clear();
	unwind_.saves.clear();
	unwind_.calls.clear();
	unwind_.pads.clear();
	unwind_.returns.clear();
	unwind_.prologues.clear();
	unwind_.adjusts.clear();
	rsp_shift_ = 0;
	dynamic_stack_ = function.eh_records_pushed;
	for (size_t i = 0; i < function.insts.size() && !dynamic_stack_; ++i)
		dynamic_stack_ = function.insts[i].op == Op::StackAlloc;
	omit_fp_ = function.omit_frame_pointer && !dynamic_stack_ && eh_model_ == EhModel::Host;
	unwind_.frameless = omit_fp_;
	incoming_shift_ = omit_fp_ && !calls_out(function) ? 8 : 0;
	current_ = 0;
	if (!relax_pass_)
	{
		branch_offset_.clear();
		branch_label_.clear();
	}
	frame_reserve(function);
	// A shrink-wrapped function sets its frame up at its prologue blocks.
	if (!function.shrink_wrapped)
		prologue(function);
	temp_homes_.clear();
	for (size_t k = 0; k < function.bindings.size(); ++k)
	{
		const FrameBinding& binding = function.bindings[k];
		if (binding.kind != BindingKind::Slot && !binding.type.is_object() && binding.type.kind() != TypeKind::F80 &&
			binding.type.kind() != TypeKind::I128)
			temp_homes_.push_back(binding.offset);
	}
	std::sort(temp_homes_.begin(), temp_homes_.end());
	const size_t blocks = function.blocks.size();
	// Jump threading: a block holding only a jump emits nothing; branches
	// to it target the end of its jump chain (a cycle of such blocks is
	// kept). Laid-out MIR (Function::branches_laid_out) was threaded by
	// the selector, and its blocks may fall through, also into a kept
	// jump-only block of a cycle, so it is encoded as it is.
	auto jump_only = [&](size_t b) {
		const Block& block = function.blocks[b];
		return !function.branches_laid_out && b != 0 && block.count == 1 && function.insts[block.first].op == Op::Jmp;
	};
	// chain_end_[b]: the block ending b's jump chain (b itself unless b is
	// jump-only), or kNoChainEnd for a chain that enters a cycle. Each
	// block is walked once: a walk stops at a block already resolved.
	const uint32_t kUnresolved = 0xffffffffu;
	const uint32_t kWalking = 0xfffffffeu;
	const uint32_t kNoChainEnd = 0xfffffffdu;
	chain_end_.assign(blocks, kUnresolved);
	for (size_t b = 0; b < blocks; ++b)
	{
		uint32_t t = static_cast<uint32_t>(b);
		chain_.clear();
		while (chain_end_[t] == kUnresolved && jump_only(t))
		{
			chain_end_[t] = kWalking;
			chain_.push_back(t);
			t = function.insts[function.blocks[t].first].ops[0].symbol;
		}
		const uint32_t end = chain_end_[t] == kUnresolved ? t
														  : (chain_end_[t] == kWalking ? kNoChainEnd : chain_end_[t]);
		if (chain_end_[t] == kUnresolved)
			chain_end_[t] = t;
		for (size_t k = 0; k < chain_.size(); ++k)
			chain_end_[chain_[k]] = end;
	}
	next_emitted_.assign(blocks + 1, static_cast<uint32_t>(blocks));
	for (size_t b = blocks; b-- > 0;)
	{
		const bool skip = chain_end_[b] != b && chain_end_[b] != kNoChainEnd;
		if (skip)
			labels_[b] = labels_[chain_end_[b]];
		next_emitted_[b] = skip ? next_emitted_[b + 1] : static_cast<uint32_t>(b);
	}
	for (size_t b = 0; b < blocks; ++b)
	{
		const Block& block = function.blocks[b];
		if (chain_end_[b] != b && chain_end_[b] != kNoChainEnd)
			continue;
		asm_.bind(labels_[b]);
		// An argument area a noreturn call left open ends with its block.
		if (rsp_shift_ != 0)
		{
			rsp_shift_ = 0;
			unwind_.adjusts.push_back(UnwindFacts::Adjust{ asm_.size() - static_cast<uint32_t>(base_), 0 });
		}
		carries_.clear();
		block_first_ = block.first;
		const uint32_t next = next_emitted_[b + 1];
		const uint32_t next_label = next < blocks ? labels_[next] : epilogue_;
		for (uint32_t i = 0; i < block.count; ++i)
		{
			const uint32_t index = block.first + i;
			const Inst& inst = function.insts[index];
			if (inst.location != 0)
				code_location(function, inst);
			const uint32_t end = block.first + block.count;
			if (carried_reload(inst))
			{
				track_carries(function, index, end);
				continue;
			}
			track_carries(function, index, end);
			// A conditional branch to the next block followed by a jump
			// becomes one inverted conditional branch.
			if (inst.op == Op::Jcc && i + 2 == block.count && next < blocks &&
				function.insts[index + 1].op == Op::Jmp && labels_[inst.ops[0].symbol] == next_label)
			{
				branch(invert(static_cast<Cond>(inst.sub)), true, labels_[function.insts[index + 1].ops[0].symbol]);
				break;
			}
			if (inst.op == Op::Jmp && next < blocks && labels_[inst.ops[0].symbol] == next_label)
				continue;
			inst_index_ = index;
			instruction(function, inst, index, next == blocks && i + 1 == block.count);
			if (pending_copy_ != kNoReg)
			{
				asm_.mov_rr(R11, pending_copy_);
				pending_copy_ = kNoReg;
			}
		}
	}
	asm_.bind(epilogue_);
	epilogue(function);
	if (terminal_used_)
		resume_terminal(function);
	if (eh_model_ == EhModel::Host)
	{
		unwind_.size = asm_.size() - static_cast<uint32_t>(base_);
		for (const EhLanding& landing : function.eh.landings)
			unwind_.pads.push_back(landing.pad == lowir::kNoId
					? 0
					: static_cast<uint32_t>(asm_.label_position(labels_[landing.pad]) - base_));
	}
	emit_tables(function);
}

// jump_table idx: idx = sign-extended entry idx + the table's address,
// then an indirect jump. The table (32-bit offsets of each target from the
// table) follows the function's code, inside its range: no relocation.
void FunctionEmitter::jump_table(const Inst& inst)
{
	const Reg idx = inst.ops[0].reg;
	const Reg table = scratch(inst);
	PendingTable pending;
	pending.label = asm_.new_label();
	pending.first = inst.aux;
	pending.count = inst.aux2;
	tables_.push_back(pending);
	RM at;
	at.is_reg = false;
	at.label = pending.label;
	asm_.lea(table, at);
	asm_.mov_load(idx, RM::m(table, 0, idx, 4), 4, true);
	asm_.alu_rr(0, idx, RM::r(table), 8);
	asm_.jmp_rm(RM::r(idx));
}

void FunctionEmitter::emit_tables(const Function& function)
{
	if (tables_.empty())
		return;
	while (asm_.size() % 4 != 0)
		asm_.byte(0xcc);
	for (const PendingTable& pending : tables_)
	{
		asm_.bind(pending.label);
		for (uint32_t k = 0; k < pending.count; ++k)
			asm_.label_offset32(labels_[function.jump_targets[pending.first + k]], pending.label);
	}
}

void FunctionEmitter::frame_reserve(const Function& function)
{
	const uint32_t saved = static_cast<uint32_t>(__builtin_popcount(function.preserve_mask));
	unwind_.reserve = omit_fp_ ? function.stack_size + 8 - 8 * saved - static_cast<uint32_t>(incoming_shift_) : 0;
	// O1+, a body that names no frame location: only a call's alignment
	// needs RSP moved past the saves (none in a function that calls
	// nothing; an odd number of pushes leaves it aligned), not the frame's
	// unused virtual-RBP slot and padding.
	if (omit_fp_ && frameless_body(function))
		unwind_.reserve = incoming_shift_ != 0 || saved % 2 == 1 ? 0 : 8;
}

void FunctionEmitter::prologue(const Function& function)
{
	const uint32_t base = static_cast<uint32_t>(base_);
	const Reg order[6] = { RBX, R12, R13, R14, R15, RBP }; // RBP: without a frame pointer
	const uint32_t saved = static_cast<uint32_t>(__builtin_popcount(function.preserve_mask));
	// A shrink-wrapped function's later prologues record only where they
	// are (the saves' registers and offsets are the first's).
	const bool first = unwind_.prologues.empty();
	UnwindFacts::Prologue at = { asm_.size() - base, 0, {} };
	if (omit_fp_)
	{
		// The frame below a virtual RBP (CFA - 16, its slot unused): the
		// same layout and alignment as with the frame pointer.
		if (unwind_.reserve != 0)
			asm_.alu_ri(5, RM::r(RSP), unwind_.reserve, 8);
		at.reserve_end = asm_.size() - base;
		if (first)
			unwind_.reserve_end = at.reserve_end;
	}
	else
	{
		asm_.push(RBP);
		unwind_.push_end = asm_.size() - base;
		asm_.mov_rr(RBP, RSP);
		unwind_.frame_end = asm_.size() - base;
	}
	// The callee-saved registers fill the frame's bottom (preserve_offset
	// = -stack_size, rbx lowest): the rest is reserved, then they are
	// pushed from the highest (r15) down.
	if (!omit_fp_ && function.stack_size != 8 * saved)
		asm_.alu_ri(5, RM::r(RSP), function.stack_size - 8 * saved, 8);
	// (Without a frame pointer the first push stores at [virtual rbp -
	// reserve].)
	int32_t offset = omit_fp_ ? 8 - static_cast<int32_t>(unwind_.reserve)
							  : function.preserve_offset + static_cast<int32_t>(8 * saved) + incoming_shift_;
	unsigned pushed = 0;
	for (unsigned k = 6; k-- > 0;)
	{
		if (!(function.preserve_mask & (1u << order[k])))
			continue;
		asm_.push(order[k]);
		offset -= 8;
		at.save_end[pushed++] = asm_.size() - base;
		if (eh_model_ == EhModel::Host && first)
			unwind_.saves.push_back(UnwindFacts::Save{ asm_.size() - base, order[k], offset });
	}
	// The over-aligned objects' base: their region's start rounded up.
	if (function.aligned_base != 0)
	{
		asm_.lea(R12, RM::m(RBP, function.aligned_disp + static_cast<int32_t>(function.aligned_base) - 1));
		asm_.alu_ri(4, RM::r(R12), -static_cast<int64_t>(function.aligned_base), 8);
	}
	if (function.shrink_wrapped)
		unwind_.prologues.push_back(at);
}

// Pops of the saved registers (RSP first back at the save area when it
// moves during the body), `leave`, `ret`.
void FunctionEmitter::epilogue_body(const Function& function, uint32_t* pop_end)
{
	const Reg order[6] = { RBX, R12, R13, R14, R15, RBP };
	if (function.preserve_mask != 0 && dynamic_stack_)
		asm_.lea(RSP, RM::m(RBP, function.preserve_offset));
	unsigned pops = 0;
	for (unsigned k = 0; k < 6; ++k)
		if (function.preserve_mask & (1u << order[k]))
		{
			asm_.pop(order[k]);
			pop_end[pops++] = asm_.size() - static_cast<uint32_t>(base_);
		}
	if (!omit_fp_)
		asm_.byte(0xc9); // leave
	else if (unwind_.reserve != 0)
		asm_.alu_ri(0, RM::r(RSP), unwind_.reserve, 8);
}

void FunctionEmitter::epilogue(const Function& function)
{
	epilogue_body(function, unwind_.pop_end);
	unwind_.leave_end = asm_.size() - static_cast<uint32_t>(base_);
	asm_.ret();
	unwind_.ret_end = asm_.size() - static_cast<uint32_t>(base_);
}

// Without a frame pointer, `sub rsp, n` / `add rsp, n` open and close a
// call's argument area: frame operands move with RSP, and the CFA rule
// follows it.
void FunctionEmitter::argument_area(const Inst& inst)
{
	if (!omit_fp_ || inst.ops[0].kind != OperandKind::Reg || inst.ops[0].reg != RSP ||
		inst.ops[1].kind != OperandKind::Imm)
		return;
	rsp_shift_ += static_cast<int32_t>(inst.op == Op::Sub ? inst.ops[1].imm : -inst.ops[1].imm);
	unwind_.adjusts.push_back(
		UnwindFacts::Adjust{ asm_.size() - static_cast<uint32_t>(base_), static_cast<uint32_t>(rsp_shift_) });
}

void FunctionEmitter::shift(const Inst& inst)
{
	const Operand& a = inst.ops[0];
	const Operand& b = inst.ops[1];
	// The group-2 opcode extension of Shl, Shr, Sar, Rol, Ror (consecutive
	// in mir_model::Op): /4, /5, /7, /0, /1.
	static const unsigned kDigits[] = { 4, 5, 7, 0, 1 };
	const unsigned digit = kDigits[static_cast<unsigned>(inst.op) - static_cast<unsigned>(Op::Shl)];
	// A u32 shift or rotation is its 32-bit form (zero-extending; count
	// below 32).
	const unsigned w = inst.type == TypeKind::U32 ? 4 : 8;
	if (b.kind == OperandKind::Imm)
	{
		if ((b.imm & (w * 8 - 1)) != 0)
			asm_.shift_ri(digit, rm(a), static_cast<unsigned>(b.imm & (w * 8 - 1)), w);
	}
	else
		asm_.shift_cl(digit, rm(a), w);
}

// `ret [value]`: the value into rax or xmm0, then the epilogue.
void FunctionEmitter::ret(const Function& function, const Inst& inst, bool last)
{
	const Operand& a = inst.ops[0];
	if (!relax_pass_)
		++stats_.returns;
	if (inst.nops == 1 && a.kind == OperandKind::Reg)
	{
		if (is_xmm(a.reg))
		{
			if (a.reg != XMM0)
				asm_.sse(0, 0x28, XMM0, RM::r(a.reg));
		}
		else
			asm_.mov_rr(RAX, a.reg);
	}
	// A shrink-wrapped function's frameless return.
	if (inst.flags & MF_FRAMELESS)
	{
		asm_.ret();
		return;
	}
	return_jump(function, last);
}

// A return before the function's last instruction: its own epilogue
// (O1+, Function::duplicate_epilogues) or a jump to the shared one.
void FunctionEmitter::return_jump(const Function& function, bool last)
{
	if (last)
		return;
	if (!function.duplicate_epilogues)
	{
		branch(CC_O, false, epilogue_);
		return;
	}
	UnwindFacts::Epilogue ret;
	epilogue_body(function, ret.pop_end);
	ret.leave_end = asm_.size() - static_cast<uint32_t>(base_);
	asm_.ret();
	ret.ret_end = asm_.size() - static_cast<uint32_t>(base_);
	unwind_.returns.push_back(ret);
}

// A sibling call (O2+): an indirect target in r11, the epilogue, the jump.
void FunctionEmitter::tail_jump(const Function& function, const Inst& inst)
{
	const Operand& target = inst.ops[0];
	if (target.kind == OperandKind::Reg)
		asm_.mov_rr(R11, target.reg);
	else if (target.kind == OperandKind::Mem)
		asm_.mov_load(R11, rm(target), 8, false);
	UnwindFacts::Epilogue ret;
	epilogue_body(function, ret.pop_end);
	ret.leave_end = asm_.size() - static_cast<uint32_t>(base_);
	if (target.kind == OperandKind::Symbol)
		asm_.jmp_symbol(target.symbol);
	else
		asm_.jmp_rm(RM::r(R11));
	ret.ret_end = asm_.size() - static_cast<uint32_t>(base_);
	unwind_.returns.push_back(ret);
	if (!relax_pass_)
		++stats_.tail_jumps;
}

void FunctionEmitter::call(const Inst& inst)
{
	const uint32_t done = (inst.flags & MF_STRLEN_PREFIX) ? strlen_prefix() : 0xffffffffu;
	const uint32_t begin = asm_.size() - static_cast<uint32_t>(base_);
	const Operand& target = inst.ops[0];
	if (target.kind == OperandKind::Symbol)
		asm_.call_symbol(target.symbol);
	else
		asm_.call_rm(rm(target));
	if (done != 0xffffffffu)
		asm_.bind(done);
	// Host EH: a potentially throwing call's LSDA range.
	if (eh_model_ == EhModel::Host && (inst.eh != 0 || !(inst.flags & MF_UNWIND_NO)))
		unwind_.calls.push_back(UnwindFacts::CallSite{
			begin, asm_.size() - static_cast<uint32_t>(base_), inst.eh, (inst.aux2 + 15) & ~15u });
}

// A strlen call's inline prefix (MF_STRLEN_PREFIX): when the 16 bytes at
// rdi lie in one page (its offset in the page is at most 4096 - 16), an
// unaligned load compared bytewise with zero gives a mask whose lowest set
// bit is the length; with no NUL among them, or across a page boundary,
// the call computes it. Returns the label past the call, where the
// result is in rax either way.
uint32_t FunctionEmitter::strlen_prefix()
{
	const uint32_t call = asm_.new_label();
	const uint32_t done = asm_.new_label();
	const Reg probe = XMM0 + 1;
	asm_.op(0, false, 0x8b, 1, RAX, RM::r(RDI)); // mov eax, edi
	asm_.alu_ri(4, RM::r(RAX), 0xfff, 4);        // and eax, 4095
	asm_.alu_ri(7, RM::r(RAX), 0x1000 - 16, 4);  // cmp eax, 4080
	asm_.jcc_short(CC_A, call);
	asm_.sse(0x66, 0xef, XMM0, RM::r(XMM0));            // pxor xmm0, xmm0
	asm_.sse(0xf3, 0x6f, probe, RM::m(RDI, 0));         // movdqu xmm1, [rdi]
	asm_.sse(0x66, 0x74, probe, RM::r(XMM0));           // pcmpeqb xmm1, xmm0
	asm_.op(0x66, false, 0x0fd7, 2, RAX, RM::r(probe)); // pmovmskb eax, xmm1
	asm_.op(0, false, 0x85, 1, RAX, RM::r(RAX));        // test eax, eax
	asm_.jcc_short(CC_E, call);
	asm_.op(0, false, 0x0fbc, 2, RAX, RM::r(RAX)); // bsf eax, eax
	asm_.jmp_short(done);
	asm_.bind(call);
	return done;
}

// The host model's one resume path per function (pa26 README 11): every
// resume of the function jumps here, which reloads the exception its
// landing pad stored and continues unwinding.
void FunctionEmitter::resume_terminal(const Function& function)
{
	asm_.bind(terminal_);
	asm_.mov_load(RDI, frame_rm(function.eh.slot), 8, false);
	const uint32_t begin = asm_.size() - static_cast<uint32_t>(base_);
	asm_.call_symbol(runtime_.unwind_resume);
	unwind_.calls.push_back(UnwindFacts::CallSite{ begin, asm_.size() - static_cast<uint32_t>(base_), 0, 0 });
}

void FunctionEmitter::branch(Cond cond, bool conditional, uint32_t label)
{
	const uint32_t site = current_++;
	if (!relax_pass_)
	{
		branch_offset_.push_back(static_cast<int32_t>(asm_.size()));
		branch_label_.push_back(label);
		branch_conditional_.push_back(conditional ? 1 : 0);
	}
	else if (site < branch_offset_.size())
		branch_offset_[site] = static_cast<int32_t>(asm_.size());
	const bool use_short = relax_pass_ && site < short_.size() && short_[site];
	if (conditional)
		use_short ? asm_.jcc_short(cond, label) : asm_.jcc(cond, label);
	else
		use_short ? asm_.jmp_short(label) : asm_.jmp(label);
}

// ---------------------------------------------------------------------------
// Instructions

// `keep_flags`: a zero by mov rather than xor (a cmov still reads them).
void FunctionEmitter::mov(const Operand& a, const Operand& b, bool keep_flags)
{
	if (b.kind == OperandKind::Reg)
		asm_.mov_rr(a.reg, b.reg);
	else if (b.kind == OperandKind::Imm)
		move_imm(a.reg, b.imm, keep_flags);
	else if (b.kind != OperandKind::Symbol)
		throw InternalError("unsupported mov form");
	else if (b.access == SymbolAccess::Direct)
		asm_.lea(a.reg, RM::rip(b.symbol, b.disp));
	else if (b.access == SymbolAccess::TlsLocal)
		asm_.tls_local_exec(a.reg, b.symbol, b.disp);
	else
	{
		// The address from the symbol's GOT entry (or its thread's), then
		// its addend.
		if (b.access == SymbolAccess::Got)
			asm_.mov_got(a.reg, b.symbol);
		else
			asm_.tls_initial_exec(a.reg, b.symbol);
		if (b.disp != 0)
			asm_.lea(a.reg, RM::m(a.reg, b.disp));
	}
}

void FunctionEmitter::instruction(const Function& function, const Inst& inst, uint32_t index, bool last)
{
	const Operand& a = inst.ops[0];
	const Operand& b = inst.ops[1];
	switch (inst.op)
	{
	case Op::Mov:
		mov(a, b, flags_read_after(function, index));
		break;
	case Op::Load:
		asm_.mov_load(a.reg, rm(b), register_bytes(inst.type), signed_load(inst.type));
		break;
	case Op::Store:
		store(inst);
		break;
	case Op::Lea:
		if (inst.type == TypeKind::I32)
			asm_.op(0, false, 0x8d, 1, a.reg, rm(b));
		else
			asm_.lea(a.reg, rm(b));
		break;
	case Op::Add:
		alu(0, inst);
		argument_area(inst);
		break;
	case Op::Or:
		alu(1, inst);
		break;
	case Op::And:
		alu(4, inst);
		break;
	case Op::Sub:
		alu(5, inst);
		argument_area(inst);
		break;
	case Op::Xor:
		alu(6, inst);
		break;
	case Op::Cmp:
		compare(inst);
		break;
	case Op::Test:
	case Op::Bt:
		bit_test(inst);
		break;
	case Op::Imul:
		multiply(inst);
		break;
	case Op::Neg:
	case Op::Not:
		// 32-bit forms for i32/u32 (zero-extending), else 64-bit.
		asm_.unary(inst.op == Op::Neg ? 3 : 2, rm(a), inst.type == TypeKind::I32 || inst.type == TypeKind::U32 ? 4 : 8);
		break;
	case Op::Bswap:
		if (register_bytes(inst.type) == 2)
			asm_.shift_ri(0, rm(a), 8, 2); // rol r16, 8
		else
		{
			const bool wide = register_bytes(inst.type) == 8;
			if (wide || (a.reg & 8))
				asm_.byte(static_cast<uint8_t>(0x40 | (wide ? 8 : 0) | ((a.reg & 8) ? 1 : 0)));
			asm_.byte(0x0f);
			asm_.byte(static_cast<uint8_t>(0xc8 + (a.reg & 7)));
		}
		break;
	case Op::Bsf:
	case Op::Bsr:
	case Op::Cmov:
		asm_.op(0, inst.op == Op::Cmov || register_bytes(inst.type) == 8,
			inst.op == Op::Bsf       ? 0x0fbc
				: inst.op == Op::Bsr ? 0x0fbd
									 : 0x0f40u + (inst.sub & 15u),
			2, a.reg, rm(b));
		break;
	case Op::Shl:
	case Op::Shr:
	case Op::Sar:
	case Op::Rol:
	case Op::Ror:
		shift(inst);
		break;
	case Op::Setcc:
		asm_.setcc(static_cast<Cond>(inst.sub), a.reg);
		break;
	case Op::Movzx:
		asm_.movzx_rr8(a.reg, b.reg);
		break;
	case Op::Sext:
		asm_.mov_load(a.reg, RM::r(inst.nops == 2 ? b.reg : a.reg), register_bytes(inst.type), true);
		break;
	case Op::Zext:
		if (inst.type == TypeKind::I1)
			asm_.alu_ri(4, RM::r(a.reg), 1, 4);
		else
			asm_.mov_load(a.reg, RM::r(inst.nops == 2 ? b.reg : a.reg), register_bytes(inst.type), false);
		break;
	case Op::Cqo:
		if (inst.type != TypeKind::I32)
			asm_.byte(0x48);
		asm_.byte(0x99);
		break;
	case Op::Idiv:
		asm_.unary(7, rm(a), inst.type == TypeKind::I32 ? 4 : 8);
		break;
	case Op::Div:
		asm_.unary(6, rm(a), inst.type == TypeKind::I32 ? 4 : 8);
		break;
	case Op::MulWide:
		asm_.unary(4, rm(a), 8);
		break;
	case Op::ImulWide:
		asm_.unary(5, rm(a), 8);
		break;
	case Op::Jcc:
		branch(static_cast<Cond>(inst.sub), true, labels_[a.symbol]);
		break;
	case Op::Jmp:
		branch(CC_O, false, labels_[a.symbol]);
		break;
	case Op::JumpTable:
		jump_table(inst);
		break;
	case Op::Nop:
		break;
	case Op::Call:
	case Op::TailJump:
		inst.op == Op::Call ? call(inst) : tail_jump(function, inst);
		break;
	case Op::CopyBytes:
		copy_bytes(inst);
		break;
	case Op::ZeroBytes:
		zero_bytes(inst);
		break;
	case Op::Ret:
		ret(function, inst, last);
		break;
	case Op::Prologue:
		prologue(function);
		break;
	case Op::FRet:
		if (!relax_pass_)
			++stats_.returns;
		// A complex long double's imaginary part (b) goes under the real.
		if (inst.nops == 2)
			x87_load(b, TypeKind::F80);
		x87_load(a, TypeKind::F80);
		return_jump(function, last);
		break;
	case Op::FStp:
		x87_result(inst);
		break;
	case Op::FMov:
		float_move(inst);
		break;
	case Op::FAdd:
	case Op::FSub:
	case Op::FMul:
	case Op::FDiv:
	case Op::FNeg:
	case Op::FAbs:
		float_arith(inst);
		break;
	case Op::FCmp:
		float_compare(a, b, inst.type);
		break;
	case Op::FSetcc:
		float_setcc(inst);
		break;
	case Op::Convert:
		convert(inst);
		break;
	case Op::Wide:
		wide(inst);
		break;
	case Op::WideSetcc:
	case Op::WideCmpBranch:
		wide_compare(inst);
		break;
	case Op::LockXadd:
	case Op::Xchg:
	case Op::LockCmpxchg:
	{
		const unsigned w = register_bytes(inst.type);
		if (inst.op != Op::Xchg)
			asm_.byte(0xf0);
		const uint32_t opcode = inst.op == Op::Xchg
			? (w == 1 ? 0x86 : 0x87)
			: (inst.op == Op::LockXadd ? (w == 1 ? 0x0fc0 : 0x0fc1) : (w == 1 ? 0x0fb0 : 0x0fb1));
		asm_.op(w == 2 ? 0x66 : 0, w == 8, opcode, opcode > 0xff ? 2 : 1, b.reg, rm(a), w == 1);
		break;
	}
	case Op::LockCmpxchg16b:
		cmpxchg16b(inst);
		break;
	case Op::Mfence:
		asm_.byte(0x0f);
		asm_.byte(0xae);
		asm_.byte(0xf0);
		break;
	case Op::VaStart:
	{
		const Reg s = scratch(inst);
		const RM list = rm(a);
		asm_.mov_store_imm(list, inst.aux & 0xffff, 4);
		asm_.mov_store_imm(list.displaced(4), inst.aux >> 16, 4);
		asm_.lea(s, frame_rm(static_cast<int32_t>(inst.aux2)));
		asm_.mov_store(list.displaced(8), s, 8);
		asm_.lea(s, rm(b));
		asm_.mov_store(list.displaced(16), s, 8);
		break;
	}
	case Op::VaArg:
		variadic_argument(inst);
		break;
	case Op::StackAlloc:
		if (b.kind == OperandKind::Imm)
			asm_.alu_ri(5, RM::r(RSP), narrowed((b.imm + 15) & ~15ll, 8), 8);
		else
		{
			const Reg s = scratch(inst);
			asm_.mov_rr(s, b.reg);
			asm_.alu_ri(0, RM::r(s), 15, 8);
			asm_.alu_ri(4, RM::r(s), -16, 8);
			asm_.alu_rr(5, RSP, RM::r(s), 8);
		}
		asm_.mov_rr(a.reg, RSP);
		break;
	case Op::EhPush:
	case Op::EhPop:
	case Op::Throw:
	case Op::Resume:
	case Op::LoadException:
	case Op::EhMark:
	case Op::EhCatch:
	case Op::LoadSelector:
	case Op::Landing:
	case Op::EhSelect:
	case Op::CatchAll:
		eh(inst);
		break;
	case Op::Trap:
		asm_.byte(0x0f);
		asm_.byte(0x0b);
		break;
	default:
		throw InternalError("unsupported machine instruction");
	}
	(void)function;
}

void FunctionEmitter::store(const Inst& inst)
{
	const Operand& dst = inst.ops[0];
	const Operand& src = inst.ops[1];
	const unsigned w = register_bytes(inst.type);
	if (src.kind == OperandKind::Reg)
	{
		asm_.mov_store(rm(dst), src.reg, w);
		return;
	}
	if (w < 8 || fits_i32(src.imm))
	{
		asm_.mov_store_imm(rm(dst), narrowed(src.imm, w), w);
		return;
	}
	const Reg s = scratch(inst);
	asm_.mov_ri(s, src.imm);
	asm_.mov_store(rm(dst), s, 8);
}

void FunctionEmitter::alu(unsigned digit, const Inst& inst)
{
	const Operand& dst = inst.ops[0];
	const Operand& src = inst.ops[1];
	const unsigned w = (inst.type == TypeKind::Void || inst.type == TypeKind::Ptr) ? 8 : register_bytes(inst.type);
	if (src.kind == OperandKind::Imm)
	{
		// An i64 AND whose mask clears the upper half is the 32-bit form.
		if (digit == 4 && w == 8 && src.imm >= 0 && src.imm <= 0xffffffffll && dst.kind == OperandKind::Reg)
		{
			asm_.alu_ri(4, rm(dst), src.imm, 4);
			return;
		}
		if (w < 8 || fits_i32(src.imm))
		{
			asm_.alu_ri(digit, rm(dst), narrowed(src.imm, w), w);
			return;
		}
		const Reg s = scratch(inst);
		asm_.mov_ri(s, src.imm);
		if (dst.kind == OperandKind::Reg)
			asm_.alu_rr(digit, dst.reg, RM::r(s), w);
		else
			asm_.alu_mr(digit, rm(dst), s, w);
		return;
	}
	if (dst.kind == OperandKind::Reg)
		asm_.alu_rr(digit, dst.reg, rm(src), w);
	else
		asm_.alu_mr(digit, rm(dst), src.reg, w);
}

// test a, b (a register or memory, b a register or an immediate) and
// bt a, b (a register, b a register or an immediate bit number).
void FunctionEmitter::bit_test(const Inst& inst)
{
	const Operand& a = inst.ops[0];
	const Operand& b = inst.ops[1];
	const unsigned w = register_bytes(inst.type);
	const uint8_t prefix = w == 2 ? 0x66 : 0;
	if (inst.op == Op::Bt)
	{
		if (b.kind == OperandKind::Imm)
		{
			asm_.op(prefix, w == 8, 0x0fba, 2, 4, rm(a), false, 1);
			asm_.imm(b.imm & (w * 8 - 1), 1);
		}
		else
			asm_.op(prefix, w == 8, 0x0fa3, 2, b.reg, rm(a));
		return;
	}
	if (b.kind == OperandKind::Reg)
	{
		asm_.op(prefix, w == 8, w == 1 ? 0x84 : 0x85, 1, b.reg, rm(a), w == 1);
		return;
	}
	if (w < 8 || fits_i32(b.imm))
	{
		const unsigned n = w == 1 ? 1 : w == 2 ? 2 : 4;
		asm_.op(prefix, w == 8, w == 1 ? 0xf6 : 0xf7, 1, 0, rm(a), w == 1, n);
		asm_.imm(narrowed(b.imm, w), n);
		return;
	}
	const Reg s = scratch(inst);
	asm_.mov_ri(s, b.imm);
	asm_.op(prefix, true, 0x85, 1, s, rm(a));
}

void FunctionEmitter::compare(const Inst& inst)
{
	const Operand& a = inst.ops[0];
	const Operand& b = inst.ops[1];
	const unsigned w = register_bytes(inst.type);
	if (b.kind == OperandKind::Imm)
	{
		if (b.imm == 0 && a.kind == OperandKind::Reg)
		{
			asm_.op(w == 2 ? 0x66 : 0, w == 8, w == 1 ? 0x84 : 0x85, 1, a.reg, RM::r(a.reg), w == 1);
			return;
		}
		if (w < 8 || fits_i32(b.imm))
		{
			asm_.alu_ri(7, rm(a), narrowed(b.imm, w), w);
			return;
		}
		const Reg s = scratch(inst);
		asm_.mov_ri(s, b.imm);
		if (a.kind == OperandKind::Reg)
			asm_.alu_rr(7, a.reg, RM::r(s), w);
		else
			asm_.alu_mr(7, rm(a), s, w);
		return;
	}
	if (a.kind == OperandKind::Reg)
		asm_.alu_rr(7, a.reg, rm(b), w);
	else
		asm_.alu_mr(7, rm(a), b.reg, w);
}

void FunctionEmitter::multiply(const Inst& inst)
{
	const Reg dst = inst.ops[0].reg;
	const Operand& src = inst.ops[1];
	const unsigned w = (inst.type == TypeKind::Void || inst.type == TypeKind::Ptr) ? 8 : register_bytes(inst.type);
	if (inst.nops == 3)
	{
		asm_.imul_rri(dst, rm(src), static_cast<int32_t>(narrowed(inst.ops[2].imm, w)), w);
		return;
	}
	if (src.kind != OperandKind::Imm)
	{
		asm_.imul_rr(dst, rm(src), w);
		return;
	}
	const int64_t v = src.imm;
	if (v > 0 && w == 8)
	{
		unsigned shift = 0;
		int64_t odd = v;
		while ((odd & 1) == 0)
		{
			odd >>= 1;
			++shift;
		}
		if (odd == 1 || odd == 3 || odd == 5 || odd == 9)
		{
			// Strength reduction: one indexed address calculation for the
			// odd factor, then the power-of-two shift.
			if (odd != 1)
				asm_.lea(dst, RM::m(dst, 0, dst, static_cast<uint8_t>(odd - 1)));
			if (shift != 0)
				asm_.shift_ri(4, RM::r(dst), shift, 8);
			return;
		}
	}
	// A 32-bit power of two: the 32-bit shift, which zero-extends as the
	// 32-bit multiply does.
	if (v > 1 && w == 4 && (v & (v - 1)) == 0 && v < (int64_t(1) << 32))
	{
		asm_.shift_ri(4, RM::r(dst), static_cast<unsigned>(__builtin_ctzll(static_cast<uint64_t>(v))), 4);
		return;
	}
	asm_.imul_rri(dst, RM::r(dst), static_cast<int32_t>(narrowed(v, w)), w);
}

} // namespace native
