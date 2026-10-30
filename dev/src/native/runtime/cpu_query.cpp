#include "native/runtime/runtime.h"

namespace native {

void emit_cpu_query(Image& image, uint32_t symbol, UnwindFacts& facts)
{
	while (image.text().size() % 16 != 0)
		image.text().push_back(0xcc);
	const uint32_t start = static_cast<uint32_t>(image.text().size());
	image.define(symbol, Section::Text, start);
	Assembler a(image.text());
	facts = UnwindFacts();
	a.push(RBP);
	facts.push_end = a.size() - start;
	a.mov_rr(RBP, RSP);
	facts.frame_end = a.size() - start;
	a.alu_ri(5, RM::r(RSP), 16, 8);
	a.mov_store(RM::m(RBP, -8), RBX, 8);
	facts.saves.push_back(UnwindFacts::Save{ a.size() - start, RBX, -8 });
	a.mov_rr(R10, RDX);
	a.op(0, false, 0x8b, 1, RAX, RM::r(RDI)); // eax = leaf
	a.op(0, false, 0x8b, 1, RCX, RM::r(RSI)); // ecx = subleaf
	a.byte(0x0f);
	a.byte(0xa2); // cpuid
	const Reg results[] = { RAX, RBX, RCX, RDX };
	for (uint32_t reg = 0; reg < 4; ++reg)
		a.mov_store(RM::m(R10, static_cast<int32_t>(reg * 4)), results[reg], 4);
	a.mov_load(RBX, RM::m(RBP, -8), 8, false);
	a.byte(0xc9); // leave
	facts.leave_end = a.size() - start;
	a.ret();
	facts.ret_end = a.size() - start;
	facts.size = facts.ret_end;
}

} // namespace native
