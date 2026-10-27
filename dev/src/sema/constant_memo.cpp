#include "sema/constant_memo.h"

#include "sema/expression_facts.h"
#include "sema/integral_arithmetic.h"
#include "syntax/ast.h"

namespace sema {

bool ConstantCallKey::operator==(const ConstantCallKey& other) const
{
	if (function != other.function || count != other.count)
		return false;
	for (uint32_t i = 0; i < count; ++i)
	{
		if (arguments[i] != other.arguments[i])
			return false;
	}
	return true;
}

size_t ConstantCallHash::operator()(const ConstantCallKey& key) const
{
	uint64_t hash = reinterpret_cast<uintptr_t>(key.function) * 0x9e3779b97f4a7c15ull;
	for (uint32_t i = 0; i < key.count; ++i)
		hash = (hash ^ key.arguments[i]) * 0x100000001b3ull;
	return static_cast<size_t>(hash ^ key.count);
}

bool ConstantMemo::remember(const ConstantCallKey& key, const ConstantCallResult& result)
{
	// An evaluation with many distinct calls must not turn its step budget
	// into an equally large cache. Saturation only gives up reuse.
	if (calls.size() >= 4096 || key.count > 65536 - argument_words_)
		return false;
	const ConstantCallKey stored{ key.function, arena_.copy_array(key.arguments, key.count), key.count };
	calls.insert(stored, result);
	argument_words_ += key.count;
	return true;
}

bool ConstantMemo::eligible(const FunctionEntity* function)
{
	const uint32_t state = purity_.find(function);
	if (state != PointerMap::kMissing)
		return state == 2;
	// An active dependency is not a purity proof. Direct recursion is
	// checked by expression(), after the rest of its own body is proved.
	purity_.set(function, 1);
	const bool pure = function_body(function);
	purity_.set(function, pure ? 2 : 3);
	return pure;
}

bool ConstantMemo::scalar_signature(const FunctionEntity* function)
{
	const Type* type = function->type->canonical;
	IntegralKind integral;
	if (!function->is_constexpr || function->member || function->constructor || type->variadic ||
		function->builtin != BuiltinFunction::None || !IntegralKindOf(type->element, integral))
		return false;
	for (uint32_t i = 0; i < type->param_count; ++i)
	{
		if (!IntegralKindOf(type->params[i], integral))
			return false;
	}
	return true;
}

bool ConstantMemo::function_body(const FunctionEntity* function)
{
	if (!scalar_signature(function) || function->body == nullptr || function->body->body == nullptr ||
		function->body->try_block != nullptr)
		return false;
	const syntax::Expr* returned = nullptr;
	for (const syntax::Stmt* statement : function->body->body->items)
	{
		if (statement->kind == syntax::StmtKind::Expression &&
			static_cast<const syntax::ExpressionStmt*>(statement)->expression == nullptr)
			continue;
		if (statement->kind != syntax::StmtKind::Return || returned != nullptr)
			return false;
		returned = static_cast<const syntax::ValueStmt*>(statement)->value;
	}
	return expression(returned, function, 0);
}

bool ConstantMemo::expression(const syntax::Expr* expr, const FunctionEntity* function, uint32_t depth)
{
	if (expr == nullptr || depth > 200 || expr->facts == nullptr ||
		(expr->facts->flags & (kUserConverted | kOperatorCall | kVirtualCall | kUserLiteral)) != 0)
		return false;
	IntegralKind integral;
	if (!IntegralKindOf(expr->facts->type, integral))
		return false;
	const auto pure = [&](const syntax::Expr* operand) { return expression(operand, function, depth + 1); };
	switch (expr->kind)
	{
	case syntax::ExprKind::Literal:
	case syntax::ExprKind::KeywordLiteral:
		return true;
	case syntax::ExprKind::Id:
		return expr->facts->entity != nullptr && expr->facts->entity->kind == EntityKind::Parameter &&
			expr->facts->entity->owner == function->scope;
	case syntax::ExprKind::Paren:
		return pure(static_cast<const syntax::ParenExpr*>(expr)->inner);
	case syntax::ExprKind::Binary:
	{
		const auto* binary = static_cast<const syntax::BinaryExpr*>(expr);
		return pure(binary->left) && pure(binary->right);
	}
	case syntax::ExprKind::Unary:
	{
		const auto* unary = static_cast<const syntax::UnaryExpr*>(expr);
		return (unary->op == TokenKind::OP_PLUS || unary->op == TokenKind::OP_MINUS ||
				   unary->op == TokenKind::OP_LNOT || unary->op == TokenKind::OP_COMPL) &&
			pure(unary->operand);
	}
	case syntax::ExprKind::Conditional:
	{
		const auto* conditional = static_cast<const syntax::ConditionalExpr*>(expr);
		return pure(conditional->condition) && pure(conditional->then_expr) && pure(conditional->else_expr);
	}
	case syntax::ExprKind::Cast:
		return pure(static_cast<const syntax::CastExpr*>(expr)->operand);
	case syntax::ExprKind::Call:
	{
		const auto* call = static_cast<const syntax::CallExpr*>(expr);
		const syntax::Expr* target = call->callee;
		while (target->kind == syntax::ExprKind::Paren)
			target = static_cast<const syntax::ParenExpr*>(target)->inner;
		// A direct target can still have a comma-expression before it.
		// Only a plain name proves that evaluating the callee does no work.
		if (target->kind != syntax::ExprKind::Id)
			return false;
		const Entity* entity = expr->facts->entity;
		if ((expr->facts->flags & kDirectCall) == 0 || entity == nullptr || entity->kind != EntityKind::Function)
			return false;
		const auto* callee = static_cast<const FunctionEntity*>(entity);
		if (call->arguments.size != callee->type->canonical->param_count || (callee != function && !eligible(callee)))
			return false;
		for (const syntax::Expr* argument : call->arguments)
		{
			if (!pure(argument))
				return false;
		}
		return true;
	}
	default:
		return false;
	}
}

} // namespace sema
