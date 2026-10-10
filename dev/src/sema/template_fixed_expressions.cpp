#include "sema/analyzer.h"

#include "lex/number_literal.h"
#include "lex/text_literal.h"
#include "sema/string_literals.h"

// A template definition's expressions whose types depend on no template
// parameter by their form alone (14.6.2.2p4: literals and sizeof are never
// type-dependent), and the built-in operators and calls of non-template
// functions over them: such an expression is not dependent (14.6p10), so
// an operand its operator cannot take, or a call no function of the set
// can accept, makes the definition ill-formed even if it is never
// instantiated (14.6p8: no valid specialization can be generated). Other
// operands (names, casts, a member's or class type's operator functions)
// are typed where the definition is instantiated.
namespace sema {

// What the validation knows of an operand: its type (null when unknown;
// never a reference: an lvalue or xvalue of the referenced type), its
// value category, whether it may be a null pointer constant (4.10p1: an
// integral constant expression evaluating to zero) and whether its
// operands are all literals and sizeof (a constant expression).
struct FixedFacts
{
	const Type* type;
	ValueCategory category;
	bool null_constant;
	bool constant;
};

namespace {

const syntax::Expr* WithoutParens(const syntax::Expr* expr)
{
	while (expr->kind == syntax::ExprKind::Paren)
		expr = static_cast<const syntax::ParenExpr*>(expr)->inner;
	return expr;
}

FixedFacts Unknown()
{
	return FixedFacts{ nullptr, ValueCategory::PRValue, false, false };
}

FixedFacts Value(const Type* type, bool constant)
{
	return FixedFacts{ type, ValueCategory::PRValue, false, constant };
}

// A type whose built-in operators the validation checks: a fundamental
// type, a pointer or an array (a class or enumeration operand may have
// operator functions, a GNU vector or complex one its own rules).
bool Plain(const FixedFacts& facts)
{
	if (facts.type == nullptr)
		return false;
	const TypeKind kind = facts.type->unqualified->kind;
	return kind == TypeKind::Fundamental || kind == TypeKind::Pointer || kind == TypeKind::Array;
}

bool IsVoidType(const Type* type)
{
	type = type->unqualified;
	return type->kind == TypeKind::Fundamental && type->fundamental == FundamentalType::Void;
}

bool IsNullptrType(const Type* type)
{
	type = type->unqualified;
	return type->kind == TypeKind::Fundamental && type->fundamental == FundamentalType::NullptrT;
}

// An argument type with no associated namespace or class (3.4.2p2), so
// that argument-dependent lookup adds no function where the definition
// is instantiated.
bool WithoutAssociations(const Type* type, uint32_t depth)
{
	if (depth > 16)
		return false;
	type = type->unqualified;
	switch (type->kind)
	{
	case TypeKind::Fundamental:
		return true;
	case TypeKind::Pointer:
	case TypeKind::Array:
		return WithoutAssociations(type->element, depth + 1);
	default:
		return false;
	}
}

// The forms the validation never types (fixed_expression's default case):
// names and the forms with their own analysis.
bool Opaque(syntax::ExprKind kind)
{
	switch (kind)
	{
	case syntax::ExprKind::Id:
	case syntax::ExprKind::SizeofPack:
	case syntax::ExprKind::TypeTrait:
	case syntax::ExprKind::New:
	case syntax::ExprKind::Delete:
	case syntax::ExprKind::Lambda:
	case syntax::ExprKind::Statement:
	case syntax::ExprKind::BuiltinTrait:
	case syntax::ExprKind::Fold:
		return true;
	default:
		return false;
	}
}

// 2.14.2, 2.14.3, 2.14.4: an integer, character or floating literal
// without a ud-suffix (a string literal is left unknown).
FixedFacts FixedLiteral(const TokenSpelling& spelling, TypeTable& types)
{
	TextLiteralParts parts;
	if (SplitTextLiteral(spelling.text, spelling.length, parts))
	{
		if (parts.quote != '\'')
			return Unknown();
		const CharacterLiteralValue character = EvaluateCharacterLiteral(spelling.text, spelling.length);
		if (!character.valid || character.suffix_begin != spelling.length)
			return Unknown();
		FixedFacts out = Value(types.fundamental(character.type), true);
		out.null_constant = character.code_point == 0;
		return out;
	}
	const NumberLiteral number = ClassifyNumber(spelling.text, spelling.length);
	FixedFacts out = Unknown();
	if (number.kind == NumberLiteralKind::Integer)
	{
		out = Value(types.fundamental(number.type), true);
		out.null_constant = number.integer == 0;
	}
	else if (number.kind == NumberLiteralKind::Floating)
		out = Value(types.fundamental(number.type), true);
	return out;
}

// Whether the operands fixed_expression walks include a literal, a keyword
// literal or a sizeof (or a nest too deep to scan): without one, every
// operand is unknown, so the walk can neither type nor reject anything.
bool Seeded(const syntax::Expr* e, uint32_t depth)
{
	for (;; ++depth)
	{
		if (depth > 200)
			return true;
		switch (e->kind)
		{
		case syntax::ExprKind::Literal:
		case syntax::ExprKind::KeywordLiteral:
		case syntax::ExprKind::Sizeof:
			return true;
		case syntax::ExprKind::Paren:
			e = static_cast<const syntax::ParenExpr*>(e)->inner;
			continue;
		case syntax::ExprKind::Unary:
		case syntax::ExprKind::Postfix:
			e = static_cast<const syntax::UnaryExpr*>(e)->operand;
			continue;
		case syntax::ExprKind::Binary:
		case syntax::ExprKind::Assign:
			if (Seeded(static_cast<const syntax::BinaryExpr*>(e)->left, depth + 1))
				return true;
			e = static_cast<const syntax::BinaryExpr*>(e)->right;
			continue;
		case syntax::ExprKind::Subscript:
			if (Seeded(static_cast<const syntax::SubscriptExpr*>(e)->base, depth + 1))
				return true;
			e = static_cast<const syntax::SubscriptExpr*>(e)->index;
			continue;
		case syntax::ExprKind::Member:
			e = static_cast<const syntax::MemberExpr*>(e)->base;
			continue;
		case syntax::ExprKind::Cast:
			e = static_cast<const syntax::CastExpr*>(e)->operand;
			continue;
		case syntax::ExprKind::PackExpansion:
			e = static_cast<const syntax::PackExpansionExpr*>(e)->pattern;
			continue;
		case syntax::ExprKind::Throw:
			e = static_cast<const syntax::ThrowExpr*>(e)->operand;
			if (e == nullptr)
				return false;
			continue;
		case syntax::ExprKind::Conditional:
		{
			const syntax::ConditionalExpr* conditional = static_cast<const syntax::ConditionalExpr*>(e);
			if (Seeded(conditional->condition, depth + 1) || Seeded(conditional->then_expr, depth + 1))
				return true;
			e = conditional->else_expr;
			continue;
		}
		case syntax::ExprKind::Call:
		{
			const syntax::CallExpr* call = static_cast<const syntax::CallExpr*>(e);
			for (const syntax::Expr* argument : call->arguments)
			{
				if (Seeded(argument, depth + 1))
					return true;
			}
			e = call->callee;
			continue;
		}
		case syntax::ExprKind::FunctionalCast:
		{
			const syntax::FunctionalCastExpr* cast = static_cast<const syntax::FunctionalCastExpr*>(e);
			if (cast->braced != nullptr && Seeded(cast->braced, depth + 1))
				return true;
			for (const syntax::Expr* argument : cast->arguments)
			{
				if (Seeded(argument, depth + 1))
					return true;
			}
			return false;
		}
		case syntax::ExprKind::BracedInit:
			for (const syntax::Expr* element : static_cast<const syntax::BracedInitList*>(e)->elements)
			{
				if (Seeded(element, depth + 1))
					return true;
			}
			return false;
		default:
			return false;
		}
	}
}

} // namespace

void Analyzer::validate_fixed_expression(const syntax::Expr& expr, Scope* scope)
{
	if (!Seeded(&expr, 0))
		return;
	FixedFacts facts;
	fixed_expression(expr, scope, 0, facts);
}

const Type* Analyzer::fixed_decayed(const FixedFacts& facts)
{
	const Type* type = facts.type;
	if (type->kind == TypeKind::Array)
		return types_.pointer(type->element);
	if (type->kind == TypeKind::Function)
		return types_.pointer(type);
	return type->unqualified;
}

void Analyzer::fixed_expression(const syntax::Expr& expr, Scope* scope, uint32_t depth, FixedFacts& out)
{
	out = Unknown();
	// Deep operator nests are left to instantiation; a name and the other
	// opaque forms are unknown without a walk.
	if (depth > 200 || Opaque(expr.kind))
		return;
	const syntax::Expr* e = WithoutParens(&expr);
	FixedFacts first, second;
	switch (e->kind)
	{
	case syntax::ExprKind::Literal:
		out = FixedLiteral(static_cast<const syntax::LiteralExpr*>(e)->spelling, types_);
		return;
	case syntax::ExprKind::KeywordLiteral:
	{
		const TokenKind keyword = static_cast<const syntax::KeywordLiteralExpr*>(e)->keyword;
		if (keyword == TokenKind::KW_TRUE || keyword == TokenKind::KW_FALSE)
		{
			out = Value(types_.fundamental(FundamentalType::Bool), true);
			out.null_constant = keyword == TokenKind::KW_FALSE;
		}
		else if (keyword == TokenKind::KW_NULLPTR)
		{
			out = Value(types_.fundamental(FundamentalType::NullptrT), true);
			out.null_constant = true;
		}
		return;
	}
	case syntax::ExprKind::Sizeof:
	{
		// 5.3.3p6: a constant of type std::size_t (its unevaluated operand
		// is checked too).
		const syntax::SizeofExpr& size = static_cast<const syntax::SizeofExpr&>(*e);
		if (size.operand != nullptr)
			fixed_expression(*size.operand, scope, depth + 1, first);
		out = Value(types_.fundamental(FundamentalType::UnsignedLongInt), true);
		return;
	}
	case syntax::ExprKind::Unary:
	case syntax::ExprKind::Postfix:
		fixed_expression(*static_cast<const syntax::UnaryExpr*>(e)->operand, scope, depth + 1, first);
		fixed_operator(*e, first, nullptr, out);
		return;
	case syntax::ExprKind::Binary:
	case syntax::ExprKind::Assign:
	{
		const syntax::BinaryExpr& binary = static_cast<const syntax::BinaryExpr&>(*e);
		fixed_expression(*binary.left, scope, depth + 1, first);
		fixed_expression(*binary.right, scope, depth + 1, second);
		fixed_operator(*e, first, &second, out);
		return;
	}
	case syntax::ExprKind::Subscript:
	{
		const syntax::SubscriptExpr& subscript = static_cast<const syntax::SubscriptExpr&>(*e);
		fixed_expression(*subscript.base, scope, depth + 1, first);
		fixed_expression(*subscript.index, scope, depth + 1, second);
		fixed_operator(*e, first, &second, out);
		return;
	}
	case syntax::ExprKind::Member:
		fixed_expression(*static_cast<const syntax::MemberExpr*>(e)->base, scope, depth + 1, first);
		fixed_operator(*e, first, nullptr, out);
		return;
	case syntax::ExprKind::Call:
		fixed_call(static_cast<const syntax::CallExpr&>(*e), scope, depth, out);
		return;
	case syntax::ExprKind::Conditional:
	{
		const syntax::ConditionalExpr& conditional = static_cast<const syntax::ConditionalExpr&>(*e);
		fixed_expression(*conditional.condition, scope, depth + 1, first);
		fixed_expression(*conditional.then_expr, scope, depth + 1, first);
		fixed_expression(*conditional.else_expr, scope, depth + 1, first);
		return;
	}
	case syntax::ExprKind::Cast:
		fixed_expression(*static_cast<const syntax::CastExpr*>(e)->operand, scope, depth + 1, first);
		return;
	case syntax::ExprKind::FunctionalCast:
	{
		const syntax::FunctionalCastExpr& cast = static_cast<const syntax::FunctionalCastExpr&>(*e);
		if (cast.braced != nullptr)
			fixed_expression(*cast.braced, scope, depth + 1, first);
		for (const syntax::Expr* argument : cast.arguments)
			fixed_expression(*argument, scope, depth + 1, first);
		return;
	}
	case syntax::ExprKind::BracedInit:
		for (const syntax::Expr* element : static_cast<const syntax::BracedInitList*>(e)->elements)
			fixed_expression(*element, scope, depth + 1, first);
		return;
	case syntax::ExprKind::PackExpansion:
		fixed_expression(*static_cast<const syntax::PackExpansionExpr*>(e)->pattern, scope, depth + 1, first);
		return;
	case syntax::ExprKind::Throw:
		if (static_cast<const syntax::ThrowExpr*>(e)->operand != nullptr)
			fixed_expression(*static_cast<const syntax::ThrowExpr*>(e)->operand, scope, depth + 1, first);
		return;
	default:
		// Names, `this`, other operators and the forms with their own
		// analysis (lambdas, statement expressions, new-expressions).
		return;
	}
}

void Analyzer::fixed_operator(
	const syntax::Expr& expr, const FixedFacts& left, const FixedFacts* right, FixedFacts& out)
{
	out = Unknown();
	if (expr.kind == syntax::ExprKind::Member)
	{
		// 5.2.5p2: the object of `.` is of class type, that of `->` a pointer
		// to one; a pseudo-destructor name (5.2.4) takes a scalar.
		const syntax::MemberExpr& member = static_cast<const syntax::MemberExpr&>(expr);
		if (!Plain(left) || member.member->last().kind == syntax::NameComponentKind::Destructor)
			return;
		if (member.op == TokenKind::OP_ARROW)
		{
			const Type* pointer = fixed_decayed(left);
			if (pointer->kind == TypeKind::Pointer && pointer->element->unqualified->kind != TypeKind::Fundamental &&
				pointer->element->unqualified->kind != TypeKind::Pointer)
				return;
		}
		error(expr.location, "member access to an object of non-class type in a template definition");
	}
	const bool binary = right != nullptr && expr.kind != syntax::ExprKind::Subscript;
	const TokenKind op = expr.kind == syntax::ExprKind::Subscript ? TokenKind::OP_LSQUARE
		: binary                                                  ? static_cast<const syntax::BinaryExpr&>(expr).op
																  : static_cast<const syntax::UnaryExpr&>(expr).op;
	if (op == TokenKind::OP_COMMA)
	{
		// 5.18p1: the right operand's type and category.
		out = *right;
		out.null_constant = out.type != nullptr && IsIntegralType(out.type) && out.category == ValueCategory::PRValue;
		out.constant = out.constant && left.constant;
		return;
	}
	if (expr.kind == syntax::ExprKind::Assign)
	{
		// 5.17p1: a modifiable lvalue on the left (whatever the right).
		if (!Plain(left))
			return;
		if (left.type->kind != TypeKind::Array &&
			(left.category != ValueCategory::LValue || (left.type->cv & kConst) != 0))
			error(expr.location, "assignment to a value that is not a modifiable lvalue in a template definition");
		out = left;
		out.null_constant = false;
		out.constant = false;
		return;
	}
	if (!Plain(left) || (right != nullptr && !Plain(*right)))
		return;
	const bool constant = left.constant && (right == nullptr || right->constant);
	const Type* lt = fixed_decayed(left);
	if (right == nullptr)
	{
		const char* failure = nullptr;
		switch (op)
		{
		case TokenKind::OP_STAR:
			// 5.3.1p1.
			if (lt->kind != TypeKind::Pointer)
				failure = "indirection through an operand that is not a pointer in a template definition";
			else
				out = FixedFacts{ lt->element, ValueCategory::LValue, false, false };
			break;
		case TokenKind::OP_AMP:
			// 5.3.1p3.
			if (left.category != ValueCategory::LValue)
				failure = "the address of a value that is not an lvalue in a template definition";
			else
				out = Value(types_.pointer(left.type), false);
			break;
		case TokenKind::OP_INC:
		case TokenKind::OP_DEC:
			// 5.2.6p1, 5.3.2p1.
			if (left.category != ValueCategory::LValue || (left.type->cv & kConst) != 0 ||
				left.type->kind == TypeKind::Array || (!IsArithmetic(lt) && lt->kind != TypeKind::Pointer))
				failure = "increment of an operand that is not a modifiable arithmetic or pointer lvalue in a template "
						  "definition";
			else if (expr.kind == syntax::ExprKind::Postfix)
				out = Value(lt, false);
			else
				out = FixedFacts{ left.type, ValueCategory::LValue, false, false };
			break;
		case TokenKind::OP_LNOT:
			// 5.3.1p9: contextually converted to bool.
			if (IsVoidType(lt))
				failure = "logical negation of a void operand in a template definition";
			else
				out = Value(types_.fundamental(FundamentalType::Bool), constant);
			break;
		case TokenKind::OP_PLUS:
		case TokenKind::OP_MINUS:
		case TokenKind::OP_COMPL:
			// 5.3.1p7, p8, p10.
			if (op == TokenKind::OP_PLUS && lt->kind == TypeKind::Pointer)
				out = Value(lt, false);
			else if (op == TokenKind::OP_COMPL ? !IsIntegralType(lt) : !IsArithmetic(lt))
				failure = "an operand of a unary arithmetic operator that it does not take in a template definition";
			else
				out = Value(promoted(lt), constant);
			break;
		default:
			break;
		}
		if (failure != nullptr)
			error(expr.location, failure);
		out.null_constant = out.constant && out.type != nullptr && IsIntegralType(out.type);
		return;
	}
	const Type* rt = fixed_decayed(*right);
	if (op == TokenKind::OP_LSQUARE)
	{
		// 5.2.1p1: one pointer to an object type, one integral operand.
		const Type* pointer = lt->kind == TypeKind::Pointer ? lt : rt;
		const Type* index = lt->kind == TypeKind::Pointer ? rt : lt;
		if (pointer->kind != TypeKind::Pointer || !IsIntegralType(index))
			error(expr.location, "subscript operands that are not a pointer and an integer in a template definition");
		out = FixedFacts{ pointer->element, ValueCategory::LValue, false, false };
		return;
	}
	const bool pointers = lt->kind == TypeKind::Pointer || rt->kind == TypeKind::Pointer;
	const bool l_null = left.null_constant || IsNullptrType(lt);
	const bool r_null = right->null_constant || IsNullptrType(rt);
	bool valid = !IsVoidType(lt) && !IsVoidType(rt);
	const Type* result = nullptr;
	switch (op)
	{
	case TokenKind::OP_STAR:
	case TokenKind::OP_DIV:
		valid = valid && IsArithmetic(lt) && IsArithmetic(rt);
		result = valid ? usual_arithmetic(lt, rt) : nullptr;
		break;
	case TokenKind::OP_MOD:
	case TokenKind::OP_AMP:
	case TokenKind::OP_BOR:
	case TokenKind::OP_XOR:
		valid = valid && IsIntegralType(lt) && IsIntegralType(rt);
		result = valid ? usual_arithmetic(lt, rt) : nullptr;
		break;
	case TokenKind::OP_LSHIFT:
	case TokenKind::OP_RSHIFT:
		valid = valid && IsIntegralType(lt) && IsIntegralType(rt);
		result = valid ? promoted(lt) : nullptr;
		break;
	case TokenKind::OP_PLUS:
		// 5.7p1.
		if (IsArithmetic(lt) && IsArithmetic(rt))
			result = usual_arithmetic(lt, rt);
		else if (lt->kind == TypeKind::Pointer && IsIntegralType(rt))
			result = lt;
		else if (rt->kind == TypeKind::Pointer && IsIntegralType(lt))
			result = rt;
		else
			valid = false;
		break;
	case TokenKind::OP_MINUS:
		// 5.7p2.
		if (IsArithmetic(lt) && IsArithmetic(rt))
			result = usual_arithmetic(lt, rt);
		else if (lt->kind == TypeKind::Pointer && IsIntegralType(rt))
			result = lt;
		else if (lt->kind == TypeKind::Pointer && rt->kind == TypeKind::Pointer)
			result = types_.fundamental(FundamentalType::LongInt);
		else
			valid = false;
		break;
	case TokenKind::OP_LT:
	case TokenKind::OP_GT:
	case TokenKind::OP_LE:
	case TokenKind::OP_GE:
	case TokenKind::OP_EQ:
	case TokenKind::OP_NE:
		// 5.9p2, 5.10p1: arithmetic operands, or pointers and null pointer
		// constants (std::nullptr_t operands among them).
		if (!(IsArithmetic(lt) && IsArithmetic(rt)) &&
			!((pointers || IsNullptrType(lt) || IsNullptrType(rt)) && (lt->kind == TypeKind::Pointer || l_null) &&
				(rt->kind == TypeKind::Pointer || r_null)))
			valid = false;
		result = types_.fundamental(FundamentalType::Bool);
		break;
	case TokenKind::OP_LAND:
	case TokenKind::OP_LOR:
		// 5.14p1, 5.15p1: each contextually converted to bool.
		result = types_.fundamental(FundamentalType::Bool);
		break;
	default:
		// Pointers to members and the GNU forms.
		return;
	}
	if (!valid)
		error(expr.location, "operands a built-in binary operator does not take in a template definition");
	const bool arithmetic = result != nullptr && !pointers;
	out = Value(result, constant && arithmetic);
	out.null_constant = out.constant && IsIntegralType(result);
}

void Analyzer::fixed_call(const syntax::CallExpr& call, Scope* scope, uint32_t depth, FixedFacts& out)
{
	// 13.3.2: some function of a set of non-template, non-member functions
	// that unqualified lookup finds accepts arguments whose types are all
	// known (none with associated namespaces or classes, so instantiation's
	// argument-dependent lookup adds none, 14.6.4.2), one of them at least
	// a constant of literals and sizeof (a call of no such argument is left
	// to instantiation, sparing the lookup).
	out = Unknown();
	const uint32_t kMaximum = 16;
	FixedFacts arguments[kMaximum];
	bool known = call.arguments.size <= kMaximum;
	bool constant = false;
	for (uint32_t i = 0; i < call.arguments.size; ++i)
	{
		FixedFacts argument;
		fixed_expression(*call.arguments[i], scope, depth + 1, argument);
		if (i < kMaximum)
			arguments[i] = argument;
		known = known && Plain(argument) && WithoutAssociations(argument.type, 0);
		constant = constant || argument.constant;
	}
	known = known && constant;
	if (call.callee->kind != syntax::ExprKind::Id)
	{
		FixedFacts callee;
		fixed_expression(*call.callee, scope, depth + 1, callee);
		return;
	}
	const syntax::Name& name = *static_cast<const syntax::IdExpr*>(call.callee)->name;
	if (!known || name.qualified() || name.last().kind != syntax::NameComponentKind::Identifier ||
		name.last().has_arguments)
		return;
	const LookupResult found = names_.unqualified(scope, name.last().identifier, LookupFilter::Ordinary);
	if (!found.found() || found.ambiguous || found.binding->kind != BindingKind::Function || found.functions == nullptr)
		return;
	uint32_t viable = 0;
	const Type* result = nullptr;
	for (const FunctionList* node = found.functions; node != nullptr; node = node->next)
	{
		const Entity* entity = node->binding->entity;
		if (entity == nullptr || entity->kind != EntityKind::Function)
			return;
		const FunctionEntity* function = static_cast<const FunctionEntity*>(entity);
		const Type* type = function->type != nullptr ? function->type->canonical : nullptr;
		if (function->templated || function->member || function->builtin != BuiltinFunction::None ||
			function->gnu_builtin != GnuBuiltin::None || type == nullptr || type->kind != TypeKind::Function ||
			dependent_type(type))
			return;
		if (call.arguments.size > type->param_count && !type->variadic)
			continue;
		bool accepted = true;
		for (uint32_t j = 0; j < type->param_count && accepted; ++j)
		{
			if (j >= call.arguments.size)
			{
				accepted = function->default_arguments != nullptr && function->default_arguments[j] != nullptr;
				continue;
			}
			// A class, enumeration or other parameter may take the argument
			// by a conversion the standard conversions do not model here.
			const Type* parameter = type->params[j];
			const Type* object = parameter->is_reference() ? parameter->element : parameter;
			if (object->unqualified->kind != TypeKind::Fundamental && object->unqualified->kind != TypeKind::Pointer)
				continue;
			Conversion conversion;
			const Operand operand{ arguments[j].type, arguments[j].category,
				static_cast<uint16_t>(arguments[j].null_constant ? kNullLiteral : 0) };
			accepted = conversions_.implicit(operand, parameter, false, conversion);
		}
		if (!accepted)
			continue;
		++viable;
		result = type->element;
	}
	if (viable == 0)
		error(call.location, "no function the nondependent call can call in a template definition");
	if (viable != 1)
		return;
	if (result->is_reference())
		out = FixedFacts{ result->element,
			result->kind == TypeKind::LValueReference || result->element->kind == TypeKind::Function
				? ValueCategory::LValue
				: ValueCategory::XValue,
			false, false };
	else
		out = Value(result, false);
}

} // namespace sema
