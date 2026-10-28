#include "sema/analyzer.h"

#include <cmath>
#include <cstdio>
#include <cstring>

#include "lex/identifier_table.h"
#include "sema/aggregates.h"
#include "sema/constant_interpreter.h"
#include "sema/instantiation_context.h"
#include "sema/string_literals.h"

// Initialization of the procedural slice (8.5, semantics mode): objects
// copy-, direct- and list-initialized from their initializers, class
// objects through the constructor their default- or copy-initialization
// selects, and default arguments checked like the initializers they are.
namespace sema {

namespace {

// An anonymous struct's or union's member object (9.5p5; the unnamed
// member of its enclosing class).
const ClassEntity* AnonymousMemberClass(const DataMember* member)
{
	const ClassEntity* cls = member->variable->name == nullptr && !member->bit_field ? ClassOf(member->variable->type)
																					 : nullptr;
	return cls != nullptr && cls->anonymous ? cls : nullptr;
}

// The anonymous class has a member of the name, its own or a nested
// anonymous member's (whose names are the enclosing class's, 9.5p1).
bool AnonymousHolds(const ClassEntity* cls, const Identifier* name)
{
	for (const DataMember* member = cls->members; member != nullptr; member = member->next)
	{
		const ClassEntity* nested = AnonymousMemberClass(member);
		if (member->variable->name == name || (nested != nullptr && AnonymousHolds(nested, name)))
			return true;
	}
	return false;
}

// 8.5.4p7: an integral constant's value is exactly a value of the floating
// type: its significant bits fit the significand (24, 53 and 64 bits for
// the SysV float, double and long double).
bool ExactlyFloating(const Constant& value, const Type* floating)
{
	uint64_t magnitude = value.bits;
	if (IsSignedConstant(value) && static_cast<int64_t>(value.bits) < 0)
		magnitude = 0 - magnitude;
	while (magnitude != 0 && (magnitude & 1) == 0)
		magnitude >>= 1;
	unsigned width = 0;
	for (; magnitude != 0; magnitude >>= 1)
		++width;
	const FundamentalType kind = FloatingRepresentation(floating->fundamental);
	return width <= (kind == FundamentalType::Float16             ? 11u
							: kind == FundamentalType::BFloat16   ? 8u
							: kind == FundamentalType::Float      ? 24u
							: kind == FundamentalType::Double     ? 53u
							: kind == FundamentalType::LongDouble ? 64u
																  : 113u);
}

// 8.5.2p1: a string literal of element type `literal` initializes an array
// of `element`: a narrow one any narrow character array, the others their
// own kind.
bool InitializesCharacterArray(const Type* literal, const Type* element)
{
	literal = literal->unqualified;
	return IsCharacterArrayElement(element) &&
		(literal == element->unqualified ||
			(literal->fundamental == FundamentalType::Char && element->unqualified->kind == TypeKind::Fundamental &&
				(element->unqualified->fundamental == FundamentalType::SignedChar ||
					element->unqualified->fundamental == FundamentalType::UnsignedChar)));
}

} // namespace

FunctionEntity* Analyzer::implicit_constructor(ClassEntity* entity)
{
	// The default constructor default-initialization selects (8.5p6,
	// 12.1p5): the constructor overload resolution chooses for an empty
	// argument list (13.3.1.3), else the implicitly declared one, created
	// once per class. Without arguments no candidate's conversions are
	// better (13.3.3p1): a non-template callable without arguments wins
	// over the templates, two of them are ambiguous (a substitution's
	// failure, 14.8.2p8, where that is a result).
	FunctionEntity*& slot = entity->default_constructor;
	if (slot != nullptr)
		return slot;
	FunctionEntity* found = nullptr;
	if (entity->scope != nullptr)
	{
		const NameEntry* entry = names_.find(entity->scope, constructor_key_);
		for (const FunctionList* node = entry != nullptr ? entry->functions : nullptr; node != nullptr;
			node = node->next)
		{
			FunctionEntity* function = static_cast<FunctionEntity*>(node->binding->entity);
			const Type* type = function->type->canonical;
			bool matches = true;
			for (uint32_t i = 0; matches && i < type->param_count; ++i)
				matches = function->default_arguments != nullptr && function->default_arguments[i] != nullptr;
			if (!matches || function->templated)
				continue;
			if (found != nullptr)
			{
				substitution_error(0, "ambiguous default constructor");
				return nullptr;
			}
			found = function;
		}
		// Else a constructor template callable without arguments (its
		// function parameter pack empty, 14.8.2.1p1): its specialization
		// from its default template arguments (14.8.2p5), no candidate
		// where their substitution fails (14.8.2p8). Two templates are not
		// ordered by a call without arguments (14.8.2.4p3).
		for (const FunctionList* node = entry != nullptr && found == nullptr ? entry->functions : nullptr;
			node != nullptr; node = node->next)
		{
			FunctionEntity* function = static_cast<FunctionEntity*>(node->binding->entity);
			const Type* type = function->type->canonical;
			bool matches = function->templated && HeadOf(function) != nullptr;
			for (uint32_t i = 0; matches && i < type->param_count; ++i)
				matches = (function->default_arguments != nullptr && function->default_arguments[i] != nullptr) ||
					(i + 1 == type->param_count && type->params[i]->kind == TypeKind::Expansion);
			TypeList arguments(matches ? HeadOf(function)->count : 0, nullptr);
			FunctionEntity* specialization = matches ? deduced_specialization(function, arguments) : nullptr;
			if (specialization == nullptr)
				continue;
			if (found != nullptr)
			{
				substitution_error(0, "ambiguous default constructor");
				return nullptr;
			}
			found = specialization;
		}
	}
	if (found != nullptr)
	{
		slot = found;
		// A default constructor defaulted on its first declaration is
		// defined as the implicit one would be (8.4.2p5).
		if (found->defaulted && found->inits == nullptr && semantic())
			implicit_default_facts(entity, found);
		return found;
	}
	if (entity->user_declared_constructor)
		return nullptr;
	const char* label = entity->name != nullptr ? entity->name->spelling
		: entity->local_label != nullptr        ? entity->local_label
												: entity->label;
	const Type* type = types_.function(
		types_.fundamental(FundamentalType::Void), nullptr, 0, false, 0, RefQualifier::None);
	FunctionEntity* function = new_entity<FunctionEntity>(EntityKind::Function, constructor_key_, entity->scope, type);
	function->label = label;
	function->signature = type;
	function->member = true;
	function->constructor = true;
	function->implicit = true;
	function->defined = true;
	function->is_inline = true;
	function->inline_member = true;
	// A closure type's is deleted (5.1.2p19).
	function->deleted = graph_.closure_call(entity) != nullptr;
	slot = function;
	if (semantic() && !function->deleted)
		implicit_default_facts(entity, function);
	return function;
}

FunctionEntity* Analyzer::queried_default_constructor(ClassEntity* entity)
{
	// The default constructor a trait or a literal-type test asks about
	// (12.1p5): an implicit (or defaulted) one whose definition would be
	// ill-formed (a subobject without a default constructor, or with a
	// deleted, inaccessible or ambiguous one) is defined as deleted, which
	// is the query's answer, not an error: its failure is the query's, as
	// a substitution's. Null when there is none. A use of it reports it
	// (check_not_deleted).
	if (entity->default_constructor != nullptr)
		return entity->default_constructor;
	SoftSubstitution soft(*this);
	return implicit_constructor(entity);
}

bool Analyzer::nontrivial_destruction(const Type* type)
{
	// A class object (or array of them) whose destructor is not trivial
	// (12.4p5), as the class's completion recorded it.
	if (type->is_reference())
		return false;
	while (type->kind == TypeKind::Array)
		type = type->element;
	const ClassEntity* entity = ClassOf(type);
	return entity != nullptr && entity->destructor != nullptr && !entity->destructor->trivial;
}

void Analyzer::default_initialize(VariableEntity* variable, const Type* type, uint32_t location)
{
	// 8.5p6: a class object's default constructor runs (for an array, each
	// element's), accessible at the declaration (12p4); a const object of
	// other type needs an initializer.
	const Type* object = type;
	while (object->kind == TypeKind::Array)
		object = object->element;
	if (object->unqualified->kind == TypeKind::Class)
	{
		ClassEntity* entity = static_cast<ClassEntity*>(object->unqualified->entity);
		const Scope* scope = expression_scope_;
		FunctionEntity* constructor = implicit_constructor(entity);
		if (constructor == nullptr)
			error(location, "no default constructor for the class");
		check_not_deleted(constructor, location);
		check_special_access(constructor, entity, scope, location);
		if ((object->cv & kConst) != 0)
			check_const_default(entity, constructor, location);
		variable->constructor = constructor;
		// The course view shows an array's element construction as one
		// action without demanding the constructor's definition.
		if (type->kind != TypeKind::Array)
			odr_use(constructor, true);
		return;
	}
	if ((object->cv & kConst) != 0 && !variable->is_extern)
		error(location, "const object without an initializer");
	if (type->is_reference())
		error(location, "reference without an initializer");
}

void Analyzer::check_const_default(ClassEntity* entity, const FunctionEntity* constructor, uint32_t location)
{
	// 8.5p6 as CWG 253 resolved it (GCC and Clang apply it to C++11): a
	// const object is default-initialized by a user-provided constructor of
	// its class, or the class leaves nothing uninitialized: each direct
	// member has a brace-or-equal-initializer or a class type of this kind,
	// a union (and each anonymous union member) initializes exactly one
	// member, and each base has a class type of this kind. Established once
	// per class.
	if (entity->const_default_constructible ||
		(constructor != nullptr && !constructor->implicit && !constructor->defaulted && !constructor->deleted))
		return;
	check_stack(location);
	for (const BaseClass* base = entity->bases; base != nullptr; base = base->next)
	{
		ClassEntity* cls = static_cast<ClassEntity*>(base->base);
		check_const_default(cls, implicit_constructor(cls), location);
	}
	const bool is_union = entity->key == TokenKind::KW_UNION;
	uint32_t members = 0, initialized = 0;
	for (const DataMember* member = entity->members; member != nullptr; member = member->next)
	{
		if (member->variable->kind != EntityKind::Variable || (member->bit_field && member->variable->name == nullptr))
			continue;
		const VariableEntity* variable = static_cast<const VariableEntity*>(member->variable);
		++members;
		if (variable->member_initializer != nullptr)
		{
			++initialized;
			continue;
		}
		if (is_union)
			continue;
		const Type* object = variable->type;
		while (object->kind == TypeKind::Array)
			object = object->element;
		if (object->unqualified->kind != TypeKind::Class)
			error(location, "a const object leaves a member uninitialized");
		ClassEntity* cls = static_cast<ClassEntity*>(object->unqualified->entity);
		check_const_default(cls, cls->anonymous ? nullptr : implicit_constructor(cls), location);
	}
	if (is_union && members != 0 && initialized != 1)
		error(location, "a const union object initializes no member");
	entity->const_default_constructible = true;
}

void Analyzer::initialize_variable(
	VariableEntity* variable, const Type* type, const syntax::Initializer* initializer, Scope* scope, uint32_t location)
{
	// Namespace-scope initializers are part of the unit's always-emitted
	// code; a body's belong to its demand list already.
	BodyContext* saved = body_;
	const syntax::Initializer* saved_initializing = initializing_;
	BodyContext context = BodyContext();
	if (body_ == nullptr)
	{
		body_ = &context;
		initializing_ = initializer;
		// A class template instance's static data member is its closures'
		// context (Itanium 5.1.8; see number_closure).
		if (variable->owner->kind == ScopeKind::Class && variable->templ != nullptr &&
			variable->templ->pattern != nullptr)
			context.closure_context = variable;
	}
	// GNU's flexible array member is given elements only by the list of an
	// object of static storage duration's own initializer (GCC and Clang
	// alike: the object's data can grow by them).
	const syntax::BracedInitList* saved_flexible = flexible_list_;
	const ClassEntity* saved_class = flexible_class_;
	const bool saved_given = flexible_given_;
	flexible_list_ = nullptr;
	flexible_class_ = nullptr;
	flexible_given_ = false;
	if (hosted_ && type->kind != TypeKind::Array && ClassOf(type) != nullptr &&
		(variable->owner->kind == ScopeKind::Namespace || variable->is_static || variable->is_thread_local) &&
		(initializer->kind == syntax::InitializerKind::Braced ||
			(initializer->kind == syntax::InitializerKind::Assign &&
				initializer->expression->kind == syntax::ExprKind::BracedInit)))
	{
		flexible_list_ = static_cast<const syntax::BracedInitList*>(initializer->expression);
		flexible_class_ = ClassOf(type);
	}
	variable->constructor = initialize_object(type, *initializer, scope, location);
	flexible_list_ = saved_flexible;
	flexible_class_ = saved_class;
	flexible_given_ = saved_given;
	body_ = saved;
	initializing_ = saved_initializing;
}

void Analyzer::check_character_array(
	const Type* type, const syntax::Expr& single, const ExprFacts* facts, uint32_t location)
{
	// 8.5.2: a character array from a string literal of its kind.
	const syntax::Expr* inner = &single;
	while (inner->kind == syntax::ExprKind::Paren)
		inner = static_cast<const syntax::ParenExpr*>(inner)->inner;
	if (inner->kind != syntax::ExprKind::Literal || facts->type->kind != TypeKind::Array ||
		!InitializesCharacterArray(facts->type->element, type->element))
		error(location, "array initialized from an expression");
	if (type->bound != 0 && facts->type->bound > type->bound)
		error(location, "string literal too long for the array");
}

void Analyzer::list_initialize(const syntax::BracedInitList& list, const Type* type, Scope* scope)
{
	// 8.5.4 for the slice: arrays element by element, scalars from at most
	// one element without narrowing; an empty list value-initializes. A
	// reference binds a single element of a reference-related type, else
	// a temporary the list initializes (8.5.4p3, see list_argument).
	// Designators name members of an aggregate class (see designated_list).
	if (list.designators != nullptr && ClassOf(type->is_reference() ? type->element : type) == nullptr)
		error(list.location, "designated initializers of no class");
	if (type->is_reference())
	{
		const syntax::Expr* element = list.elements.size == 1 && list.elements[0]->kind != syntax::ExprKind::BracedInit
			? list.elements[0]
			: nullptr;
		// (Typed once: a call's braced argument's element was, see
		// braced_argument.)
		const ExprFacts* facts = element != nullptr ? element->facts : nullptr;
		if (element != nullptr && facts == nullptr)
			facts = typed(*element, scope);
		if (failed())
			return;
		if (facts == nullptr || facts->type == nullptr || !conversions_.reference_related(type->element, facts->type))
		{
			list_argument(list, type, scope, list.location);
			return;
		}
	}
	if (type->unqualified->kind == TypeKind::Vector)
	{
		// A GNU vector: a lane from each element, the others zero.
		if (list.elements.size > type->unqualified->bound)
			error(list.location, "more initializers than the vector has lanes");
		for (const syntax::Expr* element : list.elements)
		{
			if (element->kind == syntax::ExprKind::BracedInit)
				error(element->location, "a braced vector lane initializer");
			initialize_element(*element, type->unqualified->element, scope);
			if (failed())
				return;
		}
		new_facts(list, type, ValueCategory::LValue);
		return;
	}
	if (PartList(type, list))
	{
		// A GNU complex value from its real and imaginary parts, each
		// converted to the part type (GCC's extension, which checks no
		// narrowing; one element initializes the whole value, see below).
		// A braced part is a part-typed scalar's list (one element or
		// none, 8.5.4p3; GCC and Clang: `{1.0, {}}` is 1+0i), a prvalue.
		for (const syntax::Expr* element : list.elements)
		{
			if (element->kind == syntax::ExprKind::BracedInit)
			{
				const syntax::BracedInitList& part = static_cast<const syntax::BracedInitList&>(*element);
				list_initialize(part, type->unqualified->element, scope);
				if (failed())
					return;
				facts_of(part)->category = ValueCategory::PRValue;
				continue;
			}
			if ((element->facts == nullptr && typed(*element, scope) == nullptr) ||
				convert(*element, type->unqualified->element, false) == nullptr)
				return;
		}
		new_facts(list, type, ValueCategory::LValue);
		return;
	}
	if (type->kind == TypeKind::Array)
	{
		// 8.5.2p1: a character array from a string literal, optionally in
		// braces (and parenthesized, GCC's and Clang's extension, as
		// check_character_array's and a member's).
		const syntax::Expr* literal = list.elements.size == 1 ? list.elements[0] : nullptr;
		while (literal != nullptr && literal->kind == syntax::ExprKind::Paren)
			literal = static_cast<const syntax::ParenExpr*>(literal)->inner;
		if (literal != nullptr && literal->kind == syntax::ExprKind::Literal && IsCharacterArrayElement(type->element))
		{
			ExprFacts* facts = typed(*list.elements[0], scope);
			if (facts == nullptr)
				return;
			if (facts->type->kind == TypeKind::Array)
			{
				if (!InitializesCharacterArray(facts->type->element, type->element))
					error(list.location, "array initialized from a string literal of another kind");
				if (type->bound != 0 && facts->type->bound > type->bound)
					error(list.location, "string literal too long for the array");
				// An omitted bound is the literal's (8.5.2p2, 8.5.1p4).
				new_facts(list, type->bound == 0 ? types_.array(type->element, facts->type->bound) : type,
					ValueCategory::LValue);
				return;
			}
		}
		braced_list(list, type, scope);
		return;
	}
	if (type->unqualified->kind == TypeKind::Class)
	{
		// 8.5.4p3: an aggregate is aggregate-initialized; another class by
		// the constructor the list selects (the list's facts' entity). The
		// classes initialized here are an aggregate's members and elements
		// (8.5.1p2) and std::initializer_list's array elements (8.5.4p5):
		// copy-list-initialized, so an explicit constructor selected is an
		// error (13.3.1.7p1).
		ClassEntity* cls = static_cast<ClassEntity*>(type->unqualified->entity);
		syntax::Initializer view = syntax::Initializer();
		view.kind = syntax::InitializerKind::Assign;
		view.expression = const_cast<syntax::BracedInitList*>(&list);
		view.location = list.location;
		initialize_class(cls, type, view, scope, list.location);
		return;
	}
	if (list.elements.size > 1)
		error(list.location, "list-initialization of a scalar from several values");
	// The list stands for the initialized object (an lvalue of its type in
	// the course view); an empty one value-initializes it.
	if (list.elements.empty())
	{
		new_facts(list, type, ValueCategory::LValue);
		return;
	}
	const syntax::Expr& element = *list.elements[0];
	if (element.kind == syntax::ExprKind::BracedInit)
		error(element.location, "unsupported nested braced initializer");
	initialize_element(element, type, scope);
	new_facts(list, type, ValueCategory::LValue);
}

void Analyzer::aggregate_initialize(const syntax::BracedInitList& list, ClassEntity* entity, Scope* scope)
{
	// 8.5.1p2: the members of an aggregate (8.5.1p1) are copy-initialized
	// from the elements in declaration order; the others are initialized
	// from an empty initializer list (p7).
	if (!require_complete(entity) || !IsAggregateClass(entity))
		error(list.location, "list-initialization of a class that is not an aggregate");
	if (list.designators != nullptr)
	{
		// The list becomes the one in member order it stands for (the walks
		// of the analysis, the constant evaluator and the lowering read).
		syntax::BracedInitList ordered = syntax::BracedInitList();
		designated_list(list, entity, ordered, scope);
		if (failed())
			return;
		const_cast<syntax::BracedInitList&>(list) = ordered;
	}
	++empty_list_epoch_;
	uint32_t next = 0;
	aggregate_members(list, next, entity, scope);
	if (next < list.elements.size)
		error(list.elements[next]->location, "too many initializers for the aggregate");
}

void Analyzer::aggregate_members(const syntax::BracedInitList& list, uint32_t& next, ClassEntity* entity, Scope* scope)
{
	// Unnamed bit-fields take no element (9.6p2), an anonymous union's or
	// struct's member object is a subaggregate; a union's one member is
	// initialized (8.5.1p15, see UnionListMember).
	if (entity->key == TokenKind::KW_UNION)
	{
		if (const DataMember* member = ListUnionMember(entity, &list, next))
			aggregate_element(list, next, member->variable->type, scope);
		return;
	}
	for (const DataMember* member = entity->members; member != nullptr; member = member->next)
	{
		if (member->bit_field && member->variable->name == nullptr)
			continue;
		const Type* type = member->variable->type;
		if (next < list.elements.size && hosted_ && type->kind == TypeKind::Array && type->bound == 0 &&
			type->owner == nullptr && template_depth_ == 0)
		{
			// GNU's flexible array member (see member_size) given elements:
			// only in an object of static storage duration's own list (see
			// flexible_list_); not in a nested one, an automatic object or a
			// temporary (GCC and Clang alike). `{}` gives none. A zero-length
			// array `T m[0]` (wherever it stands) takes none at all (GCC:
			// "too many initializers for 'int [0]'", Clang alike).
			const syntax::Expr* element = list.elements[next];
			if (element->kind != syntax::ExprKind::BracedInit ||
				!static_cast<const syntax::BracedInitList*>(element)->elements.empty())
			{
				if (member->variable->kind == EntityKind::Variable &&
					static_cast<const VariableEntity*>(member->variable)->zero_length)
					error(element->location, "a zero-length array member initialized with elements");
				if (member->next != nullptr || &list != flexible_list_ || entity != flexible_class_)
					error(element->location,
						"a flexible array member initialized outside the initializer of an object of "
						"static storage duration");
				flexible_given_ = true;
			}
		}
		aggregate_element(list, next, type, scope);
	}
}

void Analyzer::aggregate_element(const syntax::BracedInitList& list, uint32_t& next, const Type* type, Scope* scope)
{
	// One member or array element from the list at `next`: a braced list
	// initializes it; an expression initializes it too unless it is a
	// subaggregate the expression cannot initialize, whose elements then
	// come from this list (brace elision, 8.5.1p11). Without an element it
	// is value-initialized, a class by its default constructor; a
	// subaggregate's members once per initialization (8.5.1p7: their
	// checks and the constructors they run are the same each time).
	if (next >= list.elements.size)
	{
		if (!type->is_reference())
		{
			const Type* object = type;
			while (object->kind == TypeKind::Array)
				object = object->element;
			if (object->unqualified->kind == TypeKind::Class)
			{
				ClassEntity* cls = static_cast<ClassEntity*>(object->unqualified->entity);
				if (!IsAggregateClass(cls))
					value_initialize(cls, list.location);
				else if (cls->empty_list_mark != empty_list_epoch_)
				{
					cls->empty_list_mark = empty_list_epoch_;
					syntax::BracedInitList empty = syntax::BracedInitList();
					uint32_t none = 0;
					aggregate_members(empty, none, cls, scope);
				}
			}
		}
		// 8.5.1p7, 8.5.4p3: ill-formed; a substitution's failure (14.8.2p8).
		if (type->is_reference())
			substitution_error(list.location, "a reference member without an initializer");
		return;
	}
	const syntax::Expr& element = *list.elements[next];
	if (element.kind == syntax::ExprKind::BracedInit)
	{
		++next;
		list_initialize(static_cast<const syntax::BracedInitList&>(element), type, scope);
		return;
	}
	// A substitution's failure (see soft_substitution_) ends the element.
	if (element.facts == nullptr && typed(element, scope) == nullptr)
	{
		++next;
		return;
	}
	if (IsSubaggregate(type) && !InitializesSubaggregate(type, element))
	{
		if (type->kind == TypeKind::Array)
		{
			for (uint64_t i = 0; i < type->bound; ++i)
				aggregate_element(list, next, type->element, scope);
		}
		else
			aggregate_members(list, next, static_cast<ClassEntity*>(type->unqualified->entity), scope);
		return;
	}
	++next;
	const Type* object = type->is_reference() ? nullptr : type;
	if (object != nullptr && object->kind == TypeKind::Array)
	{
		check_character_array(object, element, element.facts, element.location);
		return;
	}
	if (object != nullptr && object->unqualified->kind == TypeKind::Class)
	{
		// A class member from an expression: a prvalue of its class
		// initializes it; another operand converts by a converting
		// constructor (copy-initialization, 8.5p16).
		ClassEntity* cls = static_cast<ClassEntity*>(object->unqualified->entity);
		if (!check_resolved(element))
			return;
		if (same_class_source(cls, element))
		{
			record_copy(cls, object, element, element.location);
			return;
		}
		Conversion conversion = Conversion();
		if (!user_conversion(element.facts, object, conversion))
			error(element.location, "no conversion to the member's class");
		expression_scope_ = scope;
		apply_user_conversion(element.facts, conversion, element.location);
		record_conversion(facts_of(element), conversion);
		return;
	}
	// A scalar or reference from the element typed above; a direct
	// aggregate construction's own element may narrow (see class_prvalue).
	const Conversion* conversion = convert(element, type, false);
	if (&list != aggregate_arguments_)
		check_narrowing(element, conversion, type, scope);
}

bool Analyzer::array_new_list(
	const syntax::NewExpr& expr, const syntax::BracedInitList& list, const Type* element, bool& trailing, Scope* scope)
{
	// 5.3.4p15, 8.5.4p3, 8.5.1: an array new-expression's braced-init-list
	// initializes the first elements as an array of unknown bound's (the
	// list deduces it, brace elision included, the list's facts its type);
	// the elements after them are initialized from empty lists (8.5.1p7),
	// value-initialized as `()` makes them. A constant count less than the
	// elements the list initializes is ill-formed (C++14's 5.3.4p7, which
	// g++ applies in C++11); a run-time one obtains no storage (see the
	// lowering's array_allocation). `trailing`: elements may follow.
	const ExprFacts* facts = nullptr;
	const syntax::Expr* literal = list.elements.size == 1 ? list.elements[0] : nullptr;
	while (literal != nullptr && literal->kind == syntax::ExprKind::Paren)
		literal = static_cast<const syntax::ParenExpr*>(literal)->inner;
	if (literal != nullptr && literal->kind == syntax::ExprKind::Literal && IsCharacterArrayElement(element))
	{
		// 8.5.2: characters from a string literal (in braces, perhaps
		// parenthesized as list_initialize's), its code units' array; the
		// element typed whole, parentheses included.
		const ExprFacts* string = typed(*list.elements[0], scope);
		if (string == nullptr)
			return false;
		if (string->type->kind == TypeKind::Array)
		{
			if (!InitializesCharacterArray(string->type->element, element))
				error(list.location, "array initialized from a string literal of another kind");
			facts = new_facts(list, types_.array(element, string->type->bound), ValueCategory::LValue);
		}
	}
	if (facts == nullptr)
		facts = braced_list(list, types_.array(element, 0), scope);
	if (failed() || facts == nullptr)
		return false;
	const uint64_t listed = facts->type->bound;
	trailing = true;
	Constant bound;
	if (constant_value(*expr.count, scope, bound, false))
	{
		IntegralKind kind;
		const bool negative = IntegralKindOf(bound.type, kind) && kind.is_signed &&
			static_cast<int64_t>(bound.bits) < 0;
		if (!negative && bound.bits < listed)
		{
			substitution_error(list.location, "more initializers than the array new-expression's elements");
			return false;
		}
		trailing = negative || bound.bits > listed;
		// A zero-length array of a class with a default constructor keeps
		// its (empty) element construction (the course's form).
		ClassEntity* cls = bound.bits == 0 && listed == 0 ? const_cast<ClassEntity*>(ClassOf(element)) : nullptr;
		const FunctionEntity* constructor = cls != nullptr ? implicit_constructor(cls) : nullptr;
		trailing = trailing || (constructor != nullptr && !constructor->deleted);
	}
	return true;
}

void Analyzer::initialize_element(const syntax::Expr& element, const Type* type, Scope* scope)
{
	// One scalar (or reference) copy-list-initialized from one element,
	// typed once (a call's braced argument's were, see braced_argument).
	if (element.facts == nullptr && typed(element, scope) == nullptr)
		return;
	const Conversion* conversion = convert(element, type, false);
	check_narrowing(element, conversion, type, scope);
}

bool Analyzer::check_narrowing(
	const syntax::Expr& element, const Conversion* conversion, const Type* type, Scope* scope)
{
	// 8.5.4p7: no narrowing conversion, unless from a constant that fits;
	// false for a substitution's failure (14.8.2p8). After a conversion
	// function its second standard conversion is checked, from the
	// function's result (13.3.3.1.2p1; a constant when the call is one,
	// 5.19p2); a converting constructor's is none of the list's.
	if (conversion == nullptr)
		return true;
	const ExprFacts* facts = element.facts;
	const Type* target = type->is_reference() ? type->element->unqualified : type->unqualified;
	const Type* source = facts->type->unqualified;
	ValueConversion second = conversion->second;
	Conversion call = Conversion();
	const Conversion* before = nullptr;
	if (conversion->constructor != nullptr)
	{
		if (conversion->constructor->constructor || conversion->after == nullptr)
			return true;
		const Type* returned = conversion->constructor->type->canonical->element;
		source = (returned->is_reference() ? returned->element : returned)->unqualified;
		second = conversion->after->second;
		call = *conversion;
		call.after = nullptr;
		before = &call;
	}
	bool narrowing = false;
	if (second == ValueConversion::FloatingIntegral)
	{
		// From a floating type always; from an integer unless a constant
		// converts to the floating type and back unchanged.
		Constant value;
		narrowing = IsFloating(source) || !narrowing_constant(element, before, scope, &value, nullptr) ||
			!ExactlyFloating(value, target);
	}
	else if (second == ValueConversion::FloatingConversion &&
		!(FloatingRank(target->fundamental) > FloatingRank(source->fundamental) ||
			FloatingRepresentation(target->fundamental) == FloatingRepresentation(source->fundamental)))
	{
		// To a narrower floating type unless from a constant whose value
		// converts to one within the target's range, even if inexactly
		// (binary16's: below 65520, which rounds to infinity; bfloat16's
		// below 2^128 - 2^119).
		long double value = 0;
		const FundamentalType representation = FloatingRepresentation(target->fundamental);
		narrowing = !narrowing_constant(element, before, scope, nullptr, &value) ||
			(representation == FundamentalType::Float             ? std::isinf(static_cast<float>(value))
					: representation == FundamentalType::Float16  ? std::fabs(value) >= 65520.0L
					: representation == FundamentalType::BFloat16 ? std::fabs(value) >= std::ldexp(511.0L, 119)
																  : std::isinf(static_cast<double>(value)));
	}
	else if (second == ValueConversion::Boolean && IsFloating(source))
	{
		// From a floating type to bool, an integer type (3.9.1p7): always.
		narrowing = true;
	}
	else if (second == ValueConversion::IntegralConversion ||
		(second == ValueConversion::Boolean && IsIntegralOrUnscopedEnum(source)))
	{
		IntegralKind to, from;
		if (IntegralKindOf(target, to) && IntegralKindOf(source, from) &&
			(to.bits < from.bits || (to.bits == from.bits && to.is_signed != from.is_signed) ||
				(to.is_signed == false && from.is_signed)))
		{
			Constant value;
			narrowing = !narrowing_constant(element, before, scope, &value, nullptr) || !Representable(value, to);
		}
	}
	if (narrowing)
		return substitution_error(element.location, "narrowing conversion in list-initialization");
	return true;
}

bool Analyzer::narrowing_constant(
	const syntax::Expr& element, const Conversion* before, Scope* scope, Constant* integer, long double* floating)
{
	// The narrowed source's value as a constant (an integer or a floating
	// one): the element's, or the result of the conversion function
	// `before` records without its second standard conversion.
	if (before == nullptr)
		return integer != nullptr ? constant_value(element, scope, *integer, false)
								  : floating_constant(element, scope, *floating);
	ConstantInterpreter interpreter(*this);
	ConstValue result;
	if (!interpreter.evaluate_after(element, before, result))
		return false;
	if (integer != nullptr && result.kind == ValueKind::Integer && HeldAsConstant(result))
		*integer = Constant{ result.bits, result.type };
	else if (floating != nullptr && result.kind == ValueKind::Floating)
		*floating = result.floating;
	else
		return false;
	return true;
}

void Analyzer::record_default_arguments(FunctionEntity* function, const syntax::ParameterClause* clause, Scope* scope)
{
	// 8.3.6: a default argument is checked like the initializer of a
	// parameter-typed object, where the declaration appears; later
	// declarations may add defaults for earlier parameters.
	const Type* type = function->type->canonical;
	if (clause->parameters.size != type->param_count)
		return;
	for (uint32_t i = 0; i < type->param_count; ++i)
	{
		const syntax::Initializer* initializer = clause->parameters[i]->default_argument;
		if (initializer == nullptr)
			continue;
		if (function->default_arguments == nullptr)
		{
			function->default_arguments = static_cast<const syntax::Expr**>(
				arena_.allocate(sizeof(syntax::Expr*) * type->param_count, alignof(syntax::Expr*)));
			std::memset(static_cast<void*>(function->default_arguments), 0, sizeof(syntax::Expr*) * type->param_count);
		}
		else if (function->default_arguments[i] != nullptr)
			error(clause->parameters[i]->location, "default argument redefined");
		const syntax::Expr* expr = initializer->expression;
		if (expr == nullptr)
			error(clause->parameters[i]->location, "a default argument without an expression");
		function->default_arguments[i] = expr;
	}
	// A member's defaults in its class are a complete-class context
	// (9.2p2): typed with the member bodies once the outermost class is
	// complete (see finish_class_nesting); a class template instance's,
	// where a call first uses them (14.7.1p1).
	if (instance_class(function->owner))
	{
	}
	else if (function->member && class_depth_ != 0)
		deferred_.push_back(DeferredBody{ function, type, clause, nullptr, nullptr, scope });
	else
		type_default_arguments(function, clause, scope);
	// 8.3.6p4: after a parameter with a default, each later one has one.
	if (function->default_arguments != nullptr)
	{
		bool seen = false;
		for (uint32_t i = 0; i < type->param_count; ++i)
		{
			if (function->default_arguments[i] != nullptr)
				seen = true;
			else if (seen)
				error(clause->location, "missing default argument after a parameter with one");
		}
	}
}

void Analyzer::record_template_default_arguments(
	FunctionEntity* function, const syntax::ParameterClause& clause, Scope* scope)
{
	// A function template's default arguments stay patterns: each
	// specialization instantiates one when a call uses it (14.7.1p12).
	const uint32_t count = function->type->canonical->param_count;
	if (clause.parameters.size != count)
		return;
	for (uint32_t i = 0; i < count; ++i)
	{
		const syntax::Initializer* initializer = clause.parameters[i]->default_argument;
		if (initializer == nullptr || initializer->expression == nullptr)
			continue;
		// The parameters without one stay null (the arena's storage is
		// uninitialized): a call supplies them (8.3.6p1).
		if (function->default_arguments == nullptr)
		{
			function->default_arguments = arena_.allocate_array<const syntax::Expr*>(count);
			std::memset(static_cast<void*>(function->default_arguments), 0, sizeof(syntax::Expr*) * count);
		}
		else if (function->default_arguments[i] != nullptr)
			error(clause.parameters[i]->location, "default argument redefined");
		function->default_arguments[i] = initializer->expression;
		HeadOf(function)->argument_defaults = scope;
	}
}

// ------------------------------------------- braced-init-list arguments

void Analyzer::braced_argument(const syntax::BracedInitList& list, Scope* scope)
{
	// 13.3.3.1.5p1: a braced-init-list argument is no expression: typeless
	// facts until its parameter's initialization; its elements are typed
	// for ranking.
	new_facts(list, nullptr, ValueCategory::PRValue);
	for (const syntax::Expr* element : list.elements)
	{
		if (element->kind == syntax::ExprKind::BracedInit)
			braced_argument(static_cast<const syntax::BracedInitList&>(*element), scope);
		else
			typed(*element, scope);
		if (failed())
			return;
	}
}

bool Analyzer::list_sequence(const syntax::BracedInitList& list, const Type* parameter, Conversion& out)
{
	// 13.3.3.1.5 (std::initializer_list aside): a reference binds the
	// temporary the list initializes (p5; a non-const lvalue reference
	// none); a non-aggregate class is a user-defined conversion by the
	// constructor 13.3.1.7 chooses (p3; `{}` value-initializes, 8.5.4p3),
	// an aggregate one by aggregate initialization (p4); a scalar is its
	// one element's conversion, or identity for `{}` (p6).
	const bool reference = parameter->is_reference();
	const Type* target = reference ? parameter->element : parameter;
	if (const Type* element = initializer_list_element(target))
		return initializer_list_sequence(list, parameter, element, out);
	// An array's cv-qualifiers are its elements' (3.9.3p5).
	const Type* qualified = target;
	while (qualified->kind == TypeKind::Array)
		qualified = qualified->element;
	if (reference && parameter->kind == TypeKind::LValueReference && (qualified->cv & (kConst | kVolatile)) != kConst)
		return false;
	Conversion sequence = Conversion();
	sequence.form = ConversionForm::Standard;
	sequence.rank = ConversionRank::Exact;
	if (IsVoid(target))
		return false;
	if (target->kind == TypeKind::Array)
	{
		// p2: an array of X (a reference's temporary) whose elements each
		// convert to X: the worst of those conversions (identity for none),
		// no more elements than its bound (8.5.1p6).
		if (target->bound == 0 || list.elements.size > target->bound)
			return false;
		for (const syntax::Expr* element : list.elements)
		{
			Conversion step = Conversion();
			if (element->kind == syntax::ExprKind::BracedInit
					? !list_sequence(static_cast<const syntax::BracedInitList&>(*element), target->element, step)
					: !implicit_sequence(*element, target->element, step))
				return false;
			if (conversions_.compare(step, sequence) < 0)
				sequence = step;
		}
		sequence.form = ConversionForm::Standard;
		sequence.result = target;
		sequence.target = parameter;
		if (reference)
		{
			sequence.form = ConversionForm::Reference;
			sequence.rvalue_reference = parameter->kind == TypeKind::RValueReference;
			sequence.temporary = true;
		}
		out = sequence;
		return true;
	}
	if (ClassEntity* cls = const_cast<ClassEntity*>(ClassOf(target)))
	{
		if (!require_complete(cls) || cls->scope == nullptr)
			return false;
		sequence.rank = ConversionRank::UserDefined;
		if (IsAggregateClass(cls))
		{
			if (!aggregate_accepts(list, cls))
				return false;
		}
		else
		{
			sequence.constructor = list_constructor(list, cls);
			if (sequence.constructor == nullptr)
				return false;
		}
		sequence.result = target->unqualified;
	}
	else if (PartList(target, list))
	{
		// GCC's `{re, im}` list of a complex value's parts (see
		// list_initialize): each part converts to the part type (a braced
		// part as a scalar's list), the worst of those conversions.
		const Type* part = target->unqualified->element;
		for (const syntax::Expr* element : list.elements)
		{
			Conversion step = Conversion();
			if (element->kind == syntax::ExprKind::BracedInit
					? !list_sequence(static_cast<const syntax::BracedInitList&>(*element), part, step)
					: !implicit_sequence(*element, part, step))
				return false;
			if (conversions_.compare(step, sequence) < 0)
				sequence = step;
		}
		// A part converted by a user-defined conversion makes the list's a
		// user-defined conversion sequence of no one conversion function,
		// indistinguishable from another such (GCC's user-rank aggregate
		// conversion, as 13.3.3.1.5p4 ranks an aggregate's list).
		if (sequence.rank == ConversionRank::UserDefined)
		{
			sequence = Conversion();
			sequence.rank = ConversionRank::UserDefined;
		}
		sequence.form = ConversionForm::Standard;
		sequence.result = target->unqualified;
	}
	else
	{
		if (list.elements.size > 1)
			return false;
		if (list.elements.size == 1 &&
			(list.elements[0]->kind == syntax::ExprKind::BracedInit ||
				!implicit_sequence(*list.elements[0], target->unqualified, sequence)))
			return false;
		sequence.result = target->unqualified;
	}
	sequence.target = parameter;
	if (reference)
	{
		sequence.form = ConversionForm::Reference;
		sequence.rvalue_reference = parameter->kind == TypeKind::RValueReference;
		sequence.temporary = true;
		sequence.result = target;
	}
	out = sequence;
	return true;
}

bool Analyzer::aggregate_accepts(const syntax::BracedInitList& list, const ClassEntity* cls)
{
	// 13.3.3.1.5p4: the list aggregate-initializes the class (8.5.1p2), for
	// ranking: each element converts to the next member, none left over. A
	// designated list as the list in member order it stands for in this
	// class (unchanged: another candidate's class may take it).
	uint32_t next = 0;
	if (list.designators != nullptr)
	{
		syntax::BracedInitList ordered = syntax::BracedInitList();
		if (!designated_list(list, cls, ordered, nullptr))
			return false;
		return members_accept(ordered, next, cls) && next == ordered.elements.size;
	}
	return members_accept(list, next, cls) && next == list.elements.size;
}

bool Analyzer::designated_list(
	const syntax::BracedInitList& list, const ClassEntity* cls, syntax::BracedInitList& out, Scope* scope)
{
	// Designated initializers (C99; C++20 9.4.5p3-p4, an extension of hosted
	// compilations here): every element designates a direct non-static data
	// member of the aggregate (GCC's rule: all or none), in declaration order
	// (C++20; GCC's too), or a member of an anonymous struct or union member
	// (whose names are the class's, 9.5p1; GCC and Clang): the consecutive
	// elements designating its members are its own designated list. A
	// subaggregate member from an expression that does not initialize it
	// takes that one element in braces (C99's brace elision, which GCC and
	// Clang keep: the next element designates another member). `out` is
	// the list in member order: a member no element designates before the
	// last one is initialized from an empty list `{}`, as 8.5.1p7
	// initializes the members after the last; a union's one element
	// initializes the designated member. With `scope` (the initialization
	// itself) a list that does not fit is an error, else false.
	out = list;
	out.designators = nullptr;
	out.union_member = nullptr;
	std::vector<syntax::Expr*> elements;
	const DataMember* member = cls->members;
	for (uint32_t i = 0; i < list.elements.size; ++i)
	{
		const Identifier* designator = list.designators[i];
		const char* problem = designator == nullptr     ? "a designated list with an element without a designator"
			: cls->key == TokenKind::KW_UNION && i != 0 ? "a designated list of a union with several elements"
														: nullptr;
		const DataMember* found = member;
		const ClassEntity* anonymous = nullptr;
		while (problem == nullptr && found != nullptr && found->variable->name != designator)
		{
			anonymous = AnonymousMemberClass(found);
			if (anonymous != nullptr && AnonymousHolds(anonymous, designator))
				break;
			anonymous = nullptr;
			found = found->next;
		}
		if (problem == nullptr && found == nullptr)
			problem = "a designator that names no later direct non-static data member (declaration order)";
		if (problem != nullptr)
		{
			if (scope != nullptr)
				error(list.elements[i]->location, problem);
			return false;
		}
		for (; cls->key != TokenKind::KW_UNION && member != found; member = member->next)
		{
			if (member->bit_field && member->variable->name == nullptr)
				continue;
			syntax::BracedInitList* empty = static_cast<syntax::BracedInitList*>(
				arena_.allocate(sizeof(syntax::BracedInitList), alignof(syntax::BracedInitList)));
			*empty = syntax::BracedInitList();
			empty->kind = syntax::ExprKind::BracedInit;
			empty->location = list.elements[i]->location;
			elements.push_back(empty);
		}
		syntax::Expr* element = list.elements[i];
		const Type* type = found->variable->type;
		if (anonymous != nullptr)
		{
			// The anonymous member's list: this element and the next ones
			// designating its members.
			uint32_t end = i + 1;
			while (end < list.elements.size && list.designators[end] != nullptr &&
				AnonymousHolds(anonymous, list.designators[end]))
				++end;
			syntax::BracedInitList* own = static_cast<syntax::BracedInitList*>(
				arena_.allocate(sizeof(syntax::BracedInitList), alignof(syntax::BracedInitList)));
			*own = syntax::BracedInitList();
			own->kind = syntax::ExprKind::BracedInit;
			own->location = element->location;
			own->elements = syntax::Slice<syntax::Expr*>{ list.elements.data + i, end - i };
			own->designators = list.designators + i;
			element = own;
			i = end - 1;
		}
		else if (element->kind != syntax::ExprKind::BracedInit && IsSubaggregate(type) &&
			(scope == nullptr ? element->facts != nullptr : typed(*element, scope) != nullptr) &&
			!InitializesSubaggregate(type, *element))
		{
			syntax::BracedInitList* braced = static_cast<syntax::BracedInitList*>(
				arena_.allocate(sizeof(syntax::BracedInitList), alignof(syntax::BracedInitList)));
			*braced = syntax::BracedInitList();
			braced->kind = syntax::ExprKind::BracedInit;
			braced->location = element->location;
			braced->elements = syntax::Slice<syntax::Expr*>{ list.elements.data + i, 1 };
			element = braced;
		}
		if (scope != nullptr && failed())
			return false;
		elements.push_back(element);
		if (cls->key == TokenKind::KW_UNION)
			out.union_member = found;
		member = found->next;
	}
	out.elements = syntax::Slice<syntax::Expr*>{ elements.empty() ? nullptr
																  : arena_.copy_array(elements.data(), elements.size()),
		static_cast<uint32_t>(elements.size()) };
	return true;
}

bool Analyzer::members_accept(const syntax::BracedInitList& list, uint32_t& next, const ClassEntity* cls)
{
	// The members from the list at `next` in declaration order: an unnamed
	// bit-field takes no element, a union's first named member alone takes
	// one (8.5.1p15-p16).
	const DataMember* selected = cls->key == TokenKind::KW_UNION ? ListUnionMember(cls, &list, next) : nullptr;
	for (const DataMember* member = cls->members; member != nullptr && next < list.elements.size; member = member->next)
	{
		if ((member->variable->name == nullptr && member->bit_field) || (selected != nullptr && member != selected))
			continue;
		if (!element_accepts(list, next, member->variable->type))
			return false;
		if (cls->key == TokenKind::KW_UNION)
			break;
	}
	return true;
}

bool Analyzer::elements_accept(const syntax::BracedInitList& list, uint32_t& next, const Type* array)
{
	// An array's elements from the list at `next`, no more than its bound.
	for (uint64_t i = 0; i < array->bound && next < list.elements.size; ++i)
	{
		if (!element_accepts(list, next, array->element))
			return false;
	}
	return true;
}

bool Analyzer::element_accepts(const syntax::BracedInitList& list, uint32_t& next, const Type* type)
{
	// One member or array element from the list at `next`, as
	// aggregate_element reads it: a braced list initializes it (an array's
	// as braced_list reads it, brace elision within); a subaggregate an
	// expression does not initialize takes its elements from this list
	// (brace elision, 8.5.1p11); a character array a string literal, in
	// braces or not (8.5.2p1).
	const syntax::Expr& element = *list.elements[next];
	Conversion ignored = Conversion();
	if (element.kind == syntax::ExprKind::BracedInit)
	{
		++next;
		const syntax::BracedInitList& own = static_cast<const syntax::BracedInitList&>(element);
		if (type->kind != TypeKind::Array)
			return list_sequence(own, type, ignored);
		if (own.elements.size == 1 && own.elements[0]->kind != syntax::ExprKind::BracedInit &&
			InitializesSubaggregate(type, *own.elements[0]))
			return true;
		uint32_t inner = 0;
		return elements_accept(own, inner, type) && inner == own.elements.size;
	}
	if (IsSubaggregate(type) && !InitializesSubaggregate(type, element))
		return type->kind == TypeKind::Array ? elements_accept(list, next, type)
											 : members_accept(list, next, ClassOf(type));
	++next;
	return type->kind == TypeKind::Array || implicit_sequence(element, type, ignored);
}

const FunctionEntity* Analyzer::list_constructor(const syntax::BracedInitList& list, ClassEntity* cls)
{
	// 13.3.1.7: the default constructor for `{}` (8.5.4p3), else an
	// initializer-list constructor taking the list, else the constructor
	// overload resolution chooses for the elements (explicit ones
	// considered); the enclosing resolution's state is kept aside.
	if (list.elements.empty() && implicit_constructor(cls) != nullptr)
		return implicit_constructor(cls);
	std::vector<FunctionEntity*> set;
	std::vector<Candidate> saved_candidates;
	std::vector<Conversion> saved_sequences;
	saved_candidates.swap(candidates_);
	saved_sequences.swap(sequences_);
	Candidate chosen = Candidate();
	const FunctionEntity* found = list_constructor_phase(cls, list, set, chosen);
	if (found == nullptr)
	{
		set.clear();
		constructor_candidates(cls, list.elements, ConstructionKind::CopyList, set);
		if (!set.empty() && resolve(list.elements, set, nullptr, chosen) == Resolution::Selected)
			found = chosen.function;
	}
	candidates_.swap(saved_candidates);
	sequences_.swap(saved_sequences);
	return found;
}

void Analyzer::list_argument(const syntax::BracedInitList& list, const Type* parameter, Scope* scope, uint32_t location)
{
	// 8.5p15, 8.5.4p3: the parameter (a reference's temporary) is copy-
	// list-initialized: a class object by the constructor the list selects
	// or by aggregate initialization, a scalar from its element. The list
	// stands for that object, a prvalue, which an lvalue reference to a type
	// that is not const (or is volatile) cannot bind (8.5.4p3's note,
	// 8.5.3p5; an array's cv-qualifiers are its elements', 3.9.3p5).
	const Type* target = parameter->is_reference() ? parameter->element : parameter;
	const Type* qualified = target;
	while (qualified->kind == TypeKind::Array)
		qualified = qualified->element;
	if (parameter->kind == TypeKind::LValueReference && (qualified->cv & (kConst | kVolatile)) != kConst)
	{
		substitution_error(location, "an lvalue reference to a type that is not const bound to a list's temporary");
		return;
	}
	// 8.5.4p3, p5: std::initializer_list<E>'s object (a reference's
	// temporary) refers to the array the elements initialize.
	if (initializer_list_element(target) != nullptr)
	{
		initializer_list_object(list, target, scope);
		return;
	}
	if (target->kind == TypeKind::Array)
	{
		// A reference's temporary array (8.5.4p3), aggregate-initialized.
		if (braced_list(list, target, scope) != nullptr)
			facts_of(list)->category = ValueCategory::PRValue;
		return;
	}
	if (ClassEntity* cls = const_cast<ClassEntity*>(ClassOf(target)))
	{
		syntax::Initializer view = syntax::Initializer();
		view.kind = syntax::InitializerKind::Assign;
		view.expression = const_cast<syntax::BracedInitList*>(&list);
		view.location = location;
		initialize_class(cls, target, view, scope, location);
		// A reference's temporary has the referenced type's cv-qualifiers.
		facts_of(list)->type = target;
		return;
	}
	list_initialize(list, target->unqualified, scope);
	facts_of(list)->category = ValueCategory::PRValue;
}

void Analyzer::type_default_arguments(FunctionEntity* function, const syntax::ParameterClause* clause, Scope* scope)
{
	// Each default argument this clause gives copy-initializes its
	// parameter (8.3.6p1); `this` and non-static members are unavailable
	// (8.3.6p8-p9: no body context). A template's are only name-checked
	// (see default_argument_names).
	if (template_depth_ != 0)
	{
		for (const syntax::ParameterDeclaration* parameter : clause->parameters)
		{
			if (parameter->default_argument != nullptr)
				expression_walk(parameter->default_argument->expression, scope);
		}
		return;
	}
	BodyContext* saved = body_;
	BodyContext context = BodyContext();
	body_ = &context;
	const Type* type = function->type->canonical;
	for (uint32_t i = 0; i < type->param_count; ++i)
	{
		const syntax::Initializer* initializer = clause->parameters[i]->default_argument;
		if (initializer == nullptr)
			continue;
		context.closure_context = function;
		context.default_argument = i;
		if (initializer->expression->kind == syntax::ExprKind::BracedInit)
		{
			// 8.3.6p1, 8.5.4: `= {...}` copy-list-initializes the parameter.
			list_argument(static_cast<const syntax::BracedInitList&>(*initializer->expression), type->params[i], scope,
				initializer->expression->location);
			continue;
		}
		typed(*initializer->expression, scope);
		// A deferred reserved builtin call (see deferrable_statement).
		if (deferred_reserved_call())
			break;
		convert(*initializer->expression, type->params[i], false);
		// A class parameter's copy from an expression of its class.
		const Type* parameter = type->params[i];
		ClassEntity* cls = parameter->is_reference() ? nullptr : const_cast<ClassEntity*>(ClassOf(parameter));
		if (cls != nullptr && same_class_source(cls, *initializer->expression))
		{
			expression_scope_ = scope;
			record_copy(cls, parameter, *initializer->expression, initializer->expression->location);
		}
	}
	body_ = saved;
}

void Analyzer::name_local_type(ClassEntity* entity)
{
	// The stable presentation name of an unnamed class (README: anonymous
	// entities get typed identities, not source names).
	if (!semantic() || entity->local_label != nullptr)
		return;
	char buffer[40];
	const int length = std::snprintf(buffer, sizeof buffer, "__local_type%u", ++graph_.local_types);
	entity->local_label = copy_label(buffer, static_cast<size_t>(length));
}

} // namespace sema
