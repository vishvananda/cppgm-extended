#include "sema/template_packs.h"

#include <algorithm>
#include <cstring>
#include <string>

#include "lex/number_literal.h"
#include "lex/text_literal.h"
#include "sema/integral_arithmetic.h"
#include "sema/placeholder_words.h"
#include "syntax/ast_clone.h"

// Placeholders of template definitions (14.5.6.1p5-p6, 14.6.2):
// the structural key that makes equivalent decltype operands and
// value-dependent template arguments one placeholder type (so equivalent
// redeclarations have one type and equivalent template-ids name one
// specialization), the placeholders made by it, the re-keying of a
// placeholder substituted with dependent arguments as the operand those
// arguments spell, and the substitution of placeholders (evaluated once
// per complete key, or pending with the levels still to apply).
namespace sema {

size_t DecltypeHash::operator()(const DecltypeKey& key) const
{
	size_t hash = key.count;
	for (uint32_t i = 0; i < key.count; ++i)
		hash = (hash ^ key.words[i]) * 0x100000001B3ull;
	return hash;
}

bool DecltypeKey::operator==(const DecltypeKey& other) const
{
	return count == other.count && std::memcmp(words, other.words, count * sizeof(uint64_t)) == 0;
}

size_t SubstitutionHash::operator()(const SubstitutionKey& key) const
{
	size_t hash = (reinterpret_cast<uintptr_t>(key.placeholder) * 0x9E3779B97F4A7C15ull) ^ key.depth ^
		reinterpret_cast<uintptr_t>(key.context);
	for (uint32_t i = 0; i < key.count; ++i)
		hash = (hash ^ reinterpret_cast<uintptr_t>(key.arguments[i])) * 0x100000001B3ull;
	return hash;
}

bool SubstitutionKey::operator==(const SubstitutionKey& other) const
{
	if (placeholder != other.placeholder || depth != other.depth || count != other.count || context != other.context)
		return false;
	for (uint32_t i = 0; i < count; ++i)
	{
		if (arguments[i] != other.arguments[i])
			return false;
	}
	return true;
}

namespace {

// The words of a structural key are sema/placeholder_words.h's.
uint64_t PointerWord(const void* pointer)
{
	return static_cast<uint64_t>(reinterpret_cast<uintptr_t>(pointer));
}

// An integral constant: an integer or boolean literal, or a non-type
// template argument's value an instantiation frame binds its parameter's
// name to (14.1p6), keyed alike by type and value (so the operand `A - B`
// of `template<int A, int B> using sub = I<A - B>` substituted with (N, 1)
// is the operand `N - 1`, 14.5.7p2).
void ConstantWords(std::vector<uint64_t>& words, const Constant& constant)
{
	words.push_back(kWordConstant);
	words.push_back(PointerWord(constant.type->canonical));
	words.push_back(constant.bits);
}

// Whether a name found by ordinary lookup denotes a non-static member (a
// data member, or a function set with a non-static member function), which
// names it through `(*this)` where `this` may appear (9.3.1p3).
bool NonStaticMember(const LookupResult& found)
{
	const Binding* binding = found.binding;
	if (binding->kind == BindingKind::Variable)
		return binding->entity != nullptr && binding->entity->kind == EntityKind::Variable &&
			static_cast<const VariableEntity*>(binding->entity)->member;
	if (binding->kind != BindingKind::Function)
		return false;
	for (const FunctionList* node = found.functions; node != nullptr; node = node->next)
	{
		const FunctionEntity* candidate = static_cast<const FunctionEntity*>(node->binding->entity);
		if (candidate->member && !candidate->is_static)
			return true;
	}
	const FunctionEntity* function = static_cast<const FunctionEntity*>(binding->entity);
	return found.functions == nullptr && function->member && !function->is_static;
}

bool PrototypeScope(const Scope* scope)
{
	return scope != nullptr && scope->kind == ScopeKind::Function && scope->entity == nullptr;
}

// The scope around a parameter-declaration-clause's scope, past the one a
// substitution adds to bind its parameter packs' names (see
// substituted_scope).
const Scope* ClauseParent(const Scope* scope)
{
	const Scope* outer = scope->parent;
	if (outer != nullptr && outer->kind == ScopeKind::TemplateParameters && !outer->dumped && outer->lists == 0 &&
		outer->entity == nullptr && PrototypeScope(outer->parent))
		outer = outer->parent;
	return outer;
}

// A literal's type and value: an integer or character literal's value
// (`integral`), a float or double literal's IEEE bits; false for another
// (a string, long double or user-defined literal).
bool LiteralValue(const TokenSpelling& spelling, FundamentalType& type, uint64_t& bits, bool& integral)
{
	TextLiteralParts parts;
	integral = true;
	if (SplitTextLiteral(spelling.text, spelling.length, parts))
	{
		if (parts.quote != '\'')
			return false;
		const CharacterLiteralValue character = EvaluateCharacterLiteral(spelling.text, spelling.length);
		if (!character.valid || character.multicharacter || character.suffix_begin != spelling.length)
			return false;
		type = character.type;
		bits = type == FundamentalType::Char
			? ConvertIntegral(character.code_point, IntegralKind{ FundamentalTypeSize(type) * 8, true })
			: character.code_point;
		return true;
	}
	if (spelling.length == 0 || ((spelling.text[0] < '0' || spelling.text[0] > '9') && spelling.text[0] != '.'))
		return false;
	const NumberLiteral number = ClassifyNumber(spelling.text, spelling.length);
	type = number.type;
	if (number.kind == NumberLiteralKind::Integer)
	{
		bits = number.integer;
		return true;
	}
	integral = false;
	if (number.kind != NumberLiteralKind::Floating ||
		(number.type != FundamentalType::Float && number.type != FundamentalType::Double))
		return false;
	std::string scratch;
	unsigned char bytes[16] = {};
	if (!ScanFloatingValue(spelling.text, number.suffix_begin, number.type, scratch, bytes))
		return false;
	bits = 0;
	std::memcpy(&bits, bytes, number.type == FundamentalType::Float ? 4 : 8);
	return true;
}

} // namespace

bool Analyzer::name_words(
	const syntax::Name& name, Scope* scope, bool resolve, Constant* constant, const syntax::CallExpr* call)
{
	// A name's components; an unqualified-id's (or a nested-name-
	// specifier's first) identifier stands for what ordinary lookup finds
	// there: a template parameter by its position, a function parameter of
	// a declarator by its index and type, another declaration by its
	// entity, a name found nowhere (for argument-dependent lookup) by its
	// spelling. Template arguments are their canonical types. A frame's
	// constant named alone is returned in `constant` (when the caller takes
	// one), which keys it as the literal of its value (see ConstantWords).
	// A non-static member named where `this` may appear (a member
	// function's trailing return type, 5.1.1p3) is `(*this).name`: keyed
	// with that `this`, whose cv-qualifiers the operand's meaning depends
	// on (and which its leading return type does not have). The callee of
	// a `call` with a type-dependent argument is a dependent name
	// (14.6.2p1): when it names functions, by its spelling, as one that
	// names nothing (14.5.6.1p5: only the name itself is considered; the
	// first equivalent declaration's placeholder keeps its lookup). A
	// function parameter is keyed by its position (see parameter_word); a
	// looked-up qualified name then records what it denotes (see
	// resolution_words). Names of dependent entities and types mark the key
	// dependent (see placeholder_words).
	std::vector<uint64_t>& words = decltype_words_;
	words.push_back(Word(kWordName, (name.components.size << 1) | (name.global ? 1 : 0)));
	// A callee whose explicit template arguments may resolve it (see
	// resolved_specialization): its function template and the position of
	// its words; the last component's arguments and their dependence.
	FunctionEntity* pending = nullptr;
	size_t pending_at = 0;
	size_t last_arguments = 0;
	bool dependent_arguments = false;
	for (uint32_t i = 0; i < name.components.size; ++i)
	{
		const syntax::NameComponent& component = name.components[i];
		if (component.kind == syntax::NameComponentKind::Operator)
			words.push_back(
				Word(kWordName, (static_cast<uint64_t>(component.op) << 1) | (component.array_form ? 1 : 0)));
		else if (component.kind != syntax::NameComponentKind::Identifier)
			return false;
		else if (!resolve || i != 0 || name.global)
			words.push_back(PointerWord(component.identifier));
		else
		{
			const LookupResult found = names_.unqualified(
				scope, component.identifier, name.qualified() ? LookupFilter::Prefix : LookupFilter::Ordinary);
			// A parameter without a type: its substitution failed (see
			// substituted_scope).
			if (found.ambiguous ||
				(found.found() && found.binding->kind == BindingKind::Parameter && found.binding->type == nullptr))
				return false;
			const Binding* binding = found.binding;
			if (constant != nullptr && found.found() && binding->kind == BindingKind::Enumerator &&
				name.components.size == 1 && !component.has_arguments &&
				static_cast<const EnumeratorEntity*>(binding->entity)->enumeration == nullptr &&
				static_cast<const EnumeratorEntity*>(binding->entity)->argument == nullptr)
			{
				*constant = static_cast<const EnumeratorEntity*>(binding->entity)->value;
				return true;
			}
			if (found.found() && body_ != nullptr && body_->this_type != nullptr && NonStaticMember(found))
			{
				words.push_back(Word(kWordThis, 1));
				words.push_back(PointerWord(body_->this_type->canonical));
				keyed_dependent_ = keyed_dependent_ || dependent_type(body_->this_type);
			}
			// A call's callee is the function it resolves to where one is
			// known (see resolved_specialization): a call's that is not
			// dependent, or the one function that is no template a class
			// member's name finds, which takes no argument-dependent lookup
			// (3.4.2p3; Clang names it whatever the arguments). Not one
			// named through `this`.
			const bool function_call = found.found() && call != nullptr && binding->kind == BindingKind::Function;
			const bool dependent_callee = function_call && this->dependent_arguments(*call, scope);
			const FunctionEntity* resolved = nullptr;
			FunctionEntity* only = function_call && semantic() &&
					!(body_ != nullptr && body_->this_type != nullptr && NonStaticMember(found))
				? SoleFunction(found)
				: nullptr;
			if (only != nullptr && !component.has_arguments && !only->templated &&
				(!dependent_callee || (only->owner != nullptr && only->owner->kind == ScopeKind::Class)))
				resolved = only;
			else if (only != nullptr && component.has_arguments && !dependent_callee && only->templated)
			{
				pending = only;
				pending_at = words.size();
			}
			// Another dependent callee (see above) is its spelling in the key
			// redeclarations compare (equivalence_keying_); a placeholder's
			// own key keeps its lookup, which each template does where it
			// is declared (14.6.4.1).
			if (dependent_callee)
			{
				keyed_dependent_ = true;
				keyed_callee_lookup_ = keyed_callee_lookup_ || (!equivalence_keying_ && resolved == nullptr);
			}
			keyed_callee_ = keyed_callee_ || dependent_callee || (!found.found() && call != nullptr);
			if (resolved != nullptr)
			{
				words.push_back(Word(kWordFunction, 1));
				words.push_back(PointerWord(resolved));
			}
			else if (!found.found() || (dependent_callee && equivalence_keying_))
			{
				words.push_back(kWordFreeName);
				words.push_back(PointerWord(component.identifier));
				keyed_dependent_ = true;
			}
			else if (binding->kind == BindingKind::Parameter && PrototypeScope(binding->entity->owner))
			{
				words.push_back(parameter_word(binding, scope));
				note_named_parameter(words.back());
				words.push_back(PointerWord(binding->type->canonical));
				keyed_dependent_ = keyed_dependent_ || dependent_type(binding->type);
			}
			else if (binding->entity != nullptr &&
				(binding->kind != BindingKind::TypeAlias || binding->entity->kind != EntityKind::TemplateParameter))
			{
				const Entity* entity = binding->entity;
				if (entity->kind == EntityKind::TemplateParameter)
				{
					entity = static_cast<const TemplateParameterEntity*>(entity)->canonical;
					keyed_dependent_ = true;
				}
				words.push_back(kWordEntity);
				words.push_back(PointerWord(entity));
			}
			else
			{
				// A type, or a frame's type parameter name: its argument.
				const Type* type = binding_type(binding, component.location)->canonical;
				words.push_back(kWordType);
				words.push_back(PointerWord(type));
				keyed_dependent_ = keyed_dependent_ || dependent_type(type);
			}
		}
		if (!component.has_arguments)
			continue;
		words.push_back(Word(kWordArguments, component.arguments.size));
		last_arguments = words.size();
		dependent_arguments = false;
		// Whether the arguments' dependence decides a resolution.
		const bool resolving = (i == 0 && pending != nullptr) ||
			(i + 1 == name.components.size && resolve && name.qualified() && semantic());
		for (const syntax::TemplateArgument& argument : component.arguments)
		{
			// A template-name argument (or its pack's expansion) is its
			// template.
			bool expansion = false;
			const syntax::Name* plain = PlainTemplateName(argument, &expansion);
			const Type* templ = plain != nullptr ? template_name_argument(*plain, scope, false) : nullptr;
			// A value argument is its value, or the placeholder of its
			// value-dependent expression (itself keyed, see template_value).
			const Type* type = templ != nullptr ? (expansion ? types_.expansion(templ) : templ)->canonical : nullptr;
			if (type == nullptr && argument.pack)
				return false;
			if (type == nullptr && argument.type == nullptr)
			{
				const Type* value = template_argument(argument, scope, component.location, nullptr);
				if (value == nullptr || failed())
					return false;
				type = value->canonical;
			}
			if (type == nullptr)
				type = type_id(*argument.type, scope)->canonical;
			words.push_back(PointerWord(type));
			if (resolving || !keyed_dependent_)
			{
				const bool dependent = dependent_type(type);
				keyed_dependent_ = keyed_dependent_ || dependent;
				dependent_arguments = dependent_arguments || dependent;
			}
		}
		// The callee named by a template-id of its one template.
		if (i == 0 && pending != nullptr && !dependent_arguments)
		{
			if (const FunctionEntity* resolved = resolved_specialization(
					pending, words.data() + last_arguments, component.arguments.size, component.location))
			{
				words[pending_at] = Word(kWordFunction, 1);
				words[pending_at + 1] = PointerWord(resolved);
			}
		}
	}
	return !resolve || !name.qualified() ||
		resolution_words(name, scope, name.last().has_arguments ? last_arguments : 0, dependent_arguments);
}

uint64_t Analyzer::parameter_word(const Binding* binding, const Scope* scope) const
{
	// A declarator's parameter named at `scope` (Itanium 5.1.6's function
	// parameter): its position in its clause, the parameters before it in
	// its scope and in the enclosing scopes of the clause's earlier
	// parameters (a later parameter's type sees them in scopes of their own,
	// see apply_suffix); and the parameter-declaration-clauses between the
	// reference and it, each one the reference is within counting one (L:
	// a later parameter's type is one function parameter scope deeper than
	// the trailing return type).
	// The positions are numbered once per scope (a parameter added to it
	// later numbers it again: those before keep theirs).
	const Scope* owner = binding->entity->owner;
	uint32_t index = parameter_positions_.find(binding->entity);
	if (index == PointerMap::kMissing)
	{
		uint32_t position = 0;
		for (const Scope* clause = owner; clause->guarded;)
		{
			clause = ClauseParent(clause);
			if (!PrototypeScope(clause) || !clause->guarded)
				break;
			for (const Binding* earlier = clause->bindings; earlier != nullptr; earlier = earlier->next)
				position += earlier->kind == BindingKind::Parameter ? 1 : 0;
		}
		for (const Binding* each = owner->bindings; each != nullptr; each = each->next)
		{
			if (each->kind == BindingKind::Parameter)
				parameter_positions_.set(each->entity, position++);
		}
		index = parameter_positions_.find(binding->entity);
	}
	uint64_t level = 0;
	bool within = false;
	for (const Scope* s = scope; s != nullptr; s = s->parent)
	{
		if (PrototypeScope(s))
		{
			level += s->guarded && !within ? 1 : 0;
			within = s->guarded;
		}
		if (s == owner)
			break;
	}
	return ParameterWord(index, level);
}

void Analyzer::note_named_parameter(uint64_t word)
{
	// A declarator parameter the operand being keyed names, by its position
	// in its clause (any clause: more named is only more substituted).
	const uint32_t index = ParameterIndex(word);
	if (named_parameters_ != nullptr)
		*named_parameters_ |= index < 63 ? 1ull << index : 0;
}

bool Analyzer::unnamed_parameter(const TemplateParameterEntity* operand, const Binding* binding)
{
	// Whether a declarator parameter of dependent type is one a keyed
	// placeholder's operand is known not to name (see substituted_scope).
	if (operand == nullptr || (operand->named_parameters & kNamedParametersKnown) == 0 ||
		!dependent_type(binding->type))
		return false;
	const uint32_t index = ParameterIndex(parameter_word(binding, binding->entity->owner));
	return index < 63 && (operand->named_parameters & 1ull << index) == 0;
}

bool Analyzer::resolution_words(const syntax::Name& name, Scope* scope, size_t last_arguments, bool dependent_arguments)
{
	// What a qualified name denotes where the placeholder is made, which its
	// mangling names (5.1.6): its dependent qualifier (an unresolved name),
	// functions (by the last component, which the key spells; not by a
	// declaration later overloads may change), or the entity. The lookup is
	// the one the expression's analysis made; a substituted copy's may fail
	// (a failure of the keying, see equivalent_placeholder).
	std::vector<uint64_t>& words = decltype_words_;
	if (name.last().kind != syntax::NameComponentKind::Identifier)
	{
		words.push_back(kWordAbsent);
		return true;
	}
	const NamePrefix saved = last_prefix_;
	const LookupResult found = lookup_name(name, scope, LookupFilter::Ordinary);
	const NamePrefix prefix = last_prefix_;
	last_prefix_ = saved;
	if (failed())
		return false;
	if (prefix.dependent != nullptr)
	{
		words.push_back(kWordType);
		words.push_back(PointerWord(prefix.dependent->canonical));
		keyed_dependent_ = true;
	}
	else if (!found.found() || found.binding->entity == nullptr || IsDependentMember(found.binding))
	{
		words.push_back(kWordAbsent);
		keyed_dependent_ = keyed_dependent_ || (found.found() && IsDependentMember(found.binding));
	}
	else if (found.binding->kind == BindingKind::Function)
	{
		// One function that is no template is what the name denotes (a
		// qualified name takes no argument-dependent lookup: the hosts
		// mangle the function, L <encoding> E); an overload set or a
		// template its last component.
		const FunctionList* set = found.functions;
		const Entity* function = set != nullptr ? set->binding->entity : found.binding->entity;
		if ((set == nullptr || set->next == nullptr) && !name.last().has_arguments &&
			!static_cast<const FunctionEntity*>(function)->templated)
		{
			words.push_back(Word(kWordFunction, 1));
			words.push_back(PointerWord(function));
		}
		else if (const FunctionEntity* resolved = semantic() && last_arguments != 0 && !dependent_arguments &&
					SoleFunction(found) != nullptr && SoleFunction(found)->templated
				? resolved_specialization(SoleFunction(found), words.data() + last_arguments,
					  name.last().arguments.size, name.last().location)
				: nullptr)
		{
			words.push_back(Word(kWordFunction, 1));
			words.push_back(PointerWord(resolved));
		}
		else
			words.push_back(Word(kWordFunction, 0));
	}
	else
	{
		words.push_back(kWordEntity);
		words.push_back(PointerWord(found.binding->entity));
	}
	return true;
}

FunctionEntity* Analyzer::SoleFunction(const LookupResult& found)
{
	// The one function a lookup found, or null (none, or an overload set).
	if (!found.found() || (found.functions != nullptr && found.functions->next != nullptr))
		return nullptr;
	Entity* function = found.functions != nullptr ? found.functions->binding->entity : found.binding->entity;
	return function != nullptr && function->kind == EntityKind::Function ? static_cast<FunctionEntity*>(function)
																		 : nullptr;
}

const FunctionEntity* Analyzer::resolved_specialization(
	FunctionEntity* function, const uint64_t* arguments, uint32_t count, uint32_t location)
{
	// The function a name that is not dependent denotes where it appears
	// (14.6p10), the callee of a call without type-dependent arguments
	// (14.6.2p1), as Clang resolves it there and mangles it (L <encoding>
	// E, 5.1.6): one function that is no template (see name_words), or, as
	// here, the specialization of one function template whose explicit
	// arguments (their keyed canonical types, none dependent) give each of
	// its parameters. Null for another (an overload set, a specialization
	// its call's arguments deduce: Clang's overload resolution is not
	// repeated here), which the name spells.
	const TemplateHead* head = HeadOf(function);
	if (head == nullptr)
		return nullptr;
	TypeList given;
	for (uint32_t i = 0; i < count; ++i)
		given.push_back(reinterpret_cast<const Type*>(static_cast<uintptr_t>(arguments[i])));
	TypeList converted;
	if (!match_template_arguments(head, given.data(), count, converted, false, location, true) ||
		converted.size() != head->count || std::find(converted.begin(), converted.end(), nullptr) != converted.end())
		return nullptr;
	return deduced_specialization(function, converted);
}

bool Analyzer::decltype_words(const syntax::Expr* expr, Scope* scope)
{
	// A decltype operand's structure (14.5.6.1p5: expressions are
	// equivalent when their tokens would satisfy the ODR, template
	// parameters compared by position): false for a form it does not
	// cover, or for a nesting past half the stack's budget (the operand
	// then has a placeholder of its own; its analysis checks the budget).
	std::vector<uint64_t>& words = decltype_words_;
	if (expr == nullptr)
	{
		words.push_back(kWordAbsent);
		return true;
	}
	if (stack_half_used())
		return false;
	const uint64_t kind = static_cast<uint64_t>(expr->kind);
	switch (expr->kind)
	{
	case syntax::ExprKind::Id:
	{
		const size_t start = words.size();
		words.push_back(Word(kWordNode, kind));
		Constant constant = Constant{ 0, nullptr };
		if (!name_words(*static_cast<const syntax::IdExpr*>(expr)->name, scope, true, &constant))
			return false;
		if (constant.type != nullptr)
		{
			words.resize(start);
			ConstantWords(words, constant);
		}
		return true;
	}
	case syntax::ExprKind::Literal:
	{
		// An integer or character literal is its value (see ConstantWords);
		// another its spelling, packed eight bytes to a word, then its type
		// and value (for the mangling, 5.1.6.1), which the spelling decides.
		const TokenSpelling& spelling = static_cast<const syntax::LiteralExpr*>(expr)->spelling;
		FundamentalType type = FundamentalType::Int;
		uint64_t bits = 0;
		bool integral = false;
		const bool typed = LiteralValue(spelling, type, bits, integral);
		if (typed && integral)
		{
			ConstantWords(words, Constant{ bits, types_.fundamental(type) });
			return true;
		}
		words.push_back(Word(kWordLiteral, spelling.length));
		for (uint32_t i = 0; i < spelling.length; i += 8)
		{
			uint64_t packed = 0;
			std::memcpy(&packed, spelling.text + i, spelling.length - i < 8 ? spelling.length - i : 8);
			words.push_back(packed);
		}
		words.push_back(typed ? PointerWord(types_.fundamental(type)) : 0);
		words.push_back(typed ? bits : 0);
		return true;
	}
	case syntax::ExprKind::KeywordLiteral:
	{
		// `this` is its type, `cv X*` in a cv member function's trailing
		// return type (5.1.1p3), elsewhere its class's (the innermost
		// enclosing one).
		const TokenKind keyword = static_cast<const syntax::KeywordLiteralExpr*>(expr)->keyword;
		if (keyword == TokenKind::KW_TRUE || keyword == TokenKind::KW_FALSE)
		{
			const uint64_t value = keyword == TokenKind::KW_TRUE ? 1 : 0;
			ConstantWords(words, Constant{ value, types_.fundamental(FundamentalType::Bool) });
			return true;
		}
		words.push_back(Word(kWordNode, kind | static_cast<uint64_t>(keyword) << 8));
		if (keyword == TokenKind::KW_THIS && body_ != nullptr && body_->this_type != nullptr)
		{
			words.push_back(Word(kWordThis, 1));
			words.push_back(PointerWord(body_->this_type->canonical));
			keyed_dependent_ = keyed_dependent_ || dependent_type(body_->this_type);
		}
		else if (keyword == TokenKind::KW_THIS)
		{
			const Scope* cls = scope;
			while (cls != nullptr && cls->kind != ScopeKind::Class)
				cls = cls->parent;
			words.push_back(Word(kWordThis, 0));
			words.push_back(PointerWord(cls != nullptr ? cls->entity : nullptr));
			keyed_dependent_ = keyed_dependent_ || TemplatedClassScope(scope);
		}
		return true;
	}
	case syntax::ExprKind::Paren:
		words.push_back(Word(kWordNode, kind));
		return decltype_words(static_cast<const syntax::ParenExpr*>(expr)->inner, scope);
	case syntax::ExprKind::Binary:
	case syntax::ExprKind::Assign:
	{
		const syntax::BinaryExpr* binary = static_cast<const syntax::BinaryExpr*>(expr);
		words.push_back(Word(kWordNode, kind | static_cast<uint64_t>(binary->op) << 8));
		return decltype_words(binary->left, scope) && decltype_words(binary->right, scope);
	}
	case syntax::ExprKind::Conditional:
	{
		const syntax::ConditionalExpr* conditional = static_cast<const syntax::ConditionalExpr*>(expr);
		words.push_back(Word(kWordNode, kind));
		return decltype_words(conditional->condition, scope) && decltype_words(conditional->then_expr, scope) &&
			decltype_words(conditional->else_expr, scope);
	}
	case syntax::ExprKind::Unary:
	case syntax::ExprKind::Postfix:
	{
		const syntax::UnaryExpr* unary = static_cast<const syntax::UnaryExpr*>(expr);
		words.push_back(Word(kWordNode, kind | static_cast<uint64_t>(unary->op) << 8));
		return decltype_words(unary->operand, scope);
	}
	case syntax::ExprKind::Call:
	{
		// An unqualified callee may be a dependent name (see name_words).
		const syntax::CallExpr* call = static_cast<const syntax::CallExpr*>(expr);
		words.push_back(Word(kWordNode, kind | static_cast<uint64_t>(call->arguments.size) << 8));
		const syntax::Name* callee = call->callee->kind == syntax::ExprKind::Id
			? static_cast<const syntax::IdExpr*>(call->callee)->name
			: nullptr;
		if (callee != nullptr && !callee->qualified() && !callee->global)
		{
			words.push_back(Word(kWordNode, static_cast<uint64_t>(syntax::ExprKind::Id)));
			if (!name_words(*callee, scope, true, nullptr, call))
				return false;
		}
		else if (!decltype_words(call->callee, scope))
			return false;
		for (const syntax::Expr* argument : call->arguments)
		{
			if (!decltype_words(argument, scope))
				return false;
		}
		return true;
	}
	case syntax::ExprKind::Subscript:
	{
		const syntax::SubscriptExpr* subscript = static_cast<const syntax::SubscriptExpr*>(expr);
		words.push_back(Word(kWordNode, kind));
		return decltype_words(subscript->base, scope) && decltype_words(subscript->index, scope);
	}
	case syntax::ExprKind::Member:
	{
		// The member's name is looked up in its object's type: spelled.
		const syntax::MemberExpr* member = static_cast<const syntax::MemberExpr*>(expr);
		words.push_back(Word(kWordNode, kind | static_cast<uint64_t>(member->op) << 8));
		return decltype_words(member->base, scope) && name_words(*member->member, scope, false);
	}
	case syntax::ExprKind::Cast:
	{
		const syntax::CastExpr* cast = static_cast<const syntax::CastExpr*>(expr);
		const Type* type = type_id(*cast->type, scope)->canonical;
		words.push_back(Word(kWordNode, kind | static_cast<uint64_t>(cast->cast) << 8));
		words.push_back(PointerWord(type));
		keyed_dependent_ = keyed_dependent_ || dependent_type(type);
		return decltype_words(cast->operand, scope);
	}
	case syntax::ExprKind::FunctionalCast:
	{
		// T(args) or T{...} (5.2.3): its canonical type, then its operands.
		const syntax::FunctionalCastExpr* cast = static_cast<const syntax::FunctionalCastExpr*>(expr);
		Specifiers specifiers;
		this->specifiers(cast->type, scope, specifiers, false);
		if (specifiers.type == nullptr)
			return false;
		words.push_back(Word(kWordNode,
			kind | static_cast<uint64_t>(cast->arguments.size) << 8 | (cast->braced != nullptr ? 1u : 0u) << 24));
		words.push_back(PointerWord(specifiers.type->canonical));
		keyed_dependent_ = keyed_dependent_ || dependent_type(specifiers.type);
		if (cast->braced != nullptr)
			return decltype_words(cast->braced, scope);
		for (const syntax::Expr* argument : cast->arguments)
		{
			if (!decltype_words(argument, scope))
				return false;
		}
		return true;
	}
	case syntax::ExprKind::SizeofPack:
	{
		// sizeof...(P): the pack by what lookup finds (a template parameter
		// by its position, a function parameter pack as named, flagged).
		const LookupResult found = names_.unqualified(
			scope, static_cast<const syntax::SizeofPackExpr*>(expr)->pack, LookupFilter::Ordinary);
		if (!found.found() || found.ambiguous || found.binding->entity == nullptr)
			return false;
		keyed_sizeof_pack_ = true;
		if (found.binding->kind == BindingKind::Parameter && found.binding->type != nullptr &&
			PrototypeScope(found.binding->entity->owner))
		{
			words.push_back(Word(kWordNode, kind | 1u << 8));
			words.push_back(parameter_word(found.binding, scope));
			note_named_parameter(words.back());
			words.push_back(PointerWord(found.binding->type->canonical));
			keyed_dependent_ = true;
			return true;
		}
		if (found.binding->entity->kind != EntityKind::TemplateParameter)
			return false;
		words.push_back(Word(kWordNode, kind));
		words.push_back(PointerWord(static_cast<const TemplateParameterEntity*>(found.binding->entity)->canonical));
		keyed_dependent_ = true;
		return true;
	}
	case syntax::ExprKind::PackExpansion:
		words.push_back(Word(kWordNode, kind));
		return decltype_words(static_cast<const syntax::PackExpansionExpr*>(expr)->pattern, scope);
	case syntax::ExprKind::Fold:
	{
		// The operator and direction, then the operands.
		const syntax::FoldExpr* fold = static_cast<const syntax::FoldExpr*>(expr);
		words.push_back(Word(kWordNode,
			kind | static_cast<uint32_t>(fold->op) << 8 | (fold->right ? 1u : 0u) << 16 |
				(fold->init != nullptr ? 1u : 0u) << 17));
		return decltype_words(fold->pattern, scope) && (fold->init == nullptr || decltype_words(fold->init, scope));
	}
	case syntax::ExprKind::Throw:
	{
		// `throw e`, or a rethrow (flagged above the kind).
		const syntax::Expr* operand = static_cast<const syntax::ThrowExpr*>(expr)->operand;
		words.push_back(Word(kWordNode, kind | (operand == nullptr ? 1u : 0u) << 8));
		return operand == nullptr || decltype_words(operand, scope);
	}
	case syntax::ExprKind::New:
	{
		// The placement count, `::`, an array's first bound and the
		// initializer's form (1 parenthesized, 2 braced) and count above the
		// kind; the canonical type the rest of the new-type-id gives (5.3.4p6,
		// see allocation), then the bound, the placement arguments and the
		// initializer's.
		const syntax::NewExpr* made = static_cast<const syntax::NewExpr*>(expr);
		syntax::TypeId type_syntax = *made->type;
		syntax::Declarator declarator = syntax::Declarator();
		const syntax::Expr* bound = nullptr;
		if (type_syntax.declarator != nullptr && !type_syntax.declarator->suffixes.empty() &&
			type_syntax.declarator->suffixes[0].kind == syntax::DeclaratorSuffixKind::Array)
		{
			bound = type_syntax.declarator->suffixes[0].bound;
			if (bound == nullptr)
				return false;
			declarator = *type_syntax.declarator;
			declarator.suffixes.data += 1;
			declarator.suffixes.size -= 1;
			type_syntax.declarator = &declarator;
		}
		const syntax::Initializer* initializer = made->initializer;
		const syntax::BracedInitList* list = nullptr;
		uint64_t form = 0;
		uint64_t count = 0;
		if (initializer != nullptr && initializer->kind == syntax::InitializerKind::Parenthesized)
		{
			form = 1;
			count = initializer->arguments.size;
		}
		else if (initializer != nullptr && initializer->kind == syntax::InitializerKind::Braced &&
			initializer->expression != nullptr && initializer->expression->kind == syntax::ExprKind::BracedInit)
		{
			form = 2;
			list = static_cast<const syntax::BracedInitList*>(initializer->expression);
			count = list->elements.size;
		}
		else if (initializer != nullptr)
			return false;
		if (made->placement.size > 0xffff || count > 0xffff)
			return false;
		const Type* type = type_id(type_syntax, scope);
		if (type == nullptr)
			return false;
		words.push_back(Word(kWordNode,
			kind | static_cast<uint64_t>(made->placement.size) << 8 | (made->global ? 1u : 0u) << 24 |
				(bound != nullptr ? 1u : 0u) << 25 | form << 26 | count << 28));
		words.push_back(PointerWord(type->canonical));
		keyed_dependent_ = keyed_dependent_ || dependent_type(type);
		if (bound != nullptr && !decltype_words(bound, scope))
			return false;
		for (const syntax::Expr* argument : made->placement)
		{
			if (!decltype_words(argument, scope))
				return false;
		}
		for (uint64_t i = 0; i < count; ++i)
		{
			if (!decltype_words(list != nullptr ? list->elements[i] : initializer->arguments[i], scope))
				return false;
		}
		return true;
	}
	case syntax::ExprKind::Delete:
	{
		// `::` and the array form above the kind, then the operand.
		const syntax::DeleteExpr* freed = static_cast<const syntax::DeleteExpr*>(expr);
		words.push_back(Word(kWordNode, kind | (freed->global ? 1u : 0u) << 8 | (freed->array ? 1u : 0u) << 9));
		return decltype_words(freed->operand, scope);
	}
	case syntax::ExprKind::Sizeof:
	case syntax::ExprKind::TypeTrait:
		return size_words(*expr, scope);
	case syntax::ExprKind::BuiltinTrait:
		return trait_words(*static_cast<const syntax::BuiltinTraitExpr*>(expr), scope);
	case syntax::ExprKind::BracedInit:
	{
		const syntax::BracedInitList* list = static_cast<const syntax::BracedInitList*>(expr);
		words.push_back(Word(kWordNode, kind | static_cast<uint64_t>(list->elements.size) << 8));
		for (const syntax::Expr* element : list->elements)
		{
			if (!decltype_words(element, scope))
				return false;
		}
		return true;
	}
	default:
		return false;
	}
}

bool Analyzer::size_words(const syntax::Expr& expr, Scope* scope)
{
	// sizeof and alignof of a type-id (its canonical type) or of an
	// expression (a dependent array bound's usual forms; decltype_words).
	std::vector<uint64_t>& words = decltype_words_;
	const bool size = expr.kind == syntax::ExprKind::Sizeof;
	const syntax::TypeId* type = size ? static_cast<const syntax::SizeofExpr&>(expr).type
									  : static_cast<const syntax::TypeTraitExpr&>(expr).type;
	const syntax::Expr* operand = size ? static_cast<const syntax::SizeofExpr&>(expr).operand
									   : static_cast<const syntax::TypeTraitExpr&>(expr).operand;
	const uint64_t keyword = size ? 0 : static_cast<uint64_t>(static_cast<const syntax::TypeTraitExpr&>(expr).keyword);
	// Itanium 5.1.6 (as Clang mangles it): an operand that is not
	// instantiation-dependent is encoded as the operator's value, an integer
	// literal of std::size_t's type (sizeof(x) of an int parameter x in a
	// dependent bound is Lm4E); a type operand's is read as the key's type
	// (the mangling's type_query).
	Constant value;
	if (type == nullptr && operand != nullptr && !instantiation_dependent(*operand, scope) &&
		constant_value(expr, scope, value, false))
	{
		ConstantWords(words, value);
		return true;
	}
	words.push_back(
		Word(kWordNode, static_cast<uint64_t>(expr.kind) | keyword << 8 | (type != nullptr ? 1u : 0u) << 24));
	if (type != nullptr)
	{
		const Type* operand_type = type_id(*type, scope)->canonical;
		words.push_back(PointerWord(operand_type));
		keyed_dependent_ = keyed_dependent_ || dependent_type(operand_type);
		return true;
	}
	return decltype_words(operand, scope);
}

bool Analyzer::trait_words(const syntax::BuiltinTraitExpr& trait, Scope* scope)
{
	// A GNU trait of its operands' canonical types (decltype_words).
	std::vector<uint64_t>& words = decltype_words_;
	words.push_back(Word(kWordNode,
		static_cast<uint64_t>(syntax::ExprKind::BuiltinTrait) | static_cast<uint64_t>(trait.trait) << 8 |
			static_cast<uint64_t>(trait.types.size) << 16));
	for (const syntax::TypeId* type : trait.types)
	{
		const Type* operand_type = type_id(*type, scope);
		if (operand_type == nullptr)
			return false;
		words.push_back(PointerWord(operand_type->canonical));
		keyed_dependent_ = keyed_dependent_ || dependent_type(operand_type);
	}
	return trait.operand == nullptr || decltype_words(trait.operand, scope);
}

bool Analyzer::dependent_arguments(const syntax::CallExpr& call, Scope* scope)
{
	// 14.6.2p1: an argument that is type-dependent or a pack expansion
	// makes an unqualified callee a dependent name.
	for (const syntax::Expr* argument : call.arguments)
	{
		if (type_dependent(argument, scope))
			return true;
	}
	return false;
}

DecltypeKey Analyzer::kept_key(const DecltypeKey& key)
{
	// A placeholder's key kept in the arena after its word count (the
	// placeholder's `key`, which its mangling reads).
	uint64_t* kept = static_cast<uint64_t*>(arena_.allocate((key.count + 1) * sizeof(uint64_t), alignof(uint64_t)));
	kept[0] = key.count;
	std::memcpy(kept + 1, key.words, key.count * sizeof(uint64_t));
	return DecltypeKey{ kept + 1, key.count };
}

const Type* Analyzer::dependent_decltype(const syntax::Expr& operand, Scope* scope)
{
	return dependent_expression(operand, scope, false);
}

const Type* Analyzer::dependent_expression(
	const syntax::Expr& operand, Scope* scope, bool value, const Type* target, bool fixed)
{
	// A decltype-specifier of a template definition, or a value-dependent
	// template argument (`value`): one placeholder per equivalent operand
	// (14.5.6.1p5-p6: an equivalent redeclaration has the same type, and
	// equivalent template-ids name one specialization, 14.4), keyed by the
	// operand's structure in the context of its namespace and class (see
	// decltype_words); an operand of a form the key does not cover has its
	// own. Its non-dependent names are bound where it appears (14.6p10,
	// 14.6.3), as a template body's are.
	if (template_depth_ != 0)
		expression_walk(&operand, scope, !value);
	const size_t base = decltype_words_.size();
	// The decl-specifiers and a member template's own template-head of an
	// out-of-class member declaration (before its declarator-id) redeclare
	// the member its class declared: keyed in that class (and namespace),
	// as its declaration there was (14.5.6.1p5, see placeholder_home_).
	// The declarator parameters it names, those of the placeholders in it
	// too (an enclosing operand's key being built names them as well).
	uint64_t* const enclosing = named_parameters_;
	uint64_t named = 0;
	named_parameters_ = &named;
	// Whether it applies sizeof... (a placeholder in it too, whose words
	// the enclosing operand's key being built then gets).
	const bool enclosing_sizeof = keyed_sizeof_pack_;
	keyed_sizeof_pack_ = false;
	const bool keyed = placeholder_words(
		operand, scope, placeholder_home_ != nullptr ? placeholder_home_ : scope, value, target);
	named_parameters_ = enclosing;
	const bool sizeof_packs = keyed_sizeof_pack_ || !keyed;
	keyed_sizeof_pack_ = enclosing_sizeof || sizeof_packs;
	DecltypeKey key = DecltypeKey{ decltype_words_.data() + base,
		static_cast<uint32_t>(decltype_words_.size() - base) };
	// An operand that uses `this` (or names a non-static member through
	// it) has it in its key: a placeholder found is one of the same `this`,
	// or of an operand that uses none.
	const bool callee_lookup = keyed_callee_lookup_;
	const bool callee = keyed_callee_;
	keyed_callee_lookup_ = false;
	keyed_callee_ = false;
	// A type transform of a type that is not dependent (its key names no
	// dependent type) is the type it gives, as Clang takes it in a
	// template's signature: an enclosing operand's key and the mangling
	// spell that type, not a placeholder naming nothing dependent (an
	// instance's member template's `__remove_cv(__remove_cv(T))`, T
	// known). The key's word after the trait's is its operand's type.
	const syntax::Expr* bare = &operand;
	while (bare->kind == syntax::ExprKind::Paren)
		bare = static_cast<const syntax::ParenExpr*>(bare)->inner;
	// A decltype-specifier (`fixed`) whose operand names nothing dependent
	// and is not type-dependent is the same type in every instantiation
	// (14.6.2.2p1): typed where it appears (14.6p10), so that a template's
	// signature has its result type at definition time (14.5.6.1p5, p6:
	// redeclarations then compare types, not the spellings of fixed
	// operands).
	// Body-local operands deliberately have no shared structural key. Their
	// dependence is still determined from the names and types they use.
	bool independent = false;
	if (fixed && !value && (!keyed || (key.words[0] & 1) == 0))
	{
		const bool precise = precise_dependence_;
		precise_dependence_ = precise || !keyed;
		independent = (keyed || !instantiation_dependent(operand, scope)) && !type_dependent(&operand, scope);
		precise_dependence_ = precise;
	}
	if (independent)
	{
		decltype_words_.resize(base);
		if (enclosing != nullptr)
			*enclosing |= named;
		const uint32_t depth = template_depth_;
		template_depth_ = 0;
		const Type* type = decltype_type(operand, scope);
		template_depth_ = depth;
		return type;
	}
	if (keyed && !value && (key.words[0] & 1) == 0 && bare->kind == syntax::ExprKind::BuiltinTrait &&
		IsGnuTypeTransform(static_cast<const syntax::BuiltinTraitExpr*>(bare)->trait) &&
		key.count == kPlaceholderKeyPrefix + 2)
	{
		const Type* type = reinterpret_cast<const Type*>(static_cast<uintptr_t>(key.words[kPlaceholderKeyPrefix + 1]));
		decltype_words_.resize(base);
		if (enclosing != nullptr)
			*enclosing |= named;
		return trait_type(static_cast<const syntax::BuiltinTraitExpr*>(bare)->trait, type, bare->location);
	}
	// A member template's declaration in an instantiated class: its
	// operands that no longer depend on anything are typed now (see
	// check_fixed_operands; the key's words may move).
	if (fixed && !value && !class_instantiations_.empty())
	{
		check_fixed_operands(operand, scope, 0);
		key = DecltypeKey{ decltype_words_.data() + base, static_cast<uint32_t>(decltype_words_.size() - base) };
	}
	if (keyed)
	{
		if (TemplateParameterEntity** found = decltype_placeholders_.find(key))
		{
			decltype_words_.resize(base);
			keyed_sizeof_pack_ = enclosing_sizeof || (*found)->sizeof_packs;
			if (enclosing != nullptr)
				*enclosing |= ((*found)->named_parameters & kNamedParametersKnown) != 0
					? (*found)->named_parameters & ~kNamedParametersKnown
					: kNamedParametersKnown;
			return (*found)->type;
		}
	}
	// Bit 63 of `named` (some unknown) or an operand the key does not cover
	// leaves them unknown.
	if (enclosing != nullptr)
		*enclosing |= keyed ? named : kNamedParametersKnown;
	TemplateParameterEntity* entity = new_entity<TemplateParameterEntity>(
		EntityKind::TemplateParameter, value ? value_key_ : decltype_key_, nullptr, nullptr);
	++placeholders_made_;
	entity->non_type = value;
	entity->canonical = entity;
	entity->value_type = target;
	entity->ordinal = definition_point();
	entity->this_type = body_ != nullptr ? body_->this_type : nullptr;
	entity->expression = &operand;
	entity->expression_scope = scope;
	entity->head_depth = HeadDepth(scope);
	entity->packs = expression_packs(operand, scope, &entity->nested_packs);
	entity->sizeof_packs = sizeof_packs || entity->nested_packs != nullptr;
	entity->friend_keyed = namespace_friend_class_ != nullptr;
	entity->fixed = value && fixed && keyed && (key.words[0] & 1) == 0 && !type_dependent(&operand, scope) &&
		(target == nullptr || !dependent_type(target));
	const syntax::Expr* inner = &operand;
	while (inner->kind == syntax::ExprKind::Paren)
		inner = static_cast<const syntax::ParenExpr*>(inner)->inner;
	const syntax::Name* name = inner->kind == syntax::ExprKind::Id ? static_cast<const syntax::IdExpr*>(inner)->name
																   : nullptr;
	if (name != nullptr && !name->qualified() && !name->last().has_arguments &&
		name->last().kind == syntax::NameComponentKind::Identifier)
	{
		const LookupResult found = names_.unqualified(scope, name->last().identifier, LookupFilter::Ordinary);
		if (found.found() && found.binding->kind == BindingKind::TemplateValue)
			entity->named = static_cast<const TemplateParameterEntity*>(found.binding->entity)->canonical;
	}
	entity->type = types_.named(TypeKind::TemplateParameter, entity);
	if (keyed)
	{
		entity->named_parameters = (named & kNamedParametersKnown) != 0 ? 0 : named | kNamedParametersKnown;
		key = kept_key(key);
		entity->key = key.words - 1;
		decltype_placeholders_.insert(key, entity);
		if (callee)
			equivalent_representative(entity, operand, scope, value, target, callee_lookup);
	}
	decltype_words_.resize(base);
	return entity->type;
}

void Analyzer::equivalent_representative(TemplateParameterEntity* entity, const syntax::Expr& operand, Scope* scope,
	bool value, const Type* target, bool lookup)
{
	// A placeholder whose key names a dependent callee by its lookup (a
	// distinct template declared where the callee finds other functions
	// is a distinct placeholder, 14.6.4.1) is equivalent, as redeclarations
	// of one template compare it, to the operands of its structure with the
	// callee's spelling alone (14.5.6.1p5: "only the name itself is
	// considered ... the result for the first declaration is used"): the
	// first such placeholder made represents them (declared_type puts it in
	// their place), and its key, which spells the callee as the mangling
	// does (an unresolved name), is theirs for the mangling. Without a
	// callee named by its lookup (`lookup`), its own key is that key.
	const size_t base = decltype_words_.size();
	bool keyed = true;
	DecltypeKey key = DecltypeKey{ entity->key + 1, static_cast<uint32_t>(entity->key[0]) };
	if (lookup)
	{
		uint64_t* const enclosing = named_parameters_;
		named_parameters_ = nullptr;
		equivalence_keying_ = true;
		keyed = placeholder_words(
			operand, scope, placeholder_home_ != nullptr ? placeholder_home_ : scope, value, target);
		equivalence_keying_ = false;
		named_parameters_ = enclosing;
		keyed_callee_lookup_ = false;
		keyed_callee_ = false;
		key = DecltypeKey{ decltype_words_.data() + base, static_cast<uint32_t>(decltype_words_.size() - base) };
	}
	if (keyed)
	{
		if (TemplateParameterEntity** found = decltype_equivalents_.find(key))
		{
			placeholder_representatives_.set(entity, static_cast<uint32_t>(representatives_.size()));
			representatives_.push_back(*found);
			entity->key = (*found)->key;
		}
		else
		{
			if (lookup)
			{
				key = kept_key(key);
				entity->key = key.words - 1;
			}
			decltype_equivalents_.insert(key, entity);
		}
	}
	decltype_words_.resize(base);
}

bool Analyzer::placeholder_words(
	const syntax::Expr& operand, Scope* scope, const Scope* home, bool value, const Type* target)
{
	// A placeholder's key (see dependent_expression) in decltype_words_:
	// the namespace and class of `home`, its kind, a pointer or reference
	// parameter's type (which resolves an overloaded function's name,
	// 14.3.2p5; 0 for none), and the operand's structure (after these
	// kPlaceholderKeyPrefix words), the namespace's word marked (bit 0)
	// when the operand names a dependent entity or type (instantiation-
	// dependent: otherwise, as a decltype of a template's signature, it is
	// its type, 5.1.6); false when it has none. A
	// placeholder of a function body (a local array's dependent bound) is
	// never substituted (each instantiation analyzes its copy of the body)
	// nor redeclared: its own, since a shared one's scope must be one a
	// substitution can rebuild.
	bool in_body = false;
	for (const Scope* s = scope; s != nullptr && s->kind != ScopeKind::TemplateParameters && !in_body; s = s->parent)
		in_body = s->kind == ScopeKind::Block || (s->kind == ScopeKind::Function && s->entity != nullptr);
	const Scope* space = home;
	while (space != nullptr && space->kind != ScopeKind::Namespace)
		space = space->parent;
	const Scope* cls = home;
	while (cls != nullptr && cls->kind != ScopeKind::Class)
		cls = cls->parent;
	// A namespace member's friend template declaration redeclares the
	// namespace's (14.5.4, 7.3.1.2p3): keyed there, as the namespace's own
	// declarations are (see namespace_friend_class_).
	if (cls != nullptr && cls == namespace_friend_class_)
		cls = nullptr;
	const size_t base = decltype_words_.size();
	decltype_words_.push_back(PointerWord(space));
	decltype_words_.push_back(PointerWord(cls) | (value ? 1 : 0));
	decltype_words_.push_back(target != nullptr ? PointerWord(target->canonical) : 0);
	if (in_body)
		return false;
	const bool enclosing = keyed_dependent_;
	keyed_dependent_ = false;
	const bool keyed = decltype_words(&operand, scope);
	decltype_words_[base] |= keyed_dependent_ ? 1 : 0;
	keyed_dependent_ = enclosing;
	return keyed;
}

const Type* Analyzer::equivalent_placeholder(const TemplateParameterEntity* placeholder, uint32_t depth,
	const Type* const* canonical, uint32_t count, DecltypeKey& key, bool& shared)
{
	// A placeholder substituted with dependent arguments is equivalent to
	// the operand written with them (14.5.6.1p5-p6: an alias template's
	// `__and_<Cond...>::value` with Cond known is the operand
	// `__and_<A, B>::value`): the copy its arguments make is keyed as that
	// operand would be, in the placeholder's own context. The placeholder
	// found, or null with `key` set (arena words) for the one the caller
	// makes, which an equivalent operand declared later then finds when
	// its arguments name no placeholder (`shared`; the key of one they do
	// name is the placeholder's own, for its mangling). The copy is only
	// keyed: a failure it meets is not the substitution's.
	if (placeholder->head_depth == kHeadsSeveral || placeholder->this_type != nullptr)
		return nullptr;
	const SubstitutionFailure failure = failure_;
	const uint32_t limit = names_.limit();
	++soft_substitution_;
	failure_ = SubstitutionFailure();
	names_.set_limit(placeholder->ordinal != 0 ? placeholder->ordinal : limit);
	// The copy names no declarator parameters of an operand being keyed.
	uint64_t* const enclosing = named_parameters_;
	named_parameters_ = nullptr;
	Scope* scope = substituted_scope(
		placeholder->expression_scope, Substitution{ depth, canonical, count, nullptr }, placeholder);
	const syntax::Expr* copy = nullptr;
	if (!failed())
	{
		FrameExpander expander(*this, scope);
		syntax::SyntaxCloner cloner(arena_, true, expander.get());
		copy = cloner.expr(placeholder->expression);
	}
	// Keyed without a `this` (the placeholder has none), not the one of the
	// body being analyzed.
	const size_t base = decltype_words_.size();
	BodyContext* body = body_;
	body_ = nullptr;
	const bool keyed = copy != nullptr && !failed() &&
		placeholder_words(
			*copy, scope, placeholder->expression_scope, placeholder->non_type, placeholder->value_type) &&
		!failed();
	body_ = body;
	named_parameters_ = enclosing;
	names_.set_limit(limit);
	--soft_substitution_;
	failure_ = failure;
	const Type* found = nullptr;
	if (keyed)
	{
		DecltypeKey probe = DecltypeKey{ decltype_words_.data() + base,
			static_cast<uint32_t>(decltype_words_.size() - base) };
		if (TemplateParameterEntity** entry = decltype_placeholders_.find(probe))
		{
			// An operand's own placeholder, which a substitution in this
			// context applies to it as to the copy: not one keyed in an
			// instantiation's frame, whose bound names a later substitution
			// would not reach (see substitute_value), nor a pending one,
			// whose recorded arguments (some the operand may not name) are
			// another context's.
			bool framed = (*entry)->pending != nullptr;
			for (const Scope* s = (*entry)->expression_scope; s != nullptr && !framed; s = s->parent)
				framed = s->kind == ScopeKind::TemplateParameters && !s->dumped && s->lists == 0;
			found = framed ? nullptr : (*entry)->type;
			// Not shared, the one the caller makes still has the operand's
			// key, for its mangling (a nested member alias template's
			// `ge<T, U>::value` substituted in its class's instance).
			if (framed)
				key = DecltypeKey{ (*entry)->key + 1, static_cast<uint32_t>((*entry)->key[0]) };
		}
		else
		{
			key = kept_key(probe);
			shared = !names_placeholder(types_.pack(canonical, count), 0);
		}
	}
	decltype_words_.resize(base);
	return found;
}

bool Analyzer::names_placeholder(const Type* type, uint32_t level) const
{
	// Whether a type names a dependent name or expression (a substitution of
	// which may fail), rather than template parameters alone: the pending
	// placeholder of arguments that do not is one an equivalent operand
	// may share, whatever parts of them the operand names (see
	// equivalent_placeholder). Deep types answer yes.
	if (level > 16)
		return true;
	switch (type->kind)
	{
	case TypeKind::TemplateParameter:
		return IsPlaceholder(static_cast<const TemplateParameterEntity*>(type->entity));
	case TypeKind::Pointer:
	case TypeKind::LValueReference:
	case TypeKind::RValueReference:
	case TypeKind::Expansion:
		return names_placeholder(type->element, level + 1);
	case TypeKind::Array:
		return type->owner != nullptr || names_placeholder(type->element, level + 1);
	case TypeKind::MemberPointer:
		return names_placeholder(type->owner, level + 1) || names_placeholder(type->element, level + 1);
	case TypeKind::Function:
	case TypeKind::Pack:
		for (uint32_t i = 0; i < type->param_count; ++i)
		{
			if (names_placeholder(type->params[i], level + 1))
				return true;
		}
		return type->kind == TypeKind::Function && names_placeholder(type->element, level + 1);
	case TypeKind::Class:
	{
		const ClassTemplateFacts* facts = static_cast<const ClassEntity*>(type->entity)->templ;
		if (facts == nullptr || !facts->dependent)
			return false;
		for (uint32_t i = 0; i < facts->argument_count; ++i)
		{
			if (names_placeholder(facts->arguments[i], level + 1))
				return true;
		}
		return false;
	}
	default:
		return false;
	}
}

const Type* Analyzer::substitute_decltype(const Type* type, const Substitution& given)
{
	// Typed as a value-dependent argument is evaluated (see
	// substitute_value), its cv-qualifiers kept.
	const Type* result = substitute_value(type->unqualified, given);
	return result != nullptr ? types_.qualified(result, type->cv) : nullptr;
}

const Type* Analyzer::failed_substitution(const TemplateParameterEntity* placeholder)
{
	// A placeholder whose substitution failed before: the failure again (an
	// error where it is no result).
	substitution_error(placeholder->expression->location, "an invalid expression in a substitution");
	return nullptr;
}

const Type* Analyzer::substitute_value(const Type* type, const Substitution& given)
{
	// A value-dependent template argument evaluated (14.3.2p1), or a
	// decltype-specifier's operand typed (14.6.2.2), once every argument is
	// known and none is dependent, once per (placeholder, depth, canonical
	// arguments; a pack an expansion is substituting stands for its
	// element). Arguments that are known but dependent (other than the
	// placeholder's own parameters) make a placeholder that applies them
	// once a later substitution makes them non-dependent (a frame binding
	// the expression's names to them would hide them from it).
	const TemplateParameterEntity* placeholder = static_cast<const TemplateParameterEntity*>(type->entity);
	if (placeholder->pending != nullptr)
		return substitute_pending(type, given);
	TypeList scratch;
	const Substitution substitution = Substitution{ given.depth, effective_arguments(given, scratch), given.count,
		nullptr };
	TypeList canonical(substitution.count, nullptr);
	bool dependent = false;
	for (uint32_t i = 0; i < substitution.count; ++i)
	{
		if (substitution.arguments[i] == nullptr)
			return type;
		canonical[i] = substitution.arguments[i]->canonical;
		dependent = dependent || dependent_type(canonical[i]);
	}
	bool own = dependent;
	for (uint32_t i = 0; i < substitution.count && own; ++i)
		own = OwnParameter(canonical[i], substitution.depth, i);
	if (own)
		return type;
	// sizeof... of a pack an enclosing expansion substitutes element by
	// element still counts the whole pack (5.3.3p5; `S<(sizeof...(I) -
	// I)...>`): an operand that applies it is keyed by those whole packs
	// too (after the arguments), which its substitution binds.
	SmallList<int32_t, 8> elements;
	bool element_packs = false;
	for (uint32_t i = 0; placeholder->sizeof_packs && given.elements != nullptr && !dependent && i < given.count; ++i)
	{
		const bool element = given.elements[i] >= 0 && given.arguments[i]->kind == TypeKind::Pack;
		elements.push_back(element ? given.elements[i] : -1);
		if (element)
			canonical.push_back(given.arguments[i]->canonical);
		element_packs = element_packs || element;
	}
	return substituted_value(placeholder, placeholder, nullptr, substitution.depth, canonical.data(),
		substitution.count, dependent, element_packs ? elements.data() : nullptr);
}

bool Analyzer::element_renames(const Scope* scope, uint32_t depth, const int32_t* elements, uint32_t count,
	const Type* nested, std::vector<std::pair<const Identifier*, const Identifier*>>& out)
{
	// The names of the parameter packs of depth `depth` an operand's scope
	// declares that `elements` substitutes element by element, each with
	// the name its element is bound to (see bind_pack); whether one of
	// them is among the packs `nested` the operand's nested expansions
	// name (which expand it whole): a pack itself, or named by the pattern
	// of a function parameter pack's type (see expression_packs).
	bool whole = false;
	while (scope != nullptr && !DeclaresDepth(scope, depth))
		scope = scope->parent;
	for (const Binding* binding = scope != nullptr ? scope->bindings : nullptr; binding != nullptr;
		binding = binding->next)
	{
		if (!DeclaresParameter(binding))
			continue;
		const TemplateParameterEntity* parameter = static_cast<const TemplateParameterEntity*>(binding->entity);
		if (parameter->depth != depth || parameter->index >= count || elements[parameter->index] < 0 ||
			parameter->name == nullptr)
			continue;
		out.push_back(std::make_pair(
			parameter->name, pack_element_name(parameter->name, static_cast<uint32_t>(elements[parameter->index]))));
		for (uint32_t i = 0; nested != nullptr && i < nested->param_count && !whole; ++i)
		{
			const Type* named = nested->params[i];
			whole = named == parameter->canonical->type;
			if (!whole && named->kind != TypeKind::TemplateParameter)
			{
				PackList indexes;
				unexpanded_packs(named, depth, indexes);
				whole = std::find(indexes.begin(), indexes.end(), parameter->index) != indexes.end();
			}
		}
	}
	return whole;
}

const Type* Analyzer::substituted_value(const TemplateParameterEntity* keyed,
	const TemplateParameterEntity* placeholder, const PendingSubstitution* earlier, uint32_t depth,
	const Type* const* canonical, uint32_t count, bool dependent, const int32_t* elements)
{
	// `placeholder` substituted by the levels `earlier` recorded (the
	// placeholder `keyed` stands for them; itself for none) and then by
	// `canonical` at `depth`. Its expression may also name parameters of
	// depths no level substitutes (a member template's partial
	// specialization's argument list naming its own and its class
	// template's, which matching substitutes one depth at a time): it is
	// then evaluated as in a template definition (a decltype-specifier is
	// not typed), and the result is a placeholder recording the levels when
	// it depends on them. With `elements`, the packs it gives an index (not
	// -1) are substituted for that element (`canonical` has the element)
	// and their whole packs follow the `count` arguments: they are bound
	// whole and the copy names each one's element (see substitute_value).
	uint32_t keyed_count = count;
	for (uint32_t i = 0; elements != nullptr && i < count; ++i)
		keyed_count += elements[i] >= 0 ? 1 : 0;
	// A deduction's substitution checks access in the context of the
	// template deduced (14.8.2p8, 11p4), not in the scope of the operand's
	// first declaration (an equivalent operand of another template, or of
	// a friend declaration in a class, shares it, 14.5.6.1p5): that
	// template is part of the result's key.
	const Entity* access = soft_substitution_ != 0 ? substitution_context_ : nullptr;
	const SubstitutionKey probe = SubstitutionKey{ keyed, canonical, keyed_count, depth, access };
	if (const Type** found = decltype_substitutions_.find(probe))
		return *found != nullptr ? *found : failed_substitution(placeholder);
	const Type* result = nullptr;
	bool invalid = false;
	const uint32_t incomplete = incomplete_classes_seen_;
	if (!dependent)
	{
		// The expression's own scope substituted first (its parameters of
		// the positions named, see substituted_scope).
		const Substitution substitution = Substitution{ depth, canonical, count, nullptr };
		TypeList whole;
		std::vector<std::pair<const Identifier*, const Identifier*>> renames;
		for (uint32_t i = 0, next = count; elements != nullptr && i < count; ++i)
			whole.push_back(elements[i] >= 0 ? canonical[next++] : canonical[i]);
		const bool whole_names = elements != nullptr &&
			element_renames(placeholder->expression_scope, depth, elements, count, placeholder->nested_packs, renames);
		Scope* scope = placeholder->expression_scope;
		const TemplateParameterEntity* operand = placeholder;
		for (const PendingSubstitution* level = earlier; level != nullptr; level = level->earlier, operand = nullptr)
			scope = substituted_scope(scope,
				Substitution{ level->depth, level->arguments->params, level->arguments->param_count, nullptr },
				operand);
		// One element of an expansion binds its packs' names and that
		// element's (linear in the elements), unless the operand also
		// expands such a pack whole (`A<I...>` in `S<(I + A<I...>::n)...>`).
		scope = substituted_scope(scope,
			elements != nullptr ? Substitution{ depth, whole.data(), count, whole_names ? nullptr : elements }
								: substitution,
			operand);
		const bool open = unsubstituted_depth(placeholder, earlier, depth);
		FrameExpander expander(*this, scope);
		syntax::SyntaxCloner cloner(arena_, true, expander.get());
		const syntax::Expr* copy = elements != nullptr ? cloner.element_expr(placeholder->expression, renames)
													   : cloner.expr(placeholder->expression);
		// Packs of different lengths expanded together (14.5.3p5).
		if (failed())
			return nullptr;
		// The copy's own names are checked in the template deduced (a
		// function template's, or a class template's, scope in its home),
		// else for an operand a friend declaration in a class keyed in its
		// namespace (and may share with templates there) in that namespace.
		Scope deduced = Scope();
		deduced.kind = access != nullptr && access->kind == EntityKind::Function ? ScopeKind::Function
																				 : ScopeKind::Class;
		deduced.parent = access != nullptr ? access->owner : nullptr;
		deduced.entity = const_cast<Entity*>(access);
		InstantiationContext context(
			*this, enter_instantiation(instantiation_depth_ + 1, copy->location), true, placeholder->ordinal);
		access_point_ = access != nullptr ? &deduced
			: placeholder->friend_keyed   ? enclosing_namespace(placeholder->expression_scope)
										  : nullptr;
		access_scope_ = scope;
		// A trailing return type's `this` (5.1.1p3), substituted.
		BodyContext body = BodyContext();
		if (placeholder->this_type != nullptr)
		{
			body.this_type = substitute(placeholder->this_type, substitution);
			if (body.this_type == nullptr)
				return nullptr;
			body_ = &body;
		}
		if (!placeholder->non_type)
		{
			if (!open)
				result = decltype_type(*copy, scope);
		}
		else if (!open)
		{
			const Type* target = placeholder->value_type;
			const bool has_target = target != nullptr;
			for (const PendingSubstitution* level = earlier; target != nullptr && level != nullptr;
				level = level->earlier)
				target = substitute(target,
					Substitution{ level->depth, level->arguments->params, level->arguments->param_count, nullptr });
			if (target != nullptr)
				target = substitute(target, substitution);
			if (failed() || (has_target && target == nullptr))
				return nullptr;
			if (target == nullptr || !dependent_type(target))
				result = template_value(*copy, scope, target);
			++graph_.value_substitutions;
		}
		else
		{
			++template_depth_;
			Constant value;
			IntegralKind kind;
			if (constant_value(*copy, scope, value, false) && IntegralKindOf(value.type, kind))
				result = types_.value(value.type->unqualified, value.bits);
			--template_depth_;
		}
		invalid = failed();
	}
	// An invalid expression is the substitution's failure (14.8.2p8). The
	// result is memoized unless the analysis met an incomplete class (it
	// may be completed later, or be the class being defined).
	const bool memoized = incomplete_classes_seen_ == incomplete;
	if (invalid)
	{
		if (memoized && !failure_.transient)
		{
			SubstitutionKey key = probe;
			key.arguments = arena_.copy_array(canonical, keyed_count);
			decltype_substitutions_.insert(key, nullptr);
		}
		return nullptr;
	}
	if (result == nullptr)
	{
		DecltypeKey key = DecltypeKey{ nullptr, 0 };
		bool shared = false;
		if (dependent && earlier == nullptr)
			result = equivalent_placeholder(placeholder, depth, canonical, count, key, shared);
		if (result == nullptr)
		{
			result = pending_value(placeholder, depth, canonical, count, earlier);
			if (key.words != nullptr)
			{
				TemplateParameterEntity* made = static_cast<TemplateParameterEntity*>(result->entity);
				made->key = key.words - 1;
				if (shared)
					decltype_placeholders_.insert(key, made);
			}
		}
	}
	if (memoized)
	{
		SubstitutionKey key = probe;
		key.arguments = arena_.copy_array(canonical, keyed_count);
		decltype_substitutions_.insert(key, result);
	}
	return result;
}

uint8_t Analyzer::HeadDepth(const Scope* scope)
{
	// The depth of the template heads around a placeholder's expression,
	// kHeadsNone or kHeadsSeveral: found once, so that substituting it
	// usually needs no walk (see unsubstituted_depth).
	uint32_t found = kHeadsNone;
	for (; scope != nullptr; scope = scope->parent)
	{
		if (scope->kind != ScopeKind::TemplateParameters)
			continue;
		for (const Binding* binding = scope->bindings; binding != nullptr; binding = binding->next)
		{
			if (!DeclaresParameter(binding))
				continue;
			const uint32_t declared = static_cast<const TemplateParameterEntity*>(binding->entity)->depth;
			if (declared >= kHeadsNone || (found != kHeadsNone && found != declared))
				return kHeadsSeveral;
			found = declared;
		}
	}
	return static_cast<uint8_t>(found);
}

bool Analyzer::unsubstituted_depth(
	const TemplateParameterEntity* placeholder, const PendingSubstitution* earlier, uint32_t depth)
{
	// Whether a template head around a placeholder's expression declares
	// parameters of a depth neither `depth` nor a level of `earlier`
	// substitutes.
	if (placeholder->head_depth == kHeadsNone)
		return false;
	if (placeholder->head_depth != kHeadsSeveral)
	{
		bool substituted = placeholder->head_depth == depth;
		for (const PendingSubstitution* level = earlier; level != nullptr && !substituted; level = level->earlier)
			substituted = level->depth == placeholder->head_depth;
		return !substituted;
	}
	for (const Scope* scope = placeholder->expression_scope; scope != nullptr; scope = scope->parent)
	{
		if (scope->kind != ScopeKind::TemplateParameters)
			continue;
		for (const Binding* binding = scope->bindings; binding != nullptr; binding = binding->next)
		{
			if (!DeclaresParameter(binding))
				continue;
			const uint32_t declared = static_cast<const TemplateParameterEntity*>(binding->entity)->depth;
			bool substituted = declared == depth;
			for (const PendingSubstitution* level = earlier; level != nullptr && !substituted; level = level->earlier)
				substituted = level->depth == declared;
			if (!substituted)
				return true;
		}
	}
	return false;
}

bool Analyzer::DeclaresParameter(const Binding* binding)
{
	// A template head's declaration of a template parameter (not a frame's
	// binding of its name to an argument).
	return binding->entity != nullptr && binding->entity->kind == EntityKind::TemplateParameter &&
		(binding->kind == BindingKind::Type || binding->kind == BindingKind::TemplateValue);
}

bool Analyzer::OwnParameter(const Type* argument, uint32_t depth, uint32_t index)
{
	// Whether a canonical argument is the template parameter at (depth,
	// index) itself (or, for a pack, the expansion of it): a substitution
	// by such arguments changes nothing. A parameter pack given alone is
	// another template's pack at that position, named in an expansion's
	// pattern for a parameter that is no pack (`A<P>...`).
	const bool expansion = argument->kind == TypeKind::Pack && argument->param_count == 1 &&
		argument->params[0]->kind == TypeKind::Expansion;
	if (expansion)
		argument = argument->params[0]->element;
	if (argument->kind != TypeKind::TemplateParameter && argument->kind != TypeKind::TemplateTemplateParameter)
		return false;
	const TemplateParameterEntity* parameter = static_cast<const TemplateParameterEntity*>(argument->entity);
	return !IsPlaceholder(parameter) && parameter->depth == depth && parameter->index == index &&
		parameter->pack == expansion;
}

const Type* Analyzer::pending_value(const TemplateParameterEntity* placeholder, uint32_t depth,
	const Type* const* canonical, uint32_t count, const PendingSubstitution* earlier)
{
	// A value (or decltype) placeholder for `placeholder` with dependent
	// arguments, or whose expression names parameters no substitution has
	// bound yet: the same expression, the packs the arguments name, and the
	// substitutions to apply once they are no longer dependent and bind
	// every parameter it names.
	TemplateParameterEntity* entity = new_entity<TemplateParameterEntity>(
		EntityKind::TemplateParameter, placeholder->name, nullptr, nullptr);
	entity->non_type = placeholder->non_type;
	entity->canonical = entity;
	entity->expression = placeholder->expression;
	entity->expression_scope = placeholder->expression_scope;
	entity->head_depth = placeholder->head_depth;
	entity->named_parameters = placeholder->named_parameters;
	entity->sizeof_packs = placeholder->sizeof_packs;
	entity->friend_keyed = placeholder->friend_keyed;
	entity->nested_packs = placeholder->nested_packs;
	const Type* arguments = types_.pack(canonical, count);
	TypeList named;
	for (uint32_t i = 0; i < count; ++i)
	{
		const Type* argument = canonical[i];
		PackList inner;
		unexpanded_packs(argument, kAnyDepth, inner);
		if (!inner.empty())
			named.push_back(argument->kind == TypeKind::Pack && argument->param_count == 1 &&
						argument->params[0]->kind == TypeKind::Expansion
					? argument->params[0]->element
					: argument);
	}
	// The packs its expression names at depths no level binds stay named.
	if (placeholder->packs != nullptr)
	{
		for (uint32_t i = 0; i < placeholder->packs->param_count; ++i)
		{
			const TemplateParameterEntity* pack = static_cast<const TemplateParameterEntity*>(
				placeholder->packs->params[i]->entity);
			bool bound = pack->depth == depth;
			for (const PendingSubstitution* level = earlier; level != nullptr && !bound; level = level->earlier)
				bound = level->depth == pack->depth;
			if (!bound)
				named.push_back(placeholder->packs->params[i]);
		}
	}
	entity->packs = named.empty() ? nullptr : types_.pack(named.data(), static_cast<uint32_t>(named.size()));
	PendingSubstitution* pending = static_cast<PendingSubstitution*>(
		arena_.allocate(sizeof(PendingSubstitution), alignof(PendingSubstitution)));
	*pending = PendingSubstitution{ placeholder, arguments, depth, earlier };
	entity->pending = pending;
	entity->type = types_.named(TypeKind::TemplateParameter, entity);
	return entity->type;
}

const Type* Analyzer::substitute_pending(const Type* type, const Substitution& given)
{
	// A pending placeholder: each level's arguments substituted, and a
	// substitution of a depth no level binds whose parameters its
	// expression's template heads declare added as a level; then the
	// levels applied again, outermost first.
	const PendingSubstitution* pending = static_cast<const TemplateParameterEntity*>(type->entity)->pending;
	SmallList<const PendingSubstitution*, 4> levels;
	for (const PendingSubstitution* level = pending; level != nullptr; level = level->earlier)
		levels.push_back(level);
	// The levels' substituted arguments, one after another (level i's from
	// starts[i] to starts[i + 1]); a substitution's failure fails it.
	TypeList arguments;
	SmallList<uint32_t, 4> starts;
	bool changed = false;
	bool covered = false;
	for (size_t i = 0; i < levels.size(); ++i)
	{
		starts.push_back(static_cast<uint32_t>(arguments.size()));
		const Type* recorded = levels[i]->arguments;
		for (uint32_t k = 0; k < recorded->param_count; ++k)
		{
			const Type* argument = substitute(recorded->params[k], given);
			if (argument == nullptr)
				return nullptr;
			arguments.push_back(argument->canonical);
			changed = changed || argument->canonical != recorded->params[k];
		}
		covered = covered || levels[i]->depth == given.depth;
	}
	starts.push_back(static_cast<uint32_t>(arguments.size()));
	const TemplateParameterEntity* placeholder = pending->placeholder;
	TypeList added;
	bool declared = false;
	for (const Scope* scope = placeholder->expression_scope; scope != nullptr && !declared; scope = scope->parent)
		declared = DeclaresDepth(scope, given.depth);
	if (!covered && declared)
	{
		TypeList scratch;
		const Type* const* effective = effective_arguments(given, scratch);
		for (uint32_t i = 0; i < given.count && effective != nullptr; ++i)
		{
			if (effective[i] == nullptr)
			{
				added.clear();
				break;
			}
			added.push_back(effective[i]->canonical);
		}
	}
	if (!changed && added.empty())
		return type;
	// Reapplied from the outermost level: a level after which the
	// expression's value is known ends it.
	const TemplateParameterEntity* keyed = placeholder;
	const PendingSubstitution* earlier = nullptr;
	const Type* result = type;
	for (size_t i = levels.size() + (added.empty() ? 0 : 1); i-- > 0;)
	{
		const bool last = i == levels.size();
		const Type* const* canonical = last ? added.data() : arguments.data() + starts[i];
		const uint32_t count = static_cast<uint32_t>(last ? added.size() : starts[i + 1] - starts[i]);
		bool dependent = false;
		for (uint32_t k = 0; k < count; ++k)
			dependent = dependent || dependent_type(canonical[k]);
		result = substituted_value(
			keyed, placeholder, earlier, last ? given.depth : levels[i]->depth, canonical, count, dependent);
		if (result == nullptr)
			return nullptr;
		const TemplateParameterEntity* entity = static_cast<const TemplateParameterEntity*>(result->entity);
		if (result->kind != TypeKind::TemplateParameter || entity->pending == nullptr)
			break;
		keyed = entity;
		earlier = entity->pending;
	}
	return result;
}

bool Analyzer::DeclaresDepth(const Scope* scope, uint32_t depth)
{
	// Whether a scope is a template head declaring parameters of `depth`.
	for (const Binding* binding = scope->kind == ScopeKind::TemplateParameters ? scope->bindings : nullptr;
		binding != nullptr; binding = binding->next)
	{
		if (DeclaresParameter(binding) && static_cast<const TemplateParameterEntity*>(binding->entity)->depth == depth)
			return true;
	}
	return false;
}

} // namespace sema
