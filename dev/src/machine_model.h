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

// Conservative observable effects; a target may refine these for a typed call.
enum Effect : std::uint16_t
{
	ReadMemory = 1,
	WriteMemory = 2,
	ReadGlobal = 4,
	WriteGlobal = 8,
	Call = 16,
	Control = 32,
	MayTrap = 64,
	MayThrow = 128
};

inline const char* spelling(Effect effect)
{
	switch (effect)
	{
	case ReadMemory:
		return "read_memory";
	case WriteMemory:
		return "write_memory";
	case ReadGlobal:
		return "read_global";
	case WriteGlobal:
		return "write_global";
	case Call:
		return "call";
	case Control:
		return "control";
	case MayTrap:
		return "may_trap";
	case MayThrow:
		return "may_throw";
	}
	return "";
}

} // namespace machine_model
