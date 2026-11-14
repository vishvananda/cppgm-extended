#include "native/select/for_each_inst.h"
#include "native/select/function_selector.h"

#include <algorithm>
#include <climits>
#include <string>
#include <utility>

#include "native/backend_error.h"
#include "native/select/bulk_policy.h"
#include "native/select/operand_roles.h"
#include "support/internal_error.h"
#include "support/telemetry.h"
#include "machine/frame_lifetimes.h"

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
// (rbp_free_). The compact orders (-O1 and up) take R13 and R12 after the
// other callee-saved registers: as the base of a memory operand R13 needs a
// displacement byte where there is no displacement, and R12 a SIB byte.
const Reg kGprOrder[] = { R8, R9, RDI, RSI, RDX, RCX, RAX, RBX, R12, R13, R14, R15, RBP };
const Reg kCompactGprOrder[] = { RAX, RCX, RDX, RSI, RDI, R8, R9, RBX, R14, R15, R13, R12, RBP };
const Reg kCalleeFirstOrder[] = { RBX, R12, R13, R14, R15, RBP, R8, R9, RDI, RSI, RDX, RCX, RAX };
const Reg kCompactCalleeFirstOrder[] = { RBX, R14, R15, R13, R12, RBP, R8, R9, RDI, RSI, RDX, RCX, RAX };

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
		if (inst.op == Op::StackAlloc || inst.op == Op::StackSave || inst.op == Op::StackRestore ||
			inst.op == Op::VaStart || inst.op == Op::VaArg ||
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
	return !areas || accesses <= kAreaFrameAccessCap || (f.preserve_mask & (1u << RBP)) != 0;
}

// The most slots of one function that sharing reads: its largest.
const uint32_t kSharedSlots = 512;

// A slot's frame bytes and alignment (layout_slots).
uint64_t slot_bytes(Type type)
{
	const uint64_t bytes = (static_cast<uint64_t>(value_bytes(type)) + 7) & ~7ull;
	return bytes == 0 ? 8 : bytes;
}

uint32_t slot_align(Type type)
{
	const uint32_t align = value_align(type);
	return type.is_object() && align < 8 ? 8 : align;
}

// The edges into host landings, as (from, to) pairs ordered by `from`: a
// call with a landing may enter it, and a resume its next landing
// (FunctionSelector::exceptional_edges).
std::vector<std::pair<uint32_t, uint32_t>> landing_edges(const SelectorState& state)
{
	const lowir::FunctionBody& body = *state.body_;
	std::vector<std::pair<uint32_t, uint32_t>> edges;
	std::vector<uint32_t> landing_block;
	for (uint32_t b = 0; b < state.eh_.eh_landing_index_.size(); ++b)
	{
		const uint32_t index = state.eh_.eh_landing_index_[b];
		if (index == lowir::kNoId)
			continue;
		if (landing_block.size() <= index)
			landing_block.resize(index + 1, lowir::kNoId);
		landing_block[index] = b;
	}
	for (const std::pair<uint32_t, uint32_t>& at : state.landing_sites_)
	{
		const uint32_t site = state.eh_.eh_sites_[at.second];
		const Opcode op = body.instructions[at.second].op;
		uint32_t to = lowir::kNoId;
		if (op == Opcode::Call && site - 1 < landing_block.size())
			to = landing_block[site - 1];
		else if ((op == Opcode::Resume || op == Opcode::Jump) && site != kResumeTerminal)
			to = site - 1;
		if (to != lowir::kNoId && (edges.empty() || edges.back() != std::make_pair(at.first, to)))
			edges.push_back(std::make_pair(at.first, to));
	}
	return edges;
}

// O1+, without course EH regions (their landings are edges this graph
// lacks; host landings are entered along landing_edges): slots that no
// execution uses both of share storage (`owner`: per slot the first slot of
// its group, whose storage it takes; `bytes` and `align`: per first slot,
// what its group needs; left empty when none share). Two interfere when a
// block naming one reaches (or is) a block naming the other; slots live for
// the whole call, so without such a path no value of one survives into a
// use of the other. A slot the entry block names meets every other and is
// left out. The largest slots choose first, each the first group none of
// whose members interferes with it. One set of slots per block, carried
// forward along the edges to a fixed point.
void share_frame_slots(const SelectorState& state, bool landings, std::vector<uint32_t>& owner,
	std::vector<uint64_t>& bytes, std::vector<uint32_t>& align)
{
	const lowir::FunctionBody& body = *state.body_;
	if (state.blocks_.size() < 3)
		return;
	std::vector<uint32_t> position(body.slots.size(), lowir::kNoId);
	for (const std::pair<uint32_t, uint32_t>& named : state.slot_blocks_)
		if (named.second == 0)
			position[named.first] = 0;
	std::vector<uint32_t> candidates;
	for (uint32_t s = 0; s < body.slots.size(); ++s)
		if (state.slot_state_[s] == SS_NORMAL && position[s] == lowir::kNoId && value_align(body.slots[s].type) <= 16)
			candidates.push_back(s);
	if (candidates.size() < 2)
		return;
	std::stable_sort(candidates.begin(), candidates.end(),
		[&](uint32_t x, uint32_t y) { return slot_bytes(body.slots[x].type) > slot_bytes(body.slots[y].type); });
	if (candidates.size() > kSharedSlots)
		candidates.resize(kSharedSlots);
	const uint32_t count = static_cast<uint32_t>(candidates.size());
	const uint32_t words = (count + 63) / 64;
	position.assign(body.slots.size(), lowir::kNoId);
	for (uint32_t c = 0; c < count; ++c)
		position[candidates[c]] = c;
	// Per block: the slots it names, then those named on a path to its end.
	const uint32_t blocks = static_cast<uint32_t>(state.blocks_.size());
	std::vector<uint64_t> named(static_cast<size_t>(blocks) * words, 0);
	std::vector<uint8_t> pending(blocks, 0);
	for (const std::pair<uint32_t, uint32_t>& at : state.slot_blocks_)
	{
		const uint32_t c = position[at.first];
		if (c == lowir::kNoId)
			continue;
		named[static_cast<size_t>(at.second) * words + c / 64] |= uint64_t(1) << (c % 64);
		pending[at.second] = 1;
	}
	std::vector<uint64_t> reach(named);
	std::vector<std::pair<uint32_t, uint32_t>> entered;
	if (landings)
		entered = landing_edges(state);
	// Sweeps in block order: a set that grew at an earlier block (a back
	// edge) takes another sweep.
	bool again = true;
	auto carry = [&](uint32_t b, uint32_t target) {
		const uint64_t* from = &reach[static_cast<size_t>(b) * words];
		uint64_t* to = &reach[static_cast<size_t>(target) * words];
		bool grew = false;
		for (uint32_t w = 0; w < words; ++w)
		{
			grew = grew || (from[w] & ~to[w]) != 0;
			to[w] |= from[w];
		}
		if (grew)
		{
			pending[target] = 1;
			again = again || target <= b;
		}
	};
	while (again)
	{
		again = false;
		auto edge = entered.begin();
		for (uint32_t b = 0; b < blocks; ++b)
		{
			while (edge != entered.end() && edge->first < b)
				++edge;
			if (!pending[b])
				continue;
			pending[b] = 0;
			for (uint32_t k = state.succ_first_[b]; k < state.succ_first_[b + 1]; ++k)
				carry(b, state.succs_[k]);
			for (; edge != entered.end() && edge->first == b; ++edge)
				carry(b, edge->second);
		}
	}
	// Per slot: the slots named where it is, or before on some path; then
	// those it is named before as well.
	std::vector<uint64_t> meets(static_cast<size_t>(count) * words, 0);
	for (uint32_t b = 0; b < blocks; ++b)
		for (uint32_t w = 0; w < words; ++w)
			for (uint64_t left = named[static_cast<size_t>(b) * words + w]; left != 0; left &= left - 1)
			{
				uint64_t* row = &meets[static_cast<size_t>(w * 64 + __builtin_ctzll(left)) * words];
				for (uint32_t v = 0; v < words; ++v)
					row[v] |= reach[static_cast<size_t>(b) * words + v];
			}
	std::vector<uint64_t> met(meets);
	for (uint32_t c = 0; c < count; ++c)
		for (uint32_t w = 0; w < words; ++w)
			for (uint64_t left = meets[static_cast<size_t>(c) * words + w]; left != 0; left &= left - 1)
				met[static_cast<size_t>(w * 64 + __builtin_ctzll(left)) * words + c / 64] |= uint64_t(1) << (c % 64);
	// (group: the candidates of each group, a set per group made.)
	std::vector<uint64_t> group;
	std::vector<uint32_t> joined(count, 0);
	uint32_t groups = 0;
	bool shared = false;
	for (uint32_t c = 0; c < count; ++c)
	{
		const uint64_t* row = &met[static_cast<size_t>(c) * words];
		uint32_t g = 0;
		for (; g < groups; ++g)
		{
			const uint64_t* members = &group[static_cast<size_t>(g) * words];
			uint32_t w = 0;
			while (w < words && (members[w] & row[w]) == 0)
				++w;
			if (w == words)
				break;
		}
		if (g == groups)
			group.resize(static_cast<size_t>(++groups) * words, 0);
		else
			shared = true;
		group[static_cast<size_t>(g) * words + c / 64] |= uint64_t(1) << (c % 64);
		joined[c] = g;
	}
	if (!shared)
		return;
	// Each group's first slot holds its storage: layout meets it first.
	std::vector<uint32_t> first(groups, lowir::kNoId);
	for (uint32_t c = 0; c < count; ++c)
		first[joined[c]] = std::min(first[joined[c]], candidates[c]);
	owner.resize(body.slots.size());
	for (uint32_t s = 0; s < owner.size(); ++s)
		owner[s] = s;
	bytes.assign(body.slots.size(), 0);
	align.assign(body.slots.size(), 0);
	for (uint32_t c = 0; c < count; ++c)
	{
		const uint32_t lead = first[joined[c]];
		owner[candidates[c]] = lead;
		bytes[lead] = std::max(bytes[lead], slot_bytes(body.slots[candidates[c]].type));
		align[lead] = std::max(align[lead], slot_align(body.slots[candidates[c]].type));
	}
}

} // namespace

void FunctionSelector::inst_clobbers(uint32_t id, uint32_t& clobber, uint32_t& early)
{
	const Instruction& inst = body_->instructions[id];
	clobber = early = 0;
	switch (inst.op)
	{
	case Opcode::FrameAddress:
		// Reading the frame reserves RBP for this activation. Record the
		// requirement in the existing register-constraint dispatch.
		frame_addressed_ = true;
		break;
	case Opcode::Asm:
		clobber = assembly_plan(inst)->writes;
		break;
	case Opcode::Call:
	{
		// A direct small copy uses only the bulk operation's reserved
		// scratch registers, without a call or ABI argument staging.
		if (unused_fixed_memcpy_bytes(inst) != kNoId)
			break;
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
		if (level_ >= 1 && inst.result != kNoId && rotate_[inst.result] >= 254)
		{
			clobber = early = 1u << RCX;
			break;
		}
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
	case Opcode::Vector:
		// A blend reads its mask in xmm0.
		if (lowir::shape_of(static_cast<lowir::VectorOp>(inst.sub)) == lowir::VectorShape::Blend)
			clobber = early = 1u << XMM0;
		break;
	case Opcode::Convert:
		// The high half of a 64-bit product multiplies in rdx:rax
		// (find_high_products).
		if (level_ >= 1 && inst.result != kNoId && high_product_[inst.result] != 0)
			clobber = early = (1u << RAX) | (1u << RDX);
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
	// loads rcx first) or a divide's dividend (which loads rax first): that
	// early clobber is the value's own load, so it may be computed there.
	std::vector<std::pair<uint32_t, int32_t>> counts, dividends;
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
				for (const std::pair<uint32_t, int32_t>& d : dividends)
					if (d.first == inst.result && d.second == t.last && next_clobber[RAX] == t.last &&
						next_early[RAX] == t.last)
						t.forbid &= ~(1u << RAX);
			}
		}
		if (inst.op == Opcode::Call && inst.result < incoming_words_.size())
		{
			const WordPair& pair = incoming_words_[inst.result];
			for (Value v : { pair.lo, pair.hi })
				if (v.kind == ValueKind::Temp)
					temps_[v.index].forbid = mask_for(temps_[v.index]);
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
			// A returned value is produced in the return register; so are
			// the values an integer two-address result is computed from.
			const Value v = resolve(inst.ops[0]);
			const bool two_address = v.kind == ValueKind::Temp && v.index >= body.parameter_count &&
				(body.instructions[body.temps[v.index].def].op == Opcode::Binary ||
					body.instructions[body.temps[v.index].def].op == Opcode::Select) &&
				!inst.type.is_float() && !inst.type.is_vector();
			if (two_address && level_ >= 1)
				hint_chain(v, b, RAX);
			else if (!two_address)
				hint(inst.ops[0], inst.type.is_float() || inst.type.is_vector() ? XMM0 : RAX, pos);
		}
		else if (inst.op == Opcode::Binary && !inst.type.is_float())
		{
			const lowir::BinaryOp op = static_cast<lowir::BinaryOp>(inst.sub);
			if (clobber & (1u << RAX))
			{
				hint(inst.ops[0], RAX, pos);
				// A dividend that dies here may be computed in rax for the
				// hardware divide, which reads it there first; the magic
				// multiplies write rax and read the dividend again.
				const Value dividend = resolve(inst.ops[0]);
				int64_t divisor = 0;
				if (level_ >= 1 && (early & (1u << RAX)) && dividend.kind == ValueKind::Temp &&
					divide_form(inst, divisor) == DivideForm::Hardware && !(resolve(inst.ops[1]) == dividend) &&
					temps_[dividend.index].last == pos && temps_[dividend.index].block == b)
					dividends.push_back(std::make_pair(dividend.index, pos));
			}
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
			if (!body.argument_facts.empty())
				machine::apply_argument_types(body, inst, arg_types_);
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
		for (uint32_t p = 0; p < body.parameter_count && p < incoming_words_.size(); ++p)
		{
			const WordPair& pair = incoming_words_[p];
			for (Value v : { pair.lo, pair.hi })
				if (v.kind == ValueKind::Temp)
					temps_[v.index].forbid = mask_for(temps_[v.index]);
		}
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
		else if (lowir::is_stack_instruction(op))
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
	std::vector<uint32_t> owner, shared_align;
	std::vector<uint64_t> shared_bytes;
	if (level_ >= 1 && (!has_eh_ || eh_model_ == EhModel::Host))
		share_frame_slots(*this, has_eh_, owner, shared_bytes, shared_align);
	if (level_ >= 1 && !has_eh_)
	{
		const auto copies = machine::sequential_copy_slots(program_, body);
		if (!copies.empty())
		{
			if (owner.empty())
			{
				owner.resize(body.slots.size());
				for (uint32_t s = 0; s < owner.size(); ++s)
					owner[s] = s;
			}
			std::vector<uint32_t> members(body.slots.size(), 0);
			for (uint32_t s = 0; s < owner.size(); ++s)
				if (owner[s] != kNoId)
					++members[owner[s]];
			for (uint32_t s = 0; s < copies.size(); ++s)
				if (copies[s] != kNoId && copies[s] != s && slot_state_[s] == SS_NORMAL &&
					slot_state_[copies[s]] == SS_NORMAL && members[s] == 1 && members[copies[s]] == 1 &&
					owner[s] == s && owner[copies[s]] == copies[s])
					owner[s] = copies[s];
		}
	}

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
		// A sharing slot takes its group's storage (the first member's, laid
		// out before it with the bytes and alignment the group needs).
		const bool leads = s < shared_bytes.size() && shared_bytes[s] != 0;
		if (shares(s))
			slot_offset_[s] = slot_offset_[owner[s]];
		else if (over_aligned(s))
			slot_offset_[s] += out_->aligned_disp;
		else
			slot_offset_[s] = frame_alloc(
				leads ? shared_bytes[s] : slot_bytes(type), leads ? shared_align[s] : slot_align(type));
		add_binding(BindingKind::Slot, lowir::own_name(body.slots[s]), slot_offset_[s], type);
	}
	if (program_.debug && out_->symbol < program_.debug->functions.size())
		for (const lowir::DebugLocal& local : program_.debug->functions[out_->symbol].locals)
			if (local.slot < slot_state_.size() && slot_state_[local.slot] == SS_NORMAL)
				out_->debug_slots.push_back(DebugSlot{ local.slot, slot_offset_[local.slot] });
	// An object parameter whose one reader copies it whole into a slot
	// in the entry block (no predecessor), before anything else there
	// touches the slot, keeps its home in the slot: the copy vanishes
	// (emit_bulk).
	if (blocks_.empty() || preds_first_[1] != preds_first_[0])
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
	// phi computed in the phi's register: register_allocation.cpp), and so
	// is a firm one (the phi of an edge the block usually leaves by).
	const bool hinted = level_ >= 1 && t.hint != kNoReg && (t.hint == prefer1 || t.firm_hint);
	// For a return, prefer rax: a copy into an add can fuse to a three-
	// address lea, while an add in its dying operand followed by the
	// return's copy cannot use that fusion.
	const bool returned = level_ >= 1 && t.hint == RAX;
	const Reg prefs[3] = { returned ? static_cast<Reg>(RAX)
			: hinted                ? t.hint
									: prefer0,
		returned || hinted ? prefer0 : prefer1, returned || (hinted && t.hint != prefer1) ? prefer1 : t.hint };
	for (unsigned k = 0; k < 3; ++k)
		if (prefs[k] != kNoReg && (allowed & (1u << prefs[k])))
			return prefs[k];
	if (t.cls == VClass::Xmm)
	{
		for (Reg r = XMM0; r < XMM0 + 14; ++r)
			if (allowed & (1u << r))
				return r;
		return kNoReg;
	}
	const bool callee_first = (t.forbid & kCallerSavedGprs) == (kCallerSavedGprs & kAllocatableGprs);
	const Reg* order = level_ >= 1 ? (callee_first ? kCompactCalleeFirstOrder : kCompactGprOrder)
		: callee_first             ? kCalleeFirstOrder
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
	if (param >= body_->parameter_count)
	{
		if (param >= incoming_origin_.size() || incoming_origin_[param] == kNoId)
			return kNoReg;
		const uint32_t origin = incoming_origin_[param], root = origin / 2;
		if (root >= body_->parameter_count)
			return kNoReg;
		const ArgLayout& arg = entry_layout_.args[root];
		return arg.part_count == 2 ? entry_layout_.parts[arg.first_part + origin % 2].reg : kNoReg;
	}
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
	if (needs_scratch_area_)
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
	// A fixed host frame has the same ABI and unwind requirements at every
	// level. Baseline user assembly retains its explicit frame environment.
	out_->omit_frame_pointer = (level_ >= 1 || !out_->assembly) && eh_model_ == EhModel::Host && !out_->variadic &&
		!out_->eh_records_pushed && !frame_addressed_ && out_->aligned_base == 0 && fixed_stack(*out_);
	if ((used_callee_saved_ & (1u << RBP)) && !out_->omit_frame_pointer)
		throw InternalError(std::string("RBP allocated in a function that keeps its frame pointer: ") +
			program_.spelling(program_.symbols[symbol_].name));
}

} // namespace native
