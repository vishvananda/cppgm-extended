#pragma once

#include "native/object/macho_object.h"

namespace native {
namespace object {
namespace macho {

struct InputSection
{
	std::string name;
	SectionKind kind;
	uint64_t address;
	uint64_t size;
	uint32_t offset;
	uint32_t align;
	uint32_t relocations;
	uint32_t count;
	std::vector<uint64_t> boundaries;
	std::vector<uint32_t> atoms;
};

struct InputSymbol
{
	std::string name;
	uint8_t type;
	uint8_t section;
	uint16_t desc;
	uint64_t value;
	uint32_t output = 0;
};

class Reader
{
public:
	Reader(const std::vector<uint8_t>& bytes, ObjectFile& object, std::string& error)
		: bytes_(bytes), object_(object), error_(error)
	{}
	bool read();

private:
	bool fail(const char* message);
	bool room(uint64_t at, uint64_t count) const;
	uint64_t get(uint64_t at, unsigned width) const;
	std::string name(uint64_t at) const;
	bool commands();
	bool symbols();
	bool atoms();
	bool relocations();
	bool relocation(uint32_t section, uint32_t& index);
	uint32_t atom(uint32_t section, uint64_t offset) const;
	uint32_t section_symbol(uint32_t section, uint64_t address, int64_t& addend);
	const std::vector<uint8_t>& bytes_;
	ObjectFile& object_;
	std::string& error_;
	std::vector<InputSection> sections_;
	std::vector<InputSymbol> symbols_;
	uint32_t symtab_ = 0;
	uint32_t symbol_count_ = 0;
	uint32_t strtab_ = 0;
	uint32_t strings_ = 0;
	std::vector<uint32_t> section_symbols_;
};

} // namespace macho
} // namespace object
} // namespace native
