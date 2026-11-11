#pragma once

#include <cstdint>

// Machine vocabulary shared by register and stack-machine targets. These
// facts describe identities and storage, not a physical ISA's registers.
namespace machine_model {

struct Range
{
	std::uint32_t first;
	std::uint32_t count;
};

enum class LocationKind : std::uint8_t
{
	None,
	Register,
	IncomingStack,
	FrameObject,
	Local
};

struct Location
{
	LocationKind kind;
	std::uint32_t index;
	std::int64_t offset;
};

struct FrameObject
{
	std::uint64_t bytes;
	std::uint32_t alignment;
	std::uint32_t identity;
};

} // namespace machine_model
