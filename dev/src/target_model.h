#pragma once

#include <cstdint>

// Target vocabulary. Architecture, execution environment, object format and
// language data layout are distinct facts, selected before source analysis.
// Containers and backend algorithms are supplied by the implementation.
namespace target_model {

enum class Architecture : std::uint8_t
{
	X86_64,
	Wasm32,
	AArch64
};

enum class Environment : std::uint8_t
{
	Linux,
	Darwin,
	Freestanding,
	WasiPreview1
};

enum class ObjectFormat : std::uint8_t
{
	Elf,
	MachO,
	WebAssembly
};

enum class ByteOrder : std::uint8_t
{
	Little,
	Big
};

enum class ExtendedFloat : std::uint8_t
{
	X87,
	Binary128
};

enum class Target : std::uint8_t
{
	Linux,
	MacOS,
	Wasm32,
	Wasi
};

struct DataLayout
{
	std::uint8_t pointer_bytes;
	std::uint8_t long_bytes;
	std::uint8_t long_double_bytes;
	std::uint8_t long_double_alignment;
	ExtendedFloat long_double_format;
	ByteOrder byte_order;
};

struct Description
{
	Architecture architecture;
	Environment environment;
	ObjectFormat object_format;
	DataLayout layout;
};

constexpr bool is_wasm(Target target)
{
	return target == Target::Wasm32 || target == Target::Wasi;
}

constexpr DataLayout data_layout(Target target)
{
	return is_wasm(target) ? DataLayout{ 4, 4, 16, 16, ExtendedFloat::Binary128, ByteOrder::Little }
						   : DataLayout{ 8, 8, 16, 16, ExtendedFloat::X87, ByteOrder::Little };
}

constexpr Description describe(Target target)
{
	return Description{ is_wasm(target) ? Architecture::Wasm32 : Architecture::X86_64,
		target == Target::Linux       ? Environment::Linux
			: target == Target::MacOS ? Environment::Darwin
			: target == Target::Wasi  ? Environment::WasiPreview1
									  : Environment::Freestanding,
		is_wasm(target)               ? ObjectFormat::WebAssembly
			: target == Target::MacOS ? ObjectFormat::MachO
									  : ObjectFormat::Elf,
		data_layout(target) };
}

inline const char* spelling(Target target)
{
	switch (target)
	{
	case Target::Linux:
		return "linux";
	case Target::MacOS:
		return "macos";
	case Target::Wasm32:
		return "wasm32-unknown-unknown";
	case Target::Wasi:
		return "wasm32-wasip1";
	}
	return "";
}

// Version-one LowIR profile names; distinct from command-line aliases.
inline const char* lowir_spelling(Target target)
{
	static const char* const names[] = { "linux", "macos", "wasm32", "wasi" };
	return names[static_cast<unsigned>(target)];
}

const unsigned kTargetCount = static_cast<unsigned>(Target::Wasi) + 1;

} // namespace target_model
