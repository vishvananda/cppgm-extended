#include "lower/function_lowering.h"

// Lexical cleanup (6.6p2 with 15.2p1): a local destroyed where its
// scope ends whose destructor may throw runs in a region of its own when
// objects or handlers lie under it inside the innermost try (or the
// function). Its landing (the enclosing tries' clauses, a cleanup clause
// retired at once) enters a chain of its own, apart from the calls'
// unwinding chains: each object under it destroyed (in the shared
// terminate region when its destructor may throw), each handler left (its
// record, then its catch), then the try's entry (its region popped) or
// resume. The chain's blocks are shared between the scope ends under one
// complete context stack (the course's form).
namespace lower {

lowir::BlockId FunctionLowering::lexical_handler(size_t depth, size_t contexts)
{
	if (!destructor_may_throw(cleanups_[depth].type))
		return lowir::kNoId;
	// Seen from the contexts still entered (as destruction_handler).
	std::vector<EhContext> inner;
	const size_t innermost = innermost_try_;
	if (contexts < eh_contexts_.size())
	{
		inner.assign(eh_contexts_.begin() + static_cast<std::ptrdiff_t>(contexts), eh_contexts_.end());
		eh_contexts_.resize(contexts);
		innermost_try_ = SIZE_MAX;
		for (size_t c = contexts; c-- > 0 && innermost_try_ == SIZE_MAX;)
		{
			if (eh_contexts_[c].is_try)
				innermost_try_ = c;
		}
	}
	const EhContext* attempt = innermost_try();
	const size_t floor = attempt != nullptr ? attempt->floor : 0;
	const size_t first_handler = attempt != nullptr ? innermost_try_ + 1 : 0;
	lowir::BlockId dispatch = lowir::kNoId;
	// A try's own region takes an exception with nothing to do first. A
	// return's completed class result is destroyed first (CWG 2176).
	const bool returned = returning_result_;
	if (depth > floor || eh_contexts_.size() > first_handler || body_unwind_target() != lowir::kNoId || returned)
	{
		const lowir::BlockId entry = lexical_entry(depth);
		dispatch = new_block(lowir::BlockKind::LexicalCleanupDispatch);
		ChainBlock block = ChainBlock();
		block.block = dispatch;
		block.unwinding = true;
		block.action = ChainAction::LexicalDispatch;
		block.tail = entry;
		block.statement = attempt != nullptr ? attempt->statement : nullptr;
		block.chain_first = attempt != nullptr ? attempt->chain_first : 0;
		block.chain_count = attempt != nullptr ? attempt->chain_count : 0;
		if (returned)
			block.cleanup = returned_result_;
		lexical_pending_.push_back(block);
	}
	if (!inner.empty())
	{
		eh_contexts_.insert(eh_contexts_.end(), inner.begin(), inner.end());
		innermost_try_ = innermost;
	}
	return dispatch;
}

lowir::BlockId FunctionLowering::lexical_entry(size_t count)
{
	// The chain of the first `count` objects inside the contexts, made as
	// eh_unwind_entry's (top-down to the first block made, then the missing
	// ones bottom-up), keyed by the complete context stack.
	const EhContext* attempt = innermost_try();
	const size_t floor = attempt != nullptr ? attempt->floor : 0;
	const size_t first_handler = attempt != nullptr ? innermost_try_ + 1 : 0;
	const uint32_t stack = eh_contexts_.empty() ? 0 : eh_contexts_.back().stack;
	const auto node = [this, stack](const EhElement& element) -> lowir::BlockId& {
		if (element.context != SIZE_MAX)
		{
			EhContext& context = eh_contexts_[element.context];
			if (context.lexical_stack != stack)
			{
				context.lexical = 0;
				context.lexical_stack = stack;
			}
			return context.lexical;
		}
		Cleanup& cleanup = cleanups_[element.object];
		if (cleanup.lexical_stack != stack)
		{
			cleanup.lexical = 0;
			cleanup.lexical_stack = stack;
		}
		return cleanup.lexical;
	};
	std::vector<EhElement>& missing = eh_elements_;
	missing.clear();
	size_t handler = eh_contexts_.size();
	lowir::BlockId tail = 0;
	for (size_t i = count + 1; i-- > floor && tail == 0;)
	{
		if (i < count)
		{
			const EhElement element = EhElement{ i, SIZE_MAX, 0 };
			tail = node(element);
			if (tail == 0)
				missing.push_back(element);
		}
		for (; tail == 0 && handler > first_handler && eh_contexts_[handler - 1].floor == i; --handler)
		{
			const EhElement element = EhElement{ SIZE_MAX, handler - 1, 0 };
			tail = node(element);
			if (tail == 0)
				missing.push_back(element);
		}
	}
	if (tail == 0)
	{
		// The terminal: a detached body's cleanup entry, the try's entry
		// (its region popped) or resume.
		const lowir::BlockId body = body_unwind_target();
		if (body != lowir::kNoId)
		{
			if (lexical_resume_ == lowir::kNoId || lexical_resume_stack_ != stack || lexical_body_ != body)
			{
				lexical_resume_ = new_block(lowir::BlockKind::LexicalUnwind);
				lexical_resume_stack_ = stack;
				lexical_body_ = body;
				ChainBlock block = ChainBlock();
				block.block = lexical_resume_;
				block.terminal = true;
				block.unwinding = true;
				block.action = ChainAction::BodyEntry;
				block.pops = 1;
				block.tail = body;
				lexical_pending_.push_back(block);
			}
			tail = lexical_resume_;
		}
		else if (attempt != nullptr)
		{
			lowir::BlockId& made = node(EhElement{ SIZE_MAX, innermost_try_, 0 });
			if (made == 0)
			{
				made = new_block(lowir::BlockKind::LexicalUnwind);
				ChainBlock block = ChainBlock();
				block.block = made;
				block.terminal = true;
				block.unwinding = true;
				block.action = ChainAction::LexicalTryEntry;
				block.tail = eh_contexts_[innermost_try_].entry;
				lexical_pending_.push_back(block);
			}
			tail = made;
		}
		else
		{
			if (lexical_resume_ == lowir::kNoId || lexical_resume_stack_ != stack || lexical_body_ != lowir::kNoId)
			{
				lexical_resume_ = new_block(lowir::BlockKind::LexicalUnwind);
				lexical_resume_stack_ = stack;
				lexical_body_ = lowir::kNoId;
				ChainBlock block = ChainBlock();
				block.block = lexical_resume_;
				block.terminal = true;
				block.unwinding = true;
				lexical_pending_.push_back(block);
			}
			tail = lexical_resume_;
		}
	}
	for (size_t e = missing.size(); e-- > 0;)
	{
		const EhElement element = missing[e];
		const lowir::BlockId block = new_block(lowir::BlockKind::LexicalUnwind);
		ChainBlock made = ChainBlock();
		made.block = block;
		made.unwinding = true;
		made.tail = tail;
		made.below = lowir::kNoId;
		if (element.context != SIZE_MAX)
			made.action = ChainAction::LexicalHandlerExit;
		else
			made.cleanup = cleanups_[element.object];
		lexical_pending_.push_back(made);
		node(element) = block;
		tail = block;
	}
	return tail;
}

// ------------------------------------------------- detached special bodies

bool FunctionLowering::detached_body()
{
	// A constructor's or destructor's body whose unwinding reaches its
	// subobjects' cleanup entry (the course's lexical body cleanup); so do
	// a constructor's member array elements' handlers when an element's
	// constructor may throw.
	if (function_->constructor && function_->copy_kind == sema::CopyKind::None)
	{
		for (uint32_t i = 0; i < function_->init_count; ++i)
		{
			const sema::SubobjectInit& init = function_->inits[i];
			if (init.member != nullptr && init.member->variable->type->kind == sema::TypeKind::Array &&
				init.constructor != nullptr && !init.constructor->trivial && call_may_throw(init.constructor))
				return true;
		}
	}
	const syntax::FunctionBody* body = function_->body;
	if (body == nullptr || function_->defaulted || body->body == nullptr)
		return false;
	bool alive = false;
	return body_unwinds(*body->body, alive);
}

bool FunctionLowering::body_unwinds(const syntax::Stmt& stmt, bool& alive)
{
	// A try statement, a local whose destructor may throw, or a statement
	// that may throw while a local (with a destructor) is alive; `alive`
	// grows with a scope's declarations.
	check_stack(stmt.location);
	switch (stmt.kind)
	{
	case syntax::StmtKind::Compound:
	{
		bool inner = alive;
		for (const syntax::Stmt* item : static_cast<const syntax::CompoundStmt&>(stmt).items)
		{
			if (body_unwinds(*item, inner))
				return true;
		}
		return false;
	}
	case syntax::StmtKind::Ambiguous:
		return body_unwinds(static_cast<const syntax::AmbiguousStmt&>(stmt).reading(), alive);
	case syntax::StmtKind::Try:
		return true;
	case syntax::StmtKind::Declaration:
	{
		const syntax::Decl* decl = static_cast<const syntax::DeclarationStmt&>(stmt).declaration;
		if (decl->kind != syntax::DeclKind::Simple)
			return false;
		for (const syntax::InitDeclarator& init : static_cast<const syntax::SimpleDecl*>(decl)->declarators)
		{
			if (alive && declarator_may_throw(init))
				return true;
			const sema::Binding* binding = init.declared;
			if (binding == nullptr || binding->kind != sema::BindingKind::Variable)
				continue;
			const sema::VariableEntity* variable = static_cast<const sema::VariableEntity*>(binding->entity);
			if (variable->is_extern || variable->is_static || variable->is_thread_local)
				continue;
			const sema::Type* type = variable->type;
			if (type->is_reference())
			{
				const syntax::Initializer* initializer = sema::VariableInitializer(variable, init.initializer);
				const syntax::Expr* single = initializer == nullptr ? nullptr
					: initializer->kind == syntax::InitializerKind::Parenthesized
					? (initializer->arguments.size == 1 ? initializer->arguments[0] : nullptr)
					: initializer->expression;
				alive = alive || (single != nullptr && extends_temporary(reference_initializer(single)));
				continue;
			}
			if (!needs_destruction(type))
				continue;
			if (destructor_may_throw(type))
				return true;
			alive = alive || type->kind == sema::TypeKind::Array ||
				!lowering_.destructor_does_nothing(object_class(type));
		}
		return false;
	}
	case syntax::StmtKind::If:
	{
		const syntax::IfStmt& branch = static_cast<const syntax::IfStmt&>(stmt);
		if (alive &&
			(branch.condition.declaration != nullptr ? initializer_may_throw(branch.condition.declaration->initializer)
													 : may_throw(branch.condition.expression)))
			return true;
		bool then_alive = alive;
		bool else_alive = alive;
		return (syntax::LiveThen(branch) != nullptr && body_unwinds(*branch.then_branch, then_alive)) ||
			(syntax::LiveElse(branch) != nullptr && body_unwinds(*branch.else_branch, else_alive));
	}
	case syntax::StmtKind::While:
	case syntax::StmtKind::Switch:
	{
		const syntax::ConditionalLoopStmt& loop = static_cast<const syntax::ConditionalLoopStmt&>(stmt);
		if (alive &&
			(loop.condition.declaration != nullptr ? initializer_may_throw(loop.condition.declaration->initializer)
												   : may_throw(loop.condition.expression)))
			return true;
		bool inner = alive;
		return body_unwinds(*loop.body, inner);
	}
	case syntax::StmtKind::Do:
	{
		const syntax::DoStmt& loop = static_cast<const syntax::DoStmt&>(stmt);
		bool inner = alive;
		return (alive && may_throw(loop.condition)) || body_unwinds(*loop.body, inner);
	}
	case syntax::StmtKind::For:
	{
		const syntax::ForStmt& loop = static_cast<const syntax::ForStmt&>(stmt);
		bool inner = alive;
		if (loop.init_declaration != nullptr)
		{
			syntax::DeclarationStmt init = syntax::DeclarationStmt();
			init.kind = syntax::StmtKind::Declaration;
			init.location = stmt.location;
			init.declaration = loop.init_declaration;
			if (body_unwinds(init, inner))
				return true;
		}
		if (inner &&
			(may_throw(loop.init_expression) || may_throw(loop.iteration) ||
				(loop.condition != nullptr &&
					(loop.condition->declaration != nullptr
							? initializer_may_throw(loop.condition->declaration->initializer)
							: may_throw(loop.condition->expression)))))
			return true;
		return body_unwinds(*loop.body, inner);
	}
	case syntax::StmtKind::Case:
		return body_unwinds(*static_cast<const syntax::CaseStmt&>(stmt).body, alive);
	case syntax::StmtKind::Default:
	case syntax::StmtKind::Labeled:
		return body_unwinds(*static_cast<const syntax::LabeledStmt&>(stmt).body, alive);
	default:
		return alive && statement_may_throw(stmt);
	}
}

void FunctionLowering::open_detached_cleanup(lowir::BlockId entry)
{
	const EhContext* attempt = innermost_try();
	if (attempt != nullptr)
		emit_try_clauses(*attempt->statement, attempt->chain_first, attempt->chain_count);
	emit_marker(lowir::Opcode::EhEnd, lowir::kNoId);
	jump(entry);
	place(entry);
}

void FunctionLowering::close_detached_cleanup(bool detached)
{
	// A detached cleanup's record was popped at its landing: the unwinding
	// resumes, or enters an enclosing try (its region popped).
	if (!detached)
	{
		emit_marker(lowir::Opcode::EhEnd, lowir::kNoId);
		emit_resume();
		return;
	}
	const EhContext* attempt = innermost_try();
	if (attempt == nullptr)
	{
		emit_resume();
		return;
	}
	emit_marker(lowir::Opcode::EhEnd, lowir::kNoId);
	jump(attempt->entry);
}

lowir::BlockId FunctionLowering::body_unwind_target() const
{
	// The detached body's cleanup entry, unless a try inside the body
	// takes the unwinding first.
	if (body_target_ == lowir::kNoId)
		return lowir::kNoId;
	if (innermost_try_ != SIZE_MAX && (body_depth_ == 0 || innermost_try_ >= body_depth_))
		return lowir::kNoId;
	return body_target_;
}

lowir::BlockId FunctionLowering::body_terminal(int key)
{
	// The calls' chains' terminal in a detached body: the landing's record
	// (when still pushed) and the body's cleanup record popped, then the
	// cleanup entry; one per key under one context stack and entry.
	const uint32_t stack = eh_contexts_.empty() ? 0 : eh_contexts_.back().stack;
	if (body_terminal_target_ != body_target_ || body_terminal_stack_ != stack)
	{
		body_terminals_[0] = body_terminals_[1] = lowir::kNoId;
		body_terminal_target_ = body_target_;
		body_terminal_stack_ = stack;
	}
	lowir::BlockId& made = body_terminals_[key];
	if (made == lowir::kNoId)
	{
		made = new_block(lowir::BlockKind::CleanupResume);
		ChainBlock block = ChainBlock();
		block.block = made;
		block.terminal = true;
		block.unwinding = true;
		block.action = ChainAction::BodyEntry;
		block.pops = static_cast<uint32_t>(key + 1);
		block.tail = body_target_;
		chain_blocks_.push_back(block);
	}
	return made;
}

void FunctionLowering::place_lexical_blocks()
{
	// After the block that made them ends (the course places them there,
	// before the blocks made later).
	std::vector<ChainBlock> blocks;
	blocks.swap(lexical_pending_);
	// Their destructions share the function's terminate landing.
	const bool saved = lexical_placing_;
	lexical_placing_ = true;
	place_blocks(blocks);
	lexical_placing_ = saved;
}

void FunctionLowering::place_lexical_block(const ChainBlock& block)
{
	switch (block.action)
	{
	case ChainAction::LexicalHandlerExit:
		emit_marker(lowir::Opcode::EhEnd, lowir::kNoId);
		call_end_catch();
		jump(block.tail);
		return;
	case ChainAction::LexicalTryEntry:
		emit_marker(lowir::Opcode::EhEnd, lowir::kNoId);
		jump(block.tail);
		return;
	case ChainAction::BodyEntry:
		for (uint32_t i = 0; i < block.pops; ++i)
			emit_marker(lowir::Opcode::EhEnd, lowir::kNoId);
		jump(block.tail);
		return;
	case ChainAction::LexicalDispatch:
		if (block.statement != nullptr)
			emit_try_clauses(*block.statement, block.chain_first, block.chain_count);
		emit_marker(lowir::Opcode::EhCleanup, lowir::kNoId);
		emit_marker(lowir::Opcode::EhEnd, lowir::kNoId);
		if (block.cleanup.type != nullptr)
			destroy_local(block.cleanup, lowir::kNoId, true);
		jump(block.tail);
		return;
	default:
		return;
	}
}

void FunctionLowering::element_exit()
{
	// An element handler's end, the elements made destroyed: in a detached
	// body (its subobjects' cleanup record pushed) the records popped and
	// the cleanup entry entered (the course's form), else the unwinding
	// resumes.
	const lowir::BlockId body = body_unwind_target();
	if (body == lowir::kNoId)
		return emit_resume();
	emit_marker(lowir::Opcode::EhEnd, lowir::kNoId);
	emit_marker(lowir::Opcode::EhEnd, lowir::kNoId);
	jump(body);
}

void FunctionLowering::flush_deferred()
{
	// A constructor's cleanup blocks follow the block that opened their
	// regions: each destroys its subobject, then continues with the
	// previous subobject's cleanup, the first one ending the region. A
	// detached body's are seen from the contexts entered where its record
	// was pushed (a try the body entered since is not theirs).
	std::vector<DeferredCleanup> blocks;
	blocks.swap(deferred_);
	for (const DeferredCleanup& deferred : blocks)
	{
		builder_.place_block(deferred.block);
		builder_.set_insertion_block(deferred.block);
		terminated_ = false;
		std::vector<EhContext> inner;
		const size_t innermost = innermost_try_;
		if (deferred.entry != lowir::kNoId && deferred.contexts < eh_contexts_.size())
		{
			inner.assign(eh_contexts_.begin() + static_cast<std::ptrdiff_t>(deferred.contexts), eh_contexts_.end());
			eh_contexts_.resize(deferred.contexts);
			innermost_try_ = SIZE_MAX;
			for (size_t c = deferred.contexts; c-- > 0 && innermost_try_ == SIZE_MAX;)
			{
				if (eh_contexts_[c].is_try)
					innermost_try_ = c;
			}
		}
		if (deferred.entry != lowir::kNoId)
			open_detached_cleanup(deferred.entry);
		// (destroy_subobject guards a destructor that may throw.)
		destroy_subobject(deferred.subobject, 0, true);
		if (deferred.tail != lowir::kNoId)
			jump(deferred.tail);
		else if (deferred.entry != lowir::kNoId)
			close_detached_cleanup(true);
		else
		{
			emit_marker(lowir::Opcode::EhEnd, lowir::kNoId);
			// A constructor's function-try-block encloses its subobjects'
			// initializations (15.3p4; these blocks follow the statement):
			// its handlers come before a terminate guard.
			if (function_ != nullptr && function_->constructor && function_try_block())
				terminate(lowir::make_instruction(lowir::Opcode::Resume, lowir::Type::scalar(lowir::TypeKind::Void)));
			else
				emit_resume();
		}
		if (!inner.empty())
		{
			eh_contexts_.insert(eh_contexts_.end(), inner.begin(), inner.end());
			innermost_try_ = innermost;
		}
	}
}

lowir::BlockId FunctionLowering::context_return_entry()
{
	// A shared return's chain inside exception-handling contexts: from the
	// innermost object down, each context left where it began (its normal
	// exit), each object destroyed (one whose destructor may throw in its
	// lexical landing), then the return of the continuation's slot (the
	// course's form). The blocks are made once per object and context
	// (their positions are lexically fixed), bottom-up.
	struct Element
	{
		size_t object;
		size_t context;
	};
	std::vector<Element> missing;
	lowir::BlockId tail = lowir::kNoId;
	size_t context = eh_contexts_.size();
	for (size_t i = cleanups_.size(); tail == lowir::kNoId; --i)
	{
		for (; context > 0 && eh_contexts_[context - 1].floor >= i && tail == lowir::kNoId; --context)
		{
			if (eh_contexts_[context - 1].return_exit != 0)
				tail = eh_contexts_[context - 1].return_exit;
			else
				missing.push_back(Element{ SIZE_MAX, context - 1 });
		}
		if (tail != lowir::kNoId || i == 0)
			break;
		if (cleanups_[i - 1].shared_return != lowir::kNoId)
			tail = cleanups_[i - 1].shared_return;
		else
			missing.push_back(Element{ i - 1, SIZE_MAX });
	}
	if (tail == lowir::kNoId)
	{
		if (return_terminal_ == lowir::kNoId)
		{
			return_terminal_ = new_block(lowir::BlockKind::ReturnCleanupTerminal);
			ChainBlock terminal = ChainBlock();
			terminal.block = return_terminal_;
			terminal.terminal = true;
			chain_blocks_.push_back(terminal);
		}
		tail = return_terminal_;
	}
	for (size_t e = missing.size(); e-- > 0;)
	{
		const Element element = missing[e];
		// A return leaves the local in the result's destination alive.
		if (element.object != SIZE_MAX && cleanups_[element.object].result)
		{
			cleanups_[element.object].shared_return = tail;
			continue;
		}
		ChainBlock made = ChainBlock();
		made.block = new_block(
			element.object != SIZE_MAX ? lowir::BlockKind::ReturnCleanupAction : lowir::BlockKind::ReturnCleanupExit);
		made.tail = tail;
		made.below = lowir::kNoId;
		if (element.object != SIZE_MAX)
		{
			made.action = ChainAction::ReturnObject;
			made.pops = static_cast<uint32_t>(element.object);
			made.cleanup = cleanups_[element.object];
			cleanups_[element.object].shared_return = made.block;
		}
		else
		{
			made.action = ChainAction::ReturnExit;
			made.pops = static_cast<uint32_t>(element.context);
			eh_contexts_[element.context].return_exit = made.block;
		}
		chain_blocks_.push_back(made);
		tail = made.block;
	}
	return tail;
}

void FunctionLowering::place_return_block(const ChainBlock& block)
{
	// A shared return chain's context exit, or an object's destruction seen
	// from the contexts still entered there.
	if (block.action == ChainAction::ReturnExit)
	{
		exit_context(block.pops);
		jump(block.tail);
		return;
	}
	size_t contexts = 0;
	while (contexts < eh_contexts_.size() && eh_contexts_[contexts].floor <= block.pops)
		++contexts;
	destroy_local(block.cleanup, lexical_handler(block.pops, contexts), false);
	jump(block.tail);
}

} // namespace lower
