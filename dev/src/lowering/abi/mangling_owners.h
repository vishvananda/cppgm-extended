#pragma once

// The owners an entity's ABI name is built from (specialized, nested and
// tagged owners; an internal variable's marker) and the typed fact-record
// helpers the lowering's ABI fact builder appends with.

#include "abi/itanium/abi_mangle.h"
#include "semantic/model/graph.h"
#include "semantic/model/program.h"

namespace cppgm
{
namespace lowering
{
namespace abi
{

void AppendTypedFact(abi_mangle::AbiTypedCase* facts,
	abi_mangle::AbiFactRecord* record);
void AppendFunctionAbiTagFacts(const semantic::Program& program,
	const semantic::BindingRecord& binding, abi_mangle::AbiMangleContext* context,
	abi_mangle::AbiTypedCase* facts);
void AppendComponentAbiTagFacts(const semantic::Program& program,
	const semantic::EntityRecord& entity, abi_mangle::AbiMangleContext* context,
	abi_mangle::AbiTypedCase* facts);
bool IsClassTemplateSpecialization(const semantic::EntityRecord& entity);
// The base class constructor an inherited constructor's name encodes.
const semantic::BindingRecord* InheritedConstructorAbiSource(
	const semantic::Program& program, const semantic::BindingRecord& binding,
	const semantic::DumpNode& node);
const semantic::FunctionTemplateAbiRecipe* FunctionAbiRecipe(
	const semantic::Program& program, const semantic::BindingRecord& binding);
bool ClassOwnerHasAbiTags(const semantic::Program& program,
	semantic::EntityId entity);
bool ClassOwnerHasSpecializedAncestor(const semantic::Program& program,
	semantic::EntityId entity);
bool ClassOwnerIsNested(const semantic::Program& program,
	semantic::EntityId entity);
// A namespace-scope variable of internal linkage carries the `L` marker
// (clang's isInternalLinkageDecl): static, or const and not extern; one of
// an unnamed namespace has none, its `_GLOBAL__N_1` prefix already being
// unit-local.  An anonymous union's object is named by its first member
// and takes no marker either.
bool VariableHasInternalMarker(const semantic::Program& program,
	const semantic::BindingRecord& binding);
semantic::NameId VariableAbiName(const semantic::BindingRecord& binding);

}  // namespace abi
}  // namespace lowering
}  // namespace cppgm
