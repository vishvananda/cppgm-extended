#include "src/backend/lowir_c_module.h"

#include <exception>
#include <memory>

#include "src/support/internal_error.h"

namespace {

// Last-resort foreign boundary. Reporting uses a fixed buffer so bad_alloc
// itself can be returned without allocating. Compiler errors never cross C.
template<class Action>
uint32_t invoke(CppgmLowir* module, uint32_t failure, Action action)
{
	if (module == nullptr || module->error[0] != 0)
		return failure;
	try
	{
		return action();
	}
	catch (const InternalError& error)
	{
		module->fail(error.what());
	}
	catch (const std::exception& error)
	{
		module->fail(error.what());
	}
	return failure;
}

} // namespace

extern "C" {

CppgmLowir* cppgm_lowir_create(uint32_t version, uint32_t target)
{
	if (version != CPPGM_LOWIR_ABI_VERSION || target > 1 || !backend::c_api::direct_layout())
		return nullptr;
	try
	{
		std::unique_ptr<CppgmLowir> module(new CppgmLowir());
		module->program.target = target == 0 ? target_model::Target::Linux : target_model::Target::MacOS;
		module->program.explicit_target = true;
		module->program.object_facts = true;
		return module.release();
	}
	catch (const InternalError&)
	{
		return nullptr;
	}
	catch (const std::exception&)
	{
		return nullptr;
	}
}

void cppgm_lowir_destroy(CppgmLowir* module)
{
	delete module;
}

const char* cppgm_lowir_error(const CppgmLowir* module)
{
	return module == nullptr ? "invalid ABI version, target, or allocation failure" : module->error;
}

uint32_t cppgm_lowir_symbol(CppgmLowir* module, const char* name, uint32_t length)
{
	return invoke(module, CPPGM_NO_ID, [&]() { return module->symbol(name, length); });
}

uint32_t cppgm_lowir_signature(
	CppgmLowir* module, CppgmType result, const CppgmType* parameters, uint32_t count, uint32_t variadic)
{
	return invoke(module, CPPGM_NO_ID, [&]() { return module->signature(result, parameters, count, variadic); });
}

int cppgm_lowir_function(CppgmLowir* module, uint32_t symbol, uint32_t signature, uint32_t binding, uint32_t definition)
{
	return invoke(module, 0, [&]() { return module->function(symbol, signature, binding, definition); });
}

int cppgm_lowir_global(CppgmLowir* module, uint32_t symbol, CppgmType type, uint32_t binding, uint32_t storage,
	const CppgmDataItem* items, uint32_t count, uint32_t declaration)
{
	return invoke(
		module, 0, [&]() { return module->global(symbol, type, binding, storage, items, count, declaration); });
}

int cppgm_lowir_body(CppgmLowir* module, uint32_t symbol, const CppgmBody* body)
{
	return invoke(
		module, 0, [&]() { return body != nullptr ? module->body(symbol, *body) : module->fail("null body"); });
}

int cppgm_lowir_body_reserve(
	CppgmLowir* module, uint32_t symbol, const CppgmBodyCounts* minimum, CppgmBodyBuffers* buffers)
{
	return invoke(module, 0, [&]() {
		return minimum != nullptr && buffers != nullptr ? module->reserve_body(symbol, *minimum, *buffers)
														: module->fail("null body buffer request");
	});
}

int cppgm_lowir_body_commit(CppgmLowir* module, uint32_t symbol, const CppgmBodyCounts* counts, uint32_t temp_count)
{
	return invoke(module, 0, [&]() {
		return counts != nullptr ? module->commit_body(symbol, *counts, temp_count) : module->fail("null body counts");
	});
}

int cppgm_lowir_validate(CppgmLowir* module)
{
	return invoke(module, 0, [&]() { return module->validate(); });
}

int cppgm_lowir_write(CppgmLowir* module, const char* path)
{
	return invoke(module, 0, [&]() { return module->write(path); });
}

int cppgm_lowir_compile(CppgmLowir* module, uint32_t level, uint32_t pic)
{
	return invoke(module, 0, [&]() { return module->compile(level, pic); });
}

const uint8_t* cppgm_lowir_object_data(const CppgmLowir* module)
{
	return module == nullptr || module->error[0] != 0 ? nullptr : module->object.data();
}

uint64_t cppgm_lowir_object_size(const CppgmLowir* module)
{
	return module == nullptr || module->error[0] != 0 ? 0 : module->object.size();
}

} // extern "C"
