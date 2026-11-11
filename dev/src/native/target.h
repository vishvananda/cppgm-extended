#pragma once

#include <cstdint>
#include <string>

#include "target/target.h"

namespace native {

// Preserve the encoder's code phase when functions move into object sections.
const uint32_t kCodeLayoutBytes = 64;

using target_model::Target;

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
	return target_model::spelling(target);
}

// A Darwin suffix is the deployment target; native CLI aliases are retained.
bool parse_target(const std::string& text, Target& target);

} // namespace native
