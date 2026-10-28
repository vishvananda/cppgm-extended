#pragma once

#include <cstdint>

// DWARF Version 4, sections 6.2 and 7: https://dwarfstd.org/doc/DWARF4.pdf
namespace native {
namespace dwarf {

enum Tag : uint32_t
{
	ArrayType = 0x01,
	EnumerationType = 0x04,
	FormalParameter = 0x05,
	Member = 0x0d,
	PointerType = 0x0f,
	ReferenceType = 0x10,
	CompileUnit = 0x11,
	StructureType = 0x13,
	SubroutineType = 0x15,
	Typedef = 0x16,
	UnionType = 0x17,
	Inheritance = 0x1c,
	SubrangeType = 0x21,
	BaseType = 0x24,
	ConstType = 0x26,
	Enumerator = 0x28,
	Variable = 0x34,
	VolatileType = 0x35,
	Namespace = 0x39,
	UnspecifiedType = 0x3b,
	RvalueReferenceType = 0x42,
	Subprogram = 0x2e
};

enum Attribute : uint32_t
{
	Location = 0x02,
	Name = 0x03,
	ByteSize = 0x0b,
	BitSize = 0x0d,
	ConstantValue = 0x1c,
	ContainingType = 0x1d,
	Artificial = 0x34,
	Count = 0x37,
	DataMemberLocation = 0x38,
	Declaration = 0x3c,
	Encoding = 0x3e,
	FrameBase = 0x40,
	Virtuality = 0x4c,
	ConstExpr = 0x6c,
	EnumClass = 0x6d,
	TypeReference = 0x49,
	DataBitOffset = 0x6b,
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
	Data1 = 0x0b,
	Flag = 0x0c,
	StringPointer = 0x0e,
	Unsigned = 0x0f,
	Reference4 = 0x13,
	SectionOffset = 0x17,
	ExpressionLocation = 0x18,
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
