#pragma once

#include <cstdint>
#include <cstring>

#include "lowir/name_table.h"
#include "lowir/types.h"
#include "lowir_records.h"

// The LowIR instruction vocabulary (lowir.md "Required PA8 Instructions",
// "PA17 Exception/Runtime Instructions"): operations, operand values,
// literal bits and the instruction record of a function body.
namespace lowir {

using lowir_model::kNoId;

// A literal outside a function body (global initializers and data items).
struct Constant
{
	ValueKind kind;      // Integer or Float
	TypeKind float_kind; // for Float
	Literal bits;
};

// A `!dbg(file, line, column)` location; file 0 means absent.
struct DebugLocation
{
	NameId file;
	uint32_t line;
	uint32_t column;
	// Global optional DebugInfo lexical scope id; zero is function scope.
	uint32_t scope;

	bool present() const { return file != 0; }
};

} // namespace lowir
