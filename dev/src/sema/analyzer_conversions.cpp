#include "sema/analyzer.h"

#include "lex/identifier_table.h"

// User-defined conversions (12.3; semantics mode): converting constructors
// and conversion functions as implicit conversion sequences (13.3.1.4-
// 13.3.1.6, 13.3.3.1.2), and class operands of built-in operators and
// conditions converted by a conversion function (13.3.1.2p7, 4p3). A
// converted operand keeps its own facts under the call's (kUserConverted).
// The conditional operator's operands converted to each other's or a
// common type (5.16) are here too.
namespace sema {

namespace {

const syntax::Expr* WithoutParentheses(const syntax::Expr* expr)
{
	while (expr->kind == syntax::ExprKind::Paren)
		expr = static_cast<const syntax::ParenExpr*>(expr)->inner;
	return expr;
}

// The complete class a base-specifier names when it has conversion
// functions to contribute (a dependent base names none).
const ClassEntity* ConversionBase(const BaseClass* base)
{
	if (base->base == nullptr || base->base->kind != EntityKind::Class)
		return nullptr;
	const ClassEntity* cls = static_cast<const ClassEntity*>(base->base);
	return cls->has_conversion_functions && cls->complete ? cls : nullptr;
}

} // namespace

void Analyzer::conversion_functions(const ClassEntity* entity, std::vector<FunctionEntity*>& out, bool templates)
{
	// 12.3.2, 13.3.1.5p1: the class's conversion functions and its bases'
	// that are not hidden within it (12.3p5), see visible_conversions.
	// With `templates`, conversion function templates too (14.5.2p6: a
	// context that knows the type it needs deduces their specializations,
	// 14.8.2.3); a derived template hides a base's of the same form.
	if (!entity->has_conversion_functions)
		return;
	const ConversionSet set = conversion_set(entity);
	for (uint32_t i = 0; i < set.count; ++i)
	{
		FunctionEntity* function = static_cast<FunctionEntity*>(set.bindings[i]->entity);
		if (templates || !function->templated)
			out.push_back(function);
	}
}

Analyzer::ConversionSet Analyzer::conversion_set(const ClassEntity* entity)
{
	// A complete class's set is computed once and kept; an incomplete
	// class's (in its own definition) is the scratch's until the next call.
	const uint32_t index = conversion_sets_.find(entity);
	if (index != PointerMap::kMissing)
		return conversion_set_list_[index];
	visible_conversions(entity);
	if (!entity->complete)
		return ConversionSet{ conversion_set_scratch_.data(), static_cast<uint32_t>(conversion_set_scratch_.size()) };
	conversion_sets_.set(entity, static_cast<uint32_t>(conversion_set_list_.size()));
	conversion_set_list_.push_back(
		ConversionSet{ arena_.copy_array(conversion_set_scratch_.data(), conversion_set_scratch_.size()),
			static_cast<uint32_t>(conversion_set_scratch_.size()) });
	return conversion_set_list_.back();
}

void Analyzer::visible_conversions(const ClassEntity* entity)
{
	// The declarations of the conversion functions and templates visible
	// in `entity`, into the scratch: each class's own (its members and
	// those its using-declarations introduce, which act as its members
	// here, 7.3.3p15-p16) unless a class derived from it on the path the
	// walk took has one of the same kind (template or not) with their
	// return type (12.3p5), each function once; bases depth-first in
	// declaration order (an explicit stack: a deep hierarchy nests no
	// recursion). A class reached again through a non-virtual
	// base-specifier is another subobject (10.2): what that path does not
	// hide joins the set too, so overload resolution sees every
	// subobject's (13.3.1p4 ranks their object arguments alike: two are
	// ambiguous; a function of a repeated base's several subobjects must
	// not be selected, conversion_subobject). Each class is walked at most
	// twice, the first and second subobject being enough to tell an
	// ambiguity. The hiding maps count the path's functions and templates
	// by return type (one probe per function; no longer kept once no class
	// on the path has a base left to walk). `seen` holds each class's
	// walks and, from the first function that can be met twice (a second
	// walk's, a using-declaration's), the functions kept.
	std::vector<Binding*>& out = conversion_set_scratch_;
	std::vector<ConversionStep>& path = conversion_path_;
	PointerMap& seen = conversion_seen_;
	out.clear();
	path.clear();
	seen.clear();
	conversion_hiding_[0].clear();
	conversion_hiding_[1].clear();
	bool shared = false, checked = false;
	uint32_t open = 0;
	const ClassEntity* cls = entity;
	while (true)
	{
		if (cls != nullptr)
		{
			const uint32_t walks = seen.find(cls);
			seen.set(cls, walks == PointerMap::kMissing ? 1 : walks + 1);
			closure_conversion(cls);
			const NameEntry* entry = cls->scope != nullptr ? names_.find(cls->scope, conversion_key_) : nullptr;
			const FunctionList* own = entry != nullptr ? entry->functions : nullptr;
			for (const FunctionList* node = own; node != nullptr; node = node->next)
			{
				const FunctionEntity* function = static_cast<const FunctionEntity*>(node->binding->entity);
				const uint32_t hiders = conversion_hiding_[function->templated].find(
					function->type->canonical->element);
				if (hiders != PointerMap::kMissing && hiders != 0)
					continue;
				if (!checked && (walks != PointerMap::kMissing || function->owner != cls->scope))
				{
					for (const Binding* kept : out)
						seen.set(kept->entity, 1);
					checked = true;
				}
				if (checked && seen.find(function) != PointerMap::kMissing)
					continue;
				if (checked)
					seen.set(function, 1);
				out.push_back(node->binding);
			}
			// Hidden or not, they hide their bases' along this path.
			count_conversion_hiders(own, 1);
			path.push_back(ConversionStep{ cls, own, cls->bases });
			open += cls->bases != nullptr ? 1 : 0;
		}
		if (path.empty())
			break;
		// The next base of the innermost class with one left to walk.
		ConversionStep& step = path.back();
		open -= step.base != nullptr ? 1 : 0;
		cls = nullptr;
		for (; step.base != nullptr && cls == nullptr; step.base = step.base->next)
		{
			const ClassEntity* next = ConversionBase(step.base);
			const uint32_t walks = next != nullptr ? seen.find(next) : PointerMap::kMissing;
			// A virtual base reached again is the same subobject.
			if (next != nullptr && (walks == PointerMap::kMissing || (walks < 2 && !step.base->is_virtual)))
			{
				cls = next;
				shared = shared || step.base->is_virtual;
			}
		}
		open += step.base != nullptr ? 1 : 0;
		if (cls == nullptr)
		{
			if (open != 0)
				count_conversion_hiders(step.own, ~0u);
			path.pop_back();
		}
	}
	if (shared)
		dominated_conversions(entity);
}

void Analyzer::count_conversion_hiders(const FunctionList* own, uint32_t delta)
{
	// Adds `delta` (1, or -1 as ~0) to the hiding count of each function's
	// return type in its kind's map.
	for (const FunctionList* node = own; node != nullptr; node = node->next)
	{
		const FunctionEntity* function = static_cast<const FunctionEntity*>(node->binding->entity);
		PointerMap& hiding = conversion_hiding_[function->templated];
		const Type* result = function->type->canonical->element;
		const uint32_t hiders = hiding.find(result);
		hiding.set(result, (hiders == PointerMap::kMissing ? 0u : hiders) + delta);
	}
}

void Analyzer::dominated_conversions(const ClassEntity* cls)
{
	// A walk through a virtual base-specifier kept a shared subobject's
	// functions where the first path to it left them visible: those
	// dominated in `cls` (conversion_dominated) are dropped. Only a
	// function whose return type and kind another one of the set shares
	// can be (the hiding one, or what hides it in turn, is in the set).
	// Virtual bases are PA23's: this runs only for a hierarchy with one,
	// computing its direct bases' sets (a copy of the scratch is kept
	// meanwhile).
	PointerMap& kinds = conversion_hiding_[0];
	kinds.clear();
	bool repeated = false;
	for (const Binding* binding : conversion_set_scratch_)
	{
		const FunctionEntity* function = static_cast<const FunctionEntity*>(binding->entity);
		const uint32_t kind = function->templated ? 2u : 1u;
		const uint32_t found = kinds.find(function->type->canonical->element);
		const uint32_t bits = found == PointerMap::kMissing ? 0u : found;
		repeated = repeated || (bits & kind) != 0;
		kinds.set(function->type->canonical->element, bits | ((bits & kind) != 0 ? kind << 2 : kind));
	}
	if (!repeated)
		return;
	std::vector<Binding*> found(conversion_set_scratch_);
	std::vector<bool> collides(found.size());
	for (size_t i = 0; i < found.size(); ++i)
	{
		const FunctionEntity* function = static_cast<const FunctionEntity*>(found[i]->entity);
		collides[i] = (kinds.find(function->type->canonical->element) & ((function->templated ? 2u : 1u) << 2)) != 0;
	}
	size_t kept = 0;
	for (size_t i = 0; i < found.size(); ++i)
	{
		if (!collides[i] || !conversion_dominated(cls, found[i]))
			found[kept++] = found[i];
	}
	found.resize(kept);
	conversion_set_scratch_.swap(found);
}

bool Analyzer::conversion_dominated(const ClassEntity* cls, const Binding* binding)
{
	// A base's function visible in `cls` only through a virtual base (the
	// class declaring it: one shared subobject, 10.1p4) that a direct base
	// reaching that subobject hides is dominated (10.2p10). A
	// base-specifier's path through a virtual one counts as shared.
	const FunctionEntity* function = static_cast<const FunctionEntity*>(binding->entity);
	const Scope* where = binding->declared_in != nullptr ? binding->declared_in : function->owner;
	const ClassEntity* home = where != nullptr && where->kind == ScopeKind::Class
		? static_cast<const ClassEntity*>(where->entity)
		: nullptr;
	if (home == nullptr || home == cls || (base_path_facts(home, cls, 0) & kVirtualBasePath) == 0)
		return false;
	bool hidden = false;
	for (const BaseClass* base = cls->bases; base != nullptr; base = base->next)
	{
		const ClassEntity* next = ConversionBase(base);
		if (next == nullptr || (next != home && !conversions_.is_base_of(home, next)))
			continue;
		const uint32_t facts = next == home ? 1u | kNonVirtualBasePath : base_path_facts(home, next, 0);
		const bool shared = base->is_virtual || (facts & kVirtualBasePath) != 0;
		const bool only_shared = base->is_virtual || (facts & kNonVirtualBasePath) == 0;
		const ConversionSet set = conversion_set(next);
		bool visible = false;
		for (uint32_t k = 0; k < set.count && !visible; ++k)
			visible = set.bindings[k]->entity == function;
		// Visible through a subobject of its own: not dominated.
		if (visible && !only_shared)
			return false;
		hidden = hidden || (!visible && shared);
	}
	return hidden;
}

const Binding* Analyzer::conversion_binding(const ClassEntity* source, const FunctionEntity* function)
{
	// The declaration through which the conversion function (or the
	// template whose specialization it is) is in the source class's set:
	// its access applies (a using-declaration's own, 7.3.3p17), named in
	// the source class (11.2p5).
	const FunctionEntity* declared = function->primary != nullptr ? function->primary : function;
	if (source != nullptr && source->has_conversion_functions)
	{
		const ConversionSet set = conversion_set(source);
		for (uint32_t i = 0; i < set.count; ++i)
		{
			if (set.bindings[i]->entity == function || set.bindings[i]->entity == declared)
				return set.bindings[i];
		}
	}
	const NameEntry* entry = names_.find(function->owner, conversion_key_);
	for (const FunctionList* node = entry != nullptr ? entry->functions : nullptr; node != nullptr; node = node->next)
	{
		if (node->binding->entity == function || node->binding->entity == declared)
			return node->binding;
	}
	return nullptr;
}

bool Analyzer::conversion_subobject(const ClassEntity* source, const FunctionEntity* function, uint32_t location)
{
	// 10.2, 5.2.5p5, 4.10p3: the selected conversion function's class is
	// an unambiguous base of the operand's class (it may be a repeated
	// base, visible_conversions); false (a substitution failure) if not.
	const ClassEntity* home = ClassOf(function->owner->entity->type);
	if (source == nullptr || home == nullptr || home == source ||
		(base_path_facts(home, source, location) & kBasePathCount) < 2)
		return true;
	substitution_error(location, "a conversion function of an ambiguous base class subobject");
	return false;
}

FunctionList* Analyzer::conversion_lookup(const ClassEntity* cls, const Type* target, uint32_t location)
{
	// A conversion-function-id `operator T` looked up in the class (12.3.2,
	// 3.4.5p7, 3.4.3.1p1): the declarations of its conversion functions to
	// T visible there (12.3p5; a using-declaration's, 7.3.3p16), all of one
	// class (10.2: of several classes the lookup is ambiguous); failing a
	// non-template one (14.5.2p2), the specialization a visible conversion
	// function template deduces from T (14.5.2p5-p6, 14.8.2.3) whose type
	// it is, the best of several by partial ordering (14.5.2p8), named
	// through its template's declaration (its access, 14.5.2p2). Null (a
	// substitution's failure, reported) for none or an ambiguity.
	std::vector<Binding*> bindings;
	if (cls->has_conversion_functions)
	{
		const ConversionSet set = conversion_set(cls);
		bindings.assign(set.bindings, set.bindings + set.count);
	}
	FunctionList* head = nullptr;
	FunctionList** tail = &head;
	const Scope* declared = nullptr;
	bool ambiguous = false;
	for (Binding* binding : bindings)
	{
		const FunctionEntity* function = static_cast<const FunctionEntity*>(binding->entity);
		if (function->templated || function->type->canonical->element != target)
			continue;
		ambiguous = ambiguous || (declared != nullptr && binding->declared_in != declared);
		declared = binding->declared_in;
		FunctionList* node = static_cast<FunctionList*>(arena_.allocate(sizeof(FunctionList), alignof(FunctionList)));
		*node = FunctionList{ binding, nullptr };
		*tail = node;
		tail = &node->next;
	}
	// Each template's specialization; of two whose implicit object
	// parameters agree, a less specialized one is dropped (14.5.2p8,
	// 13.4p4); the call's overload resolution selects among the rest by
	// the object expression (13.3.1p4).
	std::vector<std::pair<FunctionEntity*, const Binding*>> found;
	for (size_t i = 0; head == nullptr && i < bindings.size(); ++i)
	{
		FunctionEntity* function = static_cast<FunctionEntity*>(bindings[i]->entity);
		FunctionEntity* specialization = function->templated ? deduce_for_conversion(function, target, true) : nullptr;
		if (specialization != nullptr && specialization->type->canonical->element == target)
			found.push_back(std::make_pair(specialization, bindings[i]));
	}
	for (size_t i = 0; i < found.size(); ++i)
	{
		const Type* own = found[i].first->type->canonical;
		bool dominated = false;
		for (size_t j = 0; j < found.size() && !dominated; ++j)
		{
			const Type* other = found[j].first->type->canonical;
			dominated = j != i &&
				(found[j].first == found[i].first ? j < i
												  : other->function_cv == own->function_cv && other->ref == own->ref &&
							conversion_template_order(found[j].first->primary, found[i].first->primary) > 0);
		}
		if (dominated)
			continue;
		Binding* binding = static_cast<Binding*>(arena_.allocate(sizeof(Binding), alignof(Binding)));
		*binding = *found[i].second;
		binding->hidden = true;
		binding->entity = found[i].first;
		binding->type = found[i].first->type;
		binding->next = nullptr;
		FunctionList* node = static_cast<FunctionList*>(arena_.allocate(sizeof(FunctionList), alignof(FunctionList)));
		*node = FunctionList{ binding, nullptr };
		*tail = node;
		tail = &node->next;
	}
	if (head == nullptr || ambiguous)
	{
		substitution_error(location,
			ambiguous ? "ambiguous lookup of a conversion-function-id" : "no conversion function to the named type");
		return nullptr;
	}
	return head;
}

bool Analyzer::user_conversion(const ExprFacts* from, const Type* target, Conversion& out, bool direct)
{
	// 13.3.1.4-13.3.1.6, 13.3.3.1.2: the best of the target class's
	// converting constructors callable with the operand alone (its
	// conversion to the first parameter a standard one, 13.3.3.1p4) and
	// the source class's conversion functions whose result converts to
	// the target by a standard conversion (explicit ones only in direct-
	// initialization, 12.3.2p2). Candidates rank by the operand's
	// conversion, then two conversion functions by their second standard
	// conversions (13.3.3p1); none or several best: false.
	if (from->type == nullptr || (from->flags & kOverloadSet) != 0 || NonstaticMemberName(from) || IsVoid(from->type))
		return false;
	const bool reference = target->is_reference();
	const Type* object = reference ? target->element : target;
	ClassEntity* entity = const_cast<ClassEntity*>(ClassOf(object));
	const ClassEntity* source = ClassOf(from->type);
	// Class template instances' declarations are known once defined; an
	// incomplete class has none yet (a substitution noting it, see
	// incomplete_classes_seen_).
	if (entity != nullptr && (entity->templ != nullptr || !entity->complete))
		require_complete(entity);
	if (source != nullptr && (source->templ != nullptr || !source->complete))
		require_complete(source);
	// Converting constructors are declared (or inherited, 12.9) ones, the
	// implicitly declared constructors taking the class itself (13.3.3.1p4);
	// without them and without a source class's conversion function there
	// is no candidate.
	const bool constructors = entity != nullptr &&
		(entity->user_declared_constructor || entity->inherits_constructors != nullptr);
	if (!constructors && (source == nullptr || !source->has_conversion_functions))
		return false;
	if (source != nullptr && entity != nullptr && (source == entity || conversions_.is_base_of(entity, source)))
		return false;
	const size_t base = user_candidates_.size();
	// 8.5.3p5: an lvalue reference binds directly to a conversion
	// function's lvalue result when one converts (13.3.1.6); only failing
	// that is a temporary made.
	if (reference && target->kind == TypeKind::LValueReference && source != nullptr &&
		direct_reference_conversion(from, target, source, direct, out))
		return true;
	if (constructors && require_complete(entity) && entity->scope != nullptr &&
		!(reference && target->kind == TypeKind::LValueReference && (object->cv & kConst) == 0))
	{
		const NameEntry* entry = names_.find(entity->scope, constructor_key_);
		for (const FunctionList* node = entry != nullptr ? entry->functions : nullptr; node != nullptr;
			node = node->next)
		{
			const FunctionEntity* constructor = static_cast<const FunctionEntity*>(node->binding->entity);
			// A constructor template's specialization the operand deduces
			// (14.8.2.1), unless the template cannot take one argument (two
			// parameters neither pack expansions, which may be empty, nor
			// with default arguments, 13.3.2p2): no deduction then, nor
			// substitution of its defaults.
			if (constructor->templated && !(constructor->is_explicit && !direct))
			{
				const Type* pattern = constructor->type->canonical;
				uint32_t required = 0;
				for (uint32_t i = 0; required < 2 && i < pattern->param_count; ++i)
					required += pattern->params[i]->kind != TypeKind::Expansion &&
						(constructor->default_arguments == nullptr || constructor->default_arguments[i] == nullptr);
				constructor = required < 2
					? inherited_specialization(
						  entity, deduce_for_operand(const_cast<FunctionEntity*>(constructor), from), 1, 0)
					: nullptr;
			}
			if (constructor == nullptr)
				continue;
			const Type* type = constructor->type->canonical;
			if ((constructor->is_explicit && !direct) || constructor->templated ||
				(type->param_count == 0 && !type->variadic))
				continue;
			// `C(...)` converts any operand, matched by its ellipsis
			// (13.3.3.1.3).
			if (type->param_count == 0)
			{
				UserCandidate candidate = UserCandidate{ constructor, Conversion(), Conversion() };
				candidate.first.form = ConversionForm::Ellipsis;
				candidate.first.rank = ConversionRank::Ellipsis;
				if (from->type != nullptr && !IsVoid(from->type))
					user_candidates_.push_back(candidate);
				continue;
			}
			bool callable = true;
			for (uint32_t i = 1; callable && i < type->param_count; ++i)
				callable = constructor->default_arguments != nullptr && constructor->default_arguments[i] != nullptr;
			UserCandidate candidate = UserCandidate{ constructor, Conversion(), Conversion() };
			if (callable && conversions_.implicit(operand_of(from), type->params[0], false, candidate.first))
				user_candidates_.push_back(candidate);
		}
	}
	if (source != nullptr && require_complete(source) && source->scope != nullptr)
	{
		const size_t functions = conversion_scratch_.size();
		conversion_functions(source, conversion_scratch_, true);
		// A template's specialization yields the target: its class (13.3.1.4),
		// its reference type (13.3.1.6) or its type (13.3.1.5).
		const Type* required = entity != nullptr && !reference ? object->unqualified : target;
		for (size_t k = functions; k < conversion_scratch_.size(); ++k)
		{
			const FunctionEntity* function = conversion_scratch_[k];
			if (function->is_explicit && !direct)
				continue;
			// A template's explicit(condition) is its specialization's.
			if (function->templated &&
				((function = deduce_for_conversion(conversion_scratch_[k], required)) == nullptr ||
					(function->is_explicit && !direct)))
				continue;
			UserCandidate candidate = UserCandidate{ function, Conversion(), Conversion() };
			if (!object_sequence(from, function, true, candidate.first))
				continue;
			// The call's result: a prvalue, or the glvalue a reference
			// result designates (5.2.2p10).
			ValueCategory category;
			const Type* result = result_category(function->type->canonical->element, category);
			const Operand produced = Operand{ result, category, 0 };
			if (entity != nullptr && !reference)
			{
				const ClassEntity* made = ClassOf(result);
				if (made == nullptr || (made != entity && !conversions_.is_base_of(entity, made)))
					continue;
			}
			if (!conversions_.implicit(produced, target, false, candidate.second))
				continue;
			// 13.3.1.5p1: an explicit one's result is the target type (a
			// qualification conversion apart).
			if (function->is_explicit && entity == nullptr && candidate.second.second != ValueConversion::None)
				continue;
			user_candidates_.push_back(candidate);
		}
		conversion_scratch_.resize(functions);
	}
	// The candidates are this call's part of the scratch, released on
	// every return.
	const UserCandidate* candidates = user_candidates_.data() + base;
	const size_t count = user_candidates_.size() - base;
	size_t best = 0;
	for (size_t i = 1; i < count; ++i)
	{
		if (user_candidate_order(candidates[i].function, candidates[i].first, candidates[i].second,
				candidates[best].function, candidates[best].first, candidates[best].second) > 0)
			best = i;
	}
	bool unique = count != 0;
	for (size_t i = 0; i < count && unique; ++i)
	{
		unique = i == best ||
			user_candidate_order(candidates[best].function, candidates[best].first, candidates[best].second,
				candidates[i].function, candidates[i].first, candidates[i].second) > 0;
	}
	if (!unique)
	{
		user_candidates_.resize(base);
		return false;
	}
	const UserCandidate chosen = candidates[best];
	user_candidates_.resize(base);
	Conversion first = chosen.first;
	first.initialization = true;
	out = Conversion();
	out.form = reference ? ConversionForm::Reference : ConversionForm::Standard;
	out.rank = ConversionRank::UserDefined;
	out.rvalue_reference = reference && target->kind == TypeKind::RValueReference;
	out.target = target;
	out.result = reference ? object : object->unqualified;
	out.constructor = chosen.function;
	out.argument = intern_conversion(first);
	if (chosen.function->constructor)
		out.temporary = reference;
	else
	{
		Conversion second = chosen.second;
		second.initialization = true;
		out.after = intern_conversion(second);
		out.temporary = reference && second.temporary;
	}
	return true;
}

void Analyzer::apply_user_conversion(const ExprFacts* from, Conversion& conversion, uint32_t location)
{
	// 12.3p2, clause 11: a converting constructor is accessible as a
	// constructor of its class, a conversion function as a member of the
	// operand's class (through an accessible base, 4.10p3). Neither is
	// deleted (8.4.3p2). A converting constructor's class parameter is
	// copy-initialized from the operand (8.5p16, 13.3.3.1p6), recorded on
	// the operand's conversion.
	const FunctionEntity* function = conversion.constructor;
	check_not_deleted(function, location);
	if (function->constructor)
	{
		// 10.4p2: the converting constructor's object is no abstract class's.
		check_abstract_object(function->owner->entity->type, location);
		check_special_access(function, ClassOf(function->owner->entity->type), expression_scope_, location);
		check_inheriting_access(function, ClassOf(function->owner->entity->type), location);
		if (conversion.form == ConversionForm::Standard && ClassOf(conversion.result) != nullptr)
			check_final_copy(const_cast<ClassEntity*>(ClassOf(conversion.result)), location);
		const Type* parameter = function->type->canonical->param_count != 0 ? function->type->canonical->params[0]
																			: nullptr;
		ClassEntity* cls = parameter != nullptr ? const_cast<ClassEntity*>(ClassOf(parameter)) : nullptr;
		// 5.2.2p4: its class parameter is destroyed by the caller (12.4p11).
		if (cls != nullptr && !parameter->is_reference())
			check_destruction(parameter, expression_scope_, location);
		if (cls != nullptr && !parameter->is_reference() && conversion.argument != nullptr &&
			conversion.argument->form == ConversionForm::Standard && conversion.argument->constructor == nullptr)
		{
			Conversion binding = Conversion();
			if (FunctionEntity* copy = operand_constructor(
					cls, from->type, from->category, ConstructionKind::Copy, location, binding))
			{
				Conversion argument = *conversion.argument;
				argument.constructor = copy;
				argument.argument = intern_conversion(binding);
				conversion.argument = intern_conversion(argument);
			}
		}
	}
	else
	{
		// Of a unique subobject of the operand's class, accessible through
		// the declaration that makes it visible there.
		const ClassEntity* source = ClassOf(from->type);
		if (!conversion_subobject(source, function, location))
			return;
		check_access(conversion_binding(source, function), source, source, expression_scope_, location);
		if (conversion.argument != nullptr)
			check_conversion_access(from, *conversion.argument, location);
		if (conversion.form == ConversionForm::Standard && ClassOf(conversion.target) != nullptr)
		{
			// 8.5p16 (the last sub-bullet), 13.3.1.4: the result then
			// direct-initializes the object, recorded as the second step.
			ValueCategory category;
			const Type* result = result_category(function->type->canonical->element, category);
			Conversion binding = Conversion();
			ClassEntity* cls = const_cast<ClassEntity*>(ClassOf(conversion.target));
			if (FunctionEntity* copy = operand_constructor(
					cls, result, category, ConstructionKind::Direct, location, binding))
			{
				Conversion second = *conversion.after;
				second.constructor = copy;
				second.argument = intern_conversion(binding);
				conversion.after = intern_conversion(second);
			}
		}
	}
	odr_use(const_cast<FunctionEntity*>(function), true);
	if (!function->nothrow)
		++potentially_throwing_;
}

FunctionEntity* Analyzer::operand_constructor(ClassEntity* entity, const Type* type, ValueCategory category,
	ConstructionKind kind, uint32_t location, Conversion& binding)
{
	// An object of the class initialized from a value of `type` (the class
	// or a derived one) and `category`: a prvalue of the class itself
	// initializes it in place (12.8p31, null); otherwise the constructor
	// overload resolution selects with it (13.3.1.3; standard conversions
	// only, 13.3.3.1p4), checked and odr-used, its binding in `binding`.
	if (category == ValueCategory::PRValue && type->unqualified == entity->type)
		return nullptr;
	syntax::Expr source = syntax::Expr();
	source.kind = syntax::ExprKind::Id;
	ExprFacts facts = ExprFacts();
	facts.type = type;
	facts.category = category;
	source.facts = &facts;
	syntax::Expr* pointer = &source;
	syntax::Slice<syntax::Expr*> arguments;
	arguments.data = &pointer;
	arguments.size = 1;
	std::vector<FunctionEntity*>& set = overload_set_;
	set.clear();
	constructor_candidates(entity, arguments, kind, set);
	const bool saved = standard_only_;
	standard_only_ = true;
	Candidate chosen = Candidate();
	const Resolution resolution = resolve(arguments, set, nullptr, chosen, entity);
	standard_only_ = saved;
	// A substitution's failure (14.8.2p8; a trait's operand, 20.9.4.3).
	if (resolution != Resolution::Selected)
	{
		substitution_error(location, "no unambiguous constructor initializes the class object from the value");
		return nullptr;
	}
	FunctionEntity* selected = chosen.function;
	if (!selected->type->canonical->params[0]->is_reference())
		error(location, "a class object initialized through a constructor's class parameter is not supported");
	check_not_deleted(selected, location);
	check_special_access(selected, entity, expression_scope_, location);
	odr_use(selected, true);
	if (!selected->nothrow)
		++potentially_throwing_;
	binding = sequences_[chosen.first];
	binding.initialization = true;
	return selected;
}

void Analyzer::check_final_copy(ClassEntity* entity, uint32_t location)
{
	// 8.5p16 (C++11): a converting constructor's temporary then direct-
	// initializes the object, by the constructor selected for an rvalue of
	// the class (13.3.1.3), which must be usable even when the copy is
	// elided (12.8p31-32): deleted or inaccessible, the initialization is
	// ill-formed (in a substitution, its failure, 14.8.2p8). The selection
	// is not odr-used here (the course's lowering elides it). An implicit
	// move constructor that is not deleted is that selection.
	implicit_copy_members(entity);
	if (entity->move_constructor != nullptr && !entity->move_constructor->deleted)
		return;
	syntax::Expr source = syntax::Expr();
	source.kind = syntax::ExprKind::Id;
	ExprFacts facts = ExprFacts();
	facts.type = entity->type;
	facts.category = ValueCategory::XValue;
	source.facts = &facts;
	syntax::Expr* pointer = &source;
	syntax::Slice<syntax::Expr*> arguments;
	arguments.data = &pointer;
	arguments.size = 1;
	std::vector<FunctionEntity*>& set = overload_set_;
	set.clear();
	constructor_candidates(entity, arguments, ConstructionKind::Direct, set);
	const bool saved = standard_only_;
	standard_only_ = true;
	Candidate chosen = Candidate();
	const Resolution resolution = resolve(arguments, set, nullptr, chosen, entity);
	standard_only_ = saved;
	if (resolution != Resolution::Selected)
	{
		substitution_error(location, "no unambiguous constructor initializes the object from the converted temporary");
		return;
	}
	check_not_deleted(chosen.function, location);
	check_special_access(chosen.function, entity, expression_scope_, location);
}

bool Analyzer::direct_reference_conversion(
	const ExprFacts* from, const Type* target, const ClassEntity* source, bool direct, Conversion& out)
{
	// The best conversion function whose result is an lvalue the reference
	// binds directly (reference-compatible).
	if (!require_complete(source) || source->scope == nullptr)
		return false;
	const size_t functions = conversion_scratch_.size();
	conversion_functions(source, conversion_scratch_, true);
	const FunctionEntity* best = nullptr;
	Conversion best_first = Conversion(), best_second = Conversion();
	bool ambiguous = false;
	for (size_t k = functions; k < conversion_scratch_.size(); ++k)
	{
		const FunctionEntity* function = conversion_scratch_[k];
		if (function->templated &&
			(function->is_explicit && !direct
					? nullptr
					: (function = deduce_for_conversion(conversion_scratch_[k], target))) == nullptr)
			continue;
		const Type* returned = function->type->canonical->element;
		if ((function->is_explicit && !direct) || returned->kind != TypeKind::LValueReference)
			continue;
		Conversion first = Conversion(), second = Conversion();
		if (!object_sequence(from, function, true, first) ||
			!conversions_.implicit(Operand{ returned->element, ValueCategory::LValue, 0 }, target, false, second) ||
			second.temporary)
			continue;
		const int order = best == nullptr
			? 1
			: user_candidate_order(function, first, second, best, best_first, best_second);
		if (order > 0)
		{
			best = function;
			best_first = first;
			best_second = second;
			ambiguous = false;
		}
		else if (order == 0)
			ambiguous = true;
	}
	conversion_scratch_.resize(functions);
	if (best == nullptr || ambiguous)
		return false;
	best_first.initialization = true;
	best_second.initialization = true;
	out = Conversion();
	out.form = ConversionForm::Reference;
	out.rank = ConversionRank::UserDefined;
	out.target = target;
	out.result = target->element;
	out.constructor = best;
	out.argument = intern_conversion(best_first);
	out.after = intern_conversion(best_second);
	return true;
}

int Analyzer::user_candidate_order(const FunctionEntity* a, const Conversion& a_first, const Conversion& a_second,
	const FunctionEntity* b, const Conversion& b_first, const Conversion& b_second)
{
	// 13.3.3p1 for a user-defined conversion's candidates: the operand's
	// conversions, then (both conversion functions) the second standard
	// conversions, then a non-template function before a template's
	// specialization, then two templates' specializations by partial
	// ordering (14.5.2p8, 14.5.6.2): conversion functions over their return
	// types, converting constructors over the parameter the operand
	// initializes (14.8.2.4p3).
	int order = conversions_.compare(a_first, b_first);
	if (order == 0 && !a->constructor && !b->constructor)
		order = conversions_.compare(a_second, b_second);
	if (order == 0 && (a->primary != nullptr) != (b->primary != nullptr))
		order = a->primary == nullptr ? 1 : -1;
	if (order == 0 && a->primary != nullptr && b->primary != nullptr && a->primary != b->primary &&
		a->constructor == b->constructor)
	{
		if (!a->constructor)
			return conversion_template_order(a->primary, b->primary);
		return function_template_order(a->primary, b->primary, 1);
	}
	return order;
}

ExprFacts* Analyzer::convert_class_operand(const syntax::Expr& expr, const FunctionEntity* function)
{
	// The class operand now stands for the conversion function's call on
	// it: its facts are the call's, its own kept (13.3.1.2p7).
	const syntax::Expr* inner = WithoutParentheses(&expr);
	ExprFacts* original = facts_of(*inner);
	ValueCategory category;
	const Type* type = result_category(function->type->canonical->element, category);
	ExprFacts* facts = new_facts(*inner, type, category);
	facts->flags = kUserConverted | kDirectCall;
	facts->entity = function;
	facts->original = original;
	for (const syntax::Expr* node = &expr; node != inner; node = static_cast<const syntax::ParenExpr*>(node)->inner)
		node->facts = facts;
	// Of a unique subobject, accessible as apply_user_conversion checks.
	const ClassEntity* source = ClassOf(original->type);
	if (conversion_subobject(source, function, expr.location))
		check_access(conversion_binding(source, function), source, source, expression_scope_, expr.location);
	odr_use(const_cast<FunctionEntity*>(function), true);
	if (!function->nothrow)
		++potentially_throwing_;
	return facts;
}

bool Analyzer::folding_condition(const syntax::Expr& expr) const
{
	if (!hosted_)
		return false;
	const syntax::Expr* condition = WithoutParentheses(&expr);
	// Parentheses and explicit scalar conversions preserve the builtin
	// condition's identity, unlike a variable initialized from its result.
	for (;;)
	{
		if (condition->kind == syntax::ExprKind::Cast)
			condition = WithoutParentheses(static_cast<const syntax::CastExpr*>(condition)->operand);
		else if (condition->kind == syntax::ExprKind::FunctionalCast &&
			static_cast<const syntax::FunctionalCastExpr*>(condition)->arguments.size == 1)
			condition = WithoutParentheses(static_cast<const syntax::FunctionalCastExpr*>(condition)->arguments[0]);
		else
			break;
	}
	const Entity* entity = condition->facts != nullptr ? condition->facts->entity : nullptr;
	return condition->kind == syntax::ExprKind::Call && entity != nullptr && entity->kind == EntityKind::Function &&
		static_cast<const FunctionEntity*>(entity)->gnu_builtin == GnuBuiltin::ConstantP;
}

ExprFacts* Analyzer::conditional(const syntax::ConditionalExpr& expr, Scope* scope)
{
	// 5.16 for the scalar slice.
	if (typed_value(*expr.condition, scope) == nullptr)
		return nullptr;
	convert_to_bool(*expr.condition);
	ExprFacts* second = typed_value(*expr.then_expr, scope);
	ExprFacts* third = second != nullptr ? typed_value(*expr.else_expr, scope) : nullptr;
	if (third == nullptr)
		return nullptr;
	const Type* t2 = second->type;
	const Type* t3 = third->type;
	if (IsVoid(t2) || IsVoid(t3))
	{
		// p2: a throw-expression (parenthesized too, as the host compilers
		// take it) and another operand: CWG 1550 (a C++11 defect report,
		// as the host compilers and the course apply it): the result has
		// the other operand's type and value category (a glvalue, a
		// bit-field included, designates the other operand); else a prvalue
		// of the other's type after the lvalue-to-rvalue, array- and
		// function-to-pointer conversions.
		const bool throws2 = WithoutParentheses(expr.then_expr)->kind == syntax::ExprKind::Throw;
		const bool throws3 = WithoutParentheses(expr.else_expr)->kind == syntax::ExprKind::Throw;
		if (throws2 != throws3 && !IsVoid(throws2 ? t3 : t2))
		{
			const syntax::Expr& other = throws2 ? *expr.else_expr : *expr.then_expr;
			ExprFacts* facts = throws2 ? third : second;
			if (facts->category != ValueCategory::PRValue && facts->type->kind != TypeKind::Array &&
				facts->type->kind != TypeKind::Function)
			{
				ExprFacts* result = new_facts(expr, facts->type, facts->category);
				result->flags = facts->flags & kBitField;
				return result;
			}
			const Type* type = decayed(facts);
			if (type->unqualified->kind == TypeKind::Class)
			{
				type = type->unqualified;
				expression_scope_ = scope;
				record_copy(static_cast<ClassEntity*>(type->entity), type, other, other.location, true);
			}
			else
				convert_operand(other, type);
			return new_facts(expr, type, ValueCategory::PRValue);
		}
		if (!IsVoid(t2) || !IsVoid(t3))
			error(expr.location, "only one operand of ?: is void");
		return new_facts(expr, types_.fundamental(FundamentalType::Void), ValueCategory::PRValue);
	}
	// p4: glvalues of one category and type; p3 with cv-qualified
	// variants: the less qualified converts to the more qualified.
	if (second->category == third->category && second->category != ValueCategory::PRValue &&
		t2->unqualified == t3->unqualified && (t2->cv & t3->cv) == (t2->cv < t3->cv ? t2->cv : t3->cv))
	{
		const Type* type = (t2->cv & ~t3->cv) == 0 ? t3 : t2;
		ExprFacts* facts = new_facts(expr, type, second->category);
		facts->flags = (second->flags | third->flags) & kBitField;
		return facts;
	}
	if (t2->unqualified->kind == TypeKind::Class || t3->unqualified->kind == TypeKind::Class)
	{
		if (t2->unqualified != t3->unqualified)
			return converted_conditional(expr, scope);
		// p3, p6: a prvalue of the class (the more qualified operand's
		// type), copy-initialized from the selected operand: a glvalue by
		// the constructor it selects, a prvalue in place.
		const Type* type = (t2->cv & ~t3->cv) == 0 ? t3 : t2;
		ClassEntity* cls = static_cast<ClassEntity*>(type->unqualified->entity);
		expression_scope_ = scope;
		record_copy(cls, type, *expr.then_expr, expr.then_expr->location, true);
		record_copy(cls, type, *expr.else_expr, expr.else_expr->location, true);
		return new_facts(expr, type, ValueCategory::PRValue);
	}
	// p6: prvalues after the standard conversions.
	const Type* a = decayed(second);
	const Type* b = decayed(third);
	const Type* type = nullptr;
	if (a == b)
		type = a;
	else if ((IsArithmetic(a) || IsIntegralOrUnscopedEnum(a) || IsComplex(a)) &&
		(IsArithmetic(b) || IsIntegralOrUnscopedEnum(b) || IsComplex(b)))
		type = usual_arithmetic(field_promoted(second, a), field_promoted(third, b));
	else
		type = composite_pointer(second, third);
	if (type == nullptr)
	{
		substitution_error(expr.location, "incompatible operands of ?:");
		return nullptr;
	}
	convert_operand(*expr.then_expr, type);
	convert_operand(*expr.else_expr, type);
	return new_facts(expr, type, ValueCategory::PRValue);
}

ExprFacts* Analyzer::converted_conditional(const syntax::ConditionalExpr& expr, Scope* scope)
{
	// 5.16p3: operands of different types, one of class type: exactly one
	// may be converted to match the other (conditional_match), and stands in
	// its place. p4: then glvalues of one type and category give that
	// glvalue; p6: otherwise a prvalue of the common type, a class one
	// copy-initialized from the selected operand.
	expression_scope_ = scope;
	ExprFacts* second = facts_of(*expr.then_expr);
	ExprFacts* third = facts_of(*expr.else_expr);
	Conversion to_third = Conversion(), to_second = Conversion();
	const uint8_t third_match = conditional_match(*expr.then_expr, third, to_third);
	const uint8_t second_match = conditional_match(*expr.else_expr, second, to_second);
	if (third_match == 0 && second_match == 0)
		return builtin_conditional(expr);
	if (third_match != 0 && second_match != 0)
	{
		substitution_error(expr.location, "each operand of ?: converts to the other's type");
		return nullptr;
	}
	const bool first = third_match != 0;
	const syntax::Expr& converted = first ? *expr.then_expr : *expr.else_expr;
	const syntax::Expr& other = first ? *expr.else_expr : *expr.then_expr;
	const ExprFacts* target = first ? third : second;
	Conversion conversion = first ? to_third : to_second;
	const uint8_t match = first ? third_match : second_match;
	const Type* type = target->type;
	if (match <= 2)
	{
		// A glvalue of the other's type and category, bound directly (a base
		// subobject, or a conversion function's reference result).
		if (match == 2)
			convert_class_operand(converted, conversion.constructor);
		// An unambiguous accessible base (8.5.3p5, 10.2p12, 11.2p4).
		const ExprFacts* now = facts_of(converted);
		expression_scope_ = scope;
		if (ClassOf(now->type) != ClassOf(type) && !upcast_base(ClassOf(now->type), ClassOf(type), true, expr.location))
			return nullptr;
		ExprFacts* facts = new_facts(expr, type, target->category);
		facts->flags = (now->flags | target->flags) & kBitField;
		return facts;
	}
	ClassEntity* cls = const_cast<ClassEntity*>(ClassOf(type));
	if (match == 3)
	{
		// A prvalue of the base (or same) class copy-initialized from it.
		record_copy(cls, type, *expr.then_expr, expr.then_expr->location, true);
		record_copy(cls, type, *expr.else_expr, expr.else_expr->location, true);
		return new_facts(expr, type, ValueCategory::PRValue);
	}
	// The type the other operand has as a prvalue (4.1: a class keeps its
	// cv-qualifiers): the converted operand's by its implicit conversion.
	ExprFacts* from = facts_of(*WithoutParentheses(&converted));
	if (cls == nullptr)
		type = decayed(const_cast<ExprFacts*>(target));
	conversion.initialization = cls != nullptr;
	if (conversion.constructor != nullptr)
		apply_user_conversion(from, conversion, converted.location);
	check_conversion_access(from, conversion, converted.location);
	record_conversion(from, conversion);
	if (cls != nullptr)
		record_copy(cls, type, other, other.location, true);
	else
		convert_operand(other, type);
	return new_facts(expr, type, ValueCategory::PRValue);
}

uint8_t Analyzer::conditional_match(const syntax::Expr& operand, const ExprFacts* other, Conversion& out)
{
	// 5.16p3: how the operand E1 can be converted to match E2 (`other`):
	// for an lvalue (xvalue) E2, to a reference to E2's type binding
	// directly to such a glvalue: 1 by a standard binding, 2 through a
	// conversion function's reference result (in `out`); failing that, or
	// for a prvalue E2, a class E1 whose class is E2's or derived from it
	// converts to a prvalue of E2's type at least as cv-qualified (3; no
	// other way between related classes), else E1 implicitly converts to
	// the type E2 has as a prvalue (4, in `out`); 0 when it cannot.
	const ExprFacts* from = operand.facts;
	const Type* t1 = from->type;
	const Type* t2 = other->type;
	if (other->category != ValueCategory::PRValue)
	{
		const bool lvalue = other->category == ValueCategory::LValue;
		const Type* reference = types_.reference(t2, !lvalue);
		Conversion binding = Conversion();
		if (from->category == other->category && conversions_.implicit(operand_of(from), reference, false, binding) &&
			binding.form == ConversionForm::Reference && !binding.temporary)
			return 1;
		if (ClassOf(t1) != nullptr && user_conversion(from, reference, binding, false) &&
			!binding.constructor->constructor &&
			binding.constructor->type->canonical->element->kind ==
				(lvalue ? TypeKind::LValueReference : TypeKind::RValueReference))
		{
			out = binding;
			return 2;
		}
	}
	const ClassEntity* c1 = ClassOf(t1);
	const ClassEntity* c2 = ClassOf(t2);
	if (c1 != nullptr && c2 != nullptr &&
		(c1 == c2 || conversions_.is_base_of(c1, c2) || conversions_.is_base_of(c2, c1)))
		return (c1 == c2 || conversions_.is_base_of(c2, c1)) && (t2->cv & t1->cv) == t1->cv ? 3 : 0;
	const Type* target = c2 != nullptr   ? t2
		: t2->kind == TypeKind::Array    ? types_.pointer(t2->element)
		: t2->kind == TypeKind::Function ? types_.pointer(t2)
										 : t2->unqualified;
	if (IsVoid(t1))
		return 0;
	if (conversions_.implicit(operand_of(from), target, false, out))
		return 4;
	return user_conversion(from, target, out, false) ? 4 : 0;
}

ExprFacts* Analyzer::builtin_conditional(const syntax::ConditionalExpr& expr)
{
	// 5.16p5: second and third operands of different types, one of class
	// type, neither converting to the other (p3): overload resolution among
	// the built-in candidates LR operator?:(bool, L, R) for promoted
	// arithmetic L and R (13.6p25) and T operator?:(bool, T, T) for a
	// pointer, pointer to member or scoped enumeration T (p26) determines
	// their conversions, a class operand's through a conversion function
	// (13.3.1.2p3, p7). Of each pair of the operands' possible types (a
	// class operand's non-explicit conversion functions' non-class
	// results) the candidate its conversions rank best is tried: L and R
	// their promotions, T their common or composite pointer type. The
	// chosen candidate's conversions are applied and give the prvalue p6
	// describes (LR the usual arithmetic conversions' type).
	const syntax::Expr* operands[2] = { expr.then_expr, expr.else_expr };
	const size_t base = operand_options_.size();
	size_t begin[2] = { base, base };
	size_t size[2] = { 0, 0 };
	for (uint32_t i = 0; i < 2; ++i)
	{
		begin[i] = operand_options_.size();
		ExprFacts* facts = facts_of(*operands[i]);
		const ClassEntity* cls = ClassOf(facts->type);
		if (cls == nullptr)
			operand_options_.push_back(facts->type->kind == TypeKind::Array ? types_.pointer(facts->type->element)
					: facts->type->kind == TypeKind::Function               ? types_.pointer(facts->type)
																			: facts->type->unqualified);
		const size_t functions = conversion_scratch_.size();
		if (cls != nullptr && require_complete(cls))
			conversion_functions(cls, conversion_scratch_);
		for (size_t k = functions; k < conversion_scratch_.size(); ++k)
		{
			const Type* result = conversion_scratch_[k]->type->canonical->element;
			if (result->is_reference())
				result = result->element;
			result = result->kind == TypeKind::Array ? types_.pointer(result->element)
				: result->kind == TypeKind::Function ? types_.pointer(result)
													 : result->unqualified;
			bool seen = conversion_scratch_[k]->is_explicit || ClassOf(result) != nullptr;
			for (size_t j = begin[i]; j < operand_options_.size() && !seen; ++j)
				seen = operand_options_[j] == result;
			if (!seen)
				operand_options_.push_back(result);
		}
		conversion_scratch_.resize(functions);
		size[i] = operand_options_.size() - begin[i];
	}
	// The candidates' parameter types and their operands' conversions.
	std::vector<const Type*> parameters;
	std::vector<Conversion> sequences;
	for (size_t a = 0; a < size[0]; ++a)
	{
		for (size_t b = 0; b < size[1]; ++b)
		{
			const Type* l = operand_options_[begin[0] + a];
			const Type* r = operand_options_[begin[1] + b];
			const bool arithmetic = (IsArithmetic(l) || IsIntegralOrUnscopedEnum(l)) &&
				(IsArithmetic(r) || IsIntegralOrUnscopedEnum(r));
			if (arithmetic)
			{
				l = promoted(l);
				r = promoted(r);
			}
			else
			{
				ExprFacts views[2] = { ExprFacts(), ExprFacts() };
				views[0].type = l;
				views[1].type = r;
				for (uint32_t i = 0; i < 2; ++i)
				{
					if (ClassOf(operands[i]->facts->type) == nullptr)
						views[i].flags = operands[i]->facts->flags & kNullLiteral;
				}
				l = r = l == r && IsScopedEnumeration(l) ? l : composite_pointer(&views[0], &views[1]);
				if (l == nullptr)
					continue;
			}
			bool repeated = false;
			for (size_t k = 0; k < parameters.size() && !repeated; k += 2)
				repeated = parameters[k] == l && parameters[k + 1] == r;
			if (repeated)
				continue;
			Conversion first = Conversion(), second = Conversion();
			if (!implicit_sequence(*operands[0], l, first) || !implicit_sequence(*operands[1], r, second))
				continue;
			parameters.push_back(l);
			parameters.push_back(r);
			sequences.push_back(first);
			sequences.push_back(second);
		}
	}
	operand_options_.resize(base);
	// The candidate better than every other (13.3.3p1: no conversion worse
	// and one better).
	const auto order = [&](size_t x, size_t y) {
		const int left = conversions_.compare(sequences[x], sequences[y]);
		const int right = conversions_.compare(sequences[x + 1], sequences[y + 1]);
		return (left >= 0 && right >= 0 && (left > 0 || right > 0)) ? 1
			: (left <= 0 && right <= 0 && (left < 0 || right < 0))  ? -1
																	: 0;
	};
	size_t best = 0;
	for (size_t k = 2; k < sequences.size(); k += 2)
	{
		if (order(k, best) > 0)
			best = k;
	}
	bool unique = !sequences.empty();
	for (size_t k = 0; k < sequences.size() && unique; k += 2)
		unique = k == best || order(best, k) > 0;
	// No unique best candidate: ill-formed, or a substitution's failure
	// (14.8.2p8).
	if (!unique)
	{
		substitution_error(expr.location, "incompatible operands of ?:");
		return nullptr;
	}
	const Type* type = parameters[best] == parameters[best + 1]
		? parameters[best]
		: usual_arithmetic(parameters[best], parameters[best + 1]);
	for (uint32_t i = 0; i < 2; ++i)
	{
		const Conversion& conversion = sequences[best + i];
		if (conversion.rank == ConversionRank::UserDefined && conversion.constructor != nullptr &&
			!conversion.constructor->constructor)
		{
			check_not_deleted(conversion.constructor, operands[i]->location);
			convert_class_operand(*operands[i], conversion.constructor);
		}
		convert_operand(*operands[i], type);
	}
	return new_facts(expr, type, ValueCategory::PRValue);
}

bool Analyzer::convert_to_scalar(const syntax::Expr& expr, const Type* target, bool direct)
{
	// A class operand where a scalar is needed (a condition, a switch's
	// integral value, a delete's pointer): the conversion function the
	// target's user-defined conversion selects, applied; false if none.
	const ExprFacts* facts = WithoutParentheses(&expr)->facts;
	if (facts->type == nullptr || facts->type->unqualified->kind != TypeKind::Class)
		return false;
	Conversion conversion = Conversion();
	if (!user_conversion(facts, target, conversion, direct) || conversion.constructor->constructor)
		return false;
	check_not_deleted(conversion.constructor, expr.location);
	convert_class_operand(expr, conversion.constructor);
	return true;
}

const FunctionEntity* Analyzer::unique_conversion(const ClassEntity* entity, bool (*accepts)(const Type*))
{
	// The one non-explicit conversion function whose result type (without
	// its reference) the predicate accepts (5.3.5p1, 6.4.2p2), else null.
	const size_t functions = conversion_scratch_.size();
	conversion_functions(entity, conversion_scratch_);
	const FunctionEntity* found = nullptr;
	bool several = false;
	for (size_t k = functions; k < conversion_scratch_.size() && !several; ++k)
	{
		const FunctionEntity* function = conversion_scratch_[k];
		const Type* result = function->type->canonical->element;
		if (result->is_reference())
			result = result->element;
		if (function->is_explicit || !accepts(result->unqualified))
			continue;
		several = found != nullptr;
		found = function;
	}
	conversion_scratch_.resize(functions);
	return several ? nullptr : found;
}

} // namespace sema
