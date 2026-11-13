#pragma once

#include <cstdio>

#include "backend/lowir_c.h"
#include "lowir/program.h"

// The C handle owns the ordinary typed program. The foreign boundary lives
// in dev/lowir_library.cpp, alongside the executable exception boundaries.
struct CppgmLowir
{
	lowir::Program program;
	std::vector<uint8_t> object;
	std::vector<uint8_t> direct_bodies;
	char error[1024] = {};
	bool validated = false;
	bool consumed = false;

	bool fail(const char* message)
	{
		std::snprintf(error, sizeof error, "%s", message);
		return false;
	}
	bool ready()
	{
		if (error[0] != 0)
			return false;
		if (consumed)
			return fail("module has already been consumed");
		validated = false;
		return true;
	}
	uint32_t symbol(const char* name, uint32_t length);
	uint32_t signature(CppgmType result, const CppgmType* parameters, uint32_t count, uint32_t variadic);
	bool function(uint32_t symbol, uint32_t signature, uint32_t binding, uint32_t definition);
	bool global(uint32_t symbol, CppgmType type, uint32_t binding, uint32_t storage, const CppgmDataItem* items,
		uint32_t count, uint32_t declaration);
	bool body(uint32_t symbol, const CppgmBody& input);
	bool reserve_body(uint32_t symbol, const CppgmBodyCounts& minimum, CppgmBodyBuffers& buffers);
	bool commit_body(uint32_t symbol, const CppgmBodyCounts& counts, uint32_t temp_count);
	bool validate();
	bool write(const char* path);
	bool compile(uint32_t level, uint32_t pic);
};

namespace backend {
namespace c_api {

lowir::Type type(CppgmType bits);
bool valid_type(CppgmType bits);
bool direct_layout();
bool valid_value(CppgmValue input);
lowir::Value value(CppgmValue input);

} // namespace c_api
} // namespace backend
