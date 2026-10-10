#include "sema/constant_interpreter.h"

#include <cmath>
#include <cstring>
#include <string>

#include "lex/number_literal.h"
#include "lex/text_literal.h"
#include "sema/analyzer.h"
#include "sema/integral_arithmetic.h"
#include "sema/statement_facts.h"
#include "support/small_list.h"

// Calls and function bodies in constant evaluation (5.19p2, 7.1.5): the
// function the analysis selected is invoked with its parameters bound to
// the arguments as their recorded conversions initialize them, and its
// body's statements are executed (C++14's local variables, assignments,
// branches and loops included).
namespace sema {

namespace {

const size_t kCallDepth = 512;

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

// A function with a body of its own, not an implicit (or defaulted,
// 8.4.2p5) one's definition run by its plan.
bool UserProvidedFunction(const FunctionEntity* function)
{
	return !function->implicit && !function->defaulted && !function->synthesized;
}

bool IsAssignment(CopyKind kind)
{
	return kind == CopyKind::CopyAssignment || kind == CopyKind::MoveAssignment;
}

// The case or default label a switch on `value` transfers to (6.4.2p5),
// searched through the statements of its body outside nested switches.
const syntax::Stmt* SwitchTarget(
	const syntax::Stmt* stmt, uint64_t value, const syntax::Stmt*& fallback, uint32_t depth)
{
	if (stmt == nullptr || depth > 256)
		return nullptr;
	switch (stmt->kind)
	{
	case syntax::StmtKind::Compound:
		for (const syntax::Stmt* item : static_cast<const syntax::CompoundStmt*>(stmt)->items)
		{
			if (const syntax::Stmt* found = SwitchTarget(item, value, fallback, depth + 1))
				return found;
		}
		return nullptr;
	case syntax::StmtKind::Case:
	{
		const syntax::CaseStmt* label = static_cast<const syntax::CaseStmt*>(stmt);
		if (label->converted == value)
			return stmt;
		return SwitchTarget(label->body, value, fallback, depth + 1);
	}
	case syntax::StmtKind::Default:
		if (fallback == nullptr)
			fallback = stmt;
		return SwitchTarget(static_cast<const syntax::LabeledStmt*>(stmt)->body, value, fallback, depth + 1);
	case syntax::StmtKind::Labeled:
		return SwitchTarget(static_cast<const syntax::LabeledStmt*>(stmt)->body, value, fallback, depth + 1);
	default:
		return nullptr;
	}
}

// A declaration statement, labeled or not: its variables stay in scope
// after it, to the end of the block (3.3.3p1).
bool Declares(const syntax::Stmt* stmt)
{
	while (stmt->kind == syntax::StmtKind::Case || stmt->kind == syntax::StmtKind::Default ||
		stmt->kind == syntax::StmtKind::Labeled)
		stmt = stmt->kind == syntax::StmtKind::Case ? static_cast<const syntax::CaseStmt*>(stmt)->body
													: static_cast<const syntax::LabeledStmt*>(stmt)->body;
	return syntax::ChosenReading(*stmt).kind == syntax::StmtKind::Declaration;
}

} // namespace

// ------------------------------------------------------------------ calls

bool ConstantInterpreter::call(const syntax::CallExpr& expr, Result& result)
{
	// 5.2.2: the function the analysis selected (a member function on its
	// object, the object of a class's operator()), or the one a function
	// pointer designates, with the arguments initializing its parameters.
	const ExprFacts* facts = expr.facts;
	const syntax::Expr* callee = WithoutParentheses(expr.callee);
	if ((facts->flags & kPseudoDestructor) != 0)
		return fail(expr.location, "a pseudo-destructor call in a constant expression");
	SmallList<Argument, 4> arguments;
	for (const syntax::Expr* written : expr.arguments)
	{
		Argument argument = Argument();
		argument.expr = written;
		argument.conversion = WithoutParentheses(written)->facts != nullptr
			? WithoutParentheses(written)->facts->conversion
			: nullptr;
		arguments.push_back(argument);
	}
	const uint32_t count = static_cast<uint32_t>(arguments.size());
	if ((facts->flags & kDirectCall) != 0)
	{
		const FunctionEntity* function = static_cast<const FunctionEntity*>(facts->entity);
		if ((facts->flags & kVirtualCall) != 0)
			return fail(expr.location, "a virtual function call in a constant expression");
		ConstPlace object;
		const bool member = function->member && !function->is_static;
		if (member)
		{
			const ClassEntity* home = ClassOf(function->owner->entity->type);
			bool located = false;
			if ((facts->flags & kOperatorCall) != 0)
				located = object_place(*callee, object);
			else if ((callee->facts->flags & kImplicitThis) != 0)
				located = this_place(object, expr.location) &&
					through_naming(object, analyzer_.call_naming_class(expr), expr.location);
			else if (callee->kind == syntax::ExprKind::Member)
			{
				const syntax::MemberExpr& access = static_cast<const syntax::MemberExpr&>(*callee);
				if (access.op == TokenKind::OP_ARROW)
				{
					ConstValue pointer;
					located = rvalue(*access.base, pointer);
					if (located && (pointer.kind != ValueKind::Pointer || pointer.root == RootKind::Null))
						return fail(expr.location, "a member function called through a null pointer");
					if (located)
						object = PlaceOf(pointer, pointer.type->element);
				}
				else
					located = object_place(*access.base, object);
				located = located && through_naming(object, analyzer_.member_naming_class(access), expr.location);
			}
			else
				return fail(expr.location, "a member function call without its object");
			if (!located || (ClassOf(object.type) != home && !to_base(object, home, expr.location)))
				return false;
		}
		else if (callee->kind == syntax::ExprKind::Member &&
			!member_object(static_cast<const syntax::MemberExpr&>(*callee)))
			return false;
		if (function->destructor && function->trivial)
			return true;
		return invoke(function, member ? &object : nullptr, arguments.data(), count, result, expr.location);
	}
	if ((callee->facts->flags & kBoundMember) != 0)
	{
		// 5.5p6: through a pointer to member function, on the subobject of
		// the function's class it designates (a virtual function is no
		// constexpr function, 7.1.5p3).
		if (callee->kind != syntax::ExprKind::Binary)
			return fail(expr.location, "a bound member function call outside constant evaluation");
		ConstValue pointer;
		ConstPlace object;
		if (!member_pointer_object(static_cast<const syntax::BinaryExpr&>(*callee), pointer, object))
			return false;
		const FunctionEntity* function = static_cast<const FunctionEntity*>(pointer.address.entity);
		if (function->is_virtual)
			return fail(expr.location, "a virtual function call in a constant expression");
		return invoke(function, &object, arguments.data(), count, result, expr.location);
	}
	// An indirect call: the function a function lvalue or pointer designates.
	ConstPlace place;
	if (callee->facts->type->kind == TypeKind::Function)
	{
		if (!glvalue(*callee, place))
			return false;
	}
	else
	{
		ConstValue pointer;
		if (!operand(*callee, pointer))
			return false;
		if (pointer.kind != ValueKind::Pointer || pointer.root != RootKind::Function)
			return fail(expr.location, "a call through a pointer that designates no function");
		place = PlaceOf(pointer, pointer.type->element);
	}
	if (place.root != RootKind::Function)
		return fail(expr.location, "a call of an object that is no function");
	const FunctionEntity* function = static_cast<const FunctionEntity*>(place.entity);
	if (function->member && !function->is_static)
		return fail(expr.location, "a member function called without its object");
	return invoke(function, nullptr, arguments.data(), count, result, expr.location);
}

bool ConstantInterpreter::operator_call(const syntax::Expr& expr, Result& result)
{
	// 13.3.1.2: the operator function the analysis selected, its operands
	// the call's arguments (a member's object first); a postfix operator's
	// int parameter takes zero (13.5.7).
	const syntax::Expr* e = WithoutParentheses(&expr);
	const FunctionEntity* function = static_cast<const FunctionEntity*>(e->facts->entity);
	if ((e->facts->flags & kUserLiteral) != 0)
		return e->kind == syntax::ExprKind::Literal
			? user_literal(static_cast<const syntax::LiteralExpr&>(*e), result)
			: fail(e->location, "a user-defined literal outside constant evaluation");
	const syntax::Expr* operands[2] = { nullptr, nullptr };
	uint32_t count = 0;
	bool postfix = false;
	switch (e->kind)
	{
	case syntax::ExprKind::Binary:
	case syntax::ExprKind::Assign:
		operands[0] = static_cast<const syntax::BinaryExpr*>(e)->left;
		operands[1] = static_cast<const syntax::BinaryExpr*>(e)->right;
		count = 2;
		break;
	case syntax::ExprKind::Subscript:
		operands[0] = static_cast<const syntax::SubscriptExpr*>(e)->base;
		operands[1] = static_cast<const syntax::SubscriptExpr*>(e)->index;
		count = 2;
		break;
	case syntax::ExprKind::Postfix:
		postfix = true;
		// fall through
	case syntax::ExprKind::Unary:
		operands[0] = static_cast<const syntax::UnaryExpr*>(e)->operand;
		count = 1;
		break;
	case syntax::ExprKind::Call:
		return call(static_cast<const syntax::CallExpr&>(*e), result);
	default:
		return fail(e->location, "an operator call outside constant evaluation");
	}
	const bool member = function->member && !function->is_static;
	ConstPlace object;
	uint32_t first = 0;
	if (member)
	{
		const ClassEntity* home = ClassOf(function->owner->entity->type);
		if (!object_place(*operands[0], object) ||
			(ClassOf(object.type) != home && !to_base(object, home, e->location)))
			return false;
		first = 1;
	}
	SmallList<Argument, 3> arguments;
	for (uint32_t i = first; i < count; ++i)
	{
		Argument argument = Argument();
		argument.expr = operands[i];
		argument.conversion = WithoutParentheses(operands[i])->facts->conversion;
		arguments.push_back(argument);
	}
	if (postfix)
	{
		Argument zero = Argument();
		zero.valued = true;
		zero.value = IntegerValue(analyzer_.types_.fundamental(FundamentalType::Int), 0);
		arguments.push_back(zero);
	}
	return invoke(function, member ? &object : nullptr, arguments.data(), static_cast<uint32_t>(arguments.size()),
		result, e->location);
}

bool ConstantInterpreter::user_literal(const syntax::LiteralExpr& expr, Result& result)
{
	// 2.14.8: the literal operator the analysis selected, called with the
	// literal's array (its first code unit's address) and length (p5), with
	// its number's value as an unsigned long long or long double (p3-p4),
	// or without arguments (a literal operator template's specialization).
	const FunctionEntity* function = static_cast<const FunctionEntity*>(expr.facts->entity);
	const Type* const* parameters = function->type->canonical->params;
	SmallList<Argument, 2> arguments;
	Argument argument = Argument();
	argument.valued = true;
	if (expr.facts->value == kLiteralCallString)
	{
		const ConstValue* units = literal_object(&expr, expr.location);
		if (units == nullptr)
			return false;
		ConstPlace first = ConstPlace();
		first.root = RootKind::Literal;
		first.entity = &expr;
		first.type = units->type->element;
		argument.value = PointerTo(first, parameters[0]->unqualified);
		arguments.push_back(argument);
		argument.value = IntegerValue(parameters[1]->unqualified, units->count - 1);
		arguments.push_back(argument);
	}
	else if (expr.facts->value == kLiteralCallRaw)
	{
		// p3-p4: the number's characters, a string literal's array.
		const ConstValue* units = literal_object(&expr, expr.location);
		if (units == nullptr)
			return false;
		ConstPlace first = ConstPlace();
		first.root = RootKind::Literal;
		first.entity = &expr;
		first.type = units->type->element;
		argument.value = PointerTo(first, parameters[0]->unqualified);
		arguments.push_back(argument);
	}
	else if (expr.facts->value == kLiteralCallCharacter)
	{
		// p6: the character's value converted to the parameter.
		const CharacterLiteralValue character = EvaluateCharacterLiteral(expr.spelling.text, expr.spelling.length);
		const Type* type = analyzer_.types_.fundamental(character.type);
		IntegralKind kind;
		IntegralKindOf(type, kind);
		argument.value = IntegerValue(type, ConvertIntegral(character.code_point, kind));
		if (!convert_scalar(argument.value, parameters[0], expr.location))
			return false;
		arguments.push_back(argument);
	}
	else if (expr.facts->value == kLiteralCallCooked)
	{
		const NumberLiteral number = ClassifyNumber(expr.spelling.text, expr.spelling.length);
		uint64_t digits = 0;
		if (number.kind == NumberLiteralKind::UserDefinedInteger)
		{
			if (!UserIntegerValue(expr.spelling.text, number.suffix_begin, digits))
				return fail(expr.location, "a user-defined integer literal beyond unsigned long long");
			argument.value = IntegerValue(parameters[0]->unqualified, digits);
		}
		else
		{
			unsigned char bytes[16];
			std::string scratch;
			long double value = 0;
			if (!ScanFloatingValue(
					expr.spelling.text, number.suffix_begin, FundamentalType::LongDouble, scratch, bytes))
				return fail(expr.location, "a floating literal outside constant evaluation");
			std::memcpy(&value, bytes, sizeof value);
			argument.value = FloatingValue(parameters[0]->unqualified, value);
		}
		arguments.push_back(argument);
	}
	return invoke(function, nullptr, arguments.data(), static_cast<uint32_t>(arguments.size()), result, expr.location);
}

bool ConstantInterpreter::conversion_call(
	const syntax::Expr& expr, const ExprFacts* facts, const FunctionEntity* function, Result& result)
{
	// A conversion function (12.3.2) called on the operand's object: the
	// operand evaluated with its own facts, converted to the function's
	// class (13.3.1p4).
	const syntax::Expr* e = WithoutParentheses(&expr);
	stacks_.swaps.push_back(Swap{ e, e->facts, facts });
	e->facts = facts;
	ConstPlace object;
	const bool located = object_place(*e, object);
	e->facts = stacks_.swaps.back().converted;
	stacks_.swaps.pop_back();
	const ClassEntity* home = ClassOf(function->owner->entity->type);
	if (!located || (ClassOf(object.type) != home && !to_base(object, home, e->location)))
		return false;
	if (function->is_virtual)
		return fail(e->location, "a virtual conversion function in a constant expression");
	return invoke(function, &object, nullptr, 0, result, e->location);
}

bool ConstantInterpreter::call_value(const syntax::Expr& expr, ConstValue& out)
{
	// A call's scalar value (a reference result's referent read).
	const syntax::Expr* e = WithoutParentheses(&expr);
	Result result = Result();
	const bool called = e->kind == syntax::ExprKind::Call && (e->facts->flags & kUserLiteral) == 0
		? call(static_cast<const syntax::CallExpr&>(*e), result)
		: operator_call(*e, result);
	if (!called)
		return false;
	if (e->facts->category != ValueCategory::PRValue)
		return read(result.place, out, e->location);
	out = result.value;
	return true;
}

bool ConstantInterpreter::call_place(const syntax::Expr& expr, ConstPlace& out)
{
	// A call returning a reference designates its referent (5.2.2p10).
	const syntax::Expr* e = WithoutParentheses(&expr);
	Result result = Result();
	if (e->facts->category == ValueCategory::PRValue && ClassOf(e->facts->type) != nullptr)
		return temporary(*e, out);
	const bool called = e->kind == syntax::ExprKind::Call ? call(static_cast<const syntax::CallExpr&>(*e), result)
														  : operator_call(*e, result);
	if (!called)
		return false;
	out = result.place;
	return true;
}

bool ConstantInterpreter::evaluable(const FunctionEntity* function, uint32_t location)
{
	// 5.19p2: a constexpr function or constructor, with a definition; an
	// implicitly defined special member function is constexpr when its
	// actions are (12.1p6, 12.8p13), which evaluating them checks.
	if (function->deleted)
		return fail(location, "a call of a deleted function");
	const bool implicit_member = function->implicit || (function->defaulted && function->member);
	if (!function->is_constexpr && !implicit_member)
		return fail(location, "a call of a function that is not constexpr");
	if (frames_.size() - first_frame_ >= kCallDepth)
		return fail(location, "constexpr calls nested too deeply");
	if (!analyzer_.constexpr_definition(function))
		return fail(location, "a constexpr function called before its definition");
	return true;
}

bool ConstantInterpreter::invoke(const FunctionEntity* function, const ConstPlace* object, const Argument* arguments,
	uint32_t count, Result& result, uint32_t location)
{
	if (!step(location))
		return false;
	if (function->builtin != BuiltinFunction::None)
		return builtin(function, arguments, count, result, location);
	if (function->constructor)
	{
		if (result.target == nullptr)
			return fail(location, "a constructor call without its object");
		return run_constructor(function, *result.target, arguments, count, location);
	}
	if (IsAssignment(function->copy_kind) && !UserProvidedFunction(function) && object != nullptr && count >= 1)
	{
		// An implicitly defined copy or move assignment operator (12.8p28):
		// each subobject assigned, the object the result.
		ConstPlace source;
		if (!argument_place(arguments[0], function->type->canonical->params[0], source, location) ||
			!copy_subobjects(function, *object, source, location))
			return false;
		result.place = *object;
		return true;
	}
	if (!evaluable(function, location))
		return false;
	// The arguments' temporaries and the parameter objects end with the
	// caller's full-expression (5.2.2p4 leaves a parameter's end to the
	// implementation); the callee's own objects with the call.
	const size_t locals = locals_.size();
	if (!bind_parameters(function, arguments, count, location))
		return false;
	// A complex or vector result's parts or lanes outlive the callee's
	// storage: made in the caller's before it.
	const Type* returns = function->type->canonical->element;
	const uint32_t held = returns->is_reference() ? 0
		: IsComplex(returns)                      ? 2
		: LaneVector(returns)                     ? static_cast<uint32_t>(returns->unqualified->bound)
												  : 0;
	result.held = held != 0 ? arena_.allocate_array<ConstValue>(held) : nullptr;
	Region callee = region();
	callee.locals = locals;
	Frame frame = Frame();
	frame.function = function;
	frame.has_this = object != nullptr;
	if (object != nullptr)
		frame.self = *object;
	frame.first_local = locals;
	frame.result = &result;
	frames_.push_back(frame);
	const size_t swaps = suspend_swaps();
	const bool done = body(function, result, location);
	resume_swaps(swaps);
	frames_.pop_back();
	end_parameters(function, locals);
	release(callee);
	return done && valid_pointer(result.value, location);
}

bool ConstantInterpreter::builtin(
	const FunctionEntity* function, const Argument* arguments, uint32_t count, Result& result, uint32_t location)
{
	// GCC's builtins with constant results: __builtin_expect's first
	// argument; __builtin_strlen of a constant string; the absolute values.
	const Type* type = function->type->canonical;
	switch (function->builtin)
	{
	case BuiltinFunction::Expect:
	{
		ConstValue value, expected;
		if (count != 2 || !converted(*arguments[0].expr, arguments[0].conversion, value) ||
			!converted(*arguments[1].expr, arguments[1].conversion, expected) ||
			!convert_scalar(value, type->element, location))
			return false;
		result.value = value;
		return true;
	}
	case BuiltinFunction::Strlen:
	{
		ConstValue pointer;
		if (count != 1 || !converted(*arguments[0].expr, arguments[0].conversion, pointer) ||
			pointer.kind != ValueKind::Pointer)
			return false;
		uint64_t length = 0;
		for (;;)
		{
			ConstValue unit;
			if (!step(location) || !read(PlaceOf(pointer, pointer.type->element), unit, location))
				return false;
			if (unit.bits == 0)
				break;
			++length;
			if (!advance(pointer, 1, location))
				return false;
		}
		result.value = IntegerValue(type->element->unqualified, length);
		return true;
	}
	case BuiltinFunction::Library:
	{
		const char* name = function->label;
		if (library_comparison(function, arguments, count, result, location) ||
			library_floating(function, arguments, count, result, location))
			return failure_ == nullptr;
		// GCC folds __builtin_abs, labs, llabs and fabs[f|l] of a constant
		// (the most negative integer's absolute value overflows, 5p4).
		const bool integer = std::strcmp(name, "__builtin_abs") == 0 || std::strcmp(name, "__builtin_labs") == 0 ||
			std::strcmp(name, "__builtin_llabs") == 0;
		const bool floating = std::strncmp(name, "__builtin_fabs", 14) == 0;
		ConstValue value;
		if ((!integer && !floating) || count != 1 || !converted(*arguments[0].expr, arguments[0].conversion, value))
			return fail(location, "a builtin function that is not a constant expression");
		const Type* result_type = type->element->unqualified;
		if (floating && value.kind == ValueKind::Floating)
		{
			result.value = FloatingValue(result_type, std::fabs(value.floating));
			return true;
		}
		IntegralKind kind;
		if (!integer || value.kind != ValueKind::Integer || !IntegralKindOf(result_type, kind))
			return fail(location, "an absolute value of no arithmetic constant");
		result.value = IntegerValue(result_type, value.bits);
		if (static_cast<int64_t>(value.bits) < 0 &&
			!IntegralOperation(TokenKind::OP_MINUS, kind, 0, value.bits, result.value.bits))
			return fail(location, "an absolute value that overflows");
		return true;
	}
	default:
		return fail(location, "a builtin function that is not a constant expression");
	}
}

bool ConstantInterpreter::library_comparison(
	const FunctionEntity* function, const Argument* arguments, uint32_t count, Result& result, uint32_t location)
{
	// __builtin_strcmp and __builtin_strncmp of constant strings, which
	// both host compilers fold: the code units compared as unsigned char
	// up to the first difference or null (or n), the result's sign theirs.
	// False when the call is neither (`result` untouched).
	const char* name = function->label;
	const bool bounded = std::strcmp(name, "__builtin_strncmp") == 0;
	if (!bounded && std::strcmp(name, "__builtin_strcmp") != 0)
		return false;
	ConstValue left, right, limit;
	if (count != (bounded ? 3u : 2u) || !converted(*arguments[0].expr, arguments[0].conversion, left) ||
		!converted(*arguments[1].expr, arguments[1].conversion, right) ||
		(bounded && !converted(*arguments[2].expr, arguments[2].conversion, limit)))
		return true;
	if (left.kind != ValueKind::Pointer || right.kind != ValueKind::Pointer)
		return fail(location, "a string comparison of no constant strings"), true;
	int64_t answer = 0;
	for (uint64_t i = 0; !bounded || i < limit.bits; ++i)
	{
		ConstValue a, b;
		if (!step(location) || !read(PlaceOf(left, left.type->element), a, location) ||
			!read(PlaceOf(right, right.type->element), b, location))
			return true;
		const uint64_t x = a.bits & 0xff, y = b.bits & 0xff;
		if (x != y || x == 0)
		{
			answer = x < y ? -1 : x > y ? 1 : 0;
			break;
		}
		if (!advance(left, 1, location) || !advance(right, 1, location))
			return true;
	}
	result.value = IntegerValue(function->type->canonical->element->unqualified,
		ConvertIntegral(static_cast<uint64_t>(answer), IntegralKind{ 32, true }));
	return true;
}

bool ConstantInterpreter::library_floating(
	const FunctionEntity* function, const Argument* arguments, uint32_t count, Result& result, uint32_t location)
{
	// GCC folds __builtin_copysign, fmax and fmin[f|l] of constants
	// (C99 7.12.11.1, 7.12.12.2-3: a NaN operand of fmax or fmin yields
	// the other). False when the call is none of them.
	const char* name = function->label;
	if (std::strncmp(name, "__builtin_", 10) != 0)
		return false;
	name += 10;
	const size_t length = std::strlen(name);
	const size_t base = length != 0 && (name[length - 1] == 'f' || name[length - 1] == 'l') &&
			std::strncmp(name, "copysign", 8) != 0
		? length - 1
		: length;
	const bool copysign = std::strncmp(name, "copysign", 8) == 0 && (length == 8 || length == 9);
	const bool maximum = base == 4 && std::strncmp(name, "fmax", 4) == 0;
	const bool minimum = base == 4 && std::strncmp(name, "fmin", 4) == 0;
	if (!copysign && !maximum && !minimum)
		return false;
	ConstValue x, y;
	if (count != 2 || !converted(*arguments[0].expr, arguments[0].conversion, x) ||
		!converted(*arguments[1].expr, arguments[1].conversion, y))
		return true;
	if (x.kind != ValueKind::Floating || y.kind != ValueKind::Floating)
		return fail(location, "a floating builtin of no floating constants"), true;
	const long double value = copysign ? std::copysign(x.floating, y.floating)
		: maximum                      ? std::fmax(x.floating, y.floating)
									   : std::fmin(x.floating, y.floating);
	result.value = FloatingValue(function->type->canonical->element->unqualified, value);
	return true;
}

bool ConstantInterpreter::bind_parameters(
	const FunctionEntity* function, const Argument* arguments, uint32_t count, uint32_t location)
{
	// 5.2.2p4: each parameter initialized from its argument (a missing one
	// from the default argument, 8.3.6), evaluated in the caller's frame
	// before the callee's names are bound.
	const Type* type = function->type->canonical;
	if (count > type->param_count && !type->variadic)
		return fail(location, "a call with more arguments than parameters");
	// An argument matched by `...` (5.2.2p7) is evaluated, its value
	// unused: a constexpr function reads none (7.1.5p3 allows the
	// ellipsis, and no va_arg is a constant expression).
	for (uint32_t i = type->param_count; i < count; ++i)
	{
		const Argument& extra = arguments[i];
		if (extra.valued || extra.bound)
			continue;
		const ExprFacts* facts = WithoutParentheses(extra.expr)->facts;
		ConstValue value;
		if (facts != nullptr && ClassOf(facts->type) != nullptr)
		{
			ConstPlace place;
			if (!new_object(facts->type, place, location) || !initialize_from(*extra.expr, extra.conversion, place))
				return false;
		}
		else if (!converted(*extra.expr, extra.conversion, value))
			return false;
	}
	SmallList<const Entity*, 8> entities;
	for (const Binding* binding = function->scope != nullptr ? function->scope->bindings : nullptr; binding != nullptr;
		binding = binding->next)
	{
		if (binding->kind == BindingKind::Parameter)
			entities.push_back(binding->entity);
	}
	SmallList<Local, 8> bound;
	for (uint32_t i = 0; i < type->param_count; ++i)
	{
		const Type* parameter = type->params[i];
		const Entity* entity = i < entities.size() ? entities[i] : nullptr;
		Argument argument = Argument();
		// A default argument's source location builtins report the
		// outermost such use: this call (see Intrinsic::SourceFile).
		const SourceSite site(source_site_, i < count ? kNoSourceSite : location);
		if (i < count)
			argument = arguments[i];
		else
		{
			argument.expr = analyzer_.default_argument(function, i);
			if (argument.expr == nullptr)
				return fail(location, "a call without an argument for a parameter");
			argument.conversion = WithoutParentheses(argument.expr)->facts != nullptr
				? WithoutParentheses(argument.expr)->facts->conversion
				: nullptr;
		}
		Local local = Local();
		local.entity = entity;
		if (parameter->is_reference())
		{
			if (!argument_place(argument, parameter, local.place, location))
				return false;
			bound.push_back(local);
			continue;
		}
		const Type* object = entity != nullptr && !entity->type->is_reference() &&
				entity->type->kind != TypeKind::Array && entity->type->kind != TypeKind::Function
			? entity->type
			: parameter;
		ConstValue value;
		if (ClassOf(parameter) != nullptr)
		{
			if (!new_object(object, local.place, location))
				return false;
			if (argument.bound)
			{
				ConstValue* source = nullptr;
				if (!locate(argument.place, false, source, location) || !write(local.place, *source, location))
					return false;
			}
			else if (!initialize_from(*argument.expr, argument.conversion, local.place))
				return false;
			bound.push_back(local);
			continue;
		}
		if (argument.valued)
			value = argument.value;
		else if (argument.bound)
		{
			if (!read(argument.place, value, location))
				return false;
		}
		else if (!converted(*argument.expr, argument.conversion, value))
			return false;
		if (!convert_scalar(value, parameter, location) || !new_object(object, local.place, location))
			return false;
		value.flags &= ~kValueZeroed;
		if (!write(local.place, value, location))
			return false;
		bound.push_back(local);
	}
	for (const Local& local : bound)
		locals_.push_back(local);
	return true;
}

void ConstantInterpreter::end_parameters(const FunctionEntity* function, size_t first)
{
	// 5.2.2p4: a parameter's lifetime ends when its function returns; a
	// reference or pointer to one dangles (its object's generation changes,
	// its storage stays until the caller's statement ends).
	const Type* type = function->type->canonical;
	for (uint32_t i = 0; i < type->param_count && first + i < locals_.size(); ++i)
	{
		const ConstPlace& place = locals_[first + i].place;
		if (!type->params[i]->is_reference() && place.root == RootKind::Object && place.object < objects_.size())
			objects_[place.object].generation = ++stacks_.generation;
	}
}

bool ConstantInterpreter::body(const FunctionEntity* function, Result& result, uint32_t location)
{
	// The function body's statements; flowing off the end of a function
	// that returns a value is undefined (6.6.3p2).
	(void)result;
	const syntax::FunctionBody* definition = function->body;
	if (definition == nullptr || definition->body == nullptr)
		return function->constructor || fail(location, "a constexpr function without a body");
	if (definition->try_block)
		return fail(location, "a function-try-block in a constant expression");
	Flow flow = Flow::Next;
	if (!compound(*definition->body, flow))
		return false;
	if (flow == Flow::Return || function->constructor || IsVoid(function->type->canonical->element))
		return true;
	return fail(location, "a constexpr function flows off its end");
}

// -------------------------------------------------------------- statements

bool ConstantInterpreter::compound(const syntax::CompoundStmt& block, Flow& flow)
{
	// Each statement's temporaries end with it; the block's variables with
	// the block (6.7p2).
	const Region scope = region();
	bool done = true;
	for (const syntax::Stmt* item : block.items)
	{
		const Region statement = region();
		if (!execute(*item, flow))
		{
			done = false;
			break;
		}
		if (!Declares(item))
			release(statement);
		if (flow != Flow::Next)
			break;
	}
	release(scope);
	return done;
}

bool ConstantInterpreter::execute(const syntax::Stmt& stmt, Flow& flow)
{
	if (!step(stmt.location))
		return false;
	if (seeking_ != nullptr)
	{
		// A switch transferring to its label: statements before it are not
		// executed (6.4.2p5).
		switch (stmt.kind)
		{
		case syntax::StmtKind::Compound:
			return compound(static_cast<const syntax::CompoundStmt&>(stmt), flow);
		case syntax::StmtKind::Case:
		case syntax::StmtKind::Default:
		case syntax::StmtKind::Labeled:
		{
			const syntax::Stmt* inner = stmt.kind == syntax::StmtKind::Case
				? static_cast<const syntax::CaseStmt&>(stmt).body
				: static_cast<const syntax::LabeledStmt&>(stmt).body;
			if (&stmt == seeking_)
				seeking_ = nullptr;
			return execute(*inner, flow);
		}
		default:
			return true;
		}
	}
	const Region statement = region();
	switch (stmt.kind)
	{
	case syntax::StmtKind::Ambiguous:
		return execute(static_cast<const syntax::AmbiguousStmt&>(stmt).reading(), flow);
	case syntax::StmtKind::Compound:
		return compound(static_cast<const syntax::CompoundStmt&>(stmt), flow);
	case syntax::StmtKind::Expression:
	{
		const syntax::Expr* expr = static_cast<const syntax::ExpressionStmt&>(stmt).expression;
		return expr == nullptr || discard(*expr);
	}
	case syntax::StmtKind::Declaration:
		return declaration(*static_cast<const syntax::DeclarationStmt&>(stmt).declaration);
	case syntax::StmtKind::If:
	{
		const syntax::IfStmt& branch = static_cast<const syntax::IfStmt&>(stmt);
		bool selected = false;
		bool done = condition(branch.condition, selected, stmt.location);
		if (done && selected)
			done = execute(*branch.then_branch, flow);
		else if (done && syntax::LiveElse(branch) != nullptr)
			done = execute(*branch.else_branch, flow);
		release(statement);
		return done;
	}
	case syntax::StmtKind::While:
	{
		// Each iteration's objects end with it (6.5p2), and its scratch
		// with them (a region per iteration, as it may take a new block).
		const syntax::ConditionalLoopStmt& loop = static_cast<const syntax::ConditionalLoopStmt&>(stmt);
		for (;;)
		{
			const Region iteration = region();
			bool selected = false, leave = false;
			if (!step(stmt.location) || !condition(loop.condition, selected, stmt.location))
				return false;
			if (!selected)
				break;
			if (!loop_body(*loop.body, flow, leave))
				return false;
			release(iteration);
			if (leave)
				break;
		}
		release(statement);
		return true;
	}
	case syntax::StmtKind::Do:
	{
		const syntax::DoStmt& loop = static_cast<const syntax::DoStmt&>(stmt);
		for (;;)
		{
			const Region iteration = region();
			bool selected = false, leave = false;
			if (!step(stmt.location) || !loop_body(*loop.body, flow, leave))
				return false;
			if (leave)
				break;
			if (!truth(*loop.condition, selected))
				return false;
			release(iteration);
			if (!selected)
				break;
		}
		release(statement);
		return true;
	}
	case syntax::StmtKind::For:
	{
		const syntax::ForStmt& loop = static_cast<const syntax::ForStmt&>(stmt);
		bool done = loop.init_declaration != nullptr
			? declaration(*loop.init_declaration)
			: loop.init_expression == nullptr || discard(*loop.init_expression);
		while (done)
		{
			const Region iteration = region();
			bool selected = true, leave = false;
			if (!step(stmt.location) ||
				(loop.condition != nullptr && !condition(*loop.condition, selected, stmt.location)))
			{
				done = false;
				break;
			}
			if (!selected)
				break;
			if (!loop_body(*loop.body, flow, leave))
			{
				done = false;
				break;
			}
			if (leave)
				break;
			if (loop.iteration != nullptr && !discard(*loop.iteration))
			{
				done = false;
				break;
			}
			release(iteration);
		}
		release(statement);
		return done;
	}
	case syntax::StmtKind::Switch:
		return switch_statement(static_cast<const syntax::ConditionalLoopStmt&>(stmt), flow);
	case syntax::StmtKind::RangeFor:
		return range_for(static_cast<const syntax::RangeForStmt&>(stmt), flow);
	case syntax::StmtKind::Case:
		return execute(*static_cast<const syntax::CaseStmt&>(stmt).body, flow);
	case syntax::StmtKind::Default:
	case syntax::StmtKind::Labeled:
		return execute(*static_cast<const syntax::LabeledStmt&>(stmt).body, flow);
	case syntax::StmtKind::Break:
		flow = Flow::Break;
		return true;
	case syntax::StmtKind::Continue:
		flow = Flow::Continue;
		return true;
	case syntax::StmtKind::Return:
		if (!returned(static_cast<const syntax::ValueStmt&>(stmt)))
			return false;
		flow = Flow::Return;
		return true;
	default:
		return fail(stmt.location, "a statement that is not allowed in a constexpr function");
	}
}

bool ConstantInterpreter::range_for(const syntax::RangeForStmt& stmt, Flow& flow)
{
	// 6.5.4p1 (C++14 constexpr bodies): the equivalent the analysis
	// recorded (sema::RangeForFacts) -- its variables, then the loop: the
	// condition, the for-range-declaration and the statement (an
	// iteration's objects end with it), the increment.
	const RangeForFacts* facts = stmt.facts;
	if (facts == nullptr)
		return fail(stmt.location, "a range-based for statement outside constant evaluation");
	const auto declare = [&](const syntax::InitDeclarator& declarator) {
		const Binding* binding = declarator.declared;
		return binding != nullptr && binding->kind == BindingKind::Variable &&
			local_variable(static_cast<const VariableEntity*>(binding->entity),
				VariableInitializer(binding->entity, declarator.initializer), stmt.location);
	};
	const Region statement = region();
	if ((facts->range != nullptr && !declare(*facts->range)) || !declare(*facts->begin) ||
		(facts->end != nullptr && !declare(*facts->end)))
		return false;
	for (;;)
	{
		const Region iteration = region();
		bool selected = false, leave = false;
		if (!step(stmt.location) || !truth(*facts->condition, selected))
			return false;
		if (!selected)
			break;
		if (!declare(facts->declaration) || !loop_body(*stmt.body, flow, leave))
			return false;
		if (leave)
			break;
		if (!discard(*facts->increment))
			return false;
		release(iteration);
	}
	release(statement);
	return true;
}

bool ConstantInterpreter::loop_body(const syntax::Stmt& body, Flow& flow, bool& leave)
{
	if (!execute(body, flow))
		return false;
	if (flow == Flow::Break)
	{
		flow = Flow::Next;
		leave = true;
	}
	else if (flow == Flow::Continue)
		flow = Flow::Next;
	else if (flow == Flow::Return)
		leave = true;
	return true;
}

bool ConstantInterpreter::switch_statement(const syntax::ConditionalLoopStmt& stmt, Flow& flow)
{
	// 6.4.2: the condition's promoted value selects the case label (or the
	// default label) execution resumes at.
	const Region statement = region();
	ConstValue value;
	const syntax::Condition& written = stmt.condition;
	if (written.declaration != nullptr)
	{
		const VariableEntity* variable = written.declaration->declared != nullptr
			? static_cast<const VariableEntity*>(written.declaration->declared->entity)
			: nullptr;
		ConstPlace place;
		if (variable == nullptr || !local_variable(variable, written.declaration->initializer, stmt.location) ||
			!local(variable, place) || !read(place, value, stmt.location))
			return false;
		const Conversion* conversion = written.declaration->conversion;
		if (conversion != nullptr && conversion->result != nullptr &&
			!convert_scalar(value, conversion->result, stmt.location))
			return false;
	}
	else if (!operand(*written.expression, value))
		return false;
	if (value.kind != ValueKind::Integer)
		return fail(stmt.location, "a switch condition that is not an integer");
	const syntax::Stmt* fallback = nullptr;
	const syntax::Stmt* target = SwitchTarget(stmt.body, value.bits, fallback, 0);
	if (target == nullptr)
		target = fallback;
	bool done = true;
	if (target != nullptr)
	{
		seeking_ = target;
		done = execute(*stmt.body, flow);
		seeking_ = nullptr;
		if (flow == Flow::Break)
			flow = Flow::Next;
	}
	release(statement);
	return done;
}

bool ConstantInterpreter::condition(const syntax::Condition& written, bool& out, uint32_t location)
{
	// 6.4p2-p4: an expression, or a declared variable's value, contextually
	// converted to bool.
	if (written.declaration == nullptr)
		return truth(*written.expression, out);
	const VariableEntity* variable = written.declaration->declared != nullptr
		? static_cast<const VariableEntity*>(written.declaration->declared->entity)
		: nullptr;
	ConstPlace place;
	ConstValue value;
	if (variable == nullptr || !local_variable(variable, written.declaration->initializer, location) ||
		!local(variable, place) || !read(place, value, location))
		return false;
	return truth_of(value, out, location);
}

bool ConstantInterpreter::declaration(const syntax::Decl& decl)
{
	// A block-scope declaration: its variables are created and initialized
	// (a structured binding's object e; a tuple-like E's names, reference
	// variables bound to get<i>(e); the other names designate parts of e);
	// types, aliases, static assertions and using-declarations do nothing.
	if (decl.kind != syntax::DeclKind::Simple)
		return true;
	for (const syntax::InitDeclarator& declarator : static_cast<const syntax::SimpleDecl&>(decl).declarators)
	{
		const Binding* binding = declarator.declared;
		if (binding == nullptr || binding->kind != BindingKind::Variable)
			continue;
		if (!local_variable(static_cast<const VariableEntity*>(binding->entity),
				VariableInitializer(binding->entity, declarator.initializer), decl.location))
			return false;
	}
	return true;
}

bool ConstantInterpreter::local_variable(
	const VariableEntity* variable, const syntax::Initializer* initializer, uint32_t location)
{
	// 7.1.5p3 (C++14): a variable of automatic storage duration; not a
	// static or thread_local one.
	if (variable->is_extern)
		return true;
	if (variable->is_static || variable->is_thread_local)
		return fail(location, "a static or thread_local variable in a constant expression");
	Local local = Local();
	local.entity = variable;
	const Type* type = variable->type;
	if (type->is_reference())
	{
		const syntax::Expr* single = nullptr;
		if (initializer != nullptr && initializer->kind == syntax::InitializerKind::Assign)
			single = initializer->expression;
		else if (initializer != nullptr && initializer->kind == syntax::InitializerKind::Parenthesized &&
			initializer->arguments.size == 1)
			single = initializer->arguments[0];
		if (single != nullptr && single->kind == syntax::ExprKind::BracedInit && single->facts != nullptr &&
			(single->facts->flags & kInitializerList) != 0)
		{
			// A temporary std::initializer_list object the list initializes,
			// bound and extended (8.5.4p3, 12.2p5: `auto&& __range = {...}`).
			if (!new_object(single->facts->type, local.place, location) ||
				!list_object(ClassOf(single->facts->type), local.place,
					*static_cast<const syntax::BracedInitList*>(single), location))
				return false;
			locals_.push_back(local);
			return true;
		}
		if (single != nullptr && single->kind == syntax::ExprKind::BracedInit)
		{
			const syntax::BracedInitList* list = static_cast<const syntax::BracedInitList*>(single);
			single = list->elements.size == 1 ? list->elements[0] : nullptr;
		}
		if (single == nullptr || !bind(*single, RecordedConversion(single), type, local.place))
			return fail(location, "a reference variable outside constant evaluation");
		locals_.push_back(local);
		return true;
	}
	if (!new_object(type, local.place, location))
		return false;
	locals_.push_back(local);
	return construct(type, local.place, initializer, variable->constructor, location);
}

bool ConstantInterpreter::returned(const syntax::ValueStmt& stmt)
{
	// 6.6.3p2: the returned expression copy-initializes the result (a
	// reference binds to it).
	const Frame& frame = frames_.back();
	Result* result = frame.result;
	const syntax::Expr* value = stmt.value;
	const FunctionEntity* function = frame.function;
	const Type* returns = function->type->canonical->element;
	if (value == nullptr)
		return true;
	if (function->constructor || IsVoid(returns))
		return discard(*value);
	if (result == nullptr)
		return fail(stmt.location, "a return outside a constexpr call");
	const syntax::Expr* e = WithoutParentheses(value);
	if (returns->is_reference())
		return bind(*value, e->facts != nullptr ? e->facts->conversion : nullptr, returns, result->place);
	if (ClassOf(returns) != nullptr)
	{
		if (result->target == nullptr)
			return fail(stmt.location, "a class result without its object");
		if (e->kind == syntax::ExprKind::BracedInit)
			return materialize(*e, *result->target);
		return initialize_from(*value, e->facts->conversion, *result->target);
	}
	ConstValue returned_value;
	if (e->kind == syntax::ExprKind::BracedInit)
	{
		if (!rvalue(*e, returned_value))
			return false;
	}
	else if (!operand(*value, returned_value))
		return false;
	if (!convert_scalar(returned_value, returns, stmt.location))
		return false;
	if (result->held != nullptr && returned_value.kind == ValueKind::Array &&
		returned_value.count <= (IsComplex(returns) ? 2 : returns->unqualified->bound))
	{
		// Its parts or lanes in the caller's storage (see invoke).
		for (uint32_t i = 0; i < returned_value.count; ++i)
			result->held[i] = returned_value.elements[i];
		returned_value.elements = result->held;
	}
	returned_value.flags &= ~kValueZeroed;
	result->value = returned_value;
	return true;
}

} // namespace sema
