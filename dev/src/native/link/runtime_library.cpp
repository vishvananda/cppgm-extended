#include "native/link/runtime_library.h"

#include <cstring>

#include "lowir/program.h"
#include "native/image/image.h"
#include "native/link/runtime_support.h"
#include "native/object/object_compiler.h"
#include "native/runtime/runtime.h"

namespace native {

const char kInitArrayStart[] = "__cppgm_init_array_start";
const char kInitArrayEnd[] = "__cppgm_init_array_end";
const char kFiniArrayStart[] = "__cppgm_fini_array_start";
const char kFiniArrayEnd[] = "__cppgm_fini_array_end";
const char kEntrySymbol[] = "_start";

using object::Binding;
using object::ObjectFile;
using object::SectionKind;
using object::SymbolType;

namespace {

// Members encoded directly (machine-level) and the LowIR ones
// (native/link/runtime_support.h) after them.
enum Member : uint8_t
{
	kEntry,       // _start, __cppgm_exit, the exit handler list
	kEhCore,      // the handler stack state and dispatcher
	kSystem,      // __cppgm_mmap, __cppgm_munmap, __cppgm_mremap
	kTerminate,   // std::terminate, __cxa_pure_virtual, abort
	kTypeVtables, // the type_info classes' vtables
	kBuiltins,    // string builtins, by name
	kFundamental, // fundamental types' type information, by name
	kSupport      // + SupportMember
};

const uint32_t kMembers = kSupport + kSupportMembers;

struct Provided
{
	const char* name;
	uint32_t member;
};

const char* const kTypeVtableNames[] = {
	"_ZTVN10__cxxabiv117__class_type_infoE",
	"_ZTVN10__cxxabiv120__si_class_type_infoE",
	"_ZTVN10__cxxabiv121__vmi_class_type_infoE",
	"_ZTVN10__cxxabiv123__fundamental_type_infoE",
	"_ZTVN10__cxxabiv119__pointer_type_infoE",
	"_ZTVN10__cxxabiv129__pointer_to_member_type_infoE",
	"_ZTVN10__cxxabiv120__function_type_infoE",
	"_ZTVN10__cxxabiv117__array_type_infoE",
	"_ZTVN10__cxxabiv116__enum_type_infoE",
};
const unsigned kFundamentalVtable = 3;
const unsigned kPointerVtable = 4;

const Provided kProvided[] = {
	{ kEntrySymbol, kEntry },
	{ "__cppgm_exit", kEntry },
	{ "__cppgm_atexit_head", kEntry },
	{ "__cppgm_thread_atexit_head", kEntry },
	{ "__dso_handle", kEntry },
	{ kEhTopSymbol, kEhCore },
	{ kEhPayloadSymbol, kEhCore },
	{ kEhSelectorSymbol, kEhCore },
	{ kEhDispatchSymbol, kEhCore },
	{ kEhUncaughtSymbol, kEhCore },
	{ "__cppgm_mmap", kSystem },
	{ "__cppgm_munmap", kSystem },
	{ "__cppgm_mremap", kSystem },
	{ "_ZSt9terminatev", kTerminate },
	{ "__cxa_pure_virtual", kTerminate },
	{ "__cxa_deleted_virtual", kTerminate },
	{ "abort", kTerminate },
	{ "cppgm_builtin_strlen", kBuiltins },
	{ "cppgm_builtin_memcpy", kBuiltins },
	{ "cppgm_builtin_memmove", kBuiltins },
	{ "cppgm_builtin_memset", kBuiltins },
	{ "cppgm_builtin_memcmp", kBuiltins },
	// The C names freestanding GCC/Clang code calls (their -ffreestanding
	// contract).
	{ "strlen", kBuiltins },
	{ "memcpy", kBuiltins },
	{ "memmove", kBuiltins },
	{ "memset", kBuiltins },
	{ "memcmp", kBuiltins },
};

// Members a member's code refers to.
const uint32_t kDependencies[kMembers][4] = {
	{ kMembers, kMembers, kMembers, kMembers },                    // kEntry
	{ kMembers, kMembers, kMembers, kMembers },                    // kEhCore
	{ kMembers, kMembers, kMembers, kMembers },                    // kSystem
	{ kMembers, kMembers, kMembers, kMembers },                    // kTerminate
	{ kMembers, kMembers, kMembers, kMembers },                    // kTypeVtables
	{ kMembers, kMembers, kMembers, kMembers },                    // kBuiltins
	{ kTypeVtables, kMembers, kMembers, kMembers },                // kFundamental
	{ kSystem, kTerminate, kMembers, kMembers },                   // kAllocation
	{ kSupport + kAllocation, kEntry, kMembers, kMembers },        // kExitHandlers
	{ kSupport + kAllocation, kEhCore, kTerminate, kTypeVtables }, // kExceptions
	{ kTypeVtables, kMembers, kMembers, kMembers },                // kDynamicCast
	{ kSupport + kExceptions, kTypeVtables, kMembers, kMembers },  // kBadCast
	{ kMembers, kMembers, kMembers, kMembers },                    // kWideDivision
	{ kMembers, kMembers, kMembers, kMembers },                    // kFloatConversion
};

// Itanium 5.1.5 builtin types the runtime describes (libsupc++'s set):
// void, wchar_t, bool, the character and integer types, __int128, the
// floating types, nullptr_t, char16_t and char32_t.
bool fundamental_type(const char* code, size_t length)
{
	if (length == 1)
		return std::strchr("vwbcahstijlmxynofde", code[0]) != nullptr;
	return length == 2 && code[0] == 'D' && (code[1] == 'n' || code[1] == 's' || code[1] == 'i');
}

// A fundamental type, or a pointer to one (cv-qualified: K const, V
// volatile, VK both): its pointer qualification flags (Itanium 2.9.5p7)
// and the pointee's code. Returns false for other types.
bool runtime_type(const char* code, unsigned& flags, const char*& pointee, bool& pointer)
{
	const size_t length = std::strlen(code);
	pointer = false;
	flags = 0;
	pointee = code;
	if (fundamental_type(code, length))
		return true;
	if (code[0] != 'P')
		return false;
	pointer = true;
	pointee = code + 1;
	if (*pointee == 'V')
	{
		flags |= 2;
		++pointee;
	}
	if (*pointee == 'K')
	{
		flags |= 1;
		++pointee;
	}
	return fundamental_type(pointee, std::strlen(pointee));
}

// `_ZTI<type>` or `_ZTS<type>` of a runtime type: its type code.
const char* runtime_type_name(const char* name, bool& information)
{
	if (std::strncmp(name, "_ZTI", 4) != 0 && std::strncmp(name, "_ZTS", 4) != 0)
		return nullptr;
	information = name[3] == 'I';
	unsigned flags;
	const char* pointee;
	bool pointer;
	return runtime_type(name + 4, flags, pointee, pointer) ? name + 4 : nullptr;
}

uint32_t member_of(const char* name)
{
	for (const Provided& provided : kProvided)
	{
		if (std::strcmp(provided.name, name) == 0)
			return provided.member;
	}
	for (const char* vtable : kTypeVtableNames)
	{
		if (std::strcmp(vtable, name) == 0)
			return kTypeVtables;
	}
	bool information;
	if (runtime_type_name(name, information) != nullptr)
		return kFundamental;
	const SupportMember support = support_member_of(name);
	return support == kSupportMembers ? kMembers : kSupport + support;
}

// The machine-level members, encoded into one image whose symbols carry
// object names (unnamed ones are locals: labels and constants).
class CoreBuilder
{
public:
	uint32_t named(const std::string& name);
	void define_data(uint32_t symbol, Section section, uint64_t bytes, uint64_t align);
	void to_object(ObjectFile& out) const;

	Image image;

private:
	std::vector<std::string> names_;
};

uint32_t CoreBuilder::named(const std::string& name)
{
	for (size_t i = 0; i < names_.size(); ++i)
	{
		if (names_[i] == name)
			return Image::kLocalSymbols + static_cast<uint32_t>(i);
	}
	names_.push_back(name);
	return image.add_symbol();
}

void CoreBuilder::define_data(uint32_t symbol, Section section, uint64_t bytes, uint64_t align)
{
	image.define(symbol, section, image.reserve(section, bytes, align));
}

void CoreBuilder::to_object(ObjectFile& out) const
{
	const uint32_t text = out.add_section(".text", SectionKind::Text, 16);
	const uint32_t rodata = out.add_section(".rodata", SectionKind::Rodata, 16);
	const uint32_t data = out.add_section(".data", SectionKind::Data, 16);
	const uint32_t bss = out.add_section(".bss", SectionKind::Bss, 64);
	out.sections[text].bytes = image.text();
	out.sections[rodata].bytes = image.rodata();
	out.sections[data].bytes = image.data();
	for (const uint32_t s : { text, rodata, data })
		out.sections[s].size = out.sections[s].bytes.size();
	out.sections[bss].size = image.bss_size();
	std::vector<uint32_t> symbol_of(image.local_count(), 0);
	for (uint32_t i = 0; i < image.local_count(); ++i)
	{
		const ImageSymbol& from = image.symbol(Image::kLocalSymbols + i);
		object::Symbol symbol;
		symbol.section = from.section == Section::Text ? text
			: from.section == Section::Rodata          ? rodata
			: from.section == Section::Data            ? data
			: from.section == Section::Bss             ? bss
													   : object::kUndefinedSection;
		symbol.value = from.offset;
		symbol.type = from.section == Section::Text ? SymbolType::Function : SymbolType::Object;
		if (i < names_.size() && !names_[i].empty())
		{
			symbol.name = out.add_string(names_[i]);
			symbol.binding = symbol.section == object::kUndefinedSection ? Binding::Global : Binding::Weak;
			if (symbol.section == object::kUndefinedSection)
				symbol.type = SymbolType::None;
		}
		symbol_of[i] = out.add_symbol(symbol);
	}
	for (const CodeFixup& fixup : image.code_fixups())
	{
		object::Relocation relocation;
		relocation.offset = fixup.offset;
		relocation.symbol = symbol_of[fixup.target - Image::kLocalSymbols];
		relocation.type = fixup.absolute ? object::R_X86_64_64 : object::R_X86_64_PC32;
		relocation.addend = fixup.absolute ? fixup.addend : static_cast<int64_t>(fixup.addend) - fixup.end;
		out.sections[text].relocations.push_back(relocation);
	}
	for (const DataFixup& fixup : image.data_fixups())
	{
		object::Relocation relocation;
		relocation.offset = fixup.offset;
		relocation.symbol = symbol_of[fixup.target - Image::kLocalSymbols];
		relocation.type = object::R_X86_64_64;
		relocation.addend = fixup.addend;
		out.sections[fixup.section == Section::Rodata ? rodata : data].relocations.push_back(relocation);
	}
}

void align_code(Image& image)
{
	while (image.text().size() % 16 != 0)
		image.text().push_back(0xcc);
}

void begin(CoreBuilder& core, uint32_t symbol)
{
	align_code(core.image);
	core.image.define(symbol, Section::Text, core.image.text().size());
}

void emit_syscall(Assembler& a)
{
	a.byte(0x0f);
	a.byte(0x05);
}

// _start: the initialization arrays in order, main(argc, argv, envp), then
// __cppgm_exit with its result. __dso_handle: the DSO identity
// __cxa_atexit registrations name (Itanium 3.3.5). __cppgm_exit(status): the
// thread's exit functions (__cxa_thread_atexit's: its thread-local objects
// are destroyed before any object of static storage duration, 3.6.3p1),
// then the exit handlers (atexit's and __cxa_atexit's), each a list of
// {function, next, argument} nodes, most recent first; the finalization
// arrays last to first, then exit_group(status).
void emit_entry(CoreBuilder& core)
{
	Image& image = core.image;
	Assembler a(image.text());
	const uint32_t start = core.named(kEntrySymbol);
	const uint32_t exit = core.named("__cppgm_exit");
	const uint32_t head = core.named("__cppgm_atexit_head");
	core.define_data(head, Section::Bss, 8, 8);
	const uint32_t thread_head = core.named("__cppgm_thread_atexit_head");
	core.define_data(thread_head, Section::Bss, 8, 8);
	core.define_data(core.named("__dso_handle"), Section::Bss, 8, 8);
	begin(core, start);
	const uint32_t init_loop = a.new_label();
	const uint32_t init_done = a.new_label();
	a.mov_rr(R12, RSP);
	a.lea(RBX, RM::rip(core.named(kInitArrayStart), 0));
	a.lea(R13, RM::rip(core.named(kInitArrayEnd), 0));
	a.bind(init_loop);
	a.alu_rr(7, RBX, RM::r(R13), 8); // cmp rbx, r13
	a.jcc_short(CC_AE, init_done);
	a.call_rm(RM::m(RBX, 0));
	a.alu_ri(0, RM::r(RBX), 8, 8);
	a.jmp_short(init_loop);
	a.bind(init_done);
	a.mov_load(RDI, RM::m(R12, 0), 8, false);
	a.lea(RSI, RM::m(R12, 8));
	a.lea(RDX, RM::m(R12, 16, RDI, 8));
	a.call_symbol(core.named("main"));
	a.op(0, false, 0x8b, 1, RDI, RM::r(RAX)); // mov edi, eax
	a.call_symbol(exit);
	a.resolve_labels();
	begin(core, exit);
	const uint32_t fini_loop = a.new_label();
	const uint32_t done = a.new_label();
	a.push(RBP);
	a.op(0, false, 0x8b, 1, R12, RM::r(RDI)); // mov r12d, edi
	// Each list popped one node at a time (a function may register more).
	const uint32_t lists[2] = { thread_head, head };
	for (const uint32_t list : lists)
	{
		const uint32_t pop = a.new_label();
		const uint32_t empty = a.new_label();
		a.bind(pop);
		a.mov_load(RAX, RM::rip(list, 0), 8, false);
		a.op(0, true, 0x85, 1, RAX, RM::r(RAX)); // test rax, rax
		a.jcc_short(CC_E, empty);
		a.mov_load(RCX, RM::m(RAX, 8), 8, false);
		a.mov_store(RM::rip(list, 0), RCX, 8);
		a.mov_load(RDI, RM::m(RAX, 16), 8, false);
		a.call_rm(RM::m(RAX, 0));
		a.jmp_short(pop);
		a.bind(empty);
	}
	a.lea(RBX, RM::rip(core.named(kFiniArrayEnd), 0));
	a.lea(R13, RM::rip(core.named(kFiniArrayStart), 0));
	a.bind(fini_loop);
	a.alu_rr(7, RBX, RM::r(R13), 8);
	a.jcc_short(CC_BE, done);
	a.alu_ri(5, RM::r(RBX), 8, 8);
	a.call_rm(RM::m(RBX, 0));
	a.jmp_short(fini_loop);
	a.bind(done);
	emit_exit(a, R12);
	a.resolve_labels();
	image.add_code_fixups(a.fixups(), 0);
}

void emit_eh_core(CoreBuilder& core)
{
	RuntimeSymbols runtime;
	runtime.eh_top = core.named(kEhTopSymbol);
	runtime.eh_payload = core.named(kEhPayloadSymbol);
	runtime.eh_selector = core.named(kEhSelectorSymbol);
	runtime.eh_dispatch = core.named(kEhDispatchSymbol);
	runtime.eh_uncaught = core.named(kEhUncaughtSymbol);
	core.define_data(runtime.eh_top, Section::Bss, 8, 8);
	core.define_data(runtime.eh_payload, Section::Bss, 16, 16);
	core.define_data(runtime.eh_selector, Section::Bss, 8, 8);
	emit_dispatcher(core.image, runtime);
}

// __cppgm_mmap(bytes): fresh zeroed read-write pages, or null.
// __cppgm_munmap(address, bytes). __cppgm_mremap(address, bytes, new_bytes):
// the mapping grown, perhaps moved, or null.
void emit_system(CoreBuilder& core)
{
	Assembler a(core.image.text());
	begin(core, core.named("__cppgm_mmap"));
	const uint32_t done = a.new_label();
	a.mov_rr(RSI, RDI);
	a.op(0, false, 0x33, 1, RDI, RM::r(RDI)); // xor edi, edi
	a.mov_ri(RDX, 3);                         // PROT_READ | PROT_WRITE
	a.mov_ri(R10, 0x22);                      // MAP_PRIVATE | MAP_ANONYMOUS
	a.mov_ri(R8, -1);
	a.op(0, false, 0x33, 1, R9, RM::r(R9)); // xor r9d, r9d
	a.mov_ri(RAX, 9);
	emit_syscall(a);
	a.alu_ri(7, RM::r(RAX), -4096, 8); // cmp rax, -4096
	a.jcc_short(CC_BE, done);
	a.op(0, false, 0x33, 1, RAX, RM::r(RAX));
	a.bind(done);
	a.ret();
	a.resolve_labels();
	begin(core, core.named("__cppgm_munmap"));
	a.mov_ri(RAX, 11);
	emit_syscall(a);
	a.ret();
	begin(core, core.named("__cppgm_mremap"));
	const uint32_t moved = a.new_label();
	a.mov_ri(R10, 1); // MREMAP_MAYMOVE
	a.mov_ri(RAX, 25);
	emit_syscall(a);
	a.alu_ri(7, RM::r(RAX), -4096, 8);
	a.jcc_short(CC_BE, moved);
	a.op(0, false, 0x33, 1, RAX, RM::r(RAX));
	a.bind(moved);
	a.ret();
	a.resolve_labels();
}

// std::terminate and the routines that end in it: the default terminate
// handler aborts; the process exits with the status a SIGABRT death has.
void emit_terminate(CoreBuilder& core)
{
	Assembler a(core.image.text());
	const uint32_t terminate = core.named("_ZSt9terminatev");
	begin(core, terminate);
	a.mov_ri(RDI, 134);
	emit_exit(a, RDI);
	for (const char* name : { "__cxa_pure_virtual", "__cxa_deleted_virtual", "abort" })
	{
		begin(core, core.named(name));
		a.jmp_symbol(terminate);
	}
	core.image.add_code_fixups(a.fixups(), 0);
}

// Each type_info class's vtable: offset to top, type information and the
// destructor slots, all zero; type_info objects point 16 bytes in and the
// runtime tells the classes apart by these addresses.
void emit_type_vtables(CoreBuilder& core)
{
	for (const char* name : kTypeVtableNames)
		core.define_data(core.named(name), Section::Rodata, 32, 16);
}

// A runtime type's name (`_ZTS`) and type_info object (`_ZTI`):
// __fundamental_type_info {vptr, name}, or __pointer_type_info {vptr,
// name, flags, pointee}.
void emit_runtime_type(CoreBuilder& core, const std::string& code)
{
	Image& image = core.image;
	const uint32_t name = core.named("_ZTS" + code);
	if (image.symbol(name).section == Section::Undefined)
	{
		const uint64_t at = image.reserve(Section::Rodata, code.size() + 1, 1);
		std::memcpy(image.rodata().data() + at, code.c_str(), code.size() + 1);
		image.define(name, Section::Rodata, at);
	}
	const uint32_t information = core.named("_ZTI" + code);
	if (image.symbol(information).section != Section::Undefined)
		return;
	unsigned flags;
	const char* pointee;
	bool pointer;
	runtime_type(code.c_str(), flags, pointee, pointer);
	const uint64_t at = image.reserve(Section::Rodata, pointer ? 32 : 16, 16);
	image.define(information, Section::Rodata, at);
	image.add_data_fixup(
		Section::Rodata, at, core.named(kTypeVtableNames[pointer ? kPointerVtable : kFundamentalVtable]), 16);
	image.add_data_fixup(Section::Rodata, at + 8, name, 0);
	if (!pointer)
		return;
	image.rodata()[at + 16] = static_cast<uint8_t>(flags);
	image.add_data_fixup(Section::Rodata, at + 24, core.named("_ZTI" + std::string(pointee)), 0);
	emit_runtime_type(core, pointee);
}

void build_core(const std::vector<uint8_t>& members, const std::vector<std::string>& names, ObjectFile& out)
{
	CoreBuilder core;
	if (members[kEntry])
		emit_entry(core);
	if (members[kEhCore])
		emit_eh_core(core);
	if (members[kSystem])
		emit_system(core);
	if (members[kTerminate])
		emit_terminate(core);
	if (members[kTypeVtables])
		emit_type_vtables(core);
	// The exception matcher compares with void's and nullptr_t's.
	if (members[kSupport + kExceptions])
	{
		emit_runtime_type(core, "v");
		emit_runtime_type(core, "Dn");
	}
	for (const std::string& name : names)
	{
		const uint32_t member = member_of(name.c_str());
		if (member == kBuiltins)
		{
			const std::string builtin = name.compare(0, 14, "cppgm_builtin_") == 0 ? name : "cppgm_builtin_" + name;
			emit_builtin_routine(core.image, core.named(name), builtin.c_str());
		}
		bool information;
		if (member == kFundamental)
			emit_runtime_type(core, runtime_type_name(name.c_str(), information));
	}
	core.to_object(out);
}

} // namespace

bool runtime_provides(const char* name)
{
	return member_of(name) != kMembers;
}

bool build_runtime(const std::vector<std::string>& names, std::vector<ObjectFile>& out, std::string& error)
{
	std::vector<uint8_t> members(kMembers, 0);
	std::vector<uint32_t> work(1, kEntry);
	for (const std::string& name : names)
		work.push_back(member_of(name.c_str()));
	while (!work.empty())
	{
		const uint32_t member = work.back();
		work.pop_back();
		if (member >= kMembers || members[member])
			continue;
		members[member] = 1;
		for (const uint32_t dependency : kDependencies[member])
			work.push_back(dependency);
	}
	out.push_back(ObjectFile());
	out.back().path = "<runtime library>";
	build_core(members, names, out.back());
	bool support = false;
	for (uint32_t member = kSupport; member < kMembers; ++member)
		support = support || members[member];
	if (!support)
		return true;
	lowir::Program program;
	RuntimeProgram runtime(program);
	for (uint32_t member = kSupport; member < kMembers; ++member)
	{
		if (members[member])
			build_support_member(runtime, static_cast<SupportMember>(member - kSupport));
	}
	out.push_back(ObjectFile());
	ObjectFile& object = out.back();
	object.path = "<runtime library>";
	if (!compile_object(program, object, error, nullptr))
	{
		error = "runtime library: " + error;
		return false;
	}
	// The library's definitions yield to the program's.
	for (object::Symbol& symbol : object.symbols)
	{
		if (symbol.binding == Binding::Global && symbol.section != object::kUndefinedSection)
			symbol.binding = Binding::Weak;
	}
	return true;
}

} // namespace native
