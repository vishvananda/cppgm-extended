#pragma once

#include <vector>

#include "backend/options.h"
#include "lowir/program.h"
#include "wasm/encoder.h"
#include "wasm/abi.h"

namespace wasm {

struct Signature
{
	std::vector<ValueType> parameters;
	ValueType result;
	bool operator==(const Signature& other) const { return result == other.result && parameters == other.parameters; }
};

struct Module
{
	std::vector<Signature> signatures;
	std::vector<Boundary> boundaries;
	std::vector<uint32_t> signature_types;
	std::vector<uint32_t> function_indices;
	std::vector<uint32_t> definition_types;
	std::vector<uint32_t> addresses;
	std::vector<lowir::SymbolId> definitions;
	uint32_t stack_floor = 0;
	uint32_t initializer = lowir::kNoId;
	bool relocatable = false;
	uint32_t stack_symbol = lowir::kNoId;
	uint32_t stack_low_symbol = lowir::kNoId;
	uint32_t table_symbol = lowir::kNoId;
};

void prepare_module(const lowir::Program& program, Module& module);

} // namespace wasm
