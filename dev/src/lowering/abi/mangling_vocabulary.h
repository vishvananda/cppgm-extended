#pragma once

// Fixed Itanium vocabulary the lowering's ABI fact builder maps semantic
// facts onto: builtin type codes, the standard-library substitutions and
// operator terminals.

#include "abi/itanium/abi_mangle.h"
#include "semantic/model/program.h"

#include <cstddef>

namespace cppgm
{
namespace lowering
{
namespace abi
{

struct StandardTemplateSubstitution
{
	abi_mangle::AbiStandardSubstitutionKind code;
	bool includes_arguments;

	StandardTemplateSubstitution(
		abi_mangle::AbiStandardSubstitutionKind code_value =
			abi_mangle::ABI_STANDARD_SUBSTITUTION_TEXT,
		bool includes_arguments_value = false)
		: code(code_value), includes_arguments(includes_arguments_value) {}
};

// The standard substitution (`Sa`, `Ss`, `Si`, ...) a std:: class template
// specialization takes, if any.
StandardTemplateSubstitution StandardSubstitutionFor(
	const semantic::Program& program, const semantic::EntityRecord& entity);
bool MakeBuiltinAbiType(const semantic::Program& program,
	const semantic::TypeRecord& source, abi_mangle::AbiType* result);
abi_mangle::AbiTerminalKind OperatorTerminal(semantic::OperatorKind kind,
	bool member, std::size_t parameter_count);
// The <operator-name> of an operator applied in a dependent expression
// (`nt`, `aa`, `le`), or null where the operator has none.
const char* ExpressionOperatorCode(semantic::OperatorKind kind, bool unary);

}  // namespace abi
}  // namespace lowering
}  // namespace cppgm
