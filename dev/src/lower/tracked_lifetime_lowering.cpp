#include "lower/function_lowering.h"

#include <algorithm>

#include "sema/aggregates.h"
#include "sema/conversions.h"
#include "sema/expression_facts.h"

// Tracked temporary lifetimes (12.2p3 with 15.2p1): a temporary made
// on some paths of its full-expression only is destroyed only if it was
// made. Guarded temporaries, and the course's lifetime tracking: in a
// full-expression one of whose branches it outlives makes a temporary,
// every temporary it makes gets a u8 flag, all cleared at its start (the
// latest made first, as they are destroyed), each set once its temporary
// is constructed; its regions all land on one chain that destroys the
// temporaries whose flags are set, then the objects under them, made when
// the full-expression ends and placed after its first block (the course's
// form: its conditional cleanup dispatch).
namespace lower {

namespace {

const syntax::Expr* WithoutParentheses(const syntax::Expr* expr)
{
	while (expr->kind == syntax::ExprKind::Paren)
		expr = static_cast<const syntax::ParenExpr*>(expr)->inner;
	return expr;
}

bool BuiltinLogical(const syntax::Expr* e)
{
	if (e->kind != syntax::ExprKind::Binary)
		return false;
	const syntax::BinaryExpr* binary = static_cast<const syntax::BinaryExpr*>(e);
	return (binary->op == TokenKind::OP_LAND || binary->op == TokenKind::OP_LOR) &&
		(e->facts == nullptr || (e->facts->flags & (sema::kOperatorCall | sema::kUserConverted)) == 0);
}

} // namespace

bool FunctionLowering::branch_temporaries(const syntax::Expr* expr, unsigned depth)
{
	// Whether a conditional's operand or a logical operator's right operand
	// makes a temporary the full-expression destroys.
	if (expr == nullptr || depth > 64)
		return false;
	const syntax::Expr* e = WithoutParentheses(expr);
	switch (e->kind)
	{
	case syntax::ExprKind::Conditional:
	{
		// (A class prvalue's operands initialize its result itself.)
		const syntax::ConditionalExpr* choice = static_cast<const syntax::ConditionalExpr*>(e);
		const sema::Type* object = e->facts != nullptr && e->facts->category == sema::ValueCategory::PRValue &&
				e->facts->type != nullptr && sema::ClassOf(e->facts->type) != nullptr
			? e->facts->type
			: nullptr;
		return operand_temporaries(choice->then_expr, object) || operand_temporaries(choice->else_expr, object) ||
			branch_temporaries(choice->condition, depth + 1);
	}
	case syntax::ExprKind::Binary:
	case syntax::ExprKind::Assign:
	{
		const syntax::BinaryExpr* binary = static_cast<const syntax::BinaryExpr*>(e);
		if ((binary->op == TokenKind::OP_LAND || binary->op == TokenKind::OP_LOR) && creates_temporaries(binary->right))
			return true;
		return branch_temporaries(binary->left, depth + 1) || branch_temporaries(binary->right, depth + 1);
	}
	case syntax::ExprKind::Unary:
		return branch_temporaries(static_cast<const syntax::UnaryExpr*>(e)->operand, depth + 1);
	case syntax::ExprKind::Cast:
		return branch_temporaries(static_cast<const syntax::CastExpr*>(e)->operand, depth + 1);
	case syntax::ExprKind::Call:
	{
		for (const syntax::Expr* argument : static_cast<const syntax::CallExpr*>(e)->arguments)
		{
			if (branch_temporaries(argument, depth + 1))
				return true;
		}
		return false;
	}
	default:
		return false;
	}
}

const syntax::Expr* FunctionLowering::full_root(const syntax::Expr* expr)
{
	// A full-expression's root: returns the previous one (restored after).
	const syntax::Expr* saved = choice_root_;
	choice_root_ = expr != nullptr ? WithoutParentheses(expr) : nullptr;
	return saved;
}

bool FunctionLowering::branch_local(const syntax::Expr& expr) const
{
	// (A tracked full-expression destroys every temporary at its end.)
	return !tracked_.active && WithoutParentheses(&expr) == choice_root_;
}

void FunctionLowering::begin_guard_scope()
{
	// A conditional or logical expression whose branches' temporaries
	// outlive them: the outermost records where their flags are cleared
	// (before its branching).
	if (guard_scopes_++ != 0)
		return;
	if (terminated_)
		place(new_block(lowir::BlockKind::Unreachable));
	guard_block_ = builder_.insertion_block();
	guard_after_ = builder_.last_instruction(guard_block_);
}

bool FunctionLowering::guarded_temporaries(const syntax::Expr* expr, bool root, unsigned depth)
{
	// Whether evaluating the expression makes a temporary the
	// full-expression destroys in a branch it outlives (as the lowering
	// guards them): a class or glvalue conditional's operands, another
	// conditional's or a logical operator's right operand unless at the
	// root (their temporaries end with their branch there).
	if (expr == nullptr || depth > 64)
		return false;
	const syntax::Expr* e = WithoutParentheses(expr);
	const sema::ExprFacts* facts = e->facts;
	if (facts == nullptr || (facts->flags & sema::kFolded) != 0)
		return false;
	switch (e->kind)
	{
	case syntax::ExprKind::Conditional:
	{
		const syntax::ConditionalExpr* choice = static_cast<const syntax::ConditionalExpr*>(e);
		const bool prvalue = facts->category == sema::ValueCategory::PRValue;
		const sema::Type* object = prvalue && facts->type != nullptr && sema::ClassOf(facts->type) != nullptr
			? facts->type
			: nullptr;
		// A class result's conditional initializes it in each branch, whose
		// temporaries end with it (initialize_result).
		if (root && result_root_ != nullptr && (!prvalue || sema::ClassOf(facts->type) == result_root_))
			return guarded_temporaries(choice->condition, false, depth + 1) ||
				guarded_temporaries(choice->then_expr, true, depth + 1) ||
				guarded_temporaries(choice->else_expr, true, depth + 1);
		bool selected = false;
		if (prvalue && object == nullptr && !sema::IsVoid(facts->type) &&
			lowering_.constant_conversion(*choice->condition, selected))
			return guarded_temporaries(selected ? choice->then_expr : choice->else_expr, root, depth + 1);
		const bool guarded = !prvalue || object != nullptr || !root;
		if (guarded &&
			(operand_temporaries(choice->then_expr, object) || operand_temporaries(choice->else_expr, object)))
			return true;
		return guarded_temporaries(choice->condition, false, depth + 1) ||
			guarded_temporaries(choice->then_expr, false, depth + 1) ||
			guarded_temporaries(choice->else_expr, false, depth + 1);
	}
	case syntax::ExprKind::Binary:
	case syntax::ExprKind::Assign:
	{
		const syntax::BinaryExpr* binary = static_cast<const syntax::BinaryExpr*>(e);
		bool value = false;
		if (BuiltinLogical(e) && !root && !literal_truth(*binary->left, value) && creates_temporaries(binary->right))
			return true;
		return guarded_temporaries(binary->left, false, depth + 1) ||
			guarded_temporaries(binary->right, false, depth + 1);
	}
	case syntax::ExprKind::Unary:
	case syntax::ExprKind::Postfix:
		return guarded_temporaries(static_cast<const syntax::UnaryExpr*>(e)->operand, false, depth + 1);
	case syntax::ExprKind::Cast:
		return guarded_temporaries(static_cast<const syntax::CastExpr*>(e)->operand, false, depth + 1);
	case syntax::ExprKind::Member:
		return guarded_temporaries(static_cast<const syntax::MemberExpr*>(e)->base, false, depth + 1);
	case syntax::ExprKind::Subscript:
		return guarded_temporaries(static_cast<const syntax::SubscriptExpr*>(e)->base, false, depth + 1) ||
			guarded_temporaries(static_cast<const syntax::SubscriptExpr*>(e)->index, false, depth + 1);
	case syntax::ExprKind::Call:
	{
		const syntax::CallExpr* call = static_cast<const syntax::CallExpr*>(e);
		if (guarded_temporaries(call->callee, false, depth + 1))
			return true;
		for (const syntax::Expr* argument : call->arguments)
		{
			if (guarded_temporaries(argument, false, depth + 1))
				return true;
		}
		return false;
	}
	case syntax::ExprKind::FunctionalCast:
	{
		const syntax::FunctionalCastExpr* cast = static_cast<const syntax::FunctionalCastExpr*>(e);
		for (const syntax::Expr* argument : cast->arguments)
		{
			if (guarded_temporaries(argument, false, depth + 1))
				return true;
		}
		return guarded_temporaries(cast->braced, false, depth + 1);
	}
	case syntax::ExprKind::BracedInit:
		for (const syntax::Expr* element : static_cast<const syntax::BracedInitList*>(e)->elements)
		{
			if (guarded_temporaries(element, false, depth + 1))
				return true;
		}
		return false;
	case syntax::ExprKind::New:
	{
		// A non-throwing allocation's object is initialized only when it is
		// not null: the initializer's temporaries are made on that path.
		const syntax::NewExpr* allocation = static_cast<const syntax::NewExpr*>(e);
		const syntax::Initializer* init = allocation->initializer;
		bool made = false;
		if (init != nullptr && init->kind == syntax::InitializerKind::Parenthesized)
		{
			for (const syntax::Expr* argument : init->arguments)
				made = made || creates_temporaries(argument);
		}
		else if (init != nullptr)
			made = creates_temporaries(init->expression, nullptr, false, facts->target);
		return made && allocation->count == nullptr && tests_allocation(*allocation);
	}
	default:
		return false;
	}
}

bool FunctionLowering::operand_temporaries(const syntax::Expr* operand, const sema::Type* object)
{
	// Whether a conditional's operand initializing its result (of class
	// `object`, else a value) makes a temporary: an aggregate's braced list
	// initializes its members itself.
	const syntax::Expr* e = operand != nullptr ? WithoutParentheses(operand) : nullptr;
	const sema::ClassEntity* entity = object != nullptr ? sema::ClassOf(object) : nullptr;
	const syntax::BracedInitList* list = e == nullptr ? nullptr
		: e->kind == syntax::ExprKind::BracedInit     ? static_cast<const syntax::BracedInitList*>(e)
		: e->kind == syntax::ExprKind::FunctionalCast ? static_cast<const syntax::FunctionalCastExpr*>(e)->list()
													  : nullptr;
	if (list == nullptr || entity == nullptr || !sema::IsAggregateClass(entity))
		return creates_temporaries(operand, nullptr, false, object);
	for (const syntax::Expr* element : list->elements)
	{
		const syntax::Expr* item = WithoutParentheses(element);
		const sema::Type* type = item->facts != nullptr ? item->facts->type : nullptr;
		if (item->kind == syntax::ExprKind::BracedInit ? creates_temporaries(element)
													   : creates_temporaries(element, nullptr, false, type))
			return true;
	}
	return false;
}

void FunctionLowering::begin_tracking(const syntax::Expr* const* exprs, size_t count, bool root, bool discarded)
{
	// A full-expression's segment began: its temporaries are tracked when
	// one is made in a branch it outlives. A discarded one with nothing
	// under it opens its region at once (the course's segment starts with
	// the full-expression).
	if (!full_expression_ || tracked_.active || element_scopes_ != 0 || static_initializer_ ||
		lowering_.host_objects() || body_unwind_target() != lowir::kNoId ||
		lowering_.graph().nontrivial_destructors == 0)
		return;
	bool found = false;
	for (size_t i = 0; i < count && !found; ++i)
		found = guarded_temporaries(exprs[i], root, 0);
	if (!found)
		return;
	tracked_ = TrackedLifetimes();
	tracked_.active = true;
	tracked_.block = segment_block_;
	tracked_.after = segment_after_;
	tracked_.first_reset = lowir::kNoId;
	tracked_.head = lowir::kNoId;
	tracked_.floor = cleanups_.size();
	tracked_.markers = tracked_markers_.size();
	if (discarded && chain_empty())
		open_region(true, true);
}

void FunctionLowering::track_reset(const lowir::Instruction& clear)
{
	// A tracked temporary's flag is cleared at the full-expression's start,
	// before the flags cleared already; the first moves the positions
	// recorded there to after it (a region opening there follows the
	// flags).
	++lowering_.counters().instructions;
	const lowir::InstId id = builder_.insert_after(tracked_.block, tracked_.after, clear);
	if (tracked_.first_reset != lowir::kNoId)
		return;
	tracked_.first_reset = id;
	const auto moved = [this, id](lowir::BlockId block, lowir::InstId& after) {
		if (block == tracked_.block && after == tracked_.after)
			after = id;
	};
	moved(segment_block_, segment_after_);
	moved(construction_block_, construction_after_);
	moved(guard_block_, guard_after_);
	moved(declared_start_block_, declared_start_after_);
	for (CallMark& mark : call_marks_)
		moved(mark.block, mark.after);
}

lowir::BlockId FunctionLowering::tracked_target()
{
	// The landing of a tracked full-expression's regions: a placeholder
	// until its chain is made (finish_tracking retargets the markers).
	if (tracked_.made)
		return tracked_.head;
	if (tracked_.head == lowir::kNoId)
		tracked_.head = new_block(lowir::BlockKind::ConditionalCleanupLanding);
	return tracked_.head;
}

lowir::Value FunctionLowering::tracked_storage(lowir::Value address)
{
	// The slot whose address `address` is (a slot's address taken again
	// where it is destroyed), else the address itself.
	if (!address.is(lowir::ValueKind::Temp))
		return address;
	const lowir::FunctionBody& body = builder_.body();
	const lowir::InstId def = body.temps[address.index].def;
	if (def == lowir::kNoId || body.instructions[def].op != lowir::Opcode::Addr ||
		!body.instructions[def].ops[0].is(lowir::ValueKind::Slot))
		return address;
	return body.instructions[def].ops[0];
}

void FunctionLowering::note_region(lowir::InstId marker)
{
	if (!tracked_.active || tracked_.made || tracked_.head == lowir::kNoId || marker == lowir::kNoId)
		return;
	const lowir::Value target = builder_.body().instructions[marker].ops[0];
	if (target.is(lowir::ValueKind::Block) && target.index == tracked_.head)
		tracked_markers_.push_back(marker);
}

const FunctionLowering::CallMark* FunctionLowering::earliest_mark() const
{
	// The earliest throwing call still being evaluated that began in this
	// block (a tracked region opens there).
	if (terminated_)
		return nullptr;
	const lowir::BlockId block = builder_.insertion_block();
	for (const CallMark& mark : call_marks_)
	{
		if (mark.block == block && !mark.declared)
			return &mark;
	}
	return nullptr;
}

void FunctionLowering::finish_tracking()
{
	// The tracked full-expression's chain, over every object alive now
	// (its temporaries, flag-checked, then the objects under them): outside
	// exception-handling contexts the large-set chain (its blocks shared
	// with later ones); inside them a chain of its own, its landing first.
	if (!tracked_.active || tracked_.made)
		return;
	tracked_.made = true;
	const size_t count = cleanups_.size();
	std::vector<ChainBlock> made;
	lowir::BlockId head = lowir::kNoId;
	if (eh_contexts_.empty())
	{
		const size_t mark = late_blocks_.size();
		head = chain_entry(ChainKind::Conditional, conditional_resume_, count);
		made.assign(late_blocks_.begin() + static_cast<std::ptrdiff_t>(mark), late_blocks_.end());
		late_blocks_.resize(mark);
	}
	else
	{
		save_chain_memos();
		const uint32_t stack = eh_contexts_.back().stack;
		eh_contexts_.back().stack = ++eh_stack_count_;
		const size_t mark = chain_blocks_.size();
		head = region_landing(count);
		made.assign(chain_blocks_.begin() + static_cast<std::ptrdiff_t>(mark), chain_blocks_.end());
		chain_blocks_.resize(mark);
		eh_contexts_.back().stack = stack;
		restore_chain_memos();
		if (!made.empty())
			std::rotate(made.begin(), made.end() - 1, made.end());
	}
	// The tracked temporaries' slots are addressed again where they are
	// destroyed (one a reference extends unconditionally only in the
	// chain; see tracked_storage).
	lowir::FunctionBody& body = builder_.body();
	for (ChainBlock& block : made)
	{
		if (block.cleanup.guarded)
			block.cleanup.storage = tracked_storage(block.cleanup.storage);
	}
	for (size_t i = tracked_.floor; i < cleanups_.size(); ++i)
	{
		if (cleanups_[i].guarded && !cleanups_[i].temporary && !cleanups_[i].branched)
			cleanups_[i].storage = cleanups_[i].made_at;
	}
	for (size_t i = tracked_.markers; i < tracked_markers_.size(); ++i)
		body.instructions[tracked_markers_[i]].ops[0] = lowir::Value::block(head);
	tracked_markers_.resize(tracked_.markers);
	tracked_.head = head;
	place_tracked(made);
}

void FunctionLowering::place_tracked(const std::vector<ChainBlock>& made)
{
	// The chain's blocks follow the full-expression's first block; the
	// lowering's position and pending placements are set aside meanwhile.
	const lowir::BlockId block = builder_.insertion_block();
	const bool terminated = terminated_;
	const bool open = region_open_;
	const bool segment = region_segment_;
	std::vector<DeferredCleanup> deferred;
	std::vector<ChainBlock> lexical;
	std::vector<TerminateBlock> terminates;
	deferred.swap(deferred_);
	lexical.swap(lexical_pending_);
	terminates.swap(pending_terminates_);
	region_open_ = region_segment_ = false;
	terminated_ = true;
	std::vector<lowir::BlockId>& layout = builder_.body().layout;
	const size_t before = layout.size();
	place_blocks(made);
	if (!pending_terminates_.empty())
		place_terminates();
	size_t at = before;
	while (at-- > 0 && layout[at] != tracked_.block)
	{
	}
	if (at != SIZE_MAX)
		std::rotate(layout.begin() + static_cast<std::ptrdiff_t>(at + 1),
			layout.begin() + static_cast<std::ptrdiff_t>(before), layout.end());
	deferred.swap(deferred_);
	lexical.swap(lexical_pending_);
	terminates.swap(pending_terminates_);
	region_open_ = open;
	region_segment_ = segment;
	terminated_ = terminated;
	builder_.set_insertion_block(block);
}

void FunctionLowering::end_tracking()
{
	// The full-expression ends: a temporary a reference extended (12.2p5)
	// is destroyed with its scope through blocks of its own, only if made
	// when one of its branches made it.
	if (!tracked_.active)
		return;
	finish_tracking();
	for (size_t i = tracked_.floor; i < cleanups_.size(); ++i)
	{
		Cleanup& cleanup = cleanups_[i];
		if (!cleanup.guarded || cleanup.temporary)
			continue;
		cleanup.guarded = cleanup.branched;
		cleanup.unwind = cleanup.shared_return = cleanup.conditional = lowir::kNoId;
		cleanup.eh_marked = cleanup.eh_unmarked = 0;
		cleanup.lexical = 0;
	}
	tracked_.active = false;
	tracked_.made = false;
}

} // namespace lower
