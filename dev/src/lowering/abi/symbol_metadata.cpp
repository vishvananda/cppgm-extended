#include "lowering/abi/symbol_metadata.h"

#include "lowering/ir/model.h"

#include <string>

namespace cppgm
{
namespace lowering
{
namespace abi
{

void ApplyBuiltinSymbolMetadata(lowering::ir::Symbol* symbol,
	semantic::BuiltinFunctionKind kind,
	hosted_builtin::MemoryIntrinsicKind memory_kind)
{
	using namespace semantic;
	using lowering::ir::Symbol;
	if (kind == BUILTIN_FUNCTION_HOSTED_MEMORY_INTRINSIC)
	{
		switch (hosted_builtin::GetMemoryIntrinsic(memory_kind).effect)
		{
		case hosted_builtin::MEMORY_EFFECT_READNONE:
			symbol->effects = Symbol::EFFECTS_READNONE; break;
		case hosted_builtin::MEMORY_EFFECT_READONLY:
			symbol->effects = Symbol::EFFECTS_READONLY; break;
		case hosted_builtin::MEMORY_EFFECT_READWRITE:
			symbol->effects = Symbol::EFFECTS_READWRITE; break;
		}
		return;
	}
	switch (kind)
	{
	case BUILTIN_FUNCTION_STRLEN: symbol->effects = Symbol::EFFECTS_READONLY; break;
	case BUILTIN_FUNCTION_UNREACHABLE: break;
	case BUILTIN_FUNCTION_MEMCPY:
	case BUILTIN_FUNCTION_MEMMOVE: symbol->effects = Symbol::EFFECTS_READWRITE; break;
	case BUILTIN_FUNCTION_NANL:
	case BUILTIN_FUNCTION_ISNAN:
	case BUILTIN_FUNCTION_HOSTED_INTEGER_INTRINSIC:
	case BUILTIN_FUNCTION_HOSTED_FLOATING_INTRINSIC:
		symbol->effects = Symbol::EFFECTS_READNONE; break;
	case BUILTIN_FUNCTION_HOSTED_MEMORY_INTRINSIC: break;
	case BUILTIN_FUNCTION_IA32_EMMS:
		symbol->effects = Symbol::EFFECTS_READWRITE; break;
	case BUILTIN_FUNCTION_ABORT:
		symbol->runtime_role = Symbol::RUNTIME_ROLE_TERMINATE;
		symbol->noreturn = true; break;
	case BUILTIN_FUNCTION_ALLOCA:
	case BUILTIN_FUNCTION_VSNPRINTF:
	case BUILTIN_FUNCTION_VA_START:
	case BUILTIN_FUNCTION_VA_END:
	case BUILTIN_FUNCTION_VA_ARG:
		symbol->effects = Symbol::EFFECTS_READWRITE; break;
	case BUILTIN_FUNCTION_OPERATOR_NEW:
	case BUILTIN_FUNCTION_OPERATOR_NEW_ARRAY:
		symbol->runtime_role = Symbol::RUNTIME_ROLE_ALLOCATE_MEMORY;
		symbol->effects = Symbol::EFFECTS_READWRITE; break;
	case BUILTIN_FUNCTION_OPERATOR_DELETE:
	case BUILTIN_FUNCTION_OPERATOR_DELETE_ARRAY:
		symbol->runtime_role = Symbol::RUNTIME_ROLE_FREE_MEMORY;
		symbol->effects = Symbol::EFFECTS_READWRITE; break;
	case BUILTIN_FUNCTION_NONE: break;
	}
}

void ApplyNativeRuntimeSymbolMetadata(
	const lowering::ir::Program& program,
	lowering::ir::Symbol* symbol)
{
	using lowering::ir::Symbol;
	if (!symbol->c_linkage || !symbol->object_name.valid()) return;
	const std::string& object_name = program.strings.get(symbol->object_name);
	if (object_name == "malloc")
		symbol->runtime_role = Symbol::RUNTIME_ROLE_ALLOCATE_MEMORY;
	else if (object_name == "free")
		symbol->runtime_role = Symbol::RUNTIME_ROLE_FREE_MEMORY;
}

void ApplyBuiltinParameterAliasMetadata(lowering::ir::Parameter* parameter,
	semantic::BuiltinFunctionKind kind,
	hosted_builtin::MemoryIntrinsicKind memory_kind, std::size_t index)
{
	using namespace semantic;
	using lowering::ir::Parameter;
	if (kind == BUILTIN_FUNCTION_HOSTED_MEMORY_INTRINSIC)
	{
		if (memory_kind == hosted_builtin::MEMORY_INTRINSIC_MEMCPY && index < 2)
			parameter->alias = Parameter::ALIAS_NOALIAS;
		return;
	}
	if (kind == BUILTIN_FUNCTION_MEMCPY && index < 2)
	{
		parameter->alias = Parameter::ALIAS_NOALIAS;
	}
}

std::uint8_t SysvObjectClasses(const semantic::Program& program,
	semantic::TypeId type, bool result)
{
	using namespace semantic;
	if (type == kNoType) return 0;
	const TypeRecord& record = program.types.Get(program.types.RemoveTopCv(type));
	if (record.kind != TYPE_NAMED || record.entity >= program.entities.size())
		return 0;
	const EntityRecord& entity = program.entities[record.entity];
	if (!IsClassNamedFlavor(entity.flavor) || !entity.layout_complete ||
		entity.object_size == 0 || entity.object_size > 16)
		return 0;
	// lowir_model::kObjectClassesMemory.
	if (entity.sysv_memory || entity.sysv_unmodeled) return result ? 0 : 0x80;
	std::uint8_t classes = 0;
	bool stated = false;
	for (unsigned eightbyte = 0; eightbyte < (entity.object_size + 7) / 8;
		++eightbyte)
	{
		const unsigned low = (entity.sysv_quarters >> (eightbyte * 4)) & 3;
		const unsigned high = (entity.sysv_quarters >> (eightbyte * 4 + 2)) & 3;
		// 1 none, 2 integer, 3 sse; INTEGER wins a merge.
		const unsigned code = low == 1 || high == 1 ? 2 :
			low == 2 || high == 2 ? 3 : 1;
		stated = stated || code != 2;
		classes = static_cast<std::uint8_t>(classes | (code << (eightbyte * 2)));
	}
	return stated ? classes : 0;
}

}  // namespace abi
}  // namespace lowering
}  // namespace cppgm
