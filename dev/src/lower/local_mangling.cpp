#include "lower/program_lowering.h"

#include <cstdio>

#include "sema/analyzer.h"

// The names Itanium leaves to the implementation, as Clang gives them:
// `$_<n>` for an unnamed class, enumeration or closure type no other unit
// names, numbered as Clang's ItaniumMangleContext numbers them when it
// first mangles one; the discriminators of a local class or static
// variable of a function neither inline nor a template specialization,
// counted as mangled; and the order in which Clang names the classes whose
// member functions are defined in their class, which those numbers follow.
namespace lower {

using abi_mangle::AbiNode;

void ProgramLowering::name_deferred_members(const syntax::Decl& decl)
{
	// The `$_<n>` numbers Clang gives as it mangles (see anonymous_ordinal)
	// include those of the classes whose member functions are defined in
	// their class: it defers each such definition and names it at the end
	// of the top-level declaration holding it (a namespace's whole
	// definition), in the order they were parsed, used or not. The classes
	// of a namespace-scope declaration that are named so are named here,
	// after the declaration's own code (a function body, an initializer);
	// a function body's local classes are named where first used.
	switch (decl.kind)
	{
	case syntax::DeclKind::Namespace:
		for (const syntax::Decl* member : static_cast<const syntax::NamespaceDecl&>(decl).declarations)
			name_deferred_members(*member);
		return;
	case syntax::DeclKind::Linkage:
		for (const syntax::Decl* member : static_cast<const syntax::LinkageDecl&>(decl).declarations)
			name_deferred_members(*member);
		return;
	case syntax::DeclKind::Class:
	{
		const syntax::DeclSpecifier& specifier = static_cast<const syntax::ClassDecl&>(decl).specifier;
		if (specifier.kind == syntax::DeclSpecKind::Class)
			name_member_classes(*specifier.class_specifier);
		return;
	}
	case syntax::DeclKind::Simple:
		for (const syntax::DeclSpecifier& specifier : static_cast<const syntax::SimpleDecl&>(decl).specifiers)
		{
			if (specifier.kind == syntax::DeclSpecKind::Class)
				name_member_classes(*specifier.class_specifier);
		}
		return;
	default:
		// A template's members are named by its specializations' uses.
		return;
	}
}

void ProgramLowering::name_member_classes(const syntax::ClassSpecifier& specifier)
{
	// A class (and its member classes, in declaration order) with a member
	// function defined in it, when an unnamed class's `$_<n>` is part of
	// its name: its name, which numbers that class (see
	// name_deferred_members).
	bool named = false;
	for (const syntax::Decl* member : specifier.members)
	{
		const sema::Binding* declared = nullptr;
		if (member->kind == syntax::DeclKind::Function)
		{
			const syntax::FunctionDecl& definition = static_cast<const syntax::FunctionDecl&>(*member);
			if (definition.body.body != nullptr || definition.body.try_block != nullptr)
				declared = definition.declared;
		}
		else if (member->kind == syntax::DeclKind::SpecialMember)
		{
			const syntax::SpecialMemberDecl& special = static_cast<const syntax::SpecialMemberDecl&>(*member);
			if (special.definition)
				declared = special.declared;
		}
		else
		{
			name_deferred_members(*member);
			continue;
		}
		if (named || declared == nullptr || declared->entity == nullptr || declared->entity->owner == nullptr ||
			declared->entity->owner->kind != sema::ScopeKind::Class)
			continue;
		named = true;
		const sema::ClassEntity* cls = static_cast<const sema::ClassEntity*>(declared->entity->owner->entity);
		bool unnamed = false;
		for (const sema::Entity* type = cls; type != nullptr && !unnamed;
			type = type->owner != nullptr && type->owner->kind == sema::ScopeKind::Class ? type->owner->entity
																						 : nullptr)
			unnamed = type->name == nullptr;
		if (unnamed && (cls->templ == nullptr || cls->templ->role != sema::TemplateRole::Template))
			class_name(cls);
	}
}

bool ProgramLowering::numbered_closure(const sema::ClassEntity* closure)
{
	// Itanium 5.1.8 numbers a closure type where the ODR makes the closure
	// types of different units correspond: a lambda of a default member
	// initializer, of a default argument in a class definition or of a
	// templated variable's initializer (the closure's context), or one an
	// inline function's or a template specialization's body lexically
	// holds, a lambda's body included (its function call operator is
	// inline). Clang names any other `$_<n>` (pa27's
	// 200-host-eh-local-lambda-symbol-binding): one of a namespace-scope
	// variable's initializer, of a non-inline function's body.
	const sema::ClosureFacts* facts = graph_->closure_facts(closure);
	if (facts != nullptr && facts->context != nullptr)
		return true;
	for (const sema::FunctionEntity* function = EnclosingFunction(closure->owner); function != nullptr;
		function = EnclosingFunction(function->owner))
	{
		if (function->is_inline || function->instantiated)
			return true;
	}
	return false;
}

bool ProgramLowering::visible_type(const sema::Entity* entity)
{
	// Whether other units name a class or enumeration (Clang's
	// isExternallyVisible): a local one when the outermost function whose
	// body holds it is inline or a template specialization (3.2p6 makes
	// its definitions correspond; Clang's VisibleNoLinkage), another when
	// it has external linkage (3.5p4: a named one, or a member of such a
	// class, of a namespace not unnamed, over such template arguments).
	const sema::FunctionEntity* enclosing = EnclosingFunction(entity->owner);
	if (enclosing != nullptr)
	{
		const sema::FunctionEntity* outer = OutermostFunction(enclosing);
		return outer->is_inline || outer->instantiated;
	}
	return entity_locality(entity) == Locality::External;
}

uint32_t ProgramLowering::local_discriminator(const sema::Entity* entity, uint32_t declared)
{
	// Itanium 5.1.7's discriminator of a local class or static variable: the
	// count of its function's entities of its name before it (0 for the
	// first, which takes none). Where other units name it (see
	// visible_type) the declaration order the analysis counted, as Clang's
	// Sema numbers them; Clang numbers another's in the order the unit
	// mangles them, a function's classes and variables of a name together
	// (its ItaniumMangleContext's discriminators), so one the unit never
	// mangles takes no number (an unused class of a name in an inner
	// block leaves the next one undiscriminated).
	const sema::FunctionEntity* function = entity->name != nullptr ? EnclosingFunction(entity->owner) : nullptr;
	if (function == nullptr)
		return declared;
	const sema::FunctionEntity* outer = OutermostFunction(function);
	if (outer->is_inline || outer->instantiated)
		return declared;
	const uint32_t known = local_discriminators_.find(entity);
	if (known != PointerMap::kMissing)
		return known;
	const LocalName key = LocalName{ function, entity->name };
	uint32_t* count = local_name_counts_.find(key);
	uint32_t earlier = 0;
	if (count == nullptr)
		local_name_counts_.insert(key, 1);
	else
		earlier = (*count)++;
	local_discriminators_.set(entity, earlier);
	return earlier;
}

void ProgramLowering::name_local_classes(const sema::FunctionEntity* function)
{
	// A function body's local classes whose member functions are defined in
	// them, as Clang names them at the end of the function's top-level
	// declaration (see name_deferred_members): in the order they were
	// defined, those of its closures' and local classes' bodies included.
	// Only a function neither inline nor a template specialization numbers
	// them as mangled (`$_<n>`, local_discriminator).
	if (graph_->local_classes.empty() || function->is_inline || function->instantiated)
		return;
	if (!local_classes_linked_)
	{
		local_classes_linked_ = true;
		const std::vector<const sema::ClassEntity*>& classes = graph_->local_classes;
		local_class_next_.assign(classes.size(), PointerMap::kMissing);
		for (size_t i = classes.size(); i-- > 0;)
		{
			const sema::FunctionEntity* enclosing = EnclosingFunction(classes[i]->owner);
			if (enclosing == nullptr || closure_call(classes[i]) != nullptr)
				continue;
			const sema::FunctionEntity* outer = OutermostFunction(enclosing);
			local_class_next_[i] = local_class_heads_.find(outer);
			local_class_heads_.set(outer, static_cast<uint32_t>(i));
		}
	}
	for (uint32_t i = local_class_heads_.find(function); i != PointerMap::kMissing; i = local_class_next_[i])
	{
		const sema::ClassEntity* cls = graph_->local_classes[i];
		if (cls->scope == nullptr)
			continue;
		for (const sema::Binding* binding = cls->scope->bindings; binding != nullptr; binding = binding->next)
		{
			const sema::Entity* member = binding->entity;
			if (binding->kind == sema::BindingKind::Function && member != nullptr &&
				member->kind == sema::EntityKind::Function && member->owner == cls->scope &&
				!static_cast<const sema::FunctionEntity*>(member)->implicit &&
				static_cast<const sema::FunctionEntity*>(member)->body != nullptr)
			{
				class_name(cls);
				break;
			}
		}
	}
}

AbiNode ProgramLowering::anonymous_name(const sema::Entity* type)
{
	// Clang's name of an unnamed type (a closure type included) other units
	// do not name, `$_<n>` (see anonymous_ordinal); a local one's is
	// numbered in the function whose body declares it.
	const sema::FunctionEntity* context = type->owner != nullptr && type->owner->kind != sema::ScopeKind::Class
		? EnclosingFunction(type->owner)
		: nullptr;
	char spelling[16];
	const int length = std::snprintf(spelling, sizeof spelling, "$_%u", anonymous_ordinal(type, context));
	return abi_graph_.string(spelling, static_cast<size_t>(length));
}

uint32_t ProgramLowering::anonymous_ordinal(const sema::Entity* type, const sema::FunctionEntity* context)
{
	// The number of a `$_<n>` name, given the first time the unit mangles
	// the type (Clang's order of mangling, its ItaniumMangleContext's
	// anonymous-structure ids): a function's local type's is its count of
	// the function's types numbered before it; any other's the count of
	// all the unit numbered before it, local ones included.
	uint32_t ordinal = anonymous_ordinals_.find(type);
	if (ordinal != PointerMap::kMissing)
		return ordinal;
	if (context != nullptr)
	{
		const uint32_t known = anonymous_counts_.find(context);
		ordinal = known == PointerMap::kMissing ? 0 : known;
		anonymous_counts_.set(context, ordinal + 1);
	}
	else
		ordinal = anonymous_total_;
	++anonymous_total_;
	anonymous_ordinals_.set(type, ordinal);
	return ordinal;
}

} // namespace lower
