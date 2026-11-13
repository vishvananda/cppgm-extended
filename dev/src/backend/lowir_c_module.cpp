#include "backend/lowir_c_module.h"
#include "backend/lowir_c_vocabulary.h"

#include <algorithm>
#include <cstring>
#include <mutex>

#include "lowir/validator.h"
#include "lowir/text_writer.h"
#include "native/object/object_compiler.h"
#include "native/object/elf_object.h"
#include "native/object/macho_object.h"
#include "opt/optimizer.h"

namespace backend {
namespace c_api {

lowir::Type type(CppgmType bits)
{
	lowir::Type result;
	result.kind_bits = bits & 255;
	result.align_log2 = (bits >> 8) & 255;
	result.bytes = bits >> 16;
	return result;
}

bool valid_type(CppgmType bits)
{
	const lowir::Type t = type(bits);
	if (t.kind_bits >= lowir::kTypeKindCount)
		return false;
	if (t.is_object())
		return t.bytes != 0 && t.align_log2 <= 31;
	if (t.is_pointer())
		return t.align_log2 == 0 && (t.bytes == 0 || t.bytes == 8);
	return t.align_log2 == 0 && t.bytes == 0;
}

bool valid_value(CppgmValue input)
{
	const unsigned kind = input.kind & 255;
	const unsigned floating = input.kind >> 8;
	return kind < lowir::kValueKindCount &&
		(kind == CPPGM_VALUE_FLOAT ? floating >= CPPGM_TYPE_F32 && floating <= CPPGM_TYPE_F80 : floating == 0);
}

lowir::Value value(CppgmValue input)
{
	lowir::Value result = lowir::Value::make(static_cast<lowir::ValueKind>(input.kind & 255), input.index);
	result.float_kind = static_cast<lowir::TypeKind>((input.kind >> 8) & 255);
	return result;
}

} // namespace c_api
} // namespace backend

using namespace backend::c_api;

uint32_t CppgmLowir::symbol(const char* name, uint32_t length)
{
	if (!ready())
		return CPPGM_NO_ID;
	if (name == nullptr || length == 0 || std::memchr(name, 0, length) != nullptr)
	{
		fail("invalid symbol name");
		return CPPGM_NO_ID;
	}
	return program.symbol_for(program.names.intern(name, length));
}

uint32_t CppgmLowir::signature(CppgmType result, const CppgmType* parameters, uint32_t count, uint32_t variadic)
{
	if (!ready())
		return CPPGM_NO_ID;
	if (!valid_type(result) || (count != 0 && parameters == nullptr) || variadic > 1)
	{
		fail("invalid signature");
		return CPPGM_NO_ID;
	}
	lowir::Signature signature = {};
	signature.result = type(result);
	signature.first_parameter = program.parameters.size();
	signature.parameter_count = count;
	signature.boundary.arity = variadic ? ir_model::CAM_VARIADIC : ir_model::CAM_FIXED;
	for (uint32_t i = 0; i < count; ++i)
	{
		if (!valid_type(parameters[i]) || type(parameters[i]).is_void())
		{
			fail("invalid parameter type");
			return CPPGM_NO_ID;
		}
		lowir::Parameter parameter;
		parameter.name = 0;
		parameter.type = type(parameters[i]);
		program.parameters.push_back(parameter);
	}
	return program.add_signature(signature);
}

bool CppgmLowir::function(uint32_t id, uint32_t signature, uint32_t binding, uint32_t definition)
{
	if (!ready())
		return false;
	if (id >= program.symbols.size() || signature >= program.signatures.size() || binding > 3 || definition > 1)
		return fail("invalid function declaration");
	lowir::Symbol& symbol = program.symbols[id];
	if (symbol.kind == lowir::SymbolKind::Unresolved)
		program.declare_function(
			id, definition ? lowir::SymbolKind::FunctionDefinition : lowir::SymbolKind::FunctionDeclaration, signature);
	else
	{
		if (symbol.kind != lowir::SymbolKind::FunctionDeclaration || !definition)
			return fail("duplicate function declaration or definition");
		lowir::Function& function = program.functions[symbol.detail];
		if (function.signature != signature)
			return fail("function signature differs from declaration");
		symbol.kind = lowir::SymbolKind::FunctionDefinition;
		function.body.reset(new lowir::FunctionBody());
	}
	symbol.metadata.linkage = ir_model::LLM_C;
	symbol.metadata.binding = static_cast<ir_model::SymbolBindingMode>(binding);
	return true;
}

bool CppgmLowir::global(uint32_t id, CppgmType bits, uint32_t binding, uint32_t storage, const CppgmDataItem* items,
	uint32_t count, uint32_t declaration)
{
	if (!ready())
		return false;
	if (id >= program.symbols.size() || program.symbols[id].kind != lowir::SymbolKind::Unresolved ||
		!valid_type(bits) || binding > 3 || storage > 2 || declaration > 1 || (count != 0 && items == nullptr) ||
		(declaration && (count != 0 || type(bits).is_void())))
		return fail("invalid global declaration");
	lowir::Global global = {};
	global.typed = !type(bits).is_void();
	global.type = type(bits);
	global.init = declaration ? lowir::GlobalInit::None : lowir::GlobalInit::Data;
	global.first_item = program.data_items.size();
	global.item_count = count;
	for (uint32_t i = 0; i < count; ++i)
	{
		const CppgmDataItem& input = items[i];
		if (input.kind > 2 || !valid_type(input.type) || (input.kind == 1 && input.symbol >= program.symbols.size()) ||
			(input.kind == 2 && input.amount < 0))
			return fail("invalid global data item");
		lowir::DataItem item = {};
		item.kind = static_cast<lowir::DataItemKind>(input.kind);
		item.type = type(input.type);
		item.constant.kind = item.type.is_float() ? lowir::ValueKind::Float : lowir::ValueKind::Integer;
		item.constant.float_kind = item.type.is_float() ? item.type.kind() : lowir::TypeKind::Void;
		item.constant.bits = lowir::Literal{ input.bits.low, input.bits.high };
		item.address = input.symbol;
		item.amount = input.amount;
		program.data_items.push_back(item);
	}
	program.declare_global(
		id, declaration ? lowir::SymbolKind::GlobalDeclaration : lowir::SymbolKind::GlobalDefinition, global);
	program.symbols[id].metadata.binding = static_cast<ir_model::SymbolBindingMode>(binding);
	program.symbols[id].metadata.storage = static_cast<ir_model::GlobalStorageMode>(storage);
	return true;
}

bool CppgmLowir::validate()
{
	if (error[0] != 0)
		return false;
	if (consumed)
		return fail("module has already been consumed");
	if (validated)
		return true;
	if (std::find(direct_bodies.begin(), direct_bodies.end(), 1) != direct_bodies.end())
		return fail("uncommitted direct function body");
	lowir::apply_legacy_roles(program);
	lowir::Diagnostic diagnostic;
	if (!lowir::validate_constructed_program(program, diagnostic, false))
		return fail((lowir::describe(program, diagnostic) +
			(diagnostic.inst == lowir::kNoId ? std::string()
											 : " (instruction " + std::to_string(diagnostic.inst) + ")"))
				.c_str());
	validated = true;
	return true;
}

bool CppgmLowir::write(const char* path)
{
	if (path == nullptr || !validate())
		return path == nullptr ? fail("null output path") : false;
	if (!lowir::write_program_file(program, path))
		return fail("cannot write LowIR output");
	return true;
}

bool CppgmLowir::compile(uint32_t level, uint32_t pic)
{
	if (level > 3 || pic > 1)
		return fail("invalid compile options");
	if (consumed)
		return fail("module has already been consumed");
	if (!validate())
		return false;
	consumed = true;
	// The executable backend retains process-wide scratch capacities across
	// functions. Construction and validation use module-owned state; only the
	// optimizing/emitting phase must be serialized until those pools have an
	// explicit per-compilation owner. One lock per object, never per IR item.
	static std::mutex backend_mutex;
	const std::lock_guard<std::mutex> lock(backend_mutex);
	opt::OptimizerOptions options;
	options.level = level;
	options.has_gnu_sincos = target_model::has_gnu_sincos(program.target);
	native::recover_jump_thunks(program, native::EhModel::Host);
	if (level != 0)
	{
		opt::optimize_program(program, options, nullptr);
		program.jump_thunks.clear();
		native::recover_jump_thunks(program, native::EhModel::Host);
	}
	native::object::ObjectFile result;
	std::string message;
	if (!native::compile_object(
			program, result, message, nullptr, native::EhModel::Host, level, pic != 0, program.target))
		return fail(message.c_str());
	if (program.target == target_model::Target::MacOS)
	{
		if (!native::object::encode_macho_object(result, object, message))
			return fail(message.c_str());
	}
	else
		native::object::encode_elf_object(result, object);
	return true;
}
