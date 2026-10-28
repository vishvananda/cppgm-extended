#include "lower/function_lowering.h"

#include <cstring>

#include "lex/identifier_table.h"
#include "lowir/literals.h"
#include "sema/aggregates.h"
#include "sema/conversions.h"
#include "sema/statement_facts.h"
#include "sema/string_literals.h"

// Statements (clause 6) as blocks in source order. Each construct creates
// its blocks when it starts and places them as control reaches them; a
// block ends in one terminator, and statements that control cannot reach
// (after a return, break, continue or goto, until a label) emit nothing.
namespace lower {

namespace {

const syntax::Expr* WithoutParentheses(const syntax::Expr* expr)
{
	while (expr->kind == syntax::ExprKind::Paren)
		expr = static_cast<const syntax::ParenExpr*>(expr)->inner;
	return expr;
}

// Whether an expression names a member (a class member access or `this`),
// or is of a form not examined.
bool NamesMember(const syntax::Expr& expr, uint32_t depth)
{
	const syntax::Expr* e = WithoutParentheses(&expr);
	if (depth > 64)
		return true;
	switch (e->kind)
	{
	case syntax::ExprKind::Literal:
	case syntax::ExprKind::Id:
	case syntax::ExprKind::Sizeof:
	case syntax::ExprKind::SizeofPack:
	case syntax::ExprKind::TypeTrait:
	case syntax::ExprKind::BuiltinTrait:
		return false;
	case syntax::ExprKind::KeywordLiteral:
		return static_cast<const syntax::KeywordLiteralExpr*>(e)->keyword == TokenKind::KW_THIS;
	case syntax::ExprKind::Binary:
		return NamesMember(*static_cast<const syntax::BinaryExpr*>(e)->left, depth + 1) ||
			NamesMember(*static_cast<const syntax::BinaryExpr*>(e)->right, depth + 1);
	case syntax::ExprKind::Unary:
		return NamesMember(*static_cast<const syntax::UnaryExpr*>(e)->operand, depth + 1);
	case syntax::ExprKind::Conditional:
	{
		const syntax::ConditionalExpr* choice = static_cast<const syntax::ConditionalExpr*>(e);
		return NamesMember(*choice->condition, depth + 1) || NamesMember(*choice->then_expr, depth + 1) ||
			NamesMember(*choice->else_expr, depth + 1);
	}
	case syntax::ExprKind::Cast:
		return NamesMember(*static_cast<const syntax::CastExpr*>(e)->operand, depth + 1);
	case syntax::ExprKind::Call:
	{
		const syntax::CallExpr* call = static_cast<const syntax::CallExpr*>(e);
		if (NamesMember(*call->callee, depth + 1))
			return true;
		for (const syntax::Expr* argument : call->arguments)
		{
			if (NamesMember(*argument, depth + 1))
				return true;
		}
		return false;
	}
	default:
		return true;
	}
}

// Whether bfloat16 arithmetic is evaluated in the expression (see
// FunctionLowering::bfloat_operation).
bool BFloatArithmeticIn(const syntax::Expr& expr, uint32_t depth)
{
	const syntax::Expr* e = WithoutParentheses(&expr);
	if (depth > 64 || e->facts == nullptr || e->facts->type == nullptr)
		return false;
	switch (e->kind)
	{
	case syntax::ExprKind::Binary:
	{
		const syntax::BinaryExpr* binary = static_cast<const syntax::BinaryExpr*>(e);
		if (SoftFloatOf(e->facts->type) == SoftFloat::BFloat &&
			(binary->op == TokenKind::OP_PLUS || binary->op == TokenKind::OP_MINUS ||
				binary->op == TokenKind::OP_STAR || binary->op == TokenKind::OP_DIV))
			return true;
		return BFloatArithmeticIn(*binary->left, depth + 1) || BFloatArithmeticIn(*binary->right, depth + 1);
	}
	case syntax::ExprKind::Unary:
	{
		const syntax::UnaryExpr* unary = static_cast<const syntax::UnaryExpr*>(e);
		return (SoftFloatOf(e->facts->type) == SoftFloat::BFloat &&
				   (unary->op == TokenKind::OP_PLUS || unary->op == TokenKind::OP_MINUS)) ||
			BFloatArithmeticIn(*unary->operand, depth + 1);
	}
	case syntax::ExprKind::Conditional:
	{
		const syntax::ConditionalExpr* choice = static_cast<const syntax::ConditionalExpr*>(e);
		return BFloatArithmeticIn(*choice->condition, depth + 1) || BFloatArithmeticIn(*choice->then_expr, depth + 1) ||
			BFloatArithmeticIn(*choice->else_expr, depth + 1);
	}
	case syntax::ExprKind::Cast:
		return BFloatArithmeticIn(*static_cast<const syntax::CastExpr*>(e)->operand, depth + 1);
	default:
		return false;
	}
}

// Whether a non-constexpr automatic array may be copied from a constant
// image of its initializer (the course's form): one dimension of
// nonvolatile arithmetic, enumeration, pointer or std::nullptr_t elements.
bool ScalarImageArray(const sema::Type* type)
{
	if (type->kind != sema::TypeKind::Array || type->bound == 0 || (type->cv & (sema::kVolatile | sema::kAtomic)) != 0)
		return false;
	const sema::Type* element = type->element;
	if ((element->cv & (sema::kVolatile | sema::kAtomic)) != 0)
		return false;
	switch (element->unqualified->kind)
	{
	case sema::TypeKind::Fundamental:
		return element->unqualified->fundamental != FundamentalType::Void;
	case sema::TypeKind::Enum:
	case sema::TypeKind::Pointer:
		return true;
	default:
		return false;
	}
}
} // namespace

void FunctionLowering::statement(const syntax::Stmt& stmt)
{
	// Unreachable code is dropped unless a label inside makes part of it
	// reachable again (6.4.2p5 case labels, 6.6.4 goto targets).
	check_stack(stmt.location);
	const DebugScope debug(*this, stmt.location);
	if (terminated_ && !reachable_label(stmt))
	{
		// A declaration still gets its storage: a label after it, in its
		// scope, reaches code that may use it (6.7p3 allows jumping past a
		// declaration without initialization).
		const syntax::Stmt& chosen = syntax::ChosenReading(stmt);
		if (chosen.kind == syntax::StmtKind::Declaration)
			unreachable_declaration(*static_cast<const syntax::DeclarationStmt&>(chosen).declaration);
		return;
	}
	switch (stmt.kind)
	{
	case syntax::StmtKind::Ambiguous:
		return statement(static_cast<const syntax::AmbiguousStmt&>(stmt).reading());
	case syntax::StmtKind::Compound:
		return compound(static_cast<const syntax::CompoundStmt&>(stmt));
	case syntax::StmtKind::Expression:
	{
		const syntax::Expr* expr = static_cast<const syntax::ExpressionStmt&>(stmt).expression;
		const syntax::Expr* root = expr != nullptr ? WithoutParentheses(expr) : nullptr;
		if (root != nullptr && root->kind == syntax::ExprKind::Throw)
			return throw_statement(static_cast<const syntax::ThrowExpr*>(root)->operand, root->location);
		if (expr != nullptr)
		{
			const size_t first = cleanups_.size();
			const bool region = begin_full_expression(expr, true);
			const syntax::Expr* saved = full_root(expr);
			discard(*expr);
			choice_root_ = saved;
			destroy_temporaries(first);
			end_full_expression(region);
		}
		return;
	}
	case syntax::StmtKind::Declaration:
		return declaration_statement(*static_cast<const syntax::DeclarationStmt&>(stmt).declaration);
	case syntax::StmtKind::Asm:
		return asm_statement(static_cast<const syntax::AsmStmt&>(stmt));
	case syntax::StmtKind::If:
		return if_statement(static_cast<const syntax::IfStmt&>(stmt));
	case syntax::StmtKind::While:
		return while_statement(static_cast<const syntax::ConditionalLoopStmt&>(stmt));
	case syntax::StmtKind::Do:
		return do_statement(static_cast<const syntax::DoStmt&>(stmt));
	case syntax::StmtKind::For:
		return for_statement(static_cast<const syntax::ForStmt&>(stmt));
	case syntax::StmtKind::RangeFor:
		return range_for_statement(static_cast<const syntax::RangeForStmt&>(stmt));
	case syntax::StmtKind::Switch:
		return switch_statement(static_cast<const syntax::ConditionalLoopStmt&>(stmt));
	case syntax::StmtKind::Case:
	case syntax::StmtKind::Default:
	{
		enter_label(switch_label(stmt));
		const syntax::Stmt* body = stmt.kind == syntax::StmtKind::Case
			? static_cast<const syntax::CaseStmt&>(stmt).body
			: static_cast<const syntax::LabeledStmt&>(stmt).body;
		return statement(*body);
	}
	case syntax::StmtKind::Labeled:
	{
		const syntax::LabeledStmt& labeled = static_cast<const syntax::LabeledStmt&>(stmt);
		enter_label(goto_label(labeled.label));
		return statement(*labeled.body);
	}
	case syntax::StmtKind::Goto:
		return goto_statement(static_cast<const syntax::GotoStmt&>(stmt));
	case syntax::StmtKind::Break:
		// 6.6.1: to the end of the innermost loop or switch, destroying the
		// objects of the scopes left (6.6p2).
		destroy_cleanups(jumps_.back().cleanup_depth, false, jumps_.back().context_depth);
		return jump(jumps_.back().break_target);
	case syntax::StmtKind::Continue:
	{
		// 6.6.2: to the continuation of the innermost loop.
		for (size_t i = jumps_.size(); i-- > 0;)
		{
			if (jumps_[i].continue_target != lowir::kNoId)
			{
				destroy_cleanups(jumps_[i].cleanup_depth, false, jumps_[i].context_depth);
				return jump(jumps_[i].continue_target);
			}
		}
		throw LoweringError(stmt.location, "continue outside a loop");
	}
	case syntax::StmtKind::Return:
		return return_statement(static_cast<const syntax::ValueStmt&>(stmt));
	case syntax::StmtKind::Try:
		return try_statement(static_cast<const syntax::TryStmt&>(stmt));
	case syntax::StmtKind::Throw:
		return throw_statement(static_cast<const syntax::ValueStmt&>(stmt).value, stmt.location);
	default:
		throw LoweringError(stmt.location, "a statement outside the procedural lowering slice");
	}
}

bool FunctionLowering::reachable_label(const syntax::Stmt& stmt)
{
	// A case, default or identifier label within the statement: a jump may
	// enter it there. Compound statements remember the answer, so skipping
	// nested unreachable code visits each statement once.
	switch (stmt.kind)
	{
	case syntax::StmtKind::Ambiguous:
		return reachable_label(static_cast<const syntax::AmbiguousStmt&>(stmt).reading());
	case syntax::StmtKind::Case:
	case syntax::StmtKind::Default:
	case syntax::StmtKind::Labeled:
		return true;
	case syntax::StmtKind::Compound:
	{
		const uint32_t known = label_memo_.find(&stmt);
		if (known != PointerMap::kMissing)
			return known != 0;
		bool found = false;
		for (const syntax::Stmt* item : static_cast<const syntax::CompoundStmt&>(stmt).items)
		{
			if (reachable_label(*item))
			{
				found = true;
				break;
			}
		}
		label_memo_.set(&stmt, found ? 1 : 0);
		return found;
	}
	case syntax::StmtKind::If:
	{
		const syntax::IfStmt& if_stmt = static_cast<const syntax::IfStmt&>(stmt);
		return (syntax::LiveThen(if_stmt) != nullptr && reachable_label(*if_stmt.then_branch)) ||
			(syntax::LiveElse(if_stmt) != nullptr && reachable_label(*if_stmt.else_branch));
	}
	case syntax::StmtKind::While:
	case syntax::StmtKind::Switch:
		return reachable_label(*static_cast<const syntax::ConditionalLoopStmt&>(stmt).body);
	case syntax::StmtKind::Do:
		return reachable_label(*static_cast<const syntax::DoStmt&>(stmt).body);
	case syntax::StmtKind::For:
		return reachable_label(*static_cast<const syntax::ForStmt&>(stmt).body);
	case syntax::StmtKind::RangeFor:
		return reachable_label(*static_cast<const syntax::RangeForStmt&>(stmt).body);
	default:
		return false;
	}
}

void FunctionLowering::leave_scope(size_t depth, const syntax::CompoundStmt& block)
{
	if (debug_ && !terminated_ && cleanups_.size() > depth && block.items.closing_location != 0)
	{
		const DebugScope end(*this, block.items.closing_location);
		leave_scope(depth);
	}
	else
		leave_scope(depth);
}

void FunctionLowering::compound(const syntax::CompoundStmt& stmt)
{
	LexicalScope lexical(*this);
	// 6.3: the block's automatic objects are destroyed when control leaves
	// its end.
	const size_t depth = cleanups_.size();
	for (const syntax::Stmt* item : stmt.items)
		statement(*item);
	leave_scope(depth, stmt);
}

// The state of the expression enclosing a statement expression: its
// full-expression's region segment (and its throwing calls' marks), its
// conditionals' guards, a construction, an initialization being made.
// Saved and reset while the block's statements are lowered as statements,
// then restored. A region never opens inside another of the function's
// (a landing destroys every alive object and resumes past the function's
// regions), so the enclosing one closes first and opens again no earlier
// than the block's end: the positions recorded before the block (the
// calls' starts, the construction's and the segment's) move there.
struct FunctionLowering::ExpressionScope
{
	explicit ExpressionScope(FunctionLowering& lowering) : f(lowering)
	{
		f.close_region();
		f.drop_segment_floor();
		marks.swap(f.call_marks_);
		choice_root = f.choice_root_;
		guard_scopes = f.guard_scopes_;
		guarded_branches = f.guarded_branches_;
		unreachable_operands = f.unreachable_operands_;
		regions_opened = f.regions_opened_;
		reopened_choices = f.reopened_choices_;
		guard_block = f.guard_block_;
		guard_after = f.guard_after_;
		full_expression = f.full_expression_;
		region_segment = f.region_segment_;
		nothrow_segment = f.nothrow_segment_;
		segment_temporaries = f.segment_temporaries_;
		deferred_region = f.deferred_region_;
		hidden_eh = f.hidden_eh_;
		hidden_context = f.hidden_context_;
		construction = f.construction_;
		construction_depth = f.construction_depth_;
		split_block = f.split_block_;
		declaring = f.declaring_;
		declared_construction = f.declared_construction_;
		local_direct_init = f.local_direct_init_;
		allocation_copy = f.allocation_copy_;
		zero_data_only = f.zero_data_only_;
		passing_empty = f.passing_empty_;
		eager_call = f.eager_call_;
		static_local = f.static_local_;
		aggregate_extension = f.aggregate_extension_;
		scope_extension = f.scope_extension_;
		aggregate_members = f.aggregate_members_;
		member_regions = f.member_regions_;
		element_count = f.element_count_;
		counted_item = f.counted_item_;
		bit_unit = f.bit_unit_;
		bit_high = f.bit_high_;
		bit_limit = f.bit_limit_;
		hidden_base = f.hidden_base_;
		call_object_class = f.call_object_class_;
		conditional_slot = f.conditional_slot_;
		tracked = f.tracked_;
		f.tracked_.active = false;

		f.choice_root_ = nullptr;
		f.guard_scopes_ = f.guarded_branches_ = f.unreachable_operands_ = f.reopened_choices_ = 0;
		f.full_expression_ = f.region_segment_ = f.nothrow_segment_ = f.segment_temporaries_ = false;
		f.deferred_region_ = f.hidden_eh_ = f.hidden_context_ = f.construction_ = false;
		f.split_block_ = lowir::kNoId;
		f.declaring_ = f.declared_construction_ = f.local_direct_init_ = f.allocation_copy_ = false;
		f.zero_data_only_ = f.passing_empty_ = f.eager_call_ = f.static_local_ = false;
		f.aggregate_extension_ = f.scope_extension_ = nullptr;
		f.aggregate_members_ = SIZE_MAX;
		f.member_regions_ = false;
		f.element_count_ = nullptr;
		f.counted_item_ = false;
		f.bit_unit_ = f.bit_high_ = 0;
		f.bit_limit_ = UINT64_MAX;
		f.hidden_base_.active = false;
		f.call_object_class_ = nullptr;
	}

	~ExpressionScope()
	{
		// The enclosing expression's region opens here at the earliest.
		const lowir::BlockId block = f.builder_.insertion_block();
		const lowir::InstId after = f.builder_.last_instruction(block);
		marks.swap(f.call_marks_);
		for (CallMark& mark : f.call_marks_)
		{
			mark.block = block;
			mark.after = after;
		}
		f.construction_block_ = f.segment_block_ = block;
		f.construction_after_ = f.segment_after_ = after;
		f.eager_pending_ = false;

		f.choice_root_ = choice_root;
		f.guard_scopes_ = guard_scopes;
		f.guarded_branches_ = guarded_branches;
		f.unreachable_operands_ = unreachable_operands;
		f.regions_opened_ = regions_opened;
		f.reopened_choices_ = reopened_choices;
		f.guard_block_ = guard_block;
		f.guard_after_ = guard_after;
		f.full_expression_ = full_expression;
		f.region_segment_ = region_segment;
		f.nothrow_segment_ = nothrow_segment;
		f.segment_temporaries_ = segment_temporaries;
		f.deferred_region_ = deferred_region;
		f.hidden_eh_ = hidden_eh;
		f.hidden_context_ = hidden_context;
		f.construction_ = construction;
		f.construction_depth_ = construction_depth;
		f.split_block_ = split_block;
		f.declaring_ = declaring;
		f.declared_construction_ = declared_construction;
		f.local_direct_init_ = local_direct_init;
		f.allocation_copy_ = allocation_copy;
		f.zero_data_only_ = zero_data_only;
		f.passing_empty_ = passing_empty;
		f.eager_call_ = eager_call;
		f.static_local_ = static_local;
		f.aggregate_extension_ = aggregate_extension;
		f.scope_extension_ = scope_extension;
		f.aggregate_members_ = aggregate_members;
		f.member_regions_ = member_regions;
		f.element_count_ = element_count;
		f.counted_item_ = counted_item;
		f.bit_unit_ = bit_unit;
		f.bit_high_ = bit_high;
		f.bit_limit_ = bit_limit;
		f.hidden_base_ = hidden_base;
		f.call_object_class_ = call_object_class;
		f.conditional_slot_ = conditional_slot;
		f.tracked_ = tracked;
	}

	ExpressionScope(const ExpressionScope&) = delete;
	ExpressionScope& operator=(const ExpressionScope&) = delete;

	FunctionLowering& f;
	std::vector<CallMark> marks;
	const syntax::Expr* choice_root;
	uint32_t guard_scopes, guarded_branches, unreachable_operands, regions_opened, reopened_choices;
	lowir::BlockId guard_block;
	lowir::InstId guard_after;
	bool full_expression, region_segment, nothrow_segment, segment_temporaries, deferred_region, hidden_eh,
		hidden_context;
	bool construction;
	size_t construction_depth;
	lowir::BlockId split_block;
	bool declaring, declared_construction, local_direct_init, allocation_copy, zero_data_only, passing_empty;
	bool eager_call, static_local;
	const sema::Entity* aggregate_extension;
	const sema::Entity* scope_extension;
	size_t aggregate_members;
	bool member_regions;
	ElementCount* element_count;
	bool counted_item;
	uint64_t bit_unit, bit_high, bit_limit;
	HiddenBase hidden_base;
	const sema::ClassEntity* call_object_class;
	lowir::Value conditional_slot;
	TrackedLifetimes tracked;
};

RValue FunctionLowering::statement_expression(const syntax::StatementExpr& expr, lowir::Value object)
{
	LexicalScope lexical(*this);
	// The block's statements are lowered as statements, each a
	// full-expression of its own, the last expression statement's value
	// copied out before its temporaries and the block's objects are
	// destroyed. A jump out of the block (a return) leaves the rest of the
	// enclosing expression unreachable.
	ExpressionScope scope(*this);
	const syntax::CompoundStmt& block = *expr.body;
	const size_t depth = cleanups_.size();
	if (block.items.size != 0)
	{
		// Its labels (not the body's, see scan_body), a try block's too:
		// the objects alive at each are those alive here and its
		// declarations' before it; the try blocks and handlers enclosing it
		// are these and its own.
		const uint32_t contexts = scan_contexts_;
		const void* context = scan_context_;
		scan_contexts_ = static_cast<uint32_t>(eh_contexts_.size());
		scan_context_ = eh_contexts_.empty() ? nullptr : eh_contexts_.back().key;
		scan_floor_ = static_cast<uint32_t>(depth + 1);
		count_returns(block, kNoLive);
		scan_floor_ = 0;
		scan_contexts_ = contexts;
		scan_context_ = context;
	}
	const syntax::ExpressionStmt* last = sema::IsVoid(expr.facts->type) ? nullptr : syntax::StatementExprValue(expr);
	const syntax::Expr* value = last != nullptr ? last->expression : nullptr;
	const uint32_t statements = value != nullptr ? block.items.size - 1 : block.items.size;
	for (uint32_t i = 0; i < statements; ++i)
		statement(*block.items[i]);
	// The value statement's labels (a goto may name one).
	for (const syntax::Stmt* item = value != nullptr ? &syntax::ChosenReading(*block.items[statements]) : nullptr;
		item != nullptr && item->kind == syntax::StmtKind::Labeled;
		item = &syntax::ChosenReading(*static_cast<const syntax::LabeledStmt*>(item)->body))
		enter_label(goto_label(static_cast<const syntax::LabeledStmt*>(item)->label));
	RValue result;
	if (value != nullptr && !terminated_)
	{
		const size_t first = cleanups_.size();
		const bool region = begin_full_expression(value, false);
		const syntax::Expr* saved = full_root(value);
		if (object.kind != lowir::ValueKind::None)
			initialize_from(*value, value->facts->conversion, object, expr.location);
		else
			result = operand(*value, ConversionContext::Initialization);
		choice_root_ = saved;
		destroy_temporaries(first);
		end_full_expression(region);
	}
	leave_scope(depth, block);
	close_region();
	if (terminated_)
	{
		place(new_block(lowir::BlockKind::Unreachable));
		terminated_ = false;
		if (object.kind == lowir::ValueKind::None && !sema::IsVoid(expr.facts->type))
			result = zero_value(expr.facts->type);
	}
	return result;
}

void FunctionLowering::substatement(const syntax::Stmt& stmt)
{
	LexicalScope lexical(*this, stmt.kind != syntax::StmtKind::Compound);
	// 6.4p1, 6.5p2: a selection or iteration statement's substatement is a
	// block scope of its own, left each time through a loop.
	const size_t depth = cleanups_.size();
	statement(stmt);
	leave_scope(depth);
}

void FunctionLowering::enter_label(lowir::BlockId block)
{
	// Control falls through into a label's block from the code before it.
	if (!terminated_)
		jump(block);
	place(block);
}

lowir::BlockId FunctionLowering::goto_label(const Identifier* label)
{
	// One block per label of the function, made when first named.
	const uint32_t found = labels_.find(label);
	if (found != PointerMap::kMissing)
		return found;
	const lowir::BlockId block = new_block(lowir::BlockKind::Goto);
	labels_.set(label, block);
	return block;
}

// ------------------------------------------------------------ declarations

void FunctionLowering::declaration_statement(const syntax::Decl& decl)
{
	if (const sema::VariableEntity* object = ProgramLowering::anonymous_object(decl))
	{
		// A block-scope anonymous union's unnamed object (9.5p5).
		syntax::InitDeclarator unnamed = syntax::InitDeclarator();
		local_object_variable(object, unnamed);
	}
	if (decl.kind != syntax::DeclKind::Simple)
	{
		// Local classes, enumerations, typedefs and static assertions
		// declare names only.
		if (decl.kind == syntax::DeclKind::Function)
			throw LoweringError(decl.location, "a local function definition");
		return;
	}
	for (const syntax::InitDeclarator& init : static_cast<const syntax::SimpleDecl&>(decl).declarators)
	{
		// A structured binding's names: an array's or class's designate
		// parts of e (no storage, no declared variable); a tuple-like
		// type's are reference variables bound to the get<i>(e) the
		// analysis made (C++17's 11.5p3).
		syntax::InitDeclarator variable = init;
		if (init.declared != nullptr)
			variable.initializer = const_cast<syntax::Initializer*>(
				sema::VariableInitializer(init.declared->entity, init.initializer));
		local_variable(variable);
	}
}

void FunctionLowering::unreachable_declaration(const syntax::Decl& decl)
{
	if (decl.kind != syntax::DeclKind::Simple)
		return;
	unreachable_storage_ = true;
	for (const syntax::InitDeclarator& init : static_cast<const syntax::SimpleDecl&>(decl).declarators)
	{
		// A local static keeps its initializer: its constant initialization
		// happens before its block is first entered (6.7p2, 3.6.2p2); only
		// its dynamic initialization waits for a pass through it (6.7p4).
		const sema::Binding* binding = init.declared;
		syntax::InitDeclarator storage_only = init;
		if (binding == nullptr || binding->kind != sema::BindingKind::Variable ||
			!static_cast<const sema::VariableEntity*>(binding->entity)->is_static)
			storage_only.initializer = nullptr;
		else
			storage_only.initializer = const_cast<syntax::Initializer*>(
				sema::VariableInitializer(binding->entity, init.initializer));
		local_variable(storage_only);
	}
	unreachable_storage_ = false;
}

void FunctionLowering::local_variable(const syntax::InitDeclarator& init)
{
	const sema::Binding* binding = init.declared;
	if (binding == nullptr || binding->kind != sema::BindingKind::Variable)
		return;
	local_object_variable(static_cast<const sema::VariableEntity*>(binding->entity), init);
}

void FunctionLowering::local_object_variable(const sema::VariableEntity* variable, const syntax::InitDeclarator& init)
{
	// A block-scope extern declaration names a variable of the enclosing
	// namespace (3.5p6), a new one or an earlier one's entity: no frame
	// storage.
	if (variable->is_extern || variable->owner->kind == sema::ScopeKind::Namespace)
		return;
	const uint32_t location = init.declarator != nullptr ? init.declarator->location : 0;
	if (variable->is_static || variable->is_thread_local)
	{
		local_static_variable(variable, init);
		return;
	}
	const sema::Type* type = variable->type;
	Local local;
	local.reference = type->is_reference();
	local.address = lowir::Value();
	local.depth = 0;
	lowir::Type slot_type = local.reference ? lowir::Type::scalar(lowir::TypeKind::Ptr) : lowering_.storage_type(type);
	// 7.6.2: a stricter requested alignment makes the slot storage of that
	// alignment, its size rounded up to a multiple of it (as obj<> needs).
	const uint64_t requested = variable->alignment_request;
	if (requested != 0 && !local.reference && requested > lowering_.alignment_of(type, location))
	{
		unsigned log2 = 0;
		while ((uint64_t(1) << log2) < requested)
			++log2;
		const uint64_t size = lowering_.size_of(type, location);
		slot_type = lowir::Type::object((size + requested - 1) / requested * requested, log2);
	}
	// AMD64 psABI 3.1.2: an array variable of at least 16 bytes gets
	// at least 16-byte placement alignment. This is a storage fact; its
	// source type, member layout and alignof remain unchanged.
	if (lowering_.object_mode() && type->kind == sema::TypeKind::Array && slot_type.bytes >= 16 &&
		slot_type.align_log2 < 4)
		slot_type.align_log2 = 4;
	const char* spelling = variable->name != nullptr ? variable->name->spelling : nullptr;
	Place place = Place();
	if (variable == returned_local_)
	{
		// Constructed in the result's destination: no slot of its own.
		local.slot = lowir::kNoId;
		local.address = result_destination_;
		place.storage = result_destination_;
	}
	else
	{
		// (An anonymous union's object has no name: 9.5p5.)
		local.slot = spelling != nullptr
			? new_slot(lowering_.intern(spelling, static_cast<uint32_t>(std::strlen(spelling))), slot_type)
			: generated_slot(lowir::SlotKind::AnonymousUnionStorage, slot_type);
		place.storage = lowir::Value::slot(local.slot);
	}
	if (debug_)
		lowering_.debug_local(debug_function_, local.slot, variable, location, debug_scope_);
	local_index_.set(variable, static_cast<uint32_t>(locals_.size()));
	locals_.push_back(local);
	qualify(place, type);
	// A class object is constructed (and later destroyed) even without an
	// initializer; unreachable declarations only get their storage. The
	// objects its declaration made alive end at the depth it leaves.
	const uint32_t index = static_cast<uint32_t>(locals_.size() - 1);
	if (!local.reference && object_class(type) != nullptr)
	{
		if (!unreachable_storage_)
			local_object(variable, place, init.initializer, location);
		locals_[index].depth = cleanups_.size();
		return;
	}
	if (init.initializer == nullptr)
		return;
	const syntax::Initializer* written = init.initializer;
	const size_t first = cleanups_.size();
	const syntax::Expr* single = written->kind == syntax::InitializerKind::Parenthesized
		? (written->arguments.size == 1 ? written->arguments[0] : nullptr)
		: written->expression;
	// A constexpr array initialized by a braced list or a string literal is
	// copied from its constant image (the course's form); so is a mutable
	// one-dimensional array of scalars whose whole initializer is known
	// (each array keeps its own storage, the image is shared); volatile
	// elements keep their volatile stores (lowir.md: copyobj has no
	// volatile form).
	const syntax::Expr* inner = single != nullptr ? WithoutParentheses(single) : nullptr;
	const sema::Type* element = type;
	while (element->kind == sema::TypeKind::Array)
		element = element->element;
	if ((variable->is_constexpr || ScalarImageArray(type)) && type->kind == sema::TypeKind::Array &&
		(element->cv & (sema::kVolatile | sema::kAtomic)) == 0 && inner != nullptr &&
		(inner->kind == syntax::ExprKind::BracedInit ||
			(inner->kind == syntax::ExprKind::Literal && inner->facts->type->kind == sema::TypeKind::Array)) &&
		!unreachable_storage_)
	{
		const lowir::SymbolId image = lowering_.constexpr_image(variable, written);
		if (image != lowir::kNoId)
		{
			imaged_discard_slots(*inner, 0);
			copy_object(type, lowir::Value::symbol(image), address_of(place), location);
			return;
		}
	}
	// A constexpr or const integral scalar's value is its evaluation's
	// (7.1.5p9, 5.19p2), stored rather than computed again when its
	// initializer names no member (the course's form: one calling member
	// functions computes it).
	const bool storable = !local.reference && !unreachable_storage_ && type->kind != sema::TypeKind::Array &&
		single != nullptr && !NamesMember(*single, 0);
	const sema::ConstValue* evaluated = storable &&
			(variable->is_constexpr || variable->constant == sema::ConstantState::Value)
		? lowering_.evaluated_value(variable)
		: nullptr;
	// So is a const scalar's whose initializer evaluates bfloat16
	// arithmetic, when that is a constant expression: Clang initializes it
	// from the constant evaluation, which rounds each operation (its run-time
	// evaluation rounds an expression once, bfloat_arithmetic).
	if (evaluated == nullptr && storable && (type->cv & (sema::kConst | sema::kVolatile)) == sema::kConst &&
		object_class(type) == nullptr && BFloatArithmeticIn(*single, 0))
		evaluated = lowering_.initialization_value(variable, written);
	RValue constant;
	if (evaluated != nullptr && constant_rvalue(*evaluated, type, constant, location))
	{
		store(slot_type, constant, place);
		return;
	}
	const bool region = begin_full_expression(single);
	const syntax::Expr* saved = full_root(single);
	local_initializer(type, local, place, *written, location, variable);
	choice_root_ = saved;
	destroy_temporaries(first);
	end_full_expression(region);
	locals_[index].depth = cleanups_.size();
}

void FunctionLowering::local_initializer(const sema::Type* type, const Local& local, const Place& place,
	const syntax::Initializer& written, uint32_t location, const sema::Entity* variable)
{
	// A scalar, reference or aggregate local from its initializer.
	const lowir::Type slot_type = local.reference ? lowir::Type::scalar(lowir::TypeKind::Ptr)
												  : lowering_.storage_type(type);
	if (local.reference)
	{
		const syntax::Initializer* initializer = &written;
		const syntax::Expr* single = initializer->kind == syntax::InitializerKind::Parenthesized
			? initializer->arguments[0]
			: initializer->expression;
		single = reference_initializer(single);
		RValue address;
		const bool saved = passing_empty_;
		const sema::Conversion* binding = WithoutParentheses(single)->facts->conversion;
		passing_empty_ = binding == nullptr || binding->second != sema::ValueConversion::DerivedToBase;
		// 12.2p5: the temporary it is bound to lives as long as it.
		address.value = bind_extended(*single, type, variable);
		passing_empty_ = saved;
		address.type = slot_type;
		store(slot_type, address, place);
		return;
	}
	initialize_storage(type, place, &written, location);
}

void FunctionLowering::initialize_storage(
	const sema::Type* type, const Place& place, const syntax::Initializer* initializer, uint32_t location)
{
	// 8.5: an object from its initializer: a scalar from one expression
	// (or `{}`, zero), an aggregate element by element.
	const syntax::Expr* expr = initializer->kind == syntax::InitializerKind::Parenthesized
		? (initializer->arguments.size == 1 ? initializer->arguments[0] : nullptr)
		: initializer->expression;
	if (expr == nullptr)
		throw LoweringError(location, "an initializer outside the procedural lowering slice");
	if (type->kind == sema::TypeKind::Array && type->element->kind == sema::TypeKind::Array)
	{
		// A multidimensional array's elements, each address derived from
		// its row's.
		const syntax::Expr* self = expr;
		syntax::BracedInitList wrapper = syntax::BracedInitList();
		wrapper.elements.data = const_cast<syntax::Expr**>(&self);
		wrapper.elements.size = 1;
		uint32_t next = 0;
		structured_item(type, address_of(place), &wrapper, next, location);
		return;
	}
	if (type->kind == sema::TypeKind::Array || type->unqualified->kind == sema::TypeKind::Class)
	{
		initialize_object(type, address_of(place), *expr, location);
		return;
	}
	const lowir::Type storage = lowering_.storage_type(type);
	if (expr->kind == syntax::ExprKind::BracedInit)
	{
		const syntax::BracedInitList& list = static_cast<const syntax::BracedInitList&>(*expr);
		if (list.elements.empty())
		{
			store(storage, zero_value(type), place);
			return;
		}
		// A GNU vector's elements are its lanes (a complex value's two its
		// parts).
		if (sema::LaneList(type, list))
		{
			store(storage, vector_list(type, list), place);
			return;
		}
		expr = list.elements[0];
	}
	RValue value = fit(operand(*expr, ConversionContext::Initialization), type);
	store(storage, value, place);
}

void FunctionLowering::initialize_object(
	const sema::Type* type, lowir::Value base, const syntax::Expr& init, uint32_t location)
{
	// Each scalar element stored at its byte offset from the object's
	// address, elements without an initializer zeroed (8.5.1p7); an array
	// of characters from a string literal takes its code units (8.5.2).
	std::vector<InitElement> elements;
	flatten_initializer(type, &init, 0, location, elements);
	for (const InitElement& element : elements)
	{
		Place place = Place();
		qualify(place, element.type);
		if (element.offset == 0)
			place.storage = base;
		else
			place.storage = index(lowir::Type::scalar(lowir::TypeKind::I8), ir_model::IPK_NONE, base,
				integer(lowir::Type::scalar(lowir::TypeKind::I64), element.offset))
								.value;
		const lowir::Type storage = lowering_.storage_type(element.type);
		RValue value;
		if (element.expr != nullptr && element.type->is_reference())
		{
			// 8.5.3: a reference member binds to its initializer.
			value.value = bind_reference(*element.expr, element.type, ConversionContext::Initialization);
			value.type = storage;
		}
		else if (element.expr != nullptr && element.expr->kind == syntax::ExprKind::BracedInit)
			value = vector_list(element.type, static_cast<const syntax::BracedInitList&>(*element.expr));
		else if (element.expr != nullptr)
			value = fit(operand(*element.expr, ConversionContext::Initialization), element.type);
		else if (element.has_unit)
			value = integer(storage, element.unit);
		else
			value = zero_value(element.type);
		store(storage, value, place);
	}
}

void FunctionLowering::flatten_initializer(
	const sema::Type* type, const syntax::Expr* init, uint64_t offset, uint32_t location, std::vector<InitElement>& out)
{
	if (type->kind == sema::TypeKind::Array)
	{
		const sema::Type* element = type->element;
		const uint64_t size = lowering_.size_of(element, location);
		const syntax::Expr* literal = init != nullptr ? WithoutParentheses(init) : nullptr;
		if (literal != nullptr && literal->kind == syntax::ExprKind::BracedInit)
		{
			const syntax::BracedInitList& list = static_cast<const syntax::BracedInitList&>(*literal);
			if (list.elements.size == 1 && WithoutParentheses(list.elements[0])->kind == syntax::ExprKind::Literal &&
				list.elements[0]->facts->type->kind == sema::TypeKind::Array)
				literal = WithoutParentheses(list.elements[0]);
			else
			{
				for (uint64_t i = 0; i < type->bound; ++i)
					flatten_initializer(element,
						i < list.elements.size ? list.elements[static_cast<uint32_t>(i)] : nullptr, offset + i * size,
						location, out);
				return;
			}
		}
		if (literal != nullptr && literal->kind == syntax::ExprKind::Literal)
		{
			// 8.5.2: the literal's code units, then zeros.
			sema::StringLiteralFacts facts;
			std::vector<uint64_t> units;
			if (!sema::StringLiteralUnits(static_cast<const syntax::LiteralExpr*>(literal)->spelling, facts, units))
				throw LoweringError(location, "an invalid string literal");
			for (uint64_t i = 0; i < type->bound; ++i)
				out.push_back(
					InitElement{ nullptr, element, offset + i * size, true, i < units.size() ? units[i] : 0 });
			return;
		}
		if (literal != nullptr)
			throw LoweringError(location, "an array initialized from an expression");
		for (uint64_t i = 0; i < type->bound; ++i)
			flatten_initializer(element, nullptr, offset + i * size, location, out);
		return;
	}
	if (type->unqualified->kind == sema::TypeKind::Class)
	{
		const sema::ClassEntity* entity = static_cast<const sema::ClassEntity*>(type->unqualified->entity);
		const syntax::BracedInitList* list = init != nullptr && init->kind == syntax::ExprKind::BracedInit
			? static_cast<const syntax::BracedInitList*>(init)
			: nullptr;
		if (init != nullptr && list == nullptr)
			throw LoweringError(location, "copying a class object here is not supported");
		uint32_t next = 0;
		for (const sema::DataMember* member = entity->members; member != nullptr; member = member->next)
		{
			const sema::VariableEntity* variable = static_cast<const sema::VariableEntity*>(member->variable);
			if (member->bit_field)
				throw LoweringError(location, "a bit-field in a flattened initializer is not supported");
			const syntax::Expr* item = list != nullptr && next < list->elements.size ? list->elements[next++] : nullptr;
			// 9.3.2p3-like: a volatile object's members are volatile.
			const sema::Type* member_type = lowering_.graph().types.qualified(
				variable->type, static_cast<uint8_t>(type->cv & sema::kVolatile));
			flatten_initializer(member_type, item, offset + lowering_.member_offset(variable, location), location, out);
		}
		return;
	}
	// A GNU vector's list stays whole: its elements are its lanes (a
	// complex value's two its parts).
	if (init != nullptr && init->kind == syntax::ExprKind::BracedInit &&
		!sema::LaneList(type, static_cast<const syntax::BracedInitList&>(*init)))
	{
		const syntax::BracedInitList& list = static_cast<const syntax::BracedInitList&>(*init);
		init = list.elements.empty() ? nullptr : list.elements[0];
	}
	out.push_back(InitElement{ init, type, offset, false, 0 });
}

// ----------------------------------------------------------- selections

void FunctionLowering::if_statement(const syntax::IfStmt& stmt)
{
	LexicalScope lexical(*this);
	// 6.4.1: the condition branches to the then block, else to the else
	// block (present even without an else, jumping to the end); the end
	// block exists only if control reaches it. An `if constexpr` is its
	// selected branch alone.
	if (stmt.constexpr_value != 0)
	{
		const size_t depth = cleanups_.size();
		const syntax::Stmt* selected = stmt.constexpr_value == 2 ? stmt.then_branch : stmt.else_branch;
		if (selected != nullptr)
			substatement(*selected);
		leave_scope(depth);
		return;
	}
	const lowir::BlockId then_block = new_block(lowir::BlockKind::IfThen);
	const lowir::BlockId else_block = new_block(lowir::BlockKind::IfElse);
	lowir::BlockId end_block = lowir::kNoId;
	// 6.4p3: a condition's declared object lives through the whole
	// statement.
	const size_t depth = cleanups_.size();
	if (stmt.condition.declaration != nullptr)
	{
		bool region = false;
		branch(condition_variable(*stmt.condition.declaration, true, region), then_block, else_block);
		end_full_expression(region);
	}
	else
		condition_branch(*stmt.condition.expression, then_block, else_block, true);
	place(then_block);
	substatement(*stmt.then_branch);
	if (!terminated_)
	{
		end_block = new_block(lowir::BlockKind::IfEnd);
		jump(end_block);
	}
	place(else_block);
	if (stmt.else_branch != nullptr)
		substatement(*stmt.else_branch);
	if (!terminated_)
	{
		if (end_block == lowir::kNoId)
			end_block = new_block(lowir::BlockKind::IfEnd);
		jump(end_block);
	}
	if (end_block != lowir::kNoId)
		place(end_block);
	leave_scope(depth);
}

void FunctionLowering::condition_branch(
	const syntax::Expr& expr, lowir::BlockId if_true, lowir::BlockId if_false, bool fold_literals)
{
	// A condition's full-expression and its branch. Its temporaries end on
	// each edge (12.2p3): the condition is then computed as a value and
	// each target reached through a block destroying them.
	const size_t first = cleanups_.size();
	hidden_context_ = true;
	const bool region = begin_full_expression(&expr);
	const syntax::Expr* saved = full_root(&expr);
	const syntax::Expr* top = WithoutParentheses(&expr);
	const bool logical_top = top->kind == syntax::ExprKind::Binary &&
		(static_cast<const syntax::BinaryExpr*>(top)->op == TokenKind::OP_LAND ||
			static_cast<const syntax::BinaryExpr*>(top)->op == TokenKind::OP_LOR) &&
		(top->facts == nullptr || (top->facts->flags & sema::kOperatorCall) == 0);
	if (logical_top && !chain_empty() && may_throw(&expr) && !creates_temporaries(&expr, nullptr, true))
	{
		// A logical condition that may throw while objects are alive is
		// computed as a value, then branched on (the course stages its
		// full-expression so).
		const RValue value = truth(expr);
		choice_root_ = saved;
		end_full_expression(region);
		branch(value, if_true, if_false);
		return;
	}
	if (!creates_temporaries(&expr, nullptr, true))
	{
		branch_condition(expr, if_true, if_false, fold_literals);
		choice_root_ = saved;
		end_full_expression(region);
		return;
	}
	const RValue value = truth(expr);
	choice_root_ = saved;
	// A logical operator's value: with none of its temporaries left (its
	// right operand's ended in its branch), or with a constant left
	// operand, the branch follows the value (the course's form).
	const syntax::Expr* root = WithoutParentheses(&expr);
	const syntax::BinaryExpr* logical = root->kind == syntax::ExprKind::Binary &&
			(static_cast<const syntax::BinaryExpr*>(root)->op == TokenKind::OP_LAND ||
				static_cast<const syntax::BinaryExpr*>(root)->op == TokenKind::OP_LOR)
		? static_cast<const syntax::BinaryExpr*>(root)
		: nullptr;
	bool constant = false;
	if (logical != nullptr && (cleanups_.size() == first || literal_truth(*logical->left, constant)))
	{
		destroy_temporaries(first);
		end_full_expression(region);
		branch(value, if_true, if_false);
		return;
	}
	const lowir::BlockId true_cleanup = new_block(lowir::BlockKind::CondTrueCleanup);
	const lowir::BlockId false_cleanup = new_block(lowir::BlockKind::CondFalseCleanup);
	branch(value, true_cleanup, false_cleanup);
	end_full_expression(region);
	place(true_cleanup);
	destroy_temporaries(first, false);
	jump(if_true);
	place(false_cleanup);
	destroy_temporaries(first);
	jump(if_false);
}

void FunctionLowering::switch_statement(const syntax::ConditionalLoopStmt& stmt)
{
	LexicalScope lexical(*this);
	// 6.4.2: the promoted condition goes to a dispatch block whose switch
	// names each case label's block in source order (default: the default
	// label's, else the end); break leaves to the end block.
	RValue selector;
	const size_t depth = cleanups_.size();
	if (stmt.condition.declaration != nullptr)
	{
		bool region = false;
		selector = condition_variable(*stmt.condition.declaration, false, region);
		end_full_expression(region);
	}
	else
	{
		// 6.4p2, 1.9p10: the condition is a full-expression; its temporaries
		// end before the body (12.2p3).
		const size_t first = cleanups_.size();
		const bool region = begin_full_expression(stmt.condition.expression);
		selector = operand(*stmt.condition.expression, ConversionContext::Operand);
		destroy_temporaries(first);
		end_full_expression(region);
	}
	const lowir::BlockId dispatch = new_block(lowir::BlockKind::SwitchDispatch);
	const lowir::BlockId end = new_block(lowir::BlockKind::SwitchEnd);
	const size_t saved_first = switch_first_;
	switch_first_ = switch_labels_.size();
	scan_switch_labels(*stmt.body, switch_labels_);
	lowir::BlockId default_block = end;
	scratch_.clear();
	for (size_t i = switch_first_; i < switch_labels_.size(); ++i)
	{
		const syntax::Stmt* label = switch_labels_[i].statement;
		if (label->kind == syntax::StmtKind::Default)
		{
			default_block = switch_labels_[i].block;
			continue;
		}
		// A 128-bit condition's value extends by its type's signedness
		// (sema/integral_arithmetic.h).
		const syntax::CaseStmt* label_case = static_cast<const syntax::CaseStmt*>(label);
		const sema::ExprFacts* label_facts = WithoutParentheses(label_case->value)->facts;
		const sema::Type* label_type = label_facts->conversion != nullptr && label_facts->conversion->result != nullptr
			? label_facts->conversion->result
			: label_facts->type;
		scratch_.push_back(selector.type.kind() == lowir::TypeKind::I128
				? integer(label_type, label_case->converted).value
				: integer(selector.type, label_case->converted).value);
		scratch_.push_back(lowir::Value::block(switch_labels_[i].block));
	}
	jump(dispatch);
	place(dispatch);
	lowir::Instruction inst = lowir::make_instruction(
		lowir::Opcode::Switch, lowir::Type::scalar(lowir::TypeKind::Void));
	inst.ops[0] = selector.value;
	inst.ops[1] = lowir::Value::block(default_block);
	inst.extra_first = builder_.add_operands(scratch_.data(), static_cast<uint32_t>(scratch_.size()));
	inst.extra_count = static_cast<uint32_t>(scratch_.size());
	terminate(inst);
	jumps_.push_back(JumpTargets{ end, lowir::kNoId, cleanups_.size(), eh_contexts_.size() });
	substatement(*stmt.body);
	jumps_.pop_back();
	if (!terminated_)
		jump(end);
	place(end);
	switch_labels_.resize(switch_first_);
	switch_first_ = saved_first;
	leave_scope(depth);
}

void FunctionLowering::scan_switch_labels(const syntax::Stmt& stmt, std::vector<SwitchLabels>& out)
{
	// The case and default labels of this switch: anywhere in its body but
	// inside a nested switch statement.
	switch (stmt.kind)
	{
	case syntax::StmtKind::Case:
		out.push_back(SwitchLabels{ &stmt, new_block(lowir::BlockKind::SwitchCase) });
		switch_blocks_.set(&stmt, out.back().block);
		return scan_switch_labels(*static_cast<const syntax::CaseStmt&>(stmt).body, out);
	case syntax::StmtKind::Default:
		out.push_back(SwitchLabels{ &stmt, new_block(lowir::BlockKind::SwitchDefault) });
		switch_blocks_.set(&stmt, out.back().block);
		return scan_switch_labels(*static_cast<const syntax::LabeledStmt&>(stmt).body, out);
	case syntax::StmtKind::Labeled:
		return scan_switch_labels(*static_cast<const syntax::LabeledStmt&>(stmt).body, out);
	case syntax::StmtKind::Ambiguous:
		return scan_switch_labels(static_cast<const syntax::AmbiguousStmt&>(stmt).reading(), out);
	case syntax::StmtKind::Compound:
		for (const syntax::Stmt* item : static_cast<const syntax::CompoundStmt&>(stmt).items)
			scan_switch_labels(*item, out);
		return;
	case syntax::StmtKind::If:
	{
		const syntax::IfStmt& if_stmt = static_cast<const syntax::IfStmt&>(stmt);
		if (syntax::LiveThen(if_stmt) != nullptr)
			scan_switch_labels(*if_stmt.then_branch, out);
		if (syntax::LiveElse(if_stmt) != nullptr)
			scan_switch_labels(*if_stmt.else_branch, out);
		return;
	}
	case syntax::StmtKind::While:
		return scan_switch_labels(*static_cast<const syntax::ConditionalLoopStmt&>(stmt).body, out);
	case syntax::StmtKind::Do:
		return scan_switch_labels(*static_cast<const syntax::DoStmt&>(stmt).body, out);
	case syntax::StmtKind::For:
		return scan_switch_labels(*static_cast<const syntax::ForStmt&>(stmt).body, out);
	case syntax::StmtKind::RangeFor:
		return scan_switch_labels(*static_cast<const syntax::RangeForStmt&>(stmt).body, out);
	default:
		return;
	}
}

lowir::BlockId FunctionLowering::switch_label(const syntax::Stmt& stmt)
{
	const uint32_t block = switch_blocks_.find(&stmt);
	if (block == PointerMap::kMissing)
		throw LoweringError(stmt.location, "a case label outside a switch statement");
	return block;
}

// ------------------------------------------------------------ iterations

void FunctionLowering::while_statement(const syntax::ConditionalLoopStmt& stmt)
{
	LexicalScope lexical(*this);
	// 6.5.1: condition block, body, end; continue re-tests the condition.
	const lowir::BlockId condition = new_block(lowir::BlockKind::WhileCond);
	const lowir::BlockId body = new_block(lowir::BlockKind::WhileBody);
	const lowir::BlockId end = new_block(lowir::BlockKind::WhileEnd);
	jump(condition);
	place(condition);
	// 6.5.1p2: a condition's declared object is created and destroyed with
	// each iteration: after the body (break and continue leave its scope
	// too) and on the loop's exit.
	const size_t depth = cleanups_.size();
	lowir::BlockId exit = end;
	if (stmt.condition.declaration != nullptr)
	{
		bool region = false;
		const RValue value = condition_variable(*stmt.condition.declaration, true, region);
		if (cleanups_.size() > depth)
			exit = new_block(lowir::BlockKind::CondFalseCleanup);
		branch(value, body, exit);
		end_full_expression(region);
	}
	else
		condition_branch(*stmt.condition.expression, body, end, false);
	place(body);
	jumps_.push_back(JumpTargets{ end, condition, depth, eh_contexts_.size() });
	substatement(*stmt.body);
	jumps_.pop_back();
	if (!terminated_)
	{
		destroy_cleanups(depth);
		jump(condition);
	}
	if (exit != end)
	{
		place(exit);
		destroy_cleanups(depth);
		jump(end);
	}
	cleanups_.resize(depth);
	place(end);
}

void FunctionLowering::do_statement(const syntax::DoStmt& stmt)
{
	// 6.5.2: body, then the condition block branching back to the body.
	const lowir::BlockId body = new_block(lowir::BlockKind::DoBody);
	const lowir::BlockId condition = new_block(lowir::BlockKind::DoCond);
	const lowir::BlockId end = new_block(lowir::BlockKind::DoEnd);
	jump(body);
	place(body);
	jumps_.push_back(JumpTargets{ end, condition, cleanups_.size(), eh_contexts_.size() });
	substatement(*stmt.body);
	jumps_.pop_back();
	if (!terminated_)
		jump(condition);
	place(condition);
	condition_branch(*stmt.condition, body, end, false);
	place(end);
}

void FunctionLowering::for_statement(const syntax::ForStmt& stmt)
{
	LexicalScope lexical(*this);
	// 6.5.3: init; condition block (jumping to the body when there is no
	// condition), body, iteration block; continue goes to the iteration.
	// The init-statement's objects end with the loop.
	const size_t depth = cleanups_.size();
	if (stmt.init_declaration != nullptr)
		declaration_statement(*stmt.init_declaration);
	else if (stmt.init_expression != nullptr)
	{
		const size_t first = cleanups_.size();
		const bool region = begin_full_expression(stmt.init_expression, true);
		discard(*stmt.init_expression);
		destroy_temporaries(first);
		end_full_expression(region);
	}
	const lowir::BlockId condition = new_block(lowir::BlockKind::ForCond);
	const lowir::BlockId body = new_block(lowir::BlockKind::ForBody);
	const lowir::BlockId iteration = new_block(lowir::BlockKind::ForIter);
	const lowir::BlockId end = new_block(lowir::BlockKind::ForEnd);
	jump(condition);
	place(condition);
	// 6.5.3p1, 6.5.1p2: a condition's declared object is created and
	// destroyed with each iteration, after the iteration expression (which
	// continue reaches in its scope) and on the loop's exit (which break
	// reaches through its destruction).
	const size_t scope = cleanups_.size();
	lowir::BlockId exit = end;
	if (stmt.condition == nullptr)
		jump(body);
	else if (stmt.condition->declaration != nullptr)
	{
		bool region = false;
		const RValue value = condition_variable(*stmt.condition->declaration, true, region);
		if (cleanups_.size() > scope)
			exit = new_block(lowir::BlockKind::CondFalseCleanup);
		branch(value, body, exit);
		end_full_expression(region);
	}
	else
		condition_branch(*stmt.condition->expression, body, end, false);
	place(body);
	jumps_.push_back(JumpTargets{ exit, iteration, cleanups_.size(), eh_contexts_.size() });
	substatement(*stmt.body);
	jumps_.pop_back();
	if (!terminated_)
		jump(iteration);
	place(iteration);
	if (stmt.iteration != nullptr)
	{
		const size_t first = cleanups_.size();
		const bool region = begin_full_expression(stmt.iteration, true);
		discard(*stmt.iteration);
		destroy_temporaries(first);
		end_full_expression(region);
	}
	destroy_cleanups(scope);
	jump(condition);
	if (exit != end)
	{
		place(exit);
		destroy_cleanups(scope);
		jump(end);
	}
	cleanups_.resize(scope);
	place(end);
	leave_scope(depth);
}

void FunctionLowering::range_for_statement(const syntax::RangeForStmt& stmt)
{
	LexicalScope lexical(*this);
	// 6.5.4p1: the equivalent's variables, then its for loop: condition,
	// body (the for-range-declaration, destroyed with each iteration, then
	// the statement), increment; its variables end with the statement.
	const sema::RangeForFacts& facts = *stmt.facts;
	const size_t depth = cleanups_.size();
	if (facts.range != nullptr)
		range_variable(*facts.range);
	local_variable(*facts.begin);
	if (facts.end != nullptr)
		local_variable(*facts.end);
	const lowir::BlockId condition = new_block(lowir::BlockKind::ForCond);
	const lowir::BlockId body = new_block(lowir::BlockKind::ForBody);
	const lowir::BlockId iteration = new_block(lowir::BlockKind::ForIter);
	const lowir::BlockId end = new_block(lowir::BlockKind::ForEnd);
	jump(condition);
	place(condition);
	{
		const size_t first = cleanups_.size();
		const bool region = begin_full_expression(facts.condition, true);
		condition_branch(*facts.condition, body, end, false);
		destroy_temporaries(first);
		end_full_expression(region);
	}
	place(body);
	const size_t scope = cleanups_.size();
	jumps_.push_back(JumpTargets{ end, iteration, scope, eh_contexts_.size() });
	local_variable(facts.declaration);
	substatement(*stmt.body);
	leave_scope(scope);
	jumps_.pop_back();
	if (!terminated_)
		jump(iteration);
	place(iteration);
	const size_t first = cleanups_.size();
	const bool region = begin_full_expression(facts.increment, true);
	discard(*facts.increment);
	destroy_temporaries(first);
	end_full_expression(region);
	jump(condition);
	place(end);
	leave_scope(depth);
}

void FunctionLowering::range_variable(const syntax::InitDeclarator& range)
{
	// `auto&& __range = range-init;`: a class prvalue's temporary, which
	// lives as long as __range (12.2p5), is __range's own object (the
	// course's form); another range (an array prvalue's temporary among
	// them) is bound as any reference.
	const sema::VariableEntity* variable = static_cast<const sema::VariableEntity*>(range.declared->entity);
	const syntax::Expr* source = WithoutParentheses(range.initializer->expression);
	const sema::Type* type = variable->type;
	if (!type->is_reference() || source->facts->category != sema::ValueCategory::PRValue ||
		sema::ClassOf(source->facts->type) == nullptr)
	{
		local_variable(range);
		return;
	}
	type = type->element;
	Local local;
	local.reference = false;
	local.address = lowir::Value();
	local.depth = 0;
	const char* spelling = variable->name->spelling;
	local.slot = new_slot(
		lowering_.intern(spelling, static_cast<uint32_t>(std::strlen(spelling))), lowering_.storage_type(type));
	local_index_.set(variable, static_cast<uint32_t>(locals_.size()));
	locals_.push_back(local);
	const uint32_t index = static_cast<uint32_t>(locals_.size() - 1);
	Place place = Place();
	place.storage = lowir::Value::slot(local.slot);
	local_object(variable, place, range.initializer, source->location, type);
	locals_[index].depth = cleanups_.size();
}

RValue FunctionLowering::condition_variable(const syntax::ConditionDeclaration& declaration, bool to_bool, bool& region)
{
	// 6.4p2-p4: the declared variable, initialized, then its value converted
	// to bool (or promoted, for a switch).
	syntax::InitDeclarator init;
	init.declarator = declaration.declarator;
	init.initializer = declaration.initializer;
	init.declared = declaration.declared;
	local_variable(init);
	const sema::VariableEntity* variable = static_cast<const sema::VariableEntity*>(declaration.declared->entity);
	const sema::Type* type = variable->type->is_reference() ? variable->type->element : variable->type;
	const sema::Conversion* conversion = declaration.conversion;
	if (conversion_function(conversion))
	{
		// A class object's conversion function (6.4p2-p4), then its result
		// converted (a truth value stays as the function made it).
		// A call that may throw runs in a region of its own whose handler
		// destroys the declared object and unwinds (the course's form; in a
		// try block or handler, the region's landing in its contexts).
		const sema::FunctionEntity* function = conversion->constructor;
		const bool throws = call_may_throw(function) && !cleanups_.empty() && !full_expression_;
		const bool contexts = !eh_contexts_.empty();
		lowir::BlockId dispatch = lowir::kNoId;
		if (throws)
		{
			dispatch = contexts ? region_landing(cleanups_.size()) : new_block(lowir::BlockKind::CallUnwindDispatch);
			emit_marker(lowir::Opcode::EhTry, dispatch);
			full_expression_ = true;
		}
		const Place object = variable_place(variable, type, declaration.declarator->location);
		const size_t base = arguments_.size();
		const lowir::Value converted = to_base(address_of(object), sema::ClassOf(type),
			sema::ClassOf(function->owner->entity->type), declaration.declarator->location);
		arguments_.push_back(converted);
		lowir::SignatureId signature = lowir::kNoId;
		const lowir::Value callee = member_callee(function, converted, function->is_virtual, signature);
		const sema::Type* returned = function->type->canonical->element;
		RValue value;
		value.type = result_type(function);
		value.value = emit_call(value.type, callee, base, signature, false);
		(void)returned;
		if (throws)
		{
			full_expression_ = false;
			emit_marker(lowir::Opcode::EhEnd, lowir::kNoId);
			const lowir::BlockId next = new_block(lowir::BlockKind::CallUnwindEnd);
			jump(next);
			if (!contexts)
			{
				place(dispatch);
				destroy_local(cleanups_.back(), lowir::kNoId, true);
				if (cleanups_.size() > 1)
					jump(unwind_entry(cleanups_.size() - 1));
				else
					emit_resume();
			}
			place_chain_blocks();
			place(next);
		}
		(void)region;
		const sema::Conversion* after = conversion->after;
		if (after == nullptr || after->result == nullptr || sema::ClassOf(after->result) != nullptr)
			return value;
		if (to_bool && sema::IsVoid(after->result) == false && after->second == sema::ValueConversion::None)
			return value;
		return convert(value, returned, after->result, ConversionContext::Operand);
	}
	Place place = variable_place(variable, type, declaration.declarator->location);
	RValue value;
	if (type->kind == sema::TypeKind::Array)
	{
		// 4.2p1: an array (a reference's referent) converted to a pointer to
		// its first element.
		value.value = address_of(place);
		value.type = lowir::Type::scalar(lowir::TypeKind::Ptr);
	}
	else
		value = load(lowering_.storage_type(type), place);
	if (to_bool)
	{
		if (sema::IsFloating(type))
			return compare(lowir::ComparePredicate::Ne, value.type, value,
				floating(value.type, lowir::float_literal(value.type.kind(), 0.0L)));
		return value;
	}
	if (conversion == nullptr)
		return value;
	return convert(value, type, conversion->result, ConversionContext::Operand);
}

// --------------------------------------------------------------- returns

void FunctionLowering::returned_local(const syntax::Stmt& stmt, const sema::Entity*& local, bool& single)
{
	// The block-scope variable of the result's class that every return
	// statement names alone (its parenthesized id-expression), declared in
	// the function's own body: an enclosing function's a lambda captured
	// is no candidate (12.8p31, CWG 1579).
	check_stack(stmt.location);
	switch (stmt.kind)
	{
	case syntax::StmtKind::Compound:
		for (const syntax::Stmt* item : static_cast<const syntax::CompoundStmt&>(stmt).items)
			returned_local(*item, local, single);
		return;
	case syntax::StmtKind::Ambiguous:
		return returned_local(static_cast<const syntax::AmbiguousStmt&>(stmt).reading(), local, single);
	case syntax::StmtKind::Return:
	{
		const syntax::Expr* value = static_cast<const syntax::ValueStmt&>(stmt).value;
		const syntax::Expr* e = value != nullptr ? WithoutParentheses(value) : nullptr;
		const sema::Entity* named = e != nullptr && e->kind == syntax::ExprKind::Id && e->facts != nullptr
			? e->facts->entity
			: nullptr;
		const sema::VariableEntity* variable = named != nullptr && named->kind == sema::EntityKind::Variable
			? static_cast<const sema::VariableEntity*>(named)
			: nullptr;
		// A structured binding names no object of its own (C++17's 11.5).
		const bool candidate = variable != nullptr && variable->owner != nullptr && !variable->structured_binding &&
			variable->owner->kind == sema::ScopeKind::Block && !variable->is_static && !variable->is_extern &&
			!variable->is_thread_local && !variable->type->is_reference() &&
			(variable->type->cv & sema::kVolatile) == 0 &&
			variable->type->unqualified == function_->type->canonical->element->unqualified;
		bool own = false;
		for (const sema::Scope* scope = candidate ? variable->owner : nullptr; scope != nullptr && !own;
			scope = scope->parent)
			own = scope == function_->scope;
		if (!own || (local != nullptr && local != variable))
			single = false;
		local = variable;
		return;
	}
	case syntax::StmtKind::If:
	{
		const syntax::IfStmt& branch = static_cast<const syntax::IfStmt&>(stmt);
		if (syntax::LiveThen(branch) != nullptr)
			returned_local(*branch.then_branch, local, single);
		if (syntax::LiveElse(branch) != nullptr)
			returned_local(*branch.else_branch, local, single);
		return;
	}
	case syntax::StmtKind::While:
	case syntax::StmtKind::Switch:
		return returned_local(*static_cast<const syntax::ConditionalLoopStmt&>(stmt).body, local, single);
	case syntax::StmtKind::Do:
		return returned_local(*static_cast<const syntax::DoStmt&>(stmt).body, local, single);
	case syntax::StmtKind::For:
		return returned_local(*static_cast<const syntax::ForStmt&>(stmt).body, local, single);
	case syntax::StmtKind::RangeFor:
		return returned_local(*static_cast<const syntax::RangeForStmt&>(stmt).body, local, single);
	case syntax::StmtKind::Case:
		return returned_local(*static_cast<const syntax::CaseStmt&>(stmt).body, local, single);
	case syntax::StmtKind::Default:
	case syntax::StmtKind::Labeled:
		return returned_local(*static_cast<const syntax::LabeledStmt&>(stmt).body, local, single);
	default:
		return;
	}
}

void FunctionLowering::class_return(const syntax::ValueStmt& stmt, const sema::Type* result)
{
	// 6.6.3p2: the operand copy-initializes the result: in the caller's
	// destination (the local constructed there already is the result), or
	// in the function's result slot (one for every return, the course's
	// form) returned as its bytes. The automatic objects are destroyed
	// after it (6.6p2), so a copy or move into the result that throws
	// (`return x;` of a local or by-value parameter too) unwinds through
	// them in the full-expression's region (15.2p1).
	lowir::Instruction inst = lowir::make_instruction(lowir::Opcode::Return, result_type_);
	// (A result in the caller's destination: each return its own path, the
	// course's form.)
	const bool shared = result_destination_.kind == lowir::ValueKind::None && shared_return();
	const size_t first = cleanups_.size();
	hidden_context_ = true;
	result_root_ = sema::ClassOf(result);
	const bool region = begin_full_expression(stmt.value);
	result_root_ = nullptr;
	const uint32_t temporaries = temporaries_made_;
	// With a local whose destructor may throw (the result is kept against
	// it, CWG 2176), a result copied from a glvalue or made by a constructor
	// whose arguments make no temporaries opens its region at the
	// construction, after the operands (the course's form).
	const syntax::Expr* value = WithoutParentheses(stmt.value);
	bool throwing_local = false;
	for (size_t i = region_floor(); i < cleanups_.size() && result_destination_.kind != lowir::ValueKind::None; ++i)
		throwing_local = throwing_local || destructor_may_throw(cleanups_[i].type);
	bool at_construction = value->kind == syntax::ExprKind::Id;
	if (value->kind == syntax::ExprKind::FunctionalCast &&
		static_cast<const syntax::FunctionalCastExpr*>(value)->braced == nullptr)
	{
		at_construction = true;
		for (const syntax::Expr* argument : static_cast<const syntax::FunctionalCastExpr*>(value)->arguments)
			at_construction = at_construction && !creates_temporaries(argument);
	}
	const bool saved_here = result_here_;
	result_here_ = region && throwing_local && at_construction;
	const syntax::Expr* e = WithoutParentheses(stmt.value);
	const sema::Conversion* conversion = e->facts->conversion;
	// `return T();` of an empty class without bases is not zeroed (the
	// course's form, as for one passed or bound).
	const bool saved_empty = passing_empty_;
	const sema::ClassEntity* returned = sema::ClassOf(result);
	passing_empty_ = returned != nullptr && returned->bases == nullptr && e->kind == syntax::ExprKind::FunctionalCast &&
		static_cast<const syntax::FunctionalCastExpr*>(e)->arguments.size == 0 &&
		static_cast<const syntax::FunctionalCastExpr*>(e)->braced == nullptr;
	// CWG 2176: a result made here (not the local constructed in the
	// result's destination) is destroyed if a local's destruction throws.
	Cleanup made = Cleanup();
	made.type = result;
	if (result_destination_.kind != lowir::ValueKind::None)
	{
		if (returned_local_ == nullptr || e->kind != syntax::ExprKind::Id || e->facts->entity != returned_local_)
		{
			made.storage = result_destination_;
			initialize_result(*stmt.value, conversion, returned, result_destination_, stmt.location);
		}
	}
	else
	{
		if (return_slot_ == lowir::kNoId)
			return_slot_ = generated_slot(lowir::SlotKind::Retobj, result_type_);
		inst.ops[0] = lowir::Value::slot(return_slot_);
		Place place = Place();
		place.storage = inst.ops[0];
		made.storage = inst.ops[0];
		initialize_result(*stmt.value, conversion, returned, address_of(place), stmt.location);
	}
	passing_empty_ = saved_empty;
	destroy_temporaries(first);
	// So kept, the landing of the region constructing the result (not a
	// call's, nor a class conditional's, whose operands each place theirs,
	// nor that of one that made temporaries, whose segments place theirs)
	// is placed after the return (the course's form).
	const bool saved_defer = defer_landings_;
	defer_landings_ = throwing_local && temporaries_made_ == temporaries &&
		value->kind != syntax::ExprKind::Conditional && value->kind != syntax::ExprKind::Call;
	end_full_expression(region);
	defer_landings_ = saved_defer;
	result_here_ = saved_here;
	const bool saved_returning = returning_result_;
	if (made.storage.kind != lowir::ValueKind::None && needs_destruction(result))
	{
		returned_result_ = made;
		returning_result_ = true;
	}
	if (shared)
	{
		jump(return_entry());
		place_chain_blocks();
	}
	else
	{
		return_from_body(inst);
		// (The result's landing first, then the lexical landings.)
		if (terminated_ && !chain_blocks_.empty() && !saved_defer)
		{
			std::vector<ChainBlock> lexical;
			lexical.swap(lexical_pending_);
			place_chain_blocks();
			lexical_pending_.insert(lexical_pending_.begin(), lexical.begin(), lexical.end());
		}
	}
	returning_result_ = saved_returning;
}

void FunctionLowering::initialize_result(const syntax::Expr& expr, const sema::Conversion* conversion,
	const sema::ClassEntity* target, lowir::Value object, uint32_t location)
{
	// A conditional's result is initialized in each branch (the course's
	// form): from the selected operand as that operand's own conversion
	// says (a prvalue of the result's class `target`, 12.8p31), else by the
	// copy the whole glvalue selected. A prvalue of another type (a scalar,
	// an array operand's decayed pointer, another class: 5.16p6) is the one
	// value the result's converting constructor or conversion function
	// takes (8.5p16).
	const syntax::Expr* e = WithoutParentheses(&expr);
	if (e->kind != syntax::ExprKind::Conditional ||
		(e->facts->category == sema::ValueCategory::PRValue && sema::ClassOf(e->facts->type) != target))
	{
		initialize_from(expr, conversion, object, location);
		return;
	}
	const syntax::ConditionalExpr& choice = static_cast<const syntax::ConditionalExpr&>(*e);
	const bool glvalue = e->facts->category != sema::ValueCategory::PRValue;
	const syntax::Expr* operands[2] = { choice.then_expr, choice.else_expr };
	const lowir::BlockId blocks[2] = { new_block(lowir::BlockKind::CondobjThen),
		new_block(lowir::BlockKind::CondobjElse) };
	const lowir::BlockId end = new_block(lowir::BlockKind::CondobjEnd);
	branch(truth(*choice.condition), blocks[0], blocks[1]);
	for (int i = 0; i < 2; ++i)
	{
		// A branch's temporaries end with it, once the result is made.
		place(blocks[i]);
		const size_t first = cleanups_.size();
		const sema::Conversion* own = WithoutParentheses(operands[i])->facts->conversion;
		if (WithoutParentheses(operands[i])->kind == syntax::ExprKind::Throw)
			discard(*operands[i]);
		else if (glvalue && conversion != nullptr && conversion->constructor != nullptr)
			call_constructor(conversion->constructor, object, *operands[i], conversion->argument, location);
		else
			initialize_result(*operands[i], glvalue ? conversion : own, target, object, location);
		if (!tracked_.active)
			destroy_temporaries(first);
		jump(end);
	}
	place(end);
}

void FunctionLowering::return_statement(const syntax::ValueStmt& stmt)
{
	// 6.6.3: the value copy-initializes the result (a reference result
	// binds to its glvalue); a void function's operand is evaluated first.
	const syntax::Expr* saved = full_root(stmt.value);
	returned_value(stmt);
	choice_root_ = saved;
}

void FunctionLowering::returned_value(const syntax::ValueStmt& stmt)
{
	const sema::Type* result = function_ != nullptr ? function_->type->canonical->element : nullptr;
	if (stmt.value != nullptr && result != nullptr && !result->is_reference() &&
		result->unqualified->kind == sema::TypeKind::Class)
	{
		class_return(stmt, result);
		return;
	}
	lowir::Instruction inst = lowir::make_instruction(lowir::Opcode::Return, result_type_);
	const bool shared = shared_return(true);
	const size_t first = cleanups_.size();
	hidden_context_ = stmt.value != nullptr;
	const bool region = stmt.value != nullptr && begin_full_expression(stmt.value);
	if (stmt.value == nullptr || result_type_.is_void())
	{
		if (stmt.value != nullptr)
			discard(*stmt.value);
		destroy_temporaries(first);
		end_full_expression(region);
		if (shared)
		{
			// The continuation shared with the other returns.
			jump(return_entry());
			place_chain_blocks();
			return;
		}
		return_from_body(inst);
		return;
	}
	if (result->is_reference())
		inst.ops[0] = bind_reference(*stmt.value, result, ConversionContext::Return);
	else if (stmt.value->kind == syntax::ExprKind::BracedInit &&
		sema::LaneList(result, static_cast<const syntax::BracedInitList&>(*stmt.value)))
		inst.ops[0] = vector_list(result, static_cast<const syntax::BracedInitList&>(*stmt.value)).value;
	else if (stmt.value->kind == syntax::ExprKind::BracedInit)
	{
		const syntax::BracedInitList& list = static_cast<const syntax::BracedInitList&>(*stmt.value);
		inst.ops[0] = list.elements.empty() ? zero_value(result).value
											: fit(operand(*list.elements[0], ConversionContext::Return), result).value;
	}
	else
		inst.ops[0] = fit(operand(*stmt.value, ConversionContext::Return), result).value;
	if (shared)
	{
		// 6.6.3p2 then 6.6p2: the value is kept in the continuation's slot
		// before the objects are destroyed.
		if (return_slot_ == lowir::kNoId)
			return_slot_ = generated_slot(lowir::SlotKind::CleanupReturn, result_type_);
		Place slot = Place();
		slot.storage = lowir::Value::slot(return_slot_);
		RValue value;
		value.value = inst.ops[0];
		value.type = result_type_;
		store(result_type_, value, slot);
		destroy_temporaries(first);
		end_full_expression(region);
		jump(return_entry());
		place_chain_blocks();
		return;
	}
	destroy_temporaries(first);
	end_full_expression(region);
	return_from_body(inst);
}

void FunctionLowering::return_from_body(const lowir::Instruction& inst)
{
	// 6.6p2, 6.6.3: the value is computed, then every automatic object is
	// destroyed (but the returned one in the result's destination) and the
	// exception-handling contexts left; a destructor's return runs its
	// subobjects' destructors first (12.4p8), inside its function-try-block
	// if it has one (the outermost context).
	destroy_cleanups(0, true, destructor_region_ && function_try_block() ? 1 : 0);
	if (destructor_region_)
	{
		// A destructor's return leaves its body's region for the
		// subobjects' destruction.
		emit_marker(lowir::Opcode::EhEnd, lowir::kNoId);
		if (destructor_epilogue_ == lowir::kNoId)
			destructor_epilogue_ = new_block(lowir::BlockKind::DestructorReturnEpilogue);
		jump(destructor_epilogue_);
		return;
	}
	if (constructor_region_)
		emit_marker(lowir::Opcode::EhEnd, lowir::kNoId);
	terminate(inst);
}

// ------------------------------------------------- returns and labels

uint32_t FunctionLowering::count_returns(const syntax::Stmt& stmt, uint32_t live)
{
	// The return statements of each set of alive automatic objects (keyed
	// by the innermost) and the set at each label, declarations tracked
	// lexically in a persistent chain; returns the set after the statement
	// (a declaration's grows it, a nested scope's ends with it).
	check_stack(stmt.location);
	switch (stmt.kind)
	{
	case syntax::StmtKind::Compound:
	{
		uint32_t inner = live;
		for (const syntax::Stmt* item : static_cast<const syntax::CompoundStmt&>(stmt).items)
			inner = count_returns(*item, inner);
		return live;
	}
	case syntax::StmtKind::Ambiguous:
		return count_returns(static_cast<const syntax::AmbiguousStmt&>(stmt).reading(), live);
	case syntax::StmtKind::Declaration:
	{
		const syntax::Decl* decl = static_cast<const syntax::DeclarationStmt&>(stmt).declaration;
		if (decl->kind != syntax::DeclKind::Simple)
			return live;
		for (const syntax::InitDeclarator& init : static_cast<const syntax::SimpleDecl*>(decl)->declarators)
			live = declared_live(init.declared,
				init.declared != nullptr ? sema::VariableInitializer(init.declared->entity, init.initializer)
										 : init.initializer,
				live);
		return live;
	}
	case syntax::StmtKind::Return:
	{
		// A return inside a try block or handler leaves its contexts on a
		// path of its own (it never shares a continuation; see
		// shared_return), so it does not count.
		if (live == kNoLive)
			return live;
		const sema::Entity* innermost = live_nodes_[live].variable;
		if (scan_contexts_ != 0)
		{
			for (ContextReturns& known : context_returns_)
			{
				if (known.variable == innermost && known.context == scan_context_)
				{
					++known.count;
					return live;
				}
			}
			context_returns_.push_back(ContextReturns{ innermost, scan_context_, 1 });
			return live;
		}
		const uint32_t known = return_counts_.find(innermost);
		return_counts_.set(innermost, known == PointerMap::kMissing ? 1 : known + 1);
		return live;
	}
	case syntax::StmtKind::If:
	{
		// A condition's declared object is alive in the substatements
		// (6.4p3, 6.5.1p2).
		const syntax::IfStmt& branch = static_cast<const syntax::IfStmt&>(stmt);
		const syntax::ConditionDeclaration* condition = branch.condition.declaration;
		const uint32_t inner = condition != nullptr ? declared_live(condition->declared, condition->initializer, live)
													: live;
		if (syntax::LiveThen(branch) != nullptr)
			count_returns(*branch.then_branch, inner);
		if (syntax::LiveElse(branch) != nullptr)
			count_returns(*branch.else_branch, inner);
		return live;
	}
	case syntax::StmtKind::While:
	case syntax::StmtKind::Switch:
	{
		const syntax::ConditionalLoopStmt& loop = static_cast<const syntax::ConditionalLoopStmt&>(stmt);
		const syntax::ConditionDeclaration* condition = loop.condition.declaration;
		count_returns(
			*loop.body, condition != nullptr ? declared_live(condition->declared, condition->initializer, live) : live);
		return live;
	}
	case syntax::StmtKind::Do:
		count_returns(*static_cast<const syntax::DoStmt&>(stmt).body, live);
		return live;
	case syntax::StmtKind::For:
	{
		const syntax::ForStmt& loop = static_cast<const syntax::ForStmt&>(stmt);
		uint32_t inner = live;
		if (loop.init_declaration != nullptr)
		{
			syntax::DeclarationStmt init = syntax::DeclarationStmt();
			init.kind = syntax::StmtKind::Declaration;
			init.location = stmt.location;
			init.declaration = loop.init_declaration;
			inner = count_returns(init, inner);
		}
		if (loop.condition != nullptr && loop.condition->declaration != nullptr)
			inner = declared_live(
				loop.condition->declaration->declared, loop.condition->declaration->initializer, inner);
		count_returns(*loop.body, inner);
		return live;
	}
	case syntax::StmtKind::RangeFor:
	{
		// 6.5.4p1's equivalent's variables are alive in the body.
		const syntax::RangeForStmt& loop = static_cast<const syntax::RangeForStmt&>(stmt);
		const sema::RangeForFacts& facts = *loop.facts;
		uint32_t inner = live;
		if (facts.range != nullptr)
			inner = declared_live(facts.range->declared, facts.range->initializer, inner);
		inner = declared_live(facts.begin->declared, facts.begin->initializer, inner);
		if (facts.end != nullptr)
			inner = declared_live(facts.end->declared, facts.end->initializer, inner);
		inner = declared_live(facts.declaration.declared, facts.declaration.initializer, inner);
		count_returns(*loop.body, inner);
		return live;
	}
	case syntax::StmtKind::Case:
		return count_returns(*static_cast<const syntax::CaseStmt&>(stmt).body, live);
	case syntax::StmtKind::Labeled:
	{
		// The objects alive at a label: a goto destroys the others (6.6p2).
		const syntax::LabeledStmt& labeled = static_cast<const syntax::LabeledStmt&>(stmt);
		label_live_.set(labeled.label, live == kNoLive ? 0 : live + 1);
		label_contexts_.set(labeled.label, scan_contexts_);
		if (scan_floor_ != 0)
			label_floors_.set(labeled.label, scan_floor_);
		return count_returns(*labeled.body, live);
	}
	case syntax::StmtKind::Default:
		return count_returns(*static_cast<const syntax::LabeledStmt&>(stmt).body, live);
	case syntax::StmtKind::Try:
	{
		// The try block and each handler are contexts of their labels; a
		// handler's class object variable is alive in its body.
		const syntax::TryStmt& attempt = static_cast<const syntax::TryStmt&>(stmt);
		const void* context = scan_context_;
		++scan_contexts_;
		scan_context_ = &attempt;
		count_returns(*attempt.body, live);
		for (const syntax::Handler& handler : attempt.handlers)
		{
			scan_context_ = &handler;
			count_returns(*handler.body, declared_live(handler.exception.declared, nullptr, live));
		}
		scan_context_ = context;
		--scan_contexts_;
		return live;
	}
	default:
		return live;
	}
}

uint32_t FunctionLowering::declared_live(
	const sema::Binding* binding, const syntax::Initializer* initializer, uint32_t live)
{
	// The chain grown by a declaration whose objects will be alive: a class
	// object's (a single one whose destructor has no effect never is, see
	// local_object), or the temporaries a reference or aggregate extends
	// (12.2p5).
	if (binding == nullptr || binding->kind != sema::BindingKind::Variable)
		return live;
	const sema::VariableEntity* variable = static_cast<const sema::VariableEntity*>(binding->entity);
	if (variable->is_extern || variable->is_static || variable->is_thread_local ||
		variable->owner->kind == sema::ScopeKind::Namespace)
		return live;
	const sema::Type* type = variable->type;
	bool alive = needs_destruction(type) &&
		(type->kind == sema::TypeKind::Array || !lowering_.destructor_does_nothing(object_class(type)));
	if (!alive && initializer != nullptr)
	{
		const syntax::Expr* single = initializer->kind == syntax::InitializerKind::Parenthesized
			? (initializer->arguments.size == 1 ? initializer->arguments[0] : nullptr)
			: initializer->expression;
		if (single != nullptr && type->is_reference())
			alive = extends_temporary(reference_initializer(single));
		else if (single != nullptr && single->kind == syntax::ExprKind::BracedInit && object_class(type) != nullptr &&
			variable->constructor == nullptr)
			alive = aggregate_extends(single);
	}
	if (!alive)
		return live;
	live_nodes_.push_back(LiveNode{ variable, live });
	return static_cast<uint32_t>(live_nodes_.size() - 1);
}

void FunctionLowering::scan_body()
{
	// The returns of each alive set and the labels' alive sets, found by
	// one pass over the body the first time a return or goto leaves
	// objects alive.
	if (body_scanned_)
		return;
	body_scanned_ = true;
	const syntax::FunctionBody* body = function_ != nullptr ? function_->body : nullptr;
	const uint32_t live = kNoLive;
	// A function-try-block's statement is the context of the labels in its
	// body and handlers, as a try statement's (15p2: a goto there leaves
	// none of its regions).
	if (body != nullptr && !function_->defaulted)
		count_returns(body->try_block ? static_cast<const syntax::Stmt&>(*body->try_block) : *body->body, live);
}

bool FunctionLowering::shared_return(bool contexts)
{
	// Whether this return shares its continuation: another return has the
	// same objects alive (the same destructor sequence and context), keyed
	// by the innermost one it destroys (not the result's local).
	size_t innermost = cleanups_.size();
	while (innermost > 0 && cleanups_[innermost - 1].result)
		--innermost;
	if (innermost == 0 || destructor_region_ || (!eh_contexts_.empty() && !contexts))
		return false;
	scan_body();
	if (!eh_contexts_.empty())
	{
		for (const ContextReturns& known : context_returns_)
		{
			if (known.variable == cleanups_[innermost - 1].variable && known.context == eh_contexts_.back().key)
				return known.count > 1;
		}
		return false;
	}
	const uint32_t count = return_counts_.find(cleanups_[innermost - 1].variable);
	return count != PointerMap::kMissing && count > 1;
}

void FunctionLowering::goto_statement(const syntax::GotoStmt& stmt)
{
	// 6.6p2: the objects alive here but not at the label are destroyed,
	// innermost first, before the transfer, and the try blocks and handlers
	// it leaves are left as their normal exits leave them (15p2: a goto
	// enters none).
	// The label's declarations are a lexical chain: the innermost of them
	// whose objects are alive here too (its last one where its lowering
	// left it) marks the common part. A declaration may have several (a
	// reference's extended temporaries, 12.2p5) or none.
	if (cleanups_.empty() && eh_contexts_.empty())
	{
		jump(goto_label(stmt.label));
		return;
	}
	scan_body();
	const uint32_t contexts = label_contexts_.find(stmt.label);
	const uint32_t found = label_live_.find(stmt.label);
	size_t common = 0;
	for (uint32_t node = found == PointerMap::kMissing || found == 0 ? kNoLive : found - 1; node != kNoLive;
		node = live_nodes_[node].parent)
	{
		const LiveNode& live = live_nodes_[node];
		const uint32_t index = local_index_.find(live.variable);
		const size_t depth = index != PointerMap::kMissing ? locals_[index].depth : 0;
		if (depth != 0 && depth <= cleanups_.size() && cleanups_[depth - 1].variable == live.variable)
		{
			common = depth;
			break;
		}
	}
	// A label in a statement expression (the goto is inside it too): the
	// objects alive where the block began, its enclosing expression's
	// temporaries among them, are alive at the label.
	const uint32_t floor = label_floors_.find(stmt.label);
	if (floor != PointerMap::kMissing && common < floor - 1)
		common = floor - 1;
	destroy_cleanups(common, false, contexts == PointerMap::kMissing ? 0 : contexts);
	jump(goto_label(stmt.label));
}

// ------------------------------------------------------ terminate guard

} // namespace lower
