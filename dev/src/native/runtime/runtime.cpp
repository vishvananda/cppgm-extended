#include "native/runtime/runtime.h"

#include <cstring>

#include "native/runtime/eh_record.h"

namespace native {

const char kEhTopSymbol[] = "__cppgm_eh_top";
const char kEhPayloadSymbol[] = "__cppgm_eh_payload";
const char kEhSelectorSymbol[] = "__cppgm_eh_selector";
const char kEhDispatchSymbol[] = "__cppgm_eh_dispatch";
const char kEhUncaughtSymbol[] = "__cppgm_eh_uncaught";
const char kEhCatchSymbol[] = "__cppgm_eh_catch";
const char* const kWideDivisionSymbols[4] = { "__divti3", "__udivti3", "__modti3", "__umodti3" };
const char* const kFloatConversionSymbols[12] = { "__floattisf", "__floattidf", "__floattixf", "__floatuntisf",
	"__floatuntidf", "__floatuntixf", "__fixsfti", "__fixdfti", "__fixxfti", "__fixunssfti", "__fixunsdfti",
	"__fixunsxfti" };

namespace {

void align_text(Image& image)
{
	std::vector<uint8_t>& text = image.text();
	while (text.size() % 16 != 0)
		text.push_back(0xcc);
}

uint32_t begin_routine(Image& image)
{
	align_text(image);
	const uint32_t symbol = image.add_symbol();
	image.define(symbol, Section::Text, image.text().size());
	return symbol;
}

// The builtin runtime's string routines: strlen scans 16 bytes at a time,
// memcmp and short fills are byte loops, the copies use the string
// instructions.
void emit_builtin(Assembler& a, const char* name)
{
	if (std::strcmp(name, "cppgm_builtin_strlen") == 0)
	{
		// An aligned 16-byte load never crosses a page, so the scan reads
		// the aligned chunks holding the string; the first chunk's bytes
		// before the string's start are shifted out of its NUL mask. The
		// O1 call-site probe (x86/emitter.cpp strlen_prefix) calls this
		// for strings of 16 bytes or more, which a byte loop would walk at
		// a byte per iteration.
		const Reg chunk = XMM0 + 1;
		const uint32_t loop = a.new_label();
		const uint32_t first = a.new_label();
		a.mov_rr(RAX, RDI);
		a.op(0, false, 0x8b, 1, RCX, RM::r(RDI));        // mov ecx, edi
		a.alu_ri(4, RM::r(RAX), -16, 8);                 // and rax, -16
		a.alu_ri(4, RM::r(RCX), 15, 4);                  // and ecx, 15
		a.sse(0x66, 0xef, XMM0, RM::r(XMM0));            // pxor xmm0, xmm0
		a.sse(0x66, 0x6f, chunk, RM::m(RAX, 0));         // movdqa xmm1, [rax]
		a.sse(0x66, 0x74, chunk, RM::r(XMM0));           // pcmpeqb xmm1, xmm0
		a.op(0x66, false, 0x0fd7, 2, RDX, RM::r(chunk)); // pmovmskb edx, xmm1
		a.shift_cl(5, RM::r(RDX), 4);                    // shr edx, cl
		a.op(0, false, 0x85, 1, RDX, RM::r(RDX));        // test edx, edx
		a.jcc_short(CC_NE, first);
		a.bind(loop);
		a.alu_ri(0, RM::r(RAX), 16, 8);
		a.sse(0x66, 0x6f, chunk, RM::m(RAX, 0));
		a.sse(0x66, 0x74, chunk, RM::r(XMM0));
		a.op(0x66, false, 0x0fd7, 2, RDX, RM::r(chunk));
		a.op(0, false, 0x85, 1, RDX, RM::r(RDX));
		a.jcc_short(CC_E, loop);
		a.op(0, false, 0x0fbc, 2, RDX, RM::r(RDX)); // bsf edx, edx
		a.alu_rr(0, RAX, RM::r(RDX), 8);
		a.alu_rr(5, RAX, RM::r(RDI), 8);
		a.ret();
		a.bind(first);
		a.op(0, false, 0x0fbc, 2, RAX, RM::r(RDX)); // bsf eax, edx
		a.ret();
	}
	else if (std::strcmp(name, "cppgm_builtin_memcpy") == 0)
	{
		a.mov_rr(RAX, RDI);
		a.mov_rr(RCX, RDX);
		a.byte(0xf3);
		a.byte(0xa4);
		a.ret();
	}
	else if (std::strcmp(name, "cppgm_builtin_memmove") == 0)
	{
		const uint32_t forward = a.new_label();
		a.mov_rr(RAX, RDI);
		a.mov_rr(RCX, RDX);
		a.alu_rr(7, RDI, RM::r(RSI), 8);
		a.jcc_short(CC_BE, forward);
		a.lea(R11, RM::m(RSI, 0, RDX, 1));
		a.alu_rr(7, RDI, RM::r(R11), 8);
		a.jcc_short(CC_AE, forward);
		a.lea(RSI, RM::m(RSI, -1, RDX, 1));
		a.lea(RDI, RM::m(RDI, -1, RDX, 1));
		a.byte(0xfd); // std
		a.byte(0xf3);
		a.byte(0xa4);
		a.byte(0xfc); // cld
		a.ret();
		a.bind(forward);
		a.byte(0xf3);
		a.byte(0xa4);
		a.ret();
	}
	else if (std::strcmp(name, "cppgm_builtin_memset") == 0)
	{
		// Below 32 bytes a byte loop: rep stosb's start-up costs more
		// than the stores (an 8-byte fill 2.3x slower than the loop it
		// replaces in a lowiropt fill call).
		const uint32_t large = a.new_label();
		const uint32_t loop = a.new_label();
		const uint32_t done = a.new_label();
		a.mov_rr(R11, RDI);
		a.op(0, false, 0x8b, 1, RAX, RM::r(RSI)); // mov eax, esi
		a.alu_ri(7, RM::r(RDX), 32, 8);
		a.jcc_short(CC_AE, large);
		a.op(0, true, 0x85, 1, RDX, RM::r(RDX)); // test rdx, rdx
		a.jcc_short(CC_E, done);
		a.bind(loop);
		a.mov_store(RM::m(RDI, 0), RAX, 1);
		a.alu_ri(0, RM::r(RDI), 1, 8);
		a.alu_ri(5, RM::r(RDX), 1, 8);
		a.jcc_short(CC_NE, loop);
		a.bind(done);
		a.mov_rr(RAX, R11);
		a.ret();
		a.bind(large);
		a.mov_rr(RCX, RDX);
		a.byte(0xf3);
		a.byte(0xaa);
		a.mov_rr(RAX, R11);
		a.ret();
	}
	else if (std::strcmp(name, "cppgm_builtin_memcmp") == 0)
	{
		const uint32_t loop = a.new_label();
		const uint32_t differ = a.new_label();
		const uint32_t equal = a.new_label();
		a.bind(loop);
		a.op(0, true, 0x85, 1, RDX, RM::r(RDX)); // test rdx, rdx
		a.jcc_short(CC_E, equal);
		a.mov_load(RAX, RM::m(RDI, 0), 1, false);
		a.mov_load(RCX, RM::m(RSI, 0), 1, false);
		a.alu_rr(7, RAX, RM::r(RCX), 4);
		a.jcc_short(CC_NE, differ);
		a.alu_ri(0, RM::r(RDI), 1, 8);
		a.alu_ri(0, RM::r(RSI), 1, 8);
		a.alu_ri(5, RM::r(RDX), 1, 8);
		a.jmp_short(loop);
		a.bind(differ);
		a.alu_rr(5, RAX, RM::r(RCX), 4);
		a.ret();
		a.bind(equal);
		a.op(0, false, 0x33, 1, RAX, RM::r(RAX));
		a.ret();
	}
}

// The external C `atexit` (Startup::atexit) defined at `symbol`: pushes
// the callback in rdi onto the list at `state` (head, then the bump
// pointer and end of the page its 16-byte nodes, callback and next, are
// carved from; a new page is mapped when it is used up). Returns 0, or
// -1 when no page can be mapped.
void emit_exit_registration(Image& image, uint32_t symbol, uint32_t state)
{
	Assembler a(image.text());
	align_text(image);
	image.define(symbol, Section::Text, image.text().size());
	const uint32_t carve = a.new_label();
	const uint32_t failed = a.new_label();
	a.mov_load(RAX, RM::rip(state, 8), 8, false);
	a.alu_rr(7, RAX, RM::rip(state, 16), 8); // cmp rax, end (both 0 at first)
	a.jcc_short(CC_NE, carve);
	a.push(RDI);
	a.mov_ri(RAX, 9); // mmap(0, 4096, read|write, private|anonymous, -1, 0)
	a.mov_ri(RDI, 0);
	a.mov_ri(RSI, 4096);
	a.mov_ri(RDX, 3);
	a.mov_ri(R10, 0x22);
	a.mov_ri(R8, -1);
	a.mov_ri(R9, 0);
	a.byte(0x0f);
	a.byte(0x05);
	a.pop(RDI);
	a.op(0, true, 0x85, 1, RAX, RM::r(RAX)); // test rax, rax: -errno on failure
	a.jcc_short(CC_S, failed);
	a.lea(RDX, RM::m(RAX, 4096));
	a.mov_store(RM::rip(state, 16), RDX, 8);
	a.bind(carve);
	a.lea(RDX, RM::m(RAX, 16));
	a.mov_store(RM::rip(state, 8), RDX, 8);
	a.mov_store(RM::m(RAX, 0), RDI, 8);
	a.mov_load(RDX, RM::rip(state, 0), 8, false);
	a.mov_store(RM::m(RAX, 8), RDX, 8);
	a.mov_store(RM::rip(state, 0), RAX, 8);
	a.mov_ri(RAX, 0);
	a.ret();
	a.bind(failed);
	a.mov_ri(RAX, -1);
	a.ret();
	a.resolve_labels();
	image.add_code_fixups(a.fixups(), 0);
}

// Runs the registered callbacks, most recent first (C++11 18.5p8), each
// unlinked before its call so one it registers runs next.
void emit_exit_drain(Assembler& a, uint32_t state)
{
	const uint32_t loop = a.new_label();
	const uint32_t done = a.new_label();
	a.bind(loop);
	a.mov_load(RDI, RM::rip(state, 0), 8, false);
	a.op(0, true, 0x85, 1, RDI, RM::r(RDI)); // test rdi, rdi
	a.jcc_short(CC_E, done);
	a.mov_load(RAX, RM::m(RDI, 8), 8, false);
	a.mov_store(RM::rip(state, 0), RAX, 8);
	a.call_rm(RM::m(RDI, 0));
	a.jmp_short(loop);
	a.bind(done);
}

} // namespace

void emit_exit(Assembler& a, Reg status)
{
	if (status != RDI)
		a.mov_rr(RDI, status);
	a.mov_ri(RAX, 231);
	a.byte(0x0f);
	a.byte(0x05);
}

bool builtin_known(const char* name)
{
	return std::strcmp(name, "cppgm_builtin_strlen") == 0 || std::strcmp(name, "cppgm_builtin_memcpy") == 0 ||
		std::strcmp(name, "cppgm_builtin_memmove") == 0 || std::strcmp(name, "cppgm_builtin_memset") == 0 ||
		std::strcmp(name, "cppgm_builtin_memcmp") == 0;
}

void emit_builtin_routine(Image& image, uint32_t symbol, const char* name)
{
	Assembler a(image.text());
	align_text(image);
	image.define(symbol, Section::Text, image.text().size());
	emit_builtin(a, name);
	a.resolve_labels();
}

void emit_tls_wrapper(Image& image, uint32_t symbol, uint32_t object)
{
	Assembler a(image.text());
	align_text(image);
	image.define(symbol, Section::Text, image.text().size());
	a.lea(RAX, RM::rip(object, 0));
	a.ret();
	image.add_code_fixups(a.fixups(), 0);
}

void emit_jump_thunk(Image& image, uint32_t symbol, uint32_t target, Reg self, int64_t adjustment, int64_t vcall_offset)
{
	Assembler a(image.text());
	align_text(image);
	image.define(symbol, Section::Text, image.text().size());
	if (adjustment != 0)
		a.alu_ri(0, RM::r(self), adjustment, 8); // add self, adjustment
	if (vcall_offset != 0)
	{
		a.mov_load(R10, RM::m(self, 0), 8, false);
		a.alu_rr(0, self, RM::m(R10, static_cast<int32_t>(vcall_offset)), 8); // add self, [r10 + vcall]
	}
	a.jmp_symbol(target);
	image.add_code_fixups(a.fixups(), 0);
}

void declare_runtime(Image& image, RuntimeSymbols& runtime)
{
	runtime.eh_top = image.add_symbol();
	image.define(runtime.eh_top, Section::Bss, image.reserve(Section::Bss, 8, 8));
	runtime.eh_payload = image.add_symbol();
	image.define(runtime.eh_payload, Section::Bss, image.reserve(Section::Bss, 16, 16));
	runtime.eh_selector = image.add_symbol();
	image.define(runtime.eh_selector, Section::Bss, image.reserve(Section::Bss, 8, 8));
	runtime.eh_dispatch = image.add_symbol();
	runtime.eh_uncaught = image.add_symbol();
	runtime.eh_catch = image.add_symbol();
	for (unsigned k = 0; k < 4; ++k)
		runtime.wide_division[k] = image.add_symbol();
	for (unsigned k = 0; k < 12; ++k)
		runtime.float_conversion[k] = image.add_symbol();
}

void emit_dispatcher(Image& image, const RuntimeSymbols& runtime)
{
	Assembler a(image.text());
	// An exception with no handler left terminates the process.
	align_text(image);
	image.define(runtime.eh_uncaught, Section::Text, image.text().size());
	a.mov_ri(RDI, 134);
	emit_exit(a, RDI);
	// Throw/resume: drop the active records on top (their landings are
	// abandoned), mark the innermost remaining record active, restore the
	// frame and callee-saved registers it captured, and continue at its
	// landing; the record stays on the stack while the landing runs (a
	// record below the frame saved itself as RSP: eh_record.h).
	align_text(image);
	image.define(runtime.eh_dispatch, Section::Text, image.text().size());
	const uint32_t next = a.new_label();
	const uint32_t skip = a.new_label();
	const uint32_t found = a.new_label();
	a.mov_load(R11, RM::rip(runtime.eh_top, 0), 8, false);
	a.bind(next);
	a.op(0, true, 0x85, 1, R11, RM::r(R11)); // test r11, r11
	a.jcc_short(CC_NE, skip);
	a.jmp_symbol(runtime.eh_uncaught);
	a.bind(skip);
	a.op(0, false, 0xf6, 1, 0, RM::m(R11, kEhRbp), true, 1); // test byte [r11+saved rbp], active
	a.imm(kEhActive, 1);
	a.jcc_short(CC_E, found);
	a.mov_load(R11, RM::m(R11, kEhPrev), 8, false);
	a.jmp_short(next);
	a.bind(found);
	a.alu_ri(1, RM::m(R11, kEhRbp), kEhActive, 1); // or byte [r11+saved rbp], active
	a.mov_store(RM::rip(runtime.eh_top, 0), R11, 8);
	const Reg saved[5] = { RBX, R12, R13, R14, R15 };
	for (unsigned k = 0; k < 5; ++k)
		a.mov_load(saved[k], RM::m(R11, kEhSaved + 8 * static_cast<int32_t>(k)), 8, false);
	a.mov_load(RBP, RM::m(R11, kEhRbp), 8, false);
	a.alu_ri(4, RM::r(RBP), ~static_cast<int32_t>(kEhStateBits), 8); // and rbp, ~state
	a.mov_load(RSP, RM::m(R11, kEhRsp), 8, false);
	a.mov_store_imm(RM::rip(runtime.eh_selector, 0), 0, 8);
	a.op(0, false, 0xff, 1, 4, RM::m(R11, kEhTarget)); // jmp [r11+target]
	a.resolve_labels();
	image.add_code_fixups(a.fixups(), 0);
}

uint32_t emit_runtime(
	Image& image, const lowir::Program& program, const Startup& startup, const RuntimeSymbols& runtime)
{
	emit_dispatcher(image, runtime);
	Assembler a(image.text());
	// Declared functions the runtime provides.
	for (lowir::SymbolId s = 0; s < program.symbols.size(); ++s)
	{
		const lowir::Symbol& symbol = program.symbols[s];
		if (symbol.kind != lowir::SymbolKind::FunctionDeclaration)
			continue;
		if (symbol.metadata.role == ir_model::SR_CPU_QUERY)
		{
			UnwindFacts facts;
			emit_cpu_query(image, s, facts);
			continue;
		}
		if (symbol.metadata.tls_for != lowir::kNoId)
		{
			emit_tls_wrapper(image, s, symbol.metadata.tls_for);
			continue;
		}
		const char* object = symbol.metadata.object != 0 ? program.spelling(symbol.metadata.object) : nullptr;
		if (object != nullptr && builtin_known(object))
			emit_builtin_routine(image, s, object);
	}
	uint32_t exit_state = lowir::kNoId;
	if (startup.atexit != lowir::kNoId)
	{
		exit_state = image.add_symbol();
		image.define(exit_state, Section::Bss, image.reserve(Section::Bss, 24, 8));
		emit_exit_registration(image, startup.atexit, exit_state);
	}
	// Process entry.
	const uint32_t start = begin_routine(image);
	if (startup.init != lowir::kNoId)
		a.call_symbol(startup.init);
	if (startup.entry_params >= 1)
	{
		a.mov_load(RDI, RM::m(RSP, 0), 8, false);
		a.lea(RSI, RM::m(RSP, 8));
	}
	a.call_symbol(startup.entry);
	if (startup.fini != lowir::kNoId || startup.atexit != lowir::kNoId)
	{
		a.mov_rr(R12, RAX);
		if (startup.atexit != lowir::kNoId)
			emit_exit_drain(a, exit_state);
		if (startup.fini != lowir::kNoId)
			a.call_symbol(startup.fini);
		emit_exit(a, R12);
	}
	else
		emit_exit(a, RAX);
	a.resolve_labels();
	image.add_code_fixups(a.fixups(), 0);
	return start;
}

} // namespace native
