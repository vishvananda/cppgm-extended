#pragma once

#include <vector>
#include "lowir/instruction.h"

namespace lowir {

// Optional source-type facts. Id zero denotes no type/parent. References are
// ids, never source spellings; member offsets are bits from their containing
// object, and array counts are element counts. No instance exists without -g.
enum class DebugKind : uint8_t
{
	Unspecified,
	Base,
	Pointer,
	Reference,
	RvalueReference,
	Array,
	Const,
	Volatile,
	Structure,
	Union,
	Enumeration,
	Enumerator,
	Member,
	Inheritance,
	Namespace,
	Alias,
	Subroutine,
	Parameter,
	Constant,
	Method
};

enum class DebugEncoding : uint8_t
{
	None,
	Boolean,
	Signed,
	SignedChar,
	Unsigned,
	UnsignedChar,
	Floating,
	Complex,
	Address
};

enum DebugFlag : uint32_t
{
	DF_DECLARATION = 1,
	DF_BIT_FIELD = 2,
	DF_POLYMORPHIC = 4,
	DF_VIRTUAL_BASE = 8,
	DF_ARTIFICIAL = 16,
	DF_VIRTUAL_METHOD = 32,
	DF_SCOPED_ENUM = 64
};

struct DebugType
{
	DebugKind kind = DebugKind::Unspecified;
	DebugEncoding encoding = DebugEncoding::None;
	NameId name = 0;
	uint32_t parent = 0;
	uint32_t type = 0;
	uint32_t flags = 0;
	uint64_t bytes = 0;
	uint64_t bit_offset = 0;
	uint64_t count = 0;
	uint64_t constant = 0;
};

struct DebugLocal
{
	SlotId slot;
	uint32_t type;
	NameId name;
	DebugLocation declaration;
};

struct DebugFunction
{
	std::vector<DebugLocal> locals;
};

struct DebugInfo
{
	DebugInfo() : types(1) {}
	std::vector<DebugType> types;
	// Indexed by the program's SymbolId, including declaration-only symbols.
	std::vector<DebugFunction> functions;
};

} // namespace lowir
