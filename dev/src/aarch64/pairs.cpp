#include "aarch64/mir.h"

#include <algorithm>

// Load and store pairs on selected A64 code. Two single loads, or two single
// stores, of one size and register bank from one base register at adjacent
// offsets become one LDP or STP when the instructions between them leave
// the base, the moved data register and memory alone. The rule follows
// Clang's AArch64 load/store optimizer: the pair takes the place of the
// first load (its second destination is then written earlier) or of either
// store (whose data register must hold the same value at that place).
// Only instructions with a modelled effect may stand between the two; any
// other instruction, a branch, a call or a block start ends the search.
namespace aarch64 {
namespace {

constexpr uint32_t kWindow = 12;
constexpr uint32_t kPageWindow = 64;
constexpr uint32_t kSp = 1u << 31;

// The registers and memory one instruction touches. General registers are
// bits 0-30 of the gp masks and SP is bit 31; register 31 read or written as
// the zero register sets no bit. SIMD and floating registers are the fp masks.
struct Effects
{
	uint32_t gp_use = 0, gp_def = 0, fp_use = 0, fp_def = 0;
	bool reads = false, writes = false;
	void add(const Effects& other)
	{
		gp_use |= other.gp_use;
		gp_def |= other.gp_def;
		fp_use |= other.fp_use;
		fp_def |= other.fp_def;
		reads |= other.reads;
		writes |= other.writes;
	}
};

uint32_t gp(unsigned reg, bool sp = false)
{
	return reg < 31 ? 1u << reg : sp ? kSp : 0;
}

uint32_t fp(unsigned reg)
{
	return 1u << (reg & 31);
}

// Bits 10-14: a multiply's addend or a pair's second register.
unsigned ra_of(const Instruction& ins)
{
	return ((static_cast<uint32_t>(ins.op) | ins.field) >> 10) & 31;
}

bool integer_effects(const Instruction& ins, Effects& e)
{
	const bool extended = (ins.field >> 21) & 1; // ADD/SUB (extended register) can name SP
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
	case Op::Adds:
	case Op::Subs:
	case Op::SubsW:
		e.gp_def = gp(ins.rd);
		e.gp_use = gp(ins.rn, extended) | gp(ins.rm);
		return true;
	case Op::Add:
	case Op::AddW:
	case Op::Sub:
	case Op::SubW:
		e.gp_def = gp(ins.rd, extended);
		e.gp_use = gp(ins.rn, extended) | gp(ins.rm);
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
		e.gp_def = gp(ins.rd);
		e.gp_use = gp(ins.rn) | gp(ins.rm) | gp(ra_of(ins));
		return true;
	case Op::Ccmp:
	case Op::CcmpW:
		e.gp_use = gp(ins.rn) | gp(ins.rm);
		return true;
	case Op::CcmpI:
	case Op::CcmpIW:
		e.gp_use = gp(ins.rn);
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
		e.gp_def = gp(ins.rd);
		e.gp_use = gp(ins.rn);
		return true;
	case Op::AndImm:
	case Op::AndImmW:
	case Op::OrrImm:
	case Op::OrrImmW:
	case Op::EorImm:
	case Op::EorImmW:
	case Op::AndI:
		e.gp_def = gp(ins.rd, true);
		e.gp_use = gp(ins.rn);
		return true;
	case Op::AddI:
	case Op::AddIW:
	case Op::SubI:
	case Op::SubIW:
	case Op::AddPageI:
		e.gp_def = gp(ins.rd, true);
		e.gp_use = gp(ins.rn, true);
		return true;
	case Op::AddsIW:
	case Op::AddsIX:
	case Op::SubsIW:
	case Op::SubsIX:
		e.gp_def = gp(ins.rd);
		e.gp_use = gp(ins.rn, true);
		return true;
	case Op::MovZ:
	case Op::MovN:
	case Op::MovNW:
	case Op::Adrp:
		e.gp_def = gp(ins.rd);
		return true;
	case Op::MovK:
		e.gp_def = e.gp_use = gp(ins.rd);
		return true;
	case Op::Bfm:
	case Op::BfmW:
		e.gp_def = gp(ins.rd);
		e.gp_use = gp(ins.rd) | gp(ins.rn);
		return true;
	default:
		return false;
	}
}

// Scalar floating-point arithmetic, moves between the banks and the plain
// vector operations, whose only registers are rd, rn and rm (rd is taken as
// read as well, which covers the accumulating and inserting forms).
bool floating_effects(const Instruction& ins, Effects& e)
{
	switch (ins.op)
	{
	case Op::Fadd:
	case Op::Fsub:
	case Op::Fmul:
	case Op::Fdiv:
	case Op::Fcsel:
	case Op::Fminnm:
	case Op::Fmaxnm:
	case Op::Fabd:
		e.fp_def = fp(ins.rd);
		e.fp_use = fp(ins.rn) | fp(ins.rm);
		return true;
	case Op::Fmadd:
		e.fp_def = fp(ins.rd);
		e.fp_use = fp(ins.rn) | fp(ins.rm) | fp(ra_of(ins));
		return true;
	case Op::Fneg:
	case Op::Fabs:
	case Op::Fsqrt:
	case Op::FcvtDS:
	case Op::FcvtSD:
	case Op::FmovFP:
		e.fp_def = fp(ins.rd);
		e.fp_use = fp(ins.rn);
		return true;
	case Op::FmovI:
		e.fp_def = fp(ins.rd);
		return true;
	case Op::Fcmp:
		e.fp_use = fp(ins.rn) | fp(ins.rm);
		return true;
	case Op::FcmpZero:
		e.fp_use = fp(ins.rn);
		return true;
	case Op::FmovWS:
	case Op::FmovXD:
	case Op::Fcvtzs:
	case Op::Fcvtzu:
		e.gp_def = gp(ins.rd);
		e.fp_use = fp(ins.rn);
		return true;
	case Op::FmovSW:
	case Op::FmovDX:
	case Op::Scvtf:
	case Op::Ucvtf:
	case Op::ScvtfW:
	case Op::UcvtfW:
		e.fp_def = fp(ins.rd);
		e.gp_use = gp(ins.rn);
		return true;
	case Op::VAdd:
	case Op::VSub:
	case Op::VMul:
	case Op::VAnd:
	case Op::VOrr:
	case Op::VEor:
	case Op::VBic:
	case Op::VCmeq:
	case Op::VCmgt:
	case Op::VFadd:
	case Op::VFsub:
	case Op::VFmul:
	case Op::VFmulElem:
	case Op::VFdiv:
	case Op::VFcmgt:
	case Op::VBsl:
	case Op::VBit:
	case Op::VBif:
	case Op::VSmax:
	case Op::VUmax:
	case Op::VSmin:
	case Op::VUmin:
	case Op::VMla:
	case Op::VZip1:
	case Op::VZip2:
	case Op::VUzp1:
		e.fp_def = fp(ins.rd);
		e.fp_use = fp(ins.rd) | fp(ins.rn) | fp(ins.rm);
		return true;
	default:
		return false;
	}
}

// A load or store with an immediate or register offset. Unscaled forms
// with writeback also write their base.
bool memory_effects(const Instruction& ins, Effects& e)
{
	struct Form
	{
		Op op;
		bool load, simd, pair, unscaled, indexed;
	};
	static const Form forms[] = { { Op::LdrB, true, false, false, false, false },
		{ Op::LdrSBW, true, false, false, false, false }, { Op::LdrSHW, true, false, false, false, false },
		{ Op::LdrH, true, false, false, false, false }, { Op::LdrW, true, false, false, false, false },
		{ Op::LdrX, true, false, false, false, false }, { Op::StrB, false, false, false, false, false },
		{ Op::StrH, false, false, false, false, false }, { Op::StrW, false, false, false, false, false },
		{ Op::StrX, false, false, false, false, false }, { Op::LdurB, true, false, false, true, false },
		{ Op::LdurSBW, true, false, false, true, false }, { Op::LdurSHW, true, false, false, true, false },
		{ Op::LdurH, true, false, false, true, false }, { Op::LdurW, true, false, false, true, false },
		{ Op::LdurX, true, false, false, true, false }, { Op::SturB, false, false, false, true, false },
		{ Op::SturH, false, false, false, true, false }, { Op::SturW, false, false, false, true, false },
		{ Op::SturX, false, false, false, true, false }, { Op::LdrBReg, true, false, false, false, true },
		{ Op::LdrSBWReg, true, false, false, false, true }, { Op::LdrSHWReg, true, false, false, false, true },
		{ Op::LdrSBX, true, false, false, false, false }, { Op::LdrSHX, true, false, false, false, false },
		{ Op::LdrSW, true, false, false, false, false }, { Op::LdurSBX, true, false, false, true, false },
		{ Op::LdurSHX, true, false, false, true, false }, { Op::LdurSW, true, false, false, true, false },
		{ Op::LdrSBXReg, true, false, false, false, true }, { Op::LdrSHXReg, true, false, false, false, true },
		{ Op::LdrSWReg, true, false, false, false, true }, { Op::LdrHReg, true, false, false, false, true },
		{ Op::LdrWReg, true, false, false, false, true }, { Op::LdrXReg, true, false, false, false, true },
		{ Op::StrBReg, false, false, false, false, true }, { Op::StrHReg, false, false, false, false, true },
		{ Op::StrWReg, false, false, false, false, true }, { Op::StrXReg, false, false, false, false, true },
		{ Op::LdpX, true, false, true, false, false }, { Op::LdpW, true, false, true, false, false },
		{ Op::StpX, false, false, true, false, false }, { Op::StpW, false, false, true, false, false },
		{ Op::LdrS, true, true, false, false, false }, { Op::LdrD, true, true, false, false, false },
		{ Op::LdrQ, true, true, false, false, false }, { Op::StrS, false, true, false, false, false },
		{ Op::StrD, false, true, false, false, false }, { Op::StrQ, false, true, false, false, false },
		{ Op::LdurS, true, true, false, true, false }, { Op::LdurD, true, true, false, true, false },
		{ Op::LdurQ, true, true, false, true, false }, { Op::SturS, false, true, false, true, false },
		{ Op::SturD, false, true, false, true, false }, { Op::SturQ, false, true, false, true, false },
		{ Op::LdrSReg, true, true, false, false, true }, { Op::LdrDReg, true, true, false, false, true },
		{ Op::LdrQReg, true, true, false, false, true }, { Op::StrSReg, false, true, false, false, true },
		{ Op::StrDReg, false, true, false, false, true }, { Op::StrQReg, false, true, false, false, true },
		{ Op::LdpS, true, true, true, false, false }, { Op::LdpD, true, true, true, false, false },
		{ Op::LdpQ, true, true, true, false, false }, { Op::StpS, false, true, true, false, false },
		{ Op::StpD, false, true, true, false, false }, { Op::StpQ, false, true, true, false, false } };
	// Every load and store has bit 27 set and bit 25 clear.
	if ((static_cast<uint32_t>(ins.op) & 0x0a000000) != 0x08000000)
		return false;
	for (const auto& form : forms)
	{
		if (form.op != ins.op)
			continue;
		const uint32_t data = (form.simd ? fp(ins.rd) : gp(ins.rd)) |
			(form.pair ? (form.simd ? fp(ra_of(ins)) : gp(ra_of(ins))) : 0);
		(form.load ? (form.simd ? e.fp_def : e.gp_def) : (form.simd ? e.fp_use : e.gp_use)) = data;
		e.gp_use |= gp(ins.rn, true) | (form.indexed ? gp(ins.rm) : 0);
		if (form.unscaled && ((ins.field >> 10) & 3))
			e.gp_def |= gp(ins.rn, true);
		(form.load ? e.reads : e.writes) = true;
		return true;
	}
	return false;
}

bool effects(const Instruction& ins, Effects& e)
{
	e = Effects();
	return integer_effects(ins, e) || floating_effects(ins, e) || memory_effects(ins, e);
}

// A single access with an immediate offset and no writeback.
struct Access
{
	bool load, simd;
	unsigned bytes, rt, rn;
	int64_t offset;
};

struct SingleForm
{
	Op op;
	bool load, simd, unscaled;
	unsigned bytes;
};
const SingleForm kSingleForms[] = { { Op::LdrW, true, false, false, 4 }, { Op::LdrX, true, false, false, 8 },
	{ Op::StrW, false, false, false, 4 }, { Op::StrX, false, false, false, 8 }, { Op::LdurW, true, false, true, 4 },
	{ Op::LdurX, true, false, true, 8 }, { Op::SturW, false, false, true, 4 }, { Op::SturX, false, false, true, 8 },
	{ Op::LdrS, true, true, false, 4 }, { Op::LdrD, true, true, false, 8 }, { Op::LdrQ, true, true, false, 16 },
	{ Op::StrS, false, true, false, 4 }, { Op::StrD, false, true, false, 8 }, { Op::StrQ, false, true, false, 16 },
	{ Op::LdurS, true, true, true, 4 }, { Op::LdurD, true, true, true, 8 }, { Op::LdurQ, true, true, true, 16 },
	{ Op::SturS, false, true, true, 4 }, { Op::SturD, false, true, true, 8 }, { Op::SturQ, false, true, true, 16 } };

bool single_access(const Instruction& ins, Access& access)
{
	if (ins.fixup != Fixup::None)
		return false;
	for (const auto& form : kSingleForms)
	{
		if (form.op != ins.op)
			continue;
		if (form.unscaled && ((ins.field >> 10) & 3))
			return false;
		const int64_t offset = form.unscaled ? (int64_t((ins.field >> 12) & 511) ^ 256) - 256
											 : int64_t((ins.field >> 10) & 4095) * form.bytes;
		access = Access{ form.load, form.simd, form.bytes, ins.rd, ins.rn, offset };
		return true;
	}
	return false;
}

// The single access as an instruction: the scaled form from offset zero up,
// the unscaled form below it.
bool single_instruction(const Access& access, Instruction& ins)
{
	const bool unscaled = access.offset < 0;
	if (unscaled ? access.offset < -256 : access.offset % access.bytes || access.offset / access.bytes > 4095)
		return false;
	for (const auto& form : kSingleForms)
		if (form.load == access.load && form.simd == access.simd && form.unscaled == unscaled &&
			form.bytes == access.bytes)
		{
			const uint32_t field = unscaled ? (uint32_t(access.offset) & 511) << 12
											: uint32_t(access.offset / access.bytes) << 10;
			ins = Instruction{ form.op, access.rt, access.rn, 0, field, Fixup::None, 0 };
			return true;
		}
	return false;
}

Op pair_op(const Access& a)
{
	if (!a.simd)
		return a.bytes == 4 ? (a.load ? Op::LdpW : Op::StpW) : (a.load ? Op::LdpX : Op::StpX);
	if (a.bytes == 4)
		return a.load ? Op::LdpS : Op::StpS;
	return a.bytes == 8 ? (a.load ? Op::LdpD : Op::StpD) : (a.load ? Op::LdpQ : Op::StpQ);
}

// The two accesses of a pair without writeback, the lower address first.
bool pair_halves(const Instruction& ins, Access& low, Access& high)
{
	bool load = false, simd = false;
	unsigned bytes = 8;
	switch (ins.op)
	{
	case Op::LdpW:
		bytes = 4;
		// Fallthrough.
	case Op::LdpX:
		load = true;
		break;
	case Op::StpW:
		bytes = 4;
		// Fallthrough.
	case Op::StpX:
		break;
	case Op::LdpS:
	case Op::LdpD:
	case Op::LdpQ:
		load = true;
		// Fallthrough.
	case Op::StpS:
	case Op::StpD:
	case Op::StpQ:
		simd = true;
		bytes = ins.op == Op::LdpS || ins.op == Op::StpS ? 4 : ins.op == Op::LdpD || ins.op == Op::StpD ? 8 : 16;
		break;
	default:
		return false;
	}
	if (ins.fixup != Fixup::None)
		return false;
	const int64_t offset = ((int64_t((ins.field >> 15) & 127) ^ 64) - 64) * bytes;
	low = Access{ load, simd, bytes, ins.rd, ins.rn, offset };
	high = Access{ load, simd, bytes, ra_of(ins), ins.rn, offset + int64_t(bytes) };
	return true;
}

// The pair of accesses x (first) and y, separated by the effects between
// them, as an instruction at the first or the second one's place.
bool join(const Access& x, const Access& y, const Effects& between, Instruction& pair, bool& at_second)
{
	if (y.load != x.load || y.simd != x.simd || y.bytes != x.bytes || y.rn != x.rn)
		return false;
	const bool ascending = y.offset == x.offset + x.bytes;
	if (!ascending && x.offset != y.offset + x.bytes)
		return false;
	const int64_t low = std::min(x.offset, y.offset);
	if (low % x.bytes || low / x.bytes < -64 || low / x.bytes > 63)
		return false;
	const auto data = [&](unsigned reg) { return x.simd ? fp(reg) : gp(reg); };
	const uint32_t defs = x.simd ? between.fp_def : between.gp_def;
	const uint32_t uses = x.simd ? between.fp_use : between.gp_use;
	if (x.load)
	{
		// LDP writes both registers at the first load. They must differ, and
		// nothing between may read or write the second one.
		if (x.rt == y.rt || ((defs | uses) & data(y.rt)))
			return false;
		at_second = false;
	}
	else if (!(defs & data(x.rt)))
		at_second = true;
	else if (!(defs & data(y.rt)))
		at_second = false;
	else
		return false;
	const unsigned lo = ascending ? x.rt : y.rt, hi = ascending ? y.rt : x.rt;
	pair = Instruction{ pair_op(x), lo, x.rn, 0, ((uint32_t(low / x.bytes) & 127) << 15) | (hi << 10), Fixup::None, 0 };
	return true;
}

// The unscaled form of a single access at offset zero from a general base
// other than its data register, which takes pre- and post-index writeback.
Op indexed_form(const Instruction& ins)
{
	static const Op forms[][2] = { { Op::LdrB, Op::LdurB }, { Op::LdrH, Op::LdurH }, { Op::LdrW, Op::LdurW },
		{ Op::LdrX, Op::LdurX }, { Op::LdrSBW, Op::LdurSBW }, { Op::LdrSHW, Op::LdurSHW }, { Op::LdrSBX, Op::LdurSBX },
		{ Op::LdrSHX, Op::LdurSHX }, { Op::LdrSW, Op::LdurSW }, { Op::StrB, Op::SturB }, { Op::StrH, Op::SturH },
		{ Op::StrW, Op::SturW }, { Op::StrX, Op::SturX }, { Op::LdrS, Op::LdurS }, { Op::LdrD, Op::LdurD },
		{ Op::LdrQ, Op::LdurQ }, { Op::StrS, Op::SturS }, { Op::StrD, Op::SturD }, { Op::StrQ, Op::SturQ } };
	if (ins.fixup != Fixup::None || ins.field != 0 || ins.rn >= 31)
		return Op::B;
	for (const auto& form : forms)
		if (form[0] == ins.op || form[1] == ins.op)
		{
			Effects e;
			memory_effects(ins, e);
			return (e.fp_def | e.fp_use) || ins.rd != ins.rn ? form[1] : Op::B;
		}
	return Op::B;
}

// The signed amount by which `add xN, xN, #imm` or `sub` advances base xN,
// when it fits a nine-bit writeback offset.
bool advance(const Instruction& ins, unsigned base, int32_t& amount)
{
	if ((ins.op != Op::AddI && ins.op != Op::SubI) || ins.fixup != Fixup::None || ins.rd != base || ins.rn != base ||
		(ins.field & ~(4095u << 10)))
		return false;
	const int32_t imm = int32_t((ins.field >> 10) & 4095);
	amount = ins.op == Op::AddI ? imm : -imm;
	return amount >= -256 && amount <= 255 && amount != 0;
}

// Pre- and post-index forms, as Clang's AArch64 load/store optimizer makes
// them. An access at offset zero followed by an increment of its base takes
// the increment as post-index writeback; an increment followed by the access
// becomes pre-index writeback at the access. Nothing between the two may name
// the base, so only the increment moves.
void index_accesses(Function& f, uint32_t first, const std::vector<uint8_t>& boundary, std::vector<uint8_t>& gone,
	std::vector<uint32_t>& removed)
{
	std::vector<Instruction>& code = f.code;
	const uint32_t n = static_cast<uint32_t>(code.size());
	for (uint32_t a = first; a < n; ++a)
	{
		if (gone[a])
			continue;
		const Instruction& head = code[a];
		const Op single = indexed_form(head);
		int32_t amount = 0;
		const bool step = single == Op::B && head.rd < 31 && advance(head, head.rd, amount);
		if (single == Op::B && !step)
			continue;
		const unsigned base = step ? head.rd : head.rn;
		for (uint32_t b = a + 1; b < n && b <= a + kWindow && !boundary[b]; ++b)
		{
			if (gone[b])
				continue;
			Instruction& ins = code[b];
			const Op later = step ? indexed_form(ins) : Op::B;
			if (!boundary[b + 1] && (step ? later != Op::B && ins.rn == base : advance(ins, base, amount)))
			{
				Instruction& access = step ? ins : code[a];
				access.op = step ? later : single;
				access.field = ((uint32_t(amount) & 511) << 12) | ((step ? 3u : 1u) << 10);
				gone[step ? a : b] = 1;
				removed.push_back(step ? a : b);
				break;
			}
			Effects e;
			if (!effects(ins, e) || ((e.gp_use | e.gp_def) & gp(base)))
				break;
		}
	}
}

// A repeated ADRP of one page into a register that still holds the page is
// dropped. The search ends at a block start and at any instruction that may
// write the register.
void share_pages(const Function& f, uint32_t first, const std::vector<uint8_t>& boundary, std::vector<uint8_t>& gone,
	std::vector<uint32_t>& removed)
{
	const uint32_t n = static_cast<uint32_t>(f.code.size());
	for (uint32_t a = first; a < n; ++a)
	{
		const Instruction& page = f.code[a];
		if (gone[a] || page.op != Op::Adrp || page.fixup != Fixup::Page || page.rd >= 31)
			continue;
		for (uint32_t b = a + 1; b < n && b <= a + kPageWindow && !boundary[b]; ++b)
		{
			const Instruction& ins = f.code[b];
			if (ins.op == Op::Adrp && ins.fixup == Fixup::Page && ins.rd == page.rd && ins.target == page.target &&
				ins.addend == page.addend)
			{
				gone[b] = 1;
				removed.push_back(b);
				continue;
			}
			Effects e;
			if (!effects(ins, e) || (e.gp_def & gp(page.rd)))
				break;
		}
	}
}

// Forwarding in flight. A pair forwards neither half before both halves have
// their data: a pair load waits for the two stores it reads, and a load of
// one half of a pair store waits for the other half's value. A value a loop
// carries through memory then waits for its neighbour each time around. So
// inside a loop, and while the base register stays, two loads do not pair
// when a store of the loop may still be in flight where either one reads it,
// and two stores do not pair when a load of the loop may read either one in
// flight. A load reads the last store of its own block to its memory when
// the block has one, and that store has its data once a call stands between
// them; a load with no such store may read any store of the loop.
class Flights
{
public:
	explicit Flights(const Function& f) : f_(f) {}

	// The code changed.
	void reset() { stage_ = kNone; }

	// Whether the accesses x at `first` and y at `second` stay single.
	bool keeps_single(uint32_t first, uint32_t second, const Access& x, const Access& y)
	{
		if (stage_ == kNone)
			find_loops();
		const uint32_t loop = loop_.empty() ? 0 : loop_[first];
		if (!loop)
			return false;
		if (stage_ == kLoops)
			index_loops();
		const int64_t low = std::min(x.offset, y.offset);
		if ((moved_[loop] & gp(x.rn, true)) || !placed(low, 2 * x.bytes))
			return false;
		const uint32_t half = cell_of(low) + x.bytes / 4;
		for (uint32_t cell = cell_of(low), end = half + x.bytes / 4; cell != end; ++cell)
		{
			// The store of this cell: x holds the low half or the high one.
			const uint32_t store = (cell < half) == (x.offset == low) ? first : second;
			if (x.load ? fed_in_flight(loop, x.rn, cell, first) : read_in_flight(loop, x.rn, cell, store))
				return true;
		}
		return false;
	}

private:
	// One 4-byte cell an access covers, sorted by loop, base, kind and cell,
	// then by place.
	struct Cell
	{
		uint32_t loop, key, pc;
		bool operator<(const Cell& other) const
		{
			return loop != other.loop ? loop < other.loop : key != other.key ? key < other.key : pc < other.pc;
		}
	};
	using Range = std::pair<const Cell*, const Cell*>;

	static bool placed(int64_t offset, unsigned bytes) { return offset % 4 == 0 && offset >= -256 && bytes % 4 == 0; }
	static uint32_t cell_of(int64_t offset) { return static_cast<uint32_t>((offset + 256) / 4); }
	static uint32_t key(unsigned rn, bool load, uint32_t cell) { return rn << 20 | (load ? 1u << 19 : 0) | cell; }

	Range cells(uint32_t loop, unsigned rn, bool load, uint32_t cell) const
	{
		const Cell low{ loop, key(rn, load, cell), 0 };
		const Cell* end = cells_.data() + cells_.size();
		const Cell* first = std::lower_bound(cells_.data(), end, low);
		const Cell* last = first;
		while (last != end && last->loop == low.loop && last->key == low.key)
			++last;
		return Range(first, last);
	}

	// The last of the stores before `pc` in its block, or null.
	const Cell* feeder(const Range& stores, uint32_t pc) const
	{
		const Cell* after = stores.first;
		while (after != stores.second && after->pc < pc)
			++after;
		return after != stores.first && block_[after[-1].pc] == block_[pc] ? after - 1 : nullptr;
	}

	// A load at `pc` of a cell the loop stores: in flight unless its own
	// block's store feeds it past a call.
	bool fed_in_flight(uint32_t loop, unsigned rn, uint32_t cell, uint32_t pc) const
	{
		const Range stores = cells(loop, rn, false, cell);
		if (stores.first == stores.second)
			return false;
		const Cell* from = feeder(stores, pc);
		return !from || calls_[from->pc] == calls_[pc];
	}

	// A store at `pc`: read in flight by the next load of its block with no
	// call between, or by a load of the loop that no store of its own block
	// feeds.
	bool read_in_flight(uint32_t loop, unsigned rn, uint32_t cell, uint32_t pc)
	{
		const Range loads = cells(loop, rn, true, cell);
		if (loads.first == loads.second)
			return false;
		const Range stores = cells(loop, rn, false, cell);
		uint8_t& open = open_[loads.first - cells_.data()];
		if (!open)
		{
			open = 1;
			for (const Cell* load = loads.first; load != loads.second && open == 1; ++load)
				if (!feeder(stores, load->pc))
					open = 2;
		}
		if (open == 2)
			return true;
		const Cell* next = loads.first;
		while (next != loads.second && next->pc < pc)
			++next;
		if (next == loads.second || block_[next->pc] != block_[pc] || calls_[next->pc] != calls_[pc])
			return false;
		const Cell* from = feeder(stores, next->pc);
		return from && from->pc == pc;
	}

	// A loop is the code a branch back to a label spans; spans that overlap
	// are one loop.
	void find_loops()
	{
		stage_ = kLoops;
		loop_.clear();
		loops_ = 0;
		const uint32_t n = static_cast<uint32_t>(f_.code.size());
		std::vector<int32_t> depth;
		for (uint32_t pc = 0; pc < n; ++pc)
		{
			const Instruction& ins = f_.code[pc];
			if (ins.fixup != Fixup::Block || ins.target >= f_.labels.size() || f_.labels[ins.target] > pc)
				continue;
			if (depth.empty())
				depth.resize(n + 2);
			++depth[f_.labels[ins.target]];
			--depth[pc + 1];
		}
		if (depth.empty())
			return;
		loop_.resize(n + 1);
		for (uint32_t pc = 0, inside = 0; pc < n; ++pc)
		{
			const uint32_t before = inside;
			inside += depth[pc];
			loops_ += inside && !before;
			loop_[pc] = inside ? loops_ : 0;
		}
	}

	// The registers each loop writes and the cells its accesses cover.
	void index_loops()
	{
		stage_ = kIndexed;
		const uint32_t n = static_cast<uint32_t>(f_.code.size());
		block_.assign(n + 1, 0);
		calls_.assign(n + 1, 0);
		for (uint32_t pc : f_.labels)
			if (pc < n)
				block_[pc] = 1;
		moved_.assign(loops_ + 1, 0);
		cells_.clear();
		for (uint32_t pc = 0; pc < n; ++pc)
		{
			const Instruction& ins = f_.code[pc];
			const bool call = ins.op == Op::Bl || ins.op == Op::Blr;
			block_[pc] += pc ? block_[pc - 1] : 0;
			calls_[pc + 1] = calls_[pc] + call;
			const uint32_t loop = loop_[pc];
			Effects e;
			Access access, high;
			if (!loop)
				continue;
			if (integer_effects(ins, e) || floating_effects(ins, e))
			{
				moved_[loop] |= e.gp_def;
				continue;
			}
			const bool pair = pair_halves(ins, access, high);
			if (pair || single_access(ins, access))
			{
				if (access.load && !access.simd)
					moved_[loop] |= gp(access.rt) | (pair ? gp(high.rt) : 0);
				if (!placed(access.offset, access.bytes))
					continue;
				const unsigned cells = (pair ? 2 : 1) * access.bytes / 4;
				for (uint32_t cell = cell_of(access.offset), end = cell + cells; cell != end; ++cell)
					cells_.push_back(Cell{ loop, key(access.rn, access.load, cell), pc });
			}
			else if (memory_effects(ins, e))
				moved_[loop] |= e.gp_def;
			// A call keeps x19 to x29; a branch writes no register; any
			// other instruction without a model may write all but SP.
			else if (call)
				moved_[loop] |= 0x4007ffffu;
			else if (ins.fixup != Fixup::Block && ins.op != Op::Ret && ins.op != Op::Br)
				moved_[loop] |= ~kSp;
		}
		// The rule reads no cell of a base its loop writes.
		cells_.erase(std::remove_if(cells_.begin(), cells_.end(),
						 [&](const Cell& c) { return (moved_[c.loop] & gp(c.key >> 20, true)) != 0; }),
			cells_.end());
		std::sort(cells_.begin(), cells_.end());
		open_.assign(cells_.size() + 1, 0);
	}

	enum Stage
	{
		kNone,
		kLoops,
		kIndexed
	};
	const Function& f_;
	Stage stage_ = kNone;
	uint32_t loops_ = 0;
	std::vector<uint32_t> loop_;  // the loop of each instruction, from 1; 0 outside loops
	std::vector<uint32_t> block_; // labels at or before each instruction
	std::vector<uint32_t> calls_; // calls before each instruction
	std::vector<uint32_t> moved_; // general registers each loop writes
	std::vector<Cell> cells_;
	std::vector<uint8_t> open_; // at a load range's start: 1 every load has a feeder, 2 one has none
};

// A pair selection made through a pointer register falls under the same
// rule: it becomes two single accesses again. A load into the base goes last.
void split_pairs(Function& f, uint32_t first, std::vector<BitTest>& tests, Flights& flights)
{
	std::vector<uint32_t> places;
	std::vector<Instruction> added;
	for (uint32_t pc = first; pc < f.code.size(); ++pc)
	{
		Access low, high;
		Instruction one, other;
		if (!pair_halves(f.code[pc], low, high) || low.rn == 31 || !single_instruction(low, one) ||
			!single_instruction(high, other) || !flights.keeps_single(pc, pc, low, high))
			continue;
		const bool base_first = low.load && !low.simd && low.rt == low.rn;
		f.code[pc] = base_first ? other : one;
		places.push_back(pc);
		added.push_back(base_first ? one : other);
	}
	if (places.empty())
		return;
	for (auto& test : tests)
		test.pc += static_cast<uint32_t>(std::lower_bound(places.begin(), places.end(), test.pc) - places.begin());
	insert_instructions(f, places, added);
	flights.reset();
}

} // namespace

// Volatile and atomic accesses keep their own instructions and order, and
// inline assembly is not modelled, so no access of a body with any of them
// is paired.
bool orders_memory(const lowir::FunctionBody& body)
{
	using lowir::Opcode;
	for (const auto& inst : body.instructions)
		switch (inst.op)
		{
		case Opcode::Asm:
		case Opcode::AtomicLoad:
		case Opcode::AtomicStore:
		case Opcode::AtomicAddFetch:
		case Opcode::AtomicExchange:
		case Opcode::AtomicCompareExchange:
		case Opcode::AtomicThreadFence:
		case Opcode::AtomicSignalFence:
			return true;
		default:
			if (inst.flags & lowir::IF_VOLATILE)
				return true;
		}
	return false;
}

void pair_accesses(Function& f, uint32_t first, std::vector<BitTest>& tests, bool ordered)
{
	Flights flights(f);
	if (!ordered)
		split_pairs(f, first, tests, flights);
	const uint32_t n = static_cast<uint32_t>(f.code.size());
	// A block start, a call site edge or a landing pad ends a search; so does
	// any frame event, which marks prologue or epilogue code.
	std::vector<uint8_t> boundary(n + 2);
	for (uint32_t pc : f.labels)
		if (pc <= n)
			boundary[pc] = 1;
	for (const auto& site : f.sites)
	{
		boundary[std::min(site.begin / 4, n)] = 1;
		boundary[std::min(site.end / 4, n)] = 1;
	}
	for (uint32_t pad : f.pads)
		if (pad != lowir::kNoId)
			boundary[std::min(pad / 4, n)] = 1;
	for (const auto& event : f.frame_events)
		boundary[std::min(event.pc / 4, n + 1)] = 1;
	std::vector<uint8_t> gone(n);
	std::vector<uint32_t> removed;
	share_pages(f, first, boundary, gone, removed);
	for (uint32_t a = first; a < n && !ordered; ++a)
	{
		Access x;
		// A general load into its own base changes the second address.
		if (gone[a] || !single_access(f.code[a], x) || (x.load && !x.simd && x.rt == x.rn))
			continue;
		Effects between;
		for (uint32_t b = a + 1; b < n && b <= a + kWindow && !boundary[b]; ++b)
		{
			if (gone[b])
				continue;
			const Instruction& ins = f.code[b];
			Access y;
			Instruction pair;
			bool at_second = false;
			if (single_access(ins, y) && !boundary[b + 1] && join(x, y, between, pair, at_second) &&
				!flights.keeps_single(a, b, x, y))
			{
				f.code[at_second ? b : a] = pair;
				const uint32_t dropped = at_second ? a : b;
				gone[dropped] = 1;
				removed.push_back(dropped);
				break;
			}
			Effects e;
			if (!effects(ins, e))
				break;
			between.add(e);
			if ((between.gp_def & gp(x.rn, true)) || between.writes || (!x.load && between.reads))
				break;
		}
	}
	if (!ordered)
		index_accesses(f, first, boundary, gone, removed);
	if (removed.empty())
		return;
	std::sort(removed.begin(), removed.end());
	for (auto& test : tests)
		test.pc -= static_cast<uint32_t>(std::lower_bound(removed.begin(), removed.end(), test.pc) - removed.begin());
	remove_instructions(f, removed);
}

} // namespace aarch64
