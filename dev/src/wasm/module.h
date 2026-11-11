#pragma once

#include <vector>

#include "backend/options.h"
#include "lowir/program.h"
#include "wasm/encoder.h"

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
	std::vector<uint32_t> signature_types;
	std::vector<uint32_t> function_indices;
	std::vector<uint32_t> addresses;
	std::vector<lowir::SymbolId> definitions;
	Bytes data;
	uint32_t stack_floor = 0;
	uint32_t stack_top = 0;
};

void prepare_module(const lowir::Program& program, Module& module);
void encode_module(lowir::Program& program, const backend::Options& options, Bytes& output, std::string& dump);

} // namespace wasm
