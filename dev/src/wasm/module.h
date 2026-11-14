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
	std::vector<ValueType> results;
	bool operator==(const Signature& other) const { return results == other.results && parameters == other.parameters; }
};

struct Module
{
	std::vector<Signature> signatures;
	std::vector<ValueType> global_types{ ValueType::I32 };
	std::vector<bool> mutable_globals{ true };
	std::vector<Boundary> boundaries;
	std::vector<uint32_t> signature_types;
	std::vector<uint32_t> function_indices;
	std::vector<uint32_t> definition_types;
	std::vector<uint32_t> tag_types;
	std::vector<uint8_t> declared_functions;
	uint32_t memory_count = 1;
	uint32_t table_count = 1;
	std::vector<uint32_t> addresses;
	std::vector<lowir::SymbolId> definitions;
	uint32_t stack_floor = 0;
	uint32_t initializer = lowir::kNoId;
	bool relocatable = false;
	bool atomics = false;
	uint32_t stack_symbol = lowir::kNoId;
	uint32_t stack_low_symbol = lowir::kNoId;
	uint32_t table_symbol = lowir::kNoId;
	bool exception_regions = false;
	lowir::ExceptionAbi exception_abi = lowir::ExceptionAbi::Opaque;
	uint32_t personality_symbol = lowir::kNoId;
	uint32_t exception_context = lowir::kNoId;
	uint32_t exception_tag_symbol = lowir::kNoId;
	uint32_t exception_pair_type = lowir::kNoId;
};

void prepare_module(const lowir::Program& program, Module& module);
void dump_module(const lowir::Program& program, const Module& module, std::string& output);
void dump_function(
	const Function& function, const lowir::Program& program, lowir::SymbolId symbol, std::string& output);

} // namespace wasm
