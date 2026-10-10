#pragma once

#include <cstdint>

// A borrowed view of a token's UTF-8 spelling.
struct TokenSpelling
{
	const char* text;
	uint32_t length;
};
