#pragma once

#include <cstdint>
#include <string>
#include <vector>

// A relocatable object: what `cppgm++ -c` writes and link mode reads, for
// this compiler's objects and host-compiled helper objects alike. It keeps
// the ELF64 x86-64 relocatable vocabulary (sections, symbols, RELA
// relocations, COMDAT groups) restricted to the facts linking consumes:
// the reader drops what a static executable never loads (notes, unwind
// tables, debug sections) and the writer encodes the model as ET_REL
// (native/object/elf_object.h).
namespace native {
namespace object {

// What a section contributes to an executable.
enum class SectionKind : uint8_t
{
	Null,      // section 0, and sections that are not loaded
	Text,      // SHF_ALLOC | SHF_EXECINSTR
	Rodata,    // SHF_ALLOC
	Data,      // SHF_ALLOC | SHF_WRITE
	Bss,       // SHT_NOBITS
	InitArray, // SHT_INIT_ARRAY: startup function pointers, in order
	FiniArray, // SHT_FINI_ARRAY: shutdown function pointers, run last to first
	TlsData,   // SHF_TLS PROGBITS: a thread's initial thread-local bytes (host objects)
	TlsBss     // SHF_TLS NOBITS: zero-initialized thread-local storage
};

enum class Binding : uint8_t
{
	Local,
	Global,
	Weak
};

enum class SymbolType : uint8_t
{
	None,
	Object,
	Function,
	Section,
	File,
	Tls // STT_TLS: a thread-local object (its offset in the TLS block)
};

// Symbol::section values besides section indexes, beyond any index (an
// object with extended section numbering has indexes from 0xff00 up;
// ELF's SHN_ABS and SHN_COMMON in the file).
const uint32_t kUndefinedSection = 0;
const uint32_t kAbsoluteSection = 0xfffffff1u;
const uint32_t kCommonSection = 0xfffffff2u;
const uint32_t kNoGroup = 0xffffffffu;

// x86-64 relocation types (System V psABI 4.4).
enum RelocationType : uint32_t
{
	R_X86_64_NONE = 0,
	R_X86_64_64 = 1,
	R_X86_64_PC32 = 2,
	R_X86_64_PLT32 = 4,
	R_X86_64_GOTPCREL = 9,
	R_X86_64_32 = 10,
	R_X86_64_32S = 11,
	R_X86_64_16 = 12,
	R_X86_64_PC16 = 13,
	R_X86_64_8 = 14,
	R_X86_64_PC8 = 15,
	R_X86_64_GOTTPOFF = 22,
	R_X86_64_TPOFF32 = 23,
	R_X86_64_PC64 = 24,
	R_X86_64_GOTOFF64 = 25,
	R_X86_64_GOTPC32 = 26,
	R_X86_64_GOTPC64 = 29,
	R_X86_64_GOTPCRELX = 41,
	R_X86_64_REX_GOTPCRELX = 42
};

struct Relocation
{
	uint64_t offset;
	uint32_t type;
	uint32_t symbol;
	int64_t addend;
};

struct Section
{
	uint32_t name = 0; // offset in ObjectFile::strings
	SectionKind kind = SectionKind::Null;
	uint64_t align = 1;
	uint64_t size = 0; // bytes.size() except for Bss
	std::vector<uint8_t> bytes;
	std::vector<Relocation> relocations;
	uint32_t group = kNoGroup; // index into ObjectFile::groups
};

struct Symbol
{
	uint32_t name = 0; // offset in ObjectFile::strings
	Binding binding = Binding::Local;
	SymbolType type = SymbolType::None;
	uint32_t section = kUndefinedSection; // section index, or a k*Section value
	uint64_t value = 0;                   // offset in its section; a common symbol's alignment
	uint64_t size = 0;
};

// A COMDAT group: the first object with a signature keeps its members,
// later groups with that signature are discarded whole.
struct Group
{
	uint32_t signature; // index into ObjectFile::symbols
	std::vector<uint32_t> sections;
};

struct ObjectFile
{
	ObjectFile();

	ObjectFile(const ObjectFile&) = delete;
	ObjectFile& operator=(const ObjectFile&) = delete;
	ObjectFile(ObjectFile&&) = default;
	ObjectFile& operator=(ObjectFile&&) = default;

	// Appends a name to the string table; returns its offset.
	uint32_t add_string(const char* text, size_t length);
	uint32_t add_string(const std::string& text) { return add_string(text.data(), text.size()); }
	const char* string(uint32_t offset) const { return strings.data() + offset; }
	const char* symbol_name(uint32_t symbol) const { return string(symbols[symbol].name); }

	uint32_t add_section(const char* name, SectionKind kind, uint64_t align);
	// A section named by a string already in the table.
	uint32_t add_section(uint32_t name, SectionKind kind, uint64_t align);
	uint32_t add_symbol(const Symbol& symbol);

	// The input's name in diagnostics.
	std::string path;
	// NUL-terminated names; offset 0 is the empty name.
	std::vector<char> strings;
	// sections[0] and symbols[0] are null entries, as in ELF.
	std::vector<Section> sections;
	std::vector<Symbol> symbols;
	std::vector<Group> groups;
};

} // namespace object
} // namespace native
