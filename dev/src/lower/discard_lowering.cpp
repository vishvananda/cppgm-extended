#include "lower/function_lowering.h"

#include "sema/conversions.h"
#include "sema/expression_facts.h"

// Discarded-value expressions (5p10, 6.2p1): evaluated for their side
// effects only. A volatile glvalue of the 5p11 forms is still read;
// operator calls, user-defined literals and constructions still run;
// a pure operand lowers to nothing.
namespace lower {

namespace {

const syntax::Expr* WithoutParentheses(const syntax::Expr* expr)
{
	while (expr->kind == syntax::ExprKind::Paren)
		expr = static_cast<const syntax::ParenExpr*>(expr)->inner;
	return expr;
}

} // namespace

bool FunctionLowering::read_when_discarded(const syntax::Expr& expr)
{
	// 5p11: the forms a discarded volatile lvalue is read in: a
	// (parenthesized) id-expression, subscript, member access, indirection
	// or pointer-to-member operation, a conditional whose second and third
	// operands are such forms, a comma whose right operand is (built-in
	// operators only: an overloaded one is a call).
	const syntax::Expr* e = WithoutParentheses(&expr);
	check_stack(e->location);
	if (e->facts != nullptr && (e->facts->flags & sema::kOperatorCall) != 0)
		return false;
	switch (e->kind)
	{
	case syntax::ExprKind::Id:
	case syntax::ExprKind::Subscript:
	case syntax::ExprKind::Member:
		return true;
	case syntax::ExprKind::Unary:
		return static_cast<const syntax::UnaryExpr*>(e)->op == TokenKind::OP_STAR;
	case syntax::ExprKind::Conditional:
	{
		const syntax::ConditionalExpr* conditional = static_cast<const syntax::ConditionalExpr*>(e);
		return read_when_discarded(*conditional->then_expr) && read_when_discarded(*conditional->else_expr);
	}
	case syntax::ExprKind::Binary:
	{
		const syntax::BinaryExpr* binary = static_cast<const syntax::BinaryExpr*>(e);
		if (binary->op == TokenKind::OP_DOTSTAR || binary->op == TokenKind::OP_ARROWSTAR)
			return true;
		return binary->op == TokenKind::OP_COMMA && read_when_discarded(*binary->right);
	}
	default:
		return false;
	}
}

void FunctionLowering::read_discarded(const syntax::Expr& expr)
{
	// A discarded volatile lvalue in read_when_discarded's forms: the
	// selected operand of a conditional or a comma's right operand is read,
	// as the volatile glvalue the whole expression is.
	const syntax::Expr* e = WithoutParentheses(&expr);
	check_stack(e->location);
	if (e->kind == syntax::ExprKind::Conditional)
	{
		// Its operands' temporaries are guarded (5.16p1, 12.2p3).
		const syntax::ConditionalExpr& conditional = static_cast<const syntax::ConditionalExpr&>(*e);
		const lowir::BlockId then_block = new_block(lowir::BlockKind::DiscardCondThen);
		const lowir::BlockId else_block = new_block(lowir::BlockKind::DiscardCondElse);
		const lowir::BlockId end = new_block(lowir::BlockKind::DiscardCondEnd);
		begin_guard_scope();
		branch(truth(*conditional.condition), then_block, else_block);
		place(then_block);
		++guarded_branches_;
		read_discarded(*conditional.then_expr);
		--guarded_branches_;
		jump(end);
		place(else_block);
		++guarded_branches_;
		read_discarded(*conditional.else_expr);
		--guarded_branches_;
		jump(end);
		place(end);
		--guard_scopes_;
		return;
	}
	if (e->kind == syntax::ExprKind::Binary && static_cast<const syntax::BinaryExpr*>(e)->op == TokenKind::OP_COMMA)
	{
		const syntax::BinaryExpr& comma = static_cast<const syntax::BinaryExpr&>(*e);
		discard(*comma.left);
		read_discarded(*comma.right);
		return;
	}
	// A bit-field reads its unit (volatile as the field's type is).
	if (e->facts != nullptr && (e->facts->flags & sema::kBitField) != 0)
	{
		bit_field_value(*e);
		return;
	}
	Place place = glvalue(*e);
	place.is_volatile = true;
	load(lowering_.storage_type(e->facts->type), place);
}

void FunctionLowering::discard(const syntax::Expr& expr)
{
	// 5p10: a discarded-value expression is evaluated for its effects; a
	// volatile lvalue in one of read_when_discarded's forms is also read
	// (the lvalue-to-rvalue conversion is applied).
	const syntax::Expr* e = WithoutParentheses(&expr);
	check_stack(e->location);
	const DebugScope debug(*this, e->location);
	const sema::ExprFacts* facts = e->facts;
	if (facts != nullptr && (facts->flags & sema::kOperatorCall) != 0 && e->kind != syntax::ExprKind::Call &&
		(facts->category != sema::ValueCategory::PRValue || facts->type->unqualified->kind != sema::TypeKind::Class))
	{
		operator_call(*e);
		return;
	}
	if (facts != nullptr && (facts->flags & sema::kBitField) != 0 &&
		(e->kind == syntax::ExprKind::Assign || e->kind == syntax::ExprKind::Unary))
	{
		bit_field_value(*e);
		return;
	}
	if (facts != nullptr && (facts->flags & (sema::kFolded | sema::kStaticConstant)) != 0 &&
		e->kind == syntax::ExprKind::Member)
	{
		member_object(static_cast<const syntax::MemberExpr&>(*e));
		return;
	}
	if (facts != nullptr && e->kind == syntax::ExprKind::Postfix)
	{
		const sema::ExprFacts* operand = WithoutParentheses(static_cast<const syntax::UnaryExpr*>(e)->operand)->facts;
		if (operand != nullptr && (operand->flags & sema::kBitField) != 0)
		{
			bit_field_increment(static_cast<const syntax::UnaryExpr&>(*e), false);
			return;
		}
	}
	if (facts != nullptr && facts->type != nullptr && facts->category == sema::ValueCategory::LValue &&
		(facts->type->cv & sema::kVolatile) != 0 && facts->type->kind != sema::TypeKind::Array &&
		read_when_discarded(*e))
	{
		read_discarded(*e);
		return;
	}
	// An operator function's class prvalue result (the operator calls not
	// taken above) is a temporary whatever the operator's form: `a = b`,
	// `a += b`, `a[i]`, `*a`, `++a` returning a class by value are
	// materialized and destroyed below, not designated as glvalues.
	const bool operator_prvalue = facts != nullptr && (facts->flags & sema::kOperatorCall) != 0 &&
		e->kind != syntax::ExprKind::Call;
	switch (e->kind)
	{
	case syntax::ExprKind::Throw:
		throw_expression(static_cast<const syntax::ThrowExpr&>(*e));
		return;
	case syntax::ExprKind::FunctionalCast:
	{
		// `A{...}` of an array type: a temporary array its list initializes.
		const syntax::FunctionalCastExpr& functional = static_cast<const syntax::FunctionalCastExpr&>(*e);
		if (functional.braced == nullptr || facts == nullptr || facts->type->kind != sema::TypeKind::Array)
			break;
		array_temporary(functional, false, lowir::SlotKind::Discardarr);
		return;
	}
	case syntax::ExprKind::Assign:
	case syntax::ExprKind::Subscript:
	case syntax::ExprKind::Member:
		if (operator_prvalue)
			break;
		glvalue(*e);
		return;
	case syntax::ExprKind::TypeTrait:
		// A typeid's evaluated operand, and its object's table read.
		if (static_cast<const syntax::TypeTraitExpr&>(*e).keyword == TokenKind::KW_TYPEID)
		{
			if (facts != nullptr && (facts->flags & sema::kDynamicType) != 0)
				glvalue(*e);
			return;
		}
		// Fall through: alignof and noexcept are size queries.
	case syntax::ExprKind::BuiltinTrait:
		// __builtin_va_arg still reads (and advances past) its argument; a
		// complex part, vector conversion or bit cast evaluates its operand.
		if (e->kind == syntax::ExprKind::BuiltinTrait)
		{
			const GnuTrait trait = static_cast<const syntax::BuiltinTraitExpr&>(*e).trait;
			if (trait == GnuTrait::VaArg || trait == GnuTrait::ConvertVector || trait == GnuTrait::BitCast)
				operand(*e, ConversionContext::Operand);
			else if (trait == GnuTrait::RealPart || trait == GnuTrait::ImagPart)
				discard(*static_cast<const syntax::BuiltinTraitExpr&>(*e).operand);
		}
		return;
	case syntax::ExprKind::Sizeof:
	case syntax::ExprKind::SizeofPack:
		// A discarded size query is still materialized (the course's form).
		if (facts != nullptr && facts->type != nullptr)
			size_query(*e);
		return;
	case syntax::ExprKind::Unary:
	{
		const TokenKind op = static_cast<const syntax::UnaryExpr&>(*e).op;
		if ((op == TokenKind::OP_INC || op == TokenKind::OP_DEC || op == TokenKind::OP_STAR) && !operator_prvalue)
		{
			glvalue(*e);
			return;
		}
		break;
	}
	case syntax::ExprKind::Binary:
	{
		const syntax::BinaryExpr& binary = static_cast<const syntax::BinaryExpr&>(*e);
		// An overloaded comma's class prvalue result is a temporary (below).
		if (binary.op == TokenKind::OP_COMMA && !operator_prvalue)
		{
			// A left-nested comma chain, walked iteratively: the innermost
			// left operand, then each right operand upward.
			const size_t base = spine_.size();
			const syntax::BinaryExpr* node = &binary;
			for (;;)
			{
				spine_.push_back(node);
				const syntax::Expr* left = WithoutParentheses(node->left);
				if (left->kind != syntax::ExprKind::Binary ||
					static_cast<const syntax::BinaryExpr*>(left)->op != TokenKind::OP_COMMA)
					break;
				node = static_cast<const syntax::BinaryExpr*>(left);
			}
			discard(*spine_.back()->left);
			for (size_t i = spine_.size(); i-- > base;)
				discard(*spine_[i]->right);
			spine_.resize(base);
			return;
		}
		break;
	}
	case syntax::ExprKind::Conditional:
		if (sema::IsVoid(facts->type))
		{
			conditional_discard(static_cast<const syntax::ConditionalExpr&>(*e));
			return;
		}
		break;
	case syntax::ExprKind::Call:
		// An expanded builtin (va_start, va_copy ... have no value) is
		// emitted for its effects.
		if ((facts->flags & sema::kIntrinsic) != 0)
		{
			intrinsic(static_cast<const syntax::CallExpr&>(*e));
			return;
		}
		if (static_cast<const syntax::CallExpr&>(*e).callee->facts != nullptr &&
			(facts->category != sema::ValueCategory::PRValue ||
				facts->type->unqualified->kind != sema::TypeKind::Class))
		{
			call(static_cast<const syntax::CallExpr&>(*e));
			return;
		}
		break;
	case syntax::ExprKind::Cast:
		if (sema::IsVoid(facts->type))
		{
			discard_to_void(*static_cast<const syntax::CastExpr&>(*e).operand);
			return;
		}
		break;
	case syntax::ExprKind::Literal:
		// A user-defined literal is its literal operator's call (2.14.8p1):
		// a class prvalue result is a temporary and a reference designates
		// its referent (below); another (void too) is the call's value (a
		// folded constant's has no effects).
		if ((facts->flags & sema::kUserLiteral) != 0 && (facts->flags & sema::kFolded) == 0)
		{
			if (facts->category != sema::ValueCategory::PRValue ||
				facts->type->unqualified->kind == sema::TypeKind::Class)
				break;
			literal(static_cast<const syntax::LiteralExpr&>(*e));
		}
		return;
	case syntax::ExprKind::Id:
	case syntax::ExprKind::KeywordLiteral:
		return;
	case syntax::ExprKind::Delete:
		deletion(static_cast<const syntax::DeleteExpr&>(*e));
		return;
	case syntax::ExprKind::Statement:
		if (facts->type->unqualified->kind != sema::TypeKind::Class)
		{
			statement_expression(static_cast<const syntax::StatementExpr&>(*e), lowir::Value());
			return;
		}
		break;
	default:
		break;
	}
	if (facts->category != sema::ValueCategory::PRValue)
	{
		glvalue(*e);
		return;
	}
	if (facts->type->unqualified->kind == sema::TypeKind::Class)
	{
		// 6.2p1, 12.2p1: a discarded class prvalue initializes a temporary;
		// with objects alive (or tracked), a call opens the region itself
		// (after the temporary's address).
		const bool destroyed = destroyed_temporary(facts->type);
		if (destroyed && (cleanups_.empty() || !CallResult(*e, facts)) && !tracked_.active)
			temporary_region();
		Place discarded = Place();
		discarded.storage = lowir::Value::slot(
			generated_slot(lowir::SlotKind::Discard, lowering_.storage_type(facts->type)));
		const lowir::Value object = address_of(discarded);
		open_deferred_region();
		materialize(*e, object, e->location);
		if (destroyed)
			temporary_alive(object, facts->type);
		else
			temporary_done();
		return;
	}
	if (!sema::IsVoid(facts->type))
		rvalue(*e);
}

void FunctionLowering::discard_to_void(const syntax::Expr& expr)
{
	// 5.2.9p6, 5.4: an operand cast to void is a discarded-value expression
	// (5p11). A volatile scalar glvalue is read exactly as 5p11 says
	// (discard). Another is evaluated as the course reads it (behaviour-
	// neutral for non-volatile objects): a named object or reference
	// designates its storage (a scalar is read, a class object addressed);
	// so does another scalar glvalue (a subscript, member, indirection,
	// reference result, cast, conditional or comma's right operand is
	// read), but an assignment's or prefix increment's, which designates the
	// object it just wrote; other operands are evaluated for their effects.
	const syntax::Expr* e = WithoutParentheses(&expr);
	const sema::ExprFacts* facts = e->facts;
	if (facts != nullptr && facts->type != nullptr && (facts->type->cv & sema::kVolatile) != 0 &&
		facts->type->unqualified->kind != sema::TypeKind::Class && facts->type->kind != sema::TypeKind::Array)
	{
		discard(*e);
		return;
	}
	const bool writes = e->kind == syntax::ExprKind::Assign ||
		(e->kind == syntax::ExprKind::Unary &&
			(static_cast<const syntax::UnaryExpr*>(e)->op == TokenKind::OP_INC ||
				static_cast<const syntax::UnaryExpr*>(e)->op == TokenKind::OP_DEC));
	if (e->kind != syntax::ExprKind::Id && !writes && facts != nullptr && facts->type != nullptr &&
		facts->category != sema::ValueCategory::PRValue && facts->type->unqualified->kind != sema::TypeKind::Class &&
		facts->type->kind != sema::TypeKind::Array && facts->type->kind != sema::TypeKind::Function &&
		!sema::IsVoid(facts->type))
	{
		rvalue(*e);
		return;
	}
	if (e->kind == syntax::ExprKind::Id && facts != nullptr && facts->type != nullptr &&
		facts->category == sema::ValueCategory::LValue && facts->entity != nullptr &&
		(facts->entity->kind == sema::EntityKind::Variable || facts->entity->kind == sema::EntityKind::Parameter) &&
		(facts->flags & (sema::kStaticConstant | sema::kBitField)) == 0)
	{
		const sema::Type* type = facts->type;
		if (type->unqualified->kind == sema::TypeKind::Class || type->kind == sema::TypeKind::Array)
			address(*e);
		else
			load_glvalue(*e);
		return;
	}
	discard(*e);
}

void FunctionLowering::imaged_discard_slots(const syntax::Expr& expr, uint32_t depth)
{
	// An array copied from its constant image evaluates no element, but
	// the storage of each class prvalue its elements discard (a comma's
	// left operand, 6.2p1, 12.2p1) is still planned with the variable (the
	// course's form).
	const syntax::Expr* e = WithoutParentheses(&expr);
	if (depth > 64)
		return;
	if (e->kind == syntax::ExprKind::BracedInit)
	{
		for (const syntax::Expr* element : static_cast<const syntax::BracedInitList*>(e)->elements)
			imaged_discard_slots(*element, depth + 1);
		return;
	}
	if (e->kind != syntax::ExprKind::Binary || static_cast<const syntax::BinaryExpr*>(e)->op != TokenKind::OP_COMMA)
		return;
	const syntax::BinaryExpr* comma = static_cast<const syntax::BinaryExpr*>(e);
	const syntax::Expr* left = WithoutParentheses(comma->left);
	const sema::ExprFacts* facts = left->facts;
	if (facts != nullptr && facts->type != nullptr && facts->category == sema::ValueCategory::PRValue &&
		facts->type->unqualified->kind == sema::TypeKind::Class)
		generated_slot(lowir::SlotKind::Discard, lowering_.storage_type(facts->type));
	else
		imaged_discard_slots(*left, depth + 1);
	imaged_discard_slots(*comma->right, depth + 1);
}

} // namespace lower
