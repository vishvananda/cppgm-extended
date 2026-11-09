#pragma once

#include <cstdint>
#include <string>

namespace native {

// Preserve the encoder's code phase when functions move into object sections.
const uint32_t kCodeLayoutBytes = 64;

enum class Target : uint8_t
{
	Linux,
	MacOS
};

inline Target host_target()
{
#if defined(__APPLE__)
	return Target::MacOS;
#else
	return Target::Linux;
#endif
}

inline const char* target_name(Target target)
{
	return target == Target::MacOS ? "macos" : "linux";
}

// Only x86-64 targets. A Darwin version suffix is the deployment target,
// not another architecture; a Linux vendor/environment may be omitted.
bool parse_target(const std::string& text, Target& target);

} // namespace native
