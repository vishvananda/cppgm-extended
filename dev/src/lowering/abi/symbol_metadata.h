#pragma once

#include "lowering/ir/types.h"
#include "semantic/model/program.h"

#include <cstddef>

namespace cppgm
{
namespace lowering
{
namespace ir
{
struct Program;
}
namespace abi
{

void ApplyBuiltinSymbolMetadata(lowering::ir::Symbol* symbol,
	semantic::BuiltinFunctionKind kind,
	hosted_builtin::MemoryIntrinsicKind memory_kind);
void ApplyNativeRuntimeSymbolMetadata(const lowering::ir::Program& program,
	lowering::ir::Symbol* symbol);
void ApplyBuiltinParameterAliasMetadata(lowering::ir::Parameter* parameter,
	semantic::BuiltinFunctionKind kind,
	hosted_builtin::MemoryIntrinsicKind memory_kind, std::size_t index);

// The SysV classes (lowir.md "Object ABI Conventions") of a class
// object passed (or, for `result`, returned) directly, as lowir_model::
// ParameterMetadata::classes encodes them: stated only when an eightbyte is
// not INTEGER (an SSE or empty one) or a parameter goes in memory; 0
// otherwise, and for a type that is not a complete class of at most 16
// bytes.  A result with an x87 or 16-byte vector member states nothing.
std::uint8_t SysvObjectClasses(const semantic::Program& program,
	semantic::TypeId type, bool result = false);

}  // namespace abi
}  // namespace lowering
}  // namespace cppgm
