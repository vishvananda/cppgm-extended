#pragma once

#include "wasm/mir.h"

namespace wasm {

enum class Passing : uint8_t
{
	Direct,
	Ignore,
	Coerce,
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

AbiValue classify_value(lowir::Type type, ir_model::AggregateShape shape, lowir::Type pointer);

} // namespace wasm
