#include "lower/function_lowering.h"

#include <algorithm>

#include "lowir/literals.h"
#include "sema/conversions.h"
#include "sema/statement_facts.h"
#include "sema/template_records.h"

// Cleanups on the exceptional and shared paths (15.2, 6.6p2, 12.6.2,
// 12.4p8): an automatic object whose construction completed is destroyed
// when a call that may throw unwinds through its scope; a constructor
// destroys its constructed subobjects, and a destructor the subobjects it
// has not yet destroyed, when unwinding leaves them. Returns with the same
// objects alive share one continuation holding the value.
//
// The LowIR handler stack is dynamic (lowir.md, PA17): `eh_try` pushes a
// handler that is popped when entered, `eh_cleanup` one that stays until
// its block's `eh_end`, and `resume` continues to the next handler. A call
// that may throw with automatic objects alive runs in an `eh_try` region
// whose handler destroys them innermost first (one block per object,
// interned by the object: every object's predecessors are lexically fixed)
// and resumes.
namespace lower {

namespace {

const syntax::Expr* WithoutParentheses(const syntax::Expr* expr)
{
	while (expr->kind == syntax::ExprKind::Paren)
		expr = static_cast<const syntax::ParenExpr*>(expr)->inner;
	return expr;
}

} // namespace

bool FunctionLowering::CallResult(const syntax::Expr& e, const sema::ExprFacts* facts)
{
	// A function's (or operator function's, or literal operator's) call,
	// not a constructor's.
	if ((facts->flags & (sema::kOperatorCall | sema::kUserLiteral)) != 0)
		return true;
	if (e.kind != syntax::ExprKind::Call || (facts->flags & (sema::kIntrinsic | sema::kPseudoDestructor)) != 0)
		return false;
	const sema::Entity* entity = facts->entity;
	return entity == nullptr || entity->kind != sema::EntityKind::Function ||
		!static_cast<const sema::FunctionEntity*>(entity)->constructor;
}

namespace {

bool MemberTemplateSpecialization(const sema::ClassEntity* cls)
{
	// A specialization of a class template that is a class's member.
	const sema::ClassTemplateFacts* facts = cls != nullptr ? cls->templ : nullptr;
	return facts != nullptr && facts->role == sema::TemplateRole::Specialization && facts->pattern != nullptr &&
		facts->pattern->owner != nullptr && facts->pattern->owner->kind == sema::ScopeKind::Class;
}

bool NonInlineDestructor(const sema::FunctionEntity* destructor)
{
	// A destructor defined out of line (or elsewhere) has a base entry of
	// its own, as a virtual destructor does; another inline one's is an
	// alias of its complete entry, as an instantiated one's is (emitted
	// where odr-used, the course reference's form).
	return (!destructor->implicit && !destructor->defaulted && !destructor->is_inline &&
			   (!destructor->instantiated || destructor->explicit_specialization)) ||
		destructor->is_virtual;
}

} // namespace

// ------------------------------------------------------ throwing calls

bool FunctionLowering::call_may_throw(const sema::FunctionEntity* function) const
{
	// 15.4p12: a function without a non-throwing exception-specification
	// (destructors have one implicitly, 15.4p14).
	return function != nullptr && function->builtin == sema::BuiltinFunction::None &&
		((hidden_eh_ && !function->destructor) || !lowering_.function_nothrow(function));
}

bool FunctionLowering::may_throw(const syntax::Expr* root)
{
	// Whether evaluating the expression calls a function that may throw:
	// a direct call, an overloaded operator, a constructor, an allocation
	// function, a user-defined literal or a call through a pointer;
	// unevaluated operands excluded. A work list, so chains of any length
	// take constant stack.
	std::vector<const syntax::Expr*>& work = throw_scan_;
	const size_t base = work.size();
	work.push_back(root);
	bool found = false;
	while (!found && work.size() > base)
	{
		const syntax::Expr* expr = work.back();
		work.pop_back();
		if (expr == nullptr)
			continue;
		const syntax::Expr* e = WithoutParentheses(expr);
		const sema::ExprFacts* facts = e->facts;
		if (facts != nullptr && (facts->flags & sema::kFolded) != 0)
			continue;
		const sema::Conversion* conversion = facts != nullptr ? facts->conversion : nullptr;
		if (conversion != nullptr &&
			(call_may_throw(conversion->constructor) ||
				(conversion->after != nullptr && call_may_throw(conversion->after->constructor))))
		{
			found = true;
			break;
		}
		if (facts != nullptr && (facts->flags & sema::kUserConverted) != 0)
		{
			// 13.3.1.2p7, 13.3.1.5, 6.4p4: a class operand rewritten as its
			// conversion function's call, then the operand's own evaluation.
			if (call_may_throw(static_cast<const sema::FunctionEntity*>(facts->entity)))
			{
				found = true;
				break;
			}
			facts = facts->original;
		}
		const bool operator_call = facts != nullptr && (facts->flags & sema::kOperatorCall) != 0 &&
			call_may_throw(static_cast<const sema::FunctionEntity*>(facts->entity));
		switch (e->kind)
		{
		case syntax::ExprKind::Literal:
			found = facts != nullptr && (facts->flags & sema::kUserLiteral) != 0 &&
				call_may_throw(static_cast<const sema::FunctionEntity*>(facts->entity));
			break;
		case syntax::ExprKind::Throw:
		// A statement expression's statements are not walked here.
		case syntax::ExprKind::Statement:
			found = true;
			break;
		case syntax::ExprKind::Binary:
		case syntax::ExprKind::Assign:
			found = operator_call;
			work.push_back(static_cast<const syntax::BinaryExpr*>(e)->right);
			work.push_back(static_cast<const syntax::BinaryExpr*>(e)->left);
			break;
		case syntax::ExprKind::Conditional:
		{
			const syntax::ConditionalExpr* choice = static_cast<const syntax::ConditionalExpr*>(e);
			work.push_back(choice->else_expr);
			work.push_back(choice->then_expr);
			work.push_back(choice->condition);
			break;
		}
		case syntax::ExprKind::Unary:
		case syntax::ExprKind::Postfix:
			found = operator_call;
			work.push_back(static_cast<const syntax::UnaryExpr*>(e)->operand);
			break;
		case syntax::ExprKind::Call:
		{
			const syntax::CallExpr* call = static_cast<const syntax::CallExpr*>(e);
			if (facts != nullptr && (facts->flags & (sema::kIntrinsic | sema::kPseudoDestructor)) == 0)
			{
				if ((facts->flags & sema::kDirectCall) == 0)
					found = true;
				else
				{
					const sema::FunctionEntity* function = static_cast<const sema::FunctionEntity*>(facts->entity);
					found = call_may_throw(function);
					if (function->default_arguments != nullptr)
					{
						for (uint32_t i = call->arguments.size; i < function->type->canonical->param_count; ++i)
							work.push_back(lowering_.default_argument(function, i));
					}
				}
			}
			// A member call's object expression; another callee (a
			// pointer-to-member call's `.*`, a function pointer's operand).
			const syntax::Expr* callee = WithoutParentheses(call->callee);
			work.push_back(callee->kind == syntax::ExprKind::Member
					? static_cast<const syntax::MemberExpr*>(callee)->base
					: callee);
			for (const syntax::Expr* argument : call->arguments)
				work.push_back(argument);
			break;
		}
		case syntax::ExprKind::FunctionalCast:
		{
			const syntax::FunctionalCastExpr* cast = static_cast<const syntax::FunctionalCastExpr*>(e);
			found = facts != nullptr && facts->entity != nullptr && facts->entity->kind == sema::EntityKind::Function &&
				call_may_throw(static_cast<const sema::FunctionEntity*>(facts->entity));
			for (const syntax::Expr* argument : cast->arguments)
				work.push_back(argument);
			work.push_back(cast->braced);
			break;
		}
		case syntax::ExprKind::Subscript:
			work.push_back(static_cast<const syntax::SubscriptExpr*>(e)->index);
			work.push_back(static_cast<const syntax::SubscriptExpr*>(e)->base);
			break;
		case syntax::ExprKind::Member:
			work.push_back(static_cast<const syntax::MemberExpr*>(e)->base);
			break;
		case syntax::ExprKind::TypeTrait:
			// A typeid's operand is evaluated only for a dynamic type (5.2.8p2).
			if (facts != nullptr && (facts->flags & sema::kDynamicType) != 0)
				work.push_back(static_cast<const syntax::TypeTraitExpr*>(e)->operand);
			break;
		case syntax::ExprKind::Cast:
			// A cast to a class type runs the constructor it selects.
			found = facts != nullptr && facts->category == sema::ValueCategory::PRValue && facts->entity != nullptr &&
				facts->entity->kind == sema::EntityKind::Function &&
				call_may_throw(static_cast<const sema::FunctionEntity*>(facts->entity));
			work.push_back(static_cast<const syntax::CastExpr*>(e)->operand);
			break;
		case syntax::ExprKind::BracedInit:
			// A list that initializes a class object by a constructor
			// (8.5.4p3, 13.3.1.7) calls it.
			found = facts != nullptr && facts->entity != nullptr && facts->entity->kind == sema::EntityKind::Function &&
				call_may_throw(static_cast<const sema::FunctionEntity*>(facts->entity));
			for (const syntax::Expr* element : static_cast<const syntax::BracedInitList*>(e)->elements)
				work.push_back(element);
			break;
		case syntax::ExprKind::Delete:
		{
			// The destructor (the deallocation functions do not throw,
			// 18.6.1.1).
			const sema::ClassEntity* entity = facts != nullptr ? sema::ClassOf(facts->target) : nullptr;
			found = entity != nullptr && entity->destructor != nullptr && !entity->destructor->trivial &&
				call_may_throw(entity->destructor);
			work.push_back(static_cast<const syntax::DeleteExpr*>(e)->operand);
			break;
		}
		case syntax::ExprKind::New:
		{
			// (And a host object's invalid array count, 5.3.4p7.)
			const syntax::NewExpr* allocation = static_cast<const syntax::NewExpr*>(e);
			found = call_may_throw(static_cast<const sema::FunctionEntity*>(facts->entity)) ||
				call_may_throw(allocation->constructor) || length_may_throw(*allocation);
			for (const syntax::Expr* argument : allocation->placement)
				work.push_back(argument);
			work.push_back(allocation->count);
			const syntax::Initializer* init = allocation->initializer;
			if (init != nullptr && init->kind == syntax::InitializerKind::Parenthesized)
			{
				for (const syntax::Expr* argument : init->arguments)
					work.push_back(argument);
			}
			else if (init != nullptr)
				work.push_back(init->expression);
			break;
		}
		default:
			break;
		}
	}
	work.resize(base);
	return found;
}

// ------------------------------------------------------------- regions

void FunctionLowering::emit_marker(lowir::Opcode op, lowir::BlockId target)
{
	lowir::Instruction inst = lowir::make_instruction(op, lowir::Type::scalar(lowir::TypeKind::Void));
	if (target != lowir::kNoId)
		inst.ops[0] = lowir::Value::block(target);
	emit(inst);
	++markers_;
}

bool FunctionLowering::begin_full_expression(const syntax::Expr* expr, bool discarded, bool throws)
{
	// A full-expression evaluated while automatic objects are alive (or
	// that makes temporaries it destroys) that may throw: its region opens
	// at its start once a throwing call (or a temporary) is met. One that
	// may not throw is a segment too when it materializes a call's class
	// prvalue as a temporary (its root's when `discarded`): the course's
	// form (see nothrow_segment_).
	const bool context = hidden_context_;
	hidden_context_ = false;
	if (full_expression_ || expr == nullptr)
		return false;
	const bool temporaries = forced_temporaries_ || creates_temporaries(expr);
	// The objects under the innermost try's floor are its dispatch's to
	// destroy: a full-expression with none above is no segment.
	if (cleanups_.size() <= region_floor() && !temporaries)
		return false;
	segment_temporaries_ = temporaries;
	hidden_eh_ = temporaries && (context || branch_temporaries(expr, 0));
	const bool throwing = throws || may_throw(expr);
	if (throwing || forced_temporaries_)
	{
		if (!throwing)
			hidden_eh_ = false;
		begin_segment();
		if (temporaries)
			begin_tracking(&expr, 1, true, discarded);
		return true;
	}
	hidden_eh_ = false;
	CallTemporaries calls = CallTemporaries{ discarded, false, false };
	if (!temporaries || !creates_temporaries(expr, &calls) || calls.throwing_destructor)
		return false;
	begin_segment();
	nothrow_segment_ = true;
	segment_floor_ = calls.member_template ? 0 : cleanups_.size();
	begin_tracking(&expr, 1, true, discarded);
	return true;
}

bool FunctionLowering::invokes_caller_destroyed(const syntax::Expr& e, const sema::ExprFacts* facts)
{
	// The function a call-like node invokes, from its facts (a constructor
	// call's constructor, an operator call's function, a call's callee or
	// the callee expression's function type), and its converting
	// constructor.
	const sema::FunctionEntity* invoked = facts->entity != nullptr && facts->entity->kind == sema::EntityKind::Function
		? static_cast<const sema::FunctionEntity*>(facts->entity)
		: nullptr;
	if (facts->conversion != nullptr && caller_destroys(facts->conversion->constructor))
		return true;
	switch (e.kind)
	{
	case syntax::ExprKind::Call:
	{
		if (invoked != nullptr)
			return caller_destroys(invoked);
		const syntax::Expr* callee = WithoutParentheses(static_cast<const syntax::CallExpr&>(e).callee);
		const sema::Type* type = callee->facts != nullptr ? callee->facts->type : nullptr;
		if (type != nullptr &&
			(type->kind == sema::TypeKind::Pointer || type->kind == sema::TypeKind::MemberPointer ||
				type->is_reference()))
			type = type->element;
		return type != nullptr && caller_destroys(type->canonical);
	}
	case syntax::ExprKind::FunctionalCast:
	case syntax::ExprKind::BracedInit:
		return caller_destroys(invoked);
	case syntax::ExprKind::New:
		return caller_destroys(static_cast<const syntax::NewExpr&>(e).constructor);
	case syntax::ExprKind::Binary:
	case syntax::ExprKind::Assign:
	case syntax::ExprKind::Unary:
	case syntax::ExprKind::Postfix:
	case syntax::ExprKind::Subscript:
		return (facts->flags & sema::kOperatorCall) != 0 && caller_destroys(invoked);
	default:
		return false;
	}
}

bool FunctionLowering::caller_destroyed(const sema::Type* parameter)
{
	if (parameter == nullptr || parameter->is_reference())
		return false;
	const sema::ClassEntity* cls = sema::ClassOf(parameter);
	return cls != nullptr && !lowering_.passed_directly(cls) && needs_destruction(parameter);
}

bool FunctionLowering::caller_destroys(const sema::Type* function)
{
	// Memoized per function type (the scans of every full-expression ask
	// for each call they meet).
	if (function == nullptr || function->kind != sema::TypeKind::Function || function->param_count == 0)
		return false;
	PointerMap& memo = lowering_.caller_destroyed_types();
	const uint32_t known = memo.find(function);
	if (known != PointerMap::kMissing)
		return known != 0;
	bool destroys = false;
	for (uint32_t i = 0; i < function->param_count && !destroys; ++i)
		destroys = caller_destroyed(function->params[i]);
	memo.set(function, destroys ? 1 : 0);
	return destroys;
}

bool FunctionLowering::caller_destroys(const sema::FunctionEntity* function)
{
	return function != nullptr && caller_destroys(function->type->canonical);
}

bool FunctionLowering::begin_initializer(const syntax::Initializer* init, const sema::FunctionEntity* constructor)
{
	// An initializer's full-expression: its expressions together.
	if (init == nullptr || full_expression_)
		return false;
	// The constructor's parameters the caller destroys are the
	// full-expression's temporaries.
	const bool caller = caller_destroys(constructor);
	if (init->kind != syntax::InitializerKind::Parenthesized)
	{
		forced_temporaries_ = caller;
		const bool region = begin_full_expression(init->expression);
		forced_temporaries_ = false;
		return region;
	}
	bool made = caller, throws = false;
	for (const syntax::Expr* argument : init->arguments)
	{
		made = made || creates_temporaries(argument);
		throws = throws || may_throw(argument);
	}
	if (caller)
	{
		segment_temporaries_ = true;
		return begin_segment();
	}
	if (cleanups_.empty() && !made)
		return false;
	segment_temporaries_ = made;
	if (throws)
	{
		begin_segment();
		begin_tracking(init->arguments.data, init->arguments.size, false, false);
		return true;
	}
	CallTemporaries calls = CallTemporaries{ false, false, false };
	bool found = false;
	for (const syntax::Expr* argument : init->arguments)
		found = creates_temporaries(argument, &calls) || found;
	if (!found || calls.throwing_destructor)
		return false;
	begin_segment();
	nothrow_segment_ = true;
	segment_floor_ = calls.member_template ? 0 : cleanups_.size();
	return true;
}

bool FunctionLowering::begin_segment()
{
	if (terminated_)
		place(new_block(lowir::BlockKind::Unreachable));
	full_expression_ = true;
	region_segment_ = true;
	eager_pending_ = true;
	segment_block_ = builder_.insertion_block();
	segment_after_ = builder_.last_instruction(segment_block_);
	return true;
}

void FunctionLowering::end_full_expression(bool began)
{
	if (!began)
		return;
	end_tracking();
	close_region();
	// The objects it leaves alive (temporaries a reference extends, 12.2p5,
	// or not yet destroyed) unwind through every object under them later.
	drop_segment_floor();
	full_expression_ = false;
	region_segment_ = false;
	eager_pending_ = false;
	nothrow_segment_ = false;
	segment_temporaries_ = false;
	deferred_region_ = false;
	hidden_eh_ = false;
	split_block_ = lowir::kNoId;
}

void FunctionLowering::drop_segment_floor()
{
	// The course's non-throwing segment's handlers stop at its floor: the
	// unwinding chains they made for the objects above it end there, so
	// they are forgotten when handlers must destroy every alive object.
	if (segment_floor_ == 0)
		return;
	for (size_t i = segment_floor_; i < cleanups_.size(); ++i)
		cleanups_[i].unwind = lowir::kNoId;
	segment_floor_ = 0;
}

void FunctionLowering::begin_construction(const syntax::Initializer* init)
{
	// The construction of a declared object (or array of them): a region
	// opens before its first call that may throw and closes at its end.
	// Its start is the earliest a later one opens (every path to the end
	// passes it), moved past a region closing on the way (close_region).
	// One whose initializer opens or closes regions on some of its paths
	// only (branched_regions) closes its region at each terminator instead,
	// as a full-expression's segment: a start moved into one path would not
	// be on the others.
	bool branched = false;
	if (init != nullptr && init->kind == syntax::InitializerKind::Parenthesized)
	{
		for (const syntax::Expr* argument : init->arguments)
			branched = branched || branched_regions(argument);
	}
	else if (init != nullptr)
		branched = branched_regions(init->expression);
	region_segment_ = branched;
	construction_ = true;
	if (terminated_)
		place(new_block(lowir::BlockKind::Unreachable));
	construction_block_ = builder_.insertion_block();
	construction_after_ = builder_.last_instruction(construction_block_);
	construction_depth_ = cleanups_.size();
}

void FunctionLowering::end_construction()
{
	construction_ = false;
	close_region();
	region_segment_ = false;
}

bool FunctionLowering::branched_regions(const syntax::Expr* root)
{
	// Whether evaluating the expression may open or close regions on some
	// of its paths only: a statement expression or a new-expression in an
	// operand evaluated conditionally (a conditional's second or third, a
	// logical operator's right), a delete-expression whose destructor may
	// throw or a new-expression whose allocation function returns null
	// (each tests the pointer) anywhere. A work list; a marker after an
	// operand evaluated conditionally.
	static const syntax::Expr conditional = syntax::Expr();
	std::vector<const syntax::Expr*>& work = throw_scan_;
	const size_t base = work.size();
	work.push_back(root);
	bool found = false;
	bool marked = false;
	while (!found && work.size() > base)
	{
		const syntax::Expr* expr = work.back();
		work.pop_back();
		if (expr == &conditional)
		{
			marked = true;
			continue;
		}
		const bool branched = marked;
		marked = false;
		if (expr == nullptr)
			continue;
		const syntax::Expr* e = WithoutParentheses(expr);
		const sema::ExprFacts* facts = e->facts;
		if (facts != nullptr && (facts->flags & sema::kFolded) != 0)
			continue;
		const auto push = [&work](const syntax::Expr* operand, bool evaluated_conditionally) {
			work.push_back(operand);
			if (evaluated_conditionally)
				work.push_back(&conditional);
		};
		switch (e->kind)
		{
		case syntax::ExprKind::Statement:
			found = branched;
			break;
		case syntax::ExprKind::Throw:
			push(static_cast<const syntax::ThrowExpr*>(e)->operand, branched);
			break;
		case syntax::ExprKind::Binary:
		case syntax::ExprKind::Assign:
		{
			const syntax::BinaryExpr* binary = static_cast<const syntax::BinaryExpr*>(e);
			const bool logical = (binary->op == TokenKind::OP_LAND || binary->op == TokenKind::OP_LOR) &&
				(facts == nullptr || (facts->flags & sema::kOperatorCall) == 0);
			push(binary->right, branched || logical);
			push(binary->left, branched);
			break;
		}
		case syntax::ExprKind::Conditional:
		{
			// Its branching closes a construction's region (the course
			// closes regions at each terminator of a conditional there).
			found = true;
			const syntax::ConditionalExpr* choice = static_cast<const syntax::ConditionalExpr*>(e);
			push(choice->else_expr, true);
			push(choice->then_expr, true);
			push(choice->condition, branched);
			break;
		}
		case syntax::ExprKind::Unary:
		case syntax::ExprKind::Postfix:
			push(static_cast<const syntax::UnaryExpr*>(e)->operand, branched);
			break;
		case syntax::ExprKind::Call:
		{
			const syntax::CallExpr* call = static_cast<const syntax::CallExpr*>(e);
			const syntax::Expr* callee = WithoutParentheses(call->callee);
			push(callee->kind == syntax::ExprKind::Member ? static_cast<const syntax::MemberExpr*>(callee)->base
														  : callee,
				branched);
			for (const syntax::Expr* argument : call->arguments)
				push(argument, branched);
			const sema::FunctionEntity* function = facts != nullptr && (facts->flags & sema::kDirectCall) != 0 &&
					facts->entity != nullptr && facts->entity->kind == sema::EntityKind::Function
				? static_cast<const sema::FunctionEntity*>(facts->entity)
				: nullptr;
			if (function != nullptr && function->default_arguments != nullptr)
			{
				for (uint32_t i = call->arguments.size; i < function->type->canonical->param_count; ++i)
					push(lowering_.default_argument(function, i), branched);
			}
			break;
		}
		case syntax::ExprKind::FunctionalCast:
		{
			const syntax::FunctionalCastExpr* cast = static_cast<const syntax::FunctionalCastExpr*>(e);
			for (const syntax::Expr* argument : cast->arguments)
				push(argument, branched);
			push(cast->braced, branched);
			break;
		}
		case syntax::ExprKind::Subscript:
			push(static_cast<const syntax::SubscriptExpr*>(e)->index, branched);
			push(static_cast<const syntax::SubscriptExpr*>(e)->base, branched);
			break;
		case syntax::ExprKind::Member:
			push(static_cast<const syntax::MemberExpr*>(e)->base, branched);
			break;
		case syntax::ExprKind::Cast:
			push(static_cast<const syntax::CastExpr*>(e)->operand, branched);
			break;
		case syntax::ExprKind::BracedInit:
			for (const syntax::Expr* element : static_cast<const syntax::BracedInitList*>(e)->elements)
				push(element, branched);
			break;
		case syntax::ExprKind::Delete:
		{
			const sema::ClassEntity* entity = facts != nullptr ? sema::ClassOf(facts->target) : nullptr;
			found = entity != nullptr && entity->destructor != nullptr && !entity->destructor->trivial &&
				call_may_throw(entity->destructor);
			push(static_cast<const syntax::DeleteExpr*>(e)->operand, branched);
			break;
		}
		case syntax::ExprKind::New:
		{
			const syntax::NewExpr* allocation = static_cast<const syntax::NewExpr*>(e);
			found = branched || (facts != nullptr && tests_allocation(*allocation));
			for (const syntax::Expr* argument : allocation->placement)
				push(argument, branched);
			push(allocation->count, branched);
			const syntax::Initializer* init = allocation->initializer;
			if (init != nullptr && init->kind == syntax::InitializerKind::Parenthesized)
			{
				for (const syntax::Expr* argument : init->arguments)
					push(argument, branched);
			}
			else if (init != nullptr)
				push(init->expression, branched);
			break;
		}
		default:
			break;
		}
	}
	work.resize(base);
	return found;
}

void FunctionLowering::paused_call_region(lowir::Opcode op)
{
	// A throwing call whose operands branch: the course opens its region
	// where it began and pauses it at the branch (the call, after the
	// join, opens none).
	if ((op != lowir::Opcode::Branch && op != lowir::Opcode::Jump) || !region_segment_ || region_open_ ||
		!full_expression_ || explicit_regions_ != 0 || static_initializer_ || tracked_region() || terminated_ ||
		chain_empty())
		return;
	const lowir::BlockId block = builder_.insertion_block();
	const lowir::FunctionBody& body = builder_.body();
	for (CallMark& mark : call_marks_)
	{
		if (mark.block != block || mark.declared || mark.regioned || mark.depth != cleanups_.size())
			continue;
		// (Not around a region opened or ended since it began.)
		bool marked = false;
		for (lowir::InstId i = mark.after == lowir::kNoId ? body.blocks[block].first
														  : body.instructions[mark.after].next;
			i != lowir::kNoId && !marked; i = body.instructions[i].next)
		{
			const lowir::Opcode op = body.instructions[i].op;
			marked = op == lowir::Opcode::EhTry || op == lowir::Opcode::EhEnd || op == lowir::Opcode::EhCleanup;
		}
		if (marked)
			return;
		const lowir::BlockId entry = eh_contexts_.empty() ? unwind_entry(cleanups_.size())
														  : region_landing(cleanups_.size());
		lowir::Instruction inst = lowir::make_instruction(
			lowir::Opcode::EhTry, lowir::Type::scalar(lowir::TypeKind::Void));
		inst.ops[0] = lowir::Value::block(entry);
		++lowering_.counters().instructions;
		builder_.insert_after(mark.block, mark.after, inst);
		region_open_ = true;
		eager_pending_ = false;
		region_depth_ = cleanups_.size();
		region_entry_ = entry;
		++regions_opened_;
		open_serial_ = regions_opened_;
		for (CallMark& paused : call_marks_)
			paused.regioned = paused.regioned || paused.block == block;
		return;
	}
}

void FunctionLowering::before_throwing_call()
{
	// A call that may throw in a segment its scan found none in: its
	// handlers destroy every alive object after all. An object registered
	// under the open region (a local aggregate's member) joins its landing
	// before the call.
	if (stale_region_ && region_open_)
	{
		close_region();
		open_region(false, true);
	}
	stale_region_ = false;
	// (Its region opened where it began, paused at its operands' branch.)
	if (!region_open_ && !call_marks_.empty() && call_marks_.back().regioned)
		return open_region(false, true);
	if (nothrow_segment_ && segment_floor_ != 0)
	{
		drop_segment_floor();
		if (region_open_ && explicit_regions_ == 0)
		{
			close_region();
			open_region(false, true);
			return;
		}
	}
	open_region(false);
}

void FunctionLowering::open_region(bool empty, bool here)
{
	// None opens inside an explicit region (an array element's, whose
	// handler resumes into the enclosing one). Inside a handler a
	// full-expression making temporaries brackets its calls all the same
	// (its landing ends the catch).
	const bool boundary = segment_temporaries_ && full_expression_ &&
		eh_contexts_.size() > (innermost_try_ != SIZE_MAX ? innermost_try_ + 1 : 0);
	if (region_open_ || (chain_empty() && !empty && !boundary && !tracked_.active) || static_initializer_ ||
		explicit_regions_ != 0)
		return;
	// (A class return's result construction: where it is called.)
	if (result_here_)
	{
		here = true;
		eager_pending_ = false;
		result_here_ = false;
	}
	const lowir::BlockId entry = tracked_region() ? tracked_target()
		: eh_contexts_.empty()                    ? unwind_entry(cleanups_.size())
												  : region_landing(cleanups_.size());
	lowir::Instruction inst = lowir::make_instruction(lowir::Opcode::EhTry, lowir::Type::scalar(lowir::TypeKind::Void));
	inst.ops[0] = lowir::Value::block(entry);
	// The course opens a full-expression's region where the earliest
	// throwing call still being evaluated in its first block began (before
	// its operands) when a call opens it, at the segment's start when a
	// temporary does, else where the innermost throwing call began in this
	// block.
	// After a temporary closed it in this block, it reopens where it is
	// needed.
	if (!eager_pending_ && split_block_ == builder_.insertion_block())
		here = true;
	const CallMark* mark = nullptr;
	// A tracked full-expression's region not opened here opens where the
	// earliest throwing call still being evaluated in this block began.
	if (tracked_region() && !here)
		mark = earliest_mark();
	else if (eager_pending_ && !empty)
	{
		// (Not a declared object's construction call while a temporary's
		// region waits: the temporary's address comes first.)
		for (const CallMark& pending : call_marks_)
		{
			if (pending.block == segment_block_)
			{
				mark = &pending;
				break;
			}
		}
	}
	lowir::InstId marker = lowir::kNoId;
	if (mark != nullptr)
	{
		++lowering_.counters().instructions;
		marker = builder_.insert_after(mark->block, mark->after, inst);
	}
	else if (eager_pending_ && !tracked_region())
	{
		++lowering_.counters().instructions;
		marker = builder_.insert_after(segment_block_, segment_after_, inst);
	}
	else if (!here && !tracked_region() && full_expression_ && !terminated_ && !call_marks_.empty() &&
		call_marks_.back().block == builder_.insertion_block() && call_marks_.back().depth == cleanups_.size())
	{
		++lowering_.counters().instructions;
		marker = builder_.insert_after(call_marks_.back().block, call_marks_.back().after, inst);
	}
	else if (!here && construction_ && !full_expression_ && !region_segment_ &&
		construction_block_ != builder_.insertion_block() && construction_depth_ == cleanups_.size())
	{
		++lowering_.counters().instructions;
		marker = builder_.insert_after(construction_block_, construction_after_, inst);
	}
	else
		marker = emit(inst);
	note_region(marker);
	region_depth_ = cleanups_.size();
	region_entry_ = entry;
	for (size_t i = aggregate_members_; aggregate_members_ != SIZE_MAX && i < cleanups_.size() && !member_landing_; ++i)
		member_landing_ = cleanups_[i].member;
	eager_pending_ = false;
	deferred_region_ = false;
	region_open_ = true;
	++regions_opened_;
	open_serial_ = regions_opened_;
}

void FunctionLowering::temporary_region(bool zeroed, bool branched)
{
	// A temporary about to be made in a full-expression that may throw:
	// the region opens at the segment's pending start, or here when nothing
	// is alive yet (else a throwing call opens it, or, after a temporary
	// closed it in this block, the temporary's construction: region_event).
	if (!full_expression_ || region_open_)
		return;
	// In a tracked full-expression, where a throwing call being evaluated
	// began (a conditional's temporary's region opens only there).
	// One a local reference extends opens it after its address.
	if (tracked_region())
	{
		if (earliest_mark() != nullptr)
			open_region(true);
		else if (scope_extension_ != nullptr && !branched)
			deferred_region_ = true;
		else if (!branched)
			open_region(true);
		return;
	}
	if (eager_pending_ && !nothrow_segment_ && !cleanups_.empty())
	{
		// With objects alive, at the earliest throwing call being evaluated,
		// else at the temporary's construction, after its address (the
		// course's form; open_deferred_region).
		if (!pending_mark_region())
			deferred_region_ = true;
		return;
	}
	// (A tracked aggregate's member's temporary: before its address.)
	if (eager_pending_)
		open_region(true);
	else if ((cleanups_.empty() || (hidden_eh_ && zeroed) || aggregate_members_ != SIZE_MAX) &&
		split_block_ != builder_.insertion_block())
		open_region(true, true);
}

bool FunctionLowering::pending_mark_region()
{
	// The region opened where the earliest throwing call still being
	// evaluated in the segment's first block began, if one is.
	for (const CallMark& pending : call_marks_)
	{
		if (pending.block == segment_block_ && !pending.declared)
		{
			const lowir::BlockId entry = tracked_region() ? tracked_target()
				: eh_contexts_.empty()                    ? unwind_entry(cleanups_.size())
														  : region_landing(cleanups_.size());
			lowir::Instruction inst = lowir::make_instruction(
				lowir::Opcode::EhTry, lowir::Type::scalar(lowir::TypeKind::Void));
			inst.ops[0] = lowir::Value::block(entry);
			++lowering_.counters().instructions;
			note_region(builder_.insert_after(pending.block, pending.after, inst));
			region_depth_ = cleanups_.size();
			region_entry_ = entry;
			eager_pending_ = false;
			deferred_region_ = false;
			region_open_ = true;
			++regions_opened_;
			open_serial_ = regions_opened_;
			return true;
		}
	}
	return false;
}

void FunctionLowering::open_deferred_region()
{
	// A temporary's construction begins: the region temporary_region
	// deferred opens here.
	if (!deferred_region_)
		return;
	deferred_region_ = false;
	if (!full_expression_ || region_open_)
		return;
	eager_pending_ = false;
	open_region(true, true);
}

void FunctionLowering::region_event()
{
	// A call that cannot throw, or a temporary's construction (after its
	// address): in the course's non-throwing segment the first opens the
	// region at the segment's start; there, and after a temporary closed
	// the region in this block, one opens it here once something is to be
	// destroyed.
	if (tracked_.active && deferred_region_)
		return open_deferred_region();
	if (!full_expression_ || region_open_ || guarded_branches_ != 0 || reopened_choices_ != 0 ||
		explicit_regions_ != 0 || tracked_.active)
		return;
	if (deferred_region_)
		return open_deferred_region();
	// With objects alive, a segment that makes temporaries opens its region
	// at its first call (the course's form).
	if (!nothrow_segment_ && eager_pending_ && segment_temporaries_ && !cleanups_.empty())
	{
		if (!pending_mark_region())
		{
			eager_pending_ = false;
			open_region(true, true);
		}
		return;
	}
	if (nothrow_segment_ && eager_pending_)
		open_region(true);
	else if (nothrow_segment_ || split_block_ == builder_.insertion_block())
		open_region(false, true);
}

void FunctionLowering::temporary_done(bool list)
{
	// A temporary whose destruction does nothing, constructed: the course
	// ends the region there while it destroys no temporary yet, and
	// reopens it at once when something is to be destroyed (in a tracked
	// full-expression, a forced one's segment starts again at once).
	if (tracked_.active && list && full_expression_ && explicit_regions_ == 0 && tracked_region())
	{
		close_region();
		open_region(true, true);
		return;
	}
	if (!full_expression_ || !region_open_ || guarded_branches_ != 0 || reopened_choices_ != 0 ||
		explicit_regions_ != 0 || tracked_.active)
		return;
	// (A std::initializer_list object none a reference extends, or one a
	// local reference extends, ends it there all the same: `list`.)
	for (size_t i = segment_floor_; i < cleanups_.size() && !list; ++i)
	{
		if (cleanups_[i].temporary)
			return;
	}
	close_region();
	open_region(false, true);
	if (!region_open_)
		split_block_ = builder_.insertion_block();
}

void FunctionLowering::begin_call(bool throws)
{
	// A throwing call of a full-expression's segment: its region would
	// open where its lowering begins. A declared object's initialization
	// whose first call is not its constructor's has no construction call.
	// Under hidden regions, one whose arguments make temporaries opens the
	// segment's region there at once (the course's form: it covers the
	// call's operands up to the first terminator, before anything is
	// alive).
	declaring_ = false;
	const bool eager = eager_call_;
	eager_call_ = false;
	if (!throws || !full_expression_)
		return;
	if (terminated_)
		place(new_block(lowir::BlockKind::Unreachable));
	const lowir::BlockId block = builder_.insertion_block();
	call_marks_.push_back(
		CallMark{ block, builder_.last_instruction(block), declared_construction_, cleanups_.size(), false });
	if (eager && hidden_eh_ && eager_pending_ && !region_open_ && !declared_construction_)
		open_region(true, true);
}

void FunctionLowering::note_call_arguments(
	const sema::FunctionEntity* function, const syntax::Slice<syntax::Expr*>& written)
{
	// Before a hidden segment's call: whether its written or default
	// arguments (8.3.6p9) make temporaries. Written arguments inside ones
	// found to make none (a nested call's, keyed by its first) are not
	// walked again.
	if (!hidden_eh_ || !eager_pending_ || region_open_)
		return;
	if (written.size == 0 || clean_arguments_.find(written[0]) == PointerMap::kMissing)
	{
		record_clean_ = true;
		clean_scratch_.clear();
		bool made = false;
		for (const syntax::Expr* argument : written)
		{
			if (creates_temporaries(argument))
			{
				made = true;
				break;
			}
		}
		record_clean_ = false;
		if (made)
		{
			eager_call_ = true;
			return;
		}
		for (const syntax::Expr* first : clean_scratch_)
			clean_arguments_.set(first, 1);
	}
	const sema::Type* type = function != nullptr ? function->type->canonical : nullptr;
	for (uint32_t i = written.size; type != nullptr && function->default_arguments != nullptr && i < type->param_count;
		++i)
	{
		if (function->default_arguments[i] != nullptr && creates_temporaries(lowering_.default_argument(function, i)))
		{
			eager_call_ = true;
			return;
		}
	}
}

void FunctionLowering::end_call(bool throws)
{
	if (throws && full_expression_ && !call_marks_.empty())
		call_marks_.pop_back();
}

bool FunctionLowering::destroyed_temporary(const sema::Type* type)
{
	// A class temporary whose destructor is not trivial (called even when
	// it has no effect, unlike an automatic object's).
	return needs_destruction(type);
}

bool FunctionLowering::creates_temporaries(
	const syntax::Expr* root, CallTemporaries* calls, bool reachable, const sema::Type* object)
{
	// Whether the expression evaluates a class prvalue (or a converting
	// constructor's temporary) of a class whose destructor is not trivial:
	// the full-expression then destroys temporaries. Unevaluated operands
	// excluded; a work list. None in a unit without such classes. With
	// `calls`, only a call's class prvalue counts, and every one is seen.
	if (lowering_.graph().nontrivial_destructors == 0)
		return false;
	const syntax::Expr* top = WithoutParentheses(root);
	// An argument initializing a class parameter (the object itself is
	// counted as the call's, invokes_caller_destroyed) follows a marker:
	// its operands are scanned.
	static const syntax::Expr parameter_object = syntax::Expr();
	std::vector<const syntax::Expr*>& work = throw_scan_;
	const size_t base = work.size();
	work.push_back(root);
	bool found = false;
	bool parameter = false;
	while ((!found || calls != nullptr) && work.size() > base)
	{
		const syntax::Expr* expr = work.back();
		work.pop_back();
		if (expr == &parameter_object)
		{
			parameter = true;
			continue;
		}
		const bool counted = !parameter;
		parameter = false;
		if (expr == nullptr)
			continue;
		const syntax::Expr* e = WithoutParentheses(expr);
		const sema::ExprFacts* facts = e->facts;
		if (facts == nullptr || (facts->flags & sema::kFolded) != 0)
			continue;
		// A call, operator call, constructor call or converting
		// constructor with a parameter the caller destroys makes that
		// temporary.
		if (invokes_caller_destroyed(*e, facts))
		{
			found = true;
			if (calls == nullptr)
				break;
		}
		// A class operand converted by a conversion function is evaluated
		// as its own facts say (a prvalue's temporary is its object).
		if ((facts->flags & sema::kUserConverted) != 0)
			facts = facts->original;
		const bool own = object != nullptr && expr == root;
		// So is an array prvalue's temporary array (`A{...}`, or a list a
		// reference to an array binds, array_temporary).
		const bool destroyed = counted && facts->type != nullptr && facts->category == sema::ValueCategory::PRValue &&
			!facts->type->is_reference() &&
			(facts->type->unqualified->kind == sema::TypeKind::Class ||
				(facts->type->kind == sema::TypeKind::Array &&
					(e->kind == syntax::ExprKind::FunctionalCast || e->kind == syntax::ExprKind::BracedInit))) &&
			needs_destruction(facts->type) && !(own && sema::ClassOf(facts->type) == sema::ClassOf(object));
		// A std::initializer_list's array elements are temporaries too.
		if (calls == nullptr && (facts->flags & sema::kInitializerList) != 0 && needs_destruction(facts->target) &&
			facts->target->bound != 0)
		{
			found = true;
			break;
		}
		if (destroyed && calls == nullptr)
		{
			found = true;
			break;
		}
		if (destroyed && CallResult(*e, facts) && (e != top || calls->root))
		{
			found = true;
			calls->member_template = calls->member_template || MemberTemplateSpecialization(sema::ClassOf(facts->type));
			calls->throwing_destructor = calls->throwing_destructor || destructor_may_throw(facts->type);
		}
		if (calls == nullptr && counted && !own && facts->conversion != nullptr &&
			facts->conversion->constructor != nullptr && facts->conversion->result != nullptr &&
			needs_destruction(facts->conversion->result))
		{
			found = true;
			break;
		}
		// A conversion function's derived class result, materialized for the
		// constructor that copies its base (8.5p16).
		if (calls == nullptr && counted && facts->conversion != nullptr && facts->conversion->after != nullptr &&
			facts->conversion->after->constructor != nullptr &&
			needs_destruction(facts->conversion->constructor->type->canonical->element))
		{
			found = true;
			break;
		}
		switch (e->kind)
		{
		case syntax::ExprKind::Binary:
		case syntax::ExprKind::Assign:
		{
			// An operator function's class parameters by value (a member's
			// object is its left operand) are its to destroy.
			const syntax::BinaryExpr* binary = static_cast<const syntax::BinaryExpr*>(e);
			const sema::FunctionEntity* function = (facts->flags & sema::kOperatorCall) != 0
				? static_cast<const sema::FunctionEntity*>(facts->entity)
				: nullptr;
			const bool member = function != nullptr && function->member && !function->is_static;
			const syntax::Expr* operands[2] = { binary->right, binary->left };
			for (uint32_t k = 0; k < 2; ++k)
			{
				work.push_back(operands[k]);
				const int index = (k == 0 ? 1 : 0) - (member ? 1 : 0);
				const sema::Type* type = function != nullptr ? function->type->canonical : nullptr;
				if (type != nullptr && index >= 0 && static_cast<uint32_t>(index) < type->param_count &&
					!type->params[index]->is_reference() && sema::ClassOf(type->params[index]) != nullptr)
					work.push_back(&parameter_object);
			}
			break;
		}
		case syntax::ExprKind::Conditional:
		{
			// At the root, a scalar conditional's operand never evaluated
			// leaves no temporaries (see choice_operand).
			const syntax::ConditionalExpr* choice = static_cast<const syntax::ConditionalExpr*>(e);
			const bool pruned = reachable && e == top && facts != nullptr &&
				facts->category == sema::ValueCategory::PRValue && sema::ClassOf(facts->type) == nullptr;
			if (!pruned || !unreachable_operand(*choice, *choice->else_expr))
				work.push_back(choice->else_expr);
			if (!pruned || !unreachable_operand(*choice, *choice->then_expr))
				work.push_back(choice->then_expr);
			work.push_back(choice->condition);
			break;
		}
		case syntax::ExprKind::Unary:
		case syntax::ExprKind::Postfix:
			work.push_back(static_cast<const syntax::UnaryExpr*>(e)->operand);
			break;
		case syntax::ExprKind::Call:
		{
			const syntax::CallExpr* call = static_cast<const syntax::CallExpr*>(e);
			const syntax::Expr* called = WithoutParentheses(call->callee);
			work.push_back(called->kind == syntax::ExprKind::Member
					? static_cast<const syntax::MemberExpr*>(called)->base
					: called);
			const sema::FunctionEntity* callee = (facts->flags & sema::kDirectCall) != 0 && facts->entity != nullptr
				? static_cast<const sema::FunctionEntity*>(facts->entity)
				: nullptr;
			const sema::Type* function = callee != nullptr ? callee->type->canonical : nullptr;
			if (record_clean_ && call->arguments.size != 0)
			{
				// Its written arguments, each walked as a whole (none a class
				// parameter's object), are clean if the walk ends clean.
				bool whole = true;
				for (uint32_t i = 0; i < call->arguments.size && function != nullptr && i < function->param_count; ++i)
					whole = whole &&
						(function->params[i]->is_reference() || sema::ClassOf(function->params[i]) == nullptr);
				if (whole)
					clean_scratch_.push_back(call->arguments[0]);
			}
			// 8.3.6p9: default arguments are evaluated at each call, their
			// temporaries the call's full-expression's (12.2p3).
			const uint32_t count = function != nullptr && callee->default_arguments != nullptr &&
					function->param_count > call->arguments.size
				? function->param_count
				: call->arguments.size;
			for (uint32_t i = 0; i < count; ++i)
			{
				work.push_back(i < call->arguments.size ? call->arguments[i] : lowering_.default_argument(callee, i));
				if (function != nullptr && i < function->param_count && !function->params[i]->is_reference() &&
					sema::ClassOf(function->params[i]) != nullptr)
					work.push_back(&parameter_object);
			}
			break;
		}
		case syntax::ExprKind::FunctionalCast:
		{
			const syntax::FunctionalCastExpr* cast = static_cast<const syntax::FunctionalCastExpr*>(e);
			if (record_clean_ && cast->arguments.size != 0)
				clean_scratch_.push_back(cast->arguments[0]);
			for (const syntax::Expr* argument : cast->arguments)
				work.push_back(argument);
			work.push_back(cast->braced);
			break;
		}
		case syntax::ExprKind::Subscript:
			work.push_back(static_cast<const syntax::SubscriptExpr*>(e)->index);
			work.push_back(static_cast<const syntax::SubscriptExpr*>(e)->base);
			break;
		case syntax::ExprKind::Member:
			work.push_back(static_cast<const syntax::MemberExpr*>(e)->base);
			break;
		case syntax::ExprKind::TypeTrait:
			// A typeid's operand is evaluated only for a dynamic type (5.2.8p2).
			if (facts != nullptr && (facts->flags & sema::kDynamicType) != 0)
				work.push_back(static_cast<const syntax::TypeTraitExpr*>(e)->operand);
			break;
		case syntax::ExprKind::Cast:
			work.push_back(static_cast<const syntax::CastExpr*>(e)->operand);
			break;
		case syntax::ExprKind::BracedInit:
			for (const syntax::Expr* element : static_cast<const syntax::BracedInitList*>(e)->elements)
				work.push_back(element);
			break;
		case syntax::ExprKind::Delete:
			work.push_back(static_cast<const syntax::DeleteExpr*>(e)->operand);
			break;
		case syntax::ExprKind::New:
		{
			// The placement arguments, an array's count and the initializer.
			const syntax::NewExpr* allocation = static_cast<const syntax::NewExpr*>(e);
			for (const syntax::Expr* argument : allocation->placement)
				work.push_back(argument);
			work.push_back(allocation->count);
			const syntax::Initializer* init = allocation->initializer;
			if (init != nullptr && init->kind == syntax::InitializerKind::Parenthesized)
			{
				for (const syntax::Expr* argument : init->arguments)
					work.push_back(argument);
			}
			else if (init != nullptr)
				work.push_back(init->expression);
			break;
		}
		default:
			break;
		}
	}
	work.resize(base);
	return found;
}

bool FunctionLowering::temporary_alive(lowir::Value object, const sema::Type* type, uint32_t saved_elements)
{
	// 12.2p3: destroyed when its full-expression ends; from here on the
	// alive objects' region destroys it too (the region is split). One an
	// operand that is never evaluated makes is never destroyed (the
	// course's form): the region only splits.
	const bool registered = unreachable_operands_ == 0;
	temporaries_made_ += registered ? 1 : 0;
	if (registered)
		cleanups_.push_back(Cleanup{ object, type, nullptr, lowir::kNoId, lowir::kNoId, lowir::kNoId, true, false,
			lowir::kNoId, false, saved_elements });
	if ((guarded_branches_ != 0 || tracked_.active) && registered)
	{
		// Made in a branch its full-expression outlives (or in a tracked
		// full-expression): its flag, cleared before the branching (at the
		// full-expression's start), is set now.
		const lowir::Type u8 = lowir::Type::scalar(lowir::TypeKind::U8);
		const lowir::SlotId flag = generated_slot(lowir::SlotKind::Lifetime, u8);
		lowir::Instruction clear = lowir::make_instruction(lowir::Opcode::Store, u8);
		clear.ops[0] = integer(u8, 0).value;
		clear.ops[1] = lowir::Value::slot(flag);
		if (tracked_.active)
			track_reset(clear);
		else
		{
			++lowering_.counters().instructions;
			guard_after_ = builder_.insert_after(guard_block_, guard_after_, clear);
		}
		Place set = Place();
		set.storage = lowir::Value::slot(flag);
		store(u8, integer(u8, 1), set);
		cleanups_.back().guarded = true;
		cleanups_.back().flag = flag;
		cleanups_.back().branched = guarded_branches_ != 0;
		// A tracked one's slot is addressed again where it is destroyed.
		if (tracked_.active)
		{
			cleanups_.back().made_at = object;
			cleanups_.back().storage = tracked_storage(object);
		}
	}
	if (element_scopes_ != 0 && registered && guarded_branches_ == 0 && !static_initializer_)
	{
		// An array element's temporary (begin_element_temporaries): a
		// region of its own until the element is made; a sharing scope's
		// k-th one takes the previous element's k-th handler when the
		// handlers before it were taken too and it is the same object.
		const size_t k = element_handlers_.size();
		bool prefix = true;
		for (size_t j = 0; j < k && prefix; ++j)
			prefix = element_handlers_[j].shared;
		const bool shared = element_sharing_ && element_scopes_ == 1 && prefix && k < element_shared_.size() &&
			element_shared_[k].cleanup.storage == object && element_shared_[k].cleanup.type == type;
		const lowir::BlockId handler = shared ? element_shared_[k].block : new_block(lowir::BlockKind::CleanupAction);
		emit_marker(lowir::Opcode::EhTry, handler);
		element_handlers_.push_back(ElementHandler{ cleanups_.back(), handler, shared });
	}
	if (tracked_.active && registered && explicit_regions_ == 0)
	{
		// The tracked segment starts again at once.
		close_region();
		open_region(true, true);
		return registered;
	}
	if (!region_open_ || explicit_regions_ != 0)
		return registered;
	close_region();
	open_region(false, true);
	if (!region_open_)
		split_block_ = builder_.insertion_block();
	return registered;
}

void FunctionLowering::destroy_temporaries(size_t first, bool pop, const Cleanup* completed)
{
	// The full-expression's temporaries, last made first; one a reference
	// extended (12.2p5) stays with the reference's scope. A destructor
	// that may throw runs in an explicit region whose handler destroys the
	// objects under it (15.2p1-p2), all of them: the full-expression's
	// region, whose handler would destroy it again, ends first, and none
	// reopens around the destructions.
	if (tracked_.active && first <= tracked_.floor)
		finish_tracking();
	// (A tracked construction's temporaries are each destroyed in a region
	// landing on the objects under it, the course's form.)
	const bool constructed = tracked_.active && tracked_.construction && first <= tracked_.floor;
	bool throwing = false;
	for (size_t i = first; i < cleanups_.size() && !throwing; ++i)
		throwing = cleanups_[i].temporary &&
			(destructor_may_throw(cleanups_[i].type) || (completed != nullptr && completed_landing_) || constructed);
	if (throwing)
	{
		close_region();
		drop_segment_floor();
		++explicit_regions_;
	}
	const size_t pending = chain_blocks_.size();
	for (size_t i = cleanups_.size(); i-- > first;)
	{
		if (!cleanups_[i].temporary)
			continue;
		lowir::BlockId handler = lowir::kNoId;
		bool extended = false;
		for (size_t j = i + 1; j < cleanups_.size() && !extended && throwing; ++j)
			extended = !cleanups_[j].temporary;
		if (throwing && (completed != nullptr || extended) &&
			(destructor_may_throw(cleanups_[i].type) || (completed != nullptr && completed_landing_)))
		{
			// The chain of the objects under it with those completed after it
			// on top: the temporaries a reference extends (12.2p5), then the
			// completed object (their blocks made for this position).
			std::vector<Cleanup> above(cleanups_.begin() + static_cast<std::ptrdiff_t>(i), cleanups_.end());
			cleanups_.resize(i);
			for (size_t j = 1; j < above.size(); ++j)
			{
				if (above[j].temporary)
					continue;
				Cleanup kept = above[j];
				kept.unwind = kept.shared_return = kept.conditional = lowir::kNoId;
				kept.eh_marked = kept.eh_unmarked = 0;
				cleanups_.push_back(kept);
			}
			if (completed != nullptr)
				cleanups_.push_back(Cleanup{ completed->storage, completed->type, completed->variable, lowir::kNoId,
					lowir::kNoId, lowir::kNoId, false });
			handler = eh_contexts_.empty() ? unwind_entry(cleanups_.size()) : region_landing(cleanups_.size());
			cleanups_.resize(i);
			cleanups_.insert(cleanups_.end(), above.begin(), above.end());
		}
		else if (constructed)
			handler = eh_contexts_.empty() ? unwind_entry(i) : region_landing(i);
		else if (throwing)
			handler = destruction_handler(i);
		destroy_local(cleanups_[i], handler, false);
	}
	if (throwing)
		--explicit_regions_;
	if (chain_blocks_.size() != pending)
	{
		const lowir::BlockId next = new_block(lowir::BlockKind::CallUnwindEnd);
		jump(next);
		place_chain_blocks();
		place(next);
	}
	if (!pop)
		return;
	// An extended temporary kept above a destroyed one gets unwinding and
	// return chains of its own, inside exception-handling contexts too: its
	// blocks continue to the objects that were under it.
	size_t kept = first;
	for (size_t i = first; i < cleanups_.size(); ++i)
	{
		if (cleanups_[i].temporary)
			continue;
		if (kept != i)
		{
			cleanups_[i].unwind = lowir::kNoId;
			cleanups_[i].shared_return = lowir::kNoId;
			cleanups_[i].conditional = lowir::kNoId;
			cleanups_[i].eh_marked = cleanups_[i].eh_unmarked = 0;
		}
		cleanups_[kept++] = cleanups_[i];
	}
	cleanups_.resize(kept);
}

void FunctionLowering::close_region()
{
	// The region ends; handler blocks it made are placed after a jump to
	// the code that follows (or wait for a later placement).
	eager_pending_ = false;
	stale_region_ = false;
	if (!region_open_)
		return;
	region_open_ = false;
	// A region that covers no instruction ends by dropping its eh_try (the
	// course's form): the landing it would have used stays pending, and
	// the next region of the same objects, reusing it and making none,
	// does not place it either.
	if (drop_empty_region())
		dropped_chains_ = chain_blocks_.size();
	else
	{
		emit_marker(lowir::Opcode::EhEnd, lowir::kNoId);
		const bool reused = dropped_chains_ != 0 && chain_blocks_.size() == dropped_chains_;
		dropped_chains_ = 0;
		if (!chain_blocks_.empty() && !reused && !defer_landings_)
		{
			const lowir::BlockId next = new_block(region_end_kind_);
			jump(next);
			place_chain_blocks();
			place(next);
		}
	}
	// A declared object's construction opens a later region here at the
	// earliest: one at its start would enclose this one (the statement
	// expression's rule, ExpressionScope). So does a full-expression's at a
	// throwing call that began in this block (none moved to the next block
	// placed after handler blocks).
	if (construction_ && !full_expression_ && !terminated_)
	{
		construction_block_ = builder_.insertion_block();
		construction_after_ = builder_.last_instruction(construction_block_);
	}
	if (full_expression_ && !terminated_)
	{
		const lowir::BlockId block = builder_.insertion_block();
		for (CallMark& mark : call_marks_)
		{
			if (mark.block == block)
				mark.after = builder_.last_instruction(block);
		}
	}
}

bool FunctionLowering::drop_empty_region()
{
	// The open region's eh_try is the last instruction of the block it ends
	// in: it is unlinked, and a position recorded after it is moved before
	// it.
	if (terminated_ || tracked_.active)
		return false;
	lowir::FunctionBody& body = builder_.body();
	lowir::Block& block = body.blocks[builder_.insertion_block()];
	const lowir::InstId last = block.last;
	if (last == lowir::kNoId || body.instructions[last].op != lowir::Opcode::EhTry ||
		!body.instructions[last].ops[0].is(lowir::ValueKind::Block) ||
		body.instructions[last].ops[0].index != region_entry_)
		return false;
	lowir::InstId previous = lowir::kNoId;
	for (lowir::InstId i = block.first; i != last; i = body.instructions[i].next)
		previous = i;
	if (previous == lowir::kNoId)
		block.first = lowir::kNoId;
	else
		body.instructions[previous].next = lowir::kNoId;
	block.last = previous;
	--lowering_.counters().instructions;
	lowir::InstId* const positions[] = { &declared_start_after_, &guard_after_, &construction_after_, &segment_after_,
		&tracked_.after, &tracked_.first_reset };
	for (lowir::InstId* position : positions)
	{
		if (*position == last)
			*position = previous;
	}
	for (CallMark& mark : call_marks_)
	{
		if (mark.after == last)
			mark.after = previous;
	}
	return true;
}

void FunctionLowering::split_region()
{
	if (region_open_)
	{
		close_region();
		open_region(false, true);
		return;
	}
	eager_pending_ = false;
	split_block_ = builder_.insertion_block();
}

lowir::BlockKind FunctionLowering::chain_terminal_kind(ChainKind kind)
{
	return kind == ChainKind::Return     ? lowir::BlockKind::ReturnCleanupTerminal
		: kind == ChainKind::Conditional ? lowir::BlockKind::ConditionalCleanupResume
										 : lowir::BlockKind::CleanupResume;
}

lowir::BlockKind FunctionLowering::unwind_block_kind(const Cleanup& cleanup)
{
	if (cleanup.member || cleanup.counted || cleanup.deallocator != nullptr)
		return lowir::BlockKind::ConstructionCleanup;
	return cleanup.guarded ? lowir::BlockKind::ConditionalCleanupDispatch : lowir::BlockKind::CleanupAction;
}

lowir::BlockId FunctionLowering::chain_entry(ChainKind kind, lowir::BlockId& terminal, size_t count)
{
	// The terminal first, then one block per alive object (of the first
	// `count`) from the outermost in; the innermost object with a block
	// ends the search, so each block is made once. Made blocks wait in
	// chain_blocks_ (a large set's, in late_blocks_ for the function's
	// end).
	std::vector<ChainBlock>& pending = kind == ChainKind::Conditional ? late_blocks_ : chain_blocks_;
	const bool unwinding = kind != ChainKind::Return;
	if (terminal == lowir::kNoId)
	{
		terminal = new_block(chain_terminal_kind(kind));
		pending.push_back(ChainBlock{ terminal, true, unwinding, false,
			Cleanup{ lowir::Value(), nullptr, nullptr, lowir::kNoId, lowir::kNoId, lowir::kNoId, false }, lowir::kNoId,
			lowir::kNoId });
	}
	const auto node = [kind](Cleanup& cleanup) -> lowir::BlockId& {
		return kind == ChainKind::Unwind     ? cleanup.unwind
			: kind == ChainKind::Conditional ? cleanup.conditional
											 : cleanup.shared_return;
	};
	// The course's non-throwing segment's handlers stop at its floor.
	const size_t floor = kind != ChainKind::Unwind ? 0 : segment_floor_ < count ? segment_floor_ : count;
	size_t first = count;
	while (first > floor && node(cleanups_[first - 1]) == lowir::kNoId)
		--first;
	lowir::BlockId tail = first == floor ? terminal : node(cleanups_[first - 1]);
	const size_t made = pending.size();
	for (size_t i = first; i < count; ++i)
	{
		// A return leaves the local in the result's destination alive.
		if (!unwinding && cleanups_[i].result)
		{
			node(cleanups_[i]) = tail;
			continue;
		}
		const lowir::BlockId block = new_block(kind == ChainKind::Return ? lowir::BlockKind::ReturnCleanupAction
				: kind == ChainKind::Conditional                         ? lowir::BlockKind::ConditionalCleanupDispatch
																		 : unwind_block_kind(cleanups_[i]));
		const lowir::BlockId below = unwinding ? lowir::kNoId : destruction_handler(i);
		pending.push_back(ChainBlock{ block, false, unwinding, false, cleanups_[i], tail, below });
		node(cleanups_[i]) = block;
		tail = block;
	}
	// (A tracked construction's cleanup blocks follow from the innermost
	// out, the course's order.)
	if (tracked_.active && tracked_.construction && kind == ChainKind::Unwind)
		std::reverse(pending.begin() + static_cast<std::ptrdiff_t>(made), pending.end());
	return tail;
}

lowir::BlockId FunctionLowering::unwind_entry(size_t count)
{
	// More than eight alive objects unwind through a chain of their own,
	// placed at the function's end. A detached body's chains end at its
	// subobjects' cleanup entry.
	if (body_unwind_target() != lowir::kNoId)
	{
		lowir::BlockId terminal = body_terminal(1);
		const lowir::BlockId entry = chain_entry(
			count > 8 ? ChainKind::Conditional : ChainKind::Unwind, terminal, count);
		// This suffix retires both the call's record and the detached
		// body's record. Retain the former explicitly at its landing.
		if (landing_dispatch_.size() <= entry)
			landing_dispatch_.resize(entry + 1, 0);
		if (landing_dispatch_[entry] == 0)
		{
			ChainBlock dispatch = ChainBlock();
			dispatch.block = new_block(lowir::BlockKind::CallUnwindDispatch);
			dispatch.unwinding = true;
			dispatch.action = ChainAction::Dispatch;
			dispatch.tail = entry;
			chain_blocks_.push_back(dispatch);
			landing_dispatch_[entry] = dispatch.block + 1;
		}
		return landing_dispatch_[entry] - 1;
	}
	if (count > 8)
		return chain_entry(ChainKind::Conditional, conditional_resume_, count);
	return chain_entry(ChainKind::Unwind, resume_block_, count);
}

lowir::BlockId FunctionLowering::return_entry()
{
	if (!eh_contexts_.empty())
		return context_return_entry();
	return chain_entry(ChainKind::Return, return_terminal_, cleanups_.size());
}

bool FunctionLowering::destructor_may_throw(const sema::Type* type) const
{
	return needs_destruction(type) && call_may_throw(object_class(type)->destructor);
}

lowir::BlockId FunctionLowering::destruction_handler(size_t depth, size_t contexts)
{
	if ((depth == 0 && !returning_result_) || !destructor_may_throw(cleanups_[depth].type))
		return lowir::kNoId;
	if (contexts >= eh_contexts_.size())
		return returning_result_   ? result_landing(depth)
			: eh_contexts_.empty() ? unwind_entry(depth)
								   : region_landing(depth);
	// Seen from the contexts still entered (as throwing_end_catch does).
	std::vector<EhContext> inner(eh_contexts_.begin() + static_cast<std::ptrdiff_t>(contexts), eh_contexts_.end());
	const size_t innermost = innermost_try_;
	eh_contexts_.resize(contexts);
	innermost_try_ = SIZE_MAX;
	for (size_t c = contexts; c-- > 0 && innermost_try_ == SIZE_MAX;)
	{
		if (eh_contexts_[c].is_try)
			innermost_try_ = c;
	}
	const lowir::BlockId entry = returning_result_ ? result_landing(depth)
		: eh_contexts_.empty()                     ? unwind_entry(depth)
												   : region_landing(depth);
	eh_contexts_.insert(eh_contexts_.end(), inner.begin(), inner.end());
	innermost_try_ = innermost;
	return entry;
}

void FunctionLowering::place_chain_blocks()
{
	// A shared return's handlers may add unwinding blocks while placing (a
	// placement inside takes a buffer of its own). The placed list's
	// buffer becomes the spare the next pending list takes: no allocation
	// per placement.
	while (!chain_blocks_.empty())
	{
		std::vector<ChainBlock> blocks;
		blocks.swap(chain_spare_);
		blocks.swap(chain_blocks_);
		place_blocks(blocks);
		blocks.clear();
		if (chain_spare_.capacity() < blocks.capacity())
			chain_spare_.swap(blocks);
	}
}

void FunctionLowering::place_late_blocks()
{
	std::vector<ChainBlock> blocks;
	blocks.swap(late_blocks_);
	place_blocks(blocks);
	if (!pending_terminates_.empty() && terminated_)
		place_terminates();
}

lowir::BlockId FunctionLowering::cleanup_terminate(bool shared)
{
	// 15.5.1p1: a destructor exiting by an exception during unwinding
	// calls std::terminate: the destruction runs in a region whose landing
	// catches everything and calls it (the course's form: one landing for
	// the scopes' objects, one for each other destruction), placed after
	// the block that made it. A host object's landing is installed under
	// one state of the enclosing regions: each destruction has its own.
	shared = shared && !lowering_.object_mode();
	if (shared && lexical_terminate_ != lowir::kNoId)
		return lexical_terminate_;
	const lowir::BlockId block = new_block(lowir::BlockKind::LexicalCleanupTerminate);
	pending_terminates_.push_back(TerminateBlock{ block, ++selector_count_ });
	if (shared)
		lexical_terminate_ = block;
	lowering_.terminate_helper();
	return block;
}

void FunctionLowering::place_terminates()
{
	std::vector<TerminateBlock> blocks;
	blocks.swap(pending_terminates_);
	const lowir::Type pointer = lowir::Type::scalar(lowir::TypeKind::Ptr);
	for (const TerminateBlock& made : blocks)
	{
		builder_.place_block(made.block);
		builder_.set_insertion_block(made.block);
		terminated_ = false;
		lowir::Instruction catch_all = lowir::make_instruction(
			lowir::Opcode::EhCatchAll, lowir::Type::scalar(lowir::TypeKind::Void));
		catch_all.immediate = made.selector;
		catch_all.flags |= lowir::IF_SELECTOR;
		emit(catch_all);
		lowir::Value exception = emit_value(lowir::make_instruction(lowir::Opcode::Exception, pointer), pointer);
		++lowering_.counters().instructions;
		builder_.call(lowir::Type::scalar(lowir::TypeKind::Void), lowir::Value::symbol(lowering_.terminate_helper()),
			&exception, 1, lowir::kNoId, 0);
		terminate(lowir::make_instruction(lowir::Opcode::Unreachable, lowir::Type::scalar(lowir::TypeKind::Void)));
	}
}

void FunctionLowering::place_blocks(const std::vector<ChainBlock>& blocks)
{
	// Their calls are no region events.
	++explicit_regions_;
	for (const ChainBlock& block : blocks)
	{
		place(block.block);
		if (block.action != ChainAction::None)
		{
			place_eh_block(block);
			continue;
		}
		if (!block.terminal)
		{
			destroy_local(block.cleanup, block.below, block.unwinding);
			jump(block.tail);
			continue;
		}
		if (block.unwinding)
		{
			emit_resume(true);
			continue;
		}
		lowir::Instruction inst = lowir::make_instruction(lowir::Opcode::Return, result_type_);
		if (!result_type_.is_void())
			inst.ops[0] = lowir::Value::slot(return_slot_);
		terminate(inst);
	}
	--explicit_regions_;
}

lowir::Value FunctionLowering::emit_call(
	lowir::Type result, lowir::Value callee, size_t base, lowir::SignatureId signature, bool throws, bool elision)
{
	// A call; in a full-expression that may throw, its result is kept in a
	// slot of its own and read back, but a non-throwing call's before the
	// region can open (no throwing call begun, no temporary to come).
	// Outside a full-expression's segment and a construction (their scans
	// found no throwing call), a region the call opens covers it alone.
	const bool transient = throws && !full_expression_ && !construction_ && !region_open_;
	const bool uncovered = !throws && !region_open_ && eager_pending_ && call_marks_.empty() && !segment_temporaries_;
	if (throws)
		before_throwing_call();
	else
		region_event();
	// (A tracked full-expression's call that cannot throw opens its region
	// all the same, its result not kept: the course's form.)
	const bool tracked_call = !throws && tracked_region() && full_expression_ && explicit_regions_ == 0 && !destroying_;
	if (tracked_call && !region_open_)
		open_region(true, true);
	++lowering_.counters().instructions;
	if (terminated_)
		place(new_block(lowir::BlockKind::Unreachable));
	const lowir::TempId temp = builder_.call(result, callee, arguments_.data() + base,
		static_cast<uint32_t>(arguments_.size() - base), signature, 0, elision ? lowir::IF_COPY_ELISION : 0);
	if (!variadic_classes_.empty() && variadic_classes_.back().first >= base)
	{
		// The classes of its class objects passed to `...` (object mode).
		const lowir::InstId call = builder_.last_instruction(builder_.insertion_block());
		for (; !variadic_classes_.empty() && variadic_classes_.back().first >= base; variadic_classes_.pop_back())
			if (variadic_classes_.back().first < arguments_.size())
				builder_.body().argument_classes.push_back(lowir::ArgumentClasses{ call,
					static_cast<uint32_t>(variadic_classes_.back().first - base), variadic_classes_.back().second });
	}
	arguments_.resize(base);
	if (transient)
		close_region();
	if (temp == lowir::kNoId)
		return lowir::Value::none();
	RValue value;
	value.value = lowir::Value::temp(temp);
	value.type = result;
	if ((!full_expression_ && !(construction_ && region_open_)) || uncovered || tracked_call || result.is_void() ||
		result.kind() == lowir::TypeKind::Obj)
		return value.value;
	Place spill = Place();
	spill.storage = lowir::Value::slot(generated_slot(lowir::SlotKind::Call, result));
	store(result, value, spill);
	return load(result, spill).value;
}

// ------------------------------------------------------------- returns

bool FunctionLowering::initializer_may_throw(const syntax::Initializer* init)
{
	if (init == nullptr)
		return false;
	if (init->kind == syntax::InitializerKind::Parenthesized)
	{
		for (const syntax::Expr* argument : init->arguments)
		{
			if (may_throw(argument))
				return true;
		}
		return false;
	}
	return may_throw(init->expression);
}

bool FunctionLowering::statement_may_throw(const syntax::Stmt& stmt)
{
	// Whether the statement evaluates a call that may throw, a constructor
	// of a declared object included.
	check_stack(stmt.location);
	switch (stmt.kind)
	{
	case syntax::StmtKind::Compound:
		for (const syntax::Stmt* item : static_cast<const syntax::CompoundStmt&>(stmt).items)
		{
			if (statement_may_throw(*item))
				return true;
		}
		return false;
	case syntax::StmtKind::Expression:
		return may_throw(static_cast<const syntax::ExpressionStmt&>(stmt).expression);
	case syntax::StmtKind::Ambiguous:
		return statement_may_throw(static_cast<const syntax::AmbiguousStmt&>(stmt).reading());
	case syntax::StmtKind::Declaration:
	{
		const syntax::Decl* decl = static_cast<const syntax::DeclarationStmt&>(stmt).declaration;
		if (decl->kind != syntax::DeclKind::Simple)
			return false;
		for (const syntax::InitDeclarator& init : static_cast<const syntax::SimpleDecl*>(decl)->declarators)
		{
			if (declarator_may_throw(init))
				return true;
		}
		return false;
	}
	case syntax::StmtKind::If:
	{
		const syntax::IfStmt& branch = static_cast<const syntax::IfStmt&>(stmt);
		return (branch.condition.declaration != nullptr
					   ? initializer_may_throw(branch.condition.declaration->initializer)
					   : may_throw(branch.condition.expression)) ||
			(syntax::LiveThen(branch) != nullptr && statement_may_throw(*branch.then_branch)) ||
			(syntax::LiveElse(branch) != nullptr && statement_may_throw(*branch.else_branch));
	}
	case syntax::StmtKind::While:
	case syntax::StmtKind::Switch:
	{
		const syntax::ConditionalLoopStmt& loop = static_cast<const syntax::ConditionalLoopStmt&>(stmt);
		return (loop.condition.declaration != nullptr ? initializer_may_throw(loop.condition.declaration->initializer)
													  : may_throw(loop.condition.expression)) ||
			statement_may_throw(*loop.body);
	}
	case syntax::StmtKind::Do:
	{
		const syntax::DoStmt& loop = static_cast<const syntax::DoStmt&>(stmt);
		return may_throw(loop.condition) || statement_may_throw(*loop.body);
	}
	case syntax::StmtKind::For:
	{
		const syntax::ForStmt& loop = static_cast<const syntax::ForStmt&>(stmt);
		if (loop.init_declaration != nullptr)
		{
			syntax::DeclarationStmt init = syntax::DeclarationStmt();
			init.kind = syntax::StmtKind::Declaration;
			init.location = stmt.location;
			init.declaration = loop.init_declaration;
			if (statement_may_throw(init))
				return true;
		}
		const bool condition = loop.condition != nullptr &&
			(loop.condition->declaration != nullptr ? initializer_may_throw(loop.condition->declaration->initializer)
													: may_throw(loop.condition->expression));
		return condition || may_throw(loop.init_expression) || may_throw(loop.iteration) ||
			statement_may_throw(*loop.body);
	}
	case syntax::StmtKind::RangeFor:
	{
		// 6.5.4p1's equivalent: its declarations, condition and increment.
		const syntax::RangeForStmt& loop = static_cast<const syntax::RangeForStmt&>(stmt);
		const sema::RangeForFacts& facts = *loop.facts;
		return (facts.range != nullptr && declarator_may_throw(*facts.range)) || declarator_may_throw(*facts.begin) ||
			(facts.end != nullptr && declarator_may_throw(*facts.end)) || may_throw(facts.condition) ||
			may_throw(facts.increment) || declarator_may_throw(facts.declaration) || statement_may_throw(*loop.body);
	}
	case syntax::StmtKind::Return:
		return may_throw(static_cast<const syntax::ValueStmt&>(stmt).value);
	case syntax::StmtKind::Throw:
		return true;
	case syntax::StmtKind::Try:
	{
		// An exception its handlers do not take, or one they throw.
		const syntax::TryStmt& attempt = static_cast<const syntax::TryStmt&>(stmt);
		if (statement_may_throw(*attempt.body))
			return true;
		for (const syntax::Handler& handler : attempt.handlers)
		{
			if (statement_may_throw(*handler.body))
				return true;
		}
		return false;
	}
	case syntax::StmtKind::Case:
		return statement_may_throw(*static_cast<const syntax::CaseStmt&>(stmt).body);
	case syntax::StmtKind::Default:
	case syntax::StmtKind::Labeled:
		return statement_may_throw(*static_cast<const syntax::LabeledStmt&>(stmt).body);
	case syntax::StmtKind::Asm:
	{
		// Its operands' expressions (the lifted operations do not throw).
		const syntax::AsmStmt& assembly = static_cast<const syntax::AsmStmt&>(stmt);
		for (const syntax::Slice<syntax::AsmOperand>* operands : { &assembly.outputs, &assembly.inputs })
		{
			for (const syntax::AsmOperand& operand : *operands)
			{
				if (may_throw(operand.expression))
					return true;
			}
		}
		return false;
	}
	default:
		return false;
	}
}

bool FunctionLowering::declarator_may_throw(const syntax::InitDeclarator& init)
{
	// A block-scope declarator's initialization, a class object's
	// constructor included.
	const sema::Binding* binding = init.declared;
	if (binding == nullptr || binding->kind != sema::BindingKind::Variable)
		return false;
	const sema::VariableEntity* variable = static_cast<const sema::VariableEntity*>(binding->entity);
	if (variable->is_extern || variable->owner->kind == sema::ScopeKind::Namespace)
		return false;
	if (!variable->type->is_reference() && object_class(variable->type) != nullptr &&
		call_may_throw(variable->constructor) && (init.initializer != nullptr || !variable->constructor->trivial))
		return true;
	return initializer_may_throw(sema::VariableInitializer(variable, init.initializer));
}

bool FunctionLowering::definition_may_throw()
{
	// The body's statements and a constructor's subobject initializations.
	// A delete-expression counts as its destructor does: a terminate guard
	// is opened only where something in the definition can unwind.
	return definition_statements_may_throw();
}

bool FunctionLowering::definition_statements_may_throw()
{
	if (function_->constructor && plan_may_throw(function_->inits, function_->init_count))
		return true;
	if (const syntax::MemInitializer* written = function_->delegation)
	{
		// 12.6.2p6: the target constructor and its arguments.
		if (call_may_throw(function_->delegate) || (written->list != nullptr && may_throw(written->list)))
			return true;
		for (const syntax::Expr* argument : written->arguments)
		{
			if (may_throw(argument))
				return true;
		}
	}
	// A destructor's subobject destructions are part of it (12.4p8; a
	// union's variant members are not destroyed).
	if (function_->destructor)
	{
		const sema::ClassEntity* entity = sema::ClassOf(function_->owner->entity->type);
		for (const sema::DataMember* member = entity->key == TokenKind::KW_UNION ? nullptr : entity->members;
			member != nullptr; member = member->next)
		{
			const sema::Type* type = member->variable->type;
			if (needs_destruction(type) && call_may_throw(object_class(type)->destructor))
				return true;
		}
		for (const sema::BaseClass* base = entity->bases; base != nullptr; base = base->next)
		{
			const sema::ClassEntity* cls = static_cast<const sema::ClassEntity*>(base->base);
			if (needs_destruction(cls->type) && call_may_throw(cls->destructor))
				return true;
		}
	}
	const syntax::FunctionBody* body = function_->body;
	return body != nullptr && !function_->defaulted && statement_may_throw(*body->body);
}

bool FunctionLowering::plan_may_throw(const sema::SubobjectInit* inits, uint32_t count)
{
	// Subobject initializations (an anonymous member object's inner ones
	// included).
	{
		for (uint32_t i = 0; i < count; ++i)
		{
			const sema::SubobjectInit& init = inits[i];
			if (init.inner != nullptr && plan_may_throw(init.inner, init.inner_count))
				return true;
			const syntax::MemInitializer* written = init.mem_initializer;
			const bool runs = written != nullptr || init.default_initializer != nullptr ||
				(init.constructor != nullptr && !init.constructor->trivial) || init.forwards;
			if (runs && call_may_throw(init.constructor))
				return true;
			if (written != nullptr)
			{
				for (const syntax::Expr* argument : written->arguments)
				{
					if (may_throw(argument))
						return true;
				}
				if (written->list != nullptr && may_throw(written->list))
					return true;
			}
			if (initializer_may_throw(init.default_initializer))
				return true;
		}
	}
	return false;
}

void FunctionLowering::emit_resume(bool outside_tries)
{
	// Unwinding continues, or under a terminate guard ends in
	// std::terminate: the exception is the guard's unless a try statement
	// encloses the landing (15.3p2: its handlers come first, 15.5.1p1). A
	// specification's guard is unwound to (its filter decides).
	// The assignment's serialized LowIR resumes into its outer guard.
	// Native objects retain direct termination, avoiding extra EH graph
	// optimization work while preserving the same exception semantics.
	if (!lowering_.object_mode() || terminate_landing_ == lowir::kNoId || unexpected_specification_ != nullptr ||
		(!outside_tries && innermost_try() != nullptr))
	{
		terminate(lowir::make_instruction(lowir::Opcode::Resume, lowir::Type::scalar(lowir::TypeKind::Void)));
		return;
	}
	emit_terminate();
}

void FunctionLowering::emit_terminate()
{
	// The exception caught, kept and handed to the terminate helper.
	const lowir::Type pointer = lowir::Type::scalar(lowir::TypeKind::Ptr);
	lowir::Instruction catch_all = lowir::make_instruction(
		lowir::Opcode::EhCatchAll, lowir::Type::scalar(lowir::TypeKind::Void));
	catch_all.immediate = 1;
	catch_all.flags |= lowir::IF_SELECTOR;
	emit(catch_all);
	if (exception_slot_ == lowir::kNoId)
		exception_slot_ = generated_slot(lowir::SlotKind::FunctionException, pointer);
	Place slot = Place();
	slot.storage = lowir::Value::slot(exception_slot_);
	RValue exception;
	exception.value = emit_value(lowir::make_instruction(lowir::Opcode::Exception, pointer), pointer);
	exception.type = pointer;
	store(pointer, exception, slot);
	lowir::Value argument = load(pointer, slot).value;
	++lowering_.counters().instructions;
	builder_.call(lowir::Type::scalar(lowir::TypeKind::Void), lowir::Value::symbol(lowering_.terminate_helper()),
		&argument, 1, lowir::kNoId, 0);
	guard_returns_ = false;
	zero_return();
	guard_returns_ = true;
}

lowir::Value FunctionLowering::zero_result()
{
	if (result_type_.is_float())
		return floating(result_type_, lowir::float_literal(result_type_.kind(), 0.0L)).value;
	return integer(result_type_, 0).value;
}

void FunctionLowering::place_unexpected_landing()
{
	// A specification's guard: a filter of its types (a reference's
	// referenced type, cv-unqualified, as a handler's), whose violation
	// (its selector) calls __cxa_call_unexpected with the exception (15.5.2:
	// std::unexpected, or a std::bad_exception the specification allows);
	// an exception it allows unwinds on. The violation's selector is the
	// filter value the personality delivers for the function's only
	// specification (Itanium's first list: -1), so the host tables pass it
	// through untranslated; __cxa_call_unexpected does not return, so its
	// call ends the block (no unreachable continuation).
	const lowir::Type pointer = lowir::Type::scalar(lowir::TypeKind::Ptr);
	const lowir::Type i32 = lowir::Type::scalar(lowir::TypeKind::I32);
	place(terminate_landing_);
	lowir::Instruction filter = lowir::make_instruction(
		lowir::Opcode::EhFilter, lowir::Type::scalar(lowir::TypeKind::Void));
	filter.immediate = -1;
	filter.flags |= lowir::IF_SELECTOR;
	scratch_.clear();
	for (uint32_t i = 0; i < unexpected_specification_->count; ++i)
	{
		const sema::Type* type = unexpected_specification_->types[i];
		scratch_.push_back(
			lowir::Value::symbol(handler_type_information((type->is_reference() ? type->element : type)->unqualified)));
	}
	filter.extra_first = builder_.add_operands(scratch_.data(), static_cast<uint32_t>(scratch_.size()));
	filter.extra_count = static_cast<uint32_t>(scratch_.size());
	emit(filter);
	// The selector temporary is the model's i64 (as end_try's), its
	// value compared as i32.
	RValue selector;
	selector.value = emit_value(
		lowir::make_instruction(lowir::Opcode::ExceptionSelector, i32), lowir::Type::scalar(lowir::TypeKind::I64));
	selector.type = i32;
	const lowir::BlockId violated = new_block(lowir::BlockKind::FunctionExceptionAction);
	const lowir::BlockId allowed = new_block(lowir::BlockKind::FunctionExceptionResume);
	branch(compare(lowir::ComparePredicate::Eq, i32, selector, integer(i32, -1)), violated, allowed);
	place(violated);
	lowir::Value exception = emit_value(lowir::make_instruction(lowir::Opcode::Exception, pointer), pointer);
	++lowering_.counters().instructions;
	builder_.call(lowir::Type::scalar(lowir::TypeKind::Void),
		lowir::Value::symbol(lowering_.runtime_function(RuntimeFunction::CallUnexpected)), &exception, 1, lowir::kNoId,
		0);
	terminated_ = true;
	place(allowed);
	terminate(lowir::make_instruction(lowir::Opcode::Resume, lowir::Type::scalar(lowir::TypeKind::Void)));
}

void FunctionLowering::place_terminate_landing()
{
	// The guard's handler: the exception kept, then the helper.
	if (terminate_landing_ == lowir::kNoId)
		return;
	flush_deferred();
	if (unexpected_specification_ != nullptr)
		return place_unexpected_landing();
	const lowir::Type pointer = lowir::Type::scalar(lowir::TypeKind::Ptr);
	const lowir::BlockId action = new_block(lowir::BlockKind::FunctionExceptionAction);
	place(terminate_landing_);
	lowir::Instruction catch_all = lowir::make_instruction(
		lowir::Opcode::EhCatchAll, lowir::Type::scalar(lowir::TypeKind::Void));
	catch_all.immediate = 1;
	catch_all.flags |= lowir::IF_SELECTOR;
	emit(catch_all);
	if (exception_slot_ == lowir::kNoId)
		exception_slot_ = generated_slot(lowir::SlotKind::FunctionException, pointer);
	Place slot = Place();
	slot.storage = lowir::Value::slot(exception_slot_);
	RValue exception;
	exception.value = emit_value(lowir::make_instruction(lowir::Opcode::Exception, pointer), pointer);
	exception.type = pointer;
	store(pointer, exception, slot);
	jump(action);
	place(action);
	lowir::Value argument = load(pointer, slot).value;
	++lowering_.counters().instructions;
	builder_.call(lowir::Type::scalar(lowir::TypeKind::Void), lowir::Value::symbol(lowering_.terminate_helper()),
		&argument, 1, lowir::kNoId, 0);
	guard_returns_ = false;
	zero_return();
	guard_returns_ = true;
	patch_fallback_slots();
}

// ------------------------------------------------ deferred cleanup blocks

bool FunctionLowering::subobject_destructor_may_throw(const Subobject& subobject) const
{
	// A new-expression's deallocation function does not throw.
	if (subobject.deallocation)
		return false;
	const sema::FunctionEntity* destructor = nullptr;
	if (subobject.base != nullptr)
		destructor = static_cast<const sema::ClassEntity*>(subobject.base->base)->destructor;
	else if (subobject.member == nullptr)
		destructor = sema::ClassOf(function_->owner->entity->type)->destructor;
	else
		return destructor_may_throw(subobject.member->variable->type);
	return destructor != nullptr && !destructor->trivial && call_may_throw(destructor);
}

// ------------------------------------------------------------- arrays

lowir::Value FunctionLowering::array_base(const ArrayBase& where, uint32_t location)
{
	if (where.member != nullptr)
		return subobject_address(nullptr, where.member, location);
	return address_of(where.place);
}

void FunctionLowering::destroy_elements(
	const sema::Type* element, const ArrayBase& where, uint64_t first, uint32_t location)
{
	// Elements `first` - 1 down to 0, each addressed again.
	const uint64_t size = lowering_.size_of(element, location);
	for (uint64_t i = first; i-- > 0;)
		destroy(element, element_address(array_base(where, location), i, size), location);
}

lowir::BlockId FunctionLowering::open_unwind_guard(bool throws, lowir::BlockId guard)
{
	// The guard is a cleanup terminate landing (the course's form, placed
	// after the block that made it).
	if (!throws)
		return lowir::kNoId;
	if (guard == lowir::kNoId)
		guard = cleanup_terminate(false);
	++explicit_regions_;
	emit_marker(lowir::Opcode::EhTry, guard);
	return guard;
}

void FunctionLowering::close_unwind_guard(lowir::BlockId guard)
{
	if (guard == lowir::kNoId)
		return;
	emit_marker(lowir::Opcode::EhEnd, lowir::kNoId);
	--explicit_regions_;
}

void FunctionLowering::place_unwind_guard(lowir::BlockId guard)
{
	// (Placed with the pending cleanup terminate landings.)
	(void)guard;
}

bool FunctionLowering::element_temporaries(const sema::FunctionEntity* constructor, bool* throws)
{
	// Whether an element's default construction makes temporaries, and may
	// throw: its constructor, or a default argument's evaluation (an
	// element's handler then destroys the elements made before it).
	// Memoized per constructor in the unit (and whether hidden regions
	// count every call: an omitted aggregate element asks per element).
	if (constructor == nullptr || constructor->trivial)
	{
		if (throws != nullptr)
			*throws = call_may_throw(constructor);
		return false;
	}
	PointerMap& memo = lowering_.element_temporaries_memo();
	const void* key = reinterpret_cast<const char*>(constructor) + (hidden_eh_ ? 1 : 0);
	uint32_t known = memo.find(key);
	if (known == PointerMap::kMissing)
	{
		bool made = caller_destroys(constructor);
		bool raises = call_may_throw(constructor);
		const sema::Type* type = constructor->type->canonical;
		for (uint32_t i = 0; i < type->param_count && constructor->default_arguments != nullptr; ++i)
		{
			const syntax::Expr* argument = constructor->default_arguments[i] != nullptr
				? lowering_.default_argument(constructor, i)
				: nullptr;
			if (argument == nullptr)
				continue;
			made = made || creates_temporaries(argument);
			raises = raises || may_throw(argument);
		}
		known = (made ? 1u : 0u) | (raises ? 2u : 0u);
		memo.set(key, known);
	}
	if (throws != nullptr)
		*throws = (known & 2) != 0;
	return (known & 1) != 0;
}

FunctionLowering::ElementKey FunctionLowering::element_key() const
{
	return ElementKey{ markers_, regions_opened_, region_open_, cleanups_.size(), eh_contexts_.size(),
		explicit_regions_ };
}

FunctionLowering::ElementScope FunctionLowering::begin_element_temporaries(bool active, bool share)
{
	ElementScope scope = ElementScope();
	scope.active = active;
	if (!active)
		return scope;
	// A sharing scope continues the previous one's handlers when nothing
	// pushed or popped an EH record since (an outermost scope only).
	scope.share = share && element_scopes_ == 0;
	if (scope.share && !(element_key() == element_key_))
	{
		element_shared_.clear();
		element_slots_.clear();
	}
	element_sharing_ = scope.share;
	element_slots_made_.clear();
	scope.first = cleanups_.size();
	scope.handlers = element_handlers_.size();
	scope.full_expression = full_expression_;
	scope.construction = construction_;
	scope.region_open = region_open_;
	scope.eager_pending = eager_pending_;
	scope.nothrow_segment = nothrow_segment_;
	scope.stale_region = stale_region_;
	scope.deferred_region = deferred_region_;
	scope.region_segment = region_segment_;
	scope.hidden_eh = hidden_eh_;
	scope.segment_temporaries = segment_temporaries_;
	scope.tracked = tracked_.active;
	tracked_.active = false;
	full_expression_ = construction_ = region_open_ = eager_pending_ = nothrow_segment_ = false;
	stale_region_ = deferred_region_ = region_segment_ = hidden_eh_ = segment_temporaries_ = false;
	++explicit_regions_;
	++element_scopes_;
	return scope;
}

lowir::SlotId FunctionLowering::argument_slot(lowir::Type type, lowir::SlotKind kind)
{
	// In a sharing element scope the i-th such slot is the previous
	// element's i-th when of the same type.
	if (!element_sharing_ || element_scopes_ != 1)
		return generated_slot(kind, type);
	const size_t i = element_slots_made_.size();
	const lowir::SlotId slot = i < element_slots_.size() && element_slots_[i].type == type ? element_slots_[i].slot
																						   : generated_slot(kind, type);
	element_slots_made_.push_back(ElementSlot{ slot, type });
	return slot;
}

void FunctionLowering::end_element_temporaries(const ElementScope& scope)
{
	// The element is made: its temporaries' regions end, innermost first,
	// and the temporaries are destroyed (its full-expression's end); each
	// handler destroys its temporary and unwinds on to the one below (the
	// element's region, then the enclosing objects'). A shared handler
	// was placed with the element that made it.
	if (!scope.active)
		return;
	for (size_t k = element_handlers_.size(); k-- > scope.handlers;)
		emit_marker(lowir::Opcode::EhEnd, lowir::kNoId);
	std::vector<ElementHandler>& handlers = element_scratch_;
	handlers.assign(element_handlers_.begin() + static_cast<std::ptrdiff_t>(scope.handlers), element_handlers_.end());
	element_handlers_.resize(scope.handlers);
	destroy_temporaries(scope.first);
	bool placing = false;
	for (const ElementHandler& handler : handlers)
		placing = placing || !handler.shared;
	if (placing)
	{
		const lowir::BlockId next = new_block(lowir::BlockKind::CallUnwindEnd);
		jump(next);
		for (size_t k = handlers.size(); k-- > 0;)
		{
			if (handlers[k].shared)
				continue;
			place(handlers[k].block);
			destroy_local(handlers[k].cleanup, lowir::kNoId, true);
			emit_resume();
		}
		place(next);
	}
	--explicit_regions_;
	--element_scopes_;
	full_expression_ = scope.full_expression;
	construction_ = scope.construction;
	region_open_ = scope.region_open;
	eager_pending_ = scope.eager_pending;
	nothrow_segment_ = scope.nothrow_segment;
	stale_region_ = scope.stale_region;
	deferred_region_ = scope.deferred_region;
	region_segment_ = scope.region_segment;
	hidden_eh_ = scope.hidden_eh;
	segment_temporaries_ = scope.segment_temporaries;
	tracked_.active = scope.tracked;
	if (!scope.share)
		return;
	element_sharing_ = false;
	element_shared_.swap(handlers);
	element_slots_.swap(element_slots_made_);
	element_key_ = element_key();
}

void FunctionLowering::construct_elements(const sema::Type* element, const ArrayBase& where, uint64_t count,
	const sema::FunctionEntity* constructor, uint32_t location)
{
	const uint64_t size = lowering_.size_of(element, location);
	bool throws = false;
	const bool temporaries = element_temporaries(constructor, &throws);
	const bool guarded = needs_destruction(element) && throws;
	// Elements whose construction makes temporaries: the enclosing objects'
	// region opens before the first (no call inside the elements opens it).
	if (temporaries && throws)
		before_throwing_call();
	if (count <= 8)
	{
		// Each element after the first in a region whose handler destroys
		// the ones before it.
		for (uint64_t i = 0; i < count; ++i)
		{
			if (i == 0 || !guarded)
			{
				const ElementScope scope = begin_element_temporaries(temporaries, true);
				construct(element, element_address(array_base(where, location), i, size), nullptr, constructor,
					EntryVariant::Complete, location);
				end_element_temporaries(scope);
				continue;
			}
			const lowir::BlockId dispatch = new_block(lowir::BlockKind::CallUnwindDispatch);
			const lowir::BlockId next = new_block(lowir::BlockKind::CallUnwindEnd);
			emit_marker(lowir::Opcode::EhCleanup, dispatch);
			const ElementScope scope = begin_element_temporaries(temporaries);
			construct(element, element_address(array_base(where, location), i, size), nullptr, constructor,
				EntryVariant::Complete, location);
			end_element_temporaries(scope);
			emit_marker(lowir::Opcode::EhEnd, lowir::kNoId);
			jump(next);
			place(dispatch);
			const lowir::BlockId guard = open_unwind_guard(destructor_may_throw(element));
			destroy_elements(element, where, i, location);
			close_unwind_guard(guard);
			element_exit();
			place_unwind_guard(guard);
			place(next);
		}
		return;
	}
	// A loop over an index slot; a throwing construction destroys the
	// elements before the index, last first.
	const lowir::Type i64 = lowir::Type::scalar(lowir::TypeKind::I64);
	Place index_slot = Place();
	index_slot.storage = lowir::Value::slot(generated_slot(lowir::SlotKind::ConstructorArrayIndex, i64));
	store(i64, integer(i64, 0), index_slot);
	if (throws && !temporaries)
		before_throwing_call();
	const lowir::BlockId condition = new_block(lowir::BlockKind::ConstructorArrayCond);
	const lowir::BlockId body = new_block(lowir::BlockKind::ConstructorArrayBody);
	const lowir::BlockId end = new_block(lowir::BlockKind::ConstructorArrayEnd);
	jump(condition);
	place(condition);
	const RValue current = load(i64, index_slot);
	branch(compare(lowir::ComparePredicate::Ult, i64, current, integer(i64, count)), body, end);
	place(body);
	lowir::BlockId cleanup = lowir::kNoId;
	if (guarded)
	{
		cleanup = new_block(lowir::BlockKind::ConstructorArrayCleanup);
		emit_marker(lowir::Opcode::EhCleanup, cleanup);
	}
	{
		RValue offset = current;
		const lowir::Value base = array_base(where, location);
		if (size != 1)
			offset = binary(lowir::BinaryOp::Mul, i64, offset, integer(i64, size));
		const lowir::Value object =
			index(lowir::Type::scalar(lowir::TypeKind::I8), ir_model::IPK_ARRAY_ELEMENT, base, offset).value;
		const ElementScope scope = begin_element_temporaries(temporaries);
		construct(element, object, nullptr, constructor, EntryVariant::Complete, location);
		end_element_temporaries(scope);
	}
	if (guarded)
		emit_marker(lowir::Opcode::EhEnd, lowir::kNoId);
	store(i64, binary(lowir::BinaryOp::Add, i64, current, integer(i64, 1)), index_slot);
	jump(condition);
	if (guarded)
	{
		const lowir::BlockId cleanup_body = new_block(lowir::BlockKind::ConstructorArrayCleanupBody);
		const lowir::BlockId resume = new_block(lowir::BlockKind::ConstructorArrayResume);
		place(cleanup);
		const RValue constructed = load(i64, index_slot);
		branch(compare(lowir::ComparePredicate::Ne, i64, constructed, integer(i64, 0)), cleanup_body, resume);
		place(cleanup_body);
		const RValue last = binary(lowir::BinaryOp::Sub, i64, constructed, integer(i64, 1));
		store(i64, last, index_slot);
		RValue offset = last;
		const lowir::Value base = array_base(where, location);
		if (size != 1)
			offset = binary(lowir::BinaryOp::Mul, i64, offset, integer(i64, size));
		const lowir::BlockId guard = open_unwind_guard(destructor_may_throw(element));
		destroy(element,
			index(lowir::Type::scalar(lowir::TypeKind::I8), ir_model::IPK_ARRAY_ELEMENT, base, offset).value, location);
		close_unwind_guard(guard);
		jump(cleanup);
		place_unwind_guard(guard);
		place(resume);
		element_exit();
	}
	place(end);
}

void FunctionLowering::destroy_elements_guarded(
	const sema::Type* element, const ArrayBase& where, uint64_t count, uint32_t location)
{
	// A loop from the last element down, the array's address taken once,
	// each destruction under a cleanup record; its landing continues the
	// loop from the index slot (the element that threw), each destruction
	// there in a terminate guard (15.2p2, 15.5.1p1; the course's form).
	const uint64_t size = lowering_.size_of(element, location);
	const lowir::Type i64 = lowir::Type::scalar(lowir::TypeKind::I64);
	const lowir::Type i8 = lowir::Type::scalar(lowir::TypeKind::I8);
	const lowir::Value base = array_base(where, location);
	Place index_slot = Place();
	index_slot.storage = lowir::Value::slot(generated_slot(lowir::SlotKind::DestructorArrayIndex, i64));
	store(i64, integer(i64, count), index_slot);
	const lowir::BlockId condition = new_block(lowir::BlockKind::DestructorArrayCond);
	const lowir::BlockId body = new_block(lowir::BlockKind::DestructorArrayBody);
	const lowir::BlockId end = new_block(lowir::BlockKind::DestructorArrayEnd);
	const lowir::BlockId handler = new_block(lowir::BlockKind::DestructorArrayCleanup);
	jump(condition);
	place(condition);
	const RValue remaining = load(i64, index_slot);
	branch(compare(lowir::ComparePredicate::Ne, i64, remaining, integer(i64, 0)), body, end);
	place(body);
	const RValue last = binary(lowir::BinaryOp::Sub, i64, remaining, integer(i64, 1));
	store(i64, last, index_slot);
	emit_marker(lowir::Opcode::EhCleanup, handler);
	const RValue offset = size != 1 ? binary(lowir::BinaryOp::Mul, i64, last, integer(i64, size)) : last;
	destroy(element, index(i8, ir_model::IPK_ARRAY_ELEMENT, base, offset).value, location);
	emit_marker(lowir::Opcode::EhEnd, lowir::kNoId);
	jump(condition);
	const lowir::BlockId handler_body = new_block(lowir::BlockKind::DestructorArrayCleanupBody);
	const lowir::BlockId resume = new_block(lowir::BlockKind::DestructorArrayResume);
	place(handler);
	const RValue left = load(i64, index_slot);
	branch(compare(lowir::ComparePredicate::Ne, i64, left, integer(i64, 0)), handler_body, resume);
	place(handler_body);
	const RValue previous = binary(lowir::BinaryOp::Sub, i64, left, integer(i64, 1));
	store(i64, previous, index_slot);
	const lowir::BlockId guard = open_unwind_guard(destructor_may_throw(element));
	const RValue previous_offset = size != 1 ? binary(lowir::BinaryOp::Mul, i64, previous, integer(i64, size))
											 : previous;
	destroy(element, index(i8, ir_model::IPK_ARRAY_ELEMENT, base, previous_offset).value, location);
	close_unwind_guard(guard);
	jump(handler);
	place_unwind_guard(guard);
	place(resume);
	emit_resume();
	place(end);
}

void FunctionLowering::destroy_array_loop(
	const sema::Type* element, lowir::Value base, uint64_t count, uint32_t location)
{
	// More than eight elements: a loop from the last down, the array's
	// address taken once.
	const lowir::Type i64 = lowir::Type::scalar(lowir::TypeKind::I64);
	const uint64_t size = lowering_.size_of(element, location);
	Place index_slot = Place();
	index_slot.storage = lowir::Value::slot(generated_slot(lowir::SlotKind::DestructorArrayIndex, i64));
	store(i64, integer(i64, count), index_slot);
	const lowir::BlockId condition = new_block(lowir::BlockKind::DestructorArrayCond);
	const lowir::BlockId body = new_block(lowir::BlockKind::DestructorArrayBody);
	const lowir::BlockId end = new_block(lowir::BlockKind::DestructorArrayEnd);
	jump(condition);
	place(condition);
	const RValue remaining = load(i64, index_slot);
	branch(compare(lowir::ComparePredicate::Ne, i64, remaining, integer(i64, 0)), body, end);
	place(body);
	const RValue last = binary(lowir::BinaryOp::Sub, i64, remaining, integer(i64, 1));
	store(i64, last, index_slot);
	RValue offset = last;
	if (size != 1)
		offset = binary(lowir::BinaryOp::Mul, i64, offset, integer(i64, size));
	destroy(element, index(lowir::Type::scalar(lowir::TypeKind::I8), ir_model::IPK_ARRAY_ELEMENT, base, offset).value,
		location);
	jump(condition);
	place(end);
}

// -------------------------------------------- constructors and destructors

void FunctionLowering::subobject_constructed(const Subobject& subobject, uint32_t location)
{
	// 15.2p2: from here on the constructor's unwinding destroys this
	// subobject too: the previous region ends and one whose cleanup
	// destroys it, then the earlier ones, begins.
	(void)location;
	if (constructor_region_)
		emit_marker(lowir::Opcode::EhEnd, lowir::kNoId);
	const lowir::BlockId block = new_block(lowir::BlockKind::ConstructorCleanup);
	// A detached body's cleanup: the landing pops its record and enters
	// the entry, which the body's unwinding reaches too.
	const lowir::BlockId entry = body_detached_ ? new_block(lowir::BlockKind::ConstructorCleanupEntry) : lowir::kNoId;
	const lowir::BlockId previous = !constructor_region_ ? lowir::kNoId
		: body_detached_                                 ? constructor_entry_
														 : constructor_cleanup_;
	deferred_.push_back(DeferredCleanup{ block, subobject, previous, entry, eh_contexts_.size() });
	emit_marker(lowir::Opcode::EhCleanup, block);
	constructor_region_ = true;
	constructor_cleanup_ = block;
	if (body_detached_)
	{
		constructor_entry_ = entry;
		body_target_ = entry;
		body_depth_ = eh_contexts_.size();
	}
	// The unwinding chains made so far resume under the previous region:
	// later ones are made again (a landing is entered in one state, pa26
	// README 7). In a function-try-block they continue at the
	// subobjects' cleanup (function_try_subobjects).
	if (eh_contexts_.empty())
	{
		resume_block_ = conditional_resume_ = lowir::kNoId;
		for (Cleanup& cleanup : cleanups_)
			cleanup.unwind = cleanup.conditional = lowir::kNoId;
	}
	else if (function_try_subobjects())
	{
		eh_contexts_[0].chain[0] = eh_contexts_[0].chain[1] = 0;
		for (Cleanup& cleanup : cleanups_)
			cleanup.eh_marked = cleanup.eh_unmarked = 0;
	}
}

void FunctionLowering::destroy_subobject(const Subobject& subobject, uint32_t location, bool unwinding)
{
	// A base by its destructor's base entry (the complete entry when that
	// is an alias of it), a member or element by its complete destructor,
	// each addressed again from `this`. On the normal path an array of
	// more than eight elements whose destructor may throw destroys the
	// elements before the one that threw. A cleanup's destruction may be
	// lowered while a base's construction call is (its arguments'
	// landings destroy the subobjects constructed before it): that call's
	// hidden arguments wait (hidden_base_ is restored after).
	if (subobject.deallocation)
	{
		deallocate_object(location);
		return;
	}
	// 15.5.1p1: one whose destructor may throw, during unwinding, in a
	// region that terminates.
	if (unwinding && !terminate_wrapped_ && subobject_destructor_may_throw(subobject))
	{
		emit_marker(lowir::Opcode::EhTry, cleanup_terminate(false));
		terminate_wrapped_ = true;
		destroy_subobject(subobject, location, true);
		terminate_wrapped_ = false;
		emit_marker(lowir::Opcode::EhEnd, lowir::kNoId);
		return;
	}
	if (subobject.base != nullptr)
	{
		// A base with virtual bases by its base-object entry, with its part
		// of the VTT.
		const sema::ClassEntity* cls = static_cast<const sema::ClassEntity*>(subobject.base->base);
		const sema::FunctionEntity* destructor = cls->destructor;
		const lowir::Value object = subobject_address(subobject.base, nullptr, location);
		const sema::ClassEntity* entity = sema::ClassOf(function_->owner->entity->type);
		const HiddenBase waiting = hidden_base_;
		hidden_base_.active = false;
		if (VirtualBaseCount(cls) != 0)
			hidden_base_ = HiddenBase{ true,
				subobject.base->is_virtual ? lowering_.vtt_virtual_index(entity, cls)
										   : lowering_.vtt_base_index(entity, subobject.base) };
		// A destructor destroys a virtual base by its base entry, a
		// definition of its own (the course's form; a constructor's cleanup
		// by the complete entry).
		const bool own_entry = subobject.base->is_virtual && function_->destructor;
		if (own_entry)
			lowering_.demand_base_entry(destructor);
		call_special(destructor,
			NonInlineDestructor(destructor) || VirtualBaseCount(cls) != 0 || own_entry ? EntryVariant::Base
																					   : EntryVariant::Complete,
			object, syntax::Slice<syntax::Expr*>(), location);
		hidden_base_ = waiting;
		return;
	}
	if (subobject.member == nullptr)
	{
		// A delegating constructor's whole object (12.6.2p6), by the
		// destructor's entry of the constructor entry's kind.
		const sema::ClassEntity* whole = sema::ClassOf(function_->owner->entity->type);
		const sema::FunctionEntity* destructor = whole->destructor;
		const bool hidden = base_entry_ && VirtualBaseCount(whole) != 0;
		const HiddenBase waiting = hidden_base_;
		hidden_base_.active = false;
		if (hidden)
			hidden_base_ = HiddenBase{ true, 0 };
		call_special(destructor,
			base_entry_ && (NonInlineDestructor(destructor) || hidden) ? EntryVariant::Base : EntryVariant::Complete,
			this_pointer(location), syntax::Slice<syntax::Expr*>(), location);
		hidden_base_ = waiting;
		return;
	}
	const sema::Type* type = subobject.member->variable->type;
	if (type->kind != sema::TypeKind::Array)
	{
		destroy(type, subobject_address(nullptr, subobject.member, location), location);
		return;
	}
	const sema::Type* element = type;
	uint64_t count = 1;
	while (element->kind == sema::TypeKind::Array)
	{
		count *= element->bound;
		element = element->element;
	}
	const ArrayBase where = ArrayBase{ Place(), subobject.member };
	if (subobject.element >= 0)
	{
		const uint64_t size = lowering_.size_of(type->element, location);
		destroy(type->element,
			element_address(array_base(where, location), static_cast<uint64_t>(subobject.element), size), location,
			true);
		return;
	}
	if (count > 8 && !unwinding && destructor_may_throw(element))
		destroy_elements_guarded(element, where, count, location);
	else if (count > 8)
		destroy_array_loop(element, array_base(where, location), count, location);
	else if (type->element->kind == sema::TypeKind::Array)
	{
		// A multidimensional array row by row, each row's elements at their
		// offsets in it.
		const uint64_t size = lowering_.size_of(type->element, location);
		for (uint64_t i = type->bound; i-- > 0;)
			destroy(type->element, element_address(array_base(where, location), i, size), location, true);
	}
	else
		destroy_elements(element, where, count, location);
}

void FunctionLowering::destruction_order(std::vector<Subobject>& out, uint32_t location)
{
	// 12.4p8: the members in reverse declaration order (an array's
	// elements last first, one unit beyond eight), then the direct bases
	// in reverse declaration order (12.6.2p10).
	const sema::ClassEntity* entity = sema::ClassOf(function_->owner->entity->type);
	std::vector<const sema::DataMember*>& members = member_scratch_;
	const size_t first = members.size();
	// A union's members are variant members: not destroyed (12.4p8).
	for (const sema::DataMember* member = entity->key == TokenKind::KW_UNION ? nullptr : entity->members;
		member != nullptr; member = member->next)
		members.push_back(member);
	for (size_t i = members.size(); i-- > first;)
	{
		const sema::DataMember* member = members[i];
		const sema::Type* type = member->variable->type;
		if (!needs_destruction(type))
			continue;
		if (type->kind != sema::TypeKind::Array)
		{
			out.push_back(Subobject{ nullptr, member, -1 });
			continue;
		}
		uint64_t count = 1;
		for (const sema::Type* element = type; element->kind == sema::TypeKind::Array; element = element->element)
			count *= element->bound;
		if (count > 8)
		{
			out.push_back(Subobject{ nullptr, member, -1 });
			continue;
		}
		// One unit per element of the array (a row of a multidimensional
		// one).
		for (uint64_t k = type->bound; k-- > 0;)
			out.push_back(Subobject{ nullptr, member, static_cast<int64_t>(k) });
	}
	members.resize(first);
	// The complete object's destructor then destroys the virtual bases,
	// in the reverse of their construction (12.4p8).
	const size_t bases = out.size();
	if (!base_entry_)
	{
		for (const sema::BaseClass* shared : virtual_base_order(entity))
		{
			if (needs_destruction(shared->base->type))
				out.push_back(Subobject{ shared, nullptr, -1 });
		}
	}
	for (const sema::BaseClass* base = entity->bases; base != nullptr; base = base->next)
	{
		const sema::ClassEntity* cls = static_cast<const sema::ClassEntity*>(base->base);
		if (!base->is_virtual && needs_destruction(cls->type))
			out.push_back(Subobject{ base, nullptr, -1 });
	}
	std::reverse(out.begin() + static_cast<std::ptrdiff_t>(bases), out.end());
	(void)location;
}

bool FunctionLowering::destructor_regions(uint32_t location)
{
	// A destructor with subobjects to destroy runs its body in a region
	// whose cleanup destroys them all.
	subobjects_.clear();
	destruction_order(subobjects_, location);
	return !subobjects_.empty();
}

void FunctionLowering::destructor_epilogue(uint32_t location)
{
	// The subobjects in order, each but the last in a region whose cleanup
	// destroys the ones after it. Up to eight each region has its own
	// cleanup; beyond, a progress slot records the region and the body's
	// cleanup dispatches on it into one chain of the subobjects'
	// destructions, so the code stays linear.
	const lowir::BlockId end = new_block(lowir::BlockKind::DestructorEnd);
	const size_t count = subobjects_.size();
	const lowir::Type i64 = lowir::Type::scalar(lowir::TypeKind::I64);
	Place progress = Place();
	progress.storage = lowir::Value::slot(destructor_progress_);
	for (size_t i = 0; i < count; ++i)
	{
		if (i + 1 == count)
		{
			destroy_subobject(subobjects_[i], location, false);
			break;
		}
		if (destructor_progress_ != lowir::kNoId)
		{
			store(i64, integer(i64, i + 1), progress);
			emit_marker(lowir::Opcode::EhCleanup, destructor_cleanup_);
			destroy_subobject(subobjects_[i], location, false);
			emit_marker(lowir::Opcode::EhEnd, lowir::kNoId);
			continue;
		}
		const lowir::BlockId cleanup = new_block(lowir::BlockKind::DestructorSuffixCleanup);
		const lowir::BlockId next = new_block(lowir::BlockKind::DestructorSuffixNext);
		emit_marker(lowir::Opcode::EhCleanup, cleanup);
		destroy_subobject(subobjects_[i], location, false);
		emit_marker(lowir::Opcode::EhEnd, lowir::kNoId);
		jump(next);
		place(cleanup);
		for (size_t k = i + 1; k < count; ++k)
			destroy_subobject(subobjects_[k], location, true);
		emit_marker(lowir::Opcode::EhEnd, lowir::kNoId);
		emit_resume();
		place(next);
	}
	jump(end);
	place(destructor_cleanup_);
	const bool detached = body_entry_ != lowir::kNoId;
	if (detached)
		open_detached_cleanup(body_entry_);
	if (destructor_progress_ == lowir::kNoId)
	{
		// The body's cleanup, then the end.
		for (const Subobject& subobject : subobjects_)
			destroy_subobject(subobject, location, true);
		close_detached_cleanup(detached);
		place(end);
		return;
	}
	// The dispatch: progress p (0 for the body) continues with the
	// destruction of subobject p onwards.
	std::vector<lowir::BlockId> chain(count);
	for (size_t i = 0; i < count; ++i)
		chain[i] = new_block(lowir::BlockKind::DestructorSuffixCleanup);
	scratch_.clear();
	for (size_t i = 0; i < count; ++i)
	{
		scratch_.push_back(integer(i64, i).value);
		scratch_.push_back(lowir::Value::block(chain[i]));
	}
	lowir::Instruction dispatch = lowir::make_instruction(
		lowir::Opcode::Switch, lowir::Type::scalar(lowir::TypeKind::Void));
	dispatch.ops[0] = load(i64, progress).value;
	dispatch.ops[1] = lowir::Value::block(chain[0]);
	dispatch.extra_first = builder_.add_operands(scratch_.data(), static_cast<uint32_t>(scratch_.size()));
	dispatch.extra_count = static_cast<uint32_t>(scratch_.size());
	terminate(dispatch);
	for (size_t i = 0; i < count; ++i)
	{
		place(chain[i]);
		destroy_subobject(subobjects_[i], location, true);
		if (i + 1 < count)
		{
			jump(chain[i + 1]);
			continue;
		}
		close_detached_cleanup(detached);
	}
	place(end);
}

} // namespace lower
