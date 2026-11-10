#include "native/select/mir_cleanup.h"

#include <algorithm>
#include <climits>

#include "native/mir/type_kinds.h"

// Scalar and vector coalescing of immediate memory writes after selection.
namespace native {

namespace {

// A complete run of zero bytes can use the direct vector/scalar bulk form.
// Its scratch XMM is reserved by selection; no ordinary register or flags
// are clobbered. Gaps remain gaps, including padding between fields.
bool merge_zero_run(std::vector<Inst>& insts, uint32_t first, uint32_t past, int32_t lo, uint32_t span,
	const uint8_t* value, const uint8_t* written, bool wide)
{
	if (!wide || span < 16)
		return false;
	for (uint32_t p = 0; p < span; ++p)
		if (!written[p] || value[p] != 0)
			return false;
	Inst& zero = insts[first];
	zero.op = Op::ZeroBytes;
	zero.type = lowir::TypeKind::Void;
	zero.nops = 1;
	zero.ops[0].disp = lo;
	zero.aux = span;
	zero.aux2 = 1;
	for (uint32_t k = first + 1; k < past; ++k)
	{
		insts[k].op = Op::Nop;
		insts[k].nops = 0;
	}
	return true;
}

// The stores insts[first, past) (immediates, one base): rewritten in place
// when fewer chunks cover their bytes. Gaps stay untouched; zero subranges
// can use vector stores. Overlapping writes must agree byte for byte, and
// the whole run spans at most 64 bytes.
void merge_store_run(std::vector<Inst>& insts, uint32_t first, uint32_t past, bool wide)
{
	const uint32_t kSpan = 64;
	int32_t lo = INT32_MAX, hi = INT32_MIN;
	for (uint32_t k = first; k < past; ++k)
	{
		const int32_t disp = insts[k].ops[0].disp;
		if (disp > INT32_MAX - static_cast<int32_t>(register_bytes(insts[k].type)))
			return;
		lo = std::min(lo, disp);
		hi = std::max(hi, disp + static_cast<int32_t>(register_bytes(insts[k].type)));
	}
	if (static_cast<int64_t>(hi) - lo > kSpan)
		return;
	uint8_t value[kSpan] = {}, written[kSpan] = {};
	for (uint32_t k = first; k < past; ++k)
	{
		const uint32_t at = static_cast<uint32_t>(insts[k].ops[0].disp - lo);
		for (uint32_t b = 0; b < register_bytes(insts[k].type); ++b)
		{
			const uint8_t byte = static_cast<uint8_t>(static_cast<uint64_t>(insts[k].ops[1].imm) >> (8 * b));
			if (written[at + b] && value[at + b] != byte)
				return;
			written[at + b] = 1;
			value[at + b] = byte;
		}
	}
	const uint32_t span = static_cast<uint32_t>(hi - lo);
	if (merge_zero_run(insts, first, past, lo, span, value, written, wide))
		return;
	static const lowir::TypeKind kTypes[9] = { lowir::TypeKind::Void, lowir::TypeKind::I8, lowir::TypeKind::I16,
		lowir::TypeKind::Void, lowir::TypeKind::I32, lowir::TypeKind::Void, lowir::TypeKind::Void,
		lowir::TypeKind::Void, lowir::TypeKind::I64 };
	int32_t disps[kSpan];
	uint8_t widths[kSpan];
	bool zeroes[kSpan] = {};
	int64_t imms[kSpan];
	uint32_t count = 0;
	for (uint32_t p = 0; p < span;)
	{
		if (!written[p])
		{
			++p;
			continue;
		}
		uint32_t end = p;
		if (wide)
			while (end < span && written[end] && value[end] == 0)
				++end;
		if (end - p >= 16)
		{
			disps[count] = lo + static_cast<int32_t>(p);
			widths[count] = static_cast<uint8_t>(end - p);
			imms[count] = 0;
			zeroes[count++] = true;
			p = end;
			continue;
		}
		uint32_t w = 8;
		uint64_t v = 0;
		for (; w > 1; w /= 2)
		{
			bool ok = ((lo + static_cast<int32_t>(p)) & static_cast<int32_t>(w - 1)) == 0 && p + w <= span;
			for (uint32_t b = 0; ok && b < w; ++b)
				ok = written[p + b] != 0;
			v = 0;
			for (uint32_t b = 0; ok && b < w; ++b)
				v |= static_cast<uint64_t>(value[p + b]) << (8 * b);
			if (ok && (w < 8 || (static_cast<int64_t>(v) >= INT32_MIN && static_cast<int64_t>(v) <= INT32_MAX)))
				break;
		}
		if (w == 1)
			v = value[p];
		disps[count] = lo + static_cast<int32_t>(p);
		widths[count] = static_cast<uint8_t>(w);
		imms[count++] = static_cast<int64_t>(v);
		p += w;
	}
	if (count >= past - first)
		return;
	for (uint32_t c = 0; c < count; ++c)
	{
		Inst& inst = insts[first + c];
		if (zeroes[c])
		{
			inst.op = Op::ZeroBytes;
			inst.type = lowir::TypeKind::Void;
			inst.nops = 1;
			inst.ops[0].disp = disps[c];
			inst.aux = widths[c];
			inst.aux2 = 1;
			continue;
		}
		inst.type = kTypes[widths[c]];
		inst.ops[0].disp = disps[c];
		inst.ops[1] = Operand::make_imm(imms[c], imms[c] < 0 ? -1 : 0);
	}
	for (uint32_t k = first + count; k < past; ++k)
	{
		insts[k].op = Op::Nop;
		insts[k].nops = 0;
	}
}

} // namespace

// O1+: a run of consecutive stores of immediates to adjacent bytes of one
// base (a struct zeroed or set field by field) becomes the fewest aligned
// 8/4/2/1-byte stores of the combined bytes (an 8-byte one when they fit a
// sign-extended imm32); the rest become nops. A wider load of those bytes
// then forwards from one store instead of stalling on several. Linear in
// the function's MIR.
void merge_constant_stores(Function& function)
{
	std::vector<Inst>& insts = function.insts;
	auto mergeable = [&](const Inst& inst) {
		return inst.op == Op::Store && inst.ops[1].kind == OperandKind::Imm && inst.ops[0].kind == OperandKind::Mem &&
			inst.ops[0].index == kNoReg && inst.ops[0].symbol == lowir::kNoId && inst.ops[0].reg != kNoReg &&
			!(inst.flags & MF_VOLATILE);
	};
	for (const Block& block : function.blocks)
	{
		for (uint32_t i = block.first; i < block.first + block.count;)
		{
			uint32_t j = i;
			int64_t lo = INT32_MAX, hi = INT32_MIN;
			while (j < block.first + block.count && mergeable(insts[j]) && insts[j].ops[0].reg == insts[i].ops[0].reg)
			{
				const int64_t start = insts[j].ops[0].disp;
				const int64_t end = start + register_bytes(insts[j].type);
				const int64_t next_lo = std::min(lo, start), next_hi = std::max(hi, end);
				if (next_hi - next_lo > 64)
					break;
				lo = next_lo;
				hi = next_hi;
				++j;
			}
			if (j - i < 2)
			{
				i = j == i ? i + 1 : j;
				continue;
			}
			merge_store_run(insts, i, j, function.wide_bulk);
			i = j;
		}
	}
}

} // namespace native
