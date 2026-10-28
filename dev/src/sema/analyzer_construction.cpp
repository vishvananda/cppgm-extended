#include "sema/analyzer.h"

#include <cstdint>
#include <cstring>
#include <string>

#include "lex/identifier_table.h"
#include "sema/aggregates.h"
#include "sema/instantiation_context.h"
#include "support/small_list.h"

// Class object construction and destruction for the non-polymorphic object
// model (12.1, 12.4, 12.6, 8.5 for class types; semantics mode): the
// constructor each initialization of a class object (a class prvalue's
// too, 5.2.3) selects, each constructor's subobject initializations in
// order, and the destructor a complete class has. Lowering consumes these
// facts by identity.
namespace sema {

namespace {

bool IsObjectPointerType(const Type* type)
{
	return IsObjectPointer(type) && !IsVoid(type->element);
}

const syntax::Expr* WithoutParentheses(const syntax::Expr* expr)
{
	while (expr->kind == syntax::ExprKind::Paren)
		expr = static_cast<const syntax::ParenExpr*>(expr)->inner;
	return expr;
}

// The class an object of `type` or of an array (of arrays) of it is.
ClassEntity* ObjectClass(const Type* type)
{
	while (type->kind == TypeKind::Array)
		type = type->element;
	type = type->unqualified;
	return type->kind == TypeKind::Class ? static_cast<ClassEntity*>(type->entity) : nullptr;
}

// A user-provided special member (8.4.2p4): declared, and neither
// defaulted nor deleted on its first declaration.
bool UserProvided(const FunctionEntity* function)
{
	return !function->implicit && !function->defaulted && !function->deleted;
}

// A plan's entries (and its anonymous member objects', at any depth) that
// initialize `member` by its brace-or-equal-initializer take the
// constructor that initialization runs.
void TypedMemberInits(SubobjectInit* inits, uint32_t count, const VariableEntity* member)
{
	for (uint32_t i = 0; i < count; ++i)
	{
		SubobjectInit& init = inits[i];
		if (init.inner != nullptr)
			TypedMemberInits(init.inner, init.inner_count, member);
		else if (init.member != nullptr && init.member->variable == member && init.default_initializer != nullptr)
			init.constructor = member->constructor;
	}
}

} // namespace

// ------------------------------------------------------------- destructors

void Analyzer::declare_destructor(ClassEntity* entity, uint32_t location)
{
	// 12.4p4: without a user-declared destructor a class has an implicitly
	// declared one, virtual when a base's destructor is (12.4p9). 12.4p5:
	// an implicit (or defaulted) destructor is trivial when it is not
	// virtual and every base and member of class type has a trivial
	// destructor; 12.4p3, 15.4p14: without an exception-specification it
	// throws only what those destructors may.
	FunctionEntity* destructor = nullptr;
	if (const NameEntry* entry = names_.find(entity->scope, destructor_key_))
	{
		if (entry->functions != nullptr)
			destructor = static_cast<FunctionEntity*>(entry->functions->binding->entity);
	}
	bool trivial = true;
	for (const BaseClass* base = entity->bases; base != nullptr; base = base->next)
	{
		const ClassEntity* cls = static_cast<const ClassEntity*>(base->base);
		const FunctionEntity* inherited = cls->destructor;
		trivial = trivial && (inherited == nullptr || inherited->trivial);
	}
	for (const DataMember* member = entity->members; member != nullptr; member = member->next)
	{
		const Type* type = member->variable->type;
		const ClassEntity* cls = type->is_reference() ? nullptr : ObjectClass(type);
		if (cls != nullptr && cls->destructor != nullptr)
			trivial = trivial && cls->destructor->trivial;
	}
	const bool nothrow = destructor != nullptr && destructor->exception_spec ? false
																			 : implicit_destructor_nothrow(entity);
	const bool deleted = destructor_deleted(entity);
	if (destructor == nullptr)
	{
		const Type* type = types_.function(
			types_.fundamental(FundamentalType::Void), nullptr, 0, false, 0, RefQualifier::None);
		destructor = new_entity<FunctionEntity>(EntityKind::Function, destructor_key_, entity->scope, type);
		const char* name = entity->name != nullptr ? entity->name->spelling
			: entity->local_label != nullptr       ? entity->local_label
			: entity->label != nullptr             ? entity->label
												   : "";
		const std::string label = std::string("~") + name;
		destructor->label = copy_label(label.data(), label.size());
		destructor->signature = type;
		destructor->member = true;
		destructor->destructor = true;
		destructor->implicit = true;
		destructor->defined = true;
		destructor->is_inline = true;
		destructor->inline_member = true;
		destructor->nothrow = nothrow;
		destructor->trivial = trivial;
		destructor->deleted = deleted;
		override_facts(destructor, entity, location);
	}
	else
	{
		// 12.4p5: trivial when not user-provided, that is defaulted or
		// deleted on its first declaration (8.4.2p4, 8.4.3p4: a deleted
		// definition is the first declaration).
		destructor->trivial = (destructor->defaulted || destructor->deleted) && trivial && !destructor->is_virtual;
		destructor->deleted = destructor->deleted || (destructor->defaulted && deleted);
		if (!destructor->exception_spec)
			destructor->nothrow = nothrow;
	}
	entity->destructor = destructor;
	if (!destructor->trivial)
		++graph_.nontrivial_destructors;
}

bool Analyzer::implicit_destructor_nothrow(const ClassEntity* entity)
{
	// 15.4p14 (12.4p3 for a declaration without an exception-specification):
	// the implicit destructor allows what its bases' (its virtual bases'
	// included) and members' destructors allow.
	for (const BaseClass* base = entity->bases; base != nullptr; base = base->next)
	{
		const FunctionEntity* inherited = static_cast<const ClassEntity*>(base->base)->destructor;
		if (inherited != nullptr && !function_nothrow(inherited))
			return false;
	}
	for (uint32_t i = 0; i < VirtualBaseCount(entity); ++i)
	{
		const FunctionEntity* inherited =
			static_cast<const ClassEntity*>(VirtualBases(entity)[i].base->base)->destructor;
		if (!VirtualBases(entity)[i].direct && inherited != nullptr && !function_nothrow(inherited))
			return false;
	}
	for (const DataMember* member = entity->members; member != nullptr; member = member->next)
	{
		const Type* type = member->variable->type;
		const ClassEntity* cls = type->is_reference() ? nullptr : ObjectClass(type);
		if (cls != nullptr && cls->destructor != nullptr && !function_nothrow(cls->destructor))
			return false;
	}
	return true;
}

bool Analyzer::destructor_deleted(const ClassEntity* entity)
{
	// 12.4p5: an implicit (or defaulted) destructor is deleted when a
	// subobject's destructor is deleted or inaccessible from the class, or
	// a union's variant member has a non-trivial destructor.
	for (const BaseClass* base = entity->bases; base != nullptr; base = base->next)
	{
		const ClassEntity* cls = static_cast<const ClassEntity*>(base->base);
		if (cls->destructor != nullptr && !subobject_destructor_usable(cls->destructor, cls, entity, false))
			return true;
	}
	// So does an indirect virtual base's (the complete object's destructor
	// destroys it, 12.4p8).
	for (uint32_t i = 0; i < VirtualBaseCount(entity); ++i)
	{
		const ClassEntity* cls = static_cast<const ClassEntity*>(VirtualBases(entity)[i].base->base);
		if (!VirtualBases(entity)[i].direct && cls->destructor != nullptr &&
			!subobject_destructor_usable(cls->destructor, cls, entity, false))
			return true;
	}
	for (const DataMember* member = entity->members; member != nullptr; member = member->next)
	{
		const Type* type = member->variable->type;
		const ClassEntity* cls = type->is_reference() ? nullptr : ObjectClass(type);
		if (cls == nullptr || cls->destructor == nullptr)
			continue;
		if (entity->key == TokenKind::KW_UNION && !cls->destructor->trivial)
			return true;
		// An anonymous union member's is deleted when one of its variant
		// members' destructors is not trivial: the union-like class's then
		// is (12.4p5).
		if (cls->anonymous ? cls->destructor->deleted
						   : !subobject_destructor_usable(cls->destructor, cls, entity, true))
			return true;
	}
	return false;
}

bool Analyzer::subobject_destructor_usable(
	const FunctionEntity* destructor, const ClassEntity* cls, const ClassEntity* entity, bool member)
{
	// Not deleted, and accessible from a member of the enclosing class
	// `entity`; one protected, accessible only as a member of a derived
	// class P, destroys an object of P (11.4): a base subobject of
	// `entity`'s object, not a member of the base's own type.
	if (destructor->deleted)
		return false;
	if (destructor->implicit || cls->scope == nullptr)
		return true;
	const NameEntry* entry = names_.find(cls->scope, destructor_key_);
	for (const FunctionList* node = entry != nullptr ? entry->functions : nullptr; node != nullptr; node = node->next)
	{
		if (node->binding->entity != destructor || node->binding->declared_in == nullptr)
			continue;
		const ClassEntity* via = nullptr;
		if (!accessible(cls, cls, node->binding->access, entity->scope, &via, 0))
			return false;
		const ClassEntity* object = member ? cls : entity;
		return via == nullptr || via == object || conversions_.is_base_of(via, object);
	}
	return true;
}

void Analyzer::check_subobject_destruction(const ClassEntity* entity, const Scope* scope, uint32_t location)
{
	// A user-provided destructor destroys, and a non-delegating
	// user-provided constructor potentially destroys (12.6.2p10 as CWG 1424
	// has it), the bases (a most derived object's virtual bases too,
	// 12.4p8) and non-static data members: their destructors not deleted
	// and accessible there (12.4p11), a protected one acting on a base
	// subobject of the class's object, not on a member of the base's own
	// type (11.4). A union's variant members and an anonymous member's are
	// not destroyed by it.
	for (const BaseClass* base = entity->bases; base != nullptr; base = base->next)
	{
		const ClassEntity* cls = static_cast<const ClassEntity*>(base->base);
		if (cls->destructor == nullptr)
			continue;
		check_not_deleted(cls->destructor, location);
		check_special_access(cls->destructor, cls, scope, location, entity);
	}
	for (uint32_t i = 0; i < VirtualBaseCount(entity); ++i)
	{
		const ClassEntity* cls = static_cast<const ClassEntity*>(VirtualBases(entity)[i].base->base);
		if (VirtualBases(entity)[i].direct || cls->destructor == nullptr)
			continue;
		check_not_deleted(cls->destructor, location);
		check_special_access(cls->destructor, cls, scope, location, entity);
	}
	if (entity->key == TokenKind::KW_UNION)
		return;
	for (const DataMember* member = entity->members; member != nullptr; member = member->next)
	{
		const Type* type = member->variable->type;
		const ClassEntity* cls = type->is_reference() ? nullptr : ObjectClass(type);
		if (cls == nullptr || cls->destructor == nullptr || cls->anonymous)
			continue;
		check_not_deleted(cls->destructor, location);
		check_special_access(cls->destructor, cls, scope, location, cls);
	}
}

bool Analyzer::subobjects_destructible(const ClassEntity* entity)
{
	// Every base's (an indirect virtual base's too, 12.4p8) and non-static
	// data member's destructor usable from the class (see
	// subobject_destructor_usable), an anonymous member's variant members
	// counting as the class's own (GCC and Clang delete the default
	// constructor for them too).
	for (const BaseClass* base = entity->bases; base != nullptr; base = base->next)
	{
		const ClassEntity* cls = static_cast<const ClassEntity*>(base->base);
		if (cls->destructor != nullptr && !subobject_destructor_usable(cls->destructor, cls, entity, false))
			return false;
	}
	for (uint32_t i = 0; i < VirtualBaseCount(entity); ++i)
	{
		const ClassEntity* cls = static_cast<const ClassEntity*>(VirtualBases(entity)[i].base->base);
		if (!VirtualBases(entity)[i].direct && cls->destructor != nullptr &&
			!subobject_destructor_usable(cls->destructor, cls, entity, false))
			return false;
	}
	return members_destructible(entity, entity);
}

bool Analyzer::members_destructible(const ClassEntity* holder, const ClassEntity* entity)
{
	// subobjects_destructible's members: `holder`'s, an anonymous member's
	// members in its place.
	for (const DataMember* member = holder->members; member != nullptr; member = member->next)
	{
		const Type* type = member->variable->type;
		const ClassEntity* cls = type->is_reference() ? nullptr : ObjectClass(type);
		if (cls == nullptr)
			continue;
		if (cls->anonymous
				? !members_destructible(cls, entity)
				: cls->destructor != nullptr && !subobject_destructor_usable(cls->destructor, cls, entity, true))
			return false;
	}
	return true;
}

void Analyzer::check_destruction(const Type* type, const Scope* scope, uint32_t location)
{
	// 12.4p11: an object whose lifetime the program ends needs a destructor
	// that is not deleted and is accessible there.
	if (type->is_reference())
		return;
	const ClassEntity* cls = ObjectClass(type);
	if (cls == nullptr || cls->destructor == nullptr)
		return;
	check_not_deleted(cls->destructor, location);
	check_special_access(cls->destructor, cls, scope, location);
}

void Analyzer::check_temporary_destruction(const ExprFacts* facts, uint32_t location)
{
	// 12.2p3, 12.4p11: a temporary's destructor is potentially invoked, so
	// it is not deleted and is accessible where the full-expression is. A
	// class prvalue is materialized as one where a reference binds it, a
	// member of it is named or it is discarded (see the callers); in a
	// substitution that is its failure (14.8.2p8).
	if (facts == nullptr || facts->type == nullptr || facts->category != ValueCategory::PRValue ||
		(facts->flags & (kOverloadSet | kBoundMember)) != 0 || facts->type->unqualified->kind != TypeKind::Class)
		return;
	check_destruction(facts->type, expression_scope_, location);
}

void Analyzer::check_binding_temporary(const ExprFacts* from, const Conversion& conversion, uint32_t location)
{
	// 8.5.3p5: a reference bound to a temporary the conversion makes of a
	// class, or to a class prvalue, refers to a temporary (12.2p1; see
	// check_temporary_destruction).
	if (conversion.form != ConversionForm::Reference)
		return;
	if (conversion.temporary && conversion.result != nullptr && conversion.result->unqualified->kind == TypeKind::Class)
		check_destruction(conversion.result, expression_scope_, location);
	else if (conversion.constructor == nullptr)
		check_temporary_destruction(from, location);
}

// -------------------------------------------------------- default constructors

void Analyzer::implicit_default_facts(ClassEntity* entity, FunctionEntity* constructor)
{
	// 12.1p5: an implicit (or defaulted) default constructor is trivial when
	// nothing is virtual, no non-static member has a brace-or-equal-
	// initializer and every direct base and member of class type has a
	// trivial default constructor. 15.4p14: it throws only what the
	// constructors it runs and the member initializers it evaluates may.
	// Its subobject initializations are those of a constructor without
	// mem-initializers; a reference or const member it cannot initialize,
	// or a subobject's initialization that fails (in a substitution, which
	// records the failure), defines it as deleted (12.1p5), reported where
	// used.
	// A substitution's failure already pending (an earlier operand of a
	// probe) is no failure of this plan, whose facts are permanent.
	const SubstitutionFailure pending = failure_;
	failure_ = SubstitutionFailure();
	const bool saved_deleted = plan_deleted_;
	plan_deleted_ = false;
	constructor_plan(constructor, entity, nullptr, entity->scope, 0);
	const bool deleted = plan_deleted_ || failed();
	plan_deleted_ = saved_deleted;
	// A subobject's failed initialization (no usable default constructor)
	// is no trivial default constructor of it (the plan stops there).
	bool trivial = !entity->dynamic && !failed();
	if (pending.message != nullptr)
		failure_ = pending;
	bool nothrow = true;
	// 12.1p5 as CWG 2084 amended it: a union's is deleted when a variant
	// member's class has no trivial default constructor and no member has
	// a brace-or-equal-initializer; so is a union-like class's for each of
	// its anonymous unions (see anonymous_default_facts).
	bool variant_nontrivial = false, member_initialized = false, anonymous_deleted = false;
	for (uint32_t i = 0; i < constructor->init_count; ++i)
	{
		const SubobjectInit& init = constructor->inits[i];
		// An anonymous union's or struct's members initialize as the class's.
		if (init.inner != nullptr)
		{
			inner_plan_facts(init, trivial, nothrow, member_initialized);
			bool nontrivial = false;
			anonymous_default_facts(init, nontrivial, anonymous_deleted);
			variant_nontrivial = variant_nontrivial || (entity->key == TokenKind::KW_UNION && nontrivial);
		}
		if (init.default_initializer != nullptr)
		{
			trivial = false;
			member_initialized = true;
			nothrow = nothrow && !static_cast<const VariableEntity*>(init.member->variable)->throwing_initializer;
		}
		if (init.constructor != nullptr)
		{
			trivial = trivial && init.constructor->trivial;
			nothrow = nothrow && function_nothrow(init.constructor);
		}
		if (entity->key == TokenKind::KW_UNION && init.member != nullptr &&
			init.member->variable->kind == EntityKind::Variable)
		{
			ClassEntity* cls = ObjectClass(init.member->variable->type);
			if (cls != nullptr && !cls->anonymous)
			{
				const FunctionEntity* variant = implicit_constructor(cls);
				variant_nontrivial = variant_nontrivial || variant == nullptr || !variant->trivial;
			}
		}
	}
	constructor->trivial = trivial;
	// 8.4.2p2: only an implicit one or one defaulted on its first
	// declaration has the implicit exception-specification; one defaulted
	// later keeps its declaration's.
	if (constructor->implicit || constructor->defaulted)
		constructor->nothrow = nothrow;
	// 12.1p5: so it is when a base's or member's destructor is deleted or
	// inaccessible from it.
	if ((variant_nontrivial && !member_initialized) || anonymous_deleted || deleted || !subobjects_destructible(entity))
		constructor->deleted = true;
}

void Analyzer::anonymous_default_facts(const SubobjectInit& init, bool& nontrivial, bool& deleted)
{
	// An anonymous member object's default-initialization as its implicit
	// default constructor would do it, from the enclosing plan (one walk):
	// not trivial when a member has a brace-or-equal-initializer or a class
	// type without a trivial default constructor; deleted when it, or an
	// anonymous union within it, is a union with such a variant member and
	// no variant member with an initializer (12.1p5 as CWG 2084 amended
	// it: the union-like class's anonymous union, 9.5p8).
	const ClassEntity* cls = ObjectClass(init.member->variable->type);
	bool variant_nontrivial = false;
	for (uint32_t i = 0; i < init.inner_count; ++i)
	{
		const SubobjectInit& inner = init.inner[i];
		bool member_nontrivial = false;
		if (inner.inner != nullptr)
			anonymous_default_facts(inner, member_nontrivial, deleted);
		else if (inner.member != nullptr && inner.member->variable->kind == EntityKind::Variable)
		{
			const VariableEntity* variable = static_cast<const VariableEntity*>(inner.member->variable);
			ClassEntity* member_class = variable->type->is_reference() ? nullptr : ObjectClass(variable->type);
			const FunctionEntity* member_constructor = member_class != nullptr ? implicit_constructor(member_class)
																			   : nullptr;
			member_nontrivial = variable->member_initializer != nullptr ||
				(member_class != nullptr && (member_constructor == nullptr || !member_constructor->trivial));
		}
		variant_nontrivial = variant_nontrivial || member_nontrivial;
	}
	nontrivial = nontrivial || variant_nontrivial;
	if (cls != nullptr && cls->key == TokenKind::KW_UNION && variant_nontrivial && !cls->members_initialized)
		deleted = true;
}

void Analyzer::inner_plan_facts(const SubobjectInit& init, bool& trivial, bool& nothrow, bool& initialized)
{
	for (uint32_t i = 0; i < init.inner_count; ++i)
	{
		const SubobjectInit& inner = init.inner[i];
		if (inner.inner != nullptr)
			inner_plan_facts(inner, trivial, nothrow, initialized);
		if (inner.default_initializer != nullptr)
		{
			trivial = false;
			initialized = true;
			nothrow = nothrow && !static_cast<const VariableEntity*>(inner.member->variable)->throwing_initializer;
		}
		if (inner.constructor != nullptr)
		{
			trivial = trivial && inner.constructor->trivial;
			nothrow = nothrow && function_nothrow(inner.constructor);
		}
	}
}

FunctionEntity* Analyzer::default_constructor(ClassEntity* entity, uint32_t location)
{
	// The default constructor default- or value-initialization runs
	// (8.5p6-p7, 12.1p5); errors if there is none or it is deleted or
	// inaccessible where the initialization is (12p4; a defaulted one's
	// plan analyzes the class's initializers in the class).
	// Null for a substitution's failure (14.8.2p8).
	const Scope* scope = expression_scope_;
	FunctionEntity* constructor = implicit_constructor(entity);
	if (constructor == nullptr)
	{
		substitution_error(location, "no default constructor for the class");
		return nullptr;
	}
	check_not_deleted(constructor, location);
	check_special_access(constructor, entity, scope, location);
	return constructor;
}

bool Analyzer::TypeIdHoldsAuto(const syntax::TypeId& type)
{
	for (const syntax::DeclSpecifier& specifier : type.specifiers)
		if (specifier.kind == syntax::DeclSpecKind::Keyword && specifier.keyword == TokenKind::KW_AUTO)
			return true;
	return false;
}

const Type* Analyzer::new_auto_type(
	const syntax::NewExpr& expr, const syntax::TypeId& type_syntax, bool array, Scope* scope)
{
	// 5.3.4p2, 7.1.6.4p4: auto in the type-specifier-seq of a new-type-id
	// (or type-id): the new-initializer is `( assignment-expression )` and
	// the allocated type is deduced from it as a variable's declared with
	// the new-type-id's declarator would be (7.1.6.4p6).
	const syntax::Initializer* initializer = expr.initializer;
	if (array || initializer == nullptr || initializer->kind != syntax::InitializerKind::Parenthesized ||
		initializer->arguments.size != 1 || initializer->arguments[0]->kind == syntax::ExprKind::BracedInit)
		error(expr.location, "auto in a new-expression without a new-initializer ( assignment-expression ) (5.3.4p2)");
	Specifiers specifiers;
	this->specifiers(type_syntax.specifiers, scope, specifiers, false);
	if (failed())
		return nullptr;
	const bool saved_declarator = auto_declarator_;
	auto_declarator_ = specifiers.type == auto_placeholder_;
	const DeclaratorInfo info = declarator(type_syntax.declarator, specifiers.type, scope, type_syntax.location);
	auto_declarator_ = saved_declarator;
	if (failed())
		return nullptr;
	if (!holds_auto(info.type))
		error(type_syntax.location, "auto in a new-type-id that is no placeholder for the allocated type");
	const Type* saved_auto = auto_deduced_;
	auto_deduced_ = nullptr;
	const Type* type = deduced_type(specifiers, info.type, initializer, scope, expr.location);
	auto_deduced_ = saved_auto;
	return type;
}

ExprFacts* Analyzer::allocation(const syntax::NewExpr& expr, Scope* scope)
{
	// 5.3.4: the allocation function overload resolution selects for the
	// object's size and the placement arguments (p9: a class's own
	// operator new unless `::new`, else the global one), then the object
	// initialized by the new-initializer (p15: none default-initializes).
	// 5.3.4p6: an array's first bound is an expression of integral (or
	// unscoped enumeration) type, the element type the rest of the
	// new-type-id. In a substitution an invalid type or operand, and no
	// viable allocation function, are its failure (14.8.2p8).
	// 14.6p8, p10: the allocation function call of a new-expression whose
	// new-type-id, count and placement arguments are not dependent in its
	// template (they name nothing dependent, see
	// NameLookup::dependent_uses; its initializer's arguments are not the
	// call's, as Clang takes it) is resolved where the template defines
	// it: in an instantiation, some global allocation function declared in
	// the definition's context must be viable.
	const uint64_t uses = names_.dependent_uses();
	syntax::TypeId type_syntax = *expr.type;
	syntax::Declarator declarator = syntax::Declarator();
	const syntax::Expr* count = nullptr;
	Constant count_value;
	bool constant_count = false;
	if (type_syntax.declarator != nullptr && !type_syntax.declarator->suffixes.empty() &&
		type_syntax.declarator->suffixes[0].kind == syntax::DeclaratorSuffixKind::Array)
	{
		count = type_syntax.declarator->suffixes[0].bound;
		if (count == nullptr)
			error(expr.location, "an array new-expression without a bound");
		declarator = *type_syntax.declarator;
		declarator.suffixes.data += 1;
		declarator.suffixes.size -= 1;
		type_syntax.declarator = &declarator;
		ExprFacts* bound = typed_value(*count, scope);
		if (bound == nullptr)
			return nullptr;
		// A class bound by its single non-explicit conversion function to
		// an integral or unscoped enumeration type.
		if (const ClassEntity* cls = ClassOf(bound->type))
		{
			if (const FunctionEntity* converter = unique_conversion(cls, IsIntegralOrUnscopedEnum))
			{
				expression_scope_ = scope;
				bound = convert_class_operand(*count, converter);
			}
		}
		const Type* bound_type = decayed(bound);
		if (!IsIntegralOrUnscopedEnum(bound_type))
		{
			substitution_error(count->location, "an array bound of non-integral type");
			return nullptr;
		}
		convert_operand(*count, promoted(bound_type));
		// A constant count that is negative is ill-formed (C++14 5.3.4p7,
		// CWG 1464, which g++ and Clang apply in C++11 too); a run-time one
		// throws std::bad_array_new_length (see the lowering's
		// array_allocation). So is one whose array's size std::size_t
		// cannot hold (checked once the element type is known).
		IntegralKind kind;
		constant_count = constant_value(*count, scope, count_value, false);
		if (constant_count && IntegralKindOf(count_value.type, kind) && kind.is_signed &&
			static_cast<int64_t>(count_value.bits) < 0)
		{
			substitution_error(count->location, "a negative array new-expression count");
			return nullptr;
		}
	}
	const Type* type = TypeIdHoldsAuto(type_syntax) ? new_auto_type(expr, type_syntax, count != nullptr, scope)
													: type_id(type_syntax, scope);
	if (type == nullptr)
		return nullptr;
	if (type->kind == TypeKind::Function || type->is_reference() ||
		type->unqualified == types_.fundamental(FundamentalType::Void))
	{
		substitution_error(expr.location, "a new-expression allocates an object type");
		return nullptr;
	}
	const Type* element = type;
	while (element->kind == TypeKind::Array)
		element = element->element;
	ClassEntity* entity = const_cast<ClassEntity*>(ClassOf(element));
	if (entity != nullptr && !require_complete(entity))
	{
		substitution_error(expr.location, "a new-expression allocates an incomplete class type", true);
		return nullptr;
	}
	if (!check_abstract_object(element, expr.location))
		return nullptr;
	if (type->kind == TypeKind::Array && count == nullptr)
		error(expr.location, "array new-expressions are not supported here");
	// The implementation's limit on the array's size is PTRDIFF_MAX bytes,
	// as g++ and Clang take it (an array cookie that only then exceeds it
	// throws at run time, see the lowering's array_allocation).
	uint64_t element_size = 0, element_alignment = 0;
	if (constant_count && count != nullptr && type_size(type, element_size, element_alignment, expr.location) &&
		element_size != 0 && count_value.bits > uint64_t(INT64_MAX) / element_size)
	{
		substitution_error(count->location, "an array new-expression whose size exceeds PTRDIFF_MAX");
		return nullptr;
	}
	expr.count = count;
	// The size argument: a prvalue of std::size_t.
	syntax::LiteralExpr* size = static_cast<syntax::LiteralExpr*>(
		arena_.allocate(sizeof(syntax::LiteralExpr), alignof(syntax::LiteralExpr)));
	*size = syntax::LiteralExpr();
	size->kind = syntax::ExprKind::Literal;
	size->location = expr.location;
	new_facts(*size, types_.fundamental(FundamentalType::UnsignedLongInt), ValueCategory::PRValue);
	// The placement list is set only with has_placement.
	const uint32_t placements = expr.has_placement ? expr.placement.size : 0;
	syntax::Expr** arguments = static_cast<syntax::Expr**>(
		arena_.allocate(sizeof(syntax::Expr*) * (placements + 1), alignof(syntax::Expr*)));
	arguments[0] = size;
	for (uint32_t i = 0; i < placements; ++i)
	{
		if (typed(*expr.placement[i], scope) == nullptr)
			return nullptr;
		arguments[i + 1] = expr.placement[i];
	}
	syntax::Slice<syntax::Expr*> call_arguments;
	call_arguments.data = arguments;
	call_arguments.size = placements + 1;
	const Identifier* key = count != nullptr ? new_array_key_ : operator_key(TokenKind::KW_NEW);
	const bool bound = names_.limit() != NameLookup::kNoLimit && names_.dependent_uses() == uses;
	LookupResult found = LookupResult();
	const ClassEntity* naming = nullptr;
	if (!expr.global && entity != nullptr && entity->scope != nullptr)
	{
		found = names_.qualified(entity->scope, key, LookupFilter::Ordinary);
		naming = found.found() ? entity : nullptr;
	}
	// 10.2p7: members of different bases are an invalid set.
	if (found.ambiguous)
	{
		substitution_error(expr.location, "ambiguous class allocation function name");
		return nullptr;
	}
	std::vector<FunctionEntity*>& set = overload_set_;
	if (!found.found() && bound)
	{
		// One its definition's context declares is viable there (the
		// hosts' check), though the instantiation then selects among those
		// declared by its point too, as GCC and Clang resolve it again.
		const bool lexical = lexical_limit_;
		lexical_limit_ = true;
		const LookupResult declared = global_allocation_functions(key);
		lexical_limit_ = lexical;
		if (!declared.found() || declared.binding->kind != BindingKind::Function)
			error(expr.location, "no allocation function for the new-expression");
		set.clear();
		for (const FunctionList* node = declared.functions; node != nullptr; node = node->next)
			set.push_back(static_cast<FunctionEntity*>(node->binding->entity));
		if (best_viable(call_arguments, expr.location, set, nullptr, true).function == nullptr)
			return nullptr;
	}
	if (!found.found())
		found = global_allocation_functions(key);
	if (!found.found() || found.binding->kind != BindingKind::Function)
		error(expr.location, "no allocation function for the new-expression");
	set.clear();
	for (const FunctionList* node = found.functions; node != nullptr; node = node->next)
		set.push_back(static_cast<FunctionEntity*>(node->binding->entity));
	const Candidate chosen = best_viable(call_arguments, expr.location, set, nullptr, true);
	FunctionEntity* function = chosen.function;
	if (function == nullptr)
		return nullptr;
	check_not_deleted(function, expr.location);
	check_function_access(function, found.functions, naming, nullptr, scope, expr.location);
	convert_arguments(call_arguments, function->type->canonical, &chosen, 0);
	odr_use(function, true);
	if (!function->nothrow)
		++potentially_throwing_;
	// The object's (each element's) initialization.
	const FunctionEntity* constructor = nullptr;
	const syntax::BracedInitList* list = count != nullptr && expr.initializer != nullptr &&
			expr.initializer->kind == syntax::InitializerKind::Braced
		? static_cast<const syntax::BracedInitList*>(expr.initializer->expression)
		: nullptr;
	bool trailing = true;
	if (list != nullptr && !array_new_list(expr, *list, type, trailing, scope))
		return nullptr;
	if (count != nullptr && entity != nullptr && !trailing)
	{
		// Every element from the list: no default constructor runs.
		check_destruction(element, scope, expr.location);
		new_deallocation(expr, entity, delete_array_key_, scope, function);
	}
	else if (count != nullptr && entity != nullptr)
	{
		// Default-initialized, or value-initialized by `()` (5.3.4p15) or
		// from the empty lists of the elements a list leaves.
		if (expr.initializer != nullptr && list == nullptr &&
			(expr.initializer->kind != syntax::InitializerKind::Parenthesized || !expr.initializer->arguments.empty()))
			error(expr.location, "an array new-expression with this initializer is not supported");
		FunctionEntity* selected = default_constructor(entity, expr.location);
		if (selected == nullptr)
			return nullptr;
		odr_use(selected, true);
		if (!selected->nothrow)
			++potentially_throwing_;
		check_destruction(element, scope, expr.location);
		constructor = selected;
		new_deallocation(expr, entity, delete_array_key_, scope, function);
	}
	else if (count != nullptr)
	{
		if (expr.initializer != nullptr && list == nullptr &&
			(expr.initializer->kind != syntax::InitializerKind::Parenthesized || !expr.initializer->arguments.empty()))
			error(expr.location, "an array new-expression with this initializer is not supported");
		if ((element->cv & kConst) != 0 && expr.initializer == nullptr)
			error(expr.location, "a const object needs an initializer");
		// The list's elements may throw: the storage is freed (5.3.4p18).
		if (list != nullptr && !list->elements.empty())
			new_deallocation(expr, nullptr, delete_array_key_, scope, function);
	}
	else if (expr.initializer != nullptr)
	{
		constructor = initialize_object(type, *expr.initializer, scope, expr.location);
		if (failed())
			return nullptr;
	}
	else if (entity != nullptr)
	{
		FunctionEntity* selected = default_constructor(entity, expr.location);
		if (selected == nullptr)
			return nullptr;
		if ((type->cv & kConst) != 0)
			check_const_default(entity, selected, expr.location);
		odr_use(selected, true);
		if (!selected->nothrow)
			++potentially_throwing_;
		constructor = selected;
	}
	else if ((type->cv & kConst) != 0)
		error(expr.location, "a const object needs an initializer");
	// A scalar's initializer may throw too (5.3.4p18).
	if (count == nullptr && (entity != nullptr || expr.initializer != nullptr))
		new_deallocation(expr, entity, operator_key(TokenKind::KW_DELETE), scope, function);
	expr.constructor = constructor;
	ExprFacts* facts = new_facts(expr, types_.pointer(type), ValueCategory::PRValue);
	facts->entity = function;
	facts->target = type;
	if (count != nullptr && sized_array_delete(entity))
		facts->flags |= kSizedArrayDelete;
	return facts;
}

void Analyzer::new_deallocation(const syntax::NewExpr& expr, const ClassEntity* entity, const Identifier* key,
	Scope* scope, const FunctionEntity* allocation)
{
	// 5.3.4p17-p20: the deallocation function freeing the storage if the
	// initialization throws, looked up as the allocation function is: the
	// usual one (12.5p4 for its form) for a non-placement allocation, a
	// placement one whose parameters after the first are the allocation
	// function's for a placement allocation (else none is called); a class
	// one's access is checked.
	LookupResult frees = LookupResult();
	const ClassEntity* naming = nullptr;
	if (!expr.global && entity != nullptr && entity->scope != nullptr)
	{
		frees = names_.qualified(entity->scope, key, LookupFilter::Ordinary);
		naming = frees.found() ? entity : nullptr;
	}
	if (frees.ambiguous)
	{
		substitution_error(expr.location, "ambiguous class deallocation function name");
		return;
	}
	if (!frees.found())
		frees = global_allocation_functions(key);
	if (!frees.found() || frees.binding->kind != BindingKind::Function)
		return;
	if (!expr.has_placement || expr.placement.size == 0)
		expr.deallocation = usual_deallocation(frees.functions);
	else
	{
		const Type* wanted = allocation->type->canonical;
		const Type* void_pointer = types_.pointer(types_.fundamental(FundamentalType::Void));
		for (const FunctionList* node = frees.functions; node != nullptr && expr.deallocation == nullptr;
			node = node->next)
		{
			const FunctionEntity* candidate = static_cast<const FunctionEntity*>(node->binding->entity);
			const Type* type = candidate->type->canonical;
			bool same = type->param_count == wanted->param_count && type->param_count > 1 &&
				type->params[0] == void_pointer && !type->variadic && !wanted->variadic;
			for (uint32_t i = 1; same && i < type->param_count; ++i)
				same = type->params[i] == wanted->params[i];
			if (same)
				expr.deallocation = candidate;
		}
	}
	if (expr.deallocation != nullptr && entity != nullptr)
	{
		check_function_access(expr.deallocation, frees.functions, naming, nullptr, scope, expr.location);
		odr_use(const_cast<FunctionEntity*>(expr.deallocation), true);
	}
}

bool Analyzer::sized_array_delete(const ClassEntity* entity)
{
	// Itanium 2.7 (GCC's and Clang's test): the class's usual operator
	// delete[] found by member lookup is its two-parameter form (a class
	// declaring the one-parameter form has that one, 12.5p5).
	if (entity == nullptr || entity->scope == nullptr)
		return false;
	const LookupResult found = names_.qualified(entity->scope, delete_array_key_, LookupFilter::Ordinary);
	if (!found.found() || found.ambiguous || found.binding->kind != BindingKind::Function)
		return false;
	const FunctionEntity* usual = usual_deallocation(found.functions);
	return usual != nullptr && usual->member && usual->type->canonical->param_count == 2;
}

LookupResult Analyzer::global_allocation_functions(const Identifier* key)
{
	// The global allocation or deallocation functions of a new- or
	// delete-expression (5.3.4p9, 5.3.5p9): in an instantiation, those
	// declared by its point, not only in its definition context (the
	// hosts' rule; <type_traits>' placement new-expressions use a later
	// <new>'s operator new); elsewhere those declared before (3.4.1p7: a
	// non-template class member's noexcept-specification, evaluated
	// later).
	if (lexical_limit_)
		return names_.qualified(graph_.global, key, LookupFilter::Ordinary);
	const uint32_t limit = names_.limit();
	names_.set_limit(NameLookup::kNoLimit);
	const LookupResult found = names_.qualified(graph_.global, key, LookupFilter::Ordinary);
	names_.set_limit(limit);
	return found;
}

const FunctionEntity* Analyzer::usual_deallocation(const FunctionList* functions)
{
	// 3.7.4.2p2, 12.5p4: operator delete(void*) (or its array form), else
	// a class's member with a second std::size_t parameter.
	const Type* void_pointer = types_.pointer(types_.fundamental(FundamentalType::Void));
	for (uint32_t pass = 0; pass < 2; ++pass)
	{
		for (const FunctionList* node = functions; node != nullptr; node = node->next)
		{
			const FunctionEntity* candidate = static_cast<const FunctionEntity*>(node->binding->entity);
			const Type* type = candidate->type->canonical;
			const bool usual = type->param_count == 1 + pass && type->params[0] == void_pointer &&
				(pass == 0 || type->params[1] == types_.fundamental(FundamentalType::UnsignedLongInt));
			if (usual && (pass == 0 || candidate->member))
				return candidate;
		}
	}
	return nullptr;
}

ExprFacts* Analyzer::deletion(const syntax::DeleteExpr& expr, Scope* scope)
{
	// 5.3.5: a pointer to an object type (a class operand by its single
	// conversion function to one, p1); the object's destructor and the
	// usual deallocation function (p9-p10: the class's own unless
	// `::delete`, else the global one; 12.5p4: operator delete(void*)
	// before operator delete(void*, std::size_t)). In a substitution an
	// operand of another type is its failure (14.8.2p8).
	ExprFacts* operand = typed_value(*expr.operand, scope);
	if (operand == nullptr)
		return nullptr;
	if (const ClassEntity* cls = ClassOf(operand->type))
	{
		const FunctionEntity* converter = unique_conversion(cls, IsObjectPointerType);
		if (converter == nullptr)
		{
			substitution_error(expr.location, "a delete-expression's operand is not a pointer");
			return nullptr;
		}
		expression_scope_ = scope;
		convert_class_operand(*expr.operand, converter);
		operand = facts_of(*expr.operand);
	}
	const Type* pointer = decayed(operand);
	if (!IsObjectPointer(pointer) || IsVoid(pointer->element))
	{
		substitution_error(expr.location, "a delete-expression's operand is not a pointer to an object");
		return nullptr;
	}
	convert_operand(*expr.operand, pointer);
	const Type* object = pointer->element;
	// p10: an array of class T, or of arrays thereof (a new-expression's
	// `new T[n][m]`, 5.3.4p9), is destroyed and freed as T's elements.
	const Type* element = object;
	while (expr.array && element->kind == TypeKind::Array)
		element = element->element;
	ClassEntity* entity = const_cast<ClassEntity*>(ClassOf(element));
	// p5: an object of incomplete class type is deleted without its
	// destructor, by the global deallocation function (kIncompleteDelete).
	const bool incomplete = entity != nullptr && !require_complete(entity);
	if (incomplete)
		entity = nullptr;
	if (entity != nullptr)
	{
		check_destruction(element, scope, expr.location);
		if (entity->destructor != nullptr && !entity->destructor->trivial)
			odr_use(entity->destructor, true);
	}
	const Identifier* key = expr.array ? delete_array_key_ : operator_key(TokenKind::KW_DELETE);
	LookupResult found = LookupResult();
	const ClassEntity* naming = nullptr;
	if (!expr.global && entity != nullptr && entity->scope != nullptr)
	{
		found = names_.qualified(entity->scope, key, LookupFilter::Ordinary);
		naming = found.found() ? entity : nullptr;
	}
	// 10.2p7: members of different bases are an invalid set.
	if (found.ambiguous)
	{
		substitution_error(expr.location, "ambiguous class deallocation function name");
		return nullptr;
	}
	if (!found.found())
		found = global_allocation_functions(key);
	if (!found.found() || found.binding->kind != BindingKind::Function)
		error(expr.location, "no deallocation function for the delete-expression");
	FunctionEntity* function = const_cast<FunctionEntity*>(usual_deallocation(found.functions));
	if (function == nullptr)
		error(expr.location, "no usual deallocation function for the delete-expression");
	check_not_deleted(function, expr.location);
	check_function_access(function, found.functions, naming, nullptr, scope, expr.location);
	odr_use(function, true);
	ExprFacts* facts = new_facts(expr, types_.fundamental(FundamentalType::Void), ValueCategory::PRValue);
	facts->entity = function;
	facts->target = object;
	if (expr.array && sized_array_delete(entity))
		facts->flags |= kSizedArrayDelete;
	if (incomplete)
		facts->flags |= kIncompleteDelete;
	return facts;
}

// ------------------------------------------------------------ constructors

FunctionEntity* Analyzer::select_constructor(ClassEntity* entity, const syntax::Slice<syntax::Expr*>& arguments,
	ConstructionKind kind, uint32_t location, bool* elide)
{
	// 13.3.1.3 (direct-initialization), 13.3.1.4 (copy-initialization by a
	// converting constructor) and 13.3.1.7 (list-initialization, which
	// considers explicit constructors but may not select one in copy-list-
	// initialization). With `elide` set the source is a temporary of the
	// class: only a copy or move constructor binding it directly may be
	// elided (12.8p31), selected and checked (12.2p1) but not called;
	// `elide` then says whether it was. In a substitution no viable or an
	// ambiguous constructor is its failure (null; 14.8.2p8).
	std::vector<FunctionEntity*>& set = overload_set_;
	set.clear();
	constructor_candidates(entity, arguments, kind, set);
	if (set.empty())
	{
		substitution_error(location, "no constructor for the initialization");
		return nullptr;
	}
	const ClassEntity* saved_direct = direct_class_;
	const bool saved_standard = standard_only_;
	direct_class_ = kind == ConstructionKind::Direct && arguments.size == 1 ? entity : nullptr;
	standard_only_ = kind == ConstructionKind::Converting;
	const Candidate chosen = best_viable(arguments, location, set, nullptr, true);
	direct_class_ = saved_direct;
	standard_only_ = saved_standard;
	FunctionEntity* selected = inherited_specialization(entity, chosen.function, arguments.size, location);
	if (selected == nullptr)
		return nullptr;
	check_inheriting_access(selected, entity, location);
	if (kind == ConstructionKind::CopyList && selected->is_explicit)
		error(location, "copy-list-initialization selects an explicit constructor");
	check_not_deleted(selected, location);
	check_special_access(selected, entity, expression_scope_, location);
	if (elide != nullptr && *elide)
	{
		const Conversion& binding = sequences_[chosen.first];
		*elide = (selected->copy_kind == CopyKind::CopyConstructor ||
					 selected->copy_kind == CopyKind::MoveConstructor) &&
			binding.form == ConversionForm::Reference && binding.constructor == nullptr && !binding.temporary;
		if (*elide)
			return selected;
	}
	convert_arguments(arguments, selected->type->canonical, &chosen, 0);
	odr_use(selected, true);
	if (!selected->nothrow)
		++potentially_throwing_;
	return selected;
}

FunctionEntity* Analyzer::value_initialize(ClassEntity* entity, uint32_t location)
{
	// 8.5p7: a class with a user-provided default constructor runs it;
	// otherwise the object is zero-initialized and its implicit default
	// constructor, if not trivial, runs.
	FunctionEntity* constructor = default_constructor(entity, location);
	if (constructor == nullptr)
		return nullptr;
	odr_use(constructor, true);
	if (!constructor->nothrow)
		++potentially_throwing_;
	return constructor;
}

FunctionEntity* Analyzer::initialize_class(
	ClassEntity* entity, const Type* type, const syntax::Initializer& init, Scope* scope, uint32_t location)
{
	// 8.5p16 for a class object.
	expression_scope_ = scope;
	if (!require_complete(entity) || entity->scope == nullptr)
		error(location, "initialization of an incomplete class object");
	const syntax::BracedInitList* list = nullptr;
	if (init.kind == syntax::InitializerKind::Braced ||
		(init.kind == syntax::InitializerKind::Assign && init.expression->kind == syntax::ExprKind::BracedInit))
		list = static_cast<const syntax::BracedInitList*>(init.expression);
	if (list != nullptr)
	{
		// 8.5.4p3: an aggregate is aggregate-initialized; an empty list
		// value-initializes a class with a default constructor; otherwise
		// constructors are considered.
		if (IsAggregateClass(entity))
		{
			aggregate_initialize(*list, entity, scope);
			ExprFacts* facts = new_facts(*list, type->unqualified, ValueCategory::PRValue);
			if (list == flexible_list_ && flexible_given_)
			{
				facts->flags |= kFlexibleElements;
				flexible_given_ = false;
			}
			return nullptr;
		}
		if (list->designators != nullptr)
			error(list->location, "designated initializers of a class that is no aggregate");
		// std::initializer_list<E> itself refers to the array the elements
		// initialize (its copy elided).
		if (initializer_list_element(type) != nullptr)
		{
			initializer_list_object(*list, type, scope);
			return nullptr;
		}
		// `{}` of a class without a default constructor takes its
		// initializer-list constructor's empty list (13.3.1.7p1).
		ExprFacts* facts = new_facts(*list, type->unqualified, ValueCategory::PRValue);
		if (list->elements.empty() && (implicit_constructor(entity) != nullptr || failed()))
		{
			facts->entity = value_initialize(entity, location);
			return static_cast<FunctionEntity*>(const_cast<Entity*>(facts->entity));
		}
		// Each element typed once (a call's braced argument's were, for
		// ranking, see braced_argument).
		for (const syntax::Expr* element : list->elements)
		{
			if (element->facts != nullptr)
				continue;
			if (element->kind == syntax::ExprKind::BracedInit)
				braced_argument(static_cast<const syntax::BracedInitList&>(*element), scope);
			else
				typed(*element, scope);
			if (failed())
				return nullptr;
		}
		// 13.3.1.7p1: an initializer-list constructor takes the list first.
		const ConstructionKind kind = init.kind == syntax::InitializerKind::Braced ? ConstructionKind::Direct
																				   : ConstructionKind::CopyList;
		if (FunctionEntity* constructor = select_list_constructor(entity, facts, *list, kind, scope, location))
		{
			facts->entity = constructor;
			return constructor;
		}
		if (failed())
			return nullptr;
		FunctionEntity* constructor = select_constructor(entity, list->elements, kind, location);
		if (constructor == nullptr)
			return nullptr;
		// 8.5.4p3: no argument's conversion narrows (a conversion function's
		// second standard conversion included).
		const Type* parameters = constructor->type->canonical;
		for (uint32_t i = 0; i < list->elements.size && i < parameters->param_count; ++i)
		{
			const syntax::Expr* argument = WithoutParentheses(list->elements[i]);
			const Conversion* conversion = argument->facts->conversion;
			if (conversion != nullptr && conversion->form != ConversionForm::Reference &&
				!check_narrowing(*argument, conversion, parameters->params[i], scope))
				return nullptr;
		}
		facts->entity = constructor;
		return constructor;
	}
	if (init.kind == syntax::InitializerKind::Parenthesized)
	{
		// `T x(args)`, `m(args)`; an empty list (a mem-initializer's `m()`)
		// value-initializes.
		if (init.arguments.empty())
			return value_initialize(entity, location);
		for (const syntax::Expr* argument : init.arguments)
		{
			if (argument->kind == syntax::ExprKind::BracedInit)
				braced_argument(static_cast<const syntax::BracedInitList&>(*argument), scope);
			else
				typed(*argument, scope);
			if (failed())
				return nullptr;
		}
		return direct_initialization(entity, init.arguments, location);
	}
	if (init.kind != syntax::InitializerKind::Assign)
		error(location, "only functions can be defaulted or deleted");
	// Copy-initialization `T x = e` (8.5p16): an expression of the class or
	// a class derived from it initializes the object by the constructor it
	// selects. From another type the best user-defined conversion decides
	// (13.3.1.4; none or an ambiguous one is ill-formed): among the
	// converting constructors (their argument by standard conversions,
	// 13.3.3.1p4) and a source class's conversion functions, whose result
	// initializes the object as apply_user_conversion records.
	const syntax::Expr& single = *init.expression;
	if (typed(single, scope) == nullptr)
		return nullptr;
	if (!check_resolved(single))
		return nullptr;
	if (same_class_source(entity, single))
		return copy_initialization(entity, single, ConstructionKind::Copy, location);
	const ClassEntity* source = ClassOf(single.facts->type);
	if (source != nullptr && source->has_conversion_functions)
	{
		Conversion conversion = Conversion();
		if (!user_conversion(single.facts, entity->type, conversion, false))
		{
			substitution_error(location, "no unambiguous user-defined conversion initializes the class object");
			return nullptr;
		}
		if (!conversion.constructor->constructor)
		{
			ExprFacts* facts = facts_of(*WithoutParentheses(&single));
			apply_user_conversion(facts, conversion, location);
			record_conversion(facts, conversion);
			return nullptr;
		}
	}
	syntax::Slice<syntax::Expr*> arguments;
	arguments.data = const_cast<syntax::Expr**>(&init.expression);
	arguments.size = 1;
	FunctionEntity* selected = select_constructor(entity, arguments, ConstructionKind::Converting, location);
	if (selected != nullptr)
		check_final_copy(entity, location);
	return selected;
}

FunctionEntity* Analyzer::direct_initialization(
	ClassEntity* entity, const syntax::Slice<syntax::Expr*>& arguments, uint32_t location)
{
	// `T x(args)` from its typed arguments (8.5p16): the constructor
	// overload resolution selects (13.3.1.3); an expression of the class
	// (or a derived one) alone is copied or moved, a prvalue of the class
	// perhaps in place (see copy_initialization).
	if (arguments.size == 1 && same_class_source(entity, *arguments[0]))
		return copy_initialization(entity, *arguments[0], ConstructionKind::Direct, location);
	return select_constructor(entity, arguments, ConstructionKind::Direct, location);
}

ExprFacts* Analyzer::class_prvalue(
	const syntax::Expr& expr, const Type* type, const syntax::Initializer& view, Scope* scope)
{
	// A prvalue of class type initialized by the constructor its
	// initializer selects (its facts' entity), or aggregate-initialized;
	// a class prvalue keeps its cv-qualifiers (3.10p4, 5.2.3p1).
	ClassEntity* entity = static_cast<ClassEntity*>(type->unqualified->entity);
	if (!check_abstract_object(type, expr.location))
		return nullptr;
	FunctionEntity* constructor = nullptr;
	if (expr.kind == syntax::ExprKind::FunctionalCast && view.kind == syntax::InitializerKind::Parenthesized &&
		!view.arguments.empty() && require_complete(entity) && IsAggregateClass(entity))
	{
		const bool aggregate = parenthesized_aggregate(entity, view.arguments, scope);
		if (failed())
			return nullptr;
		expression_scope_ = scope;
		if (!aggregate)
			constructor = direct_initialization(entity, view.arguments, expr.location);
		else
		{
			// The course's direct aggregate construction (the README; C++20's
			// P0960): `T(a, b)` of an aggregate no constructor takes the list
			// of is aggregate-initialized from it, as `T{a, b}` would be, but
			// its own elements may narrow ([dcl.init]p17.6.2.2: each is
			// copy-initialized; a braced element stays list-initialization,
			// as clang has it where g++ and the reference accept narrowing).
			syntax::BracedInitList* list = static_cast<syntax::BracedInitList*>(
				arena_.allocate(sizeof(syntax::BracedInitList), alignof(syntax::BracedInitList)));
			*list = syntax::BracedInitList();
			list->kind = syntax::ExprKind::BracedInit;
			list->location = expr.location;
			list->elements = view.arguments;
			static_cast<const syntax::FunctionalCastExpr&>(expr).aggregate = list;
			syntax::Initializer braced = view;
			braced.kind = syntax::InitializerKind::Braced;
			braced.expression = list;
			const syntax::BracedInitList* saved = aggregate_arguments_;
			aggregate_arguments_ = list;
			constructor = initialize_class(entity, entity->type, braced, scope, expr.location);
			aggregate_arguments_ = saved;
		}
	}
	else
		constructor = initialize_class(entity, entity->type, view, scope, expr.location);
	if (failed())
		return nullptr;
	ExprFacts* facts = new_facts(expr, type, ValueCategory::PRValue);
	facts->entity = constructor;
	return facts;
}

bool Analyzer::parenthesized_aggregate(ClassEntity* entity, const syntax::Slice<syntax::Expr*>& arguments, Scope* scope)
{
	// Whether a non-empty parenthesized list aggregate-initializes the
	// aggregate: only when none of its constructors is viable (C++20's
	// [dcl.init]p17.6.2, P0960; an aggregate provides none itself, 8.5.1p1,
	// so a single argument may meet a copy or move constructor). Several
	// expressions leave none viable. A sole one is typed here, once, for
	// both forms (a braced-init-list as a call's argument is, 13.3.3.1.5):
	// one of no class (an overloaded function's name too) or of a class
	// unrelated to the aggregate without conversion functions converts to
	// no parameter; one of the class or a derived class is copied or moved;
	// otherwise overload resolution among the constructors decides
	// (13.3.1.3: a conversion function may make the object, 13.3.1.4 and
	// 13.3.1.6 with CWG 899's explicit ones; an ambiguous resolution is no
	// aggregate's).
	if (arguments.size > 1)
		return true;
	const syntax::Expr& single = *arguments[0];
	if (single.kind == syntax::ExprKind::BracedInit)
		braced_argument(static_cast<const syntax::BracedInitList&>(single), scope);
	else
	{
		const ExprFacts* argument = typed(single, scope);
		if (argument == nullptr)
			return false;
		const ClassEntity* source = argument->type != nullptr && (argument->flags & (kOverloadSet | kBoundMember)) == 0
			? ClassOf(argument->type)
			: nullptr;
		if (source == nullptr)
			return true;
		if (source == entity || conversions_.is_base_of(entity, source))
			return false;
		if (!source->has_conversion_functions)
			return true;
	}
	if (failed())
		return false;
	std::vector<FunctionEntity*>& set = overload_set_;
	set.clear();
	constructor_candidates(entity, arguments, ConstructionKind::Direct, set);
	const ClassEntity* saved_direct = direct_class_;
	const bool saved_standard = standard_only_;
	direct_class_ = entity;
	standard_only_ = false;
	Candidate chosen = Candidate();
	const Resolution resolution = resolve(arguments, set, nullptr, chosen);
	direct_class_ = saved_direct;
	standard_only_ = saved_standard;
	return resolution == Resolution::NoViable;
}

bool Analyzer::same_class_source(const ClassEntity* entity, const syntax::Expr& source)
{
	// An expression of the class or of a class derived from it (8.5p16:
	// constructors initialize from it directly).
	const ClassEntity* from = source.facts->type != nullptr &&
			(source.facts->flags & (kOverloadSet | kBoundMember)) == 0
		? ClassOf(source.facts->type)
		: nullptr;
	return from != nullptr && (from == entity || conversions_.is_base_of(entity, from));
}

FunctionEntity* Analyzer::copy_initialization(
	ClassEntity* entity, const syntax::Expr& source, ConstructionKind kind, uint32_t location, bool in_place)
{
	// The constructor overload resolution selects with the expression as
	// its argument (12.8p2-p3: a copy constructor for an lvalue, a move
	// constructor for an rvalue when there is one); a prvalue of the class
	// itself initializes the object in place when that constructor is a
	// copy or move constructor (12.8p31): still selected, accessible and
	// not deleted (12.2p1), and null returned. A conditional's prvalue is
	// the course's private temporary, copied or moved into a declared
	// object, a parameter or a member (a result or an enclosing
	// conditional's takes it in place).
	syntax::Expr* pointer = const_cast<syntax::Expr*>(&source);
	syntax::Slice<syntax::Expr*> arguments;
	arguments.data = &pointer;
	arguments.size = 1;
	const syntax::Expr* inner = WithoutParentheses(&source);
	bool elided = inner->facts->category == ValueCategory::PRValue && ClassOf(inner->facts->type) == entity &&
		(in_place || inner->kind != syntax::ExprKind::Conditional);
	FunctionEntity* constructor = select_constructor(entity, arguments, kind, location, &elided);
	return elided ? nullptr : constructor;
}

const Conversion* Analyzer::record_copy(
	ClassEntity* entity, const Type* target, const syntax::Expr& source, uint32_t location, bool in_place)
{
	// Copy-initialization of a class object (a parameter, a result, an
	// aggregate's member) from an expression of its class or a derived
	// one: the conversion records the constructor and the binding of the
	// expression to its parameter; an elided copy records no constructor.
	ExprFacts* facts = facts_of(*WithoutParentheses(&source));
	const Conversion* before = facts->conversion;
	FunctionEntity* constructor = copy_initialization(entity, source, ConstructionKind::Copy, location, in_place);
	Conversion conversion = Conversion();
	conversion.form = ConversionForm::Standard;
	conversion.rank = ConversionRank::Exact;
	conversion.initialization = true;
	conversion.target = target;
	conversion.result = target->unqualified;
	conversion.constructor = constructor;
	conversion.argument = constructor != nullptr ? facts->conversion : nullptr;
	if (constructor == nullptr && before != nullptr)
		return before;
	return record_conversion(facts, conversion);
}

FunctionEntity* Analyzer::initialize_object(
	const Type* type, const syntax::Initializer& init, Scope* scope, uint32_t location)
{
	// 8.5: an object (or reference) of `type` from its initializer; the
	// constructor a class object runs is returned.
	if (ClassEntity* entity = type->is_reference() || type->kind == TypeKind::Array ? nullptr : ObjectClass(type))
		return initialize_class(entity, type, init, scope, location);
	const syntax::Expr* single = nullptr;
	bool direct = false;
	switch (init.kind)
	{
	case syntax::InitializerKind::Assign:
		if (init.expression->kind == syntax::ExprKind::BracedInit)
		{
			list_initialize(static_cast<const syntax::BracedInitList&>(*init.expression), type, scope);
			return nullptr;
		}
		single = init.expression;
		break;
	case syntax::InitializerKind::Braced:
		list_initialize(static_cast<const syntax::BracedInitList&>(*init.expression), type, scope);
		return nullptr;
	case syntax::InitializerKind::Parenthesized:
		if (init.arguments.empty())
		{
			// A mem-initializer's `m()` value-initializes (8.5p7): zero, or
			// each class element's default constructor.
			if (type->is_reference())
				error(location, "value-initialization of a reference");
			if (ClassEntity* element = ObjectClass(type))
				return value_initialize(element, location);
			return nullptr;
		}
		if (init.arguments.size != 1)
			error(location, "direct-initialization from several values");
		single = init.arguments[0];
		direct = true;
		break;
	default:
		error(location, "only functions can be defaulted or deleted");
	}
	ExprFacts* facts = typed(*single, scope);
	if (facts == nullptr)
		return nullptr;
	if (type->kind == TypeKind::Array)
	{
		// 8.5.2: a character array from a string literal of its kind.
		check_character_array(type, *single, facts, location);
		return nullptr;
	}
	convert(*single, type, direct);
	return nullptr;
}

// ------------------------------------------------ inheriting constructors

void Analyzer::declare_inherited_constructors(ClassEntity* entity)
{
	// 12.9p1-p4: each constructor of the named base but a copy or move
	// constructor (and its forms without trailing default arguments) gives
	// the class a constructor of the same parameters, access and
	// deletedness, unless the class declares one of that signature.
	ClassEntity* base = entity->inherits_constructors;
	const NameEntry* entry = base->scope != nullptr ? names_.find(base->scope, constructor_key_) : nullptr;
	std::vector<const Binding*> inherited;
	for (const FunctionList* node = entry != nullptr ? entry->functions : nullptr; node != nullptr; node = node->next)
		inherited.push_back(node->binding);
	for (const Binding* binding : inherited)
	{
		FunctionEntity* constructor = static_cast<FunctionEntity*>(binding->entity);
		const Type* type = constructor->type->canonical;
		// A constructor template is inherited as one with the same
		// characteristics (12.9p1): the class's constructors name it, and a
		// specialization of it chosen to construct the class is that
		// template's (see inherited_specialization).
		if (constructor->templated)
		{
			Binding* declared = new_binding(BindingKind::Function, constructor_key_, constructor, binding->type);
			declare_in(entity->scope, declared);
			declared->access = binding->access;
			continue;
		}
		uint32_t first = type->param_count;
		while (first > 0 && constructor->default_arguments != nullptr &&
			constructor->default_arguments[first - 1] != nullptr)
			--first;
		// The form without parameters (a default constructor) too where the
		// class declares constructors, so has no implicit default one: P0136,
		// which both host compilers apply to C++11 (libstdc++'s
		// __uniq_ptr_data is default-constructed by its base's).
		const uint32_t least = entity->user_declared_constructor ? 0 : 1;
		for (uint32_t count = type->param_count; count >= first && count >= least && count != ~0u; --count)
		{
			if (count == 1 && type->params[0]->is_reference() && type->params[0]->element->unqualified == base->type)
				break;
			const Type* signature = types_.function(
				types_.fundamental(FundamentalType::Void), type->params, count, false, 0, RefQualifier::None);
			const FunctionList* same = names_.overload(entity->scope, constructor_key_, signature);
			if (same != nullptr && same->binding->entity->owner == entity->scope)
				continue;
			FunctionEntity* inheriting = new_entity<FunctionEntity>(
				EntityKind::Function, constructor_key_, entity->scope, signature);
			inheriting->label = entity->name != nullptr ? entity->name->spelling : entity->label;
			inheriting->signature = signature;
			inheriting->member = true;
			inheriting->constructor = true;
			inheriting->implicit = true;
			inheriting->defined = true;
			inheriting->is_inline = true;
			inheriting->inline_member = true;
			inheriting->is_explicit = constructor->is_explicit;
			inheriting->nothrow = function_nothrow(constructor);
			inheriting->deleted = constructor->deleted;
			inheriting->inherited = constructor;
			record_inherited_constructor(inheriting);
			Binding* declared = new_binding(BindingKind::Function, constructor_key_, inheriting, signature);
			declare_in(entity->scope, declared);
			declared->access = binding->access;
			if (!inheriting->deleted)
				planned_inheriting_constructor(inheriting, entity, 0);
		}
	}
}

void Analyzer::record_inherited_constructor(FunctionEntity* inheriting)
{
	// Its root: the inherited constructor's own when that one is an
	// inheriting constructor too (recorded then), else that constructor;
	// the full form has the root's parameters.
	const FunctionEntity* root = graph_.inherited_root(inheriting->inherited);
	if (root != nullptr)
		graph_.set_inherited_root(inheriting, root);
	else
		root = inheriting->inherited;
	const Type* own = inheriting->type->canonical;
	const Type* declared = root->type->canonical;
	inheriting->inherits_full = own->param_count == declared->param_count && own->variadic == declared->variadic;
}

void Analyzer::check_inheriting_access(const FunctionEntity* constructor, const ClassEntity* entity, uint32_t location)
{
	// An inheriting constructor used is defined (12.9p8): its base
	// constructor is called as from a constructor of the class (a private
	// one is inaccessible there).
	if (constructor == nullptr || constructor->inherited == nullptr || entity->inherits_constructors == nullptr)
		return;
	const ClassEntity* saved_object = base_object_;
	base_object_ = entity;
	check_special_access(constructor->inherited, entity->inherits_constructors, entity->scope, location);
	base_object_ = saved_object;
}

FunctionEntity* Analyzer::inherited_specialization(
	ClassEntity* entity, FunctionEntity* specialization, uint32_t arguments, uint32_t location)
{
	// A base's constructor template specialization chosen to construct the
	// class that inherits the template, with `arguments` arguments: the
	// specialization of the class's implicitly declared constructor
	// template (12.9p1) of that many parameters (its forms without
	// trailing default arguments), which forwards them to it (12.9p8) and
	// has its access (12.9p4); one per (class, specialization, count).
	if (specialization == nullptr || !specialization->constructor || specialization->primary == nullptr ||
		specialization->owner == entity->scope)
		return specialization;
	// A template the named base inherits in turn (its constructors include
	// the ones it inherits, 12.9p1) is forwarded through the base's
	// inheriting constructor, which initializes the base (12.9p8).
	FunctionEntity* forwarded = specialization;
	ClassEntity* base = entity->inherits_constructors;
	if (base != nullptr && base->scope != nullptr && specialization->owner != base->scope)
		forwarded = inherited_specialization(base, specialization, arguments, location);
	if (forwarded == nullptr)
		return nullptr;
	const Type* type = forwarded->type->canonical;
	const uint32_t count = arguments < type->param_count ? arguments : type->param_count;
	const EntityPair key = EntityPair{ entity, specialization };
	uint32_t* head = inherited_heads_.find(key);
	for (uint32_t index = head != nullptr ? *head : PointerMap::kMissing; index != PointerMap::kMissing;
		index = inherited_specializations_[index].next)
	{
		FunctionEntity* known = inherited_specializations_[index].inheriting;
		if (known->type->canonical->param_count == count)
			return known;
	}
	const Type* signature = types_.function(
		types_.fundamental(FundamentalType::Void), type->params, count, type->variadic, 0, RefQualifier::None);
	FunctionEntity* inheriting = new_entity<FunctionEntity>(
		EntityKind::Function, constructor_key_, entity->scope, signature);
	inheriting->label = entity->name != nullptr ? entity->name->spelling : entity->label;
	inheriting->signature = signature;
	inheriting->member = true;
	inheriting->constructor = true;
	inheriting->implicit = true;
	inheriting->defined = true;
	inheriting->is_inline = true;
	inheriting->inline_member = true;
	inheriting->is_explicit = forwarded->is_explicit;
	inheriting->nothrow = function_nothrow(forwarded);
	inheriting->deleted = forwarded->deleted;
	inheriting->inherited = forwarded;
	record_inherited_constructor(inheriting);
	const uint32_t next = head != nullptr ? *head : PointerMap::kMissing;
	if (head != nullptr)
		*head = static_cast<uint32_t>(inherited_specializations_.size());
	else
		inherited_heads_.insert(key, static_cast<uint32_t>(inherited_specializations_.size()));
	inherited_specializations_.push_back(InheritedSpecialization{ inheriting, next });
	// Planned in the class; the use's checks (its access, 12.9p4) are the
	// caller's, in its scope.
	Scope* saved_scope = expression_scope_;
	if (!inheriting->deleted)
		planned_inheriting_constructor(inheriting, entity, location);
	expression_scope_ = saved_scope;
	return inheriting;
}

void Analyzer::planned_inheriting_constructor(FunctionEntity* inheriting, ClassEntity* entity, uint32_t location)
{
	// Its implicit definition (12.9p8): the inherited constructor for its
	// base, the other subobjects as an implicit default constructor
	// initializes them. Its exception-specification (15.4p14) waits for its
	// first need, when the class's member initializers are typed (they are
	// deferred to the outermost class's end; see function_nothrow). A
	// subobject that definition cannot initialize (a reference or const
	// member, 12.9p8 with 12.6.2p8, 8.5p6; a base or member without a
	// usable default constructor) makes it ill-formed only if it is used:
	// deleted, reported where it is (as the hosts take it, CWG 2273), as
	// an implicit default constructor's plan is (see
	// queried_default_constructor).
	const bool saved_deleted = plan_deleted_;
	plan_deleted_ = false;
	bool failed_plan = false;
	{
		SoftSubstitution soft(*this);
		constructor_plan(inheriting, entity, nullptr, entity->scope, location);
		failed_plan = soft.failed();
	}
	if (plan_deleted_ || failed_plan)
		inheriting->deleted = true;
	plan_deleted_ = saved_deleted;
	inheriting->noexcept_pending = true;
}

bool Analyzer::plan_nothrow(const FunctionEntity* constructor)
{
	// 15.4p14: an implicit definition throws only what the constructors it
	// runs and the member initializers it evaluates may.
	bool trivial = true, initialized = false;
	bool nothrow = true;
	for (uint32_t i = 0; i < constructor->init_count; ++i)
	{
		const SubobjectInit& init = constructor->inits[i];
		if (init.inner != nullptr)
			inner_plan_facts(init, trivial, nothrow, initialized);
		if (init.default_initializer != nullptr)
			nothrow = nothrow && !static_cast<const VariableEntity*>(init.member->variable)->throwing_initializer;
		if (init.constructor != nullptr)
			nothrow = nothrow && function_nothrow(init.constructor);
		// An inheriting constructor evaluates the default arguments of the
		// trailing parameters it omits (12.9p8: its forwarded call).
		const Type* forwarded = init.forwards && init.constructor != nullptr ? init.constructor->type->canonical
																			 : nullptr;
		for (uint32_t k = constructor->type->canonical->param_count; nothrow && forwarded != nullptr &&
			init.constructor->default_arguments != nullptr && k < forwarded->param_count;
			++k)
			nothrow = init.constructor->default_arguments[k] == nullptr ||
				!may_throw(*init.constructor->default_arguments[k], 0);
	}
	return nothrow;
}

// ------------------------------------------------ constructor definitions

void Analyzer::constructor_plan(FunctionEntity* constructor, ClassEntity* entity, const syntax::CtorInitializer* list,
	Scope* scope, uint32_t location)
{
	// 12.6.2: each direct base, then each non-static data member, in
	// declaration order. A mem-initializer names a member of the class or a
	// direct base (p2: looked up in the class, then where the constructor
	// is); the others take their brace-or-equal-initializer or are
	// default-initialized (p8), which a user-provided constructor may not
	// leave to a reference or const-qualified non-class member. Each
	// subobject's checks are made in `scope`: planning a subobject's class's
	// implicit constructor on demand plans in that class, and the caller's
	// scope is restored at the end (a plan can be any analysis's demand).
	Scope* const saved_scope = expression_scope_;
	expression_scope_ = scope;
	const uint32_t count = entity->base_count + entity->member_count + IndirectVirtualBases(entity);
	SubobjectInit* inits = static_cast<SubobjectInit*>(
		arena_.allocate(sizeof(SubobjectInit) * (count == 0 ? 1 : count), alignof(SubobjectInit)));
	uint32_t index = 0;
	for (const BaseClass* base = entity->bases; base != nullptr; base = base->next)
		inits[index++] = SubobjectInit{ base, nullptr, nullptr, nullptr, nullptr, false, false, 0, nullptr, nullptr };
	member_plans(entity, inits + index);
	index += entity->member_count;
	// 12.6.2p10: the indirect virtual bases, which the most derived class's
	// constructor initializes too (after the members in the plan; the
	// lowering orders the construction).
	for (uint32_t i = 0; i < VirtualBaseCount(entity); ++i)
	{
		if (!VirtualBases(entity)[i].direct)
			inits[index++] = SubobjectInit{ VirtualBases(entity)[i].base, nullptr, nullptr, nullptr, nullptr, false,
				false, 0, nullptr, nullptr };
	}
	constructor->inits = inits;
	constructor->init_count = count;
	// A union's constructor initializing one variant member leaves the
	// others' brace-or-equal-initializers unused (12.6.2p8, CWG 1460).
	const bool is_union = entity->key == TokenKind::KW_UNION;
	bool variant_designated = false;
	if (list != nullptr)
	{
		for (const syntax::MemInitializer& initializer : list->initializers)
		{
			SubobjectInit* target = mem_initializer_target(constructor, entity, initializer, scope);
			if (target == nullptr)
			{
				// 12.6.2p6: a mem-initializer naming the class itself.
				if (list->initializers.size != 1)
					error(initializer.location, "a delegating mem-initializer is not the only mem-initializer");
				constructor->init_count = 0;
				delegate_construction(constructor, entity, initializer, scope);
				expression_scope_ = saved_scope;
				return;
			}
			if (target->mem_initializer != nullptr)
				error(initializer.location, "a member or base initialized twice");
			target->mem_initializer = &initializer;
		}
	}
	// 12.6.2p8: at most one variant member of a union is initialized. The
	// members of an anonymous struct member (9.5p5) are named through that
	// one member object: mem-initializers for several of them designate it
	// once.
	for (uint32_t i = 0; is_union && list != nullptr && i < count; ++i)
	{
		const syntax::MemInitializer* designation = inits[i].designated ? plan_designated(inits[i]) : nullptr;
		if (designation != nullptr && variant_designated)
			error(designation->location, "a union's constructor initializes several members");
		variant_designated = variant_designated || designation != nullptr;
	}
	const bool user = constructor->body != nullptr && UserProvided(constructor);
	const ClassEntity* saved_object = base_object_;
	for (uint32_t i = 0; i < count; ++i)
	{
		SubobjectInit& init = inits[i];
		base_object_ = init.base != nullptr ? entity : nullptr;
		expression_scope_ = scope;
		if (init.base != nullptr && init.mem_initializer != nullptr)
		{
			const syntax::MemInitializer& written = *init.mem_initializer;
			syntax::Initializer view = syntax::Initializer();
			view.kind = written.braced ? syntax::InitializerKind::Braced : syntax::InitializerKind::Parenthesized;
			view.expression = written.list;
			view.arguments = written.arguments;
			view.location = written.location;
			ClassEntity* base = static_cast<ClassEntity*>(init.base->base);
			init.constructor = initialize_class(base, base->type, view, scope, written.location);
			continue;
		}
		if (init.base != nullptr && constructor->inherited != nullptr &&
			static_cast<const ClassEntity*>(init.base->base)->scope == constructor->inherited->owner)
		{
			// 12.9p8: the inherited constructor with the parameters. The
			// inheriting constructor has its access (12.9p4), checked where
			// it is used: a private one is declared, not an error.
			init.constructor = constructor->inherited;
			init.forwards = true;
			continue;
		}
		if (init.base != nullptr)
		{
			ClassEntity* base = static_cast<ClassEntity*>(init.base->base);
			init.constructor = default_constructor(base, location);
			// The plan itself is an implicit constructor's demand (the course
			// view shows only a definition's odr-uses).
			if (user)
				odr_use(init.constructor, true);
			continue;
		}
		member_plan(init, is_union, variant_designated, user, scope, location);
	}
	base_object_ = saved_object;
	expression_scope_ = saved_scope;
}

void Analyzer::member_plans(const ClassEntity* entity, SubobjectInit* out)
{
	// One entry per member; an anonymous union's or struct's member object
	// gets a plan of its own members (recursively).
	for (const DataMember* member = entity->members; member != nullptr; member = member->next)
	{
		SubobjectInit& init = out[member->ordinal];
		init = SubobjectInit{ nullptr, member, nullptr, nullptr, nullptr, false, false, 0, nullptr, nullptr };
		const ClassEntity* cls = member->variable->kind == EntityKind::Variable && member->variable->name == nullptr &&
				!member->bit_field
			? ObjectClass(member->variable->type)
			: nullptr;
		if (cls == nullptr || !cls->anonymous)
			continue;
		init.inner_count = cls->member_count;
		init.inner = static_cast<SubobjectInit*>(arena_.allocate(
			sizeof(SubobjectInit) * (cls->member_count == 0 ? 1 : cls->member_count), alignof(SubobjectInit)));
		member_plans(cls, init.inner);
	}
}

const syntax::MemInitializer* Analyzer::plan_designated(const SubobjectInit& init)
{
	// A mem-initializer names the member or one within it (the first in
	// declaration order), along the designated subobjects (see
	// mem_initializer_target).
	if (init.mem_initializer != nullptr || !init.designated)
		return init.mem_initializer;
	for (uint32_t i = 0; i < init.inner_count; ++i)
	{
		if (init.inner[i].designated)
			return plan_designated(init.inner[i]);
	}
	return nullptr;
}

void Analyzer::member_plan(
	SubobjectInit& init, bool variant, bool designated, bool user, Scope* scope, uint32_t location)
{
	// 12.6.2p8 for one member: its mem-initializer; a variant member (of a
	// union, `variant`) is not initialized when another is designated;
	// otherwise its brace-or-equal-initializer or default-initialization,
	// which a user-provided constructor may not leave to a reference or
	// const-qualified non-class member. A variant member is not
	// default-initialized. An anonymous union's or struct's member object
	// is initialized through its members (9.5p5).
	const DataMember* member = init.member;
	if (member->variable->kind != EntityKind::Variable)
		return;
	VariableEntity* variable = static_cast<VariableEntity*>(member->variable);
	if (init.mem_initializer != nullptr)
	{
		const syntax::MemInitializer& written = *init.mem_initializer;
		syntax::Initializer view = syntax::Initializer();
		view.kind = written.braced ? syntax::InitializerKind::Braced : syntax::InitializerKind::Parenthesized;
		view.expression = written.list;
		view.arguments = written.arguments;
		view.location = written.location;
		init.constructor = initialize_object(variable->type, view, scope, written.location);
		return;
	}
	if (init.inner != nullptr)
	{
		const ClassEntity* cls = ObjectClass(variable->type);
		const bool is_union = cls->key == TokenKind::KW_UNION;
		uint32_t designations = 0;
		for (uint32_t i = 0; is_union && i < init.inner_count; ++i)
			designations += init.inner[i].designated ? 1 : 0;
		if (designations > 1)
			error(location, "a constructor initializes several members of an anonymous union");
		if (!(variant && designated) || init.designated)
		{
			for (uint32_t i = 0; i < init.inner_count; ++i)
				member_plan(init.inner[i], is_union, designations != 0, user, scope, location);
		}
		return;
	}
	if (variant && designated)
		return;
	if (variable->initializer_pending)
		instantiate_member_initializer(variable);
	if (variable->member_initializer != nullptr)
	{
		variable->initializer_used = true;
		init.default_initializer = variable->member_initializer;
		init.constructor = variable->constructor;
		return;
	}
	if (variant)
		return;
	const Type* type = variable->type;
	if (type->is_reference())
	{
		if (user)
			error(location, "a reference member is not initialized");
		plan_deleted_ = plan_deleted_ || variable->name != nullptr;
		return;
	}
	if (ClassEntity* cls = ObjectClass(type))
	{
		init.constructor = default_constructor(cls, location);
		if (user && type->kind != TypeKind::Array)
			odr_use(init.constructor, true);
		const Type* element = type;
		while (element->kind == TypeKind::Array)
			element = element->element;
		if ((element->cv & kConst) != 0)
			check_const_default(cls, init.constructor, location);
		return;
	}
	const Type* object = type;
	while (object->kind == TypeKind::Array)
		object = object->element;
	if ((object->cv & kConst) != 0)
	{
		if (user)
			error(location, "a const member is not initialized");
		plan_deleted_ = true;
	}
}

SubobjectInit* Analyzer::mem_initializer_target(
	FunctionEntity* constructor, ClassEntity* entity, const syntax::MemInitializer& initializer, Scope* scope)
{
	// 12.6.2p2: the mem-initializer-id names a non-static data member of
	// the class or a direct base (by any name that denotes it); null: it
	// names the class itself (a delegating constructor, p6).
	const syntax::Name& name = *initializer.name;
	LookupResult result = LookupResult();
	// An identifier's lookup here: 0 none (lookup_name's checks its
	// access), 1 in the class, 2 where the constructor is.
	uint32_t looked_up = 0;
	if (!name.qualified() && name.last().kind == syntax::NameComponentKind::Identifier && !name.last().has_arguments)
	{
		result = names_.qualified(entity->scope, name.last().identifier, LookupFilter::Ordinary);
		looked_up = 1;
		if (!result.found())
		{
			result = unqualified(scope, name.last().identifier, LookupFilter::Ordinary);
			looked_up = 2;
		}
	}
	else
		result = lookup_name(name, scope, LookupFilter::Ordinary);
	if (!result.found())
		error(initializer.location, "the mem-initializer names nothing");
	if (result.ambiguous)
		error(initializer.location, "the mem-initializer name is ambiguous");
	const Binding* binding = result.binding;
	uint32_t index = 0;
	if (binding->kind == BindingKind::Variable)
	{
		const VariableEntity* variable = static_cast<const VariableEntity*>(binding->entity);
		const DataMember* member = variable->data_member;
		if (!variable->member || member == nullptr)
			error(initializer.location, "the mem-initializer does not name a non-static data member of the class");
		// A member of an anonymous union (or struct) member is a member of
		// the class (9.5p5): its plan is within that member object's (at
		// any depth of nesting).
		SmallList<const DataMember*, 8> path;
		for (const Scope* owner = variable->owner; owner != entity->scope; owner = member->variable->owner)
		{
			const ClassEntity* anonymous = owner->kind == ScopeKind::Class
				? static_cast<const ClassEntity*>(owner->entity)
				: nullptr;
			if (anonymous == nullptr || !anonymous->anonymous || anonymous->storage == nullptr ||
				anonymous->storage->entity->owner->kind != ScopeKind::Class)
				error(initializer.location, "the mem-initializer does not name a non-static data member of the class");
			path.push_back(member);
			member = static_cast<const VariableEntity*>(anonymous->storage->entity)->data_member;
		}
		SubobjectInit* target = &constructor->inits[entity->base_count + member->ordinal];
		for (size_t depth = path.size(); depth > 0; --depth)
		{
			target->designated = true;
			target = &target->inner[path[depth - 1]->ordinal];
		}
		target->designated = true;
		return target;
	}
	if (binding->kind != BindingKind::Type && binding->kind != BindingKind::TypeAlias)
		error(initializer.location, "the mem-initializer does not name a member or base");
	// 11.2p5: a type name found in the class (a base's injected-class-name
	// is a member of it, 11.1p5) is named there, so a private base's is
	// inaccessible below it; one found outside, as an unqualified name.
	if (looked_up == 1 && semantic() && template_depth_ == 0)
		check_access(binding, entity, nullptr, scope, initializer.location);
	else if (looked_up == 2 && semantic() && template_depth_ == 0)
		check_unqualified_access(binding, scope, initializer.location);
	const ClassEntity* named = ClassOf(binding_type(binding, initializer.location));
	if (named == entity)
		return nullptr;
	for (const BaseClass* base = entity->bases; base != nullptr; base = base->next, ++index)
	{
		if (base->base != named)
			continue;
		// 12.6.2p2: ill-formed when it is also an inherited virtual base.
		for (uint32_t i = 0; !base->is_virtual && i < VirtualBaseCount(entity); ++i)
		{
			if (VirtualBases(entity)[i].base->base == named)
				error(initializer.location, "the mem-initializer names both a direct base and a virtual base");
		}
		return &constructor->inits[index];
	}
	// 12.6.2p2: or a virtual base (an indirect one's plan follows the
	// members).
	for (uint32_t i = entity->base_count + entity->member_count; i < constructor->init_count; ++i)
	{
		if (constructor->inits[i].base != nullptr && constructor->inits[i].base->base == named)
			return &constructor->inits[i];
	}
	error(initializer.location, "the mem-initializer does not name a direct or virtual base");
}

void Analyzer::delegate_construction(
	FunctionEntity* constructor, ClassEntity* entity, const syntax::MemInitializer& initializer, Scope* scope)
{
	// 12.6.2p6: the target constructor, selected by overload resolution
	// for the initializer; a constructor delegating to itself, directly or
	// through the chain its target starts, is rejected.
	syntax::Initializer view = syntax::Initializer();
	view.kind = initializer.braced ? syntax::InitializerKind::Braced : syntax::InitializerKind::Parenthesized;
	view.expression = initializer.list;
	view.arguments = initializer.arguments;
	view.location = initializer.location;
	FunctionEntity* target = initialize_class(entity, entity->type, view, scope, initializer.location);
	for (const FunctionEntity* next = target; next != nullptr; next = next->delegate)
	{
		if (next == constructor)
			error(initializer.location, "a constructor delegates to itself");
	}
	constructor->delegation = &initializer;
	constructor->delegate = target;
}

void Analyzer::instantiate_member_initializer(VariableEntity* member)
{
	// A class template instance's member's brace-or-equal-initializer, on
	// the first constructor that uses it: analyzed in its complete class
	// as its class's instantiation would have (its lookups limited to the
	// template's definition context, 14.6.4.2).
	member->initializer_pending = false;
	const uint32_t index = pending_member_initializers_.find(member);
	if (index == PointerMap::kMissing)
		return;
	const PendingMemberInitializer pending = pending_member_scopes_[index];
	const uint32_t location = member->member_initializer->location;
	InstantiationContext context(*this, enter_instantiation(instantiation_depth_ + 1, location), false, pending.limit);
	member_default_initializer(member, pending.scope);
}

void Analyzer::member_default_initializer(VariableEntity* member, Scope* scope)
{
	// 9.2p4, 12.6.2p8: a non-static data member's brace-or-equal-
	// initializer, typed in the complete class where `this` denotes the
	// constructor's object; it copy- or list-initializes the member. A
	// template's non-dependent names are bound there (14.6p10), its
	// instances' initializers typed.
	if (template_depth_ != 0)
	{
		initializer_names(member->member_initializer, scope, false);
		return;
	}
	BodyContext* saved_body = body_;
	FunctionEntity* saved_demanding = demanding_;
	const uint32_t saved_epoch = current_epoch_;
	BodyContext context = BodyContext();
	context.this_type = types_.pointer(member->owner->entity->type);
	context.closure_context = member;
	body_ = &context;
	demanding_ = nullptr;
	current_epoch_ = root_epoch_;
	const uint32_t throwing = potentially_throwing_;
	member->constructor = initialize_object(
		member->type, *member->member_initializer, scope, member->member_initializer->location);
	// 15.4p14 (CWG 1351): the implicit constructor may throw what the
	// initializer's potentially-throwing expressions may (5.3.7p3: calls
	// among them, which the initialization's count does not see).
	const syntax::Expr* expression = member->member_initializer->expression;
	member->throwing_initializer = potentially_throwing_ != throwing ||
		(expression != nullptr && may_throw(*expression, 0));
	member->initializer_typed = true;
	body_ = saved_body;
	demanding_ = saved_demanding;
	current_epoch_ = saved_epoch;
	typed_inheriting_plans(member);
}

void Analyzer::typed_inheriting_plans(const VariableEntity* member)
{
	// The class's inheriting constructors are planned when it is complete
	// (see declare_inherited_constructors), before its members' brace-or-
	// equal-initializers are typed (finish_class_nesting): their entries
	// for `member` take its initialization's constructor now, as a plan
	// made later does (member_plan; 12.9p8 with 12.6.2p8). A member of an
	// anonymous union or struct member is the enclosing class's (9.5p5).
	ClassEntity* cls = static_cast<ClassEntity*>(member->owner->entity);
	while (cls->anonymous && cls->storage != nullptr && cls->storage->entity->owner->kind == ScopeKind::Class)
		cls = static_cast<ClassEntity*>(cls->storage->entity->owner->entity);
	if (cls->inherits_constructors == nullptr)
		return;
	// Whether they are constexpr turns on the typed initializer too (see
	// constexpr_inheriting_plan), and with it whether the class is a
	// literal type: an answer given before is established again.
	if (cls->literal_state != 3)
		cls->literal_state = 0;
	if (member->constructor == nullptr)
		return;
	const NameEntry* entry = names_.find(cls->scope, constructor_key_);
	for (const FunctionList* node = entry != nullptr ? entry->functions : nullptr; node != nullptr; node = node->next)
	{
		FunctionEntity* constructor = static_cast<FunctionEntity*>(node->binding->entity);
		if (constructor->inherited != nullptr && constructor->owner == cls->scope)
			TypedMemberInits(constructor->inits, constructor->init_count, member);
	}
}

} // namespace sema
