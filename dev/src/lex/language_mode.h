#pragma once

#include <string>

#include "lex/number_literal.h"
#include "preprocess/lexer/pp_lexer.h"
#include "support/tool_errors.h"

// One mode description for the source driver and standalone token tools.
struct LanguageMode
{
	const char* cplusplus;
	bool strict;
	NumberDialect literals;

	PPLexerOptions lexer_options() const
	{
		PPLexerOptions options;
		options.digit_separators = literals != NumberDialect::Cxx11;
		options.binary_exponents = literals == NumberDialect::Cxx17;
		return options;
	}
};

inline LanguageMode ReadLanguageMode(const std::string& standard)
{
	static const struct
	{
		const char* names[4];
		const char* value;
		NumberDialect literals;
	} modes[] = { { { "c++11", "gnu++11", "c++0x", "gnu++0x" }, "201103L", NumberDialect::Cxx11 },
		{ { "c++14", "gnu++14", "c++1y", "gnu++1y" }, "201402L", NumberDialect::Cxx14 },
		{ { "c++17", "gnu++17", "c++1z", "gnu++1z" }, "201703L", NumberDialect::Cxx17 } };
	for (const auto& mode : modes)
		for (const char* name : mode.names)
			if (standard == name)
				return LanguageMode{ mode.value, standard.compare(0, 3, "gnu") != 0, mode.literals };
	throw InvocationError("unsupported language standard: " + standard);
}

inline LanguageMode ReadTokenToolMode(int argc, char** argv)
{
	LanguageMode mode = ReadLanguageMode("c++11");
	for (int i = 1; i < argc; ++i)
	{
		const std::string argument(argv[i]);
		if (argument.compare(0, 5, "-std=") != 0)
			throw InvocationError("invalid usage");
		mode = ReadLanguageMode(argument.substr(5));
	}
	return mode;
}
