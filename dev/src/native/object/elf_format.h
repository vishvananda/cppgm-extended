#pragma once

#include <cstdint>
#include <vector>

// ELF64 constants (System V gABI, x86-64 psABI) and little-endian field
// writers shared by the relocatable object and executable encodings.
namespace native {
namespace object {

const unsigned kElfHeaderBytes = 64;
const unsigned kSectionHeaderBytes = 64;
const unsigned kProgramHeaderBytes = 56;
const unsigned kSymbolBytes = 24;
const unsigned kRelaBytes = 24;

const uint16_t kEtRel = 1;
const uint16_t kEtExec = 2;
const uint16_t kEmX86_64 = 62;

const uint32_t kShtProgbits = 1;
const uint32_t kShtSymtab = 2;
const uint32_t kShtStrtab = 3;
const uint32_t kShtRela = 4;
const uint32_t kShtNote = 7;
const uint32_t kShtNobits = 8;
const uint32_t kShtRel = 9;
const uint32_t kShtInitArray = 14;
const uint32_t kShtFiniArray = 15;
const uint32_t kShtPreinitArray = 16;
const uint32_t kShtGroup = 17;
const uint32_t kShtSymtabShndx = 18;

const uint64_t kShfWrite = 0x1;
const uint64_t kShfAlloc = 0x2;
const uint64_t kShfExecinstr = 0x4;
const uint64_t kShfInfoLink = 0x40;
const uint64_t kShfGroup = 0x200;
const uint64_t kShfTls = 0x400;

const uint32_t kShnLoreserve = 0xff00;
const uint32_t kShnAbs = 0xfff1;
const uint32_t kShnCommon = 0xfff2;
const uint32_t kShnXindex = 0xffff;

const uint32_t kGrpComdat = 1;

const uint8_t kStbLocal = 0;
const uint8_t kStbGlobal = 1;
const uint8_t kStbWeak = 2;
const uint8_t kStbGnuUnique = 10;

const uint8_t kSttNotype = 0;
const uint8_t kSttObject = 1;
const uint8_t kSttFunc = 2;
const uint8_t kSttSection = 3;
const uint8_t kSttFile = 4;
const uint8_t kSttCommon = 5;
const uint8_t kSttTls = 6;
const uint8_t kSttGnuIfunc = 10;

inline void put16(std::vector<uint8_t>& out, uint64_t value)
{
	out.push_back(static_cast<uint8_t>(value));
	out.push_back(static_cast<uint8_t>(value >> 8));
}

inline void put32(std::vector<uint8_t>& out, uint64_t value)
{
	for (unsigned k = 0; k < 4; ++k)
		out.push_back(static_cast<uint8_t>(value >> (8 * k)));
}

inline void put64(std::vector<uint8_t>& out, uint64_t value)
{
	for (unsigned k = 0; k < 8; ++k)
		out.push_back(static_cast<uint8_t>(value >> (8 * k)));
}

// ELFCLASS64, ELFDATA2LSB, EV_CURRENT, ELFOSABI_NONE.
inline void put_elf_ident(std::vector<uint8_t>& out)
{
	const uint8_t ident[16] = { 0x7f, 'E', 'L', 'F', 2, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
	out.insert(out.end(), ident, ident + 16);
}

inline uint64_t get_le(const uint8_t* bytes, unsigned count)
{
	uint64_t value = 0;
	for (unsigned k = 0; k < count; ++k)
		value |= static_cast<uint64_t>(bytes[k]) << (8 * k);
	return value;
}

} // namespace object
} // namespace native
