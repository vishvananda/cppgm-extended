#include "native/select/function_selector.h"
#include "native/select/for_each_inst.h"

#include <climits>
#include <string>

#include "native/backend_error.h"
#include "native/select/bulk_policy.h"
#include "native/select/operand_roles.h"
#include "support/internal_error.h"
#include "support/telemetry.h"

// Register allocation facts. Values that live only inside their defining
// block get registers chosen at their definition against a precomputed
// mask: the registers clobbered strictly inside their interval, and those
// clobbered before an instruction reads its operands at the interval's
// last use. One backward scan per block computes every mask in O(32 n).
// Values crossing calls therefore land in callee-saved registers or, when
// none is free, in frame homes; values used in other blocks always have
// a home. Parameters whose incoming register no instruction of the
// function clobbers stay pinned there.
namespace native {

using lowir::Instruction;
using lowir::kNoId;
using lowir::Opcode;
using lowir::Type;
using lowir::TypeKind;
using lowir::Value;
using lowir::ValueKind;

namespace {

// RBP, last of the callee-saved registers, only without a frame pointer
// (rbp_free_).
const Reg kGprOrder[] = { R8, R9, RDI, RSI, RDX, RCX, RAX, RBX, R12, R13, R14, R15, RBP };
const Reg kCalleeFirstOrder[] = { RBX, R12, R13, R14, R15, RBP, R8, R9, RDI, RSI, RDX, RCX, RAX };
// Functions with vector parameters follow the reference order, which keeps
// the fixed registers of comparisons, division and shifts last.
const Reg kVectorBoundaryOrder[] = { R8, R9, RDI, RSI, RBX, R12, R13, R14, R15, RBP, RDX, RCX, RAX };

// The conversion scratch area (x87 transfers, saved rbx), sized as the
// reference frame layout sizes it.
const uint32_t kScratchAreaBytes = 48;
const uint64_t kMaxFrameBytes = 0x7fff0000u;

// With calls' argument areas, the most instructions addressing the frame
// a function without a frame pointer may have: each takes a SIB byte more
// RSP-based than RBP-based, which in a large body outweighs the two
// instructions per call the frame pointer costs.
const uint32_t kAreaFrameAccessCap = 64;

// Whether the frame pointer can go (Function::omit_frame_pointer): RSP
// moves after the prologue only by calls' argument areas (`sub rsp, n`
// and `add rsp, n`, which the emitter follows with the frame operands and
// the CFA rule), and with those, few instructions address the frame. No
// other instruction names RSP as a register (over-aligned argument
// realignment), allocates stack, walks the variadic areas or converts
// through the i128 routines' stack argument.
bool fixed_stack(const Function& f)
{
	bool areas = false;
	uint32_t accesses = 0;
	for (const Inst& inst : f.insts)
	{
		if (inst.op == Op::StackAlloc || inst.op == Op::VaStart || inst.op == Op::VaArg ||
			(inst.op == Op::Convert && (inst.type == TypeKind::I128 || inst.type2 == TypeKind::I128)))
			return false;
		bool frame = false;
		for (unsigned k = 0; k < inst.nops; ++k)
		{
			const Operand& op = inst.ops[k];
			frame = frame || (op.kind == OperandKind::Mem && (op.reg == kFrame || op.index == kFrame));
			if (op.kind != OperandKind::Reg || op.reg != RSP)
				continue;
			if (k != 0 || (inst.op != Op::Sub && inst.op != Op::Add) || inst.ops[1].kind != OperandKind::Imm)
				return false;
			areas = true;
		}
		accesses += frame ? 1 : 0;
	}
	return !areas || accesses <= kAreaFrameAccessCap;
}

// O1+, without EH (its landings are edges this graph lacks): object slots
// of one type that no execution uses both of share storage (`owner`: per
// slot the slot whose storage it takes; left empty when none share). Two
// interfere when a block naming one reaches (or is) a block naming the
// other; slots live for the whole call, so without such a path no value
// of one survives into a use of the other. Up to 64 candidates, one bit
// each; nothing is allocated unless two have one type.
void share_object_slots(const SelectorState& state, std::vector<uint32_t>& owner)
{
	const lowir::FunctionBody& body = *state.body_;
	uint32_t candidates[65];
	uint32_t count = 0;
	bool pair = false;
	for (uint32_t s = 0; s < body.slots.size() && count <= 64; ++s)
	{
		if (state.slot_state_[s] != SS_NORMAL || !body.slots[s].type.is_object())
			continue;
		for (uint32_t c = 0; c < count && !pair; ++c)
			pair = body.slots[candidates[c]].type == body.slots[s].type;
		candidates[count++] = s;
	}
	if (!pair || count > 64)
		return;
	const uint32_t blocks = static_cast<uint32_t>(state.blocks_.size());
	std::vector<uint64_t> bit(body.slots.size(), 0);
	for (uint32_t c = 0; c < count; ++c)
		bit[candidates[c]] = uint64_t(1) << c;
	std::vector<uint64_t> named(blocks, 0);
	for (uint32_t b = 0; b < blocks; ++b)
	{
		if (!state.blocks_[b].reachable)
			continue;
		for (lowir::InstId i = body.blocks[state.blocks_[b].id].first; i != lowir::kNoId; i = body.instructions[i].next)
		{
			const lowir::Instruction& inst = body.instructions[i];
			for (unsigned k = 0; k < 3; ++k)
				named[b] |= inst.ops[k].is(lowir::ValueKind::Slot) ? bit[inst.ops[k].index] : 0;
			for (uint32_t k = 0; k < inst.extra_count; ++k)
			{
				const lowir::Value value = body.operands[inst.extra_first + k];
				named[b] |= value.is(lowir::ValueKind::Slot) ? bit[value.index] : 0;
			}
		}
	}
	std::vector<uint64_t> reach(count, 0);
	std::vector<uint32_t> seen(blocks, lowir::kNoId), work;
	for (uint32_t c = 0; c < count; ++c)
	{
		for (uint32_t b = 0; b < blocks; ++b)
		{
			if ((named[b] >> c) & 1)
				seen[b] = c, work.push_back(b);
		}
		while (!work.empty())
		{
			const uint32_t b = work.back();
			work.pop_back();
			reach[c] |= named[b];
			for (uint32_t k = state.succ_first_[b]; k < state.succ_first_[b + 1]; ++k)
			{
				if (seen[state.succs_[k]] != c)
					seen[state.succs_[k]] = c, work.push_back(state.succs_[k]);
			}
		}
	}
	// Each joins the first earlier group of its type none of whose members
	// interferes with it (either way).
	owner.assign(body.slots.size(), lowir::kNoId);
	uint64_t group[64] = { 0 };
	for (uint32_t c = 0; c < count; ++c)
	{
		uint64_t interferes = reach[c];
		for (uint32_t d = 0; d < count; ++d)
			interferes |= ((reach[d] >> c) & 1) << d;
		uint32_t g = c;
		for (uint32_t d = 0; d < c && g == c; ++d)
		{
			if (group[d] != 0 && (group[d] & interferes) == 0 &&
				body.slots[candidates[d]].type == body.slots[candidates[c]].type)
				g = d;
		}
		group[g] |= uint64_t(1) << c;
		owner[candidates[c]] = candidates[g];
	}
}

} // namespace

void FunctionSelector::inst_clobbers(uint32_t id, uint32_t& clobber, uint32_t& early) const
{
	const Instruction& inst = body_->instructions[id];
	clobber = early = 0;
	switch (inst.op)
	{
	case Opcode::Call:
	{
		clobber = kCallerSavedGprs | kAllXmms;
		const lowir::Signature* sig = program_.call_boundary(inst);
		if (sig != nullptr)
		{
			for (uint32_t k = 0; k < inst.extra_count && k < sig->parameter_count; ++k)
			{
				const lowir::Parameter& param = program_.parameters_of(*sig)[k];
				const Type t = param.type;
				if (t.is_object() || t.kind() == TypeKind::F80 || t.kind() == TypeKind::I128)
					early = (1u << RDI) | (1u << RSI) | (1u << RCX);
				// An object value given to a by_address parameter is copied
				// to temporary storage before the arguments are set.
				const Value arg = body_->operands[inst.extra_first + k];
				if (param.pass == ir_model::PPM_BY_ADDRESS && arg.kind == ValueKind::Temp &&
					body_->temps[arg.index].type.is_object())
					early = (1u << RDI) | (1u << RSI) | (1u << RCX);
			}
		}
		if (inst.extra_count > (sig ? sig->parameter_count : 0) || inst.type.is_object())
			early = (1u << RDI) | (1u << RSI) | (1u << RCX);
		break;
	}
	case Opcode::Binary:
	{
		const lowir::BinaryOp op = static_cast<lowir::BinaryOp>(inst.sub);
		if (inst.type.is_float())
			break;
		if (inst.type.kind() == TypeKind::I128)
		{
			if (op == lowir::BinaryOp::Mul)
				clobber = (1u << RAX) | (1u << RDX);
			else if (op == lowir::BinaryOp::Div || op == lowir::BinaryOp::Mod || op == lowir::BinaryOp::UDiv ||
				op == lowir::BinaryOp::UMod)
				clobber = kCallerSavedGprs | kAllXmms;
			else if ((op == lowir::BinaryOp::Shl || op == lowir::BinaryOp::Shr || op == lowir::BinaryOp::UShr) &&
				resolve(inst.ops[1]).kind != ValueKind::Integer)
				clobber = early = 1u << RCX;
			break;
		}
		if (op == lowir::BinaryOp::Div || op == lowir::BinaryOp::Mod || op == lowir::BinaryOp::UDiv ||
			op == lowir::BinaryOp::UMod)
		{
			// The hardware divide and the magic multiplies use rax and rdx
			// (select/emit_divide.cpp); shifts and exact divisions do not.
			int64_t divisor = 0;
			const DivideForm form = divide_form(inst, divisor);
			if (form == DivideForm::Hardware || form == DivideForm::UnsignedMagic || form == DivideForm::SignedMagic)
				clobber = early = (1u << RAX) | (1u << RDX);
		}
		else if ((op == lowir::BinaryOp::Shl || op == lowir::BinaryOp::Shr || op == lowir::BinaryOp::UShr) &&
			resolve(inst.ops[1]).kind != ValueKind::Integer)
			clobber = early = 1u << RCX;
		break;
	}
	case Opcode::Compare:
		if (compare_scratch(inst))
			clobber = early = 1u << kCompareScratch;
		break;
	case Opcode::Convert:
		// An i128/floating conversion calls the runtime's routine.
		if ((inst.type.kind() == TypeKind::I128 && lowir::convert_source_type(inst).is_float()) ||
			(inst.type.is_float() && lowir::convert_source_type(inst).kind() == TypeKind::I128))
			clobber = kCallerSavedGprs | kAllXmms;
		break;
	case Opcode::CopyObject:
		if (!direct_copy(static_cast<uint64_t>(inst.immediate), 1u << inst.sub, level_ >= 1))
			clobber = early = (1u << RDI) | (1u << RSI) | (1u << RCX);
		break;
	case Opcode::Load:
	case Opcode::Store:
	case Opcode::Copy:
		// An object-typed load, store or copy is a copy_bytes of the type's
		// size and alignment (a same-type copy is an alias: no code).
		if (inst.type.is_object() && !(inst.op == Opcode::Copy && (temps_[inst.result].flags & TF_ALIAS)) &&
			!direct_copy(value_bytes(inst.type), value_align(inst.type), level_ >= 1))
			clobber = early = (1u << RDI) | (1u << RSI) | (1u << RCX);
		break;
	case Opcode::ZeroInit:
		if (!always_direct_zero(static_cast<uint64_t>(inst.immediate), level_ >= 1))
			clobber = early = (1u << RDI) | (1u << RCX) | (1u << RAX);
		break;
	case Opcode::AtomicCompareExchange:
		clobber = early = 1u << RAX;
		if (inst.type.kind() == TypeKind::I128)
			clobber = early = (1u << RAX) | (1u << RDX) | (1u << RCX);
		break;
	case Opcode::AtomicLoad:
	case Opcode::AtomicStore:
		if (inst.type.kind() == TypeKind::I128)
			clobber = early = (1u << RAX) | (1u << RDX) | (1u << RCX);
		break;
	case Opcode::Throw:
	case Opcode::Resume:
		clobber = kCallerSavedGprs | kAllXmms;
		break;
	// A catch clause calls the course runtime's matcher (host clauses are
	// LSDA facts).
	case Opcode::EhCatch:
	case Opcode::EhCatchAll:
		if (eh_model_ == EhModel::Course)
			clobber = kCallerSavedGprs | kAllXmms;
		break;
	default:
		break;
	}
}

// An integer comparison materializing its 0/1 result reads an immediate
// operand from the fixed compare scratch register, except for a result of
// at least 32 bits returned directly.
bool FunctionSelector::compare_scratch(const Instruction& inst) const
{
	// The reference compiler's convention, which PA24's exact-shape
	// fixtures pin at O0; at O1+ the compare takes its immediate directly.
	if (level_ >= 1)
		return false;
	const TypeKind kind = inst.type.kind();
	if (inst.type.is_float() || kind == TypeKind::I128 || (temps_[inst.result].flags & (TF_FUSED | TF_DEAD)))
		return false;
	if (resolve(inst.ops[0]).kind != ValueKind::Integer && resolve(inst.ops[1]).kind != ValueKind::Integer)
		return false;
	const TempInfo& t = temps_[inst.result];
	if (t.uses == 1 && t.user != kNoId &&
		(kind == TypeKind::I32 || kind == TypeKind::U32 || kind == TypeKind::I64 || kind == TypeKind::Ptr))
	{
		const Instruction& user = body_->instructions[t.user];
		if (user.op == Opcode::Return && user.ops[0] == Value::temp(inst.result))
			return false;
	}
	return true;
}

// Backward scan of one block: the forbidden-register mask of every value
// defined in it (and of the parameters, for the entry block), and the
// register hint of each value's last use.
void FunctionSelector::prepass_block(uint32_t b)
{
	const lowir::FunctionBody& body = *body_;
	int32_t next_clobber[32];
	int32_t next_early[32];
	for (unsigned r = 0; r < 32; ++r)
		next_clobber[r] = next_early[r] = INT_MAX;
	block_insts_.clear();
	for (lowir::InstId i = body.blocks[blocks_[b].id].first; i != kNoId; i = body.instructions[i].next)
		block_insts_.push_back(i);
	// The registers some instruction below clobbered (the others still
	// hold INT_MAX).
	uint32_t touched = 0;
	auto mask_for = [&](const TempInfo& t) {
		const int32_t last = t.last < t.def ? t.def : t.last;
		uint32_t forbid = last == INT_MAX ? ~touched : 0;
		for (uint32_t m = touched; m != 0; m &= m - 1)
		{
			const unsigned r = static_cast<unsigned>(__builtin_ctz(m));
			if (next_clobber[r] < last || next_early[r] <= last)
				forbid |= 1u << r;
		}
		return forbid;
	};
	auto hint = [&](Value v, Reg reg, int32_t pos) {
		v = resolve(v);
		if (v.kind != ValueKind::Temp)
			return;
		TempInfo& t = temps_[v.index];
		if (t.hint == kNoReg && t.last == pos && t.block == b)
			t.hint = reg;
	};
	// O1+: values whose last use is the count of a variable shift (which
	// loads rcx first): that early clobber is the count's own load, so the
	// count may be computed in rcx.
	std::vector<std::pair<uint32_t, int32_t>> counts;
	for (size_t k = block_insts_.size(); k-- > 0;)
	{
		const uint32_t id = block_insts_[k];
		if (!live(id))
			continue;
		const Instruction& inst = body.instructions[id];
		const int32_t pos = pos_[id];
		if (inst.result != kNoId)
		{
			TempInfo& t = temps_[inst.result];
			if (!(t.flags & (TF_ALIAS | TF_DEAD | TF_LAZY | TF_FOLDED | TF_FUSED | TF_TESTED | TF_ROTATED | TF_RMW)))
			{
				t.forbid = mask_for(t);
				for (const std::pair<uint32_t, int32_t>& c : counts)
					if (c.first == inst.result && c.second == t.last && next_clobber[RCX] == t.last &&
						next_early[RCX] == t.last)
						t.forbid &= ~(1u << RCX);
			}
		}
		const uint32_t clobber = clobber_[id];
		const uint32_t early = early_[id];
		for (uint32_t m = clobber; m != 0; m &= m - 1)
			next_clobber[__builtin_ctz(m)] = pos;
		for (uint32_t m = early; m != 0; m &= m - 1)
			next_early[__builtin_ctz(m)] = pos;
		touched |= clobber | early;
		if (inst.op == Opcode::Return && inst.ops[0].kind != ValueKind::None)
		{
			// An integer two-address result keeps the register it is computed
			// in; other returned values are produced in the return register.
			const Value v = resolve(inst.ops[0]);
			const bool two_address = v.kind == ValueKind::Temp && v.index >= body.parameter_count &&
				body.instructions[body.temps[v.index].def].op == Opcode::Binary && !inst.type.is_float();
			if (!two_address)
				hint(inst.ops[0], inst.type.is_float() ? XMM0 : RAX, pos);
		}
		else if (inst.op == Opcode::Binary && !inst.type.is_float())
		{
			const lowir::BinaryOp op = static_cast<lowir::BinaryOp>(inst.sub);
			if (clobber & (1u << RAX))
				hint(inst.ops[0], RAX, pos);
			else if ((clobber & (1u << RCX)) &&
				(op == lowir::BinaryOp::Shl || op == lowir::BinaryOp::Shr || op == lowir::BinaryOp::UShr))
			{
				hint(inst.ops[1], RCX, pos);
				const Value count = resolve(inst.ops[1]);
				if (level_ >= 1 && count.kind == ValueKind::Temp && !(resolve(inst.ops[0]) == count) &&
					temps_[count.index].last == pos && temps_[count.index].block == b)
					counts.push_back(std::make_pair(count.index, pos));
			}
		}
		else if (inst.op == Opcode::Call)
		{
			const lowir::Signature* sig = program_.call_boundary(inst);
			arg_types_.clear();
			// The emission-time types (call_argument_types) but a variadic
			// literal's: the emission's layout serves then.
			bool emission = sig != nullptr;
			for (uint32_t a = 0; a < inst.extra_count; ++a)
			{
				const Value v = body.operands[inst.extra_first + a];
				const Type declared = sig && a < sig->parameter_count
					? program_.parameters_of(*sig)[a].type
					: (v.kind == ValueKind::Slot ? body.slots[v.index].type : type_of(v));
				arg_types_.push_back(declared);
				if (!(sig && a < sig->parameter_count) && (v.kind == ValueKind::Integer || v.kind == ValueKind::Float))
					emission = false;
			}
			if (!emission)
				classify_arguments(id, sig, call_layouts_.scratch);
			const CallLayout& layout = emission ? emission_layout(id, sig, true) : call_layouts_.scratch;
			for (uint32_t a = 0; a < inst.extra_count; ++a)
			{
				const ArgLayout& arg = layout.args[a];
				// An integer argument is computed in its register when the call
				// is its last use (an object read before the call through it
				// and passed on: a virtual call's `this`) or it is an address
				// formed for the call (O1+: -O0 keeps its shapes).
				const Value v = resolve(body.operands[inst.extra_first + a]);
				const bool direct = v.kind == ValueKind::Temp && v.index >= body.parameter_count &&
					(temps_[v.index].uses == 1 || (level_ >= 1 && temps_[v.index].last == pos) ||
						body.instructions[body.temps[v.index].def].op == Opcode::Addr);
				if (arg.part_count == 1 && layout.parts[arg.first_part].reg != kNoReg &&
					((arg.cls == ArgClass::Gpr && direct) || arg.cls == ArgClass::Xmm))
					hint(body.operands[inst.extra_first + a], layout.parts[arg.first_part].reg, pos);
			}
		}
	}
	if (b != 0)
		retained_uses(b, next_clobber, next_early);
	if (b == 0)
	{
		for (uint32_t p = 0; p < body.parameter_count; ++p)
			temps_[p].forbid = mask_for(temps_[p]);
	}
}

// For each parameter with a frame home used in block b: its last use that
// precedes the first clobber of its incoming register in b (the register
// is read before an instruction's own clobbers, after its early ones).
// Up to there the block may read the parameter from that register.
void FunctionSelector::retained_uses(uint32_t b, const int32_t* first_clobber, const int32_t* first_early)
{
	const lowir::FunctionBody& body = *body_;
	const uint32_t params = body.parameter_count;
	if (params == 0)
		return;
	int32_t* last = &param_last_[static_cast<size_t>(b) * 6];
	auto note = [&](Value v, int32_t pos) {
		v = resolve(v);
		uint32_t parts[2] = { v.kind == ValueKind::Temp ? v.index : kNoId, kNoId };
		if (parts[0] != kNoId && (temps_[parts[0]].flags & TF_LAZY) && temps_[parts[0]].addr != kNoId)
		{
			const AddrExpr& expr = addrs_[temps_[parts[0]].addr];
			parts[0] = expr.kind == AddrExpr::Temp ? expr.base : kNoId;
			parts[1] = expr.index;
		}
		for (unsigned k = 0; k < 2; ++k)
		{
			const uint32_t p = parts[k];
			if (p >= params || !(temps_[p].flags & TF_GLOBAL) || temps_[p].greg != kNoReg)
				continue;
			const Reg reg = incoming_reg(p);
			if (reg == kNoReg || is_xmm(reg))
				continue;
			const unsigned slot = arg_gpr_slot(reg);
			if (pos <= first_clobber[reg] && pos < first_early[reg] && last[slot] < pos)
				last[slot] = pos;
		}
	};
	for (size_t k = 0; k < block_insts_.size(); ++k)
	{
		const uint32_t id = block_insts_[k];
		const Instruction& inst = body.instructions[id];
		if (!live(id) || inst.op == Opcode::Phi || (inst.result != kNoId && (temps_[inst.result].flags & TF_ALIAS)))
			continue;
		const int32_t pos = read_pos(id);
		for_each_operand(body, inst, [&](Value v, OperandRole) { note(v, pos); });
	}
}

// The clobber facts of every live instruction, computed once for the block
// prepasses and the global allocator.
void FunctionSelector::compute_clobbers()
{
	const lowir::FunctionBody& body = *body_;
	clobber_.assign(body.instructions.size(), 0);
	early_.assign(body.instructions.size(), 0);
	param_last_.assign(blocks_.size() * 6, -1);
	avail_out_.assign(blocks_.size(), 0);
	emitted_.assign(blocks_.size(), 0);
	has_eh_ = false;
	has_stack_alloc_ = false;
	for_each_inst([&](uint32_t id, uint32_t) {
		if (!live(id))
			return;
		inst_clobbers(id, clobber_[id], early_[id]);
		const lowir::Opcode op = body.instructions[id].op;
		if (op == Opcode::EhTry || op == Opcode::EhCleanup)
			has_eh_ = true;
		else if (op == Opcode::StackAlloc)
			has_stack_alloc_ = true;
	});
	// At O1 and above handler records go below the frame unless an
	// allocation must outlive a pop (native/runtime/eh_record.h); O0 keeps
	// frame slots, which cost no RSP updates per region (destructor-heavy
	// O0 loops ran 15-27% slower with pushed records).
	out_->eh_records_pushed = level_ >= 1 && has_eh_ && !has_stack_alloc_ && eh_model_ == EhModel::Course;
}

// Frame offsets are 32-bit displacements: a frame stays below 2 GiB with
// room for the preserve area and the final alignment.
int32_t FunctionSelector::frame_alloc(uint64_t bytes, uint32_t align)
{
	if (align < 1)
		align = 1;
	if (align > 16)
		align = 16;
	const uint64_t size = (frame_size_ + bytes + align - 1) & ~static_cast<uint64_t>(align - 1);
	if (size > kMaxFrameBytes)
		throw BackendError("stack frame exceeds the 32-bit displacement range");
	frame_size_ = static_cast<uint32_t>(size);
	frame_objects_.push_back(FrameObject{ -static_cast<int32_t>(frame_size_), static_cast<uint32_t>(bytes), align });
	return -static_cast<int32_t>(frame_size_);
}

void FunctionSelector::add_binding(BindingKind kind, lowir::NameId name, int32_t offset, Type type)
{
	FrameBinding binding;
	binding.kind = kind;
	binding.name = name;
	binding.label = nullptr;
	binding.offset = offset;
	binding.type = type;
	out_->bindings.push_back(binding);
}

void FunctionSelector::layout_slots()
{
	const lowir::FunctionBody& body = *body_;
	std::vector<uint32_t> owner;
	if (level_ >= 1 && !has_eh_)
		share_object_slots(*this, owner);
	auto shares = [&](uint32_t s) { return !owner.empty() && owner[s] != lowir::kNoId && owner[s] != s; };
	auto over_aligned = [&](uint32_t s) { return out_->aligned_base != 0 && value_align(body.slots[s].type) > 16; };
	// Objects aligned beyond 16 (Function::aligned_base): offsets from an
	// aligned base, in a region of a 16-byte guard, the objects and the
	// base's slack; no frame object moves (order_frame) once it exists.
	uint64_t area = 0;
	for (uint32_t s = 0; s < body.slots.size(); ++s)
	{
		if (slot_state_[s] != SS_NORMAL || !over_aligned(s) || shares(s))
			continue;
		const uint64_t align = value_align(body.slots[s].type);
		area = (area + align - 1) & ~(align - 1);
		slot_offset_[s] = static_cast<int32_t>(area);
		area += (static_cast<uint64_t>(value_bytes(body.slots[s].type)) + 7) & ~7ull;
	}
	if (area == 0)
		out_->aligned_base = 0;
	else
	{
		out_->aligned_disp = frame_alloc(area + out_->aligned_base, 16) + 16;
		out_->aligned_bytes = static_cast<uint32_t>(area);
		frame_pinned_ = true;
	}
	for (uint32_t s = 0; s < body.slots.size(); ++s)
	{
		if (slot_state_[s] != SS_NORMAL)
			continue;
		const Type type = body.slots[s].type;
		const uint64_t bytes = (static_cast<uint64_t>(value_bytes(type)) + 7) & ~7ull;
		uint32_t align = value_align(type);
		if (type.is_object() && align < 8)
			align = 8;
		// A sharing slot takes its group's storage (the first member's, laid
		// out before it).
		if (shares(s))
			slot_offset_[s] = slot_offset_[owner[s]];
		else if (over_aligned(s))
			slot_offset_[s] += out_->aligned_disp;
		else
			slot_offset_[s] = frame_alloc(bytes == 0 ? 8 : bytes, align);
		add_binding(BindingKind::Slot, lowir::own_name(body.slots[s]), slot_offset_[s], type);
	}
	// O1+: an object parameter whose one reader copies it whole into a slot
	// in the entry block (no predecessor), before anything else there
	// touches the slot, keeps its home in the slot: the copy vanishes
	// (emit_bulk).
	if (level_ < 1 || blocks_.empty() || preds_first_[1] != preds_first_[0])
		return;
	for (uint32_t p = 0; p < body.parameter_count; ++p)
	{
		TempInfo& t = temps_[p];
		if (t.cls != VClass::Memory || t.uses != 1 || t.user == kNoId || t.home != 0)
			continue;
		const lowir::Instruction& copy = body.instructions[t.user];
		const Value dst = resolve(copy.ops[1]);
		if (copy.op != Opcode::CopyObject || !(resolve(copy.ops[0]) == Value::temp(p)) || inst_block_[t.user] != 0 ||
			copy.immediate != value_bytes(body.temps[p].type) || dst.kind != ValueKind::Temp ||
			temps_[dst.index].addr == kNoId)
			continue;
		const AddrExpr& expr = addrs_[temps_[dst.index].addr];
		if (expr.kind != AddrExpr::Frame || expr.disp != 0 || expr.index != kNoId ||
			slot_state_[expr.base] != SS_NORMAL || value_bytes(body.slots[expr.base].type) != copy.immediate)
			continue;
		const uint32_t slot = expr.base;
		bool touched = false;
		for (lowir::InstId i = body.blocks[blocks_[0].id].first; i != t.user && !touched; i = body.instructions[i].next)
		{
			const lowir::Instruction& inst = body.instructions[i];
			if (inst.op == Opcode::Addr || inst.op == Opcode::Index)
				continue;
			for_each_operand(body, inst, [&](Value v, OperandRole) {
				const Value r = resolve(v);
				if (r.kind == ValueKind::Slot && r.index == slot)
					touched = true;
				else if (r.kind == ValueKind::Temp && temps_[r.index].addr != kNoId &&
					addrs_[temps_[r.index].addr].kind == AddrExpr::Frame && addrs_[temps_[r.index].addr].base == slot)
					touched = true;
			});
		}
		if (touched)
			continue;
		t.home = slot_offset_[slot];
		add_binding(BindingKind::ParamSlot, lowir::own_name(body.temps[p]), t.home, body.temps[p].type);
	}
}

int32_t FunctionSelector::home_of(uint32_t temp)
{
	TempInfo& t = temps_[temp];
	if (t.home != 0)
		return t.home;
	const Type type = body_->temps[temp].type;
	uint64_t bytes = (static_cast<uint64_t>(value_bytes(type)) + 7) & ~7ull;
	if (bytes == 0)
		bytes = 8;
	uint32_t align = value_align(type);
	if (align < 8)
		align = 8;
	t.home = frame_alloc(bytes, align);
	++stats_.homes;
	add_binding(temp < body_->parameter_count ? BindingKind::ParamSlot : BindingKind::Temp,
		lowir::own_name(body_->temps[temp]), t.home, type);
	return t.home;
}

Reg FunctionSelector::allocate(uint32_t temp, uint32_t exclude, Reg prefer0, Reg prefer1)
{
	const TempInfo& t = temps_[temp];
	const uint32_t pool = t.cls == VClass::Xmm ? kAllocatableXmms
		: t.cls == VClass::Gpr                 ? kAllocatableGprs | (rbp_free_ ? 1u << RBP : 0u)
											   : 0;
	const uint32_t allowed = pool & ~t.forbid & ~occupied_ & ~pinned_ & ~exclude & ~excluded_;
	if (allowed == 0)
	{
		note_busy(pool);
		return kNoReg;
	}
	// O1+: a hint naming one of the preferred (dying operand) registers
	// costs nothing there, so it is taken first (a value flowing into a
	// phi computed in the phi's register: register_allocation.cpp).
	const bool hinted = level_ >= 1 && t.hint != kNoReg && t.hint == prefer1;
	const Reg prefs[3] = { hinted ? prefer1 : prefer0, hinted ? prefer0 : prefer1, t.hint };
	for (unsigned k = 0; k < 3; ++k)
		if (prefs[k] != kNoReg && (allowed & (1u << prefs[k])))
			return prefs[k];
	if (t.cls == VClass::Xmm)
	{
		for (Reg r = XMM0; r < XMM0 + 8; ++r)
			if (allowed & (1u << r))
				return r;
		return kNoReg;
	}
	const Reg* order = (t.forbid & kCallerSavedGprs) == (kCallerSavedGprs & kAllocatableGprs) ? kCalleeFirstOrder
		: vector_params_                                                                      ? kVectorBoundaryOrder
																							  : kGprOrder;
	for (unsigned k = 0; k < 13; ++k)
		if (allowed & (1u << order[k]))
			return order[k];
	return kNoReg;
}

void FunctionSelector::bind(uint32_t temp, Reg reg)
{
	if (reg >= 32)
		throw InternalError("a value bound to no register");
	owner_[reg] = temp;
	occupied_ |= 1u << reg;
	++stats_.register_grants;
	temps_[temp].reg = reg;
	temps_[temp].flags &= ~TF_HOMED;
	if ((kCalleeSavedGprs | (1u << RBP)) & (1u << reg))
		used_callee_saved_ |= 1u << reg;
	// A parameter bound to its own incoming register leaves it intact.
	if (temp >= body_->parameter_count || incoming_reg(temp) != reg)
		written_ |= 1u << reg;
}

Reg FunctionSelector::incoming_reg(uint32_t param) const
{
	const ArgLayout& arg = entry_layout_.args[param];
	return arg.part_count == 1 ? entry_layout_.parts[arg.first_part].reg : kNoReg;
}

void FunctionSelector::release(Reg reg)
{
	const uint32_t temp = owner_[reg];
	if (temp != kNoId && temps_[temp].reg == reg)
		temps_[temp].reg = kNoReg;
	owner_[reg] = kNoId;
	occupied_ &= ~(1u << reg);
	++stats_.register_releases;
}

void FunctionSelector::note_busy(uint32_t pool)
{
	// Telemetry only: whether a parameter's value holds one of the class's
	// registers.
#if CPPGM_TELEMETRY_ENABLED
	++stats_.grant_busy;
	for (uint32_t held = pool & occupied_; held != 0; held &= held - 1)
	{
		const uint32_t temp = owner_[__builtin_ctz(held)];
		if (temp != kNoId && temp < body_->parameter_count)
		{
			++stats_.grant_busy_parameters;
			return;
		}
	}
	++stats_.grant_busy_values;
#else
	(void)pool;
#endif
}

bool FunctionSelector::dies_here(uint32_t temp) const
{
	const TempInfo& t = temps_[temp];
	if (t.greg != kNoReg)
		return precise_globals_ ? t.block_last <= position_ : hull_ends(temp, position_);
	return (t.block == block_ ? t.last : t.block_last) <= position_;
}

// Whether a cross-block value is dead after `pos` of the current block: in
// a block that leaves the function, after its last use there. (No hull
// starts in such a block, so its register is then free.)
bool FunctionSelector::hull_ends(uint32_t temp, int32_t pos) const
{
	return exit_block_ && temps_[temp].greg != kNoReg && temps_[temp].block_last <= pos;
}

// The last use in exit block b of each cross-block value bound there.
void FunctionSelector::note_hull_last_uses(uint32_t b)
{
	const lowir::FunctionBody& body = *body_;
	for (uint32_t k = block_globals_first_[b]; k < block_globals_first_[b + 1]; ++k)
		temps_[block_globals_[k]].block_last = -1;
	auto note = [&](Value v, int32_t pos) {
		v = resolve(v);
		uint32_t parts[2] = { v.kind == ValueKind::Temp ? v.index : kNoId, kNoId };
		if (parts[0] != kNoId && (temps_[parts[0]].flags & TF_LAZY) && temps_[parts[0]].addr != kNoId)
		{
			const AddrExpr& expr = addrs_[temps_[parts[0]].addr];
			parts[0] = expr.kind == AddrExpr::Temp ? expr.base : kNoId;
			parts[1] = expr.index;
		}
		for (unsigned k = 0; k < 2; ++k)
			if (parts[k] != kNoId && temps_[parts[k]].greg != kNoReg && temps_[parts[k]].block_last < pos)
				temps_[parts[k]].block_last = pos;
	};
	for (lowir::InstId i = body.blocks[blocks_[b].id].first; i != kNoId; i = body.instructions[i].next)
	{
		const Instruction& inst = body.instructions[i];
		if (!live(i) || inst.op == Opcode::Phi || (inst.result != kNoId && (temps_[inst.result].flags & TF_ALIAS)))
			continue;
		const int32_t pos = read_pos(i);
		for_each_operand(body, inst, [&](Value v, OperandRole) { note(v, pos); });
	}
}

// Frees the registers of values whose last use is at `pos`. Their locations
// stay readable until retire_dying() at the end of the instruction, so an
// instruction can free its dying operands before choosing its result.
void FunctionSelector::release_dying(int32_t pos)
{
	uint32_t live_regs = occupied_;
	while (live_regs != 0)
	{
		const Reg r = static_cast<Reg>(__builtin_ctz(live_regs));
		live_regs &= live_regs - 1;
		const uint32_t temp = owner_[r];
		if (temp == kNoId)
			continue;
		const TempInfo& t = temps_[temp];
		if (pinned_ & (1u << r)
				? t.greg == r && hull_ends(temp, pos)
				: ((precise_globals_ && t.greg == r) || t.block != block_ ? t.block_last : t.last) <= pos)
		{
			owner_[r] = kNoId;
			occupied_ &= ~(1u << r);
			pinned_ &= ~(1u << r);
			written_ |= 1u << r;
			dying_.push_back(temp);
			++stats_.register_releases;
		}
	}
}

void FunctionSelector::retire_dying()
{
	for (size_t i = 0; i < dying_.size(); ++i)
	{
		TempInfo& t = temps_[dying_[i]];
		if (t.reg != kNoReg && owner_[t.reg] != dying_[i])
			t.reg = kNoReg;
	}
	dying_.clear();
}

void FunctionSelector::finish_frame()
{
	// R12 holds the over-aligned objects' base (Function::aligned_base).
	used_callee_saved_ |= reserved_gprs(*out_);
	uint32_t count = __builtin_popcount(used_callee_saved_);
	if (needs_scratch_area_ || scratch_area_)
	{
		out_->scratch_offset = frame_alloc(kScratchAreaBytes, 16);
		out_->scratch_bytes = kScratchAreaBytes;
	}
	// The callee-saved registers occupy the frame's bottom (the prologue
	// pushes them after reserving the rest; any alignment padding lies
	// above them).
	if (count != 0)
	{
		frame_size_ = (frame_size_ + 8 * count + 15) & ~15u;
		out_->preserve_offset = -static_cast<int32_t>(frame_size_);
	}
	out_->preserve_mask = used_callee_saved_;
	out_->stack_size = (frame_size_ + 15) & ~15u;
	out_->duplicate_epilogues = level_ >= 1;
	out_->omit_frame_pointer = level_ >= 1 && eh_model_ == EhModel::Host && !out_->variadic &&
		!out_->eh_records_pushed && out_->aligned_base == 0 && fixed_stack(*out_);
	if ((used_callee_saved_ & (1u << RBP)) && !out_->omit_frame_pointer)
		throw InternalError(std::string("RBP allocated in a function that keeps its frame pointer: ") +
			program_.spelling(program_.symbols[symbol_].name));
}

} // namespace native
