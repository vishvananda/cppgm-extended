#pragma once

#include <cstdint>

#include "lex/identifier_table.h"

// Identifier::role values: what an identifier means to the preprocessor
// wherever it appears in a directive or controlling expression.
enum class IdentifierRole : uint8_t
{
	None,
	// Directive names ([cpp] 16p1).
	If,
	Ifdef,
	Ifndef,
	Elif,
	Else,
	Endif,
	Include,
	Define,
	Undef,
	Line,
	Error,
	Pragma,
	// GNU directives of hosted mode (#include_next, #warning).
	IncludeNext,
	Warning,
	// The `defined` operator ([cpp.cond] 16.1p1).
	Defined,
	// The variable-argument parameter ([cpp.replace] 16.3p5).
	VaArgs,
	// The pragma operator ([cpp.pragma.op] 16.9).
	PragmaOperator,
	// `#pragma once`.
	Once,
	// A macro name this implementation predefines ([cpp.predefined] 16.8p1);
	// #define or #undef of it has undefined behavior (16.8p4) and is
	// diagnosed.
	ReservedMacroName
};

inline IdentifierRole RoleOf(const Identifier* identifier)
{
	return static_cast<IdentifierRole>(identifier->role);
}

// Interns the names the preprocessor recognizes and records their roles.
struct PreprocessorNames
{
	explicit PreprocessorNames(IdentifierTable& identifiers);

	Identifier* defined;
	Identifier* va_args;
	Identifier* pragma_operator;
	Identifier* has_cpp_attribute;
	Identifier* has_attribute;
	Identifier* file;
	Identifier* line;
	Identifier* counter;
};
