#pragma once

#include "native/object/macho_object.h"
#include <map>

namespace native {
namespace object {
namespace macho {

inline uint64_t align_up(uint64_t value, uint64_t align)
{
	return (value + align - 1) & ~(align - 1);
}
void put(std::vector<uint8_t>& bytes, uint64_t value, unsigned width);
void patch(std::vector<uint8_t>& bytes, uint64_t at, uint64_t value, unsigned width);
void name(std::vector<uint8_t>& bytes, const std::string& value);

struct Reloc
{
	uint32_t at;
	uint32_t bits;
};

struct Section
{
	std::string segment;
	std::string name;
	uint32_t flags;
	uint64_t align = 1;
	uint64_t size = 0;
	uint64_t address = 0;
	uint32_t offset = 0;
	uint32_t relocations = 0;
	std::vector<uint8_t> bytes;
	std::vector<uint64_t> records; // cached CIE/FDE starts, only for __eh_frame
	// Each inner list is one relocation, or an inseparable SUBTRACTOR pair.
	std::vector<std::vector<Reloc>> fixups;
};

struct Symbol
{
	std::string name;
	uint8_t type;
	uint8_t section;
	uint16_t desc;
	uint64_t value;
	uint32_t string;
};

struct Placement
{
	uint32_t section; // zero-based output section
	uint64_t base;
};

class Encoder
{
public:
	Encoder(const ObjectFile& input, std::string& error) : input_(input), error_(error) {}
	bool encode(std::vector<uint8_t>& bytes);

private:
	bool sections();
	bool symbols();
	bool relocations();
	bool relocation(uint32_t section, const Relocation& relocation);
	void write(std::vector<uint8_t>& bytes);
	uint32_t section(const char* segment, const std::string& name, uint32_t flags);
	uint32_t symbol(const std::string& name, uint8_t type, uint8_t section, uint16_t desc, uint64_t value);
	void tlv(uint32_t original, const object::Symbol& source, Symbol& target);
	bool fail(const std::string& message);
	void fixup(uint32_t section, uint32_t at, uint32_t symbol, uint32_t type, unsigned width, bool pcrel,
		bool external = true);
	uint32_t base_symbol(uint32_t section, uint64_t offset);
	const ObjectFile& input_;
	std::string& error_;
	std::vector<Section> sections_;
	std::vector<Symbol> symbols_;
	std::vector<Placement> placements_;
	std::vector<uint32_t> symbol_index_;
	std::map<std::pair<uint32_t, uint64_t>, uint32_t> address_symbols_;
	uint32_t bootstrap_ = 0;
	uint32_t tlv_section_ = 0;
	bool has_tlv_ = false;
};

} // namespace macho
} // namespace object
} // namespace native
