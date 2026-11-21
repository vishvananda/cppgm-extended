#include "aarch64/shrink_wrap.h"

#include <algorithm>

#include "machine/shrink_region.h"

// Shrink-wrapping on selected A64 code, the x86 rule of
// native/select/shrink_wrap.cpp. The blocks the entry reaches without
// passing a block that needs the frame (a call, SP, the frame record, the
// link register, a floating or vector register, or an instruction not
// modelled here) and that no such block reaches back run frameless when one
// of them returns. A callee-saved register they use must be the entry's
// copy of an argument register, which they use in its place; each edge out
// of the region goes through a copy of the prologue that also makes those
// copies and performs the entry's stores to the frame. The region's returns
// lose their restores.
// A framed run that a region block falls into follows it after its prologue
// copy; the other framed blocks keep their order after the region. Frameless
// code has the CIE's rules and framed code the prologue's, restated where
// the code ahead leaves the other state; framed blocks keep their
// epilogues' unwind rules. Nothing in the region calls, so nothing unwinds
// through it.
namespace aarch64 {

namespace {

using machine::ShrinkPlan;

constexpr uint32_t kCopyBudget = 64;
constexpr uint32_t kReturnCopyCap = 4;
constexpr uint32_t kMinimalCopyBudgets[3] = { 16, 8, 4 };

enum Field : uint8_t
{
	kRd,
	kRn,
	kRm,
	kRa // bits 10-14: the addend of a multiply, the second register of a pair
};

struct Operand
{
	Field field;
	bool use, def, sp; // sp: register 31 is SP here (else the zero register)
};

struct Operands
{
	unsigned count = 0;
	Operand list[4];
	void add(Field field, bool use, bool def, bool sp = false) { list[count++] = Operand{ field, use, def, sp }; }
};

unsigned reg_of(const Instruction& ins, Field field)
{
	switch (field)
	{
	case kRd:
		return ins.rd;
	case kRn:
		return ins.rn;
	case kRm:
		return ins.rm;
	case kRa:
		break;
	}
	return ((static_cast<uint32_t>(ins.op) | ins.field) >> 10) & 31;
}

void set_reg(Instruction& ins, Field field, unsigned reg)
{
	if (field == kRd)
		ins.rd = reg;
	else if (field == kRn)
		ins.rn = reg;
	else if (field == kRm)
		ins.rm = reg;
	else
		ins.field = (ins.field & ~(31u << 10)) | (reg << 10);
}

// Integer data processing on general registers.
bool arithmetic_operands(const Instruction& ins, Operands& out)
{
	const bool extended = (ins.field >> 21) & 1; // ADD/SUB (extended register) reads SP
	switch (ins.op)
	{
	case Op::And:
	case Op::AndW:
	case Op::Orr:
	case Op::OrrW:
	case Op::Eor:
	case Op::EorW:
	case Op::OrnW:
	case Op::BicW:
	case Op::Bic:
	case Op::EonW:
	case Op::Eon:
	case Op::Orn:
	case Op::Lsl:
	case Op::Lsr:
	case Op::Asr:
	case Op::LslW:
	case Op::LsrW:
	case Op::AsrW:
	case Op::SDiv:
	case Op::UDiv:
	case Op::SDivW:
	case Op::UDivW:
	case Op::Adc:
	case Op::Sbc:
	case Op::Csel:
	case Op::CselW:
	case Op::Csinc:
	case Op::CsincW:
	case Op::Extr:
	case Op::ExtrW:
		out.add(kRd, false, true);
		out.add(kRn, true, false);
		out.add(kRm, true, false);
		return true;
	case Op::Add:
	case Op::AddW:
	case Op::Sub:
	case Op::SubW:
		out.add(kRd, false, true, extended);
		out.add(kRn, true, false, extended);
		out.add(kRm, true, false);
		return true;
	case Op::Adds:
	case Op::Subs:
	case Op::SubsW:
		out.add(kRd, false, true);
		out.add(kRn, true, false, extended);
		out.add(kRm, true, false);
		return true;
	case Op::Mul:
	case Op::MulW:
	case Op::Smull:
	case Op::Umull:
	case Op::Smulh:
	case Op::Umulh:
	case Op::Madd:
	case Op::MaddW:
	case Op::Msub:
	case Op::MsubW:
		out.add(kRd, false, true);
		out.add(kRn, true, false);
		out.add(kRm, true, false);
		out.add(kRa, true, false);
		return true;
	case Op::Ccmp:
	case Op::CcmpW:
		out.add(kRn, true, false);
		out.add(kRm, true, false);
		return true;
	case Op::CcmpI:
	case Op::CcmpIW:
		out.add(kRn, true, false);
		return true;
	case Op::Rev16:
	case Op::Rev32:
	case Op::Rev64:
	case Op::Rbit:
	case Op::Clz:
	case Op::Sbfm:
	case Op::Ubfm:
	case Op::UbfmW:
	case Op::SbfmW:
		out.add(kRd, false, true);
		out.add(kRn, true, false);
		return true;
	case Op::AndImm:
	case Op::AndImmW:
	case Op::OrrImm:
	case Op::OrrImmW:
	case Op::EorImm:
	case Op::EorImmW:
	case Op::AndI:
		out.add(kRd, false, true, true);
		out.add(kRn, true, false);
		return true;
	case Op::AddI:
	case Op::AddIW:
	case Op::SubI:
	case Op::SubIW:
	case Op::AddPageI:
		out.add(kRd, false, true, true);
		out.add(kRn, true, false, true);
		return true;
	case Op::AddsIW:
	case Op::AddsIX:
	case Op::SubsIW:
	case Op::SubsIX:
		out.add(kRd, false, true);
		out.add(kRn, true, false, true);
		return true;
	case Op::MovZ:
	case Op::MovN:
	case Op::MovNW:
		out.add(kRd, false, true);
		return true;
	case Op::MovK:
		out.add(kRd, true, true);
		return true;
	default:
		return false;
	}
}

// General-register loads and stores without writeback to SP, and control.
bool memory_operands(const Instruction& ins, Operands& out)
{
	switch (ins.op)
	{
	case Op::LdrB:
	case Op::LdrSBW:
	case Op::LdrSHW:
	case Op::LdrH:
	case Op::LdrW:
	case Op::LdrX:
		out.add(kRd, false, true);
		out.add(kRn, true, false, true);
		return true;
	case Op::StrB:
	case Op::StrH:
	case Op::StrW:
	case Op::StrX:
		out.add(kRd, true, false);
		out.add(kRn, true, false, true);
		return true;
	case Op::LdurB:
	case Op::LdurSBW:
	case Op::LdurSHW:
	case Op::LdurH:
	case Op::LdurW:
	case Op::LdurX:
	case Op::SturB:
	case Op::SturH:
	case Op::SturW:
	case Op::SturX:
	{
		const bool load = ins.op == Op::LdurB || ins.op == Op::LdurSBW || ins.op == Op::LdurSHW ||
			ins.op == Op::LdurH || ins.op == Op::LdurW || ins.op == Op::LdurX;
		out.add(kRd, !load, load);
		out.add(kRn, true, ((ins.field >> 10) & 3) != 0, true); // pre- or post-index writes the base
		return true;
	}
	case Op::LdrBReg:
	case Op::LdrSBWReg:
	case Op::LdrSHWReg:
	case Op::LdrHReg:
	case Op::LdrWReg:
	case Op::LdrXReg:
		out.add(kRd, false, true);
		out.add(kRn, true, false, true);
		out.add(kRm, true, false);
		return true;
	case Op::StrBReg:
	case Op::StrHReg:
	case Op::StrWReg:
	case Op::StrXReg:
		out.add(kRd, true, false);
		out.add(kRn, true, false, true);
		out.add(kRm, true, false);
		return true;
	case Op::LdpX:
	case Op::LdpW:
		out.add(kRd, false, true);
		out.add(kRa, false, true);
		out.add(kRn, true, false, true);
		return true;
	case Op::StpX:
	case Op::StpW:
		out.add(kRd, true, false);
		out.add(kRa, true, false);
		out.add(kRn, true, false, true);
		return true;
	case Op::Cbz:
	case Op::CbzW:
	case Op::Cbnz:
	case Op::CbnzW:
	case Op::Tbz:
	case Op::Tbnz:
		out.add(kRd, true, false);
		return true;
	case Op::Ret:
	case Op::Br:
		out.add(kRn, true, false);
		return true;
	case Op::Adrp:
	case Op::ReadThreadPointer:
		out.add(kRd, false, true);
		return true;
	case Op::B:
	case Op::BCond:
	case Op::Brk:
		return true;
	default:
		return false;
	}
}

bool operands(const Instruction& ins, Operands& out)
{
	out.count = 0;
	return arithmetic_operands(ins, out) || memory_operands(ins, out);
}

// Whether an instruction can run before the prologue: no call, SP, frame
// record, link register (but the return through it) or platform register.
bool frameless(const Instruction& ins)
{
	Operands ops;
	if (ins.fixup == Fixup::Call || ins.fixup == Fixup::Resume || ins.op == Op::Adr || !operands(ins, ops))
		return false;
	for (unsigned k = 0; k < ops.count; ++k)
	{
		const unsigned reg = reg_of(ins, ops.list[k].field);
		if (reg == 31 ? ops.list[k].sp : reg == 18 || reg == 29 || (reg == 30 && ins.op != Op::Ret))
			return false;
	}
	return true;
}

// The general registers an instruction names, reads or writes.
uint32_t registers(const Instruction& ins, bool uses, bool defs)
{
	Operands ops;
	if (!operands(ins, ops))
		return 0;
	uint32_t mask = 0;
	for (unsigned k = 0; k < ops.count; ++k)
	{
		const unsigned reg = reg_of(ins, ops.list[k].field);
		if (reg < 31 && ((uses && ops.list[k].use) || (defs && ops.list[k].def)))
			mask |= 1u << reg;
	}
	return mask;
}

bool is_move(const Instruction& ins)
{
	return ins.op == Op::Orr && ins.rn == 31 && ins.field == 0 && ins.fixup == Fixup::None;
}

bool conditional(Op op)
{
	return op == Op::BCond || op == Op::Cbz || op == Op::CbzW || op == Op::Cbnz || op == Op::CbnzW || op == Op::Tbz ||
		op == Op::Tbnz;
}

// The block ends here: a branch, a return or a trap.
bool ends_block(const Instruction& ins)
{
	return ins.op == Op::B || ins.op == Op::Ret || ins.op == Op::Br || ins.op == Op::Brk || conditional(ins.op);
}

bool falls_through(const Instruction& ins)
{
	return ins.op != Op::B && ins.op != Op::Ret && ins.op != Op::Br && ins.op != Op::Brk;
}

// A frameless return's straight-line code: fold small constants into the
// registers it defines, then drop the writes no return reads.
void fold_return(std::vector<Instruction>& code, uint32_t first, uint32_t returned)
{
	const uint32_t end = static_cast<uint32_t>(code.size()) - 1; // the return
	uint64_t value[32];
	uint32_t known = 0;
	for (uint32_t pc = first; pc < end; ++pc)
	{
		Instruction& ins = code[pc];
		const bool add = ins.op == Op::AddI || ins.op == Op::AddIW, sub = ins.op == Op::SubI || ins.op == Op::SubIW;
		if (ins.fixup == Fixup::None && ins.op == Op::MovZ && ins.rd < 31)
		{
			value[ins.rd] = uint64_t((ins.field >> 5) & 65535) << (16 * ((ins.field >> 21) & 3));
			known |= 1u << ins.rd;
			continue;
		}
		if (ins.fixup == Fixup::None && (add || sub) && ins.rd < 31 && ins.rn < 31 && (known & (1u << ins.rn)))
		{
			uint64_t imm = (ins.field >> 10) & 4095;
			if ((ins.field >> 22) & 1)
				imm <<= 12;
			uint64_t result = add ? value[ins.rn] + imm : value[ins.rn] - imm;
			if (ins.op == Op::AddIW || ins.op == Op::SubIW)
				result &= 0xffffffffu;
			if (result < 65536)
				ins = Instruction{ Op::MovZ, ins.rd, 0, 0, static_cast<uint32_t>(result << 5), Fixup::None, 0 };
			value[ins.rd] = result;
			known |= 1u << ins.rd;
			continue;
		}
		known &= ~registers(ins, false, true);
	}
	uint32_t live = returned | ~((1u << 19) - 1);
	std::vector<uint8_t> dead(end - first);
	for (uint32_t pc = end; pc-- > first;)
	{
		const Instruction& ins = code[pc];
		const uint32_t defs = registers(ins, false, true);
		const bool pure = ins.fixup == Fixup::None &&
			(ins.op == Op::MovZ || ins.op == Op::MovN || ins.op == Op::MovNW ||
				((ins.op == Op::Orr || ins.op == Op::OrrW) && ins.rn == 31 && ins.field == 0));
		if (pure && defs && !(defs & live))
		{
			dead[pc - first] = 1;
			continue;
		}
		live = (live & ~defs) | registers(ins, true, false);
	}
	uint32_t out = first;
	for (uint32_t pc = first; pc <= end; ++pc)
		if (pc == end || !dead[pc - first])
			code[out++] = code[pc];
	code.resize(out);
}

void rename(Instruction& ins, const unsigned* map)
{
	Operands ops;
	if (!operands(ins, ops))
		return;
	unsigned regs[4];
	for (unsigned k = 0; k < ops.count; ++k)
		regs[k] = reg_of(ins, ops.list[k].field);
	for (unsigned k = 0; k < ops.count; ++k)
		if (regs[k] < 31)
			set_reg(ins, ops.list[k].field, map[regs[k]]);
}

class Wrapper
{
public:
	Wrapper(Function& f, const FrameCode& frame)
		: f_(f), frame_(frame), n_(static_cast<uint32_t>(f.code.size())), p_(frame.prologue_end)
	{}
	bool run();

private:
	struct Sunk
	{
		uint32_t pc;
		unsigned count, data[2];
	};
	// The new code's pieces, in order: the region's blocks, the frameless
	// copies a branch enters, the prologue copies and the framed blocks.
	enum class Kind : uint8_t
	{
		Region,
		Copy,
		Prologue,
		Framed
	};
	struct Item
	{
		Kind kind;
		uint32_t index; // a block, or an exit for a prologue copy
	};
	bool mark_epilogues();
	bool build_blocks();
	bool block_successors(uint32_t b);
	void find_sunk();
	void classify();
	bool plan(ShrinkPlan mode, uint32_t minimal_budget);
	bool renames(const std::vector<uint8_t>& to_exit);
	void order_items();
	uint32_t label_for(Kind kind, uint32_t index);
	uint32_t target_block(uint32_t label) const { return block_of_[f_.labels[label]]; }
	bool next_is(size_t at, Kind kind, uint32_t index) const
	{
		return at + 1 < items_.size() && items_[at + 1].kind == kind && items_[at + 1].index == index;
	}
	void jump_to(uint32_t label);
	uint32_t frameless_target(uint32_t label);
	void frameless_transfer(uint32_t s, size_t at);
	void emit_frameless(uint32_t b, size_t at);
	void emit_prologue(uint32_t exit, size_t at);
	void emit_framed(uint32_t b, size_t at);
	void set_state(bool framed);
	bool finish();

	Function& f_;
	const FrameCode& frame_;
	const uint32_t n_, p_;
	std::vector<uint8_t> epilogue_;        // pc lies in an epilogue's restores
	std::vector<uint32_t> terminal_owner_; // pc of an epilogue's terminal -> epilogue
	std::vector<uint32_t> block_of_;       // pc of a block's first instruction -> block
	std::vector<uint32_t> first_, end_;
	std::vector<uint8_t> falls_;
	machine::ShrinkGraph graph_;
	std::vector<uint8_t> framed_;
	machine::ShrinkRegion region_;
	std::vector<Sunk> sunk_;
	std::vector<uint32_t> sunk_at_; // pc -> index in sunk_, or kNoId
	unsigned rename_[32];
	std::vector<std::pair<unsigned, unsigned>> copies_; // (callee-saved, its stand-in)
	// Rewriting.
	std::vector<Item> items_;
	std::vector<uint8_t> copy_unit_, prologue_before_;
	std::vector<uint32_t> exit_of_;
	std::vector<uint32_t> start_label_, copy_label_, prologue_label_;
	std::vector<uint32_t> placed_, copy_placed_, prologue_placed_;
	std::vector<Item> homes_; // what each new label names
	std::vector<Instruction> code_;
	std::vector<FrameEvent> events_;
	uint32_t straight_ = 0; // start of the straight-line code that reaches code_'s end
	bool framed_state_ = false;
};

bool Wrapper::mark_epilogues()
{
	const auto& events = f_.frame_events;
	std::vector<uint8_t> owned(events.size());
	if (frame_.prologue_events > events.size())
		return false;
	for (uint32_t e = 0; e < frame_.prologue_events; ++e)
	{
		if (events[e].pc > 4 * p_ || events[e].rule == FrameRule::Register)
			return false;
		owned[e] = 1;
	}
	epilogue_.assign(n_, 0);
	terminal_owner_.assign(n_, lowir::kNoId);
	for (uint32_t k = 0; k < frame_.epilogues.size(); ++k)
	{
		const auto& epilogue = frame_.epilogues[k];
		if (epilogue.end_event <= epilogue.first_event || epilogue.end_event > events.size() ||
			events[epilogue.first_event].rule != FrameRule::RememberState ||
			events[epilogue.end_event - 1].rule != FrameRule::RestoreState)
			return false;
		const uint32_t begin = events[epilogue.first_event].pc / 4,
					   terminal = events[epilogue.end_event - 1].pc / 4 - 1;
		if (begin < p_ || terminal < begin || terminal >= n_ || terminal_owner_[terminal] != lowir::kNoId)
			return false;
		const Instruction& last = f_.code[terminal];
		if (last.op != Op::Ret && last.op != Op::Br && !(last.op == Op::B && last.fixup == Fixup::Jump))
			return false;
		std::fill(epilogue_.begin() + begin, epilogue_.begin() + terminal, 1);
		terminal_owner_[terminal] = k;
		for (uint32_t e = epilogue.first_event; e < epilogue.end_event; ++e)
			owned[e] = 1;
	}
	return std::all_of(owned.begin(), owned.end(), [](uint8_t o) { return o != 0; });
}

bool Wrapper::block_successors(uint32_t b)
{
	auto& succs = graph_.succs[b];
	const Instruction& last = f_.code[end_[b] - 1];
	auto label = [&](uint32_t target) {
		if (target >= f_.labels.size() || f_.labels[target] < p_ || f_.labels[target] >= n_)
			return false;
		succs.push_back(block_of_[f_.labels[target]]);
		return true;
	};
	for (uint32_t pc = first_[b]; pc < end_[b]; ++pc)
	{
		const Instruction& ins = f_.code[pc];
		if (ins.op != Op::Adr || ins.fixup != Fixup::Block)
			continue;
		// A jump table's entries: the branches its label starts.
		if (!label(ins.target))
			return false;
		succs.pop_back();
		for (uint32_t at = f_.labels[ins.target];
			at < n_ && f_.code[at].op == Op::B && f_.code[at].fixup == Fixup::Block; ++at)
			succs.push_back(block_of_[at]);
		graph_.tabled[b] = 1;
	}
	if (last.fixup == Fixup::Block && (last.op == Op::B || conditional(last.op)) && !label(last.target))
		return false;
	if (falls_[b])
	{
		if (end_[b] >= n_)
			return false;
		succs.push_back(block_of_[end_[b]]);
	}
	return true;
}

bool Wrapper::build_blocks()
{
	std::vector<uint8_t> leader(n_ + 1);
	leader[p_] = 1;
	for (uint32_t pc : f_.labels)
		if (pc != lowir::kNoId)
		{
			if (pc < p_ || pc > n_)
				return false;
			leader[pc] = 1;
		}
	for (uint32_t pc = p_; pc < n_; ++pc)
		if (ends_block(f_.code[pc]))
			leader[pc + 1] = 1;
	block_of_.assign(n_ + 1, lowir::kNoId);
	for (uint32_t pc = p_; pc < n_; ++pc)
		if (leader[pc])
		{
			block_of_[pc] = static_cast<uint32_t>(first_.size());
			first_.push_back(pc);
		}
	const uint32_t blocks = static_cast<uint32_t>(first_.size());
	for (uint32_t b = 0; b < blocks; ++b)
	{
		end_.push_back(b + 1 < blocks ? first_[b + 1] : n_);
		falls_.push_back(falls_through(f_.code[end_[b] - 1]));
	}
	graph_.succs.resize(blocks);
	graph_.needs.assign(blocks, 0);
	graph_.tabled.assign(blocks, 0);
	graph_.returns.assign(blocks, 0);
	graph_.short_return.assign(blocks, 0);
	graph_.counts.assign(blocks, 0);
	for (uint32_t b = 0; b < blocks; ++b)
		if (!block_successors(b))
			return false;
	return blocks >= 2;
}

// The entry's stores of registers to the frame move to the prologue copies.
void Wrapper::find_sunk()
{
	sunk_at_.assign(n_, lowir::kNoId);
	for (uint32_t pc = first_[0]; pc < end_[0]; ++pc)
	{
		const Instruction& ins = f_.code[pc];
		const bool single = ins.op == Op::StrX || ins.op == Op::StrW || ins.op == Op::StrH || ins.op == Op::StrB;
		const bool pair = ins.op == Op::StpX || ins.op == Op::StpW;
		if (epilogue_[pc] || ins.fixup != Fixup::None || ins.rn != 31 || (!single && !pair))
			continue;
		Sunk store = { pc, pair ? 2u : 1u, { ins.rd, reg_of(ins, kRa) } };
		bool plain = true;
		for (unsigned k = 0; k < store.count; ++k)
			plain = plain && store.data[k] != 18 && store.data[k] != 29 && store.data[k] != 30;
		if (!plain)
			continue;
		sunk_at_[pc] = static_cast<uint32_t>(sunk_.size());
		sunk_.push_back(store);
	}
}

void Wrapper::classify()
{
	for (uint32_t b = 0; b < first_.size(); ++b)
	{
		for (uint32_t pc = first_[b]; pc < end_[b]; ++pc)
		{
			if (epilogue_[pc])
				continue;
			++graph_.counts[b];
			if (sunk_at_[pc] == lowir::kNoId && !frameless(f_.code[pc]))
				graph_.needs[b] = 1;
		}
		// A return or an indirect branch the selector did not end a frame
		// with stays framed.
		const Instruction& last = f_.code[end_[b] - 1];
		graph_.returns[b] = terminal_owner_[end_[b] - 1] != lowir::kNoId;
		if (!graph_.returns[b] && (last.op == Op::Ret || last.op == Op::Br || last.fixup == Fixup::Jump))
			graph_.needs[b] = 1;
		graph_.needs[b] |= graph_.tabled[b];
		graph_.short_return[b] = !graph_.needs[b] && graph_.counts[b] <= kReturnCopyCap && graph_.returns[b];
	}
}

// Each callee-saved register the region names must be one it sets just
// once on the way to an exit, first thing it does with it, from an incoming
// register it never writes: that register takes its place (a leaf block may
// write it again when no leaf block names the incoming register itself).
// Any other register in its place would cost the framed paths a copy, which
// measured worse. The registers of the entry's moved stores must reach the
// exits unchanged.
bool Wrapper::renames(const std::vector<uint8_t>& to_exit)
{
	uint32_t names = 0, written = 0, leaf_names = 0;
	uint32_t writes[32] = {}, exit_writes[32] = {};
	for (uint32_t b = 0; b < first_.size(); ++b)
	{
		if (!region_.region[b] && !region_.copied[b])
			continue;
		for (uint32_t pc = first_[b]; pc < end_[b]; ++pc)
		{
			if (epilogue_[pc])
				continue;
			const Instruction& ins = f_.code[pc];
			const uint32_t named = registers(ins, true, true);
			names |= named;
			if (!to_exit[b])
				leaf_names |= named;
			for (uint32_t defs = registers(ins, false, true); defs; defs &= defs - 1)
			{
				const unsigned reg = __builtin_ctz(defs);
				++writes[reg];
				if (to_exit[b])
				{
					written |= 1u << reg;
					++exit_writes[reg];
				}
			}
		}
	}
	for (unsigned r = 0; r < 32; ++r)
		rename_[r] = r;
	copies_.clear();
	for (unsigned c = 19; c <= 28; ++c)
	{
		if (!(names & (1u << c)))
			continue;
		uint32_t at = first_[0];
		while (at < end_[0] && (epilogue_[at] || !(registers(f_.code[at], true, true) & (1u << c))))
			++at;
		if (at == end_[0] || exit_writes[c] != 1)
			return false;
		const Instruction& ins = f_.code[at];
		const unsigned source = ins.rm;
		if (!is_move(ins) || ins.rd != c || source >= 31 || !(frame_.incoming & (1u << source)) ||
			writes[source] != 0 || (writes[c] != 1 && (leaf_names & (1u << source))))
			return false;
		rename_[c] = source;
		copies_.push_back({ c, source });
	}
	for (const auto& store : sunk_)
		for (unsigned k = 0; k < store.count; ++k)
			if (store.data[k] < 31 && (written & (1u << store.data[k])))
				return false;
	return true;
}

bool Wrapper::plan(ShrinkPlan mode, uint32_t minimal_budget)
{
	if (!machine::shrink_region(graph_, framed_, mode, kCopyBudget, minimal_budget, region_))
		return false;
	std::vector<uint8_t> to_exit;
	machine::exit_reaching(graph_, region_, to_exit);
	return renames(to_exit);
}

// The region's blocks, each followed by the copy it falls into; the copies
// a conditional branch enters (a short return an unconditional one enters
// goes inline); after a frameless block that falls into an exit, that
// exit's prologue copy (and after a region block the framed blocks that
// fall into one another from there); a prologue copy per other exit (before
// its target when the framed block ahead of it does not fall through); the
// other blocks with their frame in their own order.
void Wrapper::order_items()
{
	const uint32_t blocks = static_cast<uint32_t>(first_.size());
	exit_of_.assign(blocks, lowir::kNoId);
	for (uint32_t e = 0; e < region_.exits.size(); ++e)
		exit_of_[region_.exits[e]] = e;
	copy_unit_.assign(blocks, 0);
	std::vector<uint32_t> work;
	for (uint32_t b = 0; b < blocks; ++b)
		if (region_.region[b])
			work.push_back(b);
	while (!work.empty())
	{
		const uint32_t b = work.back();
		work.pop_back();
		const Instruction& last = f_.code[end_[b] - 1];
		auto enter = [&](uint32_t s, bool inline_short) {
			if (region_.copied[s] && !region_.region[s] && !copy_unit_[s] && !(inline_short && graph_.short_return[s]))
			{
				copy_unit_[s] = 1;
				work.push_back(s);
			}
		};
		if (last.fixup == Fixup::Block && conditional(last.op))
			enter(target_block(last.target), false);
		if (last.fixup == Fixup::Block && last.op == Op::B)
			enter(target_block(last.target), true);
		if (falls_[b])
			enter(b + 1, true);
	}
	items_.clear();
	// A copy a block's conditional branch enters follows that block when
	// the block leaves for a prologue copy or a return anyway: the branch
	// then inverts onto that jump.
	std::vector<uint8_t> placed(blocks), framed_placed(blocks), prologue_placed(region_.exits.size());
	// A frameless block that falls into an exit: that exit's prologue copy
	// follows it, and so do the framed blocks from the exit on when nothing
	// framed falls into the exit (a region block comes before it).
	auto after = [&](uint32_t b) {
		const uint32_t s = b + 1;
		if (!falls_[b] || s >= blocks || region_.region[s] || region_.copied[s] || exit_of_[s] == lowir::kNoId ||
			prologue_placed[exit_of_[s]])
			return false;
		prologue_placed[exit_of_[s]] = 1;
		items_.push_back(Item{ Kind::Prologue, exit_of_[s] });
		for (uint32_t f = s; region_.region[b] && !framed_placed[f]; ++f)
		{
			framed_placed[f] = 1;
			items_.push_back(Item{ Kind::Framed, f });
			if (!falls_[f])
				break;
		}
		return true;
	};
	auto follow = [&](uint32_t b) {
		for (;;)
		{
			const Instruction& last = f_.code[end_[b] - 1];
			if (last.fixup != Fixup::Block || !conditional(last.op))
				return;
			const uint32_t s = target_block(last.target), next = b + 1;
			if (!copy_unit_[s] || placed[s] || region_.region[next] ||
				(region_.copied[next] && !graph_.short_return[next]))
				return;
			placed[s] = 1;
			items_.push_back(Item{ Kind::Copy, s });
			if (after(s))
				return;
			b = s;
		}
	};
	// What a frameless block falls into comes next where it can.
	auto chain = [&](uint32_t b) {
		while (!after(b))
		{
			const uint32_t s = b + 1;
			if (!falls_[b] || s >= blocks || !copy_unit_[s] || placed[s])
			{
				follow(b);
				return;
			}
			placed[s] = 1;
			items_.push_back(Item{ Kind::Copy, s });
			b = s;
		}
	};
	for (uint32_t b = 0; b < blocks; ++b)
		if (region_.region[b])
		{
			items_.push_back(Item{ Kind::Region, b });
			chain(b);
		}
	for (uint32_t b = 0; b < blocks; ++b)
		if (copy_unit_[b] && !placed[b])
		{
			placed[b] = 1;
			items_.push_back(Item{ Kind::Copy, b });
			chain(b);
		}
	prologue_before_.assign(region_.exits.size(), 0);
	uint32_t previous = lowir::kNoId;
	for (uint32_t b = 0; b < blocks; ++b)
	{
		if (region_.region[b] || framed_placed[b])
			continue;
		if (exit_of_[b] != lowir::kNoId)
			prologue_before_[exit_of_[b]] = previous == lowir::kNoId || !falls_[previous];
		previous = b;
	}
	for (uint32_t e = 0; e < region_.exits.size(); ++e)
		if (!prologue_before_[e] && !prologue_placed[e])
			items_.push_back(Item{ Kind::Prologue, e });
	for (uint32_t b = 0; b < blocks; ++b)
	{
		if (region_.region[b] || framed_placed[b])
			continue;
		if (exit_of_[b] != lowir::kNoId && prologue_before_[exit_of_[b]] && !prologue_placed[exit_of_[b]])
			items_.push_back(Item{ Kind::Prologue, exit_of_[b] });
		items_.push_back(Item{ Kind::Framed, b });
	}
}

uint32_t Wrapper::label_for(Kind kind, uint32_t index)
{
	uint32_t& slot = kind == Kind::Copy ? copy_label_[index]
		: kind == Kind::Prologue        ? prologue_label_[index]
										: start_label_[index];
	if (slot == lowir::kNoId)
	{
		slot = static_cast<uint32_t>(f_.labels.size() + homes_.size());
		homes_.push_back(Item{ kind == Kind::Region ? Kind::Framed : kind, index });
	}
	return slot;
}

void Wrapper::jump_to(uint32_t label)
{
	code_.push_back(Instruction{ Op::B, 0, 0, 0, 0, Fixup::Block, label });
	straight_ = static_cast<uint32_t>(code_.size());
}

// Where a frameless branch to an original label goes: the region block,
// the block's frameless copy or the prologue copy of that exit.
uint32_t Wrapper::frameless_target(uint32_t label)
{
	const uint32_t s = target_block(label);
	if (region_.region[s])
		return label;
	if (region_.copied[s])
		return label_for(Kind::Copy, s);
	return label_for(Kind::Prologue, exit_of_[s]);
}

void Wrapper::frameless_transfer(uint32_t s, size_t at)
{
	if (region_.region[s] && next_is(at, Kind::Region, s))
		return;
	if ((region_.region[s] || region_.copied[s]) && graph_.short_return[s])
		emit_frameless(s, at);
	else if (region_.region[s])
		jump_to(label_for(Kind::Region, s));
	else if (region_.copied[s])
	{
		if (!next_is(at, Kind::Copy, s))
			jump_to(label_for(Kind::Copy, s));
	}
	else if (!next_is(at, Kind::Prologue, exit_of_[s]))
		jump_to(label_for(Kind::Prologue, exit_of_[s]));
}

// A region block or a copy, renamed, with no frame, no restores and none of
// the entry's moved stores.
void Wrapper::emit_frameless(uint32_t b, size_t at)
{
	for (uint32_t pc = first_[b]; pc < end_[b]; ++pc)
	{
		if (epilogue_[pc])
			continue;
		Instruction ins = f_.code[pc];
		if (b == 0 && sunk_at_[pc] != lowir::kNoId)
			continue;
		if (pc + 1 == end_[b] && ins.op == Op::B && ins.fixup == Fixup::Block)
		{
			frameless_transfer(target_block(ins.target), at);
			return;
		}
		rename(ins, rename_);
		if (ins.fixup == Fixup::Block)
			ins.target = frameless_target(ins.target);
		if (is_move(ins) && ins.rd == ins.rm)
			continue;
		code_.push_back(ins);
		if (ins.op == Op::Ret)
			fold_return(code_, straight_, frame_.returned);
		if (ends_block(ins))
			straight_ = static_cast<uint32_t>(code_.size());
	}
	if (falls_[b])
		frameless_transfer(b + 1, at);
}

// The prologue (after a framed piece, from the CIE's rules again), the
// callee-saved registers back from their stand-ins, the entry's stores.
void Wrapper::emit_prologue(uint32_t exit, size_t at)
{
	set_state(false);
	const uint32_t base = static_cast<uint32_t>(4 * code_.size());
	prologue_placed_[exit] = static_cast<uint32_t>(code_.size());
	code_.insert(code_.end(), f_.code.begin(), f_.code.begin() + p_);
	for (uint32_t e = 0; e < frame_.prologue_events; ++e)
	{
		FrameEvent event = f_.frame_events[e];
		event.pc += base;
		events_.push_back(event);
	}
	for (const auto& copy : copies_)
		code_.push_back(Instruction{ Op::Orr, copy.first, 31, copy.second, 0, Fixup::None, 0 });
	for (const auto& store : sunk_)
		code_.push_back(f_.code[store.pc]);
	framed_state_ = true;
	const uint32_t s = region_.exits[exit];
	if (!next_is(at, Kind::Framed, s))
		jump_to(label_for(Kind::Framed, s));
	straight_ = static_cast<uint32_t>(code_.size());
}

void Wrapper::emit_framed(uint32_t b, size_t at)
{
	set_state(true);
	const uint32_t base = static_cast<uint32_t>(code_.size());
	placed_[b] = base;
	code_.insert(code_.end(), f_.code.begin() + first_[b], f_.code.begin() + end_[b]);
	const uint32_t owner = terminal_owner_[end_[b] - 1];
	if (owner != lowir::kNoId)
		for (uint32_t e = frame_.epilogues[owner].first_event; e < frame_.epilogues[owner].end_event; ++e)
		{
			FrameEvent event = f_.frame_events[e];
			event.pc = event.pc - 4 * first_[b] + 4 * base;
			events_.push_back(event);
		}
	framed_state_ = true;
	if (falls_[b] && !next_is(at, Kind::Framed, b + 1))
		jump_to(label_for(Kind::Framed, b + 1));
	straight_ = static_cast<uint32_t>(code_.size());
}

// The unwind rules where a piece starts that the code ahead of it leaves in
// the other state: the CIE's for frameless code, the prologue's for framed.
void Wrapper::set_state(bool framed)
{
	if (framed_state_ == framed)
		return;
	const uint32_t pc = static_cast<uint32_t>(4 * code_.size());
	if (!framed)
		events_.push_back(FrameEvent{ pc, FrameRule::Offset, 31, 0 });
	for (uint32_t e = 0; e < frame_.prologue_events; ++e)
	{
		const FrameEvent& event = f_.frame_events[e];
		if (framed)
			events_.push_back(FrameEvent{ pc, event.rule, event.reg, event.offset });
		else if (event.rule == FrameRule::Save)
			events_.push_back(FrameEvent{ pc, FrameRule::Restore, event.reg, 0 });
	}
	framed_state_ = framed;
}

// Labels at their new places, branches within reach, then the new code.
bool Wrapper::finish()
{
	const uint32_t size = static_cast<uint32_t>(code_.size());
	if (size >= (1u << 18))
		return false;
	std::vector<uint32_t> labels(f_.labels.size() + homes_.size(), lowir::kNoId);
	for (uint32_t label = 0; label < f_.labels.size(); ++label)
	{
		const uint32_t pc = f_.labels[label];
		labels[label] = pc == lowir::kNoId ? pc : pc == n_ ? size : placed_[block_of_[pc]];
	}
	for (uint32_t k = 0; k < homes_.size(); ++k)
	{
		const Item home = homes_[k];
		labels[f_.labels.size() + k] = home.kind == Kind::Copy ? copy_placed_[home.index]
			: home.kind == Kind::Prologue                      ? prologue_placed_[home.index]
															   : placed_[home.index];
	}
	for (uint32_t pc = 0; pc < size; ++pc)
	{
		const Instruction& ins = code_[pc];
		if (ins.fixup != Fixup::Block)
			continue;
		if (ins.target >= labels.size() || labels[ins.target] == lowir::kNoId)
			return false;
		const int64_t words = int64_t(labels[ins.target]) - pc;
		const int64_t limit = ins.op == Op::B ? 1 << 25 : (ins.op == Op::Tbz || ins.op == Op::Tbnz) ? 1 << 13 : 1 << 18;
		if (words < -limit || words >= limit)
			return false;
	}
	f_.code = std::move(code_);
	f_.labels = std::move(labels);
	f_.frame_events = std::move(events_);
	return true;
}

bool Wrapper::run()
{
	if (p_ == 0 || p_ >= n_ || n_ >= (1u << 18) || !mark_epilogues() || !build_blocks())
		return false;
	find_sunk();
	classify();
	machine::framed_blocks(graph_, framed_);
	if (framed_[0])
		return false;
	if (!plan(ShrinkPlan::Wide, 0) && !plan(ShrinkPlan::Narrow, 0) &&
		!plan(ShrinkPlan::Minimal, kMinimalCopyBudgets[0]) && !plan(ShrinkPlan::Minimal, kMinimalCopyBudgets[1]) &&
		!plan(ShrinkPlan::Minimal, kMinimalCopyBudgets[2]))
		return false;
	order_items();
	const uint32_t blocks = static_cast<uint32_t>(first_.size());
	start_label_.assign(blocks, lowir::kNoId);
	copy_label_.assign(blocks, lowir::kNoId);
	prologue_label_.assign(region_.exits.size(), lowir::kNoId);
	placed_.assign(blocks, lowir::kNoId);
	copy_placed_.assign(blocks, lowir::kNoId);
	prologue_placed_.assign(region_.exits.size(), lowir::kNoId);
	for (size_t at = 0; at < items_.size(); ++at)
	{
		const Item item = items_[at];
		if (item.kind == Kind::Region || item.kind == Kind::Copy)
		{
			set_state(false);
			(item.kind == Kind::Region ? placed_ : copy_placed_)[item.index] = static_cast<uint32_t>(code_.size());
			straight_ = static_cast<uint32_t>(code_.size());
			emit_frameless(item.index, at);
		}
		else if (item.kind == Kind::Prologue)
			emit_prologue(item.index, at);
		else
			emit_framed(item.index, at);
	}
	return finish();
}

} // namespace

bool shrink_wrap(Function& function, const FrameCode& frame)
{
	return Wrapper(function, frame).run();
}

bool general_operands(const Instruction& ins, uint32_t& uses, uint32_t& defs)
{
	Operands ops;
	uses = defs = 0;
	if (ins.fixup == Fixup::Call || ins.fixup == Fixup::Resume || !operands(ins, ops))
		return false;
	for (unsigned k = 0; k < ops.count; ++k)
	{
		const unsigned reg = reg_of(ins, ops.list[k].field);
		const uint32_t bit = reg < 31 ? 1u << reg : ops.list[k].sp ? 1u << 31 : 0;
		if (ops.list[k].use)
			uses |= bit;
		if (ops.list[k].def)
			defs |= bit;
	}
	return true;
}

bool rename_reads(Instruction& ins, unsigned from, unsigned to)
{
	Operands ops;
	if (from >= 31 || to >= 31 || ins.fixup == Fixup::Call || ins.fixup == Fixup::Resume || !operands(ins, ops))
		return false;
	bool read = false;
	for (unsigned k = 0; k < ops.count; ++k)
		if (reg_of(ins, ops.list[k].field) == from)
		{
			if (ops.list[k].def && ops.list[k].use)
				return false;
			read |= ops.list[k].use;
		}
	if (!read)
		return false;
	Instruction renamed = ins;
	for (unsigned k = 0; k < ops.count; ++k)
		if (ops.list[k].use && reg_of(ins, ops.list[k].field) == from)
			set_reg(renamed, ops.list[k].field, to);
	ins = renamed;
	return true;
}

} // namespace aarch64
