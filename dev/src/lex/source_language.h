#pragma once

#include <cstdint>

// The input language is independent of the output target and of a C++
// declaration's language linkage (extern "C").
enum class SourceLanguage : uint8_t
{
	Cxx,
	C
};
