#include "src/lowir_interface.h"

// Optional PA27/PA33 library exercise. Replace these failing entry points
// with a facade over the same typed model used by the command-line tools.
extern "C" {

int cppgm_lowir_body_reserve(CppgmLowir*, uint32_t, const CppgmBodyCounts*, CppgmBodyBuffers*)
{
	return 0;
}

int cppgm_lowir_body_commit(CppgmLowir*, uint32_t, const CppgmBodyCounts*, uint32_t)
{
	return 0;
}

CppgmLowir* cppgm_lowir_create(uint32_t, uint32_t)
{
	return nullptr;
}

void cppgm_lowir_destroy(CppgmLowir*)
{}

const char* cppgm_lowir_error(const CppgmLowir*)
{
	return "optional LowIR library is not implemented";
}

uint32_t cppgm_lowir_symbol(CppgmLowir*, const char*, uint32_t)
{
	return CPPGM_NO_ID;
}

uint32_t cppgm_lowir_signature(CppgmLowir*, CppgmType, const CppgmType*, uint32_t, uint32_t)
{
	return CPPGM_NO_ID;
}

int cppgm_lowir_function(CppgmLowir*, uint32_t, uint32_t, uint32_t, uint32_t)
{
	return 0;
}

int cppgm_lowir_global(CppgmLowir*, uint32_t, CppgmType, uint32_t, uint32_t, const CppgmDataItem*, uint32_t, uint32_t)
{
	return 0;
}

int cppgm_lowir_validate(CppgmLowir*)
{
	return 0;
}

int cppgm_lowir_write(CppgmLowir*, const char*)
{
	return 0;
}

int cppgm_lowir_compile(CppgmLowir*, uint32_t, uint32_t)
{
	return 0;
}

const uint8_t* cppgm_lowir_object_data(const CppgmLowir*)
{
	return nullptr;
}

uint64_t cppgm_lowir_object_size(const CppgmLowir*)
{
	return 0;
}

} // extern "C"
