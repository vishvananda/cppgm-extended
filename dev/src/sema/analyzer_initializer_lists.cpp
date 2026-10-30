#include "sema/analyzer.h"

#include "lex/identifier_table.h"
#include "sema/aggregates.h"

// std::initializer_list: the program declares the class template in
// namespace std (8.5.4p2: it is not predefined), defined or not. A
// braced-init-list initializing a specialization std::initializer_list<E>
// materializes an array of its elements' copies of type const E and the
// object refers to it (8.5.4p5); an initializer-list constructor takes the
// whole list first (13.3.1.7); a list converts to the class by its
// elements' worst conversion (13.3.3.1.5p2); auto deduces from a list
// (7.1.6.4p6) and a parameter from each element (14.8.2.1p1). A template
// the program leaves undefined is completed by the course's builtin layout:
// the array's first element's address, then its element count.
namespace sema {

bool Analyzer::is_initializer_list_template(const ClassEntity* templ) const
{
	// The class template std::initializer_list.
	return templ != nullptr && templ->name == initializer_list_key_ && in_std(templ);
}

const ClassEntity* Analyzer::implementation_template(const char* name, uint32_t parameters)
{
	// The defined class template __cppgm_<name> of the builtin declarations,
	// of `parameters` template parameters; null if there is none.
	const std::string spelling = std::string("__cppgm_") + name;
	const NameEntry* entry = names_.find(graph_.global, special_key(spelling.c_str()));
	const Binding* binding = entry != nullptr ? entry->type : nullptr;
	const ClassEntity* found = binding != nullptr && binding->entity != nullptr &&
			binding->entity->kind == EntityKind::Class
		? static_cast<const ClassEntity*>(binding->entity)
		: nullptr;
	if (found == nullptr || found->templ == nullptr || found->templ->role != TemplateRole::Template ||
		found->templ->definition == nullptr || found->templ->head == nullptr || found->templ->head->count != parameters)
		return nullptr;
	return found;
}

bool Analyzer::in_std(const ClassEntity* entity) const
{
	// Declared in namespace std (or an inline namespace of it).
	if (entity->owner == nullptr)
		return false;
	for (const Scope* scope = entity->owner; scope != nullptr && scope->kind == ScopeKind::Namespace;
		scope = scope->parent)
	{
		const NamespaceEntity* space = static_cast<const NamespaceEntity*>(scope->entity);
		if (space == nullptr)
			return false;
		if (space->name == std_key_)
			return scope->parent == graph_.global;
		if (!space->is_inline)
			return false;
	}
	return false;
}

const Type* Analyzer::initializer_list_element(const Type* type) const
{
	// E of std::initializer_list<E> (or a reference to one, cv-qualified),
	// dependent or not; null for another type.
	if (type == nullptr)
		return nullptr;
	if (type->is_reference())
		type = type->element;
	type = type->unqualified;
	if (type->kind != TypeKind::Class)
		return nullptr;
	const ClassEntity* cls = static_cast<const ClassEntity*>(type->entity);
	const ClassTemplateFacts* facts = cls->templ;
	if (facts == nullptr || facts->role != TemplateRole::Specialization || facts->argument_count != 1 ||
		!is_initializer_list_template(facts->pattern))
		return nullptr;
	return facts->arguments[0];
}

ClassEntity* Analyzer::declared_initializer_list(uint32_t location)
{
	// The class template std::initializer_list, which the program must have
	// declared to use it (8.5.4p2, 7.1.6.4p6: even when the type is not
	// named).
	const LookupResult space = names_.qualified(graph_.global, std_key_, LookupFilter::Namespace);
	const LookupResult found = space.found() && space.binding->entity != nullptr &&
			space.binding->entity->kind == EntityKind::Namespace
		? names_.qualified(static_cast<NamespaceEntity*>(space.binding->entity)->scope, initializer_list_key_,
			  LookupFilter::Elaborated)
		: LookupResult();
	ClassEntity* templ = found.found() && !found.ambiguous ? class_template_of(found.binding->entity) : nullptr;
	if (templ == nullptr || !is_initializer_list_template(templ))
		error(location, "std::initializer_list used without its declaration (8.5.4p2)");
	return templ;
}

const Type* Analyzer::initializer_list_of(const Type* element, uint32_t location)
{
	// std::initializer_list<E>.
	ClassEntity* templ = declared_initializer_list(location);
	TypeList arguments;
	arguments.push_back(element->unqualified);
	ClassEntity* specialization = class_specialization(templ, arguments, location);
	return specialization != nullptr ? specialization->type : nullptr;
}

void Analyzer::builtin_initializer_list(ClassEntity* entity)
{
	// A specialization of a template the program declared without defining
	// it (the course's <initializer_list> is the program's): complete, no
	// aggregate, with the array's first element and element count, named by
	// keys no program can spell (see builtin_list_key).
	const Type* element = initializer_list_element(entity->type);
	entity->scope = new_scope(ScopeKind::Class, entity->owner, entity->owner, entity, entity->label);
	entity->scope->dumped = false;
	entity->complete = true;
	entity->not_aggregate = true;
	const Type* types[2] = { types_.pointer(types_.qualified(element, kConst)),
		types_.fundamental(FundamentalType::LongInt) };
	for (unsigned i = 0; i < 2; ++i)
	{
		VariableEntity* member = new_entity<VariableEntity>(
			EntityKind::Variable, builtin_list_key(i), entity->scope, types[i]);
		member->member = true;
		add_data_member(entity, member, 0, false);
		Binding* binding = new_binding(BindingKind::Variable, member->name, member, types[i]);
		binding->declared_in = entity->scope;
		binding->access = Access::Public;
		names_.declare(entity->scope, binding);
	}
	if (semantic())
	{
		declare_destructor(entity, 0);
		classify_copy_members(entity);
		virtual_facts(entity, 0);
	}
}

const Identifier* Analyzer::builtin_list_key(unsigned member)
{
	// The builtin layout's members: the array's address, its element count.
	if (builtin_list_keys_[member] == nullptr)
		builtin_list_keys_[member] = special_key(
			member == 0 ? "<initializer_list __begin>" : "<initializer_list __size>");
	return builtin_list_keys_[member];
}

bool Analyzer::builtin_list_class(const ClassEntity* entity) const
{
	// A specialization completed by the builtin layout.
	const Type* element = entity != nullptr ? initializer_list_element(entity->type) : nullptr;
	return element != nullptr && entity->templ->pattern->templ != nullptr &&
		entity->templ->pattern->templ->definition == nullptr;
}

bool Analyzer::initializer_list_sequence(
	const syntax::BracedInitList& list, const Type* parameter, const Type* element, Conversion& out)
{
	// 13.3.3.1.5p2: when every element converts to E, the worst of those
	// conversions (identity for an empty list); a reference binds the
	// object, a temporary (p5).
	if (parameter->is_reference() && parameter->kind == TypeKind::LValueReference &&
		(parameter->element->cv & (kConst | kVolatile)) != kConst)
		return false;
	Conversion sequence = Conversion();
	sequence.form = ConversionForm::Standard;
	sequence.rank = ConversionRank::Exact;
	for (const syntax::Expr* item : list.elements)
	{
		Conversion step = Conversion();
		if (item->kind == syntax::ExprKind::BracedInit
				? !list_sequence(static_cast<const syntax::BracedInitList&>(*item), element, step)
				: !implicit_sequence(*item, element, step))
			return false;
		if (conversions_.compare(step, sequence) < 0)
			sequence = step;
	}
	sequence.form = ConversionForm::Standard;
	sequence.constructor = nullptr;
	sequence.argument = nullptr;
	sequence.after = nullptr;
	sequence.initializer_list = true;
	sequence.target = parameter;
	sequence.result = parameter->is_reference() ? parameter->element : parameter->unqualified;
	if (parameter->is_reference())
	{
		sequence.form = ConversionForm::Reference;
		sequence.rvalue_reference = parameter->kind == TypeKind::RValueReference;
		sequence.temporary = true;
	}
	out = sequence;
	return true;
}

ExprFacts* Analyzer::initializer_list_object(const syntax::BracedInitList& list, const Type* type, Scope* scope)
{
	// 8.5.4p5: the object referring to the array its elements initialize:
	// the list stands for that object, a prvalue (the array its facts'
	// target).
	const Type* array = initializer_list_array(list, type, scope);
	if (array == nullptr)
		return nullptr;
	ExprFacts* facts = new_facts(list, type->unqualified, ValueCategory::PRValue);
	facts->flags |= kInitializerList;
	facts->target = array;
	return facts;
}

const Type* Analyzer::initializer_list_array(const syntax::BracedInitList& list, const Type* type, Scope* scope)
{
	// 8.5.4p5: the array `const E[N]` whose elements are copy-initialized
	// from the list's elements in order (each braced one list-initializes
	// its element).
	const Type* element = initializer_list_element(type);
	if (type->is_reference())
		type = type->element;
	ClassEntity* cls = static_cast<ClassEntity*>(type->unqualified->entity);
	if (!require_complete(cls))
		error(list.location, "std::initializer_list of an incomplete specialization");
	const Type* array = types_.array(types_.qualified(element, kConst), list.elements.size);
	++empty_list_epoch_;
	for (uint32_t next = 0; next < list.elements.size;)
	{
		const syntax::Expr& item = *list.elements[next];
		if (item.kind != syntax::ExprKind::BracedInit && IsSubaggregate(element) && item.facts != nullptr &&
			!InitializesSubaggregate(element, item))
			error(item.location, "an initializer_list element that does not convert to its element type");
		aggregate_element(list, next, element, scope);
		if (failed())
			return nullptr;
	}
	return array;
}

bool Analyzer::initializer_list_constructor(const FunctionEntity* constructor) const
{
	// 8.5.4p2: its first parameter std::initializer_list<E> or a reference
	// to one, its others with default arguments (a template's, deduced).
	const Type* type = constructor->type->canonical;
	if (type->param_count == 0 || initializer_list_element(type->params[0]) == nullptr)
		return false;
	for (uint32_t i = 1; i < type->param_count; ++i)
	{
		if (constructor->default_arguments == nullptr || constructor->default_arguments[i] == nullptr)
			return false;
	}
	return true;
}

FunctionEntity* Analyzer::list_constructor_phase(
	ClassEntity* entity, const syntax::BracedInitList& list, std::vector<FunctionEntity*>& set, Candidate& chosen)
{
	// 13.3.1.7p1's first phase: the initializer-list constructors, the list
	// as their one argument (a constructor template's specialization its
	// elements deduce, 14.8.2.1p1); null when none is viable (an ambiguity
	// is one: the second phase is not tried, the call ill-formed).
	syntax::Expr* argument = const_cast<syntax::BracedInitList*>(&list);
	syntax::Slice<syntax::Expr*> arguments;
	arguments.data = &argument;
	arguments.size = 1;
	constructor_candidates(entity, arguments, ConstructionKind::CopyList, set);
	size_t kept = 0;
	for (FunctionEntity* candidate : set)
	{
		if (initializer_list_constructor(candidate))
			set[kept++] = candidate;
	}
	set.resize(kept);
	if (set.empty())
		return nullptr;
	const Resolution resolution = resolve(arguments, set, nullptr, chosen, entity);
	if (resolution == Resolution::Ambiguous)
		error(list.location, "an ambiguous initializer-list constructor (13.3.1.7)");
	return resolution == Resolution::Selected ? chosen.function : nullptr;
}

FunctionEntity* Analyzer::select_list_constructor(ClassEntity* entity, ExprFacts* facts,
	const syntax::BracedInitList& list, ConstructionKind kind, Scope* scope, uint32_t location)
{
	// 13.3.1.7p1's first phase for an object the list initializes: the
	// selected initializer-list constructor takes the object referring to
	// the array its elements initialize (the list's facts: `kind`'s
	// construction by `entity`, the array their target); null when no
	// initializer-list constructor is viable.
	std::vector<FunctionEntity*>& set = overload_set_;
	set.clear();
	Candidate chosen = Candidate();
	FunctionEntity* found = list_constructor_phase(entity, list, set, chosen);
	if (found == nullptr)
		return nullptr;
	FunctionEntity* selected = inherited_specialization(entity, found, 1, location);
	if (selected == nullptr)
		return nullptr;
	check_inheriting_access(selected, entity, location);
	if (kind == ConstructionKind::CopyList && selected->is_explicit)
		error(location, "copy-list-initialization selects an explicit constructor");
	check_not_deleted(selected, location);
	check_special_access(selected, entity, expression_scope_, location);
	const Type* parameter = selected->type->canonical->params[0];
	const Type* array = initializer_list_array(list, parameter, scope);
	if (array == nullptr)
		return nullptr;
	facts->flags |= kInitializerList;
	facts->target = array;
	odr_use(selected, true);
	if (!selected->nothrow)
		++potentially_throwing_;
	return selected;
}

const Type* Analyzer::list_deduced_type(const Specifiers& specifiers, const Type* declared,
	const syntax::BracedInitList& list, Scope* scope, uint32_t location)
{
	// 7.1.6.4p6: auto replaced by std::initializer_list<U>, U deduced from
	// each element as from an argument of parameter type U (14.8.2.1p1), all
	// alike. The elements are typed once (their initialization reads these
	// facts); in a template definition a dependent element's type stands
	// for the declaration's, which its instantiations deduce.
	const Type* element = nullptr;
	for (const syntax::Expr* item : list.elements)
	{
		// A braced element deduces nothing (14.8.2.1p1: U is no
		// std::initializer_list); it initializes its element as U.
		if (item->kind == syntax::ExprKind::BracedInit)
			continue;
		const Type* type = nullptr;
		ValueCategory category = ValueCategory::PRValue;
		if (semantic() && template_depth_ == 0)
		{
			BodyContext* saved = body_;
			const syntax::Initializer* saved_initializing = initializing_;
			BodyContext context = BodyContext();
			if (body_ == nullptr)
				body_ = &context;
			ExprFacts* facts = typed_value(*item, scope);
			body_ = saved;
			initializing_ = saved_initializing;
			if (facts == nullptr)
				return declared;
			if (saved != nullptr && (facts->flags & kOverloadSet) == 0)
				facts->flags |= kImplicit;
			type = facts->type;
			category = facts->category;
		}
		else
		{
			type = decltype_type(*item, scope);
			if (template_depth_ != 0 && dependent_type(type))
				return type;
			if (type->kind == TypeKind::LValueReference || item->kind == syntax::ExprKind::Id)
				category = ValueCategory::LValue;
			if (type->is_reference())
				type = type->element;
		}
		TypeList deduced;
		deduced.push_back(nullptr);
		if (type == nullptr || !deduce_argument(auto_placeholder_, type, category, 0, deduced) ||
			deduced[0] == nullptr || (element != nullptr && deduced[0]->canonical != element->canonical))
			error(item->location, "auto deduction from a braced-init-list's elements fails (7.1.6.4p6)");
		element = deduced[0];
	}
	if (element == nullptr)
		error(location, "auto deduced from a braced-init-list without an element that deduces U (7.1.6.4p6)");
	const Type* replacing = initializer_list_of(element, location);
	if (auto_deduced_ != nullptr && replacing->canonical != auto_deduced_->canonical)
		error(location, "auto deduced as different types by one declaration's declarators (7.1.6.4p7)");
	auto_deduced_ = replacing;
	return replaced_auto(declared, replacing, specifiers.cv);
}

bool Analyzer::deduce_list_elements(
	const Type* p, const syntax::BracedInitList& list, uint32_t depth, TypeList& arguments)
{
	// 14.8.2.1p1, CWG 1591: a nonempty list deduces each element of an
	// initializer_list<P'> or P'[N], and a named bound from its length.
	// Other types and empty lists remain non-deduced contexts.
	if (list.elements.size == 0)
		return true;
	const Type* object = p->is_reference() ? p->element : p;
	const Type* array = object->unqualified->kind == TypeKind::Array ? object : nullptr;
	const Type* element = array != nullptr ? array->element : initializer_list_element(p);
	if (element == nullptr)
		return true;
	if (array != nullptr && array->owner != nullptr && !deduce_array_bound(array, list.elements.size, depth, arguments))
		return false;
	for (const syntax::Expr* item : list.elements)
	{
		if (item->kind == syntax::ExprKind::BracedInit)
		{
			if (!deduce_list_elements(element, static_cast<const syntax::BracedInitList&>(*item), depth, arguments))
				return false;
			continue;
		}
		const ExprFacts* facts = item->facts;
		if (facts == nullptr || (facts->flags & kBoundMember) != 0)
			return false;
		if ((facts->flags & kOverloadSet) != 0)
			deduce_overloaded(element, facts->functions, depth, arguments);
		else if (facts->type == nullptr || !deduce_argument(element, facts->type, facts->category, depth, arguments))
			return false;
	}
	return true;
}

} // namespace sema
