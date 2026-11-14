#include "native/x86/emitter.h"

#include "native/target.h"
#include "native/asm/binding.h"
#include "native/asm/encoding.h"
#include "native/asm/mask_forms.h"
#include "native/asm/vex_forms.h"

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

// An immediate narrowed to its instruction's encoding: refused rather than
// truncated when bits would be lost (the selector materializes those).
int64_t narrowed(int64_t value, unsigned bytes)
{
	if (!fits_operation(value, bytes))
		throw InternalError("immediate does not fit its instruction encoding");
	return value;
}

bool calls_out(const Function& function, Target target);

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
			return omit_fp_ ? RM::m(RSP, rsp_disp(op.disp), op.index, op.scale)
							: RM::m(RBP, op.disp, op.index, op.scale);
		return RM::m(op.reg, op.disp, op.index, op.scale);
	default:
		throw InternalError("operand is not a register or memory location");
	}
}

RM FunctionEmitter::scratch_rm() const
{
	if (function_->scratch_bytes == 0)
		throw InternalError("a function without a scratch area uses it");
	return frame_rm(function_->scratch_offset);
}

// [rbp + disp], RSP-based without a frame pointer (Function::omit_frame_pointer).
RM FunctionEmitter::frame_rm(int32_t disp) const
{
	return omit_fp_ ? RM::m(RSP, rsp_disp(disp)) : RM::m(RBP, disp);
}

// Without a frame pointer: a frame displacement from RSP. [virtual rbp +
// disp] is CFA - 16 + disp, and the frame's objects lie save_shift_
// lower, below the saves. Zero can name the one-past address of the
// last object; incoming stack arguments start above that boundary.
int32_t FunctionEmitter::rsp_disp(int32_t disp) const
{
	return disp + frame_top_ - 16 + rsp_shift_ - (disp <= 0 ? save_shift_ : 0);
}

int32_t FunctionEmitter::frame_base() const
{
	return omit_fp_ ? -16 - save_shift_ : -16;
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

// The most nop bytes before a loop head: to 16 when that takes this many, else
// to 8.
const unsigned kLoopPad = 10;

// chain_end_ of a block whose jump chain enters a cycle.
const uint32_t kNoChainEnd = 0xfffffffdu;

// Compact repeated paths can be sensitive to fetch and decoded-op boundaries.
// Move a short cycle to a 32-byte boundary only when it occupies fewer windows.
// The same alignment on the object symbol preserves the choice after linking.
const unsigned kWindowBytes = 32;
const int32_t kWindowLoop = 64;

// The windows `bytes` of code at `position` touch.
unsigned windows_touched(uint64_t position, int32_t bytes)
{
	return static_cast<unsigned>((position % kWindowBytes + bytes + kWindowBytes - 1) / kWindowBytes);
}

// Backward branches identify candidate heads in laid-out code. A later slow
// path can return to the same head without belonging to its compact subcycle.
void FunctionEmitter::find_loop_heads(const Function& function)
{
	loop_heads_.clear();
	loop_last_.clear();
	loop_near_.clear();
	loop_padded_.clear();
	if (!function.align_loops)
		return;
	std::vector<std::pair<int32_t, uint32_t>> heads;
	for (size_t i = 0; i < branch_offset_.size(); ++i)
		if (relax_target_[i] >= 0 && relax_target_[i] <= branch_offset_[i])
			heads.push_back(std::make_pair(relax_target_[i], static_cast<uint32_t>(i + 1)));
	// Computed branches repeatedly enter addressed blocks without a direct
	// back edge. Give their destinations the same bounded alignment policy.
	for (const lowir::CodeLabel& label : function.code_labels)
		if (!function.blocks[label.block].fallen_into &&
			(chain_end_[label.block] == label.block || chain_end_[label.block] == kNoChainEnd))
			heads.push_back(std::make_pair(block_start_[label.block], 0u));
	std::sort(heads.begin(), heads.end());
	for (size_t k = 0; k < heads.size(); ++k)
		if (k == 0 || heads[k - 1].first != heads[k].first)
		{
			size_t first = k;
			while (heads[first].second == 0 && first + 1 < heads.size() && heads[first + 1].first == heads[k].first)
				++first;
			loop_heads_.push_back(heads[k].first);
			loop_near_.push_back(heads[first].second == 0 ? lowir::kNoId : heads[first].second - 1);
			loop_last_.push_back(loop_near_.back());
		}
	loop_limit_.assign(loop_heads_.size(), 0);
	loop_counted_.assign(loop_heads_.size(), 0);
	for (size_t b = 0; b < function.blocks.size(); ++b)
	{
		const Block& block = function.blocks[b];
		if (!block.loop || block.fallen_into || (chain_end_[b] != b && chain_end_[b] != kNoChainEnd))
			continue;
		const auto head = std::lower_bound(loop_heads_.begin(), loop_heads_.end(), block_start_[b]);
		if (head != loop_heads_.end() && *head == block_start_[b])
		{
			uint8_t& limit = loop_limit_[static_cast<size_t>(head - loop_heads_.begin())];
			loop_counted_[static_cast<size_t>(head - loop_heads_.begin())] |= block.counted;
			if (block.counted)
				limit = kWindowLoop;
		}
	}
	// Keep both spans: the full cycle may already fit, while a long slow
	// continuation can hide a profitable fast subcycle.
	for (size_t k = 0, head = 0; k < heads.size(); ++k)
		if (k + 1 == heads.size() || heads[k + 1].first != heads[k].first)
		{
			const uint32_t far = heads[k].second == 0 ? lowir::kNoId : heads[k].second - 1;
			loop_last_[head] = far;
			++head;
		}
	// The heads the second encode pads: those a block not fallen into starts.
	loop_padded_.assign(loop_heads_.size(), 0);
	for (size_t b = 0; b < function.blocks.size(); ++b)
	{
		if (function.blocks[b].fallen_into || (chain_end_[b] != b && chain_end_[b] != kNoChainEnd))
			continue;
		const auto head = std::lower_bound(loop_heads_.begin(), loop_heads_.end(), block_start_[b]);
		if (head != loop_heads_.end() && *head == block_start_[b])
			loop_padded_[static_cast<size_t>(head - loop_heads_.begin())] = 1;
	}
}

// With the branch lengths settled: the bytes of each loop that may move to a
// window boundary, and whether any may.
bool FunctionEmitter::measure_short_loops(const Function& function)
{
	code_window_ = 1;
	for (size_t i = 0; i < short_.size(); ++i)
		relax_saved_[i + 1] = relax_saved_[i] + (short_[i] ? (branch_conditional_[i] ? 4 : 3) : 0);
	loop_bytes_.assign(loop_heads_.size(), 0);
	bool any = false;
	for (size_t k = 0; k < loop_heads_.size(); ++k)
	{
		if (loop_limit_[k] == 0)
			continue;
		const size_t before = static_cast<size_t>(
			std::lower_bound(branch_offset_.begin(), branch_offset_.end(), loop_heads_[k]) - branch_offset_.begin());
		const auto span = [&](uint32_t last) -> int32_t {
			if (last == lowir::kNoId || (k + 1 < loop_heads_.size() && loop_heads_[k + 1] <= branch_offset_[last]))
				return 0;
			// A call on a slow continuation does not penalize the fast cycle.
			// With no trip estimate, only add alignment to local repeated work.
			const auto barrier = std::lower_bound(loop_barriers_.begin(), loop_barriers_.end(), loop_heads_[k]);
			if (!loop_counted_[k] && barrier != loop_barriers_.end() && *barrier <= branch_offset_[last])
				return 0;
			const int32_t length = short_[last] ? 2 : (branch_conditional_[last] ? 6 : 5);
			const int64_t bytes = branch_offset_[last] + length - relax_saved_[last] -
				(loop_heads_[k] - relax_saved_[before]);
			return bytes <= loop_limit_[k] ? static_cast<int32_t>(bytes) : 0;
		};
		// Cover all back edges when that fits the existing footprint budget.
		// A tiny retry path must not hide a larger, already profitable cycle.
		loop_bytes_[k] = span(loop_last_[k]);
		if (loop_bytes_[k] == 0 && function.placement_valid)
			loop_bytes_[k] = span(loop_near_[k]);
		if (loop_bytes_[k] != 0 && !loop_counted_[k])
			code_window_ = kCodeLayoutBytes;
		any = any || loop_bytes_[k] != 0;
	}
	return any;
}

// The padding before loop head `head` when its code would start at `at`.
unsigned FunctionEmitter::loop_pad(size_t head, uint64_t at) const
{
	const int32_t bytes = loop_bytes_[head];
	// End a compact data cycle at a line boundary, keeping its cold
	// continuation out of that line. Back edges skip the entry padding.
	if (bytes != 0 && !loop_counted_[head])
		return static_cast<unsigned>((kCodeLayoutBytes - (at + bytes) % kCodeLayoutBytes) % kCodeLayoutBytes);
	const unsigned to16 = (16 - at % 16) % 16;
	const unsigned usual = to16 <= kLoopPad ? to16 : to16 - 8;
	const unsigned to32 = (kWindowBytes - at % kWindowBytes) % kWindowBytes;
	const bool fewer = bytes != 0 && windows_touched(at + to32, bytes) < windows_touched(at + usual, bytes);
	return fewer ? to32 : usual;
}

// The branches to encode short. Every branch that can be starts short; a
// pass lays the function out with the current choice (the first encode's
// offsets, less what the short branches before a position save, plus the
// padding its loop heads take there) and makes long each short branch
// whose target is then out of rel8 reach. A long branch stays long, so the
// passes end, and the last one found every short branch in reach.
void FunctionEmitter::relax_branches(const Function& function)
{
	const size_t branches = branch_offset_.size();
	for (size_t i = 0; i < branches; ++i)
		short_[i] = relax_target_[i] >= 0 ? 1 : 0;
	std::vector<int64_t>& pads = relax_pads_;
	// (Branches at or past a position do not move it; padding at or before it does.)
	const auto saved_before = [&](int32_t position) {
		return relax_saved_[static_cast<size_t>(
			std::lower_bound(branch_offset_.begin(), branch_offset_.end(), position) - branch_offset_.begin())];
	};
	const auto pads_before = [&](int32_t position) {
		return pads[static_cast<size_t>(
			std::upper_bound(loop_heads_.begin(), loop_heads_.end(), position) - loop_heads_.begin())];
	};
	for (bool changed = true; changed;)
	{
		changed = false;
		uint64_t base = base_;
		// (The loop measure also sums what the short branches save.)
		if (measure_short_loops(function))
		{
			const uint64_t alignment = std::max(kWindowBytes, code_window_);
			base = (base + alignment - 1) / alignment * alignment;
		}
		pads.assign(1, 0);
		for (size_t k = 0; k < loop_heads_.size(); ++k)
		{
			const uint64_t at = base +
				static_cast<uint64_t>(
					loop_heads_[k] - static_cast<int64_t>(base_) - saved_before(loop_heads_[k]) + pads[k]);
			pads.push_back(pads[k] + (loop_padded_[k] ? loop_pad(k, at) : 0));
		}
		for (size_t i = 0; i < branches; ++i)
		{
			if (!short_[i])
				continue;
			const int32_t target = relax_target_[i];
			const int64_t end = branch_offset_[i] - relax_saved_[i] + pads_before(branch_offset_[i]) + 2;
			if (fits_i8(target - saved_before(target) + pads_before(target) - end))
				continue;
			short_[i] = 0;
			changed = true;
		}
	}
}

void FunctionEmitter::emit(const Function& function)
{
	function_ = &function;
	bulk_ = kNoReg;
	calls_out_ = function.omit_frame_pointer && calls_out(function, image_.target);
	std::vector<uint8_t>& text = image_.text();
	uint32_t assembly_alignment = 1;
	if (function.assembly)
		for (const auto& plan : function.assembly->plans)
			for (const auto& inst : plan->instructions)
				assembly_alignment = std::max(assembly_alignment, inst.alignment);
	const uint32_t code_alignment = std::max(16u, assembly_alignment);
	while (text.size() % code_alignment != 0)
		text.push_back(0xcc);
	base_ = text.size();
	image_.define(function.symbol, Section::Text, base_);
	image_.align_code(function.symbol, code_alignment);
	short_.clear();
	branch_offset_.clear();
	branch_label_.clear();
	branch_conditional_.clear();
	relax_pass_ = false;
	encode(function);
	// The second encode: branch length is the only encoding that depends on
	// position (the tables' padding follows the code), so a layout's offsets
	// are the first encode's less the bytes the branches already short
	// before them save. With the branches laid out (O1+) or a loop head to
	// pad, relax_branches chooses; else (-O0) what fits with every other
	// branch long is short.
	const size_t branches = branch_offset_.size();
	short_.assign(branches, 0);
	relax_target_.resize(branches);
	for (size_t i = 0; i < branches; ++i)
		relax_target_[i] = asm_.label_position(branch_label_[i]);
	if (assembly_alignment <= 1)
		find_loop_heads(function);
	else
	{
		loop_heads_.clear();
		loop_padded_.clear();
	}
	relax_saved_.assign(branches + 1, 0);
	bool shortened = false;
	if (assembly_alignment <= 1 && (function.branches_laid_out || !loop_heads_.empty()))
	{
		relax_branches(function);
		shortened = std::find(short_.begin(), short_.end(), 1) != short_.end();
	}
	else if (assembly_alignment <= 1)
		for (size_t i = 0; i < branches; ++i)
			if (relax_target_[i] >= 0 && fits_i8(static_cast<int64_t>(relax_target_[i]) - (branch_offset_[i] + 2)))
				short_[i] = shortened = true;
	if (shortened || !loop_heads_.empty())
	{
		text.resize(base_);
		if (measure_short_loops(function))
		{
			// Retain a stable starting phase in the widest window this layout
			// uses, so preceding code cannot change its internal loop padding.
			const uint32_t alignment = std::max(kWindowBytes, code_window_);
			while (text.size() % alignment != 0)
				text.push_back(0xcc);
			base_ = text.size();
			image_.define(function.symbol, Section::Text, base_);
			image_.align_code(function.symbol, alignment, code_window_);
		}
		asm_.fixups().clear();
		relax_pass_ = true;
		encode(function);
	}
	if (!asm_.resolve_labels())
		throw InternalError("branch out of range");
	for (const lowir::CodeLabel& label : function.code_labels)
		image_.define(label.symbol, Section::Text, asm_.label_position(labels_[label.block]));
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
		case Op::Asm:
			if ((function.insts[k].sub & 1) != 0)
				return true;
			if ((function.insts[k].sub & 2) != 0)
				return false;
			break;
		case Op::Cmov:
		case Op::Setcc:
		case Op::Jcc:
		case Op::Adc:
		case Op::Sbb:
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
bool calls_out(const Function& function, Target target)
{
	if (function.assembly)
		for (const Inst& inst : function.insts)
			if (inst.op == Op::Asm && (inst.sub & 4) != 0)
				return true;
	for (const Inst& inst : function.insts)
	{
		if (inst.op == Op::Call || inst.op == Op::EhCatch || inst.op == Op::Resume || inst.op == Op::Throw ||
			(inst.op == Op::Wide &&
				(static_cast<WideOp>(inst.sub) == WideOp::Div || static_cast<WideOp>(inst.sub) == WideOp::UDiv ||
					static_cast<WideOp>(inst.sub) == WideOp::Mod || static_cast<WideOp>(inst.sub) == WideOp::UMod)) ||
			(inst.op == Op::Convert && (inst.type == lowir::TypeKind::I128 || inst.type2 == lowir::TypeKind::I128)))
			return true;
		// Darwin TLS address materialization calls the preserving TLV helper.
		// Its stack use still makes this a non-leaf: local storage cannot live
		// in the red zone across the lookup, and the call needs alignment.
		if (target == Target::MacOS)
			for (unsigned k = 0; k < inst.nops; ++k)
				if (inst.ops[k].kind == OperandKind::Symbol &&
					(inst.ops[k].access == SymbolAccess::TlsLocal || inst.ops[k].access == SymbolAccess::TlsInitial))
					return true;
	}
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
	return inst.op == Op::Load &&
		(inst.type == TypeKind::I64 || inst.type == TypeKind::Ptr || inst.type == TypeKind::U32) &&
		frame_slot(inst.ops[1], at) && at == offset;
}

} // namespace

bool FunctionEmitter::carried_reload(const Inst& inst)
{
	int32_t offset = 0;
	if (inst.op != Op::Load ||
		(inst.type != TypeKind::I64 && inst.type != TypeKind::Ptr && inst.type != TypeKind::U32) ||
		!frame_slot(inst.ops[1], offset))
		return false;
	for (size_t k = 0; k < carries_.size(); ++k)
	{
		if (carries_[k].offset != offset)
			continue;
		// (The low word of the home, where that is all the reload reads.)
		if (inst.type == TypeKind::U32)
			asm_.mov_load(inst.ops[0].reg, RM::r(carries_[k].reg), 4, false);
		else
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
	unwind_.ret_end = 0;
	bool shared_epilogue = false;
	bool falls_through = true;
	rsp_shift_ = 0;
	dynamic_stack_ = function.eh_records_pushed;
	for (size_t i = 0; i < function.insts.size() && !dynamic_stack_; ++i)
		dynamic_stack_ = function.insts[i].op == Op::StackAlloc || function.insts[i].op == Op::StackSave ||
			function.insts[i].op == Op::StackRestore;
	omit_fp_ = function.omit_frame_pointer && !dynamic_stack_ && eh_model_ == EhModel::Host;
	unwind_.frameless = omit_fp_;
	current_ = 0;
	if (!relax_pass_)
	{
		branch_offset_.clear();
		branch_label_.clear();
		loop_barriers_.clear();
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
	block_start_.resize(blocks);
	next_emitted_.assign(blocks + 1, static_cast<uint32_t>(blocks));
	for (size_t b = blocks; b-- > 0;)
	{
		const bool skip = chain_end_[b] != b && chain_end_[b] != kNoChainEnd;
		if (skip)
			labels_[b] = labels_[chain_end_[b]];
		next_emitted_[b] = skip ? next_emitted_[b + 1] : static_cast<uint32_t>(b);
	}
	int32_t aligned_head = -1;
	for (size_t b = 0; b < blocks; ++b)
	{
		const Block& block = function.blocks[b];
		if (chain_end_[b] != b && chain_end_[b] != kNoChainEnd)
			continue;
		if (!relax_pass_)
			block_start_[b] = static_cast<int32_t>(asm_.size());
		else if (!block.fallen_into)
		{
			const std::vector<int32_t>::const_iterator head = std::lower_bound(
				loop_heads_.begin(), loop_heads_.end(), block_start_[b]);
			if (head != loop_heads_.end() && *head == block_start_[b] && aligned_head != block_start_[b])
			{
				asm_.nops(loop_pad(static_cast<size_t>(head - loop_heads_.begin()), asm_.size()));
				// Empty fallthrough blocks may share this first-encode address.
				// Keep their labels after the padding too: relaxation assumes
				// padding at a backward target moves both target and branch.
				for (size_t prior = b; prior != 0;)
				{
					--prior;
					if (chain_end_[prior] != prior && chain_end_[prior] != kNoChainEnd)
						continue;
					if (block_start_[prior] != block_start_[b])
						break;
					asm_.bind(labels_[prior]);
				}
				aligned_head = block_start_[b];
			}
		}
		falls_through = true;
		if (block.count != 0)
		{
			const Op last = function.insts[block.first + block.count - 1].op;
			falls_through = last != Op::Ret && last != Op::FRet && last != Op::Jmp && last != Op::JumpTable &&
				last != Op::TailJump && last != Op::Throw && last != Op::Resume && last != Op::Trap;
		}
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
			// Other returns emit their own epilogues. A final framed
			// return falls into the shared one; nonduplicated returns jump.
			if ((inst.op == Op::Ret && !(inst.flags & MF_FRAMELESS)) || inst.op == Op::FRet)
				shared_epilogue |= !function.duplicate_epilogues || (next == blocks && i + 1 == block.count);
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
	// Keep implicit fallthrough conservative. Otherwise a shared return
	// with no incoming path is unreachable code, including its unwind rows.
	if (shared_epilogue || falls_through)
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
	// The target forms in the index's own register when that is a scratch
	// register, in the other scratch register otherwise (the index's
	// register keeps its value).
	const Reg target = idx == R10 || idx == R11 ? idx : scratch(inst, table);
	asm_.lea(table, at);
	asm_.mov_load(target, RM::m(table, 0, idx, 4), 4, true);
	asm_.alu_rr(0, target, RM::r(table), 8);
	asm_.jmp_rm(RM::r(target));
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
	unwind_.reserve = 0;
	save_shift_ = 0;
	frame_top_ = 0;
	if (!omit_fp_)
		return;
	// The saves lie right below the return address, the first in the
	// virtual RBP's slot (CFA - 16), and the objects move below them by a
	// multiple of 16, which keeps their alignment. (A frame whose saves sat
	// below the reserved bytes stalled on its stores in a deep recursion.)
	save_shift_ = saved == 0 ? 0 : static_cast<int32_t>((8 * saved - 8 + 15) & ~15u);
	// The objects fill [CFA - 16 - stack_size + 8 * saved - save_shift_,
	// CFA - 16 - save_shift_); a function that calls keeps RSP 16-aligned at
	// its calls, which the padding of stack_size and save_shift_ already
	// give.
	const bool leaf = !calls_out_;
	unwind_.reserve = leaf ? function.stack_size + 8 + static_cast<uint32_t>(save_shift_) - 16 * saved
						   : function.stack_size + 8 - 8 * saved;
	// A body that names no frame location: only a call's alignment
	// needs RSP moved past the saves (none in a function that calls
	// nothing; an odd number of pushes leaves it aligned), not the frame's
	// unused virtual-RBP slot and padding.
	if (frameless_body(function))
		unwind_.reserve = leaf || saved % 2 == 1 ? 0 : 8;
	// SysV reserves 128 bytes below RSP from asynchronous clobbering.
	// A fixed leaf with no register saves fits its storage there when
	// stack_size plus the unused virtual-RBP slot is at most 128 bytes.
	// Keeping that slot also leaves room for a bulk copy's temporary push.
	// Assembly controls its own stack; -mno-red-zone keeps the reservation.
	if (red_zone_ && leaf && saved == 0 && !function.assembly && function.stack_size <= 120)
		unwind_.reserve = 0;
	frame_top_ = static_cast<int32_t>(8 + 8 * saved + unwind_.reserve);
}

void FunctionEmitter::prologue(const Function& function)
{
	const uint32_t base = static_cast<uint32_t>(base_);
	const Reg order[13] = { RBX, R12, R13, R14, R15, RBP, RCX, RDX, RSI, R8, R9, R10, R11 };
	const unsigned registers = (function.preserve_mask & kCallerSavedGprs) != 0 ? 13 : 6;
	const uint32_t saved = static_cast<uint32_t>(__builtin_popcount(function.preserve_mask));
	// A shrink-wrapped function's later prologues record only where they
	// are (the saves' registers and offsets are the first's).
	const bool first = unwind_.prologues.empty();
	UnwindFacts::Prologue at = { asm_.size() - base, 0, {} };
	if (!omit_fp_)
	{
		asm_.push(RBP);
		unwind_.push_end = asm_.size() - base;
		asm_.mov_rr(RBP, RSP);
		unwind_.frame_end = asm_.size() - base;
		// The callee-saved registers fill the frame's bottom
		// (preserve_offset = -stack_size, rbx lowest): the rest is reserved,
		// then they are pushed from the highest (r15) down.
		if (function.stack_size != 8 * saved)
			asm_.alu_ri(5, RM::r(RSP), function.stack_size - 8 * saved, 8);
	}
	// Without a frame pointer they are pushed first, from [virtual rbp]
	// (CFA - 16) down, and the rest is reserved below them.
	int32_t offset = omit_fp_ ? 8 : function.preserve_offset + static_cast<int32_t>(8 * saved);
	unsigned pushed = 0;
	for (unsigned k = registers; k-- > 0;)
	{
		if (!(function.preserve_mask & (1u << order[k])))
			continue;
		asm_.push(order[k]);
		offset -= 8;
		at.save_end[pushed++] = asm_.size() - base;
		if (eh_model_ == EhModel::Host && first)
			unwind_.saves.push_back(UnwindFacts::Save{ asm_.size() - base, order[k], offset });
	}
	if (omit_fp_)
	{
		// (One word is a push of any register: a byte for the subtract's four.)
		if (unwind_.reserve == 8)
			asm_.push(RAX);
		else if (unwind_.reserve != 0)
			asm_.alu_ri(5, RM::r(RSP), unwind_.reserve, 8);
		at.reserve_end = asm_.size() - base;
		if (first)
			unwind_.reserve_end = at.reserve_end;
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
// moves during the body), then frame teardown; without a frame pointer the reserve's
// release, then the pops. Returns where the frame is released (after
// `leave` or the release).
uint32_t FunctionEmitter::epilogue_body(const Function& function, uint32_t* pop_end)
{
	const Reg order[13] = { RBX, R12, R13, R14, R15, RBP, RCX, RDX, RSI, R8, R9, R10, R11 };
	const unsigned registers = (function.preserve_mask & kCallerSavedGprs) != 0 ? 13 : 6;
	if (function.preserve_mask != 0 && dynamic_stack_)
		asm_.lea(RSP, RM::m(RBP, function.preserve_offset));
	if (omit_fp_ && unwind_.reserve != 0)
		asm_.alu_ri(0, RM::r(RSP), unwind_.reserve, 8);
	uint32_t released = asm_.size() - static_cast<uint32_t>(base_);
	unsigned pops = 0;
	for (unsigned k = 0; k < registers; ++k)
		if (function.preserve_mask & (1u << order[k]))
		{
			asm_.pop(order[k]);
			pop_end[pops++] = asm_.size() - static_cast<uint32_t>(base_);
		}
	if (!omit_fp_)
	{
		// No storage or saves were reserved, so RSP already points at the
		// saved RBP. Keep leave for dynamic stacks and user assembly.
		if (function.stack_size == 0 && function.preserve_mask == 0 && !dynamic_stack_ && !function.assembly)
			asm_.pop(RBP);
		else
			asm_.byte(0xc9); // leave
		released = asm_.size() - static_cast<uint32_t>(base_);
	}
	return released;
}

void FunctionEmitter::epilogue(const Function& function)
{
	unwind_.leave_end = epilogue_body(function, unwind_.pop_end);
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
		else if (narrow_integer(inst.type) && a.reg != RAX)
			asm_.mov_load(RAX, RM::r(a.reg), 4, false); // its callers read no more (psABI 3.2.3)
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
	ret.leave_end = epilogue_body(function, ret.pop_end);
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
	ret.leave_end = epilogue_body(function, ret.pop_end);
	if (target.kind == OperandKind::Symbol)
		asm_.jmp_symbol(target.symbol);
	else
		asm_.jmp_rm(RM::r(R11));
	ret.ret_end = asm_.size() - static_cast<uint32_t>(base_);
	unwind_.returns.push_back(ret);
	if (!relax_pass_)
		++stats_.tail_jumps;
}

void FunctionEmitter::loop_barrier()
{
	if (!relax_pass_ && function_->align_loops)
		loop_barriers_.push_back(static_cast<int32_t>(asm_.size()));
}

void FunctionEmitter::call(const Inst& inst)
{
	const uint32_t done = (inst.flags & MF_STRLEN_PREFIX) ? strlen_prefix() : 0xffffffffu;
	loop_barrier();
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
	loop_barrier();
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
	else if (image_.target == Target::MacOS &&
		(b.access == SymbolAccess::TlsLocal || b.access == SymbolAccess::TlsInitial))
	{
		asm_.mov_tlv(R11, b.symbol);
		loop_barrier();
		asm_.call_symbol(runtime_.tlv_lookup);
		asm_.mov_rr(a.reg, R11);
		if (b.disp != 0)
			asm_.lea(a.reg, RM::m(a.reg, b.disp));
	}
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
	case Op::Asm:
	{
		loop_barrier();
		const AssemblyPlan& plan = *function.assembly->plans[static_cast<uint32_t>(a.imm)];
		if (plan.transfer_bytes != 0)
		{
			if (plan.transfer_register.bank == asm_model::RegisterBank::Mask)
				encode_assembly_mask_transfer(
					asm_, plan.transfer_register, rm(b), plan.transfer_store, plan.transfer_bytes);
			else
				encode_assembly_vector_transfer(
					asm_, plan.transfer_register, rm(b), plan.transfer_store, plan.transfer_bytes);
		}
		else
			encode_assembly(asm_, plan.instructions, inst.nops == 2 ? rm(b) : RM());
		break;
	}
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
	{
		RM address = rm(b);
		// RBP/R13 require a displacement even at zero. At scale one,
		// commuting the registers removes it in either LEA width.
		if ((address.base == RBP || address.base == R13) && address.disp == 0 && address.scale == 1 &&
			address.index != kNoReg && address.index != RSP && address.index != RBP && address.index != R13 &&
			address.symbol == 0xffffffffu && address.label == 0xffffffffu)
		{
			std::swap(address.base, address.index);
		}
		// The register's own next or previous value is inc or dec where
		// nothing reads the flags.
		if (address.base == a.reg && address.index == kNoReg && (address.disp == 1 || address.disp == -1) &&
			address.symbol == 0xffffffffu && address.label == 0xffffffffu && !flags_read_after(function, index))
			asm_.step(RM::r(a.reg), address.disp == -1, inst.type == TypeKind::I32 ? 4 : 8);
		else if (inst.type == TypeKind::I32)
			asm_.op(0, false, 0x8d, 1, a.reg, address);
		else
			asm_.lea(a.reg, address);
		break;
	}
	case Op::Add:
	case Op::Sub:
		// One more or less is inc or dec, a byte shorter, where nothing reads
		// the flags (those two leave the carry as it was).
		if (b.kind == OperandKind::Imm && (b.imm == 1 || b.imm == -1) && !flags_read_after(function, index))
			asm_.step(rm(a), (inst.op == Op::Sub) == (b.imm == 1),
				inst.type == TypeKind::Void || inst.type == TypeKind::Ptr ? 8 : register_bytes(inst.type));
		else
			alu(inst.op == Op::Add ? 0 : 5, inst);
		argument_area(inst);
		break;
	case Op::Or:
		alu(1, inst);
		break;
	case Op::And:
		alu(4, inst);
		break;
	case Op::Xor:
		alu(6, inst);
		break;
	case Op::Adc:
		alu(2, inst);
		break;
	case Op::Sbb:
		alu(3, inst);
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
			asm_.bswap(a.reg, register_bytes(inst.type) == 8);
		break;
	case Op::Popcnt:
		asm_.op(0xf3, register_bytes(inst.type) == 8, 0x0fb8, 2, a.reg, rm(b));
		break;
	case Op::Bsf:
		// Count-trailing-zero lowering does not observe flags and permits
		// any result for zero. REP BSF uses TZCNT where available, with
		// the same nonzero result on older CPUs that ignore the prefix.
		asm_.op(0xf3, register_bytes(inst.type) == 8, 0x0fbc, 2, a.reg, rm(b));
		break;
	case Op::Bsr:
	case Op::Cmov:
		asm_.op(0, inst.op == Op::Cmov || register_bytes(inst.type) == 8,
			inst.op == Op::Bsr ? 0x0fbd : 0x0f40u + (inst.sub & 15u), 2, a.reg, rm(b));
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
	case Op::IndirectJump:
		asm_.jmp_rm(rm(a));
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
	case Op::Vector:
		vector_op(inst);
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
	case Op::LockAdd:
		// (The selector passes a register or an immediate the instruction
		// holds: nothing may come between the prefix and the add.)
		if (b.kind == OperandKind::Imm && register_bytes(inst.type) == 8 && !fits_i32(b.imm))
			throw InternalError("lock_add immediate needs a register");
		asm_.byte(0xf0);
		if (b.kind == OperandKind::Imm && (b.imm == 1 || b.imm == -1))
			asm_.step(rm(a), b.imm == -1, register_bytes(inst.type));
		else
			alu(0, inst);
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
	case Op::ReadMxcsr:
	case Op::WriteMxcsr:
		asm_.sse(0, 0xae, inst.op == Op::ReadMxcsr ? Reg(3) : Reg(2), rm(a));
		break;
	case Op::StackSave:
		asm_.mov_rr(a.reg, RSP);
		break;
	case Op::StackRestore:
		asm_.mov_rr(RSP, a.reg);
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
	// A 64-bit test of a mask below bit 31 sets the flags of its 32-bit form.
	const bool low_mask = inst.op == Op::Test && b.kind == OperandKind::Imm && b.imm >= 0 && b.imm <= 0x7fffffffll;
	const unsigned w = low_mask && register_bytes(inst.type) == 8 ? 4 : register_bytes(inst.type);
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
		asm_.test_ri(rm(a), narrowed(b.imm, w), w);
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
