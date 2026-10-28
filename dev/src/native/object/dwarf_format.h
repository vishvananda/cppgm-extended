#pragma once

#include <cstdint>

// DWARF Version 4, sections 6.2 and 7: https://dwarfstd.org/doc/DWARF4.pdf
namespace native {
namespace dwarf {

enum Tag : uint32_t
{
	CompileUnit = 0x11,
	Subprogram = 0x2e
};

enum Attribute : uint32_t
{
	Name = 0x03,
	StatementList = 0x10,
	LowPc = 0x11,
	HighPc = 0x12,
	Language = 0x13,
	Producer = 0x25,
	DeclFile = 0x3a,
	DeclLine = 0x3b,
	External = 0x3f,
	Ranges = 0x55,
	LinkageName = 0x6e
};

enum Form : uint32_t
{
	Address = 0x01,
	Data2 = 0x05,
	Data8 = 0x07,
	Flag = 0x0c,
	StringPointer = 0x0e,
	Unsigned = 0x0f,
	SectionOffset = 0x17,
	FlagPresent = 0x19
};

enum LineOpcode : uint8_t
{
	Copy = 1,
	AdvancePc = 2,
	AdvanceLine = 3,
	SetFile = 4,
	SetColumn = 5
};

enum ExtendedLineOpcode : uint8_t
{
	EndSequence = 1,
	SetAddress = 2
};

} // namespace dwarf
} // namespace native
