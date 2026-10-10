#include "lower/program_lowering.h"

#include <algorithm>
#include <cstdio>

#include "lex/identifier_table.h"
#include "sema/analyzer.h"
#include "sema/conversions.h"
#include "sema/integral_arithmetic.h"
#include "sema/placeholder_words.h"

// Symbols of the procedural lowering: each function and namespace-scope
// variable gets one SymbolId, its presentation spelling interned once
// (qualified by `__`, overloads numbered `__ovN` by declaration order), its
// object spelling built by PA9's typed Itanium encoder from the entity's
// scope and canonical type. External symbols are shared across units by
// their object name; internal ones are distinct per unit.
namespace lower {

namespace {

// A library builtin's C function: its name without `__builtin_`.
std::string LibraryName(const sema::FunctionEntity* function)
{
	return std::string(function->name->spelling + 10);
}

using abi_mangle::AbiNode;

abi_mangle::AbiBuiltinTypeKind BuiltinKind(FundamentalType type)
{
	switch (type)
	{
	case FundamentalType::SignedChar:
		return abi_mangle::ABI_BUILTIN_TYPE_SIGNED_CHAR;
	case FundamentalType::ShortInt:
		return abi_mangle::ABI_BUILTIN_TYPE_SHORT;
	case FundamentalType::Int:
		return abi_mangle::ABI_BUILTIN_TYPE_INT;
	case FundamentalType::LongInt:
		return abi_mangle::ABI_BUILTIN_TYPE_LONG;
	case FundamentalType::LongLongInt:
		return abi_mangle::ABI_BUILTIN_TYPE_LONG_LONG;
	case FundamentalType::UnsignedChar:
		return abi_mangle::ABI_BUILTIN_TYPE_UNSIGNED_CHAR;
	case FundamentalType::UnsignedShortInt:
		return abi_mangle::ABI_BUILTIN_TYPE_UNSIGNED_SHORT;
	case FundamentalType::UnsignedInt:
		return abi_mangle::ABI_BUILTIN_TYPE_UNSIGNED_INT;
	case FundamentalType::UnsignedLongInt:
		return abi_mangle::ABI_BUILTIN_TYPE_UNSIGNED_LONG;
	case FundamentalType::UnsignedLongLongInt:
		return abi_mangle::ABI_BUILTIN_TYPE_UNSIGNED_LONG_LONG;
	case FundamentalType::WcharT:
		return abi_mangle::ABI_BUILTIN_TYPE_WCHAR;
	case FundamentalType::Char:
		return abi_mangle::ABI_BUILTIN_TYPE_CHAR;
	case FundamentalType::Char16T:
		return abi_mangle::ABI_BUILTIN_TYPE_CHAR16;
	case FundamentalType::Char32T:
		return abi_mangle::ABI_BUILTIN_TYPE_CHAR32;
	case FundamentalType::Bool:
		return abi_mangle::ABI_BUILTIN_TYPE_BOOL;
	case FundamentalType::Float:
		return abi_mangle::ABI_BUILTIN_TYPE_FLOAT;
	case FundamentalType::Double:
		return abi_mangle::ABI_BUILTIN_TYPE_DOUBLE;
	case FundamentalType::LongDouble:
		return abi_mangle::ABI_BUILTIN_TYPE_LONG_DOUBLE;
	case FundamentalType::Void:
		return abi_mangle::ABI_BUILTIN_TYPE_VOID;
	case FundamentalType::NullptrT:
		return abi_mangle::ABI_BUILTIN_TYPE_NULLPTR;
	case FundamentalType::Int128:
		return abi_mangle::ABI_BUILTIN_TYPE_INT128;
	case FundamentalType::UnsignedInt128:
		return abi_mangle::ABI_BUILTIN_TYPE_UINT128;
	case FundamentalType::Float16:
		return abi_mangle::ABI_BUILTIN_TYPE_FLOAT16;
	case FundamentalType::Float32:
		return abi_mangle::ABI_BUILTIN_TYPE_FLOAT32;
	case FundamentalType::Float64:
		return abi_mangle::ABI_BUILTIN_TYPE_FLOAT64;
	case FundamentalType::Float128:
		return abi_mangle::ABI_BUILTIN_TYPE_STDFLOAT128;
	case FundamentalType::Float32x:
		return abi_mangle::ABI_BUILTIN_TYPE_FLOAT32X;
	case FundamentalType::Float64x:
		return abi_mangle::ABI_BUILTIN_TYPE_FLOAT64X;
	case FundamentalType::GnuFloat128:
		return abi_mangle::ABI_BUILTIN_TYPE_FLOAT128;
	case FundamentalType::BFloat16:
		return abi_mangle::ABI_BUILTIN_TYPE_BFLOAT16;
	}
	return abi_mangle::ABI_BUILTIN_TYPE_NONE;
}

} // namespace

abi_mangle::AbiTerminalKind OperatorTerminal(TokenKind op, bool array)
{
	using namespace abi_mangle;
	switch (op)
	{
	case TokenKind::KW_NEW:
		return array ? ABI_TERMINAL_NEW_ARRAY : ABI_TERMINAL_NEW;
	case TokenKind::KW_DELETE:
		return array ? ABI_TERMINAL_DELETE_ARRAY : ABI_TERMINAL_DELETE;
	case TokenKind::OP_PLUS:
		return ABI_TERMINAL_PLUS;
	case TokenKind::OP_MINUS:
		return ABI_TERMINAL_MINUS;
	case TokenKind::OP_STAR:
		return ABI_TERMINAL_MULTIPLY;
	case TokenKind::OP_DIV:
		return ABI_TERMINAL_DIVIDE;
	case TokenKind::OP_MOD:
		return ABI_TERMINAL_REMAINDER;
	case TokenKind::OP_AMP:
		return ABI_TERMINAL_BIT_AND;
	case TokenKind::OP_BOR:
		return ABI_TERMINAL_BIT_OR;
	case TokenKind::OP_XOR:
		return ABI_TERMINAL_BIT_XOR;
	case TokenKind::OP_COMPL:
		return ABI_TERMINAL_COMPLEMENT;
	case TokenKind::OP_ASS:
		return ABI_TERMINAL_ASSIGN;
	case TokenKind::OP_PLUSASS:
		return ABI_TERMINAL_PLUS_ASSIGN;
	case TokenKind::OP_MINUSASS:
		return ABI_TERMINAL_MINUS_ASSIGN;
	case TokenKind::OP_STARASS:
		return ABI_TERMINAL_MULTIPLY_ASSIGN;
	case TokenKind::OP_DIVASS:
		return ABI_TERMINAL_DIVIDE_ASSIGN;
	case TokenKind::OP_MODASS:
		return ABI_TERMINAL_REMAINDER_ASSIGN;
	case TokenKind::OP_BANDASS:
		return ABI_TERMINAL_AND_ASSIGN;
	case TokenKind::OP_BORASS:
		return ABI_TERMINAL_OR_ASSIGN;
	case TokenKind::OP_XORASS:
		return ABI_TERMINAL_XOR_ASSIGN;
	case TokenKind::OP_LSHIFT:
		return ABI_TERMINAL_LEFT_SHIFT;
	case TokenKind::OP_RSHIFT:
		return ABI_TERMINAL_RIGHT_SHIFT;
	case TokenKind::OP_LSHIFTASS:
		return ABI_TERMINAL_LEFT_SHIFT_ASSIGN;
	case TokenKind::OP_RSHIFTASS:
		return ABI_TERMINAL_RIGHT_SHIFT_ASSIGN;
	case TokenKind::OP_EQ:
		return ABI_TERMINAL_EQUAL;
	case TokenKind::OP_NE:
		return ABI_TERMINAL_NOT_EQUAL;
	case TokenKind::OP_LT:
		return ABI_TERMINAL_LESS;
	case TokenKind::OP_GT:
		return ABI_TERMINAL_GREATER;
	case TokenKind::OP_LE:
		return ABI_TERMINAL_LESS_EQUAL;
	case TokenKind::OP_GE:
		return ABI_TERMINAL_GREATER_EQUAL;
	case TokenKind::OP_LNOT:
		return ABI_TERMINAL_LOGICAL_NOT;
	case TokenKind::OP_LAND:
		return ABI_TERMINAL_LOGICAL_AND;
	case TokenKind::OP_LOR:
		return ABI_TERMINAL_LOGICAL_OR;
	case TokenKind::OP_INC:
		return ABI_TERMINAL_INCREMENT;
	case TokenKind::OP_DEC:
		return ABI_TERMINAL_DECREMENT;
	case TokenKind::OP_COMMA:
		return ABI_TERMINAL_COMMA;
	case TokenKind::OP_ARROWSTAR:
		return ABI_TERMINAL_MEMBER_POINTER;
	case TokenKind::OP_ARROW:
		return ABI_TERMINAL_ARROW;
	case TokenKind::OP_LPAREN:
		return ABI_TERMINAL_CALL;
	case TokenKind::OP_LSQUARE:
		return ABI_TERMINAL_INDEX;
	default:
		return ABI_TERMINAL_NONE;
	}
}

const sema::FunctionEntity* EnclosingFunction(const sema::Scope* scope)
{
	while (scope != nullptr && scope->kind == sema::ScopeKind::Class)
		scope = scope->parent;
	if (scope == nullptr || (scope->kind != sema::ScopeKind::Block && scope->kind != sema::ScopeKind::Function))
		return nullptr;
	for (; scope != nullptr && scope->kind != sema::ScopeKind::Namespace && scope->kind != sema::ScopeKind::Class;
		scope = scope->parent)
	{
		if (scope->kind == sema::ScopeKind::Function && scope->entity != nullptr &&
			scope->entity->kind == sema::EntityKind::Function)
			return static_cast<const sema::FunctionEntity*>(scope->entity);
	}
	return nullptr;
}

const sema::FunctionEntity* OutermostFunction(const sema::FunctionEntity* function)
{
	for (const sema::FunctionEntity* outer = EnclosingFunction(function->owner); outer != nullptr;
		outer = EnclosingFunction(outer->owner))
		function = outer;
	return function;
}

namespace {

bool IsMain(const sema::FunctionEntity* function)
{
	return function->name != nullptr && std::strcmp(function->name->spelling, "main") == 0 &&
		function->owner != nullptr && function->owner->kind == sema::ScopeKind::Namespace &&
		function->owner->parent == nullptr && !function->member;
}

// An anonymous union's first named data member in a pre-order,
// depth-first, declaration-order walk of its members (Itanium 5.1.2), or
// null.
const Identifier* FirstNamedMember(const sema::ClassEntity* cls)
{
	if (cls == nullptr)
		return nullptr;
	for (const sema::DataMember* member = cls->members; member != nullptr; member = member->next)
	{
		const sema::Entity* variable = member->variable;
		if (variable->name != nullptr)
			return variable->name;
		const sema::ClassEntity* nested = variable->type != nullptr ? sema::ClassOf(variable->type) : nullptr;
		if (nested != nullptr && nested->anonymous)
		{
			if (const Identifier* found = FirstNamedMember(nested))
				return found;
		}
	}
	return nullptr;
}

const char* NamespaceSpelling(const sema::Scope* scope)
{
	return scope->entity != nullptr && scope->entity->name != nullptr ? scope->entity->name->spelling : "_GLOBAL__N_1";
}

// The namespace a function or variable with linkage belongs to: its owner,
// or for a block-scope extern declaration the innermost enclosing
// namespace (3.5p6).
const sema::Scope* LinkageScope(const sema::Scope* scope)
{
	// A member's class; a block-scope declaration with linkage names a
	// member of the innermost enclosing namespace (3.5p7), not of a class
	// whose member function holds it.
	if (scope != nullptr && scope->kind == sema::ScopeKind::Class)
		return scope;
	while (scope != nullptr && scope->kind != sema::ScopeKind::Namespace)
		scope = scope->parent;
	return scope;
}

// Whether a return type is (a compound type of) a class or enumeration
// the function's own body declares.
bool OwnLocalType(const sema::Type* type, const sema::FunctionEntity* function)
{
	while (type->is_reference() || type->kind == sema::TypeKind::Pointer || type->kind == sema::TypeKind::Array)
		type = type->element;
	type = type->unqualified;
	return (type->kind == sema::TypeKind::Class || type->kind == sema::TypeKind::Enum) &&
		EnclosingFunction(type->entity->owner) == function;
}

bool NamespaceInternal(const sema::Scope* scope)
{
	return scope != nullptr && scope->kind == sema::ScopeKind::Namespace && scope->entity != nullptr &&
		static_cast<const sema::NamespaceEntity*>(scope->entity)->internal;
}

Locality Narrower(Locality a, Locality b)
{
	return static_cast<uint8_t>(a) > static_cast<uint8_t>(b) ? a : b;
}

// The non-type parameter a dependent array bound is just the name of (its
// bound mangles as that parameter, A T_ _), else null.
const sema::TemplateParameterEntity* BoundParameter(const sema::Type* array)
{
	return array->owner != nullptr ? static_cast<const sema::TemplateParameterEntity*>(array->owner->entity)->named
								   : nullptr;
}

// A pack expansion's pattern naming a bound pack (sema::TypeKind::BoundPack)
// outside the expansions nested in it.
bool NamesBoundPack(const sema::Type* type)
{
	switch (type->kind)
	{
	case sema::TypeKind::BoundPack:
		return true;
	case sema::TypeKind::TemplateParameter:
	{
		const sema::TemplateParameterEntity* parameter = static_cast<const sema::TemplateParameterEntity*>(
			type->entity);
		return parameter->qualifier != nullptr && NamesBoundPack(parameter->qualifier);
	}
	case sema::TypeKind::Class:
	{
		const sema::ClassTemplateFacts* facts = static_cast<const sema::ClassEntity*>(type->entity)->templ;
		if (facts == nullptr || facts->role != sema::TemplateRole::Specialization || !facts->dependent)
			return false;
		for (uint32_t i = 0; i < facts->argument_count; ++i)
		{
			if (NamesBoundPack(facts->arguments[i]))
				return true;
		}
		return false;
	}
	case sema::TypeKind::Pointer:
	case sema::TypeKind::LValueReference:
	case sema::TypeKind::RValueReference:
	case sema::TypeKind::Array:
		return NamesBoundPack(type->element);
	case sema::TypeKind::MemberPointer:
		return NamesBoundPack(type->owner) || NamesBoundPack(type->element);
	case sema::TypeKind::Function:
	case sema::TypeKind::Pack:
		for (uint32_t i = 0; i < type->param_count; ++i)
		{
			if (NamesBoundPack(type->params[i]))
				return true;
		}
		return type->kind == sema::TypeKind::Function && NamesBoundPack(type->element);
	default:
		return false;
	}
}

// A class's source name, or an unnamed class's stable label.
const char* ClassLabel(const sema::ClassEntity* entity)
{
	if (entity->name != nullptr)
		return entity->name->spelling;
	return entity->local_label != nullptr ? entity->local_label : entity->label != nullptr ? entity->label : "";
}

// A specialization's presentation label is bounded: specializations nest
// (a DAG of them through typedefs doubles its written form per level).
const size_t kLabelLimit = 96;

// A label as a presentation identifier: letters, digits and `_` kept,
// spaces dropped, any other character `_` (`~X` is `_X`, `operator[]` is
// `operator__`).
void AppendIdentifierSpelling(const char* label, std::string& out)
{
	// Spaces dropped, other characters but word ones as `_`; each run of
	// word characters appended at once.
	const char* run = label;
	for (const char* c = label;; ++c)
	{
		const char ch = *c;
		if ((ch >= 'a' && ch <= 'z') || (ch >= 'A' && ch <= 'Z') || (ch >= '0' && ch <= '9') || ch == '_')
			continue;
		out.append(run, static_cast<size_t>(c - run));
		if (ch == '\0')
			return;
		if (ch != ' ')
			out += '_';
		run = c + 1;
	}
}

} // namespace

// ------------------------------------------------------------ localities

Locality ProgramLowering::type_locality(const sema::Type* type, bool nested)
{
	// A type built from a local class or enumeration, or from a class of an
	// unnamed namespace, gives a template specialization it is an argument of
	// its locality (3.5p4, p8): a compound type's is its components'
	// narrowest, an argument pack's its elements' (`tuple<int, Local>`). A
	// function, member pointer or pack type within another type is
	// memoized, so a type whose components share types (through typedefs)
	// is walked once each; an outermost one (a declaration's) is walked.
	for (;;)
	{
		switch (type->kind)
		{
		case sema::TypeKind::Class:
		case sema::TypeKind::Enum:
			return entity_locality(type->entity);
		case sema::TypeKind::Pointer:
		case sema::TypeKind::LValueReference:
		case sema::TypeKind::RValueReference:
		case sema::TypeKind::Array:
			type = type->element;
			continue;
		case sema::TypeKind::MemberPointer:
		case sema::TypeKind::Function:
		case sema::TypeKind::Pack:
			break;
		case sema::TypeKind::Value:
			// A pointer or reference argument designating an object or
			// function of internal linkage (or of a class its unit alone
			// names) is its unit's own: each unit's specialization is
			// another (3.5p4, 14.4p1).
			if (!sema::AddressArgument(type) || type->entity == nullptr)
				return Locality::External;
			if (type->entity->kind == sema::EntityKind::Function)
				return unit_local(static_cast<const sema::FunctionEntity*>(type->entity)) ? Locality::Internal
																						  : Locality::External;
			return unit_local(static_cast<const sema::VariableEntity*>(type->entity)) ? Locality::Internal
																					  : Locality::External;
		default:
			return Locality::External;
		}
		break;
	}
	if (nested)
	{
		const uint32_t known = localities_.find(type);
		if (known != PointerMap::kMissing)
			return static_cast<Locality>(known);
	}
	const bool pack = type->kind == sema::TypeKind::Pack;
	Locality locality = pack ? Locality::External : type_locality(type->element, true);
	if (type->kind == sema::TypeKind::MemberPointer)
		locality = Narrower(locality, type_locality(type->owner, true));
	for (uint32_t i = 0; (pack || type->kind == sema::TypeKind::Function) && i < type->param_count; ++i)
		locality = Narrower(locality, type_locality(type->params[i], true));
	if (nested)
		localities_.set(type, static_cast<uint32_t>(locality));
	return locality;
}

Locality ProgramLowering::entity_locality(const sema::Entity* entity)
{
	// A class's or enumeration's: its enclosing class's, its
	// specialization's template arguments', and its namespace's or
	// function's; memoized for a specialization (whose arguments may share
	// components), walked up the enclosing classes for another.
	const sema::ClassTemplateFacts* facts = entity->kind == sema::EntityKind::Class
		? static_cast<const sema::ClassEntity*>(entity)->templ
		: nullptr;
	const bool specialization = facts != nullptr && facts->role == sema::TemplateRole::Specialization;
	if (specialization)
	{
		const uint32_t known = localities_.find(entity);
		if (known != PointerMap::kMissing)
			return static_cast<Locality>(known);
	}
	Locality locality = Locality::External;
	for (uint32_t i = 0; specialization && i < facts->argument_count; ++i)
		locality = Narrower(locality, type_locality(facts->arguments[i]));
	// 3.5p8: an unnamed class or enumeration (without a typedef-name for
	// linkage purposes) has its class's linkage as a member, none outside
	// a class.
	const sema::Scope* scope = entity->owner;
	if (scope != nullptr && scope->kind == sema::ScopeKind::Class)
	{
		locality = Narrower(locality, entity_locality(scope->entity));
		// A closure type of a class's scope numbered among its class's (a
		// default member initializer's, a member function's default
		// argument's; see Analyzer::number_closure) is the class's member
		// type in every unit. Another (a static data member's initializer's)
		// is numbered among its unit's closures outside function bodies, not
		// among its context's as Itanium 5.1.8 numbers them: its name has no
		// identity across units, so it is the unit's own.
		// Objects name one of an instance's member in its member's context
		// (closure_context_scope), alike in every unit too.
		const sema::ClosureFacts* closure = entity->name == nullptr && entity->kind == sema::EntityKind::Class
			? graph_->closure_facts(static_cast<const sema::ClassEntity*>(entity))
			: nullptr;
		if (closure != nullptr && !closure->member_context && !(object_mode_ && closure->context != nullptr))
			locality = Locality::Local;
	}
	else if (scope != nullptr && (scope->kind == sema::ScopeKind::Block || scope->kind == sema::ScopeKind::Function))
	{
		// Objects: a local type every unit names alike is one type there
		// (shared_local), with vague linkage (weak members, tables and type
		// information, as GCC and Clang emit them; specializations over it
		// too).
		if (!object_mode_ || !shared_local(entity))
			locality = Locality::Local;
	}
	else if (entity->name == nullptr)
		locality = Locality::Local;
	else if (NamespaceInternal(scope))
		locality = Narrower(locality, Locality::Internal);
	if (specialization)
		localities_.set(entity, static_cast<uint32_t>(locality));
	return locality;
}

Locality ProgramLowering::argument_locality(const sema::FunctionEntity* function)
{
	// A function template specialization's arguments'.
	Locality locality = Locality::External;
	if (sema::ArgumentsOf(function) == nullptr || function->primary == nullptr ||
		sema::HeadOf(function->primary) == nullptr)
		return locality;
	for (uint32_t i = 0; i < sema::HeadOf(function->primary)->count; ++i)
		locality = Narrower(locality, type_locality(sema::ArgumentsOf(function)[i]));
	return locality;
}

bool ProgramLowering::unit_local(const sema::FunctionEntity* function)
{
	// 3.5p4, p8: an entity of internal linkage, or a member of a class of an
	// unnamed namespace or of a local class, is its unit's own: another
	// unit's entity of the same object spelling is a different one. So is
	// a function whose type or template arguments involve such a class:
	// no other unit can name its type, and another unit's function of the
	// same name and spelling has another parameter-type-list (or, for a
	// function template, another signature: 3.5p9-p10, 14.5.6.1) and is a
	// different function (a friend defined in a specialization over such
	// a class, a function template naming one in its signature). A
	// C-linkage function is one function of its name (7.5p6). A shared
	// local class's member (see shared_local) is one function in every
	// unit, and a return type deduced as the function's own local type or
	// a shared one leaves the function one (3.5p4: its name has linkage).
	if (local_signature(function))
		return true;
	const sema::Type* result = function->type->canonical->element;
	while (result->is_reference() || result->kind == sema::TypeKind::Pointer || result->kind == sema::TypeKind::Array)
		result = result->element;
	const sema::Entity* type = result->unqualified->kind == sema::TypeKind::Class ||
			result->unqualified->kind == sema::TypeKind::Enum
		? result->unqualified->entity
		: nullptr;
	return !function->c_linkage && !OwnLocalType(result, function) && (type == nullptr || !shared_local(type)) &&
		type_locality(function->type->canonical->element, true) != Locality::External;
}

bool ProgramLowering::local_signature(const sema::FunctionEntity* function)
{
	// Whether the function's name and signature (its class, parameter
	// types and template arguments; 1.3.17) are its unit's own.
	const sema::Scope* owner = function->owner;
	if (function->internal_linkage || argument_locality(function) != Locality::External ||
		(owner != nullptr && owner->kind == sema::ScopeKind::Class &&
			entity_locality(owner->entity) != Locality::External && !shared_local(owner->entity)))
		return true;
	if (function->c_linkage)
		return false;
	const sema::Type* type = function->type->canonical;
	for (uint32_t i = 0; i < type->param_count; ++i)
	{
		if (type_locality(type->params[i], true) != Locality::External)
			return true;
	}
	return false;
}

bool ProgramLowering::shared_local(const sema::Entity* entity)
{
	// A local class (a closure included, or a member class of one) of a
	// function every unit defines alike (inline, no template's
	// specialization, its name and signature not its unit's own) is the
	// same type in every unit (7.1.2p4, 3.2p5): its members are one in
	// every unit, named in the function's context (Itanium 5.1.6, 5.1.8),
	// when its name is the same there (a named class's, a closure's; an
	// unnamed class's label is numbered in its unit). Template
	// specializations over it stay their unit's own (as the reference's
	// object roots present them). Memoized: a closure nested in another's
	// operator asks about the enclosing closure.
	// Objects share what every unit names alike as GCC and Clang do (5.1.5.5,
	// 5.1.6-5.1.8): a template specialization's local types too (each unit
	// instantiates the one definition, 14.7.1), unnamed ones by their numbers
	// (Ut<n>_ in their function or class), and specializations over such
	// types (see entity_locality).
	const uint32_t known = shared_locals_.find(entity);
	if (known != PointerMap::kMissing)
		return known != 0;
	const sema::Entity* type = entity;
	bool shared = true;
	for (;;)
	{
		const bool closure = type->kind == sema::EntityKind::Class &&
			closure_call(static_cast<const sema::ClassEntity*>(type)) != nullptr;
		const bool numbered = object_mode_ && type->name == nullptr && type->owner != nullptr &&
			(type->owner->kind == sema::ScopeKind::Class ||
				graph_->unnamed_local_ordinal(type) != PointerMap::kMissing);
		if ((type->name == nullptr && !closure && !numbered) || type->owner == nullptr ||
			(type->kind == sema::EntityKind::Class && static_cast<const sema::ClassEntity*>(type)->templ != nullptr))
		{
			shared = false;
			break;
		}
		if (type->owner->kind != sema::ScopeKind::Class)
			break;
		type = type->owner->entity;
	}
	if (shared)
	{
		const sema::FunctionEntity* enclosing = EnclosingFunction(type->owner);
		shared = enclosing != nullptr &&
			(object_mode_ ? enclosing->is_inline || enclosing->instantiated
						  : enclosing->is_inline && !enclosing->instantiated) &&
			!local_signature(enclosing);
	}
	shared_locals_.set(entity, shared ? 1 : 0);
	return shared;
}

bool ProgramLowering::private_type_name(const sema::Type* type)
{
	// A type information object's name string starts with `*` when the
	// type is its unit's own (g++'s mark of a name object that is not
	// public; libsupc++ and the link-time runtime then compare the type
	// information by address, not by name): a type no other unit can name
	// (3.5p4, p8), or a compound type or specialization over one. Another
	// unit's type of the same spelling is a different type. Object mode's
	// (the LowIR contract's references spell every name unmarked).
	for (;;)
	{
		switch (type->kind)
		{
		case sema::TypeKind::Class:
		case sema::TypeKind::Enum:
			return private_entity_name(type->entity);
		case sema::TypeKind::Pointer:
		case sema::TypeKind::LValueReference:
		case sema::TypeKind::RValueReference:
		case sema::TypeKind::Array:
			type = type->element;
			continue;
		case sema::TypeKind::MemberPointer:
			if (private_type_name(type->owner))
				return true;
			type = type->element;
			continue;
		case sema::TypeKind::Function:
			for (uint32_t i = 0; i < type->param_count; ++i)
			{
				if (private_type_name(type->params[i]))
					return true;
			}
			type = type->element;
			continue;
		default:
			return type_locality(type) != Locality::External;
		}
	}
}

bool ProgramLowering::private_entity_name(const sema::Entity* entity)
{
	// A class or enumeration of an unnamed namespace, or a specialization
	// over a private type, is private; a local one (a closure included, or
	// a member class of one) is unless its function is defined alike by
	// every unit that uses it (inline, or a template's specialization; not
	// its unit's own): then it is one type in every unit (3.2p5; g++ emits
	// its name weak, unmarked).
	if (entity_locality(entity) == Locality::External)
		return false;
	const sema::ClassTemplateFacts* facts = entity->kind == sema::EntityKind::Class
		? static_cast<const sema::ClassEntity*>(entity)->templ
		: nullptr;
	if (facts != nullptr && facts->role == sema::TemplateRole::Specialization)
	{
		for (uint32_t i = 0; i < facts->argument_count; ++i)
		{
			if (private_type_name(facts->arguments[i]))
				return true;
		}
		const sema::Scope* scope = entity->owner;
		if (scope != nullptr && scope->kind == sema::ScopeKind::Class)
			return private_entity_name(scope->entity);
		return scope == nullptr || scope->kind != sema::ScopeKind::Namespace || NamespaceInternal(scope);
	}
	const sema::Entity* type = entity;
	while (type->owner != nullptr && type->owner->kind == sema::ScopeKind::Class)
	{
		const bool closure = type->kind == sema::EntityKind::Class &&
			closure_call(static_cast<const sema::ClassEntity*>(type)) != nullptr;
		if ((type->name == nullptr && !closure) ||
			(type->kind == sema::EntityKind::Class && static_cast<const sema::ClassEntity*>(type)->templ != nullptr))
			return true;
		type = type->owner->entity;
	}
	const bool closure = type->kind == sema::EntityKind::Class &&
		closure_call(static_cast<const sema::ClassEntity*>(type)) != nullptr;
	if ((type->name == nullptr && !closure) || type->owner == nullptr ||
		(type->kind == sema::EntityKind::Class && static_cast<const sema::ClassEntity*>(type)->templ != nullptr))
		return true;
	const sema::FunctionEntity* enclosing = EnclosingFunction(type->owner);
	return enclosing == nullptr || !(enclosing->is_inline || enclosing->instantiated) || local_signature(enclosing);
}

bool ProgramLowering::unit_local(const sema::VariableEntity* variable)
{
	// As for functions: a variable of internal linkage, a member of a class
	// of internal or no linkage, or one whose type no other unit can name.
	const sema::Scope* owner = variable->owner;
	return variable->internal_linkage ||
		(owner != nullptr && owner->kind == sema::ScopeKind::Class &&
			entity_locality(owner->entity) != Locality::External) ||
		(!variable->c_linkage && type_locality(variable->type) != Locality::External);
}

// ---------------------------------------------------------------- labels

void ProgramLowering::append_type_label(const sema::Type* type, std::string& out, size_t limit)
{
	// A type as a C++ declaration would spell it (`const char*`), for a
	// specialization's presentation label; a class specialization by its
	// compact identity. Nothing more once the label is past `limit` (its
	// caller then labels the whole by an identity).
	if (out.size() > limit)
		return;
	if ((type->cv & sema::kConst) != 0 && type->kind != sema::TypeKind::Pointer)
		out += "const ";
	if ((type->cv & sema::kVolatile) != 0 && type->kind != sema::TypeKind::Pointer)
		out += "volatile ";
	switch (type->kind)
	{
	case sema::TypeKind::Fundamental:
		out += FundamentalTypeName(type->fundamental);
		break;
	case sema::TypeKind::Class:
	{
		const sema::ClassEntity* entity = static_cast<const sema::ClassEntity*>(type->entity);
		if (entity->templ != nullptr && entity->templ->role == sema::TemplateRole::Specialization)
			append_label_identity(entity, out);
		else
			out += ClassLabel(entity);
		break;
	}
	case sema::TypeKind::Enum:
	{
		const sema::EnumEntity* entity = static_cast<const sema::EnumEntity*>(type->entity);
		out += entity->label != nullptr ? entity->label : "";
		break;
	}
	case sema::TypeKind::TemplateParameter:
	case sema::TypeKind::TemplateTemplateParameter:
		out += type->entity->name != nullptr ? type->entity->name->spelling : "<parameter>";
		break;
	case sema::TypeKind::Template:
		// A template template-argument: its template's name.
		out += type->entity->kind == sema::EntityKind::Class
			? ClassLabel(static_cast<const sema::ClassEntity*>(type->entity))
			: type->entity->name->spelling;
		break;
	case sema::TypeKind::Pointer:
		append_type_label(type->element, out, limit);
		out += '*';
		if ((type->cv & sema::kConst) != 0)
			out += " const";
		if ((type->cv & sema::kVolatile) != 0)
			out += " volatile";
		break;
	case sema::TypeKind::LValueReference:
	case sema::TypeKind::RValueReference:
		append_type_label(type->element, out, limit);
		out += type->kind == sema::TypeKind::LValueReference ? "&" : "&&";
		break;
	case sema::TypeKind::MemberPointer:
		append_type_label(type->element, out, limit);
		out += ' ';
		append_type_label(type->owner, out, limit);
		out += "::*";
		break;
	case sema::TypeKind::Array:
		append_type_label(type->element, out, limit);
		out += '[' + std::to_string(type->bound) + ']';
		break;
	case sema::TypeKind::Vector:
		append_type_label(type->element, out, limit);
		out += " vector[" + std::to_string(type->bound) + ']';
		break;
	case sema::TypeKind::Complex:
		out += "_Complex ";
		append_type_label(type->element, out, limit);
		break;
	case sema::TypeKind::BlockPointer:
		append_type_label(type->element, out, limit);
		out += " block";
		break;
	case sema::TypeKind::Function:
		append_type_label(type->element, out, limit);
		out += '(';
		for (uint32_t i = 0; i < type->param_count && out.size() <= limit; ++i)
		{
			if (i != 0)
				out += ", ";
			append_type_label(type->params[i], out, limit);
		}
		out += type->variadic ? (type->param_count != 0 ? ", ...)" : "...)") : ")";
		break;
	case sema::TypeKind::Value:
	{
		if (sema::AddressArgument(type))
		{
			// A pointer or reference argument: `&name`, `name`, or a
			// null pointer.
			if (type->entity == nullptr)
			{
				out += "nullptr";
				break;
			}
			if (!type->element->is_reference())
				out += '&';
			qualified_spelling(LinkageScope(type->entity->owner), out);
			out += type->entity->name != nullptr ? type->entity->name->spelling : "";
			break;
		}
		// A non-type argument as a literal (`(Policy)2` for an
		// enumerator's).
		const sema::Type* value_type = type->element;
		if (value_type->kind == sema::TypeKind::Enum)
		{
			out += '(';
			append_type_label(value_type, out, limit);
			out += ')';
		}
		if (value_type->kind == sema::TypeKind::Fundamental && value_type->fundamental == FundamentalType::Bool)
			out += type->bound != 0 ? "true" : "false";
		else if (sema::IsSignedConstant(sema::Constant{ type->bound, value_type }))
			out += std::to_string(static_cast<long long>(type->bound));
		else
			out += std::to_string(type->bound);
		break;
	}
	case sema::TypeKind::Pack:
		for (uint32_t i = 0; i < type->param_count && out.size() <= limit; ++i)
		{
			if (i != 0)
				out += ", ";
			append_type_label(type->params[i], out, limit);
		}
		break;
	case sema::TypeKind::BoundPack:
		append_type_label(type->element, out, limit);
		break;
	case sema::TypeKind::Expansion:
		append_type_label(type->element, out, limit);
		out += "...";
		break;
	}
}

void ProgramLowering::append_class_label(const sema::ClassEntity* entity, std::string& out)
{
	// A class's label; a template specialization's with its arguments
	// (`box<const char*>`), or, were that longer than kLabelLimit, with its
	// compact identity.
	const sema::ClosureFacts* closure = located_closure(entity);
	if (closure != nullptr)
	{
		append_closure_label(entity, *closure, out);
		return;
	}
	const size_t start = out.size();
	out += ClassLabel(entity);
	const sema::ClassTemplateFacts* facts = entity->templ;
	if (facts == nullptr || facts->role != sema::TemplateRole::Specialization)
		return;
	out += '<';
	for (uint32_t i = 0; i < facts->argument_count; ++i)
	{
		if (i != 0)
			out += ", ";
		append_type_label(facts->arguments[i], out, start + kLabelLimit);
	}
	out += '>';
	if (out.size() - start <= kLabelLimit)
		return;
	out.resize(start);
	out += ClassLabel(entity);
	out += '<';
	append_label_identity(entity, out);
	out += '>';
}

const sema::ClosureFacts* ProgramLowering::located_closure(const sema::ClassEntity* entity) const
{
	if (entity->name != nullptr)
		return nullptr;
	const sema::ClosureFacts* facts = graph_->closure_facts(entity);
	return facts != nullptr && facts->enclosing != nullptr ? facts : nullptr;
}

// The closure type of a lambda in a function body is
// `__lambda_<function>_t<clause>_<body>` (its function's qualified name,
// the token ordinals of its parameter clause and body, `_n<k>` after for
// the function's k-th earlier closure of the same parameter types),
// declared where the function is; one in another lambda's body is
// `__lambda_nested_t<clause>_<body>`, declared in that lambda's closure.
void ProgramLowering::closure_prefix(const sema::ClosureFacts& facts, std::string& out)
{
	const sema::Scope* owner = facts.enclosing->owner;
	const sema::ClassEntity* outer = owner != nullptr && owner->kind == sema::ScopeKind::Class
		? static_cast<const sema::ClassEntity*>(owner->entity)
		: nullptr;
	if (outer != nullptr && graph_->closure_call(outer) == facts.enclosing)
	{
		out += class_spelling(outer);
		out += "__";
		return;
	}
	qualified_spelling(LinkageScope(owner), out);
}

void ProgramLowering::append_closure_label(
	const sema::ClassEntity* entity, const sema::ClosureFacts& facts, std::string& out)
{
	const sema::FunctionEntity* function = facts.enclosing;
	const sema::Scope* owner = function->owner;
	out += "__lambda_";
	if (owner != nullptr && owner->kind == sema::ScopeKind::Class &&
		graph_->closure_call(static_cast<const sema::ClassEntity*>(owner->entity)) == function)
		out += "nested";
	else
	{
		qualified_spelling(LinkageScope(owner), out);
		AppendIdentifierSpelling(function->label != nullptr ? function->label : function->name->spelling, out);
	}
	out += "_t" + std::to_string(facts.clause_token) + "_" + std::to_string(facts.body_token);
	if (entity->local_discriminator != 0)
		out += "_n" + std::to_string(entity->local_discriminator);
}

void ProgramLowering::owner_spelling(const sema::ClassEntity* entity, std::string& out)
{
	const sema::ClosureFacts* closure = located_closure(entity);
	if (closure != nullptr)
		closure_prefix(*closure, out);
	else
		qualified_spelling(entity->owner, out);
}

void ProgramLowering::append_label_identity(const sema::ClassEntity* entity, std::string& out)
{
	// A specialization's compact presentation identity, numbered in its
	// unit by first use (`__cppgm_class_template_identity_3`).
	uint32_t ordinal = label_identities_.find(entity);
	if (ordinal == PointerMap::kMissing)
	{
		ordinal = static_cast<uint32_t>(label_identities_.size()) + 1;
		label_identities_.set(entity, ordinal);
	}
	out += "__cppgm_class_template_identity_";
	out += std::to_string(ordinal);
}

// --------------------------------------------------------------- symbols

lowir::SymbolId ProgramLowering::new_symbol(const std::string& spelling)
{
	// A spelling another symbol took (an internal entity of an earlier
	// unit, a source name equal to a qualified one) is numbered apart.
	// Numbers below a spelling's next suffix are taken (symbols stay), so
	// the search resumes there: linear in the symbols made.
	const lowir::NameId base = intern(spelling.data(), static_cast<uint32_t>(spelling.size()));
	lowir::NameId name = base;
	if (program_.find_symbol(name) != lowir::kNoId)
	{
		if (next_suffix_.size() <= base)
			next_suffix_.resize(std::max<size_t>(base + 1, next_suffix_.size() * 2), 2);
		uint32_t suffix = next_suffix_[base];
		do
		{
			const std::string numbered = spelling + "__" + std::to_string(suffix++);
			name = intern(numbered.data(), static_cast<uint32_t>(numbered.size()));
		} while (program_.find_symbol(name) != lowir::kNoId);
		next_suffix_[base] = suffix;
	}
	const lowir::SymbolId symbol = program_.symbol_for(name);
	grow_symbol_tables();
	return symbol;
}

void ProgramLowering::grow_symbol_tables()
{
	// Tables indexed by symbol grow geometrically with the program.
	const size_t count = program_.symbols.size();
	if (pending_index_.size() < count)
		pending_index_.resize(std::max(count, pending_index_.size() * 2), 0);
	if (symbol_objects_.size() < count)
		symbol_objects_.resize(std::max(count, symbol_objects_.size() * 2), 0);
}

lowir::SymbolId ProgramLowering::external_symbol(const std::string& object, const std::string& spelling)
{
	const lowir::NameId key = intern(object.data(), static_cast<uint32_t>(object.size()));
	if (key < by_object_.size() && by_object_[key] != lowir::kNoId && by_object_[key] != kLocalObject)
		return by_object_[key];
	const lowir::SymbolId symbol = new_symbol(spelling);
	if (by_object_.size() <= key)
		by_object_.resize(std::max<size_t>(key + 1, by_object_.size() * 2), lowir::kNoId);
	by_object_[key] = symbol;
	symbol_objects_[symbol] = key;
	return symbol;
}

lowir::NameId ProgramLowering::local_object(const std::string& object, bool c_linkage)
{
	// An internal entity's object spelling is the backend's label for it,
	// so it must be unambiguous in the program: units merged into one
	// program each have their own `static` or unnamed-namespace entities of
	// one spelling (only the first keeps it), and a C name of an internal
	// entity could be an external C function's (it is left out). Without
	// one, the unique symbol name serves.
	if (c_linkage)
		return 0;
	const lowir::NameId key = intern(object.data(), static_cast<uint32_t>(object.size()));
	if (key < by_object_.size() && by_object_[key] != lowir::kNoId)
		return 0;
	if (by_object_.size() <= key)
		by_object_.resize(std::max<size_t>(key + 1, by_object_.size() * 2), lowir::kNoId);
	by_object_[key] = kLocalObject;
	return key;
}

void ProgramLowering::set_pending(lowir::SymbolId symbol, const PendingDeclaration& pending)
{
	grow_symbol_tables();
	if (pending_index_[symbol] != 0)
		return;
	pending_.push_back(pending);
	pending_index_[symbol] = static_cast<uint32_t>(pending_.size());
	// A back end selecting code before finish declares the global knows
	// how the declaration will bind (a host object's GOT access).
	if (!pending.function)
		program_.set_forward_global(symbol, pending.metadata.binding, pending.metadata.storage, pending.defined_here);
}

void ProgramLowering::state_pending_signatures(size_t first)
{
	// The unit's referenced functions still without a definition get their
	// declaration's boundary while their types exist; a definition states
	// its own.
	for (size_t i = first; i < pending_.size(); ++i)
	{
		if (pending_[i].function && program_.symbols[pending_[i].symbol].kind == lowir::SymbolKind::Unresolved &&
			pending_[i].signature == lowir::kNoId)
		{
			const lowir::SignatureId signature = function_signature(pending_[i].entity, false, pending_[i].variant);
			pending_[i].signature = signature;
		}
		pending_[i].entity = nullptr;
	}
}

void ProgramLowering::qualified_spelling(const sema::Scope* scope, std::string& out)
{
	// Enclosing namespaces and classes outermost first, each followed by
	// `__` (a class's enclosing scope is the one it is a member of).
	if (scope == nullptr || scope->parent == nullptr)
		return;
	if (scope->kind == sema::ScopeKind::Namespace)
	{
		qualified_spelling(scope->parent, out);
		out += NamespaceSpelling(scope);
		out += "__";
		return;
	}
	if (scope->kind != sema::ScopeKind::Class || scope->entity == nullptr)
		return;
	// A class scope's is made once per unit (its members' functions each
	// ask for it).
	const uint32_t index = class_spelling_index_.find(scope);
	if (index != PointerMap::kMissing)
	{
		out += class_spellings_[index];
		return;
	}
	const size_t start = out.size();
	owner_spelling(static_cast<const sema::ClassEntity*>(scope->entity), out);
	std::string label;
	append_class_label(static_cast<const sema::ClassEntity*>(scope->entity), label);
	AppendIdentifierSpelling(label.c_str(), out);
	out += "__";
	class_spelling_index_.set(scope, static_cast<uint32_t>(class_spellings_.size()));
	class_spellings_.push_back(out.substr(start));
}

std::string ProgramLowering::function_spelling(const sema::FunctionEntity* function)
{
	if (function->builtin == sema::BuiltinFunction::Library)
		return LibraryName(function);
	if (function->builtin != sema::BuiltinFunction::None || (function->c_linkage && !function->internal_linkage))
		return function->name->spelling;
	std::string spelling;
	qualified_spelling(LinkageScope(function->owner), spelling);
	// Constructors, destructors and operators by their label (`X`, `~X`,
	// `operator+`), spelled as an identifier.
	AppendIdentifierSpelling(function->label != nullptr ? function->label : function->name->spelling, spelling);
	const uint32_t ordinal = overload_ordinal(function);
	if (ordinal > 1)
		spelling += "__ov" + std::to_string(ordinal);
	return spelling;
}

std::string ProgramLowering::class_spelling(const sema::ClassEntity* entity)
{
	std::string spelling;
	owner_spelling(entity, spelling);
	std::string label;
	append_class_label(entity, label);
	AppendIdentifierSpelling(label.c_str(), spelling);
	return spelling;
}

uint32_t ProgramLowering::overload_ordinal(const sema::FunctionEntity* function)
{
	// The functions of the name declared in its scope, in declaration
	// order (the lookup set lists them newest first), numbered once for
	// the whole set.
	const uint32_t known = overload_ordinals_.find(function);
	if (known != PointerMap::kMissing)
		return known;
	uint32_t total = 0;
	const sema::NameEntry* entry = graph_->names.find(function->owner, function->name);
	for (const sema::FunctionList* node = entry != nullptr ? entry->functions : nullptr; node != nullptr;
		node = node->next)
		total += node->binding->entity->owner == function->owner ? 1 : 0;
	uint32_t position = 0;
	for (const sema::FunctionList* node = entry != nullptr ? entry->functions : nullptr; node != nullptr;
		node = node->next)
	{
		if (node->binding->entity->owner != function->owner)
			continue;
		overload_ordinals_.set(node->binding->entity, total - position);
		++position;
	}
	const uint32_t ordinal = overload_ordinals_.find(function);
	if (ordinal != PointerMap::kMissing)
		return ordinal;
	overload_ordinals_.set(function, 1);
	return 1;
}

lowir::SymbolId ProgramLowering::function_symbol(const sema::FunctionEntity* function)
{
	// A block-scope declaration names the function it denotes (3.5p6).
	if (function->denotes != nullptr)
		function = function->denotes;
	const uint32_t found = symbols_.find(function);
	if (found != PointerMap::kMissing)
		return found;
	const std::string object = function_object(function, EntryVariant::Complete);
	lowir::SymbolId symbol = lowir::kNoId;
	if (unit_local(function))
	{
		symbol = new_symbol(function_spelling(function));
		symbol_objects_[symbol] = local_object(object, function->c_linkage);
	}
	else
		symbol = external_symbol(object, function_spelling(function));
	symbols_.set(function, symbol);
	if (program_.symbols[symbol].kind == lowir::SymbolKind::Unresolved)
	{
		PendingDeclaration pending = PendingDeclaration();
		pending.symbol = symbol;
		pending.function = true;
		pending.entity = function;
		pending.signature = lowir::kNoId;
		pending.metadata = function_metadata(function, symbol);
		set_pending(symbol, pending);
	}
	return symbol;
}

lowir::SymbolId ProgramLowering::entry_symbol(const sema::FunctionEntity* function, EntryVariant variant)
{
	if (variant == EntryVariant::Complete || !(function->constructor || function->destructor))
		return function_symbol(function);
	PointerMap& symbols = variant == EntryVariant::Base ? base_symbols_ : deleting_symbols_;
	const uint32_t found = symbols.find(function);
	if (found != PointerMap::kMissing)
		return found;
	const std::string object = function_object(function, variant);
	const std::string spelling = function_spelling(function) +
		(variant == EntryVariant::Base ? "__base_entry" : "__deleting_entry");
	lowir::SymbolId symbol = lowir::kNoId;
	if (unit_local(function))
	{
		symbol = new_symbol(spelling);
		symbol_objects_[symbol] = local_object(object, false);
	}
	else
		symbol = external_symbol(object, spelling);
	symbols.set(function, symbol);
	if (program_.symbols[symbol].kind == lowir::SymbolKind::Unresolved)
	{
		PendingDeclaration pending = PendingDeclaration();
		pending.symbol = symbol;
		pending.function = true;
		pending.entity = function;
		pending.variant = variant;
		pending.signature = lowir::kNoId;
		pending.metadata = function_metadata(function, symbol);
		set_pending(symbol, pending);
	}
	return symbol;
}

lowir::SymbolId ProgramLowering::variable_symbol(const sema::VariableEntity* variable)
{
	const uint32_t found = symbols_.find(variable);
	if (found != PointerMap::kMissing)
		return found;
	std::string spelling;
	qualified_spelling(LinkageScope(variable->owner), spelling);
	lowir::SymbolId symbol = lowir::kNoId;
	const sema::Decomposition* decomposition = variable->name == nullptr ? graph_->decomposition(variable) : nullptr;
	if (variable->name == nullptr && decomposition == nullptr)
	{
		// An anonymous union's unnamed object (internal, 9.5p5): Itanium
		// 5.1.2 names it by its first named data member in a pre-order,
		// depth-first, declaration-order walk, which Clang mangles in the
		// object's scope without the internal marker (`_Z1a`,
		// `_ZN12_GLOBAL__N_11aE`); one without any has no object spelling.
		spelling += "__anonymous_union_object";
		symbol = new_symbol(spelling);
		if (const Identifier* first = FirstNamedMember(sema::ClassOf(variable->type)))
		{
			abi_mangle::AbiTarget target;
			target.kind = abi_mangle::ABI_TARGET_ENTITY;
			target.node = abi_graph_.name(
				abi_scope(LinkageScope(variable->owner)), abi_graph_.string(first->spelling, first->length));
			std::string object;
			encoder_->encode(target, object);
			++counters_.mangled_names;
			symbol_objects_[symbol] = local_object(object, false);
		}
		symbols_.set(variable, symbol);
		return symbol;
	}
	if (decomposition != nullptr)
	{
		// A structured binding declaration's unnamed object: spelled by
		// its names, which no other declaration of its scope has.
		spelling += "__binding";
		for (uint32_t i = 0; i < decomposition->count; ++i)
		{
			spelling += "__";
			spelling += decomposition->bindings[i]->name->spelling;
		}
	}
	else
		spelling += variable->name->spelling;
	const std::string object = variable_object(variable);
	if (unit_local(variable))
	{
		symbol = new_symbol(spelling);
		symbol_objects_[symbol] = local_object(object, variable->c_linkage);
	}
	else
		symbol = external_symbol(object, spelling);
	symbols_.set(variable, symbol);
	if (program_.symbols[symbol].kind == lowir::SymbolKind::Unresolved)
	{
		// An object of unknown bound, class or array type is imported
		// untyped (its layout is not this unit's to state).
		PendingDeclaration pending = PendingDeclaration();
		pending.symbol = symbol;
		pending.function = false;
		const sema::Type* type = variable->type;
		pending.typed = type->kind != sema::TypeKind::Array && type->unqualified->kind != sema::TypeKind::Class;
		if (pending.typed)
			pending.type = storage_type(type);
		pending.metadata = variable_metadata(variable, symbol);
		// (a GNU weak one stays reached through its GOT entry, as defined)
		pending.defined_here = variable->defined && (pending.metadata.flags & lowir::SF_REPLACEABLE) == 0;
		set_pending(symbol, pending);
	}
	// A class template instance's static data member is defined where it is
	// used (14.7.1p1), weak like the unit's other demand-emitted entities.
	const sema::VariableTemplateFacts* facts = variable->templ;
	if (facts != nullptr && facts->pattern != nullptr &&
		program_.symbols[symbol].kind == lowir::SymbolKind::Unresolved &&
		!sema::Analyzer::InstantiationSuppressed(variable) && analyzer_->instantiate_variable(variable))
	{
		define_global(symbol, variable, facts->initializer, facts->location);
		++counters_.globals;
	}
	// A static data member defined inline in its class (C++17's 9.4.2p3)
	// is defined where it is used too, weak as an inline variable.
	else if (facts == nullptr && variable->is_inline && variable->defined && variable->owner != nullptr &&
		variable->owner->kind == sema::ScopeKind::Class &&
		program_.symbols[symbol].kind == lowir::SymbolKind::Unresolved)
	{
		define_global(symbol, variable, nullptr, 0);
		++counters_.globals;
	}
	return symbol;
}

// ----------------------------------------------------------- ABI names

AbiNode ProgramLowering::abi_scope(const sema::Scope* scope)
{
	if (scope == nullptr || scope->parent == nullptr)
		return abi_mangle::ABI_NONE;
	switch (scope->kind)
	{
	case sema::ScopeKind::Namespace:
	{
		// ::std is the St prefix (5.1.2), never a substitution candidate.
		const char* spelling = NamespaceSpelling(scope);
		if (scope->parent->parent == nullptr && std::strcmp(spelling, "std") == 0)
			return abi_graph_.std_scope();
		const AbiNode parent = abi_scope(scope->parent);
		const AbiNode unqualified = abi_graph_.string(spelling, std::strlen(spelling));
		// An inline namespace's GNU ABI tags are not encoded (see
		// with_implicit_tags).
		const size_t base = abi_scratch_.size();
		const uint32_t count = push_abi_tags(scope);
		const AbiNode name = count == 0
			? abi_graph_.name(parent, unqualified)
			: abi_graph_.tagged_namespace(parent, unqualified, abi_scratch_.data() + base, count);
		abi_scratch_.resize(base);
		return name;
	}
	case sema::ScopeKind::Class:
	case sema::ScopeKind::Enum:
	{
		const sema::Entity* entity = scope->entity;
		if (scope->kind == sema::ScopeKind::Class)
			return class_name(static_cast<const sema::ClassEntity*>(entity));
		return abi_graph_.name(abi_scope(entity->owner), type_unqualified_name(entity));
	}
	case sema::ScopeKind::Block:
	case sema::ScopeKind::Function:
	{
		// 5.1.6: a local entity's name is Z <function encoding> E <name>.
		const sema::Scope* function = scope;
		while (function != nullptr && function->kind != sema::ScopeKind::Function)
			function = function->parent;
		if (function == nullptr || function->entity == nullptr || function->entity->kind != sema::EntityKind::Function)
			throw LoweringError(0, "a local entity's name is outside the procedural lowering slice");
		const sema::FunctionEntity* enclosing = static_cast<const sema::FunctionEntity*>(function->entity);
		if (IsMain(enclosing) || enclosing->c_linkage)
		{
			// An unmangled function's context is its bare source name.
			const std::string spelling = enclosing->name->spelling;
			const std::string raw = "Z" + std::to_string(spelling.size()) + spelling + "E";
			return abi_graph_.raw_function_scope(abi_graph_.string(raw.data(), raw.size()));
		}
		// A function whose implicit tags are being derived (its result a
		// type local to it, as a lambda returning a closure) is named by its
		// encoding without them: a local type adds no tag its function's
		// name lacks.
		for (const EncodingUnderway& underway : encodings_underway_)
		{
			if (underway.function == enclosing)
				return abi_graph_.function_scope(underway.underived);
		}
		return abi_graph_.function_scope(function_encoding(enclosing, EntryVariant::Complete));
	}
	default:
		throw LoweringError(0, "a local entity's name is outside the procedural lowering slice");
	}
}

uint32_t ProgramLowering::push_abi_tags(const void* owner)
{
	// GNU abi_tag (pa28): the tags of a class, function, variable or
	// inline namespace (none in a unit without tags), pushed on
	// abi_scratch_ as STRING nodes; their count.
	const sema::SemanticGraph::AbiTags* tags = graph_->has_abi_tags() ? graph_->abi_tags(owner) : nullptr;
	if (tags == nullptr)
		return 0;
	for (uint32_t i = 0; i < tags->count; ++i)
		abi_scratch_.push_back(abi_graph_.string(tags->tags[i], std::strlen(tags->tags[i])));
	return tags->count;
}

AbiNode ProgramLowering::tagged_name(
	AbiNode scope, AbiNode unqualified, const void* owner, uint32_t discriminator, bool internal_linkage)
{
	// <unqualified-name> ::= <source-name> <abi-tags> (and the other forms
	// with theirs): a name with its entity's explicit ABI tags.
	const size_t base = abi_scratch_.size();
	const uint32_t count = push_abi_tags(owner);
	const AbiNode name = abi_graph_.name(
		scope, unqualified, count == 0 ? nullptr : abi_scratch_.data() + base, count, discriminator, internal_linkage);
	abi_scratch_.resize(base);
	return name;
}

AbiNode ProgramLowering::with_implicit_tags(AbiNode entity, const sema::Type* type)
{
	// GCC's implicit ABI tags (its abi_tag documentation; Clang derives
	// them alike): the tags of a function's result type, or a variable's
	// type, that its encoding (a function's name, template arguments and
	// signature; a variable's name) lacks join its name's.
	if (!graph_->has_abi_tags())
		return entity;
	abi_type_tags_.clear();
	abi_graph_.collect_tags(abi_type(type), abi_type_tags_, true);
	if (abi_type_tags_.empty())
		return entity;
	abi_used_tags_.clear();
	abi_graph_.collect_tags(entity, abi_used_tags_, true);
	// The type's tags less the encoding's, as sorted sets of interned
	// STRING nodes.
	std::sort(abi_type_tags_.begin(), abi_type_tags_.end());
	abi_type_tags_.erase(std::unique(abi_type_tags_.begin(), abi_type_tags_.end()), abi_type_tags_.end());
	std::sort(abi_used_tags_.begin(), abi_used_tags_.end());
	size_t added = 0;
	for (size_t t = 0, u = 0; t < abi_type_tags_.size(); ++t)
	{
		while (u < abi_used_tags_.size() && abi_used_tags_[u] < abi_type_tags_[t])
			++u;
		if (u == abi_used_tags_.size() || abi_used_tags_[u] != abi_type_tags_[t])
			abi_type_tags_[added++] = abi_type_tags_[t];
	}
	if (added == 0)
		return entity;
	const abi_mangle::AbiNodeData data = abi_graph_.node(entity);
	AbiNode derived = abi_mangle::ABI_NONE;
	if (data.kind != abi_mangle::ABI_NODE_FUNCTION_ENCODING)
		derived = abi_graph_.with_tags(entity, abi_type_tags_.data(), added);
	else
	{
		const AbiNode name = abi_graph_.with_tags(data.a, abi_type_tags_.data(), added);
		const size_t base = abi_scratch_.size();
		abi_scratch_.insert(abi_scratch_.end(), abi_graph_.list(entity), abi_graph_.list(entity) + data.list_size);
		derived = abi_graph_.function_encoding(name, data.b, abi_scratch_.data() + base, data.list_size,
			(data.flags & abi_mangle::ABI_FLAG_VARIADIC) != 0, data.code);
		abi_scratch_.resize(base);
	}
	abi_graph_.note_implicit_tags(derived, entity);
	return derived;
}

AbiNode ProgramLowering::abi_type(const sema::Type* type)
{
	// Types nest by sharing (a DAG of specializations or function types
	// through typedefs): a node with several components, a specialization's
	// arguments, a function's parameters, a member pointer's class and
	// member, is built once per unit, which keeps the walk linear in the
	// distinct types; the others (fundamental types, and pointers,
	// references, arrays and other classes over one component) are built
	// directly, the graph interning what they build.
	const sema::Type* bare = type->unqualified;
	const sema::ClassTemplateFacts* facts = bare->kind == sema::TypeKind::Class
		? static_cast<const sema::ClassEntity*>(bare->entity)->templ
		: nullptr;
	// A pack expansion's pattern naming a bound pack encodes the template
	// parameters of the signature being encoded by their levels (see
	// abi_own_level_): its types are not shared, nor are those naming it
	// (`abi_level_dependent_`).
	const bool shared = !abi_bound_pattern_ &&
		(bare->kind == sema::TypeKind::Function || bare->kind == sema::TypeKind::MemberPointer ||
			(facts != nullptr &&
				(facts->role == sema::TemplateRole::Specialization || facts->role == sema::TemplateRole::Template)));
	if (!shared)
		return abi_type_node(type);
	const uint32_t known = abi_types_.find(type);
	if (known != PointerMap::kMissing)
		return known;
	const bool saved = abi_level_dependent_;
	abi_level_dependent_ = false;
	const AbiNode node = abi_type_node(type);
	if (!abi_level_dependent_)
		abi_types_.set(type, node);
	abi_level_dependent_ = abi_level_dependent_ || saved;
	return node;
}

uint32_t ProgramLowering::own_level(const sema::TemplateParameterEntity* parameter) const
{
	// A template parameter of the encoded signature, inside a pattern naming
	// a bound pack: its depth from the outermost template (else none, T_).
	return abi_bound_pattern_ ? abi_own_level_ + parameter->depth : 0;
}

AbiNode ProgramLowering::abi_type_node(const sema::Type* type)
{
	AbiNode node = abi_mangle::ABI_NONE;
	const sema::Type* bare = type->unqualified;
	switch (bare->kind)
	{
	case sema::TypeKind::Fundamental:
		node = abi_graph_.builtin(BuiltinKind(bare->fundamental));
		break;
	case sema::TypeKind::BlockPointer:
		// Clang's U13block_pointer <function type>.
		node = abi_graph_.vendor_qualified(abi_type(bare->element), abi_graph_.string("block_pointer", 13));
		break;
	case sema::TypeKind::Complex:
		// C <type> (the hosts' Cf, Cd, Ce, CDF16_, Ci ...), substitutable.
		node = abi_graph_.complex_pair(abi_type(bare->element));
		break;
	case sema::TypeKind::Class:
	{
		// An alias template-id dropping dependent arguments names the type
		// it aliases (CWG 1558; the hosts mangle aliases transparently).
		if (const sema::Type* aliased = analyzer_->aliased_of(bare))
		{
			node = abi_type(aliased);
			break;
		}
		node = class_name(static_cast<const sema::ClassEntity*>(bare->entity));
		break;
	}
	case sema::TypeKind::Enum:
		// With its GNU ABI tags (GCC's mangling; Clang rejects them).
		node = tagged_name(abi_scope(bare->entity->owner), type_unqualified_name(bare->entity), bare->entity, 0, false);
		break;
	case sema::TypeKind::TemplateParameter:
	{
		// 5.1.8: a template parameter of the encoded template's signature
		// (T_, T0_, ...); a dependent name, its qualifier's member; a
		// decltype-specifier or value-dependent argument, its expression
		// (see placeholder_type).
		const sema::TemplateParameterEntity* parameter = static_cast<const sema::TemplateParameterEntity*>(
			bare->entity);
		const AbiNode placeholder = parameter->expression != nullptr ? placeholder_type(parameter)
																	 : abi_mangle::ABI_NONE;
		if (placeholder != abi_mangle::ABI_NONE)
			node = placeholder;
		else if (parameter->qualifier != nullptr)
		{
			const char* spelling = parameter->name->spelling;
			node = abi_graph_.name(abi_type(parameter->qualifier), abi_graph_.string(spelling, std::strlen(spelling)));
		}
		// A non-type parameter as a template argument is an expression
		// (5.1.5: X T_ E).
		else if (parameter->non_type)
			node = abi_graph_.expression_argument(
				abi_graph_.template_param(parameter->index, false, own_level(parameter)));
		else
			node = abi_graph_.template_param(parameter->index, true, own_level(parameter), parameter->pack ? 1 : 0);
		break;
	}
	case sema::TypeKind::BoundPack:
	{
		// A pack an instance binds, in a pattern it expands with packs of
		// the encoded template (no rule of the ABI's; GCC's, ABI 19: the
		// pack's own parameter at its level from the outermost template,
		// and the encoded template's parameters there at theirs, see
		// own_level).
		const sema::TemplateParameterEntity* parameter = static_cast<const sema::TemplateParameterEntity*>(
			bare->entity);
		const uint32_t level = static_cast<uint32_t>(bare->bound);
		if (parameter->non_type)
			node = abi_graph_.expression_argument(abi_graph_.template_param(parameter->index, false, level));
		else
			node = abi_graph_.template_param(parameter->index, true, level, parameter->template_template ? 3 : 1);
		break;
	}
	case sema::TypeKind::Pointer:
		node = abi_graph_.pointer(abi_type(bare->element));
		break;
	case sema::TypeKind::LValueReference:
		node = abi_graph_.lvalue_reference(abi_type(bare->element));
		break;
	case sema::TypeKind::RValueReference:
		node = abi_graph_.rvalue_reference(abi_type(bare->element));
		break;
	case sema::TypeKind::MemberPointer:
		node = abi_graph_.member_pointer(abi_type(bare->owner), abi_type(bare->element));
		break;
	case sema::TypeKind::Vector:
		// Dv <lanes> _ <lane type> (Itanium 5.1.5, GCC's and Clang's).
		node = abi_graph_.array(abi_type(bare->element), abi_mangle::ABI_ARRAY_BOUND_VECTOR,
			static_cast<long long>(bare->bound), abi_mangle::ABI_NONE);
		break;
	case sema::TypeKind::Array:
	{
		// A bound that is a non-type parameter's name is that parameter
		// (5.1.5: A T_ _ element); another dependent bound its expression.
		const AbiNode bound = bare->owner != nullptr && BoundParameter(bare) == nullptr
			? placeholder_expression(static_cast<const sema::TemplateParameterEntity*>(bare->owner->entity))
			: abi_mangle::ABI_NONE;
		if (const sema::TemplateParameterEntity* parameter = BoundParameter(bare))
			node = abi_graph_.array(abi_type(bare->element), abi_mangle::ABI_ARRAY_BOUND_EXPRESSION, 0,
				abi_graph_.template_param(parameter->index, true));
		else if (bound != abi_mangle::ABI_NONE)
			node = abi_graph_.array(abi_type(bare->element), abi_mangle::ABI_ARRAY_BOUND_EXPRESSION, 0, bound);
		else
			node = abi_graph_.array(abi_type(bare->element),
				bare->bound != 0 ? abi_mangle::ABI_ARRAY_BOUND_VALUE : abi_mangle::ABI_ARRAY_BOUND_NONE,
				static_cast<long long>(bare->bound), abi_mangle::ABI_NONE);
		break;
	}
	case sema::TypeKind::Function:
	{
		const AbiNode result = abi_type(bare->element);
		const size_t base = abi_scratch_.size();
		for (uint32_t i = 0; i < bare->param_count; ++i)
		{
			const AbiNode param = abi_type(bare->params[i]);
			abi_scratch_.push_back(param);
		}
		unsigned qualifiers = 0;
		if ((bare->function_cv & sema::kConst) != 0)
			qualifiers |= abi_mangle::ABI_QUALIFIER_CONST;
		if ((bare->function_cv & sema::kVolatile) != 0)
			qualifiers |= abi_mangle::ABI_QUALIFIER_VOLATILE;
		if (bare->ref == sema::RefQualifier::LValue)
			qualifiers |= abi_mangle::ABI_QUALIFIER_LVALUE_REF;
		else if (bare->ref == sema::RefQualifier::RValue)
			qualifiers |= abi_mangle::ABI_QUALIFIER_RVALUE_REF;
		node = abi_graph_.function_type(
			result, abi_scratch_.data() + base, bare->param_count, bare->variadic, qualifiers);
		abi_scratch_.resize(base);
		break;
	}
	case sema::TypeKind::Value:
		// 5.1.5, 5.1.6: a non-type argument is its literal (Li3E, Lb1E,
		// L6Policy2E); a pointer argument the address of its entity
		// (XadL_Z1xEE), a reference argument the entity (L_Z1xE), a null
		// pointer its type's zero (LPi0E).
		if (sema::AddressArgument(bare) && bare->entity != nullptr)
		{
			const AbiNode entity = bare->entity->kind == sema::EntityKind::Function
				? function_encoding(static_cast<const sema::FunctionEntity*>(bare->entity), EntryVariant::Complete)
				: variable_name(static_cast<const sema::VariableEntity*>(bare->entity));
			node = abi_graph_.entity_argument(entity, !bare->element->is_reference());
			break;
		}
		node = abi_literal(bare->element, bare->bound);
		break;
	case sema::TypeKind::Template:
	{
		// 5.1.5: a template template-argument is its template's name
		// (without its ABI tags, as Clang writes it; GCC adds them).
		const char* spelling = bare->entity->name->spelling;
		node = abi_graph_.name(abi_scope(bare->entity->owner), abi_graph_.string(spelling, std::strlen(spelling)));
		break;
	}
	case sema::TypeKind::TemplateTemplateParameter:
	{
		// A template template-parameter is T_ (5.1.8); a dependent member
		// template-name, its qualifier's member.
		const sema::TemplateParameterEntity* parameter = static_cast<const sema::TemplateParameterEntity*>(
			bare->entity);
		if (parameter->qualifier != nullptr)
		{
			const char* spelling = parameter->name->spelling;
			node = abi_graph_.name(abi_type(parameter->qualifier), abi_graph_.string(spelling, std::strlen(spelling)));
		}
		else
			node = abi_graph_.template_param(parameter->index, true, own_level(parameter), parameter->pack ? 3 : 2);
		break;
	}
	case sema::TypeKind::Pack:
	{
		// 5.1.5: an argument pack, J <template-arg>* E.
		const size_t base = abi_scratch_.size();
		for (uint32_t i = 0; i < bare->param_count; ++i)
		{
			const AbiNode element = abi_type(bare->params[i]);
			abi_scratch_.push_back(element);
		}
		node = abi_graph_.argument_pack(abi_scratch_.data() + base, bare->param_count);
		abi_scratch_.resize(base);
		break;
	}
	case sema::TypeKind::Expansion:
	{
		// A non-type parameter pack's expansion is an expression's (5.1.6:
		// X sp T_ E), as a value-dependent pattern's is (X sp <pattern> E);
		// a type pattern's is Dp.
		const sema::Type* pattern = bare->element;
		const sema::TemplateParameterEntity* parameter = pattern->kind == sema::TypeKind::TemplateParameter
			? static_cast<const sema::TemplateParameterEntity*>(pattern->entity)
			: nullptr;
		const bool bound = !abi_bound_pattern_ && NamesBoundPack(pattern);
		abi_bound_pattern_ = abi_bound_pattern_ || bound;
		abi_level_dependent_ = abi_level_dependent_ || bound;
		const AbiNode operand = parameter == nullptr || !parameter->non_type ? abi_mangle::ABI_NONE
			: parameter->expression == nullptr
			? abi_graph_.template_param(parameter->index, false, own_level(parameter))
			: placeholder_expression(parameter);
		if (operand != abi_mangle::ABI_NONE)
			node = abi_graph_.expression_argument(
				abi_graph_.operation(abi_mangle::abi_pack_operation_code('s', 'p'), &operand, 1));
		else
			node = abi_graph_.pack_expansion(abi_type(pattern));
		abi_bound_pattern_ = abi_bound_pattern_ && !bound;
		break;
	}
	default:
		throw LoweringError(0, "a type outside the procedural lowering slice");
	}
	// An atomic type is Clang's U7_Atomic <type>, inside its cv-qualifiers.
	if ((type->cv & sema::kAtomic) != 0)
		node = abi_graph_.vendor_qualified(node, abi_graph_.string("_Atomic", 7));
	unsigned cv = 0;
	if ((type->cv & sema::kConst) != 0)
		cv |= abi_mangle::ABI_QUALIFIER_CONST;
	if ((type->cv & sema::kVolatile) != 0)
		cv |= abi_mangle::ABI_QUALIFIER_VOLATILE;
	return cv != 0 ? abi_graph_.qualified(node, cv) : node;
}

AbiNode ProgramLowering::abi_literal(const sema::Type* type, uint64_t bits)
{
	// L <type> <value> E (5.1.6.1): an enumeration's value is its
	// underlying type's, unsigned for an unsigned one (2^64 - 1 is
	// 18446744073709551615, not n1).
	sema::IntegralKind kind;
	const bool unsigned_value = type->unqualified->kind == sema::TypeKind::Enum && sema::IntegralKindOf(type, kind) &&
		!kind.is_signed;
	if (unsigned_value && kind.bits < 64)
		bits &= (uint64_t(1) << kind.bits) - 1;
	return abi_graph_.literal(abi_type(type), static_cast<long long>(bits), unsigned_value);
}

AbiNode ProgramLowering::type_unqualified_name(const sema::Entity* entity)
{
	// A class's or enumeration's <unqualified-name>: its name, or the
	// typedef-name the analysis named it by for linkage purposes (7.1.3p9).
	// An unnamed one other units name (a member of a class with linkage, a
	// local one of an inline function or template specialization) is an
	// <unnamed-type-name> (Itanium 5.1.5.5): Ut_ for its class's or
	// function's first unnamed type in declaration order and Ut<n-2>_ for
	// its nth, so it is named alike in every unit. Any other has no name
	// another unit uses (3.5p8, 5.1.5.5 leaves it unspecified): Clang's
	// `$_<n>` (see anonymous_name).
	if (entity->name != nullptr)
		return abi_graph_.string(entity->name->spelling, std::strlen(entity->name->spelling));
	// A closure type's is its <closure-type-name> (5.1.8): its function call
	// operator's parameter types and its number among its context's closure
	// types of those types; one 5.1.8 does not number, `$_<n>`.
	const sema::FunctionEntity* lambda = entity->kind == sema::EntityKind::Class
		? closure_call(static_cast<const sema::ClassEntity*>(entity))
		: nullptr;
	if (lambda != nullptr)
	{
		const sema::ClassEntity* closure = static_cast<const sema::ClassEntity*>(entity);
		if (!numbered_closure(closure))
			return anonymous_name(closure);
		const sema::Type* call = lambda->type->canonical;
		const size_t base = abi_scratch_.size();
		for (uint32_t i = 0; i < call->param_count; ++i)
		{
			const AbiNode parameter = abi_type(call->params[i]);
			abi_scratch_.push_back(parameter);
		}
		// Objects number one of a member-specification in its own context
		// (closure_context_scope), and write a variadic lambda's ellipsis
		// (z), numbering its closure apart (Analyzer::number_closure); the
		// course's LowIR names neither.
		const sema::ClosureFacts* facts = object_mode_ ? graph_->closure_facts(closure) : nullptr;
		const uint32_t number = facts == nullptr ? closure->local_discriminator
			: facts->context != nullptr          ? facts->context_number
												 : facts->closure_number;
		const AbiNode name = abi_graph_.closure_name(
			abi_scratch_.data() + base, call->param_count, number, facts != nullptr && call->variadic);
		abi_scratch_.resize(base);
		return name;
	}
	const sema::Scope* owner = entity->owner;
	if (owner != nullptr && owner->kind == sema::ScopeKind::Class)
		return visible_type(owner->entity) ? abi_graph_.unnamed_type_name(unnamed_ordinal(entity))
										   : anonymous_name(entity);
	// An unnamed local type: Ut<n>_ by its number among its function's
	// (numbered as GCC and Clang count them) where other units name it.
	const uint32_t local = graph_->unnamed_local_ordinal(entity);
	if (local != PointerMap::kMissing && visible_type(entity))
		return abi_graph_.unnamed_type_name(local);
	return anonymous_name(entity);
}

AbiNode ProgramLowering::closure_context_scope(const sema::ClassEntity* closure, AbiNode scope)
{
	// Itanium 5.1.8's contexts of a closure of a class's member-
	// specification, as GCC and Clang name it in objects: a default member
	// initializer's is <class> <member> M, a member function's default
	// argument's the local scope Z <function encoding> E d [n] _ (n the
	// parameter's number from the last less one, omitted for the last; a
	// constructor's complete-object encoding, as Clang names it). LowIR
	// keeps the course's name in the class.
	const sema::ClosureFacts* facts = graph_->closure_facts(closure);
	if (facts == nullptr || facts->context == nullptr)
		return scope;
	if (facts->context->kind == sema::EntityKind::Variable)
	{
		const char* spelling = facts->context->name->spelling;
		return abi_graph_.data_member_prefix(scope, abi_graph_.string(spelling, std::strlen(spelling)));
	}
	const sema::FunctionEntity* function = static_cast<const sema::FunctionEntity*>(facts->context);
	return abi_graph_.function_scope(function_encoding(function, EntryVariant::Complete),
		function->type->canonical->param_count - facts->default_argument);
}

uint32_t ProgramLowering::unnamed_ordinal(const sema::Entity* entity)
{
	// An unnamed member type's position among its class's unnamed classes
	// and enumerations in declaration order (anonymous unions count, types
	// with a typedef-name for linkage purposes do not, as in GCC and
	// Clang): numbered for the whole class by one walk of its declarations.
	uint32_t ordinal = unnamed_ordinals_.find(entity);
	if (ordinal != PointerMap::kMissing)
		return ordinal;
	uint32_t next = 0;
	for (const sema::Binding* binding = entity->owner->bindings; binding != nullptr; binding = binding->next)
	{
		const sema::Entity* type = binding->entity;
		if (binding->kind != sema::BindingKind::Type || type == nullptr || type->name != nullptr ||
			type->owner != entity->owner ||
			(type->kind != sema::EntityKind::Class && type->kind != sema::EntityKind::Enum) ||
			unnamed_ordinals_.find(type) != PointerMap::kMissing)
			continue;
		unnamed_ordinals_.set(type, next++);
	}
	ordinal = unnamed_ordinals_.find(entity);
	if (ordinal == PointerMap::kMissing)
		throw LoweringError(0, "an unnamed member type missing from its class's declarations");
	return ordinal;
}

AbiNode ProgramLowering::class_name(const sema::ClassEntity* entity)
{
	// A class's name in its scope, built once per unit; a class template
	// specialization's is the template-id of its template's name and
	// arguments (5.1.5), with 5.1.10's abbreviations of ::std's allocator,
	// basic_string and char string and stream types. 5.1.7: a local class after the first of its name
	// in its function takes a discriminator, as a type (a template
	// argument, a parameter type, the type of its run-time data) as well
	// as a scope, so the type of its member's parameter is the
	// substitution of the member's prefix. Not inside a bound pack's
	// pattern, nor one naming such a pattern (see abi_type).
	const uint32_t known = abi_bound_pattern_ ? PointerMap::kMissing : class_names_.find(entity);
	if (known != PointerMap::kMissing)
		return known;
	const bool saved = abi_level_dependent_;
	abi_level_dependent_ = false;
	const AbiNode name = class_name_node(entity);
	if (!abi_bound_pattern_ && !abi_level_dependent_)
		class_names_.set(entity, name);
	abi_level_dependent_ = abi_level_dependent_ || saved;
	return name;
}

AbiNode ProgramLowering::class_name_node(const sema::ClassEntity* entity)
{
	const sema::ClassTemplateFacts* facts = entity->templ;
	// A dependent template-name's specialization (TT<A...>) is the
	// template-id of that name.
	if (facts != nullptr && facts->role == sema::TemplateRole::Specialization &&
		facts->pattern->templ->template_name != nullptr)
		return name_template_id(facts->pattern->templ->template_name->type, facts->arguments, facts->argument_count);
	// A closure type's number is its name's own (5.1.8), no discriminator.
	// A specialization's ABI tags are its template's (whose tagged name is
	// the substitution candidate, an explicit specialization's own unused:
	// GCC and Clang alike), an instance's member class's its pattern's.
	const void* tagged = facts != nullptr && facts->pattern != nullptr &&
			(facts->role == sema::TemplateRole::Specialization || facts->role == sema::TemplateRole::MemberInstance)
		? static_cast<const void*>(facts->pattern)
		: entity;
	const AbiNode scope = object_mode_ ? closure_context_scope(entity, abi_scope(entity->owner))
									   : abi_scope(entity->owner);
	AbiNode name = tagged_name(scope, type_unqualified_name(entity), tagged,
		closure_call(entity) != nullptr ? 0 : local_discriminator(entity, entity->local_discriminator), false);
	// A class template itself (the current instantiation, as a template
	// signature names it) is the template-id of its own parameters.
	const bool specialization = facts != nullptr && facts->role == sema::TemplateRole::Specialization;
	if (specialization || (facts != nullptr && facts->role == sema::TemplateRole::Template && facts->head != nullptr))
	{
		const size_t base = abi_scratch_.size();
		const uint32_t count = specialization ? facts->argument_count : facts->head->count;
		for (uint32_t i = 0; i < count; ++i)
		{
			const AbiNode argument = abi_type(specialization ? facts->arguments[i] : facts->head->parameters[i]->type);
			abi_scratch_.push_back(argument);
		}
		name = abi_graph_.abbreviated_template_id(name, abi_scratch_.data() + base, count);
		abi_scratch_.resize(base);
	}
	return name;
}

AbiNode ProgramLowering::name_template_id(const sema::Type* name, const sema::Type* const* arguments, uint32_t count)
{
	// A dependent template-name's template-id: its arguments as written but
	// for those of a template template-parameter's pack, an argument pack
	// as its own parameters take them (J <template-arg>* E, 5.1.5.10:
	// `TT<int>` of `template<class...> class TT` is T_IJiEE in Clang and
	// g++, TT<int, long> of `template<class, class...> class TT` T_IiJlEE).
	// An expansion before the pack's position leaves the positions unknown:
	// all as written.
	const AbiNode templ = abi_type(name);
	const sema::TemplateParameterEntity* parameter = name->unqualified->kind ==
			sema::TypeKind::TemplateTemplateParameter
		? static_cast<const sema::TemplateParameterEntity*>(name->unqualified->entity)
		: nullptr;
	const sema::TemplateHead* head = parameter != nullptr && parameter->qualifier == nullptr ? parameter->template_head
																							 : nullptr;
	uint32_t pack = 0;
	while (head != nullptr && pack < head->count && !head->parameters[pack]->pack)
		++pack;
	bool packed = head != nullptr && pack < head->count && pack <= count;
	for (uint32_t i = 0; packed && i < pack; ++i)
		packed = arguments[i]->kind != sema::TypeKind::Expansion;
	const size_t base = abi_scratch_.size();
	for (uint32_t i = 0; i < count; ++i)
	{
		const AbiNode argument = abi_type(arguments[i]);
		abi_scratch_.push_back(argument);
	}
	if (packed)
	{
		const size_t first = base + pack;
		const AbiNode elements = abi_graph_.argument_pack(abi_scratch_.data() + first, abi_scratch_.size() - first);
		abi_scratch_.resize(first);
		abi_scratch_.push_back(elements);
	}
	const AbiNode id = abi_graph_.template_id(templ, abi_scratch_.data() + base, abi_scratch_.size() - base);
	abi_scratch_.resize(base);
	return id;
}

std::string ProgramLowering::function_object(const sema::FunctionEntity* function, EntryVariant variant)
{
	if (function->builtin == sema::BuiltinFunction::Memcpy)
		return "cppgm_builtin_memcpy";
	if (function->builtin == sema::BuiltinFunction::Memmove)
		return "cppgm_builtin_memmove";
	if (function->builtin == sema::BuiltinFunction::Strlen)
		return "cppgm_builtin_strlen";
	if (function->builtin == sema::BuiltinFunction::Abort)
		return "abort";
	if (function->builtin == sema::BuiltinFunction::Library)
		return LibraryName(function);
	if (const char* label = graph_->asm_label(function))
		return label;
	// The replaceable allocation functions are the runtime's (a program's
	// definition replaces the runtime's under the same object name).
	switch (function->allocation)
	{
	case sema::AllocationRole::New:
		return "cppgm_builtin_operator_new";
	case sema::AllocationRole::NewArray:
		return "cppgm_builtin_operator_new_array";
	case sema::AllocationRole::Delete:
		return "cppgm_builtin_operator_delete";
	case sema::AllocationRole::DeleteArray:
		return "cppgm_builtin_operator_delete_array";
	case sema::AllocationRole::None:
		break;
	}
	if (function->builtin != sema::BuiltinFunction::None || function->c_linkage || IsMain(function))
		return function->name->spelling;
	abi_mangle::AbiTarget target;
	target.kind = abi_mangle::ABI_TARGET_ENTITY;
	target.node = function_encoding(function, variant);
	std::string out;
	encoder_->encode(target, out);
	++counters_.mangled_names;
	return out;
}

AbiNode ProgramLowering::function_encoding(const sema::FunctionEntity* function, EntryVariant variant)
{
	// 5.1.2: _Z <name> <bare-function-type> over the canonical (adjusted)
	// parameter types; a non-template function's result is not encoded. A
	// function template specialization encodes its template's signature
	// (template parameters as T_) with its result, and its name is the
	// template-id of its arguments (5.1.5).
	// An entity of C linkage is not mangled (5.1.2): its encoding, as a
	// template argument or an expression names it, is its source name
	// (5.1.6.2: `CB<IsEmpty>` is 2CBIL_Z7IsEmptyEE).
	if (function->c_linkage || IsMain(function))
		return abi_graph_.name(
			abi_mangle::ABI_NONE, abi_graph_.string(function->name->spelling, std::strlen(function->name->spelling)));
	// An inheriting constructor's object name is Itanium's CI1/CI2 of the
	// class that declared the constructor it inherits, through a base's
	// inheriting constructors (5.1.4.3; GCC and Clang alike; the root sema
	// recorded), a constructor template's specialization's with its
	// template arguments and signature. A form without trailing default
	// arguments (C++11 12.9p1; the hosts implement P0136, which has none,
	// the caller passing the defaults to the full form) keeps the course's
	// name of its own signature, not the full form's. LowIR keeps the
	// course's C1/C2.
	const sema::FunctionEntity* inherited = object_mode_ && function->inherits_full ? graph_->inherited_root(function)
																					: nullptr;
	const AbiNode inherited_base = inherited != nullptr ? abi_type(inherited->owner->entity->type)
														: abi_mangle::ABI_NONE;
	const sema::FunctionEntity* specialization = inherited != nullptr && inherited->primary != nullptr &&
			sema::ArgumentsOf(inherited) != nullptr
		? inherited
		: function;
	const sema::FunctionEntity* primary = specialization->primary != nullptr &&
			sema::ArgumentsOf(specialization) != nullptr
		? specialization->primary
		: nullptr;
	const sema::Type* type = primary != nullptr ? primary->type->canonical : function->type->canonical;
	// A placeholder of the template's signature is its expression
	// (placeholder_expression); one the key does not mangle leaves its
	// type to the specialization's.
	const sema::Type* specialized = specialization->type->canonical;
	AbiNode unqualified = abi_mangle::ABI_NONE;
	if (function->constructor)
		unqualified = abi_graph_.ctor_dtor_name(variant == EntryVariant::Base
				? abi_mangle::ABI_TERMINAL_CONSTRUCTOR_BASE
				: abi_mangle::ABI_TERMINAL_CONSTRUCTOR_COMPLETE,
			inherited_base);
	else if (function->destructor)
		unqualified = abi_graph_.ctor_dtor_name(variant == EntryVariant::Base ? abi_mangle::ABI_TERMINAL_DESTRUCTOR_BASE
				: variant == EntryVariant::Deleting ? abi_mangle::ABI_TERMINAL_DESTRUCTOR_DELETING
													: abi_mangle::ABI_TERMINAL_DESTRUCTOR_COMPLETE);
	else if (function->conversion)
	{
		// 5.1.3: `cv <type>`, the conversion type.
		unqualified = abi_graph_.conversion_name(abi_type(type->element));
	}
	else if (function->is_operator)
	{
		// A member's implicit object is an operand (`V::operator+(long)` is
		// binary `pl`, `V::operator*()` unary `de`).
		const uint32_t operands = type->param_count + (function->member && !function->is_static ? 1 : 0);
		const abi_mangle::AbiTerminalKind kind = abi_mangle::abi_terminal_for_operands(
			OperatorTerminal(function->operator_token, function->operator_array), operands);
		if (kind == abi_mangle::ABI_TERMINAL_NONE)
			throw LoweringError(0, "an operator function outside the procedural lowering slice");
		unqualified = abi_graph_.operator_name(kind);
	}
	else if (sema::IsLiteralOperator(function))
	{
		// 5.1.3: a literal operator, `li <source-name>` of its ud-suffix.
		const char* suffix = sema::LiteralOperatorSuffix(function);
		unqualified = abi_graph_.literal_operator_name(abi_graph_.string(suffix, std::strlen(suffix)));
	}
	else
		unqualified = abi_graph_.string(function->name->spelling, std::strlen(function->name->spelling));
	// An internal name is marked (`L`) but as its own symbol (see
	// variable_object); an unnamed namespace's members are not.
	const sema::Scope* owner = LinkageScope(function->owner);
	const bool local_internal = function->internal_linkage && !NamespaceInternal(owner);
	// A function template's specialization has its template's ABI tags.
	const AbiNode scope = abi_scope(owner);
	AbiNode name = tagged_name(scope, unqualified, primary != nullptr ? primary : function, 0, local_internal);
	AbiNode result = abi_mangle::ABI_NONE;
	// The template's parameters' depth from the outermost template (a
	// member template of an instance's: below the instances' levels).
	const uint32_t saved_level = abi_own_level_;
	const bool saved_pattern = abi_bound_pattern_;
	abi_own_level_ = primary != nullptr ? sema::InstanceLevels(primary->owner) : 0;
	abi_bound_pattern_ = false;
	if (primary != nullptr)
	{
		// A list under construction is the top of abi_scratch_: its types'
		// nodes may build lists of their own (a local class's enclosing
		// function's encoding).
		const size_t base = abi_scratch_.size();
		const sema::TemplateHead* head = sema::HeadOf(primary);
		for (uint32_t i = 0; i < head->count; ++i)
		{
			const sema::Type* given = sema::ArgumentsOf(specialization)[i];
			const AbiNode argument = declared_argument(head->parameters[i], given, abi_type(given));
			abi_scratch_.push_back(argument);
		}
		name = abi_graph_.template_id(name, abi_scratch_.data() + base, abi_scratch_.size() - base);
		abi_scratch_.resize(base);
		if (!function->constructor && !function->destructor && !function->conversion)
			result = abi_type(unmangled_placeholder(type->element) ? specialized->element : type->element);
	}
	const size_t base = abi_scratch_.size();
	for (uint32_t i = 0; i < type->param_count; ++i)
	{
		const AbiNode param = abi_type(
			primary != nullptr && unmangled_placeholder(type->params[i]) ? specialized->params[i] : type->params[i]);
		abi_scratch_.push_back(param);
	}
	// 5.1.2: a member function's cv- and ref-qualifiers follow its nested
	// name's N.
	unsigned qualifiers = 0;
	if ((type->function_cv & sema::kConst) != 0)
		qualifiers |= abi_mangle::ABI_QUALIFIER_CONST;
	if ((type->function_cv & sema::kVolatile) != 0)
		qualifiers |= abi_mangle::ABI_QUALIFIER_VOLATILE;
	if (type->ref == sema::RefQualifier::LValue)
		qualifiers |= abi_mangle::ABI_QUALIFIER_LVALUE_REF;
	else if (type->ref == sema::RefQualifier::RValue)
		qualifiers |= abi_mangle::ABI_QUALIFIER_RVALUE_REF;
	AbiNode encoding = abi_graph_.function_encoding(
		name, result, abi_scratch_.data() + base, abi_scratch_.size() - base, type->variadic, qualifiers);
	abi_scratch_.resize(base);
	abi_own_level_ = saved_level;
	abi_bound_pattern_ = saved_pattern;
	// The result type's tags its encoding lacks (the specialization's
	// result, as Clang takes it).
	if (graph_->has_abi_tags())
	{
		const bool scope_encoding = variant == EntryVariant::Complete;
		if (scope_encoding)
			encodings_underway_.push_back(EncodingUnderway{ function, encoding });
		encoding = with_implicit_tags(encoding, specialized->element);
		if (scope_encoding)
			encodings_underway_.pop_back();
	}
	return encoding;
}

AbiNode ProgramLowering::declared_argument(
	const sema::TemplateParameterEntity* parameter, const sema::Type* given, AbiNode argument)
{
	// <template-arg> ::= <template-param-decl> <template-arg> (the current
	// ABI's 5.1.5.10, cxx-abi issue 47, as Clang 18 and later write it): a
	// function template may be overloaded on its template parameters'
	// declarations, so its specialization's argument declares its parameter
	// when the parameter its form implies, `typename T` for a type, `T V`
	// for a value of type T, the argument template's own head for a
	// template, would not be the parameter. The parameter is the primary
	// template's, in the terms of the encoded signature (an instance's
	// member template's has its class's arguments substituted). A type
	// parameter (unconstrained in C++11) is the implied one; a non-type one
	// whose type depends on a template parameter is not; nor is a template
	// template-parameter whose head is not the argument's. An empty pack's
	// parameter is never implied (its form is `typename...`), a non-empty
	// one's is as for its first element.
	if (!parameter->non_type && !parameter->template_template)
		return argument;
	const sema::Type* first = given;
	bool empty = false;
	if (parameter->pack && given->kind == sema::TypeKind::Pack)
	{
		empty = given->param_count == 0;
		first = empty ? nullptr : given->params[0];
	}
	if (parameter->non_type && parameter->pack && NamesBoundPack(parameter->value_type))
	{
		// A pack whose type expands a pack its instance binds (an expanded
		// pack: `Ts... Vs` of a member template of an instance of
		// `template<class... Ts>`) declares each element's type (its
		// value's), none for an empty one.
		const size_t base = abi_scratch_.size();
		for (uint32_t i = 0; given->kind == sema::TypeKind::Pack && i < given->param_count; ++i)
		{
			const sema::Type* value = given->params[i];
			if (value->kind != sema::TypeKind::Value)
				throw LoweringError(0, "an expanded template parameter pack's argument is not a value");
			const AbiNode declaration = abi_graph_.template_param_decl(
				abi_mangle::ABI_PARAM_DECL_NON_TYPE, false, abi_type(value->element));
			abi_scratch_.push_back(declaration);
		}
		const size_t count = abi_scratch_.size() - base;
		const AbiNode declared = count == 0 ? argument
											: abi_graph_.declared_argument(abi_scratch_.data() + base, count, argument);
		abi_scratch_.resize(base);
		return declared;
	}
	if (parameter->non_type && !empty && !declared_type_dependent(parameter->value_type))
		return argument;
	// One declaration per parameter: in its template's own terms, every
	// specialization's is the same.
	AbiNode declaration = parameter_declarations_.find(parameter);
	if (declaration == PointerMap::kMissing)
	{
		declaration = declaration_node(parameter);
		parameter_declarations_.set(parameter, declaration);
	}
	if (parameter->template_template && !empty)
	{
		// Heads compare as their encodings (Clang's rule): the argument's
		// own parameters at the level of the parameter's.
		const sema::Type* templ = first->unqualified;
		const sema::TemplateHead* head = nullptr;
		if (templ->kind == sema::TypeKind::Template && templ->entity->kind == sema::EntityKind::AliasTemplate)
			head = static_cast<const sema::AliasTemplateEntity*>(templ->entity)->head;
		else if (templ->kind == sema::TypeKind::Template)
			head = static_cast<const sema::ClassEntity*>(templ->entity)->templ->head;
		else if (templ->kind == sema::TypeKind::TemplateTemplateParameter)
			head = static_cast<const sema::TemplateParameterEntity*>(templ->entity)->template_head;
		if (head != nullptr && parameter->template_head != nullptr &&
			argument_head_declaration(head, parameter->pack, parameter->depth + 1u) == declaration)
			return argument;
	}
	return abi_graph_.declared_argument(&declaration, 1, argument);
}

bool ProgramLowering::declared_type_dependent(const sema::Type* type)
{
	// Whether a non-type parameter's declared type depends on a template
	// parameter. The analysis keeps every decltype of an expression in a
	// template's signature as a placeholder; one whose operand names no
	// dependent entity or type (its key's mark clear, see
	// Analyzer::placeholder_words) is that operand's type, as the hosts
	// mangle it (5.1.6), not a dependent one.
	const sema::Type* bare = type->unqualified;
	switch (bare->kind)
	{
	case sema::TypeKind::TemplateParameter:
	{
		const sema::TemplateParameterEntity* parameter = static_cast<const sema::TemplateParameterEntity*>(
			bare->entity);
		return parameter->expression == nullptr || parameter->key == nullptr ||
			parameter->key[0] <= sema::kPlaceholderKeyPrefix || (parameter->key[1] & 1) != 0;
	}
	case sema::TypeKind::Pointer:
	case sema::TypeKind::LValueReference:
	case sema::TypeKind::RValueReference:
		return declared_type_dependent(bare->element);
	case sema::TypeKind::MemberPointer:
		return declared_type_dependent(bare->owner) || declared_type_dependent(bare->element);
	case sema::TypeKind::Function:
		for (uint32_t i = 0; i < bare->param_count; ++i)
		{
			if (declared_type_dependent(bare->params[i]))
				return true;
		}
		return declared_type_dependent(bare->element);
	default:
		return analyzer_->dependent(bare);
	}
}

AbiNode ProgramLowering::declaration_node(const sema::TemplateParameterEntity* parameter)
{
	// <template-param-decl>: Ty, Tn <declared type>, Tt <head> E (Tp first
	// for a pack); a template template-parameter's own parameters are a
	// level below its template's (TL0__ for the first of a function
	// template's).
	if (parameter->template_template)
		return head_declaration(parameter->template_head, parameter->pack, 0);
	if (!parameter->non_type)
		return abi_graph_.template_param_decl(abi_mangle::ABI_PARAM_DECL_TYPE, parameter->pack, abi_mangle::ABI_NONE);
	return abi_graph_.template_param_decl(
		abi_mangle::ABI_PARAM_DECL_NON_TYPE, parameter->pack, abi_type(parameter->value_type));
}

AbiNode ProgramLowering::argument_head_declaration(const sema::TemplateHead* head, bool pack, uint32_t level)
{
	// A template argument's head as a parameter's declaration would encode
	// it, once per head (each specialization naming the template compares
	// it; a head is seen at one pack and level in practice, and another
	// replaces the record).
	const uint32_t key = level << 1 | (pack ? 1u : 0u);
	const uint32_t found = head_declaration_index_.find(head);
	if (found != PointerMap::kMissing && head_declarations_[found].key == key)
		return head_declarations_[found].node;
	const AbiNode node = head_declaration(head, pack, level);
	if (found != PointerMap::kMissing)
		head_declarations_[found] = HeadDeclaration{ key, node };
	else
	{
		head_declaration_index_.set(head, static_cast<uint32_t>(head_declarations_.size()));
		head_declarations_.push_back(HeadDeclaration{ key, node });
	}
	return node;
}

AbiNode ProgramLowering::head_declaration(const sema::TemplateHead* head, bool pack, uint32_t level)
{
	// Tt <template-param-decl>* E of a template parameter list; its
	// parameters (and the encoded template's) named in its non-type
	// parameters' types are encoded by their levels, offset by `level`
	// (see own_level).
	const uint32_t saved_level = abi_own_level_;
	const bool saved_pattern = abi_bound_pattern_;
	abi_own_level_ = level;
	abi_bound_pattern_ = true;
	const size_t base = abi_scratch_.size();
	for (uint32_t i = 0; head != nullptr && i < head->count; ++i)
	{
		const AbiNode declaration = declaration_node(head->parameters[i]);
		abi_scratch_.push_back(declaration);
	}
	const AbiNode node = abi_graph_.template_param_decl(abi_mangle::ABI_PARAM_DECL_TEMPLATE, pack, abi_mangle::ABI_NONE,
		abi_scratch_.data() + base, abi_scratch_.size() - base);
	abi_scratch_.resize(base);
	abi_own_level_ = saved_level;
	abi_bound_pattern_ = saved_pattern;
	return node;
}

std::string ProgramLowering::variable_object(const sema::VariableEntity* variable)
{
	if (const char* label = graph_->asm_label(variable))
		return label;
	// 5.1.2: a variable of the global namespace is not mangled (nor is a
	// C-linkage one); others are _Z <nested-name>. The ABI leaves names of
	// internal linkage to the implementation (local symbols no other
	// object sees): a variable's carry Clang's `L` marker as a function's
	// do (`_ZL1v`, `_ZN2nsL1vE`; `_ZL1fv`), a global-namespace variable's
	// mangled apart from any C name; one of an unnamed namespace has none
	// (`_ZN12_GLOBAL__N_11vE`).
	const sema::Scope* owner = LinkageScope(variable->owner);
	const bool global_scope = owner->parent == nullptr;
	const bool local_internal = variable->internal_linkage && !NamespaceInternal(owner);
	const sema::VariableTemplateFacts* facts = variable->templ;
	const bool specialization = facts != nullptr && facts->arguments != nullptr;
	// One with ABI tags (its own or its type's) is mangled wherever it is,
	// as GCC and Clang mangle it; so is a structured binding declaration's
	// unnamed object (5.1.4: _ZDC1a1bE).
	const bool named = variable->name != nullptr;
	if (named &&
		(variable->c_linkage || (global_scope && !local_internal && !specialization && !graph_->has_abi_tags())))
		return variable->name->spelling;
	const AbiNode name = variable_name(variable);
	if (named && !variable->c_linkage && global_scope && !local_internal && !specialization &&
		abi_graph_.list_size(name) == 0)
		return variable->name->spelling;
	abi_mangle::AbiTarget target;
	target.kind = abi_mangle::ABI_TARGET_ENTITY;
	target.node = name;
	std::string out;
	encoder_->encode(target, out);
	++counters_.mangled_names;
	return out;
}

AbiNode ProgramLowering::variable_source_name(const sema::VariableEntity* variable)
{
	// A variable's <unqualified-name>: its source name, or for a structured
	// binding declaration's unnamed object its names, DC <source-name>+ E
	// (5.1.4).
	if (variable->name != nullptr)
		return abi_graph_.string(variable->name->spelling, std::strlen(variable->name->spelling));
	const sema::Decomposition* decomposition = graph_->decomposition(variable);
	if (decomposition == nullptr)
		throw LoweringError(0, "an unnamed variable without an Itanium name");
	const size_t base = abi_scratch_.size();
	for (uint32_t i = 0; i < decomposition->count; ++i)
	{
		const char* spelling = decomposition->bindings[i]->name->spelling;
		const AbiNode name = abi_graph_.string(spelling, std::strlen(spelling));
		abi_scratch_.push_back(name);
	}
	const AbiNode name = abi_graph_.decomposition_name(abi_scratch_.data() + base, decomposition->count);
	abi_scratch_.resize(base);
	return name;
}

AbiNode ProgramLowering::variable_name(const sema::VariableEntity* variable)
{
	// A variable's <name> (5.1.2), as its mangled name and a template
	// argument designating it encode it: a C-linkage one's is its source
	// name (5.1.6.2), an internal one's is marked (see function_encoding).
	const AbiNode spelling = variable_source_name(variable);
	if (variable->c_linkage)
		return abi_graph_.name(abi_mangle::ABI_NONE, spelling);
	const sema::Scope* owner = LinkageScope(variable->owner);
	const bool local_internal = variable->internal_linkage && !NamespaceInternal(owner);
	// A variable template's specialization is the template-id of its
	// arguments (5.1.5), mangled wherever it is.
	const sema::VariableTemplateFacts* facts = variable->templ;
	const bool specialization = facts != nullptr && facts->arguments != nullptr;
	const AbiNode scope = abi_scope(owner);
	AbiNode name = tagged_name(scope, spelling,
		specialization && facts->pattern != nullptr ? static_cast<const void*>(facts->pattern) : variable, 0,
		local_internal);
	if (specialization)
	{
		const size_t base = abi_scratch_.size();
		for (uint32_t i = 0; i < facts->argument_count; ++i)
		{
			const AbiNode argument = abi_type(facts->arguments[i]);
			abi_scratch_.push_back(argument);
		}
		name = abi_graph_.template_id(name, abi_scratch_.data() + base, facts->argument_count);
		abi_scratch_.resize(base);
	}
	// Its type's tags its name lacks.
	return graph_->has_abi_tags() ? with_implicit_tags(name, variable->type) : name;
}

std::string ProgramLowering::tls_wrapper_object(const sema::VariableEntity* variable)
{
	// 5.1.4: a thread-local variable's wrapper is _ZTW <name> (its
	// initialization function _ZTH <name>).
	return special_object(abi_mangle::ABI_TARGET_TLS_WRAPPER, thread_local_name(variable));
}

AbiNode ProgramLowering::thread_local_name(const sema::VariableEntity* variable)
{
	// Its <name> as other names name it (an internal one's marked).
	return variable_name(variable);
}

std::string ProgramLowering::special_object(abi_mangle::AbiTargetKind kind, AbiNode name)
{
	// A special name of an object's name (5.1.4): `kind` prefixes it.
	abi_mangle::AbiTarget target;
	target.kind = kind;
	target.node = name;
	std::string out;
	encoder_->encode(target, out);
	++counters_.mangled_names;
	return out;
}

// ------------------------------------------------------------- metadata

lowir::SymbolMetadata ProgramLowering::function_metadata(const sema::FunctionEntity* function, lowir::SymbolId symbol)
{
	lowir::SymbolMetadata metadata;
	if (IsMain(function))
	{
		metadata.role = ir_model::SR_ENTRY;
		metadata.binding = ir_model::SBM_STRONG;
		metadata.flags = lowir::SF_KEEP_ALIAS;
		return metadata;
	}
	if (function->c_linkage &&
		(function->builtin == sema::BuiltinFunction::None || function->builtin == sema::BuiltinFunction::Library))
		metadata.linkage = ir_model::LLM_C;
	// The scalar forms own allocate_memory and free_memory (pa8/lowir.md:
	// one owner per role, and no array role). In object mode the array
	// forms are references to their own names (see
	// resolve_singleton_roles); the program's replacement of one
	// (17.6.4.6p2) owns no role either, since its definition can complete
	// before a scalar form is first named.
	const bool array_form = function->allocation == sema::AllocationRole::NewArray ||
		function->allocation == sema::AllocationRole::DeleteArray;
	if (array_form &&
		(object_mode_ ||
			(symbol < program_.symbols.size() &&
				program_.symbols[symbol].kind == lowir::SymbolKind::FunctionDefinition)))
		;
	else if (function->allocation == sema::AllocationRole::New ||
		function->allocation == sema::AllocationRole::NewArray)
		metadata.role = ir_model::SR_ALLOCATE_MEMORY;
	else if (function->allocation != sema::AllocationRole::None)
		metadata.role = ir_model::SR_FREE_MEMORY;
	// __builtin_abort is the course runtime's terminate (pa8/lowir.md:
	// whose default handler aborts); in an object it is the host C
	// library's abort, never std::terminate, which runs the program's
	// terminate handler (18.8.3.3).
	else if (function->builtin == sema::BuiltinFunction::Abort && !object_mode_)
		metadata.role = ir_model::SR_TERMINATE;
	if (function->allocation == sema::AllocationRole::NewArray ||
		function->allocation == sema::AllocationRole::DeleteArray ||
		(function->builtin == sema::BuiltinFunction::Abort && !object_mode_))
	{
		if (secondary_roles_.size() <= symbol)
			secondary_roles_.resize(std::max<size_t>(symbol + 1, secondary_roles_.size() * 2));
		secondary_roles_[symbol] = true;
	}
	// 3.5p4: a member of a class of internal linkage, or of a local class
	// (no linkage, 3.5p8), is internal to its unit.
	metadata.binding = unit_local(function)             ? ir_model::SBM_INTERNAL
		: function->is_inline || function->instantiated ? ir_model::SBM_WEAK
														: ir_model::SBM_STRONG;
	// GNU weak (the analysis allows it on entities with external linkage):
	// replaceable, an inline function's too (lowir.md `replaceable=yes`).
	const sema::SemanticGraph::SymbolAttributes* attributes = graph_->symbol_attributes(function);
	if (attributes != nullptr && attributes->weak && metadata.binding != ir_model::SBM_INTERNAL)
	{
		metadata.binding = ir_model::SBM_WEAK;
		metadata.flags |= lowir::SF_REPLACEABLE;
	}
	if (function->owner != nullptr && function->owner->kind == sema::ScopeKind::Class &&
		entity_locality(function->owner->entity) == Locality::Local)
		metadata.flags |= lowir::SF_PREFER_LOCAL;
	// The object spelling, unless the symbol is named by it (an external C
	// function).
	const lowir::NameId object = symbol_objects_[symbol];
	if (object != 0 && object != program_.symbols[symbol].name)
		metadata.object = object;
	if (function->is_inline)
		metadata.flags |= lowir::SF_INLINE_HINT;
	if (function->no_inline)
		metadata.flags |= lowir::SF_NO_INLINE;
	if (function->used)
		metadata.flags |= lowir::SF_OBJECT_ROOT;
	// A captureless closure's function with its operator's effect is an
	// object root (the course's form).
	const sema::ClosureFacts* closure = function->owner != nullptr && function->owner->kind == sema::ScopeKind::Class
		? graph_->closure_facts(sema::ClassOf(function->owner->entity->type))
		: nullptr;
	if (closure != nullptr && closure->invoker == function)
		metadata.flags |= lowir::SF_OBJECT_ROOT;
	return metadata;
}

lowir::SymbolMetadata ProgramLowering::variable_metadata(const sema::VariableEntity* variable, lowir::SymbolId symbol)
{
	lowir::SymbolMetadata metadata;
	if (variable->c_linkage)
		metadata.linkage = ir_model::LLM_C;
	metadata.binding = unit_local(variable) ? ir_model::SBM_INTERNAL
		: (variable->templ != nullptr && variable->templ->pattern != nullptr) || variable->is_inline
		? ir_model::SBM_WEAK
		: ir_model::SBM_STRONG;
	if (variable->is_thread_local)
		metadata.storage = ir_model::GSM_THREAD_LOCAL;
	// GNU weak and section (pa27 README 9: section=<name> metadata).
	if (const sema::SemanticGraph::SymbolAttributes* attributes = graph_->symbol_attributes(variable))
	{
		if (attributes->weak && metadata.binding != ir_model::SBM_INTERNAL)
		{
			metadata.binding = ir_model::SBM_WEAK;
			metadata.flags |= lowir::SF_REPLACEABLE;
		}
	}
	metadata.section = variable_section(variable);
	// The object spelling, unless the symbol is named by it (an external
	// one's unmangled name); an internal one's always (a global-namespace
	// variable whose type no other unit can name keeps its name, Clang's
	// `extlam`, which the back end would otherwise mark as a LowIR name).
	const lowir::NameId object = symbol_objects_[symbol];
	if (object != 0 && (object != program_.symbols[symbol].name || metadata.binding == ir_model::SBM_INTERNAL))
		metadata.object = object;
	return metadata;
}

lowir::NameId ProgramLowering::variable_section(const sema::VariableEntity* variable)
{
	// A static-storage object's GNU section (pa27 README 9: section=<name>
	// metadata), a block-scope static's included; a class template
	// instance's static data member takes its pattern's (the section of
	// the member's declaration or out-of-class definition).
	if (graph_->symbol_attribute_list.empty())
		return 0;
	for (const sema::VariableEntity* v = variable; v != nullptr;
		v = v->templ != nullptr && v->templ->pattern != v ? v->templ->pattern : nullptr)
	{
		const sema::SemanticGraph::SymbolAttributes* attributes = graph_->symbol_attributes(v);
		if (attributes != nullptr && attributes->section != nullptr)
			return intern(attributes->section, static_cast<uint32_t>(std::strlen(attributes->section)));
	}
	return 0;
}

// ----------------------------------------------------------- signatures

bool ProgramLowering::passed_directly(const sema::ClassEntity* entity)
{
	// The course's call boundary for a class object by value: its bytes
	// (`obj<size x align>`) when its destructor is trivial and each copy
	// and move constructor is trivial or deleted, one of them not deleted
	// (12.2p1 as amended by CWG 1590, the Itanium ABI's "trivial for the
	// purposes of calls"); otherwise the address of an object the caller
	// owns (`ptr [pass=by_address]`, an indirect result for a return).
	// Decided once per class and unit from the special members' facts.
	const uint32_t known = direct_classes_.find(entity);
	if (known != PointerMap::kMissing)
		return known == 1;
	const bool direct = !entity->polymorphic && (entity->destructor == nullptr || entity->destructor->trivial) &&
		copied_trivially(entity);
	direct_classes_.set(entity, direct ? 1 : 2);
	return direct;
}

bool ProgramLowering::copied_trivially(const sema::ClassEntity* entity)
{
	// Each copy and move constructor trivial or deleted, one of them not
	// deleted. A by-value parameter of a function no call in the unit
	// reaches may be the first demand of the class's implicit members.
	analyzer_->implicit_copy_members(entity);
	bool trivial = true, usable = false;
	const auto admit = [&trivial, &usable](const sema::FunctionEntity* function) {
		if (function == nullptr || function->deleted)
			return;
		trivial = trivial && function->trivial;
		usable = true;
	};
	admit(entity->copy_constructor);
	admit(entity->move_constructor);
	for (const sema::Binding* binding = entity->scope != nullptr ? entity->scope->bindings : nullptr;
		binding != nullptr && trivial; binding = binding->next)
	{
		if (binding->kind != sema::BindingKind::Function || binding->entity == nullptr)
			continue;
		const sema::FunctionEntity* function = static_cast<const sema::FunctionEntity*>(binding->entity);
		if (function->constructor && function->copy_kind != sema::CopyKind::None)
			admit(function);
	}
	return trivial && usable;
}

namespace {

// The psABI's classes (3.2.3) while an object's fields are merged into its
// eightbytes.
enum AbiClass : uint8_t
{
	AC_NO_CLASS,
	AC_INTEGER,
	AC_SSE,
	AC_X87,
	AC_X87UP,
	AC_MEMORY,
	AC_SSEUP
};

// psABI 3.2.3 step 4(c): the class of an eightbyte holding fields of
// classes `a` and `b` (SSEUP only beside SSEUP or NO_CLASS; else as SSE).
uint8_t MergedClass(uint8_t a, uint8_t b)
{
	if (a == b || b == AC_NO_CLASS)
		return a;
	if (a == AC_NO_CLASS)
		return b;
	if (a == AC_SSEUP)
		a = AC_SSE;
	if (b == AC_SSEUP)
		b = AC_SSE;
	if (a == b)
		return a;
	if (a == AC_MEMORY || b == AC_MEMORY)
		return AC_MEMORY;
	if (a == AC_INTEGER || b == AC_INTEGER)
		return AC_INTEGER;
	if (a == AC_X87 || a == AC_X87UP || b == AC_X87 || b == AC_X87UP)
		return AC_MEMORY;
	return AC_SSE;
}

} // namespace

void ProgramLowering::classify_eightbytes(const sema::Type* type, uint64_t offset, uint8_t* classes)
{
	// The scalar fields of an object of `type` at byte `offset` of an
	// object of at most 16 bytes, merged into the classes of its two
	// eightbytes (psABI 3.2.3: bases and members at their offsets, array
	// elements each, a union's members together, an empty class nothing);
	// a field not at a multiple of its type's alignment (GNU packed) makes
	// the object MEMORY.
	const auto mark = [classes](uint64_t eightbyte, uint8_t cls) {
		if (eightbyte < 2)
			classes[eightbyte] = MergedClass(classes[eightbyte], cls);
		else
			classes[0] = AC_MEMORY;
	};
	type = type->unqualified;
	if (type->kind == sema::TypeKind::Array && type->bound == 0)
		return;
	uint64_t size = 0, alignment = 0;
	if (!analyzer_->object_layout(type, size, alignment) || (alignment != 0 && offset % alignment != 0))
	{
		classes[0] = AC_MEMORY;
		return;
	}
	switch (type->kind)
	{
	case sema::TypeKind::Fundamental:
		switch (type->fundamental)
		{
		case FundamentalType::Float:
		case FundamentalType::Double:
		case FundamentalType::Float16:
		case FundamentalType::BFloat16:
		case FundamentalType::Float32:
		case FundamentalType::Float64:
		case FundamentalType::Float32x:
			mark(offset / 8, AC_SSE);
			return;
		case FundamentalType::LongDouble:
		case FundamentalType::Float64x:
			mark(offset / 8, AC_X87);
			mark(offset / 8 + 1, AC_X87UP);
			return;
		case FundamentalType::Float128:
		case FundamentalType::GnuFloat128:
			mark(offset / 8, AC_SSE);
			mark(offset / 8 + 1, AC_SSEUP);
			return;
		case FundamentalType::Void:
			return;
		default:
			// Integers, bool, characters, std::nullptr_t (__int128 two
			// eightbytes).
			for (uint64_t eightbyte = offset / 8; eightbyte <= (offset + size - 1) / 8; ++eightbyte)
				mark(eightbyte, AC_INTEGER);
			return;
		}
	case sema::TypeKind::Enum:
	case sema::TypeKind::Pointer:
	case sema::TypeKind::BlockPointer:
	case sema::TypeKind::LValueReference:
	case sema::TypeKind::RValueReference:
	case sema::TypeKind::MemberPointer:
		// A member function pointer is two eightbytes (Itanium's pair).
		for (uint64_t eightbyte = offset / 8; eightbyte <= (offset + size - 1) / 8; ++eightbyte)
			mark(eightbyte, AC_INTEGER);
		return;
	case sema::TypeKind::Array:
	{
		uint64_t element = 0;
		if (!analyzer_->object_layout(type->element, element, alignment) || element == 0)
		{
			classes[0] = AC_MEMORY;
			return;
		}
		for (uint64_t i = 0; i < type->bound; ++i)
			classify_eightbytes(type->element, offset + i * element, classes);
		return;
	}
	case sema::TypeKind::Complex:
		// Its real and imaginary parts as two fields (psABI 3.2.3).
		classify_eightbytes(type->element, offset, classes);
		classify_eightbytes(type->element, offset + size / 2, classes);
		return;
	case sema::TypeKind::Vector:
		// As vector_classes: one floating lane MEMORY (GCC has no mode for
		// it); a 16-byte vector (__m128's) SSE and SSEUP, an 8-byte one
		// (__m64's, a Clang bool vector's too) SSE; a smaller one, its lanes
		// as an array's elements.
		if (type->bound == 1 && sema::IsFloating(type->element))
		{
			classes[0] = AC_MEMORY;
			return;
		}
		if (size == 16 || size == 8)
		{
			mark(offset / 8, AC_SSE);
			if (size == 16)
				mark(offset / 8 + 1, AC_SSEUP);
			return;
		}
		for (uint64_t eightbyte = offset / 8; eightbyte <= (offset + size - 1) / 8; ++eightbyte)
			mark(eightbyte, sema::IsFloating(type->element) ? AC_SSE : AC_INTEGER);
		return;
	case sema::TypeKind::Class:
	{
		const sema::ClassEntity* entity = static_cast<const sema::ClassEntity*>(type->entity);
		for (const sema::BaseClass* base = entity->bases; base != nullptr; base = base->next)
		{
			if (base->is_virtual)
				classes[0] = AC_MEMORY;
			else
				classify_eightbytes(base->base->type, offset + base->offset, classes);
		}
		for (const sema::DataMember* member = entity->members; member != nullptr; member = member->next)
		{
			if (!member->bit_field)
			{
				classify_eightbytes(member->variable->type, offset + member->offset / 8, classes);
				continue;
			}
			// A bit-field's bits (an unnamed zero-width one has none).
			if (member->width == 0)
				continue;
			const uint64_t first = offset * 8 + member->offset;
			for (uint64_t eightbyte = first / 64; eightbyte <= (first + member->width - 1) / 64; ++eightbyte)
				mark(eightbyte, AC_INTEGER);
		}
		return;
	}
	default:
		classes[0] = AC_MEMORY;
		return;
	}
}

uint8_t ProgramLowering::object_classes(const sema::Type* type)
{
	static_assert(static_cast<unsigned>(AC_NO_CLASS) == lowir::EB_NONE &&
			static_cast<unsigned>(AC_INTEGER) == lowir::EB_INTEGER && static_cast<unsigned>(AC_SSE) == lowir::EB_SSE,
		"merged classes below MEMORY's are eightbyte classes");
	// A member function pointer (an i128) is Itanium's pair of eightbytes,
	// 8-aligned on the stack (an __int128 is 16-aligned).
	if (object_mode_ && type->unqualified->kind == sema::TypeKind::MemberPointer &&
		type->unqualified->element->kind == sema::TypeKind::Function)
		return lowir::OC_STATED | lowir::EB_INTEGER | (lowir::EB_INTEGER << 2);
	if (!type->is_reference() && SoftFloatOf(type) != SoftFloat::None)
		return soft_float_classes(SoftFloatOf(type));
	if (!type->is_reference() && type->unqualified->kind == sema::TypeKind::Complex)
		return complex_classes(type);
	if (!type->is_reference() && type->unqualified->kind == sema::TypeKind::Vector)
		return vector_classes(type);
	if (!object_mode_)
		return course_classes(type, false);
	const sema::ClassEntity* entity = !type->is_reference() ? sema::ClassOf(type) : nullptr;
	if (entity == nullptr)
		return 0;
	const uint32_t known = object_classes_.find(entity);
	if (known != PointerMap::kMissing)
		return static_cast<uint8_t>(known);
	// psABI 3.2.3: an object over two eightbytes is MEMORY; then the merged
	// classes, MEMORY if any is, or an X87UP not after X87; an X87 pair
	// (one long double) is the class of its own.
	uint8_t result = lowir::OC_STATED | lowir::OC_MEMORY;
	uint64_t size = 0, alignment = 0;
	if (analyzer_->object_layout(type, size, alignment) && size <= 16)
	{
		uint8_t classes[2] = { AC_NO_CLASS, AC_NO_CLASS };
		classify_eightbytes(type, 0, classes);
		// An SSEUP eightbyte after anything but SSE is SSE (step 5(d)).
		if (classes[1] == AC_SSEUP && classes[0] != AC_SSE)
			classes[1] = AC_SSE;
		if (classes[0] == AC_X87 && classes[1] == AC_X87UP)
			result = lowir::OC_STATED | lowir::OC_X87;
		else if (classes[0] <= AC_SSE && (classes[1] <= AC_SSE || classes[1] == AC_SSEUP))
			result = static_cast<uint8_t>(lowir::OC_STATED | classes[0] |
				((classes[1] == AC_SSEUP ? static_cast<uint8_t>(lowir::EB_SSEUP) : classes[1]) << 2));
	}
	object_classes_.set(entity, result);
	return result;
}

uint8_t ProgramLowering::result_classes(const sema::Type* type)
{
	return object_mode_ ? object_classes(type) : course_classes(type, true);
}

namespace {

// The course's classification (ProgramLowering::course_class_quarters):
// two bits for each four bytes, then these.
const uint32_t CC_MEMORY = 0x100;
const uint32_t CC_UNMODELED = 0x200;
const uint32_t CC_KNOWN = 0x400;

// Merges a class (1 INTEGER, 2 SSE) into the quarters bytes [first, last)
// of a 16-byte object touch; INTEGER wins (psABI 3.2.3).
void MergeQuarters(uint32_t& quarters, uint64_t first, uint64_t last, uint32_t cls)
{
	for (uint64_t quarter = first / 4; quarter < 4 && quarter * 4 < last; ++quarter)
	{
		const unsigned shift = static_cast<unsigned>(quarter * 2);
		const uint32_t current = (quarters >> shift) & 3;
		const uint32_t merged = current == 1 ? 1 : current == 0 ? cls : (cls == 1 ? 1 : current);
		quarters = (quarters & ~(3u << shift)) | (merged << shift);
	}
}

} // namespace

void ProgramLowering::course_member_quarters(const sema::Type* type, uint64_t offset, uint32_t& quarters)
{
	// A member of `type` at byte `offset` (psABI 3.2.3, as the course's
	// model reads it): misaligned or past 16 bytes, MEMORY; an x87 or
	// binary128 value or a vector of more than 8 bytes, unmodeled; a
	// floating value or a vector SSE; another scalar INTEGER.
	type = type->unqualified;
	uint64_t size = 0, alignment = 0;
	if (!analyzer_->object_layout(type, size, alignment))
	{
		quarters |= CC_MEMORY;
		return;
	}
	if (size == 0)
		return;
	if (offset + size > 16 || (alignment != 0 && offset % alignment != 0))
	{
		quarters |= CC_MEMORY;
		return;
	}
	uint32_t cls = 1;
	switch (type->kind)
	{
	case sema::TypeKind::Fundamental:
		switch (type->fundamental)
		{
		case FundamentalType::Float:
		case FundamentalType::Double:
		case FundamentalType::Float16:
		case FundamentalType::BFloat16:
		case FundamentalType::Float32:
		case FundamentalType::Float32x:
		case FundamentalType::Float64:
			cls = 2;
			break;
		case FundamentalType::LongDouble:
		case FundamentalType::Float64x:
		case FundamentalType::Float128:
		case FundamentalType::GnuFloat128:
			quarters |= CC_UNMODELED;
			return;
		default:
			break;
		}
		break;
	case sema::TypeKind::Array:
	case sema::TypeKind::Complex:
	{
		uint64_t element = 0, element_alignment = 0;
		if (!analyzer_->object_layout(type->element, element, element_alignment) || element == 0)
			return;
		for (uint64_t at = 0; at < size; at += element)
			course_member_quarters(type->element, offset + at, quarters);
		return;
	}
	case sema::TypeKind::Vector:
		if (size > 8)
			quarters |= CC_UNMODELED;
		cls = 2;
		break;
	case sema::TypeKind::Class:
	{
		const uint32_t member = course_class_quarters(static_cast<const sema::ClassEntity*>(type->entity));
		quarters |= member & (CC_MEMORY | CC_UNMODELED);
		for (unsigned quarter = 0; quarter < 4; ++quarter)
		{
			const uint32_t inner = (member >> (quarter * 2)) & 3;
			if (inner != 0)
				MergeQuarters(quarters, offset + quarter * 4, offset + quarter * 4 + 4, inner);
		}
		return;
	}
	default:
		break;
	}
	MergeQuarters(quarters, offset, offset + size, cls);
}

uint32_t ProgramLowering::course_class_quarters(const sema::ClassEntity* entity)
{
	// Its bases and members merged (a bit-field's storage unit holds
	// integers); MEMORY when it is larger than 16 bytes or dynamic.
	const uint32_t known = object_classes_.find(entity);
	if (known != PointerMap::kMissing)
		return known;
	uint32_t quarters = 0;
	uint64_t size = 0, alignment = 0;
	if (!entity->complete || !analyzer_->object_layout(entity->type, size, alignment) || size > 16 || entity->dynamic)
		quarters |= CC_MEMORY;
	for (const sema::BaseClass* base = entity->bases; (quarters & CC_MEMORY) == 0 && base != nullptr; base = base->next)
		course_member_quarters(base->base->type, base->offset, quarters);
	for (const sema::DataMember* member = entity->members; (quarters & CC_MEMORY) == 0 && member != nullptr;
		member = member->next)
	{
		if (!member->bit_field)
		{
			course_member_quarters(member->variable->type, member->offset / 8, quarters);
			continue;
		}
		uint64_t unit = 0, unit_alignment = 0;
		if (member->width == 0 || !analyzer_->object_layout(member->variable->type, unit, unit_alignment) || unit == 0)
			continue;
		const uint64_t first = member->offset / (8 * unit) * unit;
		MergeQuarters(quarters, first, first + unit, 1);
	}
	if ((quarters & CC_MEMORY) != 0)
		quarters = CC_MEMORY;
	else if ((quarters & CC_UNMODELED) != 0)
		quarters = CC_UNMODELED;
	quarters |= CC_KNOWN;
	object_classes_.set(entity, quarters);
	return quarters;
}

uint8_t ProgramLowering::course_classes(const sema::Type* type, bool result)
{
	const sema::ClassEntity* entity = type->is_reference() ? nullptr : sema::ClassOf(type);
	uint64_t size = 0, alignment = 0;
	if (entity == nullptr || !entity->complete || !analyzer_->object_layout(type, size, alignment) || size == 0 ||
		size > 16)
		return 0;
	const uint32_t quarters = course_class_quarters(entity);
	if ((quarters & (CC_MEMORY | CC_UNMODELED)) != 0)
		return result ? 0 : lowir::OC_STATED | lowir::OC_MEMORY;
	// Each eightbyte's: INTEGER if a quarter is, else SSE if one is, else
	// none.
	uint8_t classes = lowir::OC_STATED;
	bool stated = false;
	for (unsigned eightbyte = 0; eightbyte < (size + 7) / 8; ++eightbyte)
	{
		const uint32_t low = (quarters >> (eightbyte * 4)) & 3, high = (quarters >> (eightbyte * 4 + 2)) & 3;
		const uint8_t cls = low == 1 || high == 1 ? lowir::EB_INTEGER
			: low == 2 || high == 2               ? lowir::EB_SSE
												  : lowir::EB_NONE;
		stated = stated || cls != lowir::EB_INTEGER;
		classes = static_cast<uint8_t>(classes | cls << (2 * eightbyte));
	}
	return stated ? classes : 0;
}

bool ProgramLowering::returned_directly(const sema::ClassEntity* entity)
{
	// A class result passed directly and at most sixteen bytes is returned
	// as its bytes; any other (a class non-trivial for the purposes of
	// calls, empty or of sixteen bytes included) through the caller's
	// destination (Itanium 3.1.3.1), and the course's one the psABI
	// classifies MEMORY (a misaligned member, lowir.md "Object ABI
	// Conventions"). The boundary is the completed class's alone: neither
	// the function nor its result's spelling changes it.
	return passed_directly(entity) && size_of(entity->type, 0) <= 16 &&
		(object_mode_ || (course_class_quarters(entity) & CC_MEMORY) == 0);
}

lowir::Parameter ProgramLowering::parameter_for(const sema::Type* type, lowir::NameId name)
{
	// 8.3.5: a reference parameter crosses as the address of the object
	// it binds (`ptr [pass=by_address]`, with the referenced complete
	// object's size); a class object by value as its bytes or its address
	// (see passed_directly).
	lowir::Parameter parameter = lowir::Parameter();
	parameter.name = name;
	uint64_t size = 0, alignment = 0;
	if (type->is_reference())
	{
		parameter.type = lowir::Type::scalar(lowir::TypeKind::Ptr);
		parameter.pass = ir_model::PPM_BY_ADDRESS;
		if (type->element->kind != sema::TypeKind::Function &&
			analyzer_->object_layout(type->element, size, alignment) && size != 0)
			parameter.object_bytes = size;
		return parameter;
	}
	if (type->unqualified->kind == sema::TypeKind::Class &&
		!passed_directly(static_cast<const sema::ClassEntity*>(type->unqualified->entity)))
	{
		parameter.type = lowir::Type::scalar(lowir::TypeKind::Ptr);
		parameter.pass = ir_model::PPM_BY_ADDRESS;
		parameter.object_bytes = size_of(type, 0);
		return parameter;
	}
	parameter.type = storage_type(type);
	parameter.classes = object_classes(type);
	// An X87 or COMPLEX_X87 parameter is passed in memory (psABI 3.2.3):
	// lowir.md's `memory`.
	if (parameter.classes == (lowir::OC_STATED | lowir::OC_X87) ||
		parameter.classes == (lowir::OC_STATED | lowir::OC_COMPLEX_X87))
		parameter.classes = lowir::OC_STATED | lowir::OC_MEMORY;
	return parameter;
}

bool ProgramLowering::indirect_result(const sema::Type* result, const sema::Type* function)
{
	// A class result of a function of type `function` that is not returned
	// as its bytes (its class alone decides, see returned_directly).
	(void)function;
	const sema::ClassEntity* entity = result->is_reference() ? nullptr : sema::ClassOf(result);
	return entity != nullptr && entity->complete && !returned_directly(entity);
}

bool ProgramLowering::indirect_result(const sema::FunctionEntity* function)
{
	// As for its type.
	return indirect_result(function->type->canonical->element, function->type);
}

const sema::FunctionEntity* ProgramLowering::closure_call(const sema::ClassEntity* entity) const
{
	// A closure type's function call operator (null for another class).
	return graph_->closure_call(entity);
}

void ProgramLowering::push_result_parameter(const sema::Type* result, bool named, lowir::Signature& signature)
{
	// The caller's destination of an indirect class result, first.
	lowir::Parameter destination = lowir::Parameter();
	destination.name = named ? ret_name_ : 0;
	destination.type = lowir::Type::scalar(lowir::TypeKind::Ptr);
	destination.pass = ir_model::PPM_INDIRECT_RESULT;
	destination.object_bytes = size_of(result, 0);
	program_.parameters.push_back(destination);
	++signature.parameter_count;
}

bool ProgramLowering::HiddenBaseParameters(const sema::FunctionEntity* function, EntryVariant variant)
{
	return variant == EntryVariant::Base && (function->constructor || function->destructor) &&
		sema::VirtualBaseCount(ClassOf(function->owner->entity->type)) != 0;
}

lowir::NameId ProgramLowering::vtt_name()
{
	if (vtt_name_ == 0)
		vtt_name_ = intern("__vtt", 5);
	return vtt_name_;
}

lowir::SignatureId ProgramLowering::function_signature(
	const sema::FunctionEntity* function, bool named, EntryVariant variant)
{
	const sema::Type* type = function->type->canonical;
	lowir::Signature signature = lowir::Signature();
	signature.first_parameter = static_cast<uint32_t>(program_.parameters.size());
	signature.parameter_count = type->param_count;
	const sema::Type* result = type->element;
	const bool indirect = indirect_result(function);
	if (indirect)
		push_result_parameter(result, named, signature);
	// 9.3.2: a non-static member function's object is its hidden first
	// parameter (after an indirect result's destination), the address of an
	// object of its class.
	if (function->member && !function->is_static)
	{
		lowir::Parameter self = lowir::Parameter();
		self.name = named ? this_name_ : 0;
		self.type = lowir::Type::scalar(lowir::TypeKind::Ptr);
		self.object_bytes = size_of(function->owner->entity->type, 0);
		program_.parameters.push_back(self);
		++signature.parameter_count;
	}
	// A copy or move constructor's destination and source are distinct
	// live objects during the construction (the README's boundary fact;
	// an assignment operator's may be the same object).
	const bool distinct = function->copy_kind == sema::CopyKind::CopyConstructor ||
		function->copy_kind == sema::CopyKind::MoveConstructor;
	if (distinct)
		program_.parameters.back().alias = ir_model::PALM_NOALIAS;
	// A base-object entry of a class with virtual bases takes its VTT next
	// (Itanium 2.6); no parameter carries a virtual base's address.
	if (HiddenBaseParameters(function, variant))
	{
		lowir::Parameter vtt = lowir::Parameter();
		vtt.name = named ? vtt_name() : 0;
		vtt.type = lowir::Type::scalar(lowir::TypeKind::Ptr);
		program_.parameters.push_back(vtt);
		++signature.parameter_count;
	}
	// A definition's parameters take the names of its function scope's
	// parameter declarations (unnamed ones print by position); an implicit
	// copy or move function's is `other`.
	const sema::Binding* binding = named && function->scope != nullptr ? function->scope->bindings : nullptr;
	const bool implicit_copy = named && function->implicit && function->copy_kind != sema::CopyKind::None &&
		type->param_count == 1;
	for (uint32_t i = 0; i < type->param_count; ++i)
	{
		while (binding != nullptr && binding->kind != sema::BindingKind::Parameter)
			binding = binding->next;
		lowir::NameId name = implicit_copy ? other_name_ : 0;
		if (binding != nullptr)
		{
			if (binding->name != nullptr)
				name = parameter_name(binding->name);
			binding = binding->next;
		}
		lowir::Parameter parameter = parameter_for(type->params[i], name);
		if ((function->builtin == sema::BuiltinFunction::Memcpy && i < 2) || (distinct && i == 0))
			parameter.alias = ir_model::PALM_NOALIAS;
		program_.parameters.push_back(parameter);
	}
	// A declaration returning a still incomplete class (only named, its
	// address taken) states no result, as the course's declarations do.
	const bool incomplete_result = result->unqualified->kind == sema::TypeKind::Class &&
		!static_cast<const sema::ClassEntity*>(result->unqualified->entity)->complete && !function->defined;
	signature.result = incomplete_result || indirect ? lowir::Type::scalar(lowir::TypeKind::Void)
		: result->is_reference()                     ? lowir::Type::scalar(lowir::TypeKind::Ptr)
													 : storage_type(result);
	signature.boundary.result_classes = incomplete_result || indirect ? 0 : result_classes(result);
	if (type->variadic)
		signature.boundary.arity = ir_model::CAM_VARIADIC;
	if (function_nothrow(function))
		signature.boundary.unwind = ir_model::CUM_NO;
	if (function->builtin == sema::BuiltinFunction::Abort)
	{
		// The C library's abort: it neither returns nor throws.
		signature.boundary.unwind = ir_model::CUM_NO;
		signature.boundary.returns = ir_model::CRM_NORETURN;
	}
	if (function->noreturn)
		signature.boundary.returns = ir_model::CRM_NORETURN;
	if (function->builtin == sema::BuiltinFunction::Strlen || function->effects == sema::FunctionEffect::ReadOnly)
		signature.boundary.effects = ir_model::CFXM_READONLY;
	// GNU const (pa28).
	if (function->effects == sema::FunctionEffect::ReadNone)
		signature.boundary.effects = ir_model::CFXM_READNONE;
	return program_.add_signature(signature);
}

lowir::SignatureId ProgramLowering::member_call_signature(const sema::Type* member_pointer, bool nothrow)
{
	member_pointer = member_pointer->canonical->unqualified;
	PointerMap& signatures = nothrow ? nothrow_call_signatures_ : call_signatures_;
	const uint32_t known = signatures.find(member_pointer);
	if (known != PointerMap::kMissing)
		return known;
	const sema::Type* type = member_pointer->element->canonical;
	lowir::Signature signature = lowir::Signature();
	signature.first_parameter = static_cast<uint32_t>(program_.parameters.size());
	signature.parameter_count = type->param_count;
	const sema::Type* result = type->element;
	const bool indirect = indirect_result(result, type);
	if (indirect)
	{
		// The destination is the boundary's first argument, named arg0.
		push_result_parameter(result, false, signature);
		program_.parameters.back().name = argument_name(0);
	}
	// The object: one of the pointer's class, whose size it states, when
	// that class has no base class; otherwise the pointer may adjust it to
	// a base subobject (5.5p6) of a class its type does not name, and it
	// states none.
	lowir::Parameter self = lowir::Parameter();
	self.name = argument_name(signature.parameter_count - type->param_count);
	self.type = lowir::Type::scalar(lowir::TypeKind::Ptr);
	const sema::ClassEntity* owner = sema::ClassOf(member_pointer->owner);
	if (owner != nullptr && owner->bases == nullptr)
		self.object_bytes = size_of(member_pointer->owner, 0);
	program_.parameters.push_back(self);
	++signature.parameter_count;
	for (uint32_t i = 0; i < type->param_count; ++i)
		program_.parameters.push_back(
			parameter_for(type->params[i], argument_name(signature.parameter_count - type->param_count + i)));
	signature.result = indirect  ? lowir::Type::scalar(lowir::TypeKind::Void)
		: result->is_reference() ? lowir::Type::scalar(lowir::TypeKind::Ptr)
								 : storage_type(result);
	signature.boundary.result_classes = indirect ? 0 : result_classes(result);
	if (type->variadic)
		signature.boundary.arity = ir_model::CAM_VARIADIC;
	if (nothrow)
		signature.boundary.unwind = ir_model::CUM_NO;
	const lowir::SignatureId id = program_.add_signature(signature);
	signatures.set(member_pointer, id);
	return id;
}

lowir::SignatureId ProgramLowering::call_signature(const sema::Type* function, bool nothrow)
{
	// An indirect call's `as (...) -> T` boundary, parameters named argN:
	// one per function type (and callee non-throwing or not, 15.4p2: the
	// pointer's or reference's declaration says), unless a reference
	// parameter's class is still incomplete (its object size may be stated
	// by a later call).
	const sema::Type* type = function->canonical;
	PointerMap& signatures = nothrow ? nothrow_call_signatures_ : call_signatures_;
	const uint32_t known = signatures.find(type);
	if (known != PointerMap::kMissing)
		return known;
	lowir::Signature signature = lowir::Signature();
	signature.first_parameter = static_cast<uint32_t>(program_.parameters.size());
	signature.parameter_count = type->param_count;
	const sema::Type* result = type->element;
	const bool indirect = indirect_result(result, type);
	if (indirect)
	{
		// The destination is the boundary's first argument, named arg0.
		push_result_parameter(result, false, signature);
		program_.parameters.back().name = argument_name(0);
	}
	bool complete = true;
	for (uint32_t i = 0; i < type->param_count; ++i)
	{
		const lowir::Parameter parameter = parameter_for(type->params[i], argument_name(indirect ? i + 1 : i));
		complete = complete &&
			(!type->params[i]->is_reference() || type->params[i]->element->kind == sema::TypeKind::Function ||
				parameter.object_bytes != 0);
		program_.parameters.push_back(parameter);
	}
	signature.result = indirect  ? lowir::Type::scalar(lowir::TypeKind::Void)
		: result->is_reference() ? lowir::Type::scalar(lowir::TypeKind::Ptr)
								 : storage_type(result);
	signature.boundary.result_classes = indirect ? 0 : result_classes(result);
	if (type->variadic)
		signature.boundary.arity = ir_model::CAM_VARIADIC;
	if (nothrow)
		signature.boundary.unwind = ir_model::CUM_NO;
	const lowir::SignatureId id = program_.add_signature(signature);
	if (complete)
		signatures.set(type, id);
	return id;
}

} // namespace lower
