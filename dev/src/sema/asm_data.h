#pragma once

#include <cstdint>
#include <string>
#include <vector>

namespace sema {

// Namespace basic asm's nonloaded PROGBITS data. Analysis lifts the
// directives once; later phases see only section properties and bytes.
struct AsmDataSection
{
	const char* name;
	const uint8_t* bytes;
	uint32_t size;
	bool strings;
};

struct AsmDataImage
{
	std::string name;
	std::vector<uint8_t> bytes;
	bool strings;
};

bool LiftAsmData(const std::string& text, std::vector<AsmDataImage>& sections, std::string& error);

} // namespace sema
