#include "lower/function_lowering.h"

#include "lowir/literals.h"
#include "sema/constant_values.h"
#include "sema/conversions.h"

// Conditions and choices: statement conditions branch in the source's
// sense (a comparison is one `cmp` with the source's predicate, `!x` is
// `cmp eq x, 0`, a bare scalar is branched on directly) and `&&`/`||`
// there become direct short-circuit control flow; as values, `&&`, `||`
// and `?:` select through a frame slot written on each path.
namespace lower {

namespace {

const syntax::Expr* WithoutParentheses(const syntax::Expr* expr)
{
	while (expr->kind == syntax::ExprKind::Paren)
		expr = static_cast<const syntax::ParenExpr*>(expr)->inner;
	return expr;
}

bool IsComparisonOperator(TokenKind op)
{
	return op == TokenKind::OP_EQ || op == TokenKind::OP_NE || op == TokenKind::OP_LT || op == TokenKind::OP_GT ||
		op == TokenKind::OP_LE || op == TokenKind::OP_GE;
}

// A built-in operator's own facts: not an overloaded one's call (13.3.1.2)
// nor the conversion function's call on its class result (kUserConverted:
// a condition's operator function returning a class, 6.4p4).
bool BuiltinOperator(const syntax::Expr* expr)
{
	return expr->facts == nullptr || (expr->facts->flags & (sema::kOperatorCall | sema::kUserConverted)) == 0;
}

// A built-in `&&` or `||`.
bool IsLogical(const syntax::Expr* expr, TokenKind op)
{
	return expr->kind == syntax::ExprKind::Binary && static_cast<const syntax::BinaryExpr*>(expr)->op == op &&
		BuiltinOperator(expr);
}

} // namespace

bool FunctionLowering::literal_truth(const syntax::Expr& expr, bool& value) const
{
	// An integer or boolean literal decides a condition at compile time.
	const syntax::Expr* e = WithoutParentheses(&expr);
	if (e->kind == syntax::ExprKind::KeywordLiteral)
	{
		const TokenKind keyword = static_cast<const syntax::KeywordLiteralExpr*>(e)->keyword;
		if (keyword != TokenKind::KW_TRUE && keyword != TokenKind::KW_FALSE)
			return false;
		value = keyword == TokenKind::KW_TRUE;
		return true;
	}
	// So does a conditional selecting such an operand at compile time (the
	// course's integral_constant form, sema::Analyzer::constant_conversion).
	bool selected = false;
	if (e->kind == syntax::ExprKind::Conditional && e->facts != nullptr && e->facts->type != nullptr &&
		sema::ClassOf(e->facts->type) == nullptr)
	{
		const syntax::ConditionalExpr* choice = static_cast<const syntax::ConditionalExpr*>(e);
		return lowering_.constant_conversion(*choice->condition, selected) &&
			literal_truth(selected ? *choice->then_expr : *choice->else_expr, value);
	}
	// So does a static data member's constant named alone (9.4.2p3: a
	// value read without odr-use), and an enumerator (or a non-type
	// template parameter's integral value) named alone.
	const bool static_constant = e->kind == syntax::ExprKind::Id && e->facts != nullptr &&
		(e->facts->flags & (sema::kStaticConstant | sema::kFolded)) != 0;
	// And a constexpr pointer whose value is null (the course's form).
	const sema::VariableEntity* named = e->kind == syntax::ExprKind::Id && e->facts != nullptr &&
			e->facts->entity != nullptr && e->facts->entity->kind == sema::EntityKind::Variable
		? static_cast<const sema::VariableEntity*>(e->facts->entity)
		: nullptr;
	if (named != nullptr && named->is_constexpr && !named->type->is_reference() &&
		(e->facts->type->cv & sema::kVolatile) == 0)
	{
		const sema::ConstValue* evaluated = lowering_.evaluated_value(named);
		if (evaluated != nullptr && evaluated->kind == sema::ValueKind::Pointer &&
			evaluated->root == sema::RootKind::Null)
		{
			value = false;
			return true;
		}
	}
	if ((e->kind != syntax::ExprKind::Literal && !static_constant) || e->facts == nullptr ||
		e->facts->type == nullptr || sema::IsFloating(e->facts->type) || e->facts->type->kind == sema::TypeKind::Array)
		return false;
	value = e->facts->value != 0;
	return true;
}

void FunctionLowering::branch_condition(
	const syntax::Expr& expr, lowir::BlockId if_true, lowir::BlockId if_false, bool fold_literals)
{
	// 5.14, 5.15: each right operand's block follows its left operand's
	// branch; a literal operand in an `if` condition is a jump. A
	// left-nested chain is walked iteratively: its right blocks are placed
	// innermost first, each continuing to the enclosing operator's targets.
	check_stack(expr.location);
	const size_t base = spine_.size();
	const size_t target_base = targets_.size();
	const syntax::Expr* e = WithoutParentheses(&expr);
	lowir::BlockId on_true = if_true, on_false = if_false;
	while (IsLogical(e, TokenKind::OP_LAND) || IsLogical(e, TokenKind::OP_LOR))
	{
		const syntax::BinaryExpr* logical = static_cast<const syntax::BinaryExpr*>(e);
		const lowir::BlockId right = new_block(
			logical->op == TokenKind::OP_LAND ? lowir::BlockKind::LandRhs : lowir::BlockKind::LorRhs);
		spine_.push_back(logical);
		targets_.push_back(ChainTargets{ right, on_true, on_false });
		if (logical->op == TokenKind::OP_LAND)
			on_true = right;
		else
			on_false = right;
		e = WithoutParentheses(logical->left);
	}
	// A right operand's temporaries (a conditional designator's condition
	// has them) end with the full-expression, guarded (12.2p3).
	const bool chain = spine_.size() > base;
	if (chain)
		begin_guard_scope();
	bool value = false;
	if (fold_literals && literal_truth(*e, value))
	{
		// (A folded branch: its target has an incoming edge whatever the
		// block, see block_targeted.)
		folded_targets_.push_back(value ? on_true : on_false);
		jump(value ? on_true : on_false);
	}
	else
		branch(truth(*e), on_true, on_false);
	for (size_t i = spine_.size(); i-- > base;)
	{
		const ChainTargets targets = targets_[target_base + (i - base)];
		place(targets.right);
		++guarded_branches_;
		branch_condition(*spine_[i]->right, targets.if_true, targets.if_false, fold_literals);
		--guarded_branches_;
	}
	spine_.resize(base);
	targets_.resize(target_base);
	if (chain)
		--guard_scopes_;
}

RValue FunctionLowering::truth(const syntax::Expr& expr)
{
	// The scalar a branch tests: a floating value compares with zero, a
	// pointer to member is its truth (member_pointer_truth).
	const RValue value = truth_operand(expr);
	if (WithoutParentheses(&expr)->facts->type->unqualified->kind == sema::TypeKind::MemberPointer)
		return member_pointer_truth(value);
	if (!value.type.is_float())
		return value;
	return compare(lowir::ComparePredicate::Ne, value.type, value,
		floating(value.type, lowir::float_literal(value.type.kind(), 0.0L)));
}

RValue FunctionLowering::truth_operand(const syntax::Expr& expr)
{
	// A condition's value before its conversion to bool (4.12 is the test
	// itself): a comparison's or logical operator's truth, else the
	// operand's scalar value.
	const syntax::Expr* e = WithoutParentheses(&expr);
	if (e->kind == syntax::ExprKind::Binary && BuiltinOperator(e))
	{
		const syntax::BinaryExpr* binary = static_cast<const syntax::BinaryExpr*>(e);
		if (IsComparisonOperator(binary->op) || binary->op == TokenKind::OP_LAND || binary->op == TokenKind::OP_LOR)
			return binary_expression(*binary);
	}
	// A binary16 or binary128 value's truth is its comparison with zero, a
	// complex value's its parts'.
	const RValue value = rvalue(*e);
	if (value.type.is_object() && e->facts->type->unqualified->kind == sema::TypeKind::Complex)
		return complex_truth(value, e->facts->type);
	const SoftFloat soft = value.type.is_object() ? SoftFloatOf(e->facts->type) : SoftFloat::None;
	if (soft != SoftFloat::None)
		return soft_compare(lowir::ComparePredicate::Ne, soft, value, soft_zero(soft));
	return value;
}

RValue FunctionLowering::normalized_truth(const syntax::Expr& expr)
{
	// The canonical 0 or 1 an operand of a value `&&`/`||` contributes:
	// a pointer's comparison with null, otherwise `cmp ne i64 v, 0` on its
	// truth (the course spelling, which compares at i64).
	const RValue value = truth_operand(expr);
	const lowir::Type i64 = lowir::Type::scalar(lowir::TypeKind::I64);
	if (value.type.is_pointer())
		return compare(lowir::ComparePredicate::Ne, value.type, value, integer(value.type, 0));
	// A pointer to member by its truth (an i128 is no operand of an i64
	// comparison, lowir.md cmp); a 128-bit integer is zero by both halves.
	if (WithoutParentheses(&expr)->facts->type->unqualified->kind == sema::TypeKind::MemberPointer)
		return member_pointer_null(value, false);
	if (value.type.kind() == lowir::TypeKind::I128)
		return compare(lowir::ComparePredicate::Ne, value.type, value, integer(value.type, 0));
	RValue truth_value = value;
	if (value.type.is_float())
		truth_value = compare(lowir::ComparePredicate::Ne, value.type, value,
			floating(value.type, lowir::float_literal(value.type.kind(), 0.0L)));
	return compare(lowir::ComparePredicate::Ne, i64, truth_value, integer(i64, 0));
}

RValue FunctionLowering::logical_value(const syntax::BinaryExpr& expr)
{
	// A left-nested chain of `&&` and `||` values, walked iteratively: the
	// innermost operator first, each outer one taking the value below as
	// its left operand's truth.
	const size_t base = spine_.size();
	const syntax::BinaryExpr* node = &expr;
	for (;;)
	{
		spine_.push_back(node);
		const syntax::Expr* left = WithoutParentheses(node->left);
		if (!IsLogical(left, TokenKind::OP_LAND) && !IsLogical(left, TokenKind::OP_LOR))
			break;
		node = static_cast<const syntax::BinaryExpr*>(left);
	}
	// 12.2p3: the right operands' temporaries end with the full-expression,
	// destroyed only if made. At its root the outermost operator's end
	// with its branch (nothing is evaluated after it); an inner operator's
	// are guarded, a later right operand being evaluated before them.
	RValue value = RValue();
	bool known = false;
	const bool local = branch_local(expr);
	const bool guards = !local || spine_.size() - base > 1;
	if (guards)
		begin_guard_scope();
	for (size_t i = spine_.size(); i-- > base;)
	{
		value = logical_step(*spine_[i], known ? &value : nullptr, local && i == base);
		known = true;
	}
	spine_.resize(base);
	if (guards)
		--guard_scopes_;
	return value;
}

RValue FunctionLowering::logical_step(const syntax::BinaryExpr& expr, const RValue* left_truth, bool local)
{
	// 5.14, 5.15 as a value: the right operand only when the left does not
	// decide; a literal left operand decides at compile time.
	const bool conjunction = expr.op == TokenKind::OP_LAND;
	const lowir::Type i64 = lowir::Type::scalar(lowir::TypeKind::I64);
	bool left_value = false;
	if (left_truth == nullptr && literal_truth(*expr.left, left_value))
	{
		// The deciding literal is the bool result itself.
		if (left_value != conjunction)
			return integer(lowir::Type::scalar(lowir::TypeKind::U8), left_value ? 1 : 0);
		return normalized_truth(*expr.right);
	}
	const lowir::SlotId slot = generated_slot(conjunction ? lowir::SlotKind::Land : lowir::SlotKind::Lor, i64);
	Place result = Place();
	result.storage = lowir::Value::slot(slot);
	const lowir::BlockId right = new_block(conjunction ? lowir::BlockKind::LandRhs : lowir::BlockKind::LorRhs);
	const lowir::BlockId shortcut = new_block(conjunction ? lowir::BlockKind::LandShort : lowir::BlockKind::LorShort);
	const lowir::BlockId end = new_block(conjunction ? lowir::BlockKind::LandEnd : lowir::BlockKind::LorEnd);
	const RValue left = left_truth != nullptr ? *left_truth : truth(*expr.left);
	if (conjunction)
		branch(left, right, shortcut);
	else
		branch(left, shortcut, right);
	place(right);
	const size_t first = cleanups_.size();
	if (!local)
		++guarded_branches_;
	store(i64, normalized_truth(*expr.right), result);
	if (!local)
		--guarded_branches_;
	else
		destroy_temporaries(first);
	jump(end);
	place(shortcut);
	store(i64, integer(i64, conjunction ? 0 : 1), result);
	jump(end);
	place(end);
	return load(i64, result);
}

bool FunctionLowering::continues_choice(const syntax::Expr& expr) const
{
	// An operand whose value conditional_value computes itself: a
	// conditional read as a scalar value. Not one a conversion function
	// converts (5.16p3: a class operand of a conditional whose other is
	// not), which is called on it as its object (choice_operand).
	const syntax::Expr* e = WithoutParentheses(&expr);
	const sema::ExprFacts* facts = e->facts;
	return e->kind == syntax::ExprKind::Conditional && facts != nullptr && facts->type != nullptr &&
		(facts->flags & (sema::kFolded | sema::kUserConverted)) == 0 && !conversion_function(facts->conversion) &&
		facts->type->kind != sema::TypeKind::Array && facts->type->kind != sema::TypeKind::Function &&
		!sema::IsVoid(facts->type);
}

bool FunctionLowering::unreachable_operand(const syntax::ConditionalExpr& expr, const syntax::Expr& operand)
{
	// An operand the analysis marks as never evaluated (sema's
	// kThenUnreachable, kElseUnreachable): the temporaries it would destroy
	// at its branch's end (at the full-expression's root) get no cleanups.
	const uint32_t flags = expr.facts->flags;
	return (&operand == expr.then_expr && (flags & sema::kThenUnreachable) != 0) ||
		(&operand == expr.else_expr && (flags & sema::kElseUnreachable) != 0);
}

RValue FunctionLowering::conditional_value(const syntax::ConditionalExpr& expr)
{
	// 5.16 for a prvalue: each branch stores its converted operand in the
	// result slot; the end block reads it. A chain of conditionals nested
	// in one operand position (`a ? x : b ? y : ...` or `a ? b ? x : y :
	// z`, as long as the source makes it) is walked iteratively: each level
	// descends into its nested operand's block, and the levels finish
	// innermost first, each storing its value into the enclosing level's
	// slot, in the block order of a recursive walk.
	bool selected = false;
	if (sema::ClassOf(expr.facts->type) == nullptr && lowering_.constant_conversion(*expr.condition, selected))
		return fit(operand(selected ? *expr.then_expr : *expr.else_expr, ConversionContext::Operand), expr.facts->type);
	const size_t base = choices_.size();
	const syntax::ConditionalExpr* current = &expr;
	const bool local = branch_local(expr);
	// A nested level's condition is evaluated in its enclosing level's
	// branch: its temporaries are guarded (12.2p3).
	const bool guards = !local || continues_choice(*expr.then_expr) || continues_choice(*expr.else_expr);
	if (guards)
		begin_guard_scope();
	for (;;)
	{
		check_stack(current->location);
		const sema::Type* type = current->facts->type;
		Choice level = Choice();
		level.expr = current;
		level.local = local;
		level.storage = lowering_.storage_type(type);
		level.slot.storage = lowir::Value::slot(generated_slot(lowir::SlotKind::Cond, level.storage));
		const lowir::BlockId then_block = new_block(lowir::BlockKind::CondThen);
		level.else_block = new_block(lowir::BlockKind::CondElse);
		level.end = new_block(lowir::BlockKind::CondEnd);
		const bool nested = current != &expr;
		if (nested)
			++guarded_branches_;
		const RValue condition = truth(*current->condition);
		if (nested)
			--guarded_branches_;
		branch(condition, then_block, level.else_block);
		place(then_block);
		if (continues_choice(*current->then_expr))
		{
			level.nested_then = true;
			level.unreachable_nested = local && unreachable_operand(*current, *current->then_expr);
			unreachable_operands_ += level.unreachable_nested ? 1 : 0;
			choices_.push_back(level);
			current = static_cast<const syntax::ConditionalExpr*>(WithoutParentheses(current->then_expr));
			continue;
		}
		const uint32_t opened = regions_opened_;
		choice_operand(*current->then_expr, level);
		if (!terminated_)
			jump(level.end);
		place(level.else_block);
		choices_.push_back(level);
		if (continues_choice(*current->else_expr))
		{
			choices_.back().unreachable_nested = local && unreachable_operand(*current, *current->else_expr);
			unreachable_operands_ += choices_.back().unreachable_nested ? 1 : 0;
			current = static_cast<const syntax::ConditionalExpr*>(WithoutParentheses(current->else_expr));
			continue;
		}
		// A region the first operand's temporaries opened opens again at
		// the second's start (the course's form), when it has calls or
		// temporaries (under hidden regions).
		const bool reopened = full_expression_ && !region_open_ && regions_opened_ != opened && !tracked_.active &&
			(!hidden_eh_ || may_throw(current->else_expr) || creates_temporaries(current->else_expr));
		if (reopened)
		{
			open_region(true, true);
			++reopened_choices_;
		}
		choice_operand(*current->else_expr, level);
		if (reopened)
			--reopened_choices_;
		if (reopened && region_open_)
			close_region();
		if (!terminated_)
			jump(level.end);
		break;
	}
	Choice inner = choices_.back();
	place(inner.end);
	RValue value = load(inner.storage, inner.slot);
	for (size_t i = choices_.size() - 1; i > base; --i)
	{
		const Choice outer = choices_[i - 1];
		const sema::Type* type = outer.expr->facts->type;
		const syntax::Expr& nested = outer.nested_then ? *outer.expr->then_expr : *outer.expr->else_expr;
		store(outer.storage, fit(apply_conversion(nested, value, ConversionContext::Operand), type), outer.slot);
		unreachable_operands_ -= outer.unreachable_nested ? 1 : 0;
		jump(outer.end);
		if (outer.nested_then)
		{
			place(outer.else_block);
			choice_operand(*outer.expr->else_expr, outer);
			jump(outer.end);
		}
		place(outer.end);
		value = load(outer.storage, outer.slot);
	}
	conditional_slot_ = choices_[base].slot.storage;
	choices_.resize(base);
	if (guards)
		--guard_scopes_;
	return value;
}

void FunctionLowering::choice_operand(const syntax::Expr& operand_expr, const Choice& level)
{
	// A selected operand's converted value in the level's slot; the
	// temporaries it made end with its branch, or (guarded) with the
	// full-expression.
	const size_t first = cleanups_.size();
	const uint32_t unreachable = level.local && unreachable_operand(*level.expr, operand_expr) ? 1 : 0;
	unreachable_operands_ += unreachable;
	if (!level.local)
		++guarded_branches_;
	// 5.16p2: a throw-expression operand yields no value.
	if (WithoutParentheses(&operand_expr)->kind == syntax::ExprKind::Throw)
		discard(operand_expr);
	else
		store(
			level.storage, fit(operand(operand_expr, ConversionContext::Operand), level.expr->facts->type), level.slot);
	unreachable_operands_ -= unreachable;
	if (!level.local)
		--guarded_branches_;
	else
		destroy_temporaries(first);
}

Place FunctionLowering::conditional_place(const syntax::ConditionalExpr& expr)
{
	// 5.16p4 for glvalues: each branch stores its operand's address. The
	// result may designate (a subobject of) a branch's temporary, which
	// lives to the full-expression's end, guarded (5.16p1: only one operand
	// is evaluated; 12.2p3).
	const lowir::Type pointer = lowir::Type::scalar(lowir::TypeKind::Ptr);
	Place result = Place();
	result.storage = lowir::Value::slot(generated_slot(lowir::SlotKind::Condaddr, pointer));
	const lowir::BlockId then_block = new_block(lowir::BlockKind::CondaddrThen);
	const lowir::BlockId else_block = new_block(lowir::BlockKind::CondaddrElse);
	const lowir::BlockId end = new_block(lowir::BlockKind::CondaddrEnd);
	begin_guard_scope();
	branch(truth(*expr.condition), then_block, else_block);
	place(then_block);
	// An operand of a derived class is converted to the base class the
	// result has (5.16p3).
	const sema::ClassEntity* common = sema::ClassOf(expr.facts->type);
	RValue address_value;
	address_value.type = pointer;
	// A throw-expression operand (5.16p2, CWG 1550) ends its branch.
	for (const syntax::Expr* operand : { expr.then_expr, expr.else_expr })
	{
		if (operand == expr.else_expr)
			place(else_block);
		++guarded_branches_;
		if (WithoutParentheses(operand)->kind == syntax::ExprKind::Throw)
			discard(*operand);
		else
		{
			address_value.value = address(*operand);
			if (common != nullptr)
				address_value.value = to_base(
					address_value.value, sema::ClassOf(operand->facts->type), common, expr.location);
			store(pointer, address_value, result);
		}
		--guarded_branches_;
		if (!terminated_)
			jump(end);
	}
	place(end);
	--guard_scopes_;
	Place place_result = Place();
	place_result.storage = load(pointer, result).value;
	qualify(place_result, expr.facts->type);
	return place_result;
}

void FunctionLowering::conditional_discard(const syntax::ConditionalExpr& expr)
{
	// A void conditional: each branch evaluates its operand for effects.
	// Its temporaries end with the branch at the full-expression's root,
	// else with the full-expression, guarded (5.16p1, 12.2p3).
	const lowir::BlockId then_block = new_block(lowir::BlockKind::DiscardCondThen);
	const lowir::BlockId else_block = new_block(lowir::BlockKind::DiscardCondElse);
	const lowir::BlockId end = new_block(lowir::BlockKind::DiscardCondEnd);
	const bool local = branch_local(expr);
	if (!local)
		begin_guard_scope();
	branch(truth(*expr.condition), then_block, else_block);
	const syntax::Expr* operands[2] = { expr.then_expr, expr.else_expr };
	const lowir::BlockId blocks[2] = { then_block, else_block };
	for (int i = 0; i < 2; ++i)
	{
		place(blocks[i]);
		const size_t first = cleanups_.size();
		const uint32_t unreachable = local && unreachable_operand(expr, *operands[i]) ? 1 : 0;
		unreachable_operands_ += unreachable;
		if (!local)
			++guarded_branches_;
		discard(*operands[i]);
		unreachable_operands_ -= unreachable;
		if (!local)
			--guarded_branches_;
		else
			destroy_temporaries(first);
		jump(end);
	}
	place(end);
	if (!local)
		--guard_scopes_;
}

} // namespace lower
