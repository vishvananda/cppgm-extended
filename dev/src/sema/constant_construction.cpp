#include "sema/constant_interpreter.h"

#include "sema/aggregates.h"
#include "sema/analyzer.h"
#include "support/small_list.h"

// Object initialization in constant evaluation (8.5, 8.5.1, 12.6): class
// prvalues materialized in the object they initialize, aggregates member
// by member with brace elision, and constructors run through the plan of
// subobject initializations the analysis recorded (as the lowering runs
// them, lower/object_lowering.cpp).
namespace sema {

namespace {

const syntax::Expr* WithoutParentheses(const syntax::Expr* expr)
{
	while (expr->kind == syntax::ExprKind::Paren)
		expr = static_cast<const syntax::ParenExpr*>(expr)->inner;
	return expr;
}

// The conversion the analysis recorded where an operand is used; none for
// an expression it left untyped (a hosted wrapper's deferred statement, see
// Analyzer::define_function), whose evaluation then fails.
const Conversion* RecordedConversion(const syntax::Expr* expr)
{
	const ExprFacts* facts = WithoutParentheses(expr)->facts;
	return facts != nullptr ? facts->conversion : nullptr;
}

bool ConversionFunction(const Conversion* conversion)
{
	return conversion != nullptr && conversion->constructor != nullptr && !conversion->constructor->constructor;
}

const syntax::BracedInitList* BracedList(const syntax::Initializer& init)
{
	if (init.kind == syntax::InitializerKind::Braced ||
		(init.kind == syntax::InitializerKind::Assign && init.expression->kind == syntax::ExprKind::BracedInit))
		return static_cast<const syntax::BracedInitList*>(init.expression);
	return nullptr;
}

// A mem-initializer as the initializer it is (12.6.2p3).
const syntax::Initializer* MemInitializerView(const syntax::MemInitializer* written, syntax::Initializer& view)
{
	if (written == nullptr)
		return nullptr;
	view.kind = written->braced ? syntax::InitializerKind::Braced : syntax::InitializerKind::Parenthesized;
	view.expression = written->list;
	view.arguments = written->arguments;
	view.location = written->location;
	return &view;
}

// 8.4.2p4: not user-provided when implicitly declared or defaulted on its
// first declaration.
bool UserProvidedFunction(const FunctionEntity* function)
{
	return !function->implicit && !function->defaulted;
}

// A trivial copy or move constructor: the source's value (12.8p16).
bool CopiesStorage(const FunctionEntity* function)
{
	return function->trivial &&
		(function->copy_kind == CopyKind::CopyConstructor || function->copy_kind == CopyKind::MoveConstructor);
}

} // namespace

// --------------------------------------------------------------- prvalues

bool ConstantInterpreter::temporary(const syntax::Expr& expr, ConstPlace& out)
{
	// 12.2p1: the temporary a class prvalue initializes (it lives until the
	// evaluation leaves the statement or call that made it).
	const syntax::Expr* e = WithoutParentheses(&expr);
	return new_object(e->facts->type, out, e->location) && materialize(*e, out);
}

bool ConstantInterpreter::materialize(const syntax::Expr& expr, const ConstPlace& target)
{
	// A class prvalue initializes the object it is materialized in (12.2,
	// 12.8p31): `T(args)`, `T{...}` and `T()` construct there, a call's
	// result is returned into it.
	const syntax::Expr* e = WithoutParentheses(&expr);
	const ExprFacts* facts = e->facts;
	if (facts == nullptr || facts->type == nullptr)
		return fail(e->location, "an expression outside constant evaluation");
	if (!step(e->location))
		return false;
	const FunctionEntity* constructor = facts->entity != nullptr && facts->entity->kind == EntityKind::Function
		? static_cast<const FunctionEntity*>(facts->entity)
		: nullptr;
	const Type* type = facts->type;
	syntax::Initializer view = syntax::Initializer();
	view.location = e->location;
	if (facts->category != ValueCategory::PRValue)
	{
		// A glvalue of the class: its value copied (the analysis left the
		// copy to the object's bytes).
		ConstPlace source;
		ConstValue* value = nullptr;
		if (!glvalue(*e, source) || !locate(source, false, value, e->location))
			return false;
		return write(target, *value, e->location);
	}
	switch (e->kind)
	{
	case syntax::ExprKind::FunctionalCast:
	{
		const syntax::FunctionalCastExpr& cast = static_cast<const syntax::FunctionalCastExpr&>(*e);
		if (cast.braced != nullptr)
		{
			view.kind = syntax::InitializerKind::Braced;
			view.expression = cast.braced;
		}
		else
		{
			view.kind = syntax::InitializerKind::Parenthesized;
			view.arguments = cast.arguments;
		}
		return construct(type, target, &view, constructor, e->location);
	}
	case syntax::ExprKind::BracedInit:
		// A braced-init-list copy-list-initializes its object (8.5.4p3).
		view.kind = syntax::InitializerKind::Assign;
		view.expression = const_cast<syntax::Expr*>(e);
		return construct(type, target, &view, constructor, e->location);
	case syntax::ExprKind::Cast:
	{
		// 5.2.9p4, 5.4p4: `(T)e` and `static_cast<T>(e)` are `T t(e);`.
		syntax::Expr* operand = static_cast<const syntax::CastExpr*>(e)->operand;
		view.kind = syntax::InitializerKind::Parenthesized;
		view.arguments.data = &operand;
		view.arguments.size = 1;
		return construct(type, target, &view, constructor, e->location);
	}
	case syntax::ExprKind::Call:
	{
		const syntax::CallExpr& call_expr = static_cast<const syntax::CallExpr&>(*e);
		if (call_expr.callee->facts == nullptr)
		{
			// `T(args)` whose type name the parser did not classify.
			view.kind = syntax::InitializerKind::Parenthesized;
			view.arguments = call_expr.arguments;
			return construct(type, target, &view, constructor, e->location);
		}
		Result result = Result();
		result.target = &target;
		return call(call_expr, result);
	}
	case syntax::ExprKind::Conditional:
	{
		// 5.16p6: the selected operand copy-initializes the object.
		const syntax::ConditionalExpr& choice = static_cast<const syntax::ConditionalExpr&>(*e);
		bool selected = false;
		if (!truth(*choice.condition, selected))
			return false;
		const syntax::Expr& chosen = selected ? *choice.then_expr : *choice.else_expr;
		return initialize_from(chosen, WithoutParentheses(&chosen)->facts->conversion, target);
	}
	case syntax::ExprKind::Binary:
		if (static_cast<const syntax::BinaryExpr*>(e)->op == TokenKind::OP_COMMA && (facts->flags & kOperatorCall) == 0)
			return discard(*static_cast<const syntax::BinaryExpr*>(e)->left) &&
				materialize(*static_cast<const syntax::BinaryExpr*>(e)->right, target);
		break;
	default:
		break;
	}
	if ((facts->flags & kOperatorCall) != 0 || (facts->flags & kUserLiteral) != 0)
	{
		Result result = Result();
		result.target = &target;
		return operator_call(*e, result);
	}
	if ((facts->flags & kUserConverted) != 0)
	{
		Result result = Result();
		result.target = &target;
		return conversion_call(*e, facts->original, static_cast<const FunctionEntity*>(facts->entity), result);
	}
	return fail(e->location, "a class prvalue outside constant evaluation");
}

bool ConstantInterpreter::initialize_from(
	const syntax::Expr& expr, const Conversion* conversion, const ConstPlace& target)
{
	// A class object copy-initialized from an expression (8.5p16) as its
	// recorded conversion says: a conversion function's result, a
	// converting, copy or move constructor, a glvalue's value, or the class
	// prvalue itself constructing the object (12.8p31).
	const syntax::Expr* e = WithoutParentheses(&expr);
	if (ConversionFunction(conversion))
	{
		const FunctionEntity* function = conversion->constructor;
		const Conversion* after = conversion->after;
		const FunctionEntity* constructor = after != nullptr ? after->constructor : nullptr;
		Result result = Result();
		if (constructor == nullptr)
		{
			result.target = &target;
			return conversion_call(*e, e->facts, function, result);
		}
		const Type* returned = function->type->canonical->element;
		ConstPlace source;
		if (returned->is_reference())
		{
			if (!conversion_call(*e, e->facts, function, result))
				return false;
			source = result.place;
		}
		else
		{
			if (!new_object(returned, source, e->location))
				return false;
			result.target = &source;
			if (!conversion_call(*e, e->facts, function, result))
				return false;
		}
		const Type* parameter = constructor->type->canonical->params[0];
		if (after->argument != nullptr && after->argument->second == ValueConversion::DerivedToBase &&
			!to_base(source, ClassOf(parameter->element), e->location))
			return false;
		Argument argument = Argument();
		argument.bound = true;
		argument.place = source;
		return run_constructor(constructor, target, &argument, 1, e->location);
	}
	if (conversion != nullptr && conversion->constructor != nullptr)
	{
		Argument argument = Argument();
		argument.expr = &expr;
		argument.conversion = conversion->argument;
		return run_constructor(conversion->constructor, target, &argument, 1, e->location);
	}
	if (e->facts->category != ValueCategory::PRValue)
	{
		ConstPlace source;
		ConstValue* value = nullptr;
		if (!glvalue(*e, source))
			return false;
		const ClassEntity* to = ClassOf(
			conversion != nullptr && conversion->result != nullptr ? conversion->result : e->facts->type);
		if (to != nullptr && ClassOf(source.type) != to && !to_base(source, to, e->location))
			return false;
		return locate(source, false, value, e->location) && write(target, *value, e->location);
	}
	return materialize(*e, target);
}

bool ConstantInterpreter::construct(const Type* type, const ConstPlace& target, const syntax::Initializer* init,
	const FunctionEntity* constructor, uint32_t location)
{
	// 8.5p16 for the object at `target`: a class object by the constructor
	// the analysis selected, value-initialization, aggregate
	// initialization or a class prvalue; another object from its
	// initializer.
	if (!step(location))
		return false;
	const ClassEntity* cls = type->is_reference() || type->kind == TypeKind::Array ? nullptr : ClassOf(type);
	if (cls == nullptr)
	{
		if (init != nullptr)
			return initialize_object(type, target, *init, location);
		// Default-initialization: an array's class elements are
		// constructed, scalars left indeterminate (8.5p6).
		const Type* element = type;
		while (element->kind == TypeKind::Array)
			element = element->element;
		if (type->kind != TypeKind::Array || constructor == nullptr || constructor->trivial)
			return true;
		const uint64_t size = size_of(element, location);
		const uint64_t total = size_of(type, location);
		for (uint64_t offset = 0; size != 0 && offset < total; offset += size)
		{
			ConstPlace place = target;
			place.offset += static_cast<int64_t>(offset);
			place.type = element;
			if (!constructor_call(constructor, place, syntax::Slice<syntax::Expr*>(), location))
				return false;
		}
		return true;
	}
	if (init == nullptr)
	{
		if (constructor != nullptr && !constructor->trivial)
			return constructor_call(constructor, target, syntax::Slice<syntax::Expr*>(), location);
		return true;
	}
	if (const syntax::BracedInitList* list = BracedList(*init))
	{
		if (constructor == nullptr && IsAggregateClass(cls))
			return aggregate(type, target, *list, location);
		if (list->elements.empty())
			return value_initialize(type, target, constructor, location);
		if (constructor == nullptr && list->facts != nullptr && (list->facts->flags & kInitializerList) != 0)
			return list_object(cls, target, *list, location);
		if (constructor == nullptr)
			return fail(location, "a list-initialization outside constant evaluation");
		return constructor_call(constructor, target, list->elements, location);
	}
	if (init->kind == syntax::InitializerKind::Parenthesized)
	{
		if (init->arguments.empty())
			return value_initialize(type, target, constructor, location);
		if (constructor != nullptr)
			return constructor_call(constructor, target, init->arguments, location);
		return materialize(*init->arguments[0], target);
	}
	if (init->kind != syntax::InitializerKind::Assign)
		return fail(location, "an initializer outside constant evaluation");
	if (constructor == nullptr)
	{
		const Conversion* conversion = RecordedConversion(init->expression);
		if (ConversionFunction(conversion))
			return initialize_from(*init->expression, conversion, target);
		return materialize(*init->expression, target);
	}
	syntax::Slice<syntax::Expr*> single;
	single.data = const_cast<syntax::Expr**>(&init->expression);
	single.size = 1;
	return constructor_call(constructor, target, single, location);
}

bool ConstantInterpreter::value_initialize(
	const Type* type, const ConstPlace& target, const FunctionEntity* constructor, uint32_t location)
{
	// 8.5p7: a user-provided default constructor runs; otherwise the object
	// is zero-initialized, then an implicit default constructor that is
	// not trivial runs; an array's elements each.
	if (type->kind == TypeKind::Array)
	{
		const uint64_t size = size_of(type->element, location);
		for (uint64_t i = 0; i < type->bound; ++i)
		{
			ConstPlace place = target;
			place.offset += static_cast<int64_t>(i * size);
			place.type = type->element;
			if (!value_initialize(type->element, place, constructor, location))
				return false;
		}
		return true;
	}
	if (constructor != nullptr && UserProvidedFunction(constructor))
		return constructor_call(constructor, target, syntax::Slice<syntax::Expr*>(), location);
	ConstValue* value = nullptr;
	if (!locate(target, true, value, location))
		return false;
	zero_into(*value);
	if (constructor != nullptr && !constructor->trivial)
		return constructor_call(constructor, target, syntax::Slice<syntax::Expr*>(), location);
	return true;
}

bool ConstantInterpreter::initialize_object(
	const Type* type, const ConstPlace& target, const syntax::Initializer& init, uint32_t location)
{
	// A non-class object (or reference) from its initializer: an array
	// element by element or from a string literal, a scalar from its
	// single expression converted, a reference bound.
	const syntax::BracedInitList* list = BracedList(init);
	const syntax::Expr* single = nullptr;
	if (list != nullptr)
	{
		if (type->kind == TypeKind::Array)
		{
			if (list->elements.size == 1 && WithoutParentheses(list->elements[0])->kind == syntax::ExprKind::Literal &&
				WithoutParentheses(list->elements[0])->facts->type->kind == TypeKind::Array)
				return string_array(type, target, *list->elements[0], location);
			return aggregate(type, target, *list, location);
		}
		if (list->elements.size == 2 && IsComplex(type))
		{
			// A complex value's list of its two parts.
			ConstValue value;
			return part_list(list->elements, type, value, location) && write(target, value, location);
		}
		if (type->unqualified->kind == TypeKind::Vector && !list->elements.empty())
		{
			ConstValue value;
			return lane_list(list->elements, type, value, location) && write(target, value, location);
		}
		if (list->elements.size > 1)
			return fail(location, "a scalar initialized from several values");
		single = list->elements.empty() ? nullptr : list->elements[0];
	}
	else if (init.kind == syntax::InitializerKind::Parenthesized)
	{
		if (init.arguments.size > 1)
			return fail(location, "a scalar initialized from several values");
		single = init.arguments.empty() ? nullptr : init.arguments[0];
		if (single == nullptr && type->kind == TypeKind::Array)
			return value_initialize(type, target, nullptr, location);
	}
	else if (init.kind == syntax::InitializerKind::Assign)
		single = init.expression;
	else
		return fail(location, "an initializer outside constant evaluation");
	if (type->kind == TypeKind::Array)
	{
		if (single != nullptr && WithoutParentheses(single)->kind == syntax::ExprKind::Literal)
			return string_array(type, target, *single, location);
		return fail(location, "an array initialized from an expression");
	}
	return initialize_scalar(type, target, single, location);
}

bool ConstantInterpreter::initialize_scalar(
	const Type* type, const ConstPlace& target, const syntax::Expr* single, uint32_t location, bool extend)
{
	ConstValue value = ConstValue();
	if (type->is_reference())
	{
		// A reference member's binding: the referent it designates. A
		// temporary bound to a reference member of an aggregate a variable
		// of static storage duration's initializer initializes persists with
		// the variable (12.2p5, `extend`), as a reference variable's does.
		ConstPlace referent;
		if (single == nullptr || !bind(*single, RecordedConversion(single), type, referent))
			return single != nullptr || fail(location, "a reference without an initializer");
		if (extend && referent.root == RootKind::Object && static_variable_place(target) &&
			binds_temporary(*single, RecordedConversion(single)) && !extend_temporary(referent, location))
			return false;
		value = PointerTo(referent, type);
	}
	else if (single == nullptr)
	{
		// `{}` or `()`: value-initialized, zero.
		value.type = type->unqualified;
		zero_into(value);
		value.flags = 0;
	}
	else if (!operand(*single, value) || !convert_scalar(value, type, location))
		return false;
	value.flags &= ~kValueZeroed;
	return write(target, value, location);
}

bool ConstantInterpreter::string_array(
	const Type* type, const ConstPlace& target, const syntax::Expr& literal, uint32_t location)
{
	// 8.5.2: the literal's code units and null, the rest of the array zero.
	const syntax::Expr* e = WithoutParentheses(&literal);
	const ConstValue* units = literal_object(static_cast<const syntax::LiteralExpr*>(e), location);
	ConstValue* array = nullptr;
	if (units == nullptr || !locate(target, true, array, location) || array->kind != ValueKind::Array)
		return false;
	for (uint32_t i = 0; i < array->count; ++i)
	{
		if (i < units->count)
			array->elements[i] = IntegerValue(type->element->unqualified, units->elements[i].bits);
		else
			array->elements[i] = IntegerValue(type->element->unqualified, 0);
	}
	return true;
}

bool ConstantInterpreter::list_object(
	const ClassEntity* entity, const ConstPlace& target, const syntax::BracedInitList& list, uint32_t location)
{
	// 8.5.4p5: the std::initializer_list object refers to the array `const
	// E[N]` its elements copy-initialize, a temporary of the evaluation
	// (8.5.4p6): the array's first element's address and its element count
	// in the object's first two members (the builtin layout's, or the
	// program's own class's), as the lowering stores them.
	const DataMember* first = entity->members;
	const DataMember* second = first != nullptr ? first->next : nullptr;
	if (second == nullptr || first->variable->kind != EntityKind::Variable ||
		second->variable->kind != EntityKind::Variable)
		return fail(location, "a std::initializer_list without its array's address and size members");
	const Type* array = list.facts->target;
	ConstPlace elements;
	if (!new_object(array, elements, location) || !aggregate(array, elements, list, location))
		return false;
	elements.type = array->element;
	const VariableEntity* members[2] = { static_cast<const VariableEntity*>(first->variable),
		static_cast<const VariableEntity*>(second->variable) };
	ConstPlace places[2];
	for (unsigned i = 0; i < 2; ++i)
	{
		if (!member_slot(target, members[i], places[i], location))
			return false;
	}
	return write(places[0], PointerTo(elements, members[0]->type->unqualified), location) &&
		write(places[1], IntegerValue(members[1]->type->unqualified, list.elements.size), location);
}

bool ConstantInterpreter::aggregate(
	const Type* type, const ConstPlace& target, const syntax::BracedInitList& list, uint32_t location)
{
	// 8.5.1p2: the elements initialize the members (array elements) in
	// order; brace elision takes a subaggregate's from the same list.
	uint32_t next = 0;
	if (type->kind == TypeKind::Array)
	{
		const uint64_t size = size_of(type->element, location);
		for (uint64_t i = 0; i < type->bound; ++i)
		{
			ConstPlace place = target;
			place.offset += static_cast<int64_t>(i * size);
			place.type = type->element;
			if (!aggregate_element(type->element, place, list, next, location))
				return false;
		}
	}
	else if (!aggregate_members(ClassOf(type), target, list, next, location))
		return false;
	return next >= list.elements.size || fail(location, "too many initializers for the aggregate");
}

bool ConstantInterpreter::aggregate_members(const ClassEntity* entity, const ConstPlace& target,
	const syntax::BracedInitList& list, uint32_t& next, uint32_t location)
{
	// Unnamed bit-fields take no element (9.6p2); a union's one member is
	// initialized (8.5.1p15, see UnionListMember).
	const DataMember* selected = entity->key == TokenKind::KW_UNION ? ListUnionMember(entity, &list, next) : nullptr;
	for (const DataMember* member = entity->members; member != nullptr; member = member->next)
	{
		if ((member->bit_field && member->variable->name == nullptr) || (selected != nullptr && member != selected))
			continue;
		if (member->variable->kind != EntityKind::Variable)
			continue;
		const VariableEntity* variable = static_cast<const VariableEntity*>(member->variable);
		ConstPlace place;
		if (!member_slot(target, variable, place, location) ||
			!aggregate_element(variable->type, place, list, next, location))
			return false;
		if (entity->key == TokenKind::KW_UNION)
			break;
	}
	return true;
}

bool ConstantInterpreter::aggregate_element(
	const Type* type, const ConstPlace& target, const syntax::BracedInitList& list, uint32_t& next, uint32_t location)
{
	// One member or element from the list at `next` (as the analysis's
	// aggregate_element reads it); without an element it is value-
	// initialized (8.5.1p7).
	if (next >= list.elements.size)
	{
		if (type->is_reference())
			return fail(location, "a reference member without an initializer");
		const Type* object = type;
		while (object->kind == TypeKind::Array)
			object = object->element;
		const ClassEntity* cls = ClassOf(object);
		if (cls != nullptr && IsAggregateClass(cls))
		{
			if (type->kind == TypeKind::Array)
			{
				ConstValue* value = nullptr;
				if (!locate(target, true, value, location))
					return false;
				zero_into(*value);
				return true;
			}
			syntax::BracedInitList empty = syntax::BracedInitList();
			uint32_t none = 0;
			return aggregate_members(cls, target, empty, none, location);
		}
		return value_initialize(type, target,
			cls != nullptr ? analyzer_.implicit_constructor(const_cast<ClassEntity*>(cls)) : nullptr, location);
	}
	const syntax::Expr& element = *list.elements[next];
	if (element.kind == syntax::ExprKind::BracedInit)
	{
		++next;
		const syntax::BracedInitList& inner = static_cast<const syntax::BracedInitList&>(element);
		syntax::Initializer view = syntax::Initializer();
		view.kind = syntax::InitializerKind::Braced;
		view.expression = const_cast<syntax::BracedInitList*>(&inner);
		view.location = inner.location;
		const ClassEntity* cls = type->is_reference() || type->kind == TypeKind::Array ? nullptr : ClassOf(type);
		const FunctionEntity* constructor = cls != nullptr && inner.facts != nullptr &&
				inner.facts->entity != nullptr && inner.facts->entity->kind == EntityKind::Function
			? static_cast<const FunctionEntity*>(inner.facts->entity)
			: nullptr;
		return construct(type, target, &view, constructor, inner.location);
	}
	if (IsSubaggregate(type) && !InitializesSubaggregate(type, element))
	{
		if (type->kind == TypeKind::Array)
		{
			const uint64_t size = size_of(type->element, location);
			for (uint64_t i = 0; i < type->bound; ++i)
			{
				ConstPlace place = target;
				place.offset += static_cast<int64_t>(i * size);
				place.type = type->element;
				if (!aggregate_element(type->element, place, list, next, location))
					return false;
			}
			return true;
		}
		return aggregate_members(ClassOf(type), target, list, next, location);
	}
	++next;
	if (!type->is_reference() && type->kind == TypeKind::Array)
		return string_array(type, target, element, location);
	if (!type->is_reference() && ClassOf(type) != nullptr)
		return initialize_from(element, RecordedConversion(&element), target);
	return initialize_scalar(type, target, &element, location, true);
}

// -------------------------------------------------------------- constructors

bool ConstantInterpreter::constructor_call(const FunctionEntity* constructor, const ConstPlace& target,
	const syntax::Slice<syntax::Expr*>& written, uint32_t location)
{
	SmallList<Argument, 4> arguments;
	for (const syntax::Expr* expr : written)
	{
		Argument argument = Argument();
		argument.expr = expr;
		argument.conversion = WithoutParentheses(expr)->facts != nullptr ? WithoutParentheses(expr)->facts->conversion
																		 : nullptr;
		arguments.push_back(argument);
	}
	return run_constructor(constructor, target, arguments.data(), static_cast<uint32_t>(arguments.size()), location);
}

bool ConstantInterpreter::run_constructor(const FunctionEntity* constructor, const ConstPlace& target,
	const Argument* arguments, uint32_t count, uint32_t location)
{
	// 12.6.2 as 7.1.5p4 constrains a constexpr constructor: the subobjects'
	// initializations (or the target constructor's), then the body; a
	// trivial copy or move is the source's value.
	if (CopiesStorage(constructor) && count == 1)
	{
		ConstPlace source;
		if (arguments[0].bound)
			source = arguments[0].place;
		else
		{
			const syntax::Expr* e = WithoutParentheses(arguments[0].expr);
			if (e->facts->category == ValueCategory::PRValue &&
				ClassOf(e->facts->type) == ClassOf(constructor->owner->entity->type))
				return materialize(*e, target);
			if (!bind(*arguments[0].expr, arguments[0].conversion, constructor->type->canonical->params[0], source))
				return false;
		}
		ConstValue* value = nullptr;
		return locate(source, false, value, location) && write(target, *value, location);
	}
	if (constructor->copy_kind != CopyKind::None && (!UserProvidedFunction(constructor) || constructor->synthesized) &&
		count >= 1)
	{
		// An implicitly defined copy or move constructor: each subobject
		// from the source's (12.8p15).
		ConstPlace source;
		return argument_place(arguments[0], constructor->type->canonical->params[0], source, location) &&
			copy_subobjects(constructor, target, source, location);
	}
	if (!evaluable(constructor, location))
		return false;
	// 5.19p2: a constexpr constructor for a literal class; the object a
	// constant initialization initializes may be of any class (3.6.2p2).
	const ClassEntity* home = ClassOf(constructor->owner->entity->type);
	const bool initialized = target.root == RootKind::Variable && target.entity == constructing_ &&
		target.offset == 0 && ClassOf(target.type) == home;
	if (!initialized && !analyzer_.literal_type(home->type))
		return fail(location, "a constructor of a class that is not a literal type in a constant expression");
	// The arguments' temporaries end with the caller's full-expression,
	// the parameters when the constructor returns (5.2.2p4), the callee's
	// own objects with the call.
	const size_t locals = locals_.size();
	if (!bind_parameters(constructor, arguments, count, location))
		return false;
	Region callee = region();
	callee.locals = locals;
	Frame frame = Frame();
	frame.function = constructor;
	frame.has_this = true;
	frame.self = target;
	frame.first_local = locals;
	frame.result = nullptr;
	frames_.push_back(frame);
	const size_t swaps = suspend_swaps();
	bool done = false;
	const ClassEntity* cls = ClassOf(constructor->owner->entity->type);
	if (constructor->delegation != nullptr)
	{
		// 12.6.2p6: the target constructor on the object.
		syntax::Initializer view = syntax::Initializer();
		done = construct(
			cls->type, target, MemInitializerView(constructor->delegation, view), constructor->delegate, location);
	}
	else
		done = subobjects(constructor, target, location);
	if (done && constructor->body != nullptr && constructor->body->body != nullptr)
	{
		Result result = Result();
		frames_.back().result = &result;
		done = body(constructor, result, location);
	}
	resume_swaps(swaps);
	frames_.pop_back();
	end_parameters(constructor, locals);
	release(callee);
	return done;
}

bool ConstantInterpreter::argument_place(
	const Argument& argument, const Type* parameter, ConstPlace& out, uint32_t location)
{
	// The object a reference parameter binds.
	if (argument.bound)
	{
		out = argument.place;
		return true;
	}
	if (argument.expr == nullptr)
		return fail(location, "a call without an argument for a parameter");
	return bind(*argument.expr, argument.conversion, parameter, out);
}

bool ConstantInterpreter::copy_subobjects(
	const FunctionEntity* function, const ConstPlace& target, const ConstPlace& source, uint32_t location)
{
	// An implicitly defined copy or move constructor or assignment
	// operator: each base and member from the source's by the function the
	// analysis selected for it, a scalar's value copied (12.8p15, p28).
	for (uint32_t i = 0; i < function->init_count; ++i)
	{
		const SubobjectInit& init = function->inits[i];
		ConstPlace to = target, from = source;
		if (init.base != nullptr)
		{
			const ClassEntity* base = static_cast<const ClassEntity*>(init.base->base);
			if (!to_base(to, base, location) || !to_base(from, base, location))
				return false;
		}
		else if (init.member != nullptr && init.member->variable->kind == EntityKind::Variable)
		{
			const VariableEntity* member = static_cast<const VariableEntity*>(init.member->variable);
			if (!member_slot(target, member, to, location) || !member_slot(source, member, from, location))
				return false;
		}
		else
			continue;
		const FunctionEntity* copy = init.constructor;
		if (copy == nullptr || copy->trivial)
		{
			ConstValue* value = nullptr;
			if (!locate(from, false, value, location) || !write(to, *value, location))
				return false;
			continue;
		}
		Argument argument = Argument();
		argument.bound = true;
		argument.place = from;
		Result result = Result();
		if (copy->constructor ? !run_constructor(copy, to, &argument, 1, location)
							  : !invoke(copy, &to, &argument, 1, result, location))
			return false;
	}
	return true;
}

bool ConstantInterpreter::subobjects(const FunctionEntity* constructor, const ConstPlace& self, uint32_t location)
{
	// 12.6.2p10: the direct bases, then the members in declaration order,
	// each by its mem-initializer, brace-or-equal-initializer or
	// default-initialization.
	for (uint32_t i = 0; i < constructor->init_count; ++i)
	{
		const SubobjectInit& init = constructor->inits[i];
		if (init.base == nullptr)
		{
			if (!member_initialization(init, self, location))
				return false;
			continue;
		}
		const ClassEntity* base = static_cast<const ClassEntity*>(init.base->base);
		ConstPlace place = self;
		if (!to_base(place, base, location))
			return false;
		if (init.forwards)
		{
			// 12.9p8: the inherited constructor with this one's parameters.
			SmallList<Argument, 4> forwarded;
			const size_t first = frames_.back().first_local;
			for (size_t j = first; j < locals_.size(); ++j)
			{
				Argument argument = Argument();
				argument.bound = true;
				argument.place = locals_[j].place;
				forwarded.push_back(argument);
			}
			if (!run_constructor(
					init.constructor, place, forwarded.data(), static_cast<uint32_t>(forwarded.size()), location))
				return false;
			continue;
		}
		syntax::Initializer view = syntax::Initializer();
		const syntax::Initializer* written = MemInitializerView(init.mem_initializer, view);
		if (written == nullptr && (init.constructor == nullptr || init.constructor->trivial))
			continue;
		if (!construct(base->type, place, written, init.constructor, location))
			return false;
	}
	return true;
}

bool ConstantInterpreter::member_initialization(const SubobjectInit& init, const ConstPlace& self, uint32_t location)
{
	// A member by its mem-initializer or brace-or-equal-initializer (in the
	// constructor's frame, `this` its object); an anonymous union's or
	// struct's members through their own plans (9.5p5); a scalar member
	// without one is left indeterminate.
	if (init.inner != nullptr)
	{
		for (uint32_t i = 0; i < init.inner_count; ++i)
		{
			if (!member_initialization(init.inner[i], self, location))
				return false;
		}
		return true;
	}
	const DataMember* member = init.member;
	if (member == nullptr || member->variable->kind != EntityKind::Variable || member->variable->name == nullptr)
		return true;
	const VariableEntity* variable = static_cast<const VariableEntity*>(member->variable);
	syntax::Initializer view = syntax::Initializer();
	const syntax::Initializer* written = init.mem_initializer != nullptr
		? MemInitializerView(init.mem_initializer, view)
		: init.default_initializer;
	if (written == nullptr && (init.constructor == nullptr || init.constructor->trivial))
		return true;
	// A brace-or-equal-initializer's source location builtins report the
	// constructor's body, where it runs (see Intrinsic::SourceFile).
	const FunctionEntity* constructor = frames_.size() > first_frame_ ? frames_.back().function : nullptr;
	const SourceSite site(source_site_,
		init.mem_initializer == nullptr && constructor != nullptr && constructor->body != nullptr &&
				constructor->body->body != nullptr
			? constructor->body->body->location
			: kNoSourceSite);
	ConstPlace place;
	if (!member_slot(self, variable, place, location))
		return false;
	// A union's member becomes its active one as its initialization begins
	// (7.1.5p4, 9.5p1): its own subobjects' writes then land in it, not in
	// an earlier member sharing their offset and type.
	ConstValue* activated = nullptr;
	if (static_cast<const ClassEntity*>(variable->owner->entity)->key == TokenKind::KW_UNION &&
		!variable->type->is_reference() && !locate(place, true, activated, location))
		return false;
	return construct(variable->type, place, written, init.constructor, location);
}

} // namespace sema
