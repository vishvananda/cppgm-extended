#pragma once

#include "wasm/mir.h"

namespace wasm {

enum class Passing : uint8_t
{
	Direct,
	Ignore,
	Coerce,
	Expand,
	Indirect
};

struct AbiValue
{
	Passing passing;
	lowir::Type scalar;
};

struct Boundary
{
	std::vector<AbiValue> parameters;
	AbiValue result;
};

struct VariadicArgument
{
	lowir::Type type;
	AbiValue passing;
	lowir::Type storage;
};
struct VariadicPlan
{
	std::vector<VariadicArgument> arguments;
	uint32_t alignment_log = 2;
};

inline bool memory_value(lowir::Type type)
{
	return type.is_object() || type.kind() == lowir::TypeKind::I128;
}

void legalize_wide_helpers(lowir::Program& program);

AbiValue classify_value(lowir::Type type, ir_model::AggregateShape shape, lowir::Type pointer);
lowir::Type variadic_storage(lowir::Type type, AbiValue passing);

} // namespace wasm
