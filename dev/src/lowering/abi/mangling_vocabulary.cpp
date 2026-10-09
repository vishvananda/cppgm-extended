#include "lowering/abi/mangling_vocabulary.h"

#include "lowering/support/errors.h"

#include <string>
#include <vector>

namespace cppgm
{
namespace lowering
{
namespace abi
{

using namespace semantic;

namespace
{

NameId StandardTemplateTerminal(const semantic::Program& program,
	const EntityRecord& entity)
{
	std::vector<NameId> path;
	program.BuildEmissionPath(entity.owner, entity.identity_name, &path);
	return path.size() == 2 && program.IsStandardNamespace(entity.owner) ?
		path[1] : 0;
}

bool IsFundamentalType(const semantic::Program& program, TypeId type,
	FundamentalKind fundamental)
{
	type = program.types.RemoveTopCv(type);
	const TypeRecord& record = program.types.Get(type);
	return record.kind == TYPE_FUNDAMENTAL &&
		record.fundamental == fundamental;
}

bool IsStandardUnaryCharTemplate(const semantic::Program& program, TypeId type,
	const char* terminal)
{
	type = program.types.RemoveTopCv(type);
	const TypeRecord& record = program.types.Get(type);
	if (record.kind != TYPE_NAMED || record.entity >= program.entities.size())
		return false;
	const EntityRecord& entity = program.entities[record.entity];
	const NameId name = StandardTemplateTerminal(program, entity);
	if (name == 0 || program.names.Get(name) != terminal ||
		entity.template_argument_count != 1 ||
		entity.template_argument_begin >= program.template_arguments.size())
		return false;
	return IsFundamentalType(program,
		program.template_arguments[entity.template_argument_begin], FUND_CHAR);
}

bool HasExactStandardCharacterArguments(const semantic::Program& program,
	const EntityRecord& entity, bool allocator)
{
	const std::size_t expected = allocator ? 3 : 2;
	const std::size_t first = entity.template_argument_begin;
	if (entity.template_argument_count != expected ||
		first > program.template_arguments.size() ||
		expected > program.template_arguments.size() - first)
		return false;
	return IsFundamentalType(program, program.template_arguments[first],
			FUND_CHAR) &&
		IsStandardUnaryCharTemplate(program,
			program.template_arguments[first + 1], "char_traits") &&
		(!allocator || IsStandardUnaryCharTemplate(program,
			program.template_arguments[first + 2], "allocator"));
}

}  // namespace

StandardTemplateSubstitution StandardSubstitutionFor(
	const semantic::Program& program, const EntityRecord& entity)
{
	const NameId terminal_id = StandardTemplateTerminal(program, entity);
	if (terminal_id == 0) return StandardTemplateSubstitution();
	const std::string& terminal = program.names.Get(terminal_id);
	if (terminal == "allocator")
		return StandardTemplateSubstitution(
			abi_mangle::ABI_STANDARD_SUBSTITUTION_ALLOCATOR, false);
	if (terminal == "basic_string")
		return HasExactStandardCharacterArguments(program, entity, true) ?
			StandardTemplateSubstitution(
				abi_mangle::ABI_STANDARD_SUBSTITUTION_STRING, true) :
			StandardTemplateSubstitution(
				abi_mangle::ABI_STANDARD_SUBSTITUTION_BASIC_STRING, false);
	if (terminal == "basic_istream" &&
		HasExactStandardCharacterArguments(program, entity, false))
		return StandardTemplateSubstitution(
			abi_mangle::ABI_STANDARD_SUBSTITUTION_ISTREAM, true);
	if (terminal == "basic_ostream" &&
		HasExactStandardCharacterArguments(program, entity, false))
		return StandardTemplateSubstitution(
			abi_mangle::ABI_STANDARD_SUBSTITUTION_OSTREAM, true);
	if (terminal == "basic_iostream" &&
		HasExactStandardCharacterArguments(program, entity, false))
		return StandardTemplateSubstitution(
			abi_mangle::ABI_STANDARD_SUBSTITUTION_IOSTREAM, true);
	return StandardTemplateSubstitution();
}

bool MakeBuiltinAbiType(const semantic::Program& program, const TypeRecord& source,
	abi_mangle::AbiType* result)
{
	using namespace abi_mangle;
	if (source.kind == TYPE_BITINT)
	{
		if (source.dependent_bound_parameter != kNoTemplateParameter ||
			source.bound == 0)
			ThrowLoweringInternal("dependent _BitInt has no ABI encoding");
		result->kind = ABI_TYPE_BUILTIN;
		result->builtin_type = source.bitint_unsigned ?
			ABI_BUILTIN_TYPE_UNSIGNED_BITINT : ABI_BUILTIN_TYPE_BITINT;
		result->index = source.bound;
		return true;
	}
	if (source.kind == TYPE_COMPLEX)
	{
		const TypeRecord& element = program.types.Get(source.child);
		if (element.kind != TYPE_FUNDAMENTAL)
			ThrowLoweringInternal("complex ABI element is not fundamental");
		result->kind = ABI_TYPE_BUILTIN;
		switch (element.fundamental)
		{
		case FUND_FLOAT:
			result->builtin_type = ABI_BUILTIN_TYPE_COMPLEX_FLOAT;
			return true;
		case FUND_DOUBLE:
			result->builtin_type = ABI_BUILTIN_TYPE_COMPLEX_DOUBLE;
			return true;
		case FUND_LONG_DOUBLE:
			result->builtin_type = ABI_BUILTIN_TYPE_COMPLEX_LONG_DOUBLE;
			return true;
		default:
			ThrowLoweringSource("unsupported complex ABI element type");
		}
	}
	if (source.kind != TYPE_FUNDAMENTAL) return false;
	static const AbiBuiltinTypeKind fundamental_types[] = {
		ABI_BUILTIN_TYPE_BOOL, ABI_BUILTIN_TYPE_CHAR,
		ABI_BUILTIN_TYPE_SIGNED_CHAR, ABI_BUILTIN_TYPE_UNSIGNED_CHAR,
		ABI_BUILTIN_TYPE_SHORT, ABI_BUILTIN_TYPE_UNSIGNED_SHORT,
		ABI_BUILTIN_TYPE_INT, ABI_BUILTIN_TYPE_UNSIGNED_INT,
		ABI_BUILTIN_TYPE_LONG, ABI_BUILTIN_TYPE_UNSIGNED_LONG,
		ABI_BUILTIN_TYPE_LONG_LONG, ABI_BUILTIN_TYPE_UNSIGNED_LONG_LONG,
		ABI_BUILTIN_TYPE_FLOAT, ABI_BUILTIN_TYPE_DOUBLE,
		ABI_BUILTIN_TYPE_LONG_DOUBLE, ABI_BUILTIN_TYPE_VOID,
		ABI_BUILTIN_TYPE_NULLPTR, ABI_BUILTIN_TYPE_WCHAR,
		ABI_BUILTIN_TYPE_CHAR16, ABI_BUILTIN_TYPE_CHAR32,
		ABI_BUILTIN_TYPE_INT128, ABI_BUILTIN_TYPE_UINT128,
		ABI_BUILTIN_TYPE_FLOAT16, ABI_BUILTIN_TYPE_FLOAT32,
		ABI_BUILTIN_TYPE_FLOAT32X, ABI_BUILTIN_TYPE_FLOAT64,
		ABI_BUILTIN_TYPE_FLOAT64X, ABI_BUILTIN_TYPE_STDFLOAT128,
		ABI_BUILTIN_TYPE_FLOAT128
	};
	static_assert(sizeof(fundamental_types) / sizeof(fundamental_types[0]) ==
		FUND_FLOAT128 + 1, "fundamental ABI type table is incomplete");
	result->kind = ABI_TYPE_BUILTIN;
	result->builtin_type = fundamental_types[source.fundamental];
	return true;
}

abi_mangle::AbiTerminalKind OperatorTerminal(OperatorKind kind, bool member,
	std::size_t parameter_count)
{
	using namespace abi_mangle;
	switch (kind)
	{
	case OPERATOR_PLUS: return ABI_TERMINAL_PLUS;
	case OPERATOR_MINUS: return ABI_TERMINAL_MINUS;
	case OPERATOR_STAR:
		return (member ? parameter_count == 0 : parameter_count == 1) ?
			ABI_TERMINAL_DEREFERENCE : ABI_TERMINAL_MULTIPLY;
	case OPERATOR_AMPERSAND:
		return (member ? parameter_count == 0 : parameter_count == 1) ?
			ABI_TERMINAL_ADDRESS_OF : ABI_TERMINAL_BIT_AND;
	case OPERATOR_DIVIDE: return ABI_TERMINAL_DIVIDE;
	case OPERATOR_REMAINDER: return ABI_TERMINAL_REMAINDER;
	case OPERATOR_BIT_OR: return ABI_TERMINAL_BIT_OR;
	case OPERATOR_BIT_XOR: return ABI_TERMINAL_BIT_XOR;
	case OPERATOR_ASSIGN: return ABI_TERMINAL_ASSIGN;
	case OPERATOR_PLUS_ASSIGN: return ABI_TERMINAL_PLUS_ASSIGN;
	case OPERATOR_MINUS_ASSIGN: return ABI_TERMINAL_MINUS_ASSIGN;
	case OPERATOR_MULTIPLY_ASSIGN: return ABI_TERMINAL_MULTIPLY_ASSIGN;
	case OPERATOR_DIVIDE_ASSIGN: return ABI_TERMINAL_DIVIDE_ASSIGN;
	case OPERATOR_REMAINDER_ASSIGN: return ABI_TERMINAL_REMAINDER_ASSIGN;
	case OPERATOR_AND_ASSIGN: return ABI_TERMINAL_AND_ASSIGN;
	case OPERATOR_OR_ASSIGN: return ABI_TERMINAL_OR_ASSIGN;
	case OPERATOR_XOR_ASSIGN: return ABI_TERMINAL_XOR_ASSIGN;
	case OPERATOR_LEFT_SHIFT: return ABI_TERMINAL_LEFT_SHIFT;
	case OPERATOR_RIGHT_SHIFT: return ABI_TERMINAL_RIGHT_SHIFT;
	case OPERATOR_LEFT_SHIFT_ASSIGN: return ABI_TERMINAL_LEFT_SHIFT_ASSIGN;
	case OPERATOR_RIGHT_SHIFT_ASSIGN: return ABI_TERMINAL_RIGHT_SHIFT_ASSIGN;
	case OPERATOR_EQUAL: return ABI_TERMINAL_EQUAL;
	case OPERATOR_NOT_EQUAL: return ABI_TERMINAL_NOT_EQUAL;
	case OPERATOR_LESS: return ABI_TERMINAL_LESS;
	case OPERATOR_GREATER: return ABI_TERMINAL_GREATER;
	case OPERATOR_LESS_EQUAL: return ABI_TERMINAL_LESS_EQUAL;
	case OPERATOR_GREATER_EQUAL: return ABI_TERMINAL_GREATER_EQUAL;
	case OPERATOR_LOGICAL_NOT: return ABI_TERMINAL_LOGICAL_NOT;
	case OPERATOR_COMPLEMENT: return ABI_TERMINAL_COMPLEMENT;
	case OPERATOR_LOGICAL_AND: return ABI_TERMINAL_LOGICAL_AND;
	case OPERATOR_LOGICAL_OR: return ABI_TERMINAL_LOGICAL_OR;
	case OPERATOR_INCREMENT: return ABI_TERMINAL_INCREMENT;
	case OPERATOR_DECREMENT: return ABI_TERMINAL_DECREMENT;
	case OPERATOR_COMMA: return ABI_TERMINAL_COMMA;
	case OPERATOR_MEMBER_POINTER: return ABI_TERMINAL_MEMBER_POINTER;
	case OPERATOR_ARROW: return ABI_TERMINAL_ARROW;
	case OPERATOR_CALL: return ABI_TERMINAL_CALL;
	case OPERATOR_INDEX: return ABI_TERMINAL_INDEX;
	case OPERATOR_NEW: return ABI_TERMINAL_NEW;
	case OPERATOR_NEW_ARRAY: return ABI_TERMINAL_NEW_ARRAY;
	case OPERATOR_DELETE: return ABI_TERMINAL_DELETE;
	case OPERATOR_DELETE_ARRAY: return ABI_TERMINAL_DELETE_ARRAY;
	case OPERATOR_NONE:
	case OPERATOR_LITERAL: return ABI_TERMINAL_NONE;
	}
	ThrowLoweringInternal("invalid typed operator terminal");
}

}  // namespace abi
}  // namespace lowering
}  // namespace cppgm
