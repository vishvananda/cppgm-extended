#include "native/runtime/runtime.h"

namespace native {

void emit_tlv_lookup(Image& image, uint32_t symbol, UnwindFacts& facts)
{
	while (image.text().size() % 16 != 0)
		image.text().push_back(0xcc);
	const uint32_t base = static_cast<uint32_t>(image.text().size());
	image.define(symbol, Section::Text, base);
	Assembler a(image.text());
	a.push(RBP);
	facts.push_end = a.size() - base;
	a.mov_rr(RBP, RSP);
	facts.frame_end = a.size() - base;
	a.byte(0x9c); // pushfq: a symbol-address move preserves flags
	const Reg saved[] = { RAX, RCX, RDX, RSI, RDI, R8, R9, R10, RBX, R12, R13, R14, R15 };
	for (unsigned i = 0; i < sizeof(saved) / sizeof(saved[0]); ++i)
	{
		a.push(saved[i]);
		if ((kCalleeSavedGprs & reg_bit(saved[i])) != 0)
			facts.saves.push_back(UnwindFacts::Save{ a.size() - base, saved[i], -16 - 8 * static_cast<int32_t>(i) });
	}
	// FXSAVE64 preserves SSE, x87 and MXCSR as well as the GP registers.
	// Align dynamically: this private call can occur in a leaf or while
	// ordinary outgoing arguments are being staged on the stack.
	a.alu_ri(5, RM::r(RSP), 528, 8);
	a.alu_ri(4, RM::r(RSP), -16, 8);
	a.op(0, true, 0x0fae, 2, 0, RM::m(RSP, 0));
	a.mov_rr(RDI, R11);
	a.call_rm(RM::m(RDI, 0));
	a.mov_rr(R11, RAX);
	a.op(0, true, 0x0fae, 2, 1, RM::m(RSP, 0));
	a.lea(RSP, RM::m(RBP, -112));
	for (unsigned i = sizeof(saved) / sizeof(saved[0]); i != 0; --i)
		a.pop(saved[i - 1]);
	a.byte(0x9d); // popfq
	a.byte(0xc9); // leave
	facts.leave_end = a.size() - base;
	a.ret();
	facts.ret_end = a.size() - base;
	facts.size = a.size() - base;
}

} // namespace native
