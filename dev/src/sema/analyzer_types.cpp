#include "sema/analyzer.h"

#include "lex/identifier_table.h"

namespace sema {

namespace {

// A name of one unqualified identifier without template arguments that is
// `key` (or one of `keys`): GCC's `__int128` spelled as such (a type name
// here), which combines with `signed` and `unsigned` as its keyword does
// there (the typedef names __int128_t and __uint128_t do not), and GNU's
// complex type specifier (`_Complex`, a type name here combining with the
// type keywords).
bool IsBareName(const syntax::Name& name, const Identifier* const* keys, uint32_t count)
{
	const syntax::NameComponent& last = name.last();
	if (name.global || name.components.size != 1 || last.kind != syntax::NameComponentKind::Identifier ||
		last.has_arguments)
		return false;
	for (uint32_t i = 0; i < count; ++i)
	{
		if (last.identifier == keys[i])
			return true;
	}
	return false;
}

// A function suffix holding a virt-specifier (override, final).
bool HasVirtSpecifier(const syntax::DeclaratorSuffix& suffix)
{
	if (suffix.kind != syntax::DeclaratorSuffixKind::Function)
		return false;
	for (const syntax::FunctionQualifier& qualifier : suffix.qualifiers)
	{
		if (qualifier.kind == syntax::FunctionQualifierKind::Virt)
			return true;
	}
	return false;
}

uint8_t CvBits(const syntax::Slice<TokenKind>& cv)
{
	uint8_t bits = 0;
	for (TokenKind kind : cv)
		bits |= kind == TokenKind::KW_CONST ? kConst : kVolatile;
	return bits;
}

bool IsTypeKeyword(TokenKind kind)
{
	switch (kind)
	{
	case TokenKind::KW_VOID:
	case TokenKind::KW_BOOL:
	case TokenKind::KW_CHAR:
	case TokenKind::KW_CHAR16_T:
	case TokenKind::KW_CHAR32_T:
	case TokenKind::KW_WCHAR_T:
	case TokenKind::KW_SHORT:
	case TokenKind::KW_INT:
	case TokenKind::KW_LONG:
	case TokenKind::KW_SIGNED:
	case TokenKind::KW_UNSIGNED:
	case TokenKind::KW_FLOAT:
	case TokenKind::KW_DOUBLE:
		return true;
	default:
		return false;
	}
}

// The declarator-id of a declarator, looking through parentheses.
const syntax::Name* DeclaratorId(const syntax::Declarator* declarator)
{
	for (; declarator != nullptr; declarator = declarator->nested)
	{
		if (declarator->name != nullptr)
			return declarator->name;
	}
	return nullptr;
}

// A parameter declaration whose type names a decltype-specifier.
bool SpecifiersNameValues(const syntax::Slice<syntax::DeclSpecifier>& specifiers, uint32_t level);

// A name whose template arguments hold an expression (an unevaluated
// operand in it may name an earlier parameter, 3.3.4p1), or a type-id
// that does.
bool NameHoldsValues(const syntax::Name& name, uint32_t level)
{
	if (level > 32)
		return true;
	for (const syntax::NameComponent& component : name.components)
	{
		if (component.kind == syntax::NameComponentKind::Decltype)
			return true;
		for (const syntax::TemplateArgument& argument : component.arguments)
		{
			if (argument.expression != nullptr ||
				(argument.type != nullptr && SpecifiersNameValues(argument.type->specifiers, level + 1)))
				return true;
		}
	}
	return false;
}

bool SpecifiersNameValues(const syntax::Slice<syntax::DeclSpecifier>& specifiers, uint32_t level)
{
	for (const syntax::DeclSpecifier& specifier : specifiers)
	{
		if (specifier.kind == syntax::DeclSpecKind::Decltype ||
			(specifier.kind == syntax::DeclSpecKind::Name && NameHoldsValues(*specifier.name, level)))
			return true;
	}
	return false;
}

// Whether a parameter's type may name an earlier parameter: a
// decltype-specifier, or a template argument expression, in its
// decl-specifiers, or an array bound other than a literal in its
// declarator (an unevaluated operand there may name one, 3.3.4p1).
bool UsesDecltype(const syntax::ParameterDeclaration& parameter)
{
	for (const syntax::Declarator* declarator = parameter.declarator; declarator != nullptr;
		declarator = declarator->nested)
	{
		for (const syntax::DeclaratorSuffix& suffix : declarator->suffixes)
		{
			if (suffix.kind == syntax::DeclaratorSuffixKind::Array && suffix.bound != nullptr &&
				suffix.bound->kind != syntax::ExprKind::Literal)
				return true;
		}
	}
	return SpecifiersNameValues(parameter.specifiers, 0);
}

// A class template's name found other than as its injected-class-name
// (declared in its own scope, 14.6.1p1): no class-name without a template
// argument list.
bool ClassTemplateNameAlone(const Binding* binding)
{
	if (binding->kind != BindingKind::Type || binding->entity == nullptr || binding->entity->kind != EntityKind::Class)
		return false;
	const ClassEntity* cls = static_cast<const ClassEntity*>(binding->entity);
	return cls->templ != nullptr && cls->templ->role == TemplateRole::Template && binding->declared_in != cls->scope;
}

} // namespace

// ------------------------------------------------------------- specifiers

void Analyzer::specifiers(
	const syntax::Slice<syntax::DeclSpecifier>& list, Scope* scope, Specifiers& out, bool lone_declaration)
{
	out = Specifiers();
	uint8_t cv = 0;
	bool keywords = false;
	out.location = list.empty() ? 0 : list[0].location;
	out.uses = names_.dependent_uses();
	for (const syntax::DeclSpecifier& specifier : list)
	{
		if (specifier.kind == syntax::DeclSpecKind::Constinit)
		{
			if (out.is_constinit)
				error(specifier.location, "__constinit repeated in a decl-specifier-seq");
			out.is_constinit = true;
			continue;
		}
		if (specifier.kind != syntax::DeclSpecKind::Keyword)
			continue;
		switch (specifier.keyword)
		{
		case TokenKind::KW_TYPEDEF:
			out.is_typedef = true;
			break;
		case TokenKind::KW_FRIEND:
			out.is_friend = true;
			break;
		case TokenKind::KW_EXTERN:
			out.is_extern = true;
			break;
		case TokenKind::KW_STATIC:
			out.is_static = true;
			break;
		case TokenKind::KW_CONSTEXPR:
			out.is_constexpr = true;
			break;
		case TokenKind::KW_INLINE:
			out.is_inline = true;
			break;
		case TokenKind::KW_VIRTUAL:
			out.is_virtual = true;
			break;
		case TokenKind::KW_THREAD_LOCAL:
			out.is_thread_local = true;
			break;
		case TokenKind::KW_MUTABLE:
			out.is_mutable = true;
			break;
		case TokenKind::KW_EXPLICIT:
			// C++20 [dcl.spec]p2: one explicit-specifier; a repeated
			// conditional one would leave the condition undecided (both
			// host compilers reject it; a repeated plain `explicit` is GNU-
			// rejected, Clang-accepted, and kept).
			if (out.is_explicit && (out.explicit_condition != nullptr || specifier.operand != nullptr))
				error(specifier.location, "an explicit-specifier repeated in a decl-specifier-seq");
			out.is_explicit = true;
			out.explicit_condition = specifier.operand;
			break;
		case TokenKind::KW_AUTO:
			out.auto_type = true;
			break;
		case TokenKind::KW_CONST:
		case TokenKind::KW_VOLATILE:
		{
			// 7.1.6p2: const and volatile each combine with any type
			// specifier but itself.
			const uint8_t bit = specifier.keyword == TokenKind::KW_CONST ? kConst : kVolatile;
			if ((cv & bit) != 0)
				error(specifier.location, "a cv-qualifier repeated in a decl-specifier-seq (7.1.6p2)");
			cv |= bit;
			break;
		}
		default:
			keywords = keywords || IsTypeKeyword(specifier.keyword);
			break;
		}
	}
	if (out.is_constinit && (lone_declaration || out.is_constexpr || out.is_typedef || out.is_friend))
		error(out.location, "__constinit requires a variable declaration without constexpr, typedef or friend");
	const Type* type = nullptr;
	bool int128 = false, complex = false;
	for (const syntax::DeclSpecifier& specifier : list)
	{
		const Type* named = nullptr;
		switch (specifier.kind)
		{
		case syntax::DeclSpecKind::Keyword:
		case syntax::DeclSpecKind::Constinit:
		case syntax::DeclSpecKind::Vector:
			continue;
		case syntax::DeclSpecKind::Name:
			if (IsBareName(*specifier.name, complex_keys_, 3))
			{
				if (complex)
					error(specifier.location, "_Complex repeated in a decl-specifier-seq");
				complex = true;
				continue;
			}
			named = named_type(*specifier.name, scope);
			if (named_binding_->aligned_alias)
				out.alias_alignment = declared_alignment(named_binding_, 0);
			if (keywords && !int128 && named == types_.fundamental(FundamentalType::Int128) &&
				IsBareName(*specifier.name, &int128_key_, 1))
			{
				int128 = true;
				continue;
			}
			break;
		case syntax::DeclSpecKind::Decltype:
			named = decltype_type(*specifier.operand, scope);
			// `unsigned _BitInt ( N )`: the sign specifier is the _BitInt's.
			if (specifier.operand->kind == syntax::ExprKind::BuiltinTrait &&
				static_cast<const syntax::BuiltinTraitExpr*>(specifier.operand)->trait == GnuTrait::BitInt)
				keywords = false;
			break;
		case syntax::DeclSpecKind::Class:
			// 11.3p2: a class is not defined in a friend declaration (14.5.4p2:
			// nor a class template).
			if (out.is_friend)
				error(specifier.location, "a class defined in a friend declaration");
			out.type_specifier = &specifier;
			out.defines = true;
			out.declared = class_specifier(*specifier.class_specifier, scope, out);
			named = out.declared->type;
			break;
		case syntax::DeclSpecKind::Elaborated:
			out.type_specifier = &specifier;
			out.elaborated = true;
			out.declared = elaborated_class(specifier, scope, lone_declaration, out.is_friend, out.declared_binding);
			named = out.declared != nullptr ? out.declared->type : nullptr;
			break;
		case syntax::DeclSpecKind::Enum:
			out.type_specifier = &specifier;
			out.defines = specifier.enum_specifier->has_body;
			out.elaborated = !out.defines;
			out.declared = enum_specifier(*specifier.enum_specifier, scope, lone_declaration, out.declared_binding);
			named = out.declared != nullptr ? out.declared->type : nullptr;
			break;
		}
		// A substitution's failure (see soft_substitution_) leaves no type.
		if (failed())
		{
			out.type = nullptr;
			return;
		}
		if (type != nullptr)
			error(specifier.location, "more than one type in a declaration");
		type = named;
		if (out.type_specifier == &specifier)
			out.leading_specifiers = &specifier != &list[0];
	}
	if (keywords)
	{
		if (type != nullptr)
			error(out.location, "more than one type in a declaration");
		type = int128 ? int128_type(list, out.location) : keyword_type(list, out.location);
	}
	if (complex)
	{
		// GNU's complex types: of a floating or integral type, alone of
		// double.
		if (type == nullptr)
			type = types_.fundamental(FundamentalType::Double);
		if (type->kind != TypeKind::Fundamental || IsVoid(type) || type->fundamental == FundamentalType::Bool ||
			type->fundamental == FundamentalType::NullptrT)
			error(out.location, "_Complex of a type that is no arithmetic type");
		type = types_.complex(type);
	}
	if (out.auto_type)
	{
		if (type != nullptr)
			error(out.location, "auto with another type specifier");
		type = auto_placeholder_;
	}
	if (type == nullptr)
	{
		out.type = nullptr;
		if (cv != 0)
			error(out.location, "cv-qualifier without a type");
		return;
	}
	out.type = type == auto_placeholder_ ? type : types_.qualified(type, cv);
	out.cv = cv;
	for (const syntax::DeclSpecifier& specifier : list)
	{
		if (specifier.kind == syntax::DeclSpecKind::Vector)
			out.type = vector_type(
				out.type, *specifier.operand, specifier.keyword == TokenKind::KW_SIZEOF, scope, specifier.location);
	}
}

const Type* Analyzer::vector_type(
	const Type* type, const syntax::Expr& size, bool bytes, Scope* scope, uint32_t location)
{
	// GNU vector_size(bytes) and Clang ext_vector_type(lanes); the scalar's
	// cv-qualifiers qualify the vector. In a template, a dependent lane type
	// or size keeps the size's expression (14.6.2.1p7, as an array bound),
	// checked where it is substituted.
	if (type == nullptr)
		return nullptr;
	const Type* lane = type->unqualified;
	Constant value;
	if (template_depth_ != 0 && (dependent_type(lane) || !constant_value(size, scope, value, false)))
		return types_.qualified(types_.vector(lane, 0, dependent_decltype(size, scope), bytes), type->cv);
	if (!integral_constant(size, scope, value))
		error(location, "a vector size that is no positive integral constant");
	const Type* vector = checked_vector(lane, value.bits, bytes, location);
	return vector != nullptr ? types_.qualified(vector, type->cv) : nullptr;
}

const Type* Analyzer::checked_vector(const Type* lane, uint64_t size, bool bytes, uint32_t location)
{
	// Lanes of an integral or floating type (Clang's ext_vector_type of bool
	// too), positive and at most 1024; a byte size a power-of-two multiple of
	// the lane's size. Otherwise a substitution's failure (14.8.2p8).
	lane = lane->unqualified;
	if (lane->kind != TypeKind::Fundamental || IsVoid(lane) || lane->fundamental == FundamentalType::NullptrT ||
		(bytes && lane->fundamental == FundamentalType::Bool))
	{
		substitution_error(location, "a vector of no integral or floating type");
		return nullptr;
	}
	const uint64_t lane_size = FundamentalTypeSize(lane->fundamental);
	const uint64_t lanes = bytes ? size / lane_size : size;
	if (lanes == 0 || lanes > 1024 || (bytes && (size % lane_size != 0 || (lanes & (lanes - 1)) != 0)))
	{
		substitution_error(location, "a vector size that is no positive power-of-two multiple of its lane's size");
		return nullptr;
	}
	return types_.vector(lane, lanes);
}

bool Analyzer::vector_reinterpretation(const Type* from, const Type* to)
{
	// GCC's casts of GNU vectors (C-style, functional, reinterpret_cast): a
	// vector's bytes as another vector or an integer of their size, an
	// integer's (not a bool's) as a vector.
	from = from->unqualified;
	to = to->unqualified;
	const bool from_vector = from->kind == TypeKind::Vector;
	const bool to_vector = to->kind == TypeKind::Vector;
	if ((!from_vector && !to_vector) || (from_vector && from->owner != nullptr) ||
		(to_vector && to->owner != nullptr) ||
		(!from_vector && (!IsIntegralType(from) || from->fundamental == FundamentalType::Bool)) ||
		(!to_vector && !IsIntegralType(to)))
		return false;
	uint64_t from_size = 0;
	uint64_t to_size = 0;
	uint64_t alignment = 0;
	return object_layout(from, from_size, alignment) && object_layout(to, to_size, alignment) && from_size == to_size;
}

const Type* Analyzer::keyword_type(const syntax::Slice<syntax::DeclSpecifier>& list, uint32_t location)
{
	// 7.1.6.2 Table 10.
	unsigned signed_count = 0, unsigned_count = 0, short_count = 0, long_count = 0, int_count = 0, char_count = 0,
			 double_count = 0, other_count = 0;
	FundamentalType other = FundamentalType::Void;
	for (const syntax::DeclSpecifier& specifier : list)
	{
		if (specifier.kind != syntax::DeclSpecKind::Keyword)
			continue;
		switch (specifier.keyword)
		{
		case TokenKind::KW_SIGNED:
			++signed_count;
			break;
		case TokenKind::KW_UNSIGNED:
			++unsigned_count;
			break;
		case TokenKind::KW_SHORT:
			++short_count;
			break;
		case TokenKind::KW_LONG:
			++long_count;
			break;
		case TokenKind::KW_INT:
			++int_count;
			break;
		case TokenKind::KW_CHAR:
			++char_count;
			break;
		case TokenKind::KW_DOUBLE:
			++double_count;
			break;
		case TokenKind::KW_VOID:
			++other_count;
			other = FundamentalType::Void;
			break;
		case TokenKind::KW_BOOL:
			++other_count;
			other = FundamentalType::Bool;
			break;
		case TokenKind::KW_CHAR16_T:
			++other_count;
			other = FundamentalType::Char16T;
			break;
		case TokenKind::KW_CHAR32_T:
			++other_count;
			other = FundamentalType::Char32T;
			break;
		case TokenKind::KW_WCHAR_T:
			++other_count;
			other = FundamentalType::WcharT;
			break;
		case TokenKind::KW_FLOAT:
			++other_count;
			other = FundamentalType::Float;
			break;
		default:
			break;
		}
	}
	const unsigned sign = signed_count + unsigned_count;
	if (signed_count > 1 || unsigned_count > 1 || (signed_count && unsigned_count) || short_count > 1 ||
		long_count > 2 || int_count > 1 || char_count > 1 || double_count > 1 || other_count > 1 ||
		(short_count && long_count))
		error(location, "invalid combination of type specifiers");
	if (other_count != 0)
	{
		if (sign || short_count || long_count || int_count || char_count || double_count)
			error(location, "invalid combination of type specifiers");
		return types_.fundamental(other);
	}
	if (char_count != 0)
	{
		if (short_count || long_count || int_count || double_count)
			error(location, "invalid combination of type specifiers");
		return types_.fundamental(signed_count ? FundamentalType::SignedChar
				: unsigned_count               ? FundamentalType::UnsignedChar
											   : FundamentalType::Char);
	}
	if (double_count != 0)
	{
		if (sign || short_count || int_count || long_count > 1)
			error(location, "invalid combination of type specifiers");
		return types_.fundamental(long_count ? FundamentalType::LongDouble : FundamentalType::Double);
	}
	FundamentalType result;
	if (short_count)
		result = unsigned_count ? FundamentalType::UnsignedShortInt : FundamentalType::ShortInt;
	else if (long_count == 2)
		result = unsigned_count ? FundamentalType::UnsignedLongLongInt : FundamentalType::LongLongInt;
	else if (long_count == 1)
		result = unsigned_count ? FundamentalType::UnsignedLongInt : FundamentalType::LongInt;
	else
		result = unsigned_count ? FundamentalType::UnsignedInt : FundamentalType::Int;
	return types_.fundamental(result);
}

const Type* Analyzer::int128_type(const syntax::Slice<syntax::DeclSpecifier>& list, uint32_t location)
{
	// GCC's `signed __int128` and `unsigned __int128` (in either order): the
	// 7.1.6.2 rules with one sign specifier and no other type keyword.
	unsigned signs = 0, others = 0;
	bool is_unsigned = false;
	for (const syntax::DeclSpecifier& specifier : list)
	{
		if (specifier.kind != syntax::DeclSpecKind::Keyword || !IsTypeKeyword(specifier.keyword))
			continue;
		if (specifier.keyword == TokenKind::KW_SIGNED || specifier.keyword == TokenKind::KW_UNSIGNED)
		{
			++signs;
			is_unsigned = specifier.keyword == TokenKind::KW_UNSIGNED;
		}
		else
			++others;
	}
	if (signs > 1 || others != 0)
		error(location, "invalid combination of type specifiers");
	return types_.fundamental(is_unsigned ? FundamentalType::UnsignedInt128 : FundamentalType::Int128);
}

const Type* Analyzer::named_type(const syntax::Name& name, Scope* scope, NameTypeUse use)
{
	// 14.6p5: typename applies to qualified names only.
	if (name.typename_keyword && !name.qualified())
		error(name.location, "typename applied to an unqualified name");
	LookupResult result = lookup_name(name, scope, LookupFilter::Ordinary);
	if (failed())
		return nullptr;
	if (use == NameTypeUse::TemplateArgument)
	{
		const bool dependent = name.qualified() && last_prefix_.dependent != nullptr;
		const Type* templ = dependent && name.last().template_keyword
			? dependent_template_name(last_prefix_.dependent, name.last().identifier)
			: template_name_type(result.binding, result.injected_template);
		if (templ != nullptr)
		{
			if (!dependent && semantic() && template_depth_ == 0)
			{
				if (name.qualified() && last_prefix_.scope != nullptr && last_prefix_.scope->kind == ScopeKind::Class)
					check_access(result.binding, static_cast<const ClassEntity*>(last_prefix_.scope->entity), nullptr,
						scope, name.location);
				else if (!name.qualified())
					check_unqualified_access(result.binding, scope, name.location);
			}
			named_binding_ = result.binding;
			return templ;
		}
	}
	if (name.qualified() && !name.typename_keyword)
	{
		// 14.6p2-p3: a name qualified by a dependent type, or naming a
		// member of an unknown specialization through the current
		// instantiation (one the current instantiation lacks while it has a
		// dependent base, 14.6.2.1p5), names a type only with `typename`.
		if (last_prefix_.dependent != nullptr || IsDependentMember(result.binding))
			error(name.location, "a dependent qualified name needs typename to name a type");
		// Before an out-of-class member's qualified declarator-id, the class
		// it names is not yet the current instantiation for this rule: a
		// member of it needs typename too (the course's C++11 reading,
		// README).
		if (owner_specifiers_ && last_prefix_.scope != nullptr && templated_class(last_prefix_.scope))
			error(name.location, "a dependent qualified name before the declarator-id needs typename");
		// 3.4.3.1p2: after a nested-name-specifier nominating a class, the
		// class's own injected-class-name names its constructor, which is
		// no type (`K::K*`; an elaborated-type-specifier, a typename-
		// specifier and a nested-name-specifier find the class).
		if (last_prefix_.scope != nullptr && result.binding->declared_in == last_prefix_.scope &&
			result.binding->kind == BindingKind::Type && result.binding->entity == last_prefix_.scope->entity &&
			!name.last().has_arguments)
		{
			named_binding_ = result.binding;
			substitution_error(
				name.location, "a class's qualified injected-class-name names its constructor, not a type");
			return nullptr;
		}
	}
	if (!name.last().has_arguments)
		check_template_name_type(result.binding, name.location);
	named_binding_ = result.binding;
	const Type* type = binding_type(result.binding, name.location);
	// A template-name is no type (14.3.3p1: only a template argument).
	if (type->kind == TypeKind::Template || type->kind == TypeKind::TemplateTemplateParameter)
		error(name.location, "a template-name without a template argument list");
	return type;
}

void Analyzer::check_template_name_type(const Binding* binding, uint32_t location)
{
	// 14.6.1p1, p5: only a class template's injected-class-name (declared in
	// its own scope) names a type without a template argument list; its
	// name found elsewhere, say past a dependent base whose injected-class-
	// name lookup does not see (14.6.2p3), names the template itself.
	if (binding->kind == BindingKind::TypeAlias && binding->entity != nullptr &&
		binding->entity->kind == EntityKind::AliasTemplate)
		error(location, "an alias template name without a template argument list");
	if (binding->kind != BindingKind::Type || binding->entity == nullptr || binding->entity->kind != EntityKind::Class)
		return;
	const ClassEntity* cls = static_cast<const ClassEntity*>(binding->entity);
	if (cls->templ != nullptr && cls->templ->role == TemplateRole::Template && binding->declared_in != cls->scope)
		error(location, "a class template name without a template argument list");
}

const Type* Analyzer::binding_type(const Binding* binding, uint32_t location)
{
	switch (binding->kind)
	{
	case BindingKind::Type:
		return binding->entity->type;
	case BindingKind::TypeAlias:
		return binding->type;
	default:
		error(location, "name does not denote a type");
	}
}

// ------------------------------------------------------------------ names

LookupResult Analyzer::lookup_name(const syntax::Name& name, Scope* scope, LookupFilter filter)
{
	const syntax::NameComponent& last = name.last();
	if (last.kind != syntax::NameComponentKind::Identifier)
		error(name.location, "operator and special member names are not supported here");
	LookupResult result = LookupResult();
	last_prefix_ = NamePrefix{ nullptr, nullptr, nullptr };
	if (!name.qualified())
		result = unqualified(scope, last.identifier, filter);
	else
	{
		last_prefix_ = name_prefix(name, scope);
		if (failed())
			return result;
		result = lookup_in(last_prefix_, last.identifier, filter);
		// A builtin is a global-namespace name: `::__builtin_x`.
		if (!result.found() && name.global && name.components.size == 1 && filter == LookupFilter::Ordinary)
			result = builtin_lookup(last.identifier, result);
		// A qualified name its substituted qualifier lacks (14.8.2p8).
		if (!result.found())
		{
			substitution_error(name.location, "undeclared name");
			return result;
		}
	}
	if (!result.found())
		error(name.location, "undeclared name");
	// An ambiguous member (10.2p6) in a substitution is its failure.
	if (result.ambiguous && !(result.injected_template && last.has_arguments))
	{
		substitution_error(name.location, "ambiguous name");
		return LookupResult();
	}
	// 11.2p5: a member named through its class (or a derived class) is
	// accessible where the name appears; a function's access waits for
	// overload resolution.
	// So is one a member template's declaration in an instance names
	// through a class the instantiation made known (14.7.1p1: the member
	// template's declaration is instantiated with its class).
	if (semantic() && last_prefix_.scope != nullptr && last_prefix_.scope->kind == ScopeKind::Class &&
		result.binding->kind != BindingKind::Function && (template_depth_ == 0 || InstanceMemberTemplate(scope)))
		check_access(
			result.binding, static_cast<const ClassEntity*>(last_prefix_.scope->entity), nullptr, scope, name.location);
	else if (semantic() && template_depth_ == 0 && !name.qualified() && result.binding->kind != BindingKind::Function)
		check_unqualified_access(result.binding, scope, name.location);
	// A typename-specifier's dependent member template-id names the
	// dependent member template's specialization (14.6p2, 14.2p4).
	if (last.has_arguments && last_prefix_.dependent != nullptr && name.typename_keyword)
	{
		if (!last.template_keyword)
			error(last.location, "a dependent member template-id needs template");
		const NamePrefix prefix = last_prefix_;
		TypeList arguments;
		if (!template_arguments(last, scope, arguments))
			return LookupResult();
		const Type* templ = dependent_template_name(prefix.dependent, last.identifier);
		result = LookupResult{ specialization_binding(
								   specialize_template(templ, arguments, last.location), templ->entity),
			nullptr, false, false };
		last_prefix_ = prefix;
		return result;
	}
	// A template-id (14.2) names a class template specialization (a
	// function template's is resolved by its use).
	if (last.has_arguments && last_prefix_.dependent == nullptr && result.binding->kind != BindingKind::Function)
	{
		// The arguments' own lookups leave this name's prefix as it was.
		const NamePrefix prefix = last_prefix_;
		Binding* specialization = template_id_binding(result.binding, last, scope);
		if (failed())
			return LookupResult();
		if (specialization == nullptr)
			specialization = variable_template_binding(result.binding, last, scope);
		if (specialization == nullptr)
			error(name.location, "a template-id that names no class or variable template");
		result = LookupResult{ specialization, nullptr, false };
		last_prefix_ = prefix;
	}
	return result;
}

NamePrefix Analyzer::name_prefix(const syntax::Name& name, Scope* scope, bool declared)
{
	NamePrefix prefix = NamePrefix{ nullptr, nullptr, nullptr };
	if (name.global)
		prefix = NamePrefix{ graph_.global, graph_.global->entity, nullptr };
	for (uint32_t i = 0; i + 1 < name.components.size; ++i)
	{
		const syntax::NameComponent& component = name.components[i];
		if (component.kind == syntax::NameComponentKind::Decltype)
		{
			const Type* type = decltype_type(*component.decltype_operand, scope);
			if (type == nullptr)
				return NamePrefix{ nullptr, nullptr, nullptr };
			type = type->unqualified;
			if (template_depth_ != 0 && dependent_type(type))
			{
				prefix = NamePrefix{ nullptr, nullptr, type };
				continue;
			}
			if (type->kind != TypeKind::Class && type->kind != TypeKind::Enum)
			{
				substitution_error(
					component.location, "decltype in a nested-name-specifier must name a class or enumeration");
				return NamePrefix{ nullptr, nullptr, nullptr };
			}
			Binding carrier = Binding();
			carrier.kind = BindingKind::TypeAlias;
			carrier.type = type;
			prefix = scope_of_binding(&carrier, component.location);
			if (failed())
				return prefix;
			continue;
		}
		if (component.kind != syntax::NameComponentKind::Identifier)
			error(component.location, "invalid nested-name-specifier");
		// `Q::template N<A...>::` with Q dependent: the dependent member
		// template's specialization (14.2p4, 14.6.2.1p7).
		if (prefix.dependent != nullptr && component.has_arguments)
		{
			if (!component.template_keyword)
				error(component.location, "a dependent member template-id needs template");
			TypeList arguments;
			if (!template_arguments(component, scope, arguments))
				return NamePrefix{ nullptr, nullptr, nullptr };
			const Type* templ = dependent_template_name(prefix.dependent, component.identifier);
			prefix = NamePrefix{ nullptr, nullptr, specialize_template(templ, arguments, component.location) };
			continue;
		}
		const bool first = prefix.entity == nullptr && prefix.dependent == nullptr;
		LookupResult result = first ? names_.unqualified(scope, component.identifier, LookupFilter::Prefix)
									: lookup_in(prefix, component.identifier, LookupFilter::Prefix);
		if (!result.found())
		{
			substitution_error(component.location, "undeclared name in a nested-name-specifier");
			return NamePrefix{ nullptr, nullptr, nullptr };
		}
		if (result.ambiguous && !(result.injected_template && component.has_arguments))
		{
			substitution_error(component.location, "ambiguous name in a nested-name-specifier");
			return NamePrefix{ nullptr, nullptr, nullptr };
		}
		const Binding* binding = result.binding;
		// 11p4: access control applies to every name, a nested-name-
		// specifier's too: a member named through its class, or found by
		// unqualified lookup in the innermost class that has it (11.2p5).
		if (semantic() && template_depth_ == 0 && !declared)
		{
			if (first && binding->declared_in != nullptr && binding->declared_in->kind == ScopeKind::Class)
				check_unqualified_access(binding, scope, component.location);
			else if (prefix.scope != nullptr && prefix.scope->kind == ScopeKind::Class)
				check_access(
					binding, static_cast<const ClassEntity*>(prefix.scope->entity), nullptr, scope, component.location);
		}
		if (component.has_arguments && prefix.dependent == nullptr)
		{
			binding = template_id_binding(binding, component, scope);
			if (failed())
				return NamePrefix{ nullptr, nullptr, nullptr };
			if (binding == nullptr)
				error(component.location, "a template-id in a nested-name-specifier that names no class template");
		}
		// 14.6.1p1: a class template's name alone is a class-name only as
		// its injected-class-name; elsewhere its template argument list
		// follows (14.2p1), before `::` too. In a substitution the name is
		// invalid: its failure (14.8.2p8).
		else if (ClassTemplateNameAlone(binding))
		{
			substitution_error(component.location, "a class template name without a template argument list");
			return NamePrefix{ nullptr, nullptr, nullptr };
		}
		prefix = scope_of_binding(binding, component.location);
		if (failed())
			return prefix;
	}
	return prefix;
}

NamePrefix Analyzer::scope_of_binding(const Binding* binding, uint32_t location)
{
	switch (binding->kind)
	{
	case BindingKind::Namespace:
	{
		NamespaceEntity* entity = static_cast<NamespaceEntity*>(binding->entity);
		return NamePrefix{ entity->scope, entity, nullptr };
	}
	case BindingKind::NamespaceAlias:
	{
		NamespaceEntity* entity = static_cast<NamespaceAliasEntity*>(binding->entity)->target;
		return NamePrefix{ entity->scope, entity, nullptr };
	}
	case BindingKind::Type:
	case BindingKind::TypeAlias:
		break;
	// In a substitution, a qualifier that names no class, namespace or
	// enumeration (a non-type member, a scalar type) is its failure
	// (14.8.2p8: a type in a qualified name that is no class or
	// enumeration; a qualifier naming a member that is no type).
	default:
		substitution_error(location, "name before :: is not a class, namespace or enumeration");
		return NamePrefix{ nullptr, nullptr, nullptr };
	}
	const Type* type = binding_type(binding, location)->unqualified;
	switch (type->kind)
	{
	case TypeKind::Class:
	{
		ClassEntity* entity = static_cast<ClassEntity*>(type->entity);
		// A dependent class other than the current instantiation (whose
		// members are searched in its definition) is searched when
		// instantiated (14.6.2.1).
		if (entity->scope == nullptr && dependent_type(type))
			return NamePrefix{ nullptr, nullptr, type };
		// Where a substitution's failure is a result (14.8.2p8), one naming
		// an incomplete class fails for now: the class may be completed
		// later, or be the one being defined (its partial specialization's
		// selection naming it again).
		if (entity->scope == nullptr && !require_complete(entity))
		{
			substitution_error(location, "incomplete class in a nested-name-specifier", true);
			return NamePrefix{ nullptr, nullptr, nullptr };
		}
		return NamePrefix{ entity->scope, entity, nullptr };
	}
	case TypeKind::Enum:
	{
		EnumEntity* entity = static_cast<EnumEntity*>(type->entity);
		return NamePrefix{ entity->scoped ? entity->scope : nullptr, entity, nullptr };
	}
	case TypeKind::TemplateParameter:
	case TypeKind::TemplateTemplateParameter:
		if (!dependent_type(type))
			error(location, "name before :: is not a class, namespace or enumeration");
		return NamePrefix{ nullptr, nullptr, type };
	case TypeKind::BoundPack:
		// A bound pack in a kept expansion's pattern: each element's member.
		return NamePrefix{ nullptr, nullptr, type };
	default:
		substitution_error(location, "name before :: is not a class, namespace or enumeration");
		return NamePrefix{ nullptr, nullptr, nullptr };
	}
}

LookupResult Analyzer::lookup_in(const NamePrefix& prefix, const Identifier* name, LookupFilter filter)
{
	// A prefix whose substitution failed (see scope_of_binding) finds
	// nothing.
	if (prefix.entity == nullptr && prefix.dependent == nullptr && prefix.scope == nullptr)
		return LookupResult();
	if (prefix.dependent != nullptr)
	{
		// A member of a dependent type: a placeholder until instantiation
		// (a namespace cannot be one).
		if (filter == LookupFilter::Namespace)
			return LookupResult();
		return LookupResult{ dependent_member_binding(prefix.dependent, name), nullptr, false };
	}
	if (prefix.scope != nullptr)
	{
		implicit_members_for(prefix.scope, name);
		LookupResult result = names_.qualified(prefix.scope, name, filter);
		// 14.6.2.1p5: a name the current instantiation lacks while it has a
		// dependent base names a member of an unknown specialization.
		if (!result.found() && filter != LookupFilter::Namespace && has_dependent_bases(prefix.scope))
			result = LookupResult{ dependent_member_binding(prefix.entity->type, name), nullptr, false };
		// A class being defined may declare it later (see
		// incomplete_classes_seen_).
		if (!result.found() && prefix.scope->kind == ScopeKind::Class &&
			!static_cast<const ClassEntity*>(prefix.scope->entity)->complete)
			++incomplete_classes_seen_;
		return result;
	}
	// An unscoped enumeration names its enumerators, which are declared in
	// the scope enclosing it (5.1.1p8, 7.2p10).
	LookupResult result = LookupResult();
	const EnumEntity* enumeration = static_cast<const EnumEntity*>(prefix.entity);
	if (filter != LookupFilter::Ordinary)
		return result;
	const NameEntry* entry = names_.find(enumeration->owner, name);
	if (entry != nullptr && entry->value != nullptr && entry->value->kind == BindingKind::Enumerator &&
		static_cast<const EnumeratorEntity*>(entry->value->entity)->enumeration == enumeration)
		result.binding = entry->value;
	return result;
}

// ------------------------------------------------------------ declarators

const char* Analyzer::asm_label(const TokenSpelling& spelling)
{
	// The symbol name an asm label's string literal spells: its pieces'
	// characters concatenated (escapes of `\` and `"` decoded).
	std::string name;
	bool inside = false;
	for (uint32_t i = 0; i < spelling.length; ++i)
	{
		const char c = spelling.text[i];
		if (c == '"')
			inside = !inside;
		else if (inside && c == '\\' && i + 1 < spelling.length)
			name.push_back(spelling.text[++i]);
		else if (inside)
			name.push_back(c);
	}
	return copy_label(name.data(), name.size());
}

DeclaratorInfo Analyzer::declarator(const syntax::Declarator* declarator, const Type* type, Scope* scope,
	uint32_t location, const Specifiers* specifiers)
{
	DeclaratorInfo info = DeclaratorInfo();
	info.location = declarator != nullptr ? declarator->location : location;
	info.name = DeclaratorId(declarator);
	// GNU's vector attributes after the declarator-id make the declared
	// type's lane type a vector (`typedef float v4sf
	// __attribute__((vector_size(16)));`).
	for (const syntax::Attribute* attribute = declarator != nullptr ? declarator->attributes : nullptr;
		attribute != nullptr && type != nullptr; attribute = attribute->next)
	{
		if (attribute->kind == syntax::AttributeKind::VectorSize ||
			attribute->kind == syntax::AttributeKind::ExtVectorType)
			type = vector_type(type, *attribute->expression, attribute->kind == syntax::AttributeKind::VectorSize,
				scope, attribute->location);
	}
	// Names after a qualified declarator-id are looked up in the scope it
	// names (3.4.3p3); the ptr-operators precede it.
	Scope* after = scope;
	if (info.name != nullptr && info.name->qualified())
	{
		const NamePrefix prefix = name_prefix(*info.name, scope, true);
		if (prefix.scope == nullptr || prefix.scope->kind == ScopeKind::Enum)
			error(info.name->location, "qualified declarator-id must name a namespace or class member");
		info.qualifier = prefix.scope;
		// A friend declaration naming a namespace's function: its names
		// are looked up as in the class granting friendship (as g++ and
		// Clang do; 3.4.1p10's rule for a class's member function).
		if (specifiers != nullptr && specifiers->is_friend && prefix.scope != nullptr &&
			prefix.scope->kind == ScopeKind::Namespace)
			after = scope;
		else
			after = qualified_lookup_scope(prefix.scope, scope);
	}
	info.after = after;
	// 5.1.1p3: in the declarator of a non-static member function of a class
	// X, `this` may appear after the cv-qualifier-seq: in the trailing
	// return type (whose declaration's type is `auto`, 8.3.5p2) of the
	// function's own parameter list (the last suffix applied, the first of
	// the declarator-id's level).
	const ClassEntity* member = nullptr;
	const syntax::DeclaratorSuffix* member_suffix = nullptr;
	if (specifiers != nullptr && specifiers->auto_type && !specifiers->is_friend && !specifiers->is_static &&
		!specifiers->is_typedef && info.name != nullptr)
	{
		const syntax::Declarator* innermost = declarator;
		while (innermost->name == nullptr)
			innermost = innermost->nested;
		const Scope* owner = info.qualifier != nullptr ? info.qualifier : declaration_scope(scope);
		if (owner->kind == ScopeKind::Class && !innermost->suffixes.empty() &&
			innermost->suffixes[0].kind == syntax::DeclaratorSuffixKind::Function)
		{
			member = static_cast<const ClassEntity*>(owner->entity);
			member_suffix = &innermost->suffixes[0];
		}
	}
	const syntax::DeclaratorSuffix* function = nullptr;
	// The last array suffix applied, unless a ptr-operator followed.
	const syntax::DeclaratorSuffix* array = nullptr;
	// 9.2, 9.2p9: virt-specifiers only after the whole declarator (the
	// outermost declarator's last suffix) of a function.
	const syntax::DeclaratorSuffix* last = declarator != nullptr && declarator->suffixes.size != 0
		? &declarator->suffixes[declarator->suffixes.size - 1]
		: nullptr;
	// 15.4p2: an exception-specification only on a function declarator
	// whose function type (or a pointer, reference or pointer to member to
	// it: one derivation) is the declaration's type, or a parameter or
	// return type of a function declarator.
	bool specified = false;
	uint32_t derivations = 0;
	for (const syntax::Declarator* current = declarator; current != nullptr; current = current->nested)
	{
		check_stack(current->location);
		for (const syntax::PtrOperator& pointer : current->pointers)
		{
			// An invalid type in a substitution (14.8.2p8) is its failure.
			type = ptr_operator(pointer, type, scope, current->location);
			if (failed())
				return info;
			function = nullptr;
			array = nullptr;
			if (specified && ++derivations > 1)
				error(current->location, "an exception-specification on a function type nested in a declarator");
		}
		for (uint32_t i = current->suffixes.size; i-- > 0;)
		{
			const syntax::DeclaratorSuffix& suffix = current->suffixes[i];
			if (&suffix != last && HasVirtSpecifier(suffix))
				error(current->location, "a virt-specifier inside a declarator");
			type = apply_suffix(suffix, type, after, current->location, &suffix == member_suffix ? member : nullptr);
			if (failed())
				return info;
			function = suffix.kind == syntax::DeclaratorSuffixKind::Function ? &suffix : nullptr;
			array = suffix.kind == syntax::DeclaratorSuffixKind::Array ? &suffix : nullptr;
			if (specified && function == nullptr)
				error(current->location, "an exception-specification on a function type nested in a declarator");
			specified = function != nullptr && HasExceptionSpecification(function);
			derivations = 0;
		}
	}
	// A declarator's pack ellipsis (8p4, 8.3.5p14): a function parameter
	// pack, whose type is a pack expansion of its pattern.
	if (declarator != nullptr && declarator->pack != syntax::PackPosition::None)
		type = pack_expansion_type(type, declarator->location);
	info.type = type;
	if (declarator != nullptr)
		for (const syntax::Attribute* attribute = declarator->attributes; attribute != nullptr;
			attribute = attribute->next)
			if (attribute->kind == syntax::AttributeKind::AsmLabel)
				info.asm_label = asm_label(static_cast<const syntax::LiteralExpr*>(attribute->expression)->spelling);
	info.function_suffix = type->kind == TypeKind::Function ? function : nullptr;
	info.bound = type->kind == TypeKind::Array && array != nullptr ? array->bound : nullptr;
	info.virt_suffix = last != nullptr && HasVirtSpecifier(*last) ? last : nullptr;
	if (info.virt_suffix != nullptr && type->kind != TypeKind::Function)
		error(info.location, "a virt-specifier in a declaration of no function");
	info.parameters = info.function_suffix != nullptr ? info.function_suffix->parameters : nullptr;
	// The declared function's own prototype scope joins the dump here.
	if (info.parameters != nullptr && pending_prototype_.clause == info.parameters && info.name != nullptr)
	{
		// (A specialization's, in an explicit instantiation or
		// specialization: its template's name.)
		Scope* prototype = pending_prototype_.scope;
		const bool specialization = info.name->last().has_arguments;
		prototype->label = specialization ? specialization_key(*info.name)->spelling
										  : declared_name(*info.name, declaration_scope(after)).label;
		if (prototype->label == nullptr)
			prototype->label = info.name->last().identifier->spelling;
		attach_scope(prototype, scope);
	}
	pending_prototype_ = PendingPrototype{ nullptr, nullptr };
	return info;
}

Scope* Analyzer::qualified_lookup_scope(Scope* qualifier, Scope* scope)
{
	// After a qualified declarator-id the named class or namespace is
	// searched first (3.4.3p3), yet a templated declaration's own template
	// parameters stay visible: a bridge scope holding them leads into the
	// named scope, the innermost parameter of a name winning. In the
	// definition of a member of a class template, a member of the class (of
	// an enclosing class, or of a base that does not depend on a template
	// parameter) hides a parameter of the enclosing class templates of the
	// same name (14.6.1p7, p9): the bridge leaves that parameter out, and
	// the instantiations bind only the names it holds (instantiation_frame).
	// A template member's own parameters, the lists beyond one per
	// enclosing class template, stay; so does a parameter named like a
	// member of the enclosing namespace, which it hides (14.6.1p8).
	if (scope->kind != ScopeKind::TemplateParameters)
	{
		// No template-head (an explicit specialization's member, a member
		// of an instance): no template parameter is in scope either, not
		// those of the class's (or its enclosing classes') definitions or
		// instantiation frames (14.7.3p5, 14.5.1p3): an empty bridge.
		bool frames = false;
		for (const Scope* s = qualifier; !frames && s != nullptr && s->kind != ScopeKind::Namespace; s = s->parent)
			frames = s->kind == ScopeKind::TemplateParameters;
		if (!frames)
			return qualifier;
		Scope* empty = new_scope(ScopeKind::TemplateParameters, qualifier, nullptr, nullptr, nullptr);
		empty->dumped = false;
		empty->lists = 0;
		empty->bridge = true;
		empty->concrete_members = qualifier->kind == ScopeKind::Class;
		return empty;
	}
	Scope* bridge = new_scope(ScopeKind::TemplateParameters, qualifier, nullptr, nullptr, nullptr);
	bridge->dumped = false;
	bridge->bridge = true;
	uint32_t lists = 0;
	for (const Scope* current = scope; current->kind == ScopeKind::TemplateParameters; current = current->parent)
		++lists;
	const uint32_t enclosing = EnclosingTemplateLists(qualifier);
	bridge->lists = static_cast<uint8_t>(lists > enclosing ? lists - enclosing : 0);
	uint32_t level = 0;
	for (const Scope* current = scope; current->kind == ScopeKind::TemplateParameters;
		current = current->parent, ++level)
	{
		const bool own = level + enclosing < lists;
		for (Binding* binding = current->bindings; binding != nullptr; binding = binding->next)
		{
			if (binding->entity->kind != EntityKind::TemplateParameter || names_.find(bridge, binding->name) != nullptr)
				continue;
			if (!own && member_hides_parameter(qualifier, binding->name))
				continue;
			declare_in(bridge, binding);
		}
	}
	return bridge;
}

bool Analyzer::member_hides_parameter(Scope* qualifier, const Identifier* name)
{
	// A member of the class or of an enclosing class, found as unqualified
	// lookup from the member's definition would find it: bases that depend
	// on a template parameter are not searched (14.6.2p3).
	for (Scope* current = qualifier;
		current != nullptr && (current->kind == ScopeKind::Class || current->kind == ScopeKind::TemplateParameters);
		current = current->parent)
	{
		if (current->kind == ScopeKind::Class && names_.members(current, name, LookupFilter::Ordinary).found())
			return true;
	}
	return false;
}

const Type* Analyzer::ptr_operator(
	const syntax::PtrOperator& pointer, const Type* type, Scope* scope, uint32_t location)
{
	if (type == auto_placeholder_ && !auto_declarator_)
		error(location, "auto with a declarator operator is not supported here");
	const uint8_t cv = CvBits(pointer.cv);
	switch (pointer.kind)
	{
	case TokenKind::OP_STAR:
	{
		const Type* pointer = checked_pointer(type, location);
		return pointer != nullptr ? types_.qualified(pointer, cv) : type;
	}
	case TokenKind::OP_XOR:
		// A block pointer (Clang's blocks): to a function type, outside
		// templates.
		if (type->kind != TypeKind::Function || template_depth_ != 0 || dependent_type(type))
			error(location, "a block pointer to no function type or in a template is not supported");
		return types_.qualified(types_.block_pointer(type), cv);
	case TokenKind::OP_AMP:
	case TokenKind::OP_LAND:
		if (cv != 0)
			error(location, "cv-qualified reference");
		if (type->unqualified == types_.fundamental(FundamentalType::Void))
		{
			substitution_error(location, "reference to void");
			return type;
		}
		return types_.reference(type, pointer.kind == TokenKind::OP_LAND);
	default:
		break;
	}
	// A pointer to member (8.3.3): the nested-name-specifier, stored as a
	// name, names the class.
	const LookupResult owner = lookup_name(*pointer.class_name, scope, LookupFilter::Prefix);
	if (failed())
		return type;
	const Type* owner_type = nullptr;
	if (owner.binding->kind == BindingKind::Type || owner.binding->kind == BindingKind::TypeAlias)
		owner_type = binding_type(owner.binding, location)->unqualified;
	const bool dependent = owner_type != nullptr && owner_type->kind == TypeKind::TemplateParameter;
	const char* failure = owner_type == nullptr || (owner_type->kind != TypeKind::Class && !dependent)
		? "pointer to member of a non-class"
		: type->is_reference()                                           ? "pointer to member of reference type"
		: type->unqualified == types_.fundamental(FundamentalType::Void) ? "pointer to member of type void"
																		 : nullptr;
	if (failure != nullptr)
	{
		substitution_error(location, failure);
		return type;
	}
	return types_.qualified(types_.member_pointer(owner_type, type), cv);
}

const Type* Analyzer::checked_pointer(const Type* pointee, uint32_t location)
{
	// 8.3.2p5: no pointer to a reference. Null for a substitution's failure
	// (14.8.2p8), as for the checks below.
	if (pointee->is_reference())
	{
		substitution_error(location, "pointer to reference");
		return nullptr;
	}
	return types_.pointer(pointee);
}

const Type* Analyzer::checked_array(const Type* element, uint64_t bound, uint32_t location)
{
	// 8.3.4p1: no arrays of references, void, functions or arrays of
	// unknown bound, nor of an abstract class (10.4p3; a complete class's).
	const char* failure = element->is_reference()                           ? "array of references"
		: element->kind == TypeKind::Function                               ? "array of functions"
		: element->unqualified == types_.fundamental(FundamentalType::Void) ? "array of void"
		: element->kind == TypeKind::Array && element->bound == 0 && element->owner == nullptr
		? "array of arrays of unknown bound"
		: nullptr;
	if (failure != nullptr)
	{
		substitution_error(location, failure);
		return nullptr;
	}
	if (semantic() && template_depth_ == 0 && !check_abstract_object(element, location))
		return nullptr;
	return types_.array(element, bound);
}

const Type* Analyzer::apply_suffix(const syntax::DeclaratorSuffix& suffix, const Type* type, Scope* scope,
	uint32_t location, const ClassEntity* member)
{
	if (suffix.kind == syntax::DeclaratorSuffixKind::Array)
	{
		if (holds_auto(type))
			error(location, "array of auto");
		const uint64_t bound = suffix.bound != nullptr ? array_bound(*suffix.bound, scope) : 0;
		const Type* array = checked_array(type, bound, location);
		if (array == nullptr)
			return type;
		// A template's bound it cannot evaluate depends on its parameters
		// (14.6.2.3): the array keeps the expression, evaluated where the
		// type is substituted (its instantiations analyze their own copy).
		if (bound == 0 && suffix.bound != nullptr && template_depth_ != 0)
			return types_.array(type, 0, dependent_decltype(*suffix.bound, scope));
		return array;
	}
	const syntax::ParameterClause& clause = *suffix.parameters;
	const size_t base = parameter_scratch_.size();
	const bool void_list = clause.parameters.size == 1 && !clause.ellipsis &&
		clause.parameters[0]->default_argument == nullptr && DeclaratorId(clause.parameters[0]->declarator) == nullptr;
	// 3.3.4p1: a parameter is in scope from its declarator on; a later
	// parameter's decltype-specifier may name it.
	Scope* earlier = nullptr;
	// Whether the previous parameter's type made a placeholder in `earlier`.
	bool placed = false;
	for (const syntax::ParameterDeclaration* parameter : clause.parameters)
	{
		const uint32_t index = static_cast<uint32_t>(parameter_scratch_.size() - base);
		if (earlier == nullptr && index != 0 && UsesDecltype(*parameter))
		{
			// The parameters' object types: an array or function parameter's
			// adjusted to a pointer (8.3.5p5).
			earlier = prototype_scope(clause, nullptr, 0, scope);
			earlier->dumped = false;
			earlier->guarded = true;
			for (uint32_t i = 0; i < index; ++i)
				declare_parameter(earlier, clause, i, parameter_object_type(parameter_scratch_[base + i]));
		}
		else if (earlier != nullptr)
		{
			// The previous parameter joins the ones before it, in a scope of
			// its own below them when its type made a placeholder there: a
			// placeholder sees the parameters before its own only (its
			// substitution rebuilds its scope's parameters, 3.3.4p1). So the
			// chain is as long as the parameters making placeholders.
			if (placed)
			{
				earlier = new_scope(ScopeKind::Function, earlier, nullptr, nullptr, nullptr);
				earlier->dumped = false;
				earlier->guarded = true;
			}
			declare_parameter(earlier, clause, index - 1, parameter_object_type(parameter_scratch_[base + index - 1]));
		}
		const uint32_t made = placeholders_made_;
		// 14.6.1p6: a parameter does not redeclare a template parameter.
		const syntax::Name* id = template_depth_ != 0 ? DeclaratorId(parameter->declarator) : nullptr;
		if (id != nullptr && id->last().kind == syntax::NameComponentKind::Identifier)
			check_template_parameter_name(scope, id->last().identifier, parameter->location);
		const Type* parameter_type = this->parameter_type(*parameter, earlier != nullptr ? earlier : scope);
		if (failed())
		{
			parameter_scratch_.resize(base);
			return type;
		}
		placed = placeholders_made_ != made;
		parameter_scratch_.push_back(parameter_type);
	}
	uint32_t count = static_cast<uint32_t>(parameter_scratch_.size() - base);
	// A sole unnamed parameter of non-dependent type void is an empty list
	// (8.3.5p4): one of a template parameter's type that an instantiation
	// makes void is a parameter of type void. Otherwise no parameter has
	// type void.
	if (void_list && template_depth_ != 0 && dependent_type(parameter_scratch_[base]))
		clause.dependent_sole = true;
	if (void_list && !clause.dependent_sole && parameter_scratch_[base] == types_.fundamental(FundamentalType::Void))
		count = 0;
	else
	{
		for (size_t i = base; i < parameter_scratch_.size(); ++i)
		{
			if (parameter_scratch_[i]->unqualified == types_.fundamental(FundamentalType::Void))
			{
				substitution_error(location, "parameter of type void");
				parameter_scratch_.resize(base);
				return type;
			}
		}
	}
	uint8_t cv = 0;
	RefQualifier ref = RefQualifier::None;
	for (const syntax::FunctionQualifier& qualifier : suffix.qualifiers)
	{
		switch (qualifier.kind)
		{
		case syntax::FunctionQualifierKind::Cv:
			cv |= qualifier.token == TokenKind::KW_CONST ? kConst : kVolatile;
			break;
		case syntax::FunctionQualifierKind::Ref:
			ref = qualifier.token == TokenKind::OP_AMP ? RefQualifier::LValue : RefQualifier::RValue;
			break;
		case syntax::FunctionQualifierKind::TrailingReturn:
			// 8.3.5p2: `auto` declarator (parameters) -> trailing-return-type,
			// in which the parameters are visible (3.3.4p1).
			{
				if (type != auto_placeholder_)
					error(location, "a trailing return type needs auto");
				Scope* prototype = prototype_scope(clause, parameter_scratch_.data() + base, count, scope);
				// A member function's `this` (5.1.1p3), qualified by the
				// cv-qualifiers before it; the context of an enclosing body
				// (a local class's member) does not reach in.
				BodyContext* saved_body = body_;
				BodyContext context = BodyContext();
				if (member != nullptr && semantic())
				{
					context.this_type = types_.pointer(types_.qualified(member->type, cv));
					context.enclosing = saved_body;
					body_ = &context;
				}
				type = type_id(*qualifier.trailing, prototype);
				body_ = saved_body;
				if (type == nullptr)
				{
					parameter_scratch_.resize(base);
					return auto_placeholder_;
				}
				// Set after the type-id's own declarators are done.
				pending_prototype_ = PendingPrototype{ &clause, prototype };
				break;
			}
		default:
			// Exception specifications and virt-specifiers are not part of
			// the type.
			break;
		}
	}
	// The course's extension (the README): a function definition's
	// placeholder return type without a trailing return type, deduced from
	// its return statements (as C++14's 7.1.6.4p2 and p9; see
	// deduced_result), on a declaration's own function declarator.
	if (holds_auto(type) && &suffix != placeholder_suffix_)
		error(location, "auto return type without a trailing return type");
	if (type->kind == TypeKind::Function || type->kind == TypeKind::Array)
	{
		substitution_error(location, "function returning a function or an array");
		parameter_scratch_.resize(base);
		return type;
	}
	const Type* result = types_.function(type, parameter_scratch_.data() + base, count, clause.ellipsis, cv, ref);
	parameter_scratch_.resize(base);
	return result;
}

const Type* Analyzer::parameter_object_type(const Type* type)
{
	if (type->kind == TypeKind::Array)
		return types_.pointer(type->element);
	if (type->kind == TypeKind::Function)
		return types_.pointer(type);
	return type;
}

Scope* Analyzer::prototype_scope(
	const syntax::ParameterClause& clause, const Type* const* types, uint32_t count, Scope* scope)
{
	// The function parameter scope of a declarator (3.3.4), for its trailing
	// return type. The course dump prints it, with the parameters, where the
	// declaration appears (declarator() attaches it); a definition's
	// function scope follows it.
	Scope* prototype = new_scope(ScopeKind::Function, scope, nullptr, nullptr, nullptr);
	for (uint32_t i = 0; i < count; ++i)
		declare_parameter(prototype, clause, i, types[i]);
	return prototype;
}

void Analyzer::prototype_dependence(Scope* scope, const Type* pattern, const syntax::ParameterClause& clause,
	const std::pair<const Identifier*, uint32_t>* packs, size_t count)
{
	// The original parameter types retain lookup dependence after
	// substitution. Expand only the pack's own parameters, using the
	// cloner's arities, so fixed parameters beside it stay fixed.
	pattern = pattern->canonical;
	Binding* binding = scope->bindings;
	size_t next = 0;
	for (uint32_t i = 0; i < pattern->param_count && binding != nullptr; ++i)
	{
		const syntax::Declarator* declarator = clause.parameters[i]->declarator;
		const uint32_t arity = declarator != nullptr && declarator->pack != syntax::PackPosition::None && next < count
			? packs[next++].second
			: 1;
		const bool dependent = dependent_type(pattern->params[i]);
		for (uint32_t k = 0; k < arity && binding != nullptr; ++k, binding = binding->next)
			binding->dependent = dependent;
	}
}

void Analyzer::declare_parameter(
	Scope* prototype, const syntax::ParameterClause& clause, uint32_t index, const Type* type)
{
	const syntax::Name* id = DeclaratorId(clause.parameters[index]->declarator);
	const Identifier* name = id != nullptr && id->last().kind == syntax::NameComponentKind::Identifier
		? id->last().identifier
		: nullptr;
	Entity* parameter = new_entity<Entity>(EntityKind::Parameter, name, prototype, type);
	Binding* binding = new_binding(BindingKind::Parameter, name, parameter, type);
	print_in(prototype, binding);
	if (name != nullptr)
		declare_in(prototype, binding);
	if (name != nullptr && semantic())
		note_nothrow_target(parameter, type, clause.parameters[index]->declarator, prototype);
}

const Type* Analyzer::parameter_type(const syntax::ParameterDeclaration& parameter, Scope* scope)
{
	Specifiers specifiers;
	this->specifiers(parameter.specifiers, scope, specifiers, false);
	if (specifiers.is_constinit)
		error(parameter.location, "__constinit in a parameter declaration");
	if (specifiers.type == nullptr || specifiers.type == auto_placeholder_)
		error(parameter.location, "parameter without a type");
	// 7.1.2p1: function-specifiers only in function declarations; 9.2p9:
	// virt-specifiers only in member function declarations.
	if (specifiers.is_virtual)
		error(parameter.location, "virtual in a parameter declaration");
	// 7.1.5p1 (its note and example): a parameter is not declared
	// constexpr.
	if (specifiers.is_constexpr)
		error(parameter.location, "constexpr in a parameter declaration");
	const DeclaratorInfo info = declarator(parameter.declarator, specifiers.type, scope, parameter.location);
	if (info.virt_suffix != nullptr)
		error(parameter.location, "a virt-specifier in a parameter declaration");
	return info.type;
}

const Type* Analyzer::type_id(const syntax::TypeId& type, Scope* scope)
{
	Specifiers specifiers;
	this->specifiers(type.specifiers, scope, specifiers, false);
	if (specifiers.is_constinit)
		error(type.location, "__constinit in a type-id");
	if (failed())
		return nullptr;
	if (specifiers.type == nullptr)
		error(type.location, "type-id without a type");
	const DeclaratorInfo info = declarator(type.declarator, specifiers.type, scope, type.location);
	if (failed())
		return nullptr;
	if (info.type == auto_placeholder_)
		error(type.location, "auto in a type-id");
	if (info.virt_suffix != nullptr)
		error(type.location, "a virt-specifier in a type-id");
	type_id_alignment_ = info.type == specifiers.type ? specifiers.alias_alignment : 0;
	return info.type;
}

void Analyzer::default_template_argument_names(const syntax::TypeId& type, Scope* scope)
{
	// A default template argument is analyzed where a template-id omits its
	// argument (14.1p9), but its non-dependent names are bound where it
	// appears (14.6p10): with the parameters before its own in scope, not
	// its own or later ones (3.3.9p1, 3.3.2p9), nor the template's name
	// (declared after its template-head). Keep that typed fact for later
	// substitution, including a dependent default. A member template's
	// default is no complete-class context (9.2p2), so a member declared
	// later is not found (3.4.1p7; `decltype(h(0))` before a better h).
	++template_depth_;
	const Type* bound = type_id(type, scope);
	--template_depth_;
	const DefaultKey key = DefaultKey{ &type, scope };
	if (bound != nullptr && !failed() && unbound_defaults_.find(key) == nullptr)
		unbound_defaults_.insert(key, bound);
}

uint64_t Analyzer::array_bound(const syntax::Expr& bound, Scope* scope)
{
	// 8.3.4p1: an integral constant expression converted to std::size_t,
	// greater than zero.
	Constant value;
	// Semantics mode types it first (a constexpr call, 5.19p2; a class
	// operand's conversion to std::size_t).
	if (semantic() && template_depth_ == 0)
		integral_constant_operand(bound, scope, types_.fundamental(FundamentalType::UnsignedLongInt));
	else if (semantic())
		validate_fixed_expression(bound, scope);
	if (failed())
		return 1;
	// The bound has a fixed integral type, but its value can depend on the
	// template. Keep that provenance in the declaration's type dependence.
	if (bound.facts != nullptr && (bound.facts->flags & kValueDependentUse) != 0)
		names_.note_dependent_use();
	// A bound in a template may depend on its parameters: evaluated when
	// it can be, otherwise left unknown for instantiation.
	if (!constant_value(bound, scope, value, template_depth_ == 0 && soft_substitution_ == 0))
	{
		if (template_depth_ == 0)
			substitution_error(bound.location, "an array bound that is not an integral constant expression");
		return 0;
	}
	if (template_depth_ != 0 && (value.bits == 0 || (IsSignedConstant(value) && static_cast<int64_t>(value.bits) < 0)))
		return 0;
	// GNU's zero-length array (hosted compilations), as its flexible array
	// member of unknown bound (see member_size); a substitution's still
	// fails (14.8.2p8, as in GCC).
	if (hosted_ && value.bits == 0 && soft_substitution_ == 0 && value.type->kind != TypeKind::Enum)
		return 0;
	// 14.8.2p8: a substitution forming an array of no positive size fails.
	const char* failure = value.type->kind == TypeKind::Enum &&
			static_cast<const EnumEntity*>(value.type->entity)->scoped
		? "a scoped enumeration value is not an array bound"
		: IsSignedConstant(value) && static_cast<int64_t>(value.bits) < 0 ? "negative array bound"
		: value.bits == 0                                                 ? "array bound is zero"
																		  : nullptr;
	if (failure != nullptr)
	{
		substitution_error(bound.location, failure);
		return 1;
	}
	return value.bits;
}

} // namespace sema
