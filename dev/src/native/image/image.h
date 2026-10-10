#pragma once

#include <cstdint>
#include <string>
#include <unordered_map>
#include <vector>

#include "native/x86/assembler.h"

// The executable image: code, read-only data, initialized data and
// zero-filled storage, a symbol table over all four, and the fixups that
// are patched once the sections are laid out. The writer produces a
// static Linux x86-64 ELF executable with one PT_LOAD per protection
// class (code r-x, constants r--, data rw-).
namespace native {

enum class Section : uint8_t
{
	Undefined,
	Text,
	Rodata,
	Data,
	Bss
};

struct ImageSymbol
{
	Section section = Section::Undefined;
	uint64_t offset = 0;
};

struct DataFixup
{
	Section section;
	uint64_t offset;
	uint32_t target;
	int64_t addend;
};

class Image
{
public:
	Image();

	std::vector<uint8_t>& text() { return text_; }
	std::vector<uint8_t>& rodata() { return rodata_; }
	std::vector<uint8_t>& data() { return data_; }
	const std::vector<uint8_t>& text() const { return text_; }
	const std::vector<uint8_t>& rodata() const { return rodata_; }
	const std::vector<uint8_t>& data() const { return data_; }

	// Two symbol spaces: a program's symbols by their LowIR ids (from 0, as
	// many as are ever defined or referenced, so a program may grow while
	// its functions are emitted), and the image's own (runtime routines and
	// state, constants) from kLocalSymbols, made by add_symbol.
	static const uint32_t kLocalSymbols = 0x40000000u;
	uint32_t add_symbol();
	void define(uint32_t symbol, Section section, uint64_t offset);
	const ImageSymbol& symbol(uint32_t symbol) const
	{
		if (symbol >= kLocalSymbols)
			return locals_[symbol - kLocalSymbols];
		return symbol < symbols_.size() ? symbols_[symbol] : undefined_;
	}
	uint32_t local_count() const { return static_cast<uint32_t>(locals_.size()); }

	// Reserves aligned space at the end of a section; returns its offset.
	uint64_t reserve(Section section, uint64_t bytes, uint64_t align);
	// A read-only constant (deduplicated); returns its symbol.
	uint32_t constant(const void* bytes, uint32_t size, uint32_t align);

	// Code fixups of a function emitted at `base` in text.
	void add_code_fixups(const std::vector<CodeFixup>& fixups, uint64_t base);
	void add_data_fixup(Section section, uint64_t offset, uint32_t target, int64_t addend);
	const std::vector<CodeFixup>& code_fixups() const { return code_fixups_; }
	const std::vector<DataFixup>& data_fixups() const { return data_fixups_; }

	// Lays out the sections, applies every fixup and writes the file.
	// Returns false with `error` set on an I/O error, or on a reference to
	// an undefined symbol, which is then stored in `undefined`.
	bool write_executable(const std::string& path, uint32_t entry, std::string& error, uint32_t& undefined);

	uint64_t bss_size() const { return bss_size_; }

private:
	uint64_t address_of(uint32_t symbol) const;

	std::vector<uint8_t> text_;
	std::vector<uint8_t> rodata_;
	std::vector<uint8_t> data_;
	uint64_t bss_size_ = 0;
	std::vector<ImageSymbol> symbols_;
	std::vector<ImageSymbol> locals_;
	ImageSymbol undefined_;
	std::vector<CodeFixup> code_fixups_;
	std::vector<DataFixup> data_fixups_;
	std::unordered_map<std::string, uint32_t> constants_;
	uint64_t text_address_ = 0;
	uint64_t rodata_address_ = 0;
	uint64_t data_address_ = 0;
	uint64_t bss_address_ = 0;
};

} // namespace native
