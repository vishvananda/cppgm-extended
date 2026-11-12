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

enum class CallingConvention : std::uint8_t
{
	SysVAmd64,
	DarwinAmd64,
	WasmBasicC
};

enum class MemberFunctionTag : std::uint8_t
{
	Function,
	Adjustment
};

struct CxxAbi
{
	std::uint8_t word_bytes;
	MemberFunctionTag member_function_tag;
	bool indirect_member_function;
	bool structor_returns_this;
	std::uint8_t guard_bytes;
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
	CallingConvention calling_convention;
	DataLayout layout;
};

struct OptimizationModel
{
	bool scaled_memory_operands;
	bool locals_survive_calls;
	bool floating_literal_loads;
	bool packed_i32_multiply;
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
		is_wasm(target)               ? CallingConvention::WasmBasicC
			: target == Target::MacOS ? CallingConvention::DarwinAmd64
									  : CallingConvention::SysVAmd64,
		data_layout(target) };
}

constexpr OptimizationModel optimization_model(Target target)
{
	return is_wasm(target) ? OptimizationModel{ false, true, false, true }
						   : OptimizationModel{ true, false, true, false };
}

// The course's Linux runtime provides GNU's combined sin/cos call ABI.
constexpr bool has_gnu_sincos(Target target)
{
	return describe(target).environment == Environment::Linux;
}

constexpr CxxAbi cxx_abi(Target target)
{
	return CxxAbi{ data_layout(target).pointer_bytes,
		is_wasm(target) ? MemberFunctionTag::Adjustment : MemberFunctionTag::Function, is_wasm(target), is_wasm(target),
		is_wasm(target) ? std::uint8_t(4) : std::uint8_t(8) };
}

// An object-to-integer signature rewrite must retain its public call ABI.
// Register-sized integers carry these aggregate bits on the AMD64 ABIs;
// Wasm's typed function signatures distinguish those representations.
constexpr bool aggregate_integer_carriers(Target target)
{
	return describe(target).calling_convention == CallingConvention::SysVAmd64 ||
		describe(target).calling_convention == CallingConvention::DarwinAmd64;
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
