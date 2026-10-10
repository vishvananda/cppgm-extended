#include "sema/pretty_function.h"

#include <cstdio>
#include <cstring>
#include <vector>

#include "lex/identifier_table.h"
#include "sema/function_records.h"
#include "sema/integral_arithmetic.h"
#include "sema/semantic_graph.h"
#include "sema/semantic_unit.h"
#include "sema/template_records.h"

namespace sema {

namespace {

void AppendType(const Type* type, std::string inner, std::string& out);
void AppendScope(const Scope* scope, std::string& out);

void AppendCv(uint8_t cv, std::string& out)
{
	if (cv & kConst)
		out += "const ";
	if (cv & kVolatile)
		out += "volatile ";
	if (cv & kAtomic)
		out += "_Atomic ";
}

// Clang's spellings of the fundamental types.
const char* FundamentalSpelling(FundamentalType type)
{
	switch (type)
	{
	case FundamentalType::ShortInt:
		return "short";
	case FundamentalType::LongInt:
		return "long";
	case FundamentalType::LongLongInt:
		return "long long";
	case FundamentalType::UnsignedShortInt:
		return "unsigned short";
	case FundamentalType::UnsignedLongInt:
		return "unsigned long";
	case FundamentalType::UnsignedLongLongInt:
		return "unsigned long long";
	case FundamentalType::NullptrT:
		return "std::nullptr_t";
	default:
		return FundamentalTypeName(type);
	}
}

void AppendParameters(const Type* function, std::string& out)
{
	// 8.3.5p5: a parameter of array or function type is a pointer in the
	// function's type.
	out += '(';
	for (uint32_t i = 0; i < function->param_count; ++i)
	{
		if (i != 0)
			out += ", ";
		const Type* parameter = function->params[i];
		if (parameter->kind == TypeKind::Array)
			AppendType(parameter->element, parameter->element->kind == TypeKind::Array ? "(*)" : "*", out);
		else if (parameter->kind == TypeKind::Function)
			AppendType(parameter, "(*)", out);
		else
			AppendType(parameter, std::string(), out);
	}
	if (function->variadic)
		out += function->param_count != 0 ? ", ..." : "...";
	out += ')';
	if (function->function_cv & kConst)
		out += " const";
	if (function->function_cv & kVolatile)
		out += " volatile";
	if (function->ref != RefQualifier::None)
		out += function->ref == RefQualifier::LValue ? " &" : " &&";
}

void AppendArgument(const Type* argument, std::string& out);

// A class's name in its scope: its template arguments after a
// specialization's.
void AppendClassName(const ClassEntity* cls, std::string& out)
{
	AppendScope(cls->owner, out);
	if (cls->name == nullptr)
	{
		out += cls->key == TokenKind::KW_UNION ? "(anonymous union)"
			: cls->key == TokenKind::KW_STRUCT ? "(anonymous struct)"
											   : "(anonymous class)";
		return;
	}
	out += cls->name->spelling;
	const ClassTemplateFacts* facts = cls->templ;
	if (facts == nullptr || facts->role != TemplateRole::Specialization)
		return;
	out += '<';
	bool first = true;
	for (uint32_t i = 0; i < facts->argument_count; ++i)
	{
		const Type* argument = facts->arguments[i];
		// A pack's elements are arguments of the list (14.5.3p1).
		const uint32_t count = argument->kind == TypeKind::Pack ? argument->param_count : 1;
		for (uint32_t k = 0; k < count; ++k)
		{
			if (!first)
				out += ", ";
			first = false;
			AppendArgument(argument->kind == TypeKind::Pack ? argument->params[k] : argument, out);
		}
	}
	out += '>';
}

// The qualification of a name declared in `scope`: its enclosing named
// namespaces but inline ones, classes and functions (a local class's).
void AppendScope(const Scope* scope, std::string& out)
{
	while (scope != nullptr && scope->kind != ScopeKind::Namespace && scope->kind != ScopeKind::Class &&
		(scope->kind != ScopeKind::Function || scope->entity == nullptr || scope->entity->kind != EntityKind::Function))
		scope = scope->parent;
	if (scope == nullptr || scope->entity == nullptr)
		return;
	if (scope->kind == ScopeKind::Namespace)
	{
		const NamespaceEntity* space = static_cast<const NamespaceEntity*>(scope->entity);
		if (space->owner == nullptr)
			return;
		AppendScope(space->owner, out);
		if (space->name == nullptr)
			out += "(anonymous namespace)::";
		else if (!space->is_inline)
		{
			out += space->name->spelling;
			out += "::";
		}
		return;
	}
	if (scope->kind == ScopeKind::Class)
	{
		AppendClassName(static_cast<const ClassEntity*>(scope->entity), out);
		out += "::";
		return;
	}
	const FunctionEntity* function = static_cast<const FunctionEntity*>(scope->entity);
	AppendScope(function->owner, out);
	out += PredefinedFunctionName(function);
	AppendParameters(function->type, out);
	out += "::";
}

void AppendCharacter(uint64_t value, std::string& out)
{
	if (value >= 0x20 && value < 0x7f && value != '\'' && value != '\\')
		out += static_cast<char>(value);
	else
	{
		char buffer[24];
		std::snprintf(buffer, sizeof buffer, "\\x%llx", static_cast<unsigned long long>(value));
		out += buffer;
	}
}

// An integral or enumeration argument's value as Clang writes it: an
// enumerator's name, a character literal, true or false, a literal with
// its type's suffix or cast.
void AppendValue(const Type* type, uint64_t bits, std::string& out)
{
	if (type->kind == TypeKind::Enum)
	{
		const EnumEntity* enumeration = static_cast<const EnumEntity*>(type->entity);
		const Scope* scope = enumeration->scoped ? enumeration->scope : enumeration->owner;
		for (const Binding* binding = scope != nullptr ? scope->bindings : nullptr; binding != nullptr;
			binding = binding->next)
		{
			const Entity* entity = binding->entity;
			if (entity == nullptr || entity->kind != EntityKind::Enumerator ||
				static_cast<const EnumeratorEntity*>(entity)->enumeration != enumeration ||
				static_cast<const EnumeratorEntity*>(entity)->value.bits != bits)
				continue;
			// A scoped enumeration's enumerators are in its scope (7.2p10).
			if (enumeration->scoped)
			{
				AppendType(type, std::string(), out);
				out += "::";
			}
			else
				AppendScope(entity->owner, out);
			out += entity->name->spelling;
			return;
		}
		out += '(';
		AppendType(type, std::string(), out);
		out += ')';
		type = enumeration->underlying != nullptr ? enumeration->underlying : type;
		if (type->kind == TypeKind::Enum)
		{
			out += std::to_string(bits);
			return;
		}
	}
	const bool negative = IsSignedConstant(Constant{ bits, type }) && static_cast<int64_t>(bits) < 0;
	const std::string digits = negative ? "-" + std::to_string(0 - bits) : std::to_string(bits);
	switch (type->fundamental)
	{
	case FundamentalType::Bool:
		out += bits != 0 ? "true" : "false";
		return;
	case FundamentalType::Char:
	case FundamentalType::Char16T:
	case FundamentalType::Char32T:
	case FundamentalType::WcharT:
		out += type->fundamental == FundamentalType::Char16T ? "u'"
			: type->fundamental == FundamentalType::Char32T  ? "U'"
			: type->fundamental == FundamentalType::WcharT   ? "L'"
															 : "'";
		AppendCharacter(bits, out);
		out += '\'';
		return;
	case FundamentalType::UnsignedInt:
		out += digits + "U";
		return;
	case FundamentalType::LongInt:
		out += digits + "L";
		return;
	case FundamentalType::UnsignedLongInt:
		out += digits + "UL";
		return;
	case FundamentalType::LongLongInt:
		out += digits + "LL";
		return;
	case FundamentalType::UnsignedLongLongInt:
		out += digits + "ULL";
		return;
	case FundamentalType::Int:
		out += digits;
		return;
	default:
		out += '(';
		out += FundamentalSpelling(type->fundamental);
		out += ')';
		out += digits;
		return;
	}
}

void AppendArgument(const Type* argument, std::string& out)
{
	switch (argument->kind)
	{
	case TypeKind::Value:
		if (AddressArgument(argument))
		{
			if (argument->entity == nullptr)
			{
				out += "nullptr";
				return;
			}
			if (!argument->element->is_reference())
				out += '&';
			AppendScope(argument->entity->owner, out);
			out += argument->entity->name != nullptr ? argument->entity->name->spelling : "";
			return;
		}
		AppendValue(argument->element, argument->bound, out);
		return;
	case TypeKind::Template:
		AppendScope(argument->entity->owner, out);
		out += argument->entity->name != nullptr ? argument->entity->name->spelling : "";
		return;
	case TypeKind::Pack:
		out += '<';
		for (uint32_t i = 0; i < argument->param_count; ++i)
		{
			if (i != 0)
				out += ", ";
			AppendArgument(argument->params[i], out);
		}
		out += '>';
		return;
	default:
		AppendType(argument, std::string(), out);
		return;
	}
}

// A type as a declaration of `inner` (the declarator so far) writes it.
void AppendType(const Type* type, std::string inner, std::string& out)
{
	for (;;)
	{
		const bool wrap = !inner.empty();
		switch (type->kind)
		{
		case TypeKind::Pointer:
		case TypeKind::BlockPointer:
		case TypeKind::LValueReference:
		case TypeKind::RValueReference:
		case TypeKind::MemberPointer:
		{
			std::string prefix;
			if (type->kind == TypeKind::MemberPointer)
			{
				AppendType(type->owner, std::string(), prefix);
				prefix += "::";
			}
			prefix += type->kind == TypeKind::Pointer || type->kind == TypeKind::MemberPointer ? "*"
				: type->kind == TypeKind::BlockPointer                                         ? "^"
				: type->kind == TypeKind::LValueReference                                      ? "&"
																							   : "&&";
			if (type->cv != 0)
			{
				std::string cv;
				AppendCv(type->cv, cv);
				cv.resize(cv.size() - 1);
				prefix += cv;
				if (wrap)
					prefix += ' ';
			}
			inner = prefix + inner;
			const TypeKind next = type->element->kind;
			if (next == TypeKind::Array || next == TypeKind::Function)
				inner = "(" + inner + ")";
			type = type->element;
			continue;
		}
		case TypeKind::Complex:
			AppendCv(type->cv, out);
			out += std::string("_Complex ") + FundamentalSpelling(type->element->fundamental);
			if (!inner.empty())
				out += " " + inner;
			return;
		case TypeKind::Vector:
			// Clang's spelling.
			AppendCv(type->cv, out);
			out += "__attribute__((__vector_size__(" + std::to_string(type->bound) + " * sizeof(" +
				FundamentalSpelling(type->element->fundamental) + ")))) " +
				FundamentalSpelling(type->element->fundamental);
			if (!inner.empty())
				out += " " + inner;
			return;
		case TypeKind::Array:
			inner += type->bound != 0 ? "[" + std::to_string(type->bound) + "]" : "[]";
			type = type->element;
			continue;
		case TypeKind::Function:
		{
			std::string parameters;
			AppendParameters(type, parameters);
			inner += parameters;
			type = type->element;
			continue;
		}
		default:
			break;
		}
		AppendCv(type->cv, out);
		switch (type->kind)
		{
		case TypeKind::Fundamental:
			out += FundamentalSpelling(type->fundamental);
			break;
		case TypeKind::Class:
			AppendClassName(static_cast<const ClassEntity*>(type->entity), out);
			break;
		case TypeKind::Enum:
			AppendScope(type->entity->owner, out);
			out += type->entity->name != nullptr ? type->entity->name->spelling : "(anonymous enum)";
			break;
		case TypeKind::Expansion:
			AppendType(type->element, std::string(), out);
			out += "...";
			break;
		case TypeKind::BoundPack:
		case TypeKind::Value:
		case TypeKind::Pack:
		case TypeKind::Template:
			AppendArgument(type->kind == TypeKind::BoundPack ? type->element : type, out);
			break;
		default:
			// A template parameter (a dependent signature's).
			out += type->entity != nullptr && type->entity->name != nullptr ? type->entity->name->spelling : "auto";
			break;
		}
		if (wrap)
			out += (inner[0] == '[' ? "" : " ") + inner;
		return;
	}
}

// The template parameters a specialization binds, with its arguments.
void AppendBindings(const TemplateHead* head, const Type* const* arguments, uint32_t count, std::string& out)
{
	for (uint32_t i = 0; head != nullptr && arguments != nullptr && i < head->count && i < count; ++i)
	{
		const TemplateParameterEntity* parameter = head->parameters[i];
		if (parameter == nullptr || parameter->name == nullptr)
			continue;
		out += out.empty() ? "" : ", ";
		out += parameter->name->spelling;
		out += " = ";
		AppendArgument(arguments[i], out);
	}
}

} // namespace

std::string PredefinedFunctionName(const FunctionEntity* function)
{
	// `operator T` (12.3.2) names its result type; an allocation or
	// deallocation function's keyword is a word of its own (the label runs
	// the tokens together, as the course dump spells them).
	const char* label = function->label;
	if (function->conversion)
	{
		std::string name = "operator ";
		AppendType(function->type->element, std::string(), name);
		return name;
	}
	if (std::strncmp(label, "operator", 8) == 0 && label[8] >= 'a' && label[8] <= 'z')
		return std::string("operator ") + (label + 8);
	return label;
}

std::string PrettyFunctionName(const FunctionEntity* function, const SemanticGraph& graph)
{
	std::string text;
	if (function->is_static && function->member)
		text += "static ";
	if (function->is_virtual)
		text += "virtual ";
	std::string declarator;
	AppendScope(function->owner, declarator);
	declarator += PredefinedFunctionName(function);
	AppendParameters(function->type, declarator);
	// A placeholder result is written as declared, as is a lambda's
	// function call operator's (Clang's form, whatever it returns).
	const Scope* owner = function->owner;
	const bool lambda = owner != nullptr && owner->kind == ScopeKind::Class && owner->entity != nullptr &&
		graph.closure_call(static_cast<const ClassEntity*>(owner->entity)) == function;
	if (function->constructor || function->destructor)
		text += declarator;
	else if (function->placeholder_result || lambda)
		text += "auto " + declarator;
	else
		AppendType(function->type->element, declarator, text);
	// The enclosing class specializations' parameters, outermost first,
	// then a function template specialization's own.
	std::vector<const ClassEntity*> classes;
	for (const Scope* scope = function->owner; scope != nullptr; scope = scope->parent)
	{
		if (scope->kind != ScopeKind::Class || scope->entity == nullptr)
			continue;
		const ClassEntity* cls = static_cast<const ClassEntity*>(scope->entity);
		if (cls->templ != nullptr && cls->templ->role == TemplateRole::Specialization &&
			cls->templ->pattern != nullptr && cls->templ->pattern->templ != nullptr &&
			!cls->templ->explicit_specialization)
			classes.push_back(cls);
	}
	std::string bindings;
	for (size_t i = classes.size(); i-- > 0;)
	{
		const ClassTemplateFacts* facts = classes[i]->templ;
		AppendBindings(facts->pattern->templ->head, facts->arguments, facts->argument_count, bindings);
	}
	if (function->primary != nullptr && !function->explicit_specialization && ArgumentsOf(function) != nullptr &&
		HeadOf(function->primary) != nullptr)
	{
		const TemplateHead* head = HeadOf(function->primary);
		AppendBindings(head, ArgumentsOf(function), head->count, bindings);
	}
	if (!bindings.empty())
		text += " [" + bindings + "]";
	return text;
}

} // namespace sema
