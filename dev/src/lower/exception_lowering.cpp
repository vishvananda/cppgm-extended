#include "lower/function_lowering.h"

#include <cstring>

#include "lex/identifier_table.h"
#include "sema/conversions.h"

// Throwing and handling exceptions in source (clause 15) on the
// LowIR handler stack (see cleanup_lowering.cpp): an `eh_try` record is
// popped when its handler is entered, an `eh_cleanup` record (a handler's,
// or a landing's targetless marker) stays until `eh_end`, and a landing is
// entered whatever its clauses: `exception_selector` reads the selector of
// its clause the exception matched (0 for none).
//
// A try statement pushes its region, then dispatches on the selector of
// its own clauses: a handler's body begins the catch and runs under its
// `eh_cleanup` landing, which ends the catch when an exception leaves it; an
// exception no handler takes destroys the objects outside the statement
// and resumes (or reaches the enclosing statement's entry, its region
// popped). A try without a `...` handler inside another also advertises
// the enclosing statements' clauses (up to one with `...`), and a cleanup
// clause retired at once when objects or handlers lie between them.
// Inside a try block or a handler, a region's landing is a dispatch block
// (the enclosing tries' clauses and a marker, when inside a try) jumping
// to the unwinding chain: the objects, the handlers' exits and the try's
// entry (or resume).
// Each context-dependent chain block is keyed by whether the landing's own
// record is still pushed, so shared suffixes pop exactly the records their
// paths pushed (the README's continuation keys).
namespace lower {

namespace {

const syntax::Expr* WithoutParentheses(const syntax::Expr* expr)
{
	while (expr->kind == syntax::ExprKind::Paren)
		expr = static_cast<const syntax::ParenExpr*>(expr)->inner;
	return expr;
}

} // namespace

// ------------------------------------------------------------- selectors

uint32_t FunctionLowering::handler_selector(const syntax::Handler& handler)
{
	// Numbered through the function in the order the handlers' clauses are
	// first emitted (a try's dispatch, a region's landing inside it).
	uint32_t found = handler_selectors_.find(&handler);
	if (found == PointerMap::kMissing)
	{
		found = ++selector_count_;
		handler_selectors_.set(&handler, found);
	}
	return found;
}

lowir::SymbolId FunctionLowering::handler_type_information(const sema::Type* type)
{
	// The type information a clause or throw names: the runtime's own for
	// a fundamental type and a pointer to one (cv-qualified, the course's
	// form), else the program's.
	return lowering_.exception_type_information(type);
}

bool FunctionLowering::emit_clauses(const syntax::TryStmt& stmt)
{
	// Each handler's clause with its selector: `eh_catch` of the caught
	// type (a reference's referenced type, cv-unqualified), or
	// `eh_catch_all` for `...` (the last handler).
	for (const syntax::Handler& handler : stmt.handlers)
	{
		const syntax::ExceptionDeclaration& exception = handler.exception;
		const bool all = exception.ellipsis || exception.type == nullptr;
		lowir::Instruction inst = lowir::make_instruction(
			all ? lowir::Opcode::EhCatchAll : lowir::Opcode::EhCatch, lowir::Type::scalar(lowir::TypeKind::Void));
		inst.immediate = handler_selector(handler);
		inst.flags |= lowir::IF_SELECTOR;
		if (!all)
		{
			const sema::Type* caught = exception.type->is_reference() ? exception.type->element : exception.type;
			inst.ops[0] = lowir::Value::symbol(handler_type_information(caught->unqualified));
		}
		emit(inst);
		if (all)
			return true;
	}
	return false;
}

void FunctionLowering::emit_try_clauses(const syntax::TryStmt& stmt, uint32_t chain_first, uint32_t chain_count)
{
	// 15.3p2: an exception the statement's handlers miss goes on to the
	// enclosing tries' handlers, so its landing advertises their clauses
	// too, up to one with `...`.
	if (emit_clauses(stmt))
		return;
	for (uint32_t i = 0; i < chain_count; ++i)
	{
		if (emit_clauses(*clause_chains_[chain_first + i].statement))
			return;
	}
}

// ------------------------------------------------------------- statements

bool FunctionLowering::catches_all(const syntax::TryStmt& stmt)
{
	for (const syntax::Handler& handler : stmt.handlers)
	{
		if (handler.exception.ellipsis || handler.exception.type == nullptr)
			return true;
	}
	return false;
}

void FunctionLowering::try_statement(const syntax::TryStmt& stmt)
{
	const size_t enclosing = begin_try(stmt);
	compound(*stmt.body);
	end_try(enclosing);
}

size_t FunctionLowering::begin_function_try()
{
	// The function-try-block's statement: the body and its handlers.
	rethrow_handlers_ = function_->constructor || function_->destructor;
	return begin_try(*function_->body->try_block);
}

size_t FunctionLowering::begin_try(const syntax::TryStmt& stmt)
{
	// 15.3: the block in the statement's region; the dispatch, then each
	// handler in turn (body, its catch's landing, the next test); the
	// statement's end. Inside another try, one without a `...` handler
	// dispatches on the enclosing statements' clauses too (up to one with
	// `...`): an exception its handlers do not take goes on to the
	// enclosing statement's test with its selector.
	EhContext context = EhContext();
	context.is_try = true;
	context.floor = cleanups_.size();
	context.statement = &stmt;
	context.dispatch = new_block(lowir::BlockKind::CatchDispatch);
	context.entry = new_block(lowir::BlockKind::CatchEntry);
	context.end = new_block(lowir::BlockKind::TryEnd);
	context.chain[0] = context.chain[1] = 0;
	context.landing = lowir::kNoId;
	context.stack = ++eh_stack_count_;
	context.chain_stack = 0;
	context.chain_first = static_cast<uint32_t>(clause_chains_.size());
	context.chain_count = 0;
	context.key = &stmt;
	context.unwinds = cleanups_.size() > region_floor() ||
		eh_contexts_.size() > (innermost_try_ != SIZE_MAX ? innermost_try_ + 1 : 0) ||
		(body_target_ != lowir::kNoId && eh_contexts_.size() >= body_depth_);
	if (!catches_all(stmt))
	{
		for (size_t c = eh_contexts_.size(); c-- > 0;)
		{
			const EhContext& outer = eh_contexts_[c];
			if (!outer.is_try)
				continue;
			clause_chains_.push_back(ClauseSet{ outer.statement });
			++context.chain_count;
			if (catches_all(*outer.statement))
				break;
		}
	}
	emit_marker(lowir::Opcode::EhTry, context.dispatch);
	const size_t enclosing = innermost_try_;
	save_chain_memos();
	eh_contexts_.push_back(context);
	innermost_try_ = eh_contexts_.size() - 1;
	return enclosing;
}

void FunctionLowering::end_try(size_t enclosing)
{
	const EhContext context = eh_contexts_.back();
	const syntax::TryStmt& stmt = *context.statement;
	if (!terminated_)
	{
		emit_marker(lowir::Opcode::EhEnd, lowir::kNoId);
		jump(context.end);
	}
	eh_contexts_.pop_back();
	restore_chain_memos();
	innermost_try_ = enclosing;
	place(context.dispatch);
	emit_try_clauses(stmt, context.chain_first, context.chain_count);
	// The objects and handlers between the statement and the enclosing one
	// a miss unwinds: a cleanup clause, its record retired at once.
	if (context.unwinds && !catches_all(stmt))
	{
		emit_marker(lowir::Opcode::EhCleanup, lowir::kNoId);
		emit_marker(lowir::Opcode::EhEnd, lowir::kNoId);
	}
	jump(context.entry);
	place(context.entry);
	for (uint32_t i = 0; i < stmt.handlers.size; ++i)
	{
		const lowir::Type pointer = lowir::Type::scalar(lowir::TypeKind::Ptr);
		const lowir::Type i32 = lowir::Type::scalar(lowir::TypeKind::I32);
		RValue exception;
		exception.value = emit_value(lowir::make_instruction(lowir::Opcode::Exception, pointer), pointer);
		exception.type = pointer;
		// The selector temporary is the model's i64 (as a comparison's truth
		// value); its clause numbers compare as i32 (the course's form).
		const RValue selector = RValue{ emit_value(lowir::make_instruction(lowir::Opcode::ExceptionSelector, i32),
											lowir::Type::scalar(lowir::TypeKind::I64)),
			i32 };
		const lowir::BlockId body = new_block(lowir::BlockKind::CatchBody);
		const lowir::BlockId next = new_block(lowir::BlockKind::CatchNext);
		const lowir::BlockId landing = new_block(lowir::BlockKind::CatchCleanup);
		branch(compare(lowir::ComparePredicate::Eq, i32, selector, integer(i32, handler_selector(stmt.handlers[i]))),
			body, next);
		place(body);
		handler(stmt, stmt.handlers[i], exception, landing, context);
		place(landing);
		handler_landing(context);
		place(next);
	}
	unmatched(context);
	place(context.end);
}

void FunctionLowering::handler(const syntax::TryStmt& stmt, const syntax::Handler& written, const RValue& exception,
	lowir::BlockId landing, const EhContext& attempt)
{
	// 15.3p16: the catch begins (__cxa_begin_catch yields the adjusted
	// exception object, a pointer's value for a pointer) and its variable
	// is initialized from it, under the handler's landing; the body; the
	// handler's normal exit ends the catch (15.1p4), which destroys the
	// exception object when no other handler holds it: with a caught class
	// type whose destructor may throw, in a region (throwing_end_catch).
	const lowir::Type pointer = lowir::Type::scalar(lowir::TypeKind::Ptr);
	const size_t base = arguments_.size();
	arguments_.push_back(exception.value);
	++lowering_.counters().instructions;
	RValue caught;
	caught.value = lowir::Value::temp(
		builder_.call(pointer, lowir::Value::symbol(lowering_.runtime_function(RuntimeFunction::BeginCatch)),
			arguments_.data() + base, 1, lowir::kNoId, 0));
	caught.type = pointer;
	arguments_.resize(base);
	const syntax::ExceptionDeclaration& declaration = written.exception;
	const sema::VariableEntity* variable = declaration.declared != nullptr
		? static_cast<const sema::VariableEntity*>(declaration.declared->entity)
		: nullptr;
	Place record = Place();
	if (variable != nullptr)
	{
		record.storage = lowir::Value::slot(generated_slot(lowir::SlotKind::Catch, pointer));
		store(pointer, caught, record);
	}
	emit_marker(lowir::Opcode::EhCleanup, landing);
	EhContext context = EhContext();
	context.is_try = false;
	context.floor = cleanups_.size();
	context.statement = &stmt;
	context.dispatch = attempt.dispatch;
	context.entry = attempt.entry;
	context.end = attempt.end;
	context.chain[0] = context.chain[1] = 0;
	context.landing = landing;
	context.key = &written;
	context.stack = ++eh_stack_count_;
	context.chain_stack = 0;
	context.chain_first = context.chain_count = 0;
	const sema::Type* type = declaration.ellipsis || declaration.type == nullptr ? nullptr
		: declaration.type->is_reference()                                       ? declaration.type->element
																				 : declaration.type;
	context.end_throws = type != nullptr && destructor_may_throw(type);
	if (type != nullptr)
		lowering_.caught_type(type);
	save_chain_memos();
	eh_contexts_.push_back(context);
	const size_t depth = cleanups_.size();
	if (variable != nullptr)
		catch_variable(variable, caught, record, written.body->location);
	compound(*written.body);
	leave_scope(depth);
	// 15.3p15: a constructor's or destructor's function-try-block handler
	// rethrows when control reaches its end.
	if (!terminated_ && rethrow_handlers_ && &stmt == function_->body->try_block)
		throw_statement(nullptr, written.body->location);
	if (!terminated_ && context.end_throws)
	{
		emit_marker(lowir::Opcode::EhEnd, lowir::kNoId);
		throwing_end_catch(eh_contexts_.size() - 1, cleanups_.size());
		jump(attempt.end);
		if (!chain_blocks_.empty())
			place_chain_blocks();
	}
	else if (!terminated_)
	{
		emit_marker(lowir::Opcode::EhEnd, lowir::kNoId);
		call_end_catch();
		jump(attempt.end);
	}
	eh_contexts_.pop_back();
	restore_chain_memos();
}

void FunctionLowering::save_chain_memos()
{
	// Before a context is pushed: the chain blocks made so far under the
	// current stack (the objects', the contexts', the resume block).
	chain_memo_marks_.push_back(chain_memos_.size());
	chain_memos_.push_back(
		ChainMemo{ { eh_resume_block_, 0 }, eh_resume_stack_, lexical_resume_, lexical_resume_stack_ });
	for (const Cleanup& cleanup : cleanups_)
		chain_memos_.push_back(ChainMemo{
			{ cleanup.eh_marked, cleanup.eh_unmarked }, cleanup.eh_stack, cleanup.lexical, cleanup.lexical_stack });
	for (const EhContext& context : eh_contexts_)
		chain_memos_.push_back(ChainMemo{
			{ context.chain[0], context.chain[1] }, context.chain_stack, context.lexical, context.lexical_stack });
	chain_memos_.push_back(ChainMemo{ { static_cast<lowir::BlockId>(cleanups_.size()), 0 }, 0, 0, 0 });
}

void FunctionLowering::restore_chain_memos()
{
	// After the context pops: the objects and contexts alive before it (the
	// same ones, its own objects gone) get their blocks back.
	const size_t mark = chain_memo_marks_.back();
	chain_memo_marks_.pop_back();
	const size_t objects = chain_memos_.back().blocks[0];
	eh_resume_block_ = chain_memos_[mark].blocks[0];
	eh_resume_stack_ = chain_memos_[mark].stack;
	lexical_resume_ = chain_memos_[mark].lexical;
	lexical_resume_stack_ = chain_memos_[mark].lexical_stack;
	for (size_t i = 0; i < objects && i < cleanups_.size(); ++i)
	{
		const ChainMemo& memo = chain_memos_[mark + 1 + i];
		cleanups_[i].eh_marked = memo.blocks[0];
		cleanups_[i].eh_unmarked = memo.blocks[1];
		cleanups_[i].eh_stack = memo.stack;
		cleanups_[i].lexical = memo.lexical;
		cleanups_[i].lexical_stack = memo.lexical_stack;
	}
	for (size_t c = 0; c < eh_contexts_.size() && mark + 1 + objects + c < chain_memos_.size() - 1; ++c)
	{
		const ChainMemo& memo = chain_memos_[mark + 1 + objects + c];
		eh_contexts_[c].chain[0] = memo.blocks[0];
		eh_contexts_[c].chain[1] = memo.blocks[1];
		eh_contexts_[c].chain_stack = memo.stack;
		eh_contexts_[c].lexical = memo.lexical;
		eh_contexts_[c].lexical_stack = memo.lexical_stack;
	}
	chain_memos_.resize(mark);
}

void FunctionLowering::catch_variable(
	const sema::VariableEntity* variable, const RValue& caught, const Place& record, uint32_t location)
{
	// The handler's variable in a slot of its own: a reference bound to the
	// exception object, a pointer the yielded value, another scalar read
	// from the object, a class object copy-initialized from it. `record`
	// holds the yielded value (handler).
	const sema::Type* type = variable->type;
	const bool reference = type->is_reference();
	Local local;
	local.reference = reference;
	local.address = lowir::Value();
	local.depth = 0;
	const lowir::Type storage = reference ? lowir::Type::scalar(lowir::TypeKind::Ptr) : lowering_.storage_type(type);
	const char* spelling = variable->name != nullptr ? variable->name->spelling : nullptr;
	local.slot = new_slot(
		spelling != nullptr ? lowering_.intern(spelling, static_cast<uint32_t>(std::strlen(spelling))) : 0, storage);
	Place place = Place();
	place.storage = lowir::Value::slot(local.slot);
	local_index_.set(variable, static_cast<uint32_t>(locals_.size()));
	locals_.push_back(local);
	const size_t index = locals_.size() - 1;
	if (reference && type->element->unqualified->kind == sema::TypeKind::Pointer)
	{
		// The yielded value of a pointer handler is the adjusted pointer,
		// not the exception object's address (15.3p3: a pointer is caught
		// by value; Itanium 2.5.3): a reference to a pointer binds to a
		// copy of it, as g++ and clang++ do (changing it does not change
		// the exception object, which 15.3p17 would for an exact match),
		// the handler's record of the yielded value written again (the
		// course's form).
		const lowir::Type pointer = lowir::Type::scalar(lowir::TypeKind::Ptr);
		store(pointer, caught, record);
		RValue address;
		address.value = address_of(record);
		address.type = pointer;
		store(storage, address, place);
		return;
	}
	if (reference || type->unqualified->kind == sema::TypeKind::Pointer)
	{
		store(storage, caught, place);
		return;
	}
	const sema::ClassEntity* entity = object_class(type);
	if (entity == nullptr)
	{
		Place object = Place();
		object.storage = caught.value;
		store(storage, load(storage, object), place);
		return;
	}
	// Host objects: the copy's default arguments' temporaries (a by-value
	// parameter the caller destroys) end with the initialization (1.9p10),
	// before the handler's statements (the course keeps them to its end).
	const size_t first = cleanups_.size();
	const bool host = lowering_.host_objects();
	if (variable->constructor != nullptr && call_may_throw(variable->constructor) && host)
	{
		// 15.1p7, 15.3p7: the handler is active once its variable is
		// initialized; a copy constructor exiting by an exception before
		// calls std::terminate (15.5.1p1): in a host object its own region
		// (no other opens, nor does a temporary split it). The course's
		// copy has none, nor any region.
		const lowir::BlockId guard = new_block(lowir::BlockKind::LexicalCleanupTerminate);
		const lowir::BlockId next = new_block(lowir::BlockKind::CallUnwindEnd);
		emit_marker(lowir::Opcode::EhTry, guard);
		region_open_ = true;
		explicit_regions_ += host ? 1 : 0;
		construct_from(variable->constructor, address_of(place), caught.value, location);
		explicit_regions_ -= host ? 1 : 0;
		region_open_ = false;
		emit_marker(lowir::Opcode::EhEnd, lowir::kNoId);
		jump(next);
		this->place(guard);
		emit_terminate();
		this->place(next);
	}
	else if (variable->constructor != nullptr)
	{
		++explicit_regions_;
		construct_from(variable->constructor, address_of(place), caught.value, location);
		--explicit_regions_;
	}
	if (host)
		destroy_temporaries(first);
	if (needs_destruction(type))
		cleanups_.push_back(Cleanup{ place.storage, type, variable, lowir::kNoId, lowir::kNoId, lowir::kNoId, false });
	locals_[index].depth = cleanups_.size();
}

void FunctionLowering::call_end_catch()
{
	++lowering_.counters().instructions;
	builder_.call(lowir::Type::scalar(lowir::TypeKind::Void),
		lowir::Value::symbol(lowering_.runtime_function(RuntimeFunction::EndCatch)), nullptr, 0, lowir::kNoId, 0);
}

void FunctionLowering::throwing_end_catch(size_t contexts, size_t count)
{
	// 15.2p1: the exception object's destructor exiting by an exception
	// leaves the handler as a throwing call there would, its records
	// popped: the end runs in a region whose landing is the unwinding chain
	// of the objects alive inside the contexts enclosing the handler (seen
	// alone while it is made). None opens when no object is alive above
	// the innermost enclosing try's floor: that try's dispatch, or an
	// enclosing handler's landing, takes the exception. Its blocks wait for
	// the caller's placement.
	std::vector<EhContext> inner(eh_contexts_.begin() + static_cast<std::ptrdiff_t>(contexts), eh_contexts_.end());
	const size_t innermost = innermost_try_;
	eh_contexts_.resize(contexts);
	innermost_try_ = SIZE_MAX;
	for (size_t c = contexts; c-- > 0 && innermost_try_ == SIZE_MAX;)
	{
		if (eh_contexts_[c].is_try)
			innermost_try_ = c;
	}
	const size_t floor = innermost_try_ != SIZE_MAX ? eh_contexts_[innermost_try_].floor : 0;
	lowir::BlockId entry = lowir::kNoId;
	if (count > floor)
		entry = eh_contexts_.empty() ? unwind_entry(count) : region_landing(count);
	eh_contexts_.insert(eh_contexts_.end(), inner.begin(), inner.end());
	innermost_try_ = innermost;
	if (entry != lowir::kNoId)
		emit_marker(lowir::Opcode::EhTry, entry);
	call_end_catch();
	if (entry != lowir::kNoId)
		emit_marker(lowir::Opcode::EhEnd, lowir::kNoId);
}

void FunctionLowering::handler_landing(const EhContext& attempt)
{
	// An exception leaving the handler's own code (outside every region):
	// the enclosing statements' clauses, the catch ended and the handler's
	// record popped, then the path out.
	(void)attempt;
	const EhContext* outer = innermost_try();
	if (outer != nullptr)
		emit_try_clauses(*outer->statement, outer->chain_first, outer->chain_count);
	// A detached body's landing states its cleanup (its record popped at
	// the cleanup entry), inside its tries too.
	if (body_target_ != lowir::kNoId)
		emit_marker(lowir::Opcode::EhCleanup, lowir::kNoId);
	call_end_catch();
	emit_marker(lowir::Opcode::EhEnd, lowir::kNoId);
	leave_contexts(0);
}

void FunctionLowering::unmatched(const EhContext& attempt)
{
	// No handler takes the exception: the path out of the statement with
	// its marker pushed (a statement with a `...` handler never gets here:
	// the course jumps to the enclosing statement's test, or resumes).
	if (catches_all(*attempt.statement))
	{
		// The impossible miss of a `...` handler leaves every enclosing
		// context's region too (the course's form).
		for (size_t c = 0; c < eh_contexts_.size(); ++c)
			emit_marker(lowir::Opcode::EhEnd, lowir::kNoId);
		emit_resume(true);
		return;
	}
	leave_contexts(0);
}

void FunctionLowering::leave_contexts(uint32_t extra)
{
	// An unwinding path from here with `extra` records pushed above the
	// contexts': the objects destroyed and each handler left (its catch
	// ended, its records popped) up to the innermost try, whose entry it
	// reaches with the try's region popped and its marker's record kept
	// (the entry's state from the try's dispatch); out of the function
	// without one.
	size_t top = cleanups_.size();
	size_t c = eh_contexts_.size();
	for (; c > 0 && !eh_contexts_[c - 1].is_try; --c)
	{
		const EhContext& context = eh_contexts_[c - 1];
		for (size_t i = top; i-- > context.floor;)
			destroy_local(cleanups_[i], lowir::kNoId, true);
		top = context.floor;
		for (; extra > 0; --extra)
			emit_marker(lowir::Opcode::EhEnd, lowir::kNoId);
		call_end_catch();
		emit_marker(lowir::Opcode::EhEnd, lowir::kNoId);
	}
	const size_t floor = c > 0 ? eh_contexts_[c - 1].floor : 0;
	for (size_t i = top; i-- > floor;)
		destroy_local(cleanups_[i], lowir::kNoId, true);
	// A detached body's unwinding reaches its subobjects' cleanup entry.
	const lowir::BlockId body = body_unwind_target();
	if (body != lowir::kNoId)
	{
		for (; extra > 0; --extra)
			emit_marker(lowir::Opcode::EhEnd, lowir::kNoId);
		emit_marker(lowir::Opcode::EhEnd, lowir::kNoId);
		jump(body);
		return;
	}
	if (c == 0)
	{
		for (; extra > 0; --extra)
			emit_marker(lowir::Opcode::EhEnd, lowir::kNoId);
		emit_resume();
		return;
	}
	const EhContext& attempt = eh_contexts_[c - 1];
	// A constructor's or destructor's function-try-block with its
	// subobjects' cleanup record pushed inside it: that record's landing
	// destroys them first, its resume entering the try's (15.3p11, as the
	// chains' TryEntry blocks).
	if (function_try_subobjects(c - 1))
	{
		for (; extra > 0; --extra)
			emit_marker(lowir::Opcode::EhEnd, lowir::kNoId);
		terminate(lowir::make_instruction(lowir::Opcode::Resume, lowir::Type::scalar(lowir::TypeKind::Void)));
		return;
	}
	for (uint32_t pops = extra + 1; pops > 0; --pops)
		emit_marker(lowir::Opcode::EhEnd, lowir::kNoId);
	jump(attempt.entry);
}

const FunctionLowering::EhContext* FunctionLowering::innermost_try() const
{
	return innermost_try_ != SIZE_MAX ? &eh_contexts_[innermost_try_] : nullptr;
}

bool FunctionLowering::function_try_subobjects(size_t context) const
{
	return context == 0 && function_try_block() &&
		((function_->constructor && constructor_region_) || (function_->destructor && destructor_region_));
}

size_t FunctionLowering::region_floor() const
{
	const EhContext* attempt = innermost_try();
	return attempt != nullptr ? attempt->floor : 0;
}

void FunctionLowering::exit_context(size_t index)
{
	// A normal exit from a context: a try's region ends, a handler's
	// record pops and its catch ends (the objects above were destroyed;
	// see handler for one whose end may throw).
	emit_marker(lowir::Opcode::EhEnd, lowir::kNoId);
	if (eh_contexts_[index].is_try)
		return;
	if (eh_contexts_[index].end_throws)
	{
		throwing_end_catch(index, eh_contexts_[index].floor);
		return;
	}
	call_end_catch();
}

// ------------------------------------------------------------ unwinding

lowir::BlockId FunctionLowering::eh_unwind_entry(size_t count)
{
	// The chain from a landing at the first `count` objects inside the
	// contexts: from the innermost try's floor (or the function's bottom)
	// up, each handler's exit where it began, each object; blocks keyed by
	// whether the landing's own record is still pushed (its marker inside a
	// try, else its active try record): it is above the innermost handler's
	// exit, which pops it, and popped below it. The elements are visited
	// from the top down to the first whose block exists (the blocks below
	// it were made with it), then the missing ones made bottom-up.
	const EhContext* attempt = innermost_try();
	const size_t floor = attempt != nullptr ? attempt->floor : 0;
	const size_t first_handler = attempt != nullptr ? innermost_try_ + 1 : 0;
	const int marker = 1;
	const uint32_t stack = eh_contexts_.back().stack;
	const auto node = [this, stack](const EhElement& element) -> lowir::BlockId& {
		if (element.context != SIZE_MAX)
		{
			EhContext& context = eh_contexts_[element.context];
			if (context.chain_stack != stack)
			{
				context.chain[0] = context.chain[1] = 0;
				context.chain_stack = stack;
			}
			return context.chain[element.key];
		}
		Cleanup& cleanup = cleanups_[element.object];
		if (cleanup.eh_stack != stack)
		{
			cleanup.eh_marked = cleanup.eh_unmarked = 0;
			cleanup.eh_stack = stack;
		}
		return element.key != 0 ? cleanup.eh_marked : cleanup.eh_unmarked;
	};
	// Top-down: at each position the object there, then the handlers that
	// began there, innermost first.
	std::vector<EhElement>& missing = eh_elements_;
	missing.clear();
	int key = marker;
	size_t handler = eh_contexts_.size();
	lowir::BlockId tail = 0;
	for (size_t i = count + 1; i-- > floor && tail == 0;)
	{
		if (i < count)
		{
			const EhElement element = EhElement{ i, SIZE_MAX, key };
			tail = node(element);
			if (tail == 0)
				missing.push_back(element);
		}
		for (; tail == 0 && handler > first_handler && eh_contexts_[handler - 1].floor == i; --handler)
		{
			const EhElement element = EhElement{ SIZE_MAX, handler - 1, key };
			tail = node(element);
			if (tail == 0)
				missing.push_back(element);
			key = 0;
		}
	}
	if (tail == 0)
	{
		// The terminal: a detached body's cleanup entry, the try's entry
		// (its region popped, and the landing's marker unless a handler's
		// exit popped it) or resume.
		if (body_unwind_target() != lowir::kNoId)
			tail = body_terminal(key);
		else if (attempt != nullptr)
		{
			EhContext& owner = eh_contexts_[innermost_try_];
			if (owner.chain_stack != stack)
			{
				owner.chain[0] = owner.chain[1] = 0;
				owner.chain_stack = stack;
			}
			lowir::BlockId& made = owner.chain[key];
			if (made == 0)
			{
				made = new_block(lowir::BlockKind::CleanupResume);
				ChainBlock block = ChainBlock();
				block.block = made;
				block.terminal = true;
				block.unwinding = true;
				block.action = ChainAction::TryEntry;
				block.pops = static_cast<uint32_t>(key + 1);
				block.tail = owner.entry;
				// A constructor's or destructor's subobjects' cleanup
				// record, pushed inside its function-try-block's region,
				// destroys them before the handler (15.3p11): the
				// landing's marker popped, the unwinding resumes into it,
				// and its resume into the try's.
				if (function_try_subobjects())
				{
					block.pops = static_cast<uint32_t>(key);
					block.tail = lowir::kNoId;
				}
				chain_blocks_.push_back(block);
			}
			tail = made;
		}
		else
		{
			if (eh_resume_block_ == lowir::kNoId || eh_resume_stack_ != stack)
			{
				eh_resume_block_ = new_block(lowir::BlockKind::CleanupResume);
				eh_resume_stack_ = stack;
				ChainBlock block = ChainBlock();
				block.block = eh_resume_block_;
				block.terminal = true;
				block.unwinding = true;
				chain_blocks_.push_back(block);
			}
			tail = eh_resume_block_;
		}
	}
	for (size_t e = missing.size(); e-- > 0;)
	{
		const EhElement element = missing[e];
		const lowir::BlockId block = new_block(element.context == SIZE_MAX
				? unwind_block_kind(cleanups_[element.object])
				: lowir::BlockKind::CleanupAction);
		ChainBlock made = ChainBlock();
		made.block = block;
		made.unwinding = true;
		made.tail = tail;
		made.below = lowir::kNoId;
		if (element.context != SIZE_MAX)
		{
			made.action = ChainAction::HandlerExit;
			made.pops = (element.key != 0 ? 1 : 0) + 1;
		}
		else
			made.cleanup = cleanups_[element.object];
		chain_blocks_.push_back(made);
		node(element) = block;
		tail = block;
	}
	return tail;
}

lowir::BlockId FunctionLowering::region_landing(size_t count)
{
	// A region's handler inside a try block or handler: a dispatch block
	// with the innermost try's clauses and a marker (none outside a try),
	// jumping to the chain; one per chain entry (a lone handler's exit
	// with nothing to destroy too).
	const lowir::BlockId entry = eh_unwind_entry(count);
	const EhContext* attempt = innermost_try();
	if (landing_dispatch_.size() <= entry)
		landing_dispatch_.resize(entry + 1, 0);
	if (landing_dispatch_[entry] != 0)
		return landing_dispatch_[entry] - 1;
	const lowir::BlockId dispatch = new_block(lowir::BlockKind::CallUnwindDispatch);
	ChainBlock block = ChainBlock();
	block.block = dispatch;
	block.unwinding = true;
	block.action = ChainAction::Dispatch;
	block.tail = entry;
	block.statement = attempt != nullptr ? attempt->statement : nullptr;
	block.chain_first = attempt != nullptr ? attempt->chain_first : 0;
	block.chain_count = attempt != nullptr ? attempt->chain_count : 0;
	chain_blocks_.push_back(block);
	landing_dispatch_[entry] = dispatch + 1;
	return dispatch;
}

lowir::BlockId FunctionLowering::result_landing(size_t depth)
{
	// The result's destruction runs under a terminate guard when its
	// destructor may throw (destroy_local).
	const bool contexts = !eh_contexts_.empty();
	ChainBlock destroy = ChainBlock();
	destroy.block = new_block(lowir::BlockKind::CleanupAction);
	destroy.unwinding = true;
	destroy.cleanup = returned_result_;
	destroy.tail = contexts ? eh_unwind_entry(depth) : unwind_entry(depth);
	destroy.below = lowir::kNoId;
	chain_blocks_.push_back(destroy);
	const EhContext* attempt = contexts ? innermost_try() : nullptr;
	if (attempt == nullptr)
		return destroy.block;
	ChainBlock dispatch = ChainBlock();
	dispatch.block = new_block(lowir::BlockKind::CallUnwindDispatch);
	dispatch.unwinding = true;
	dispatch.action = ChainAction::Dispatch;
	dispatch.tail = destroy.block;
	dispatch.statement = attempt->statement;
	dispatch.chain_first = attempt->chain_first;
	dispatch.chain_count = attempt->chain_count;
	chain_blocks_.push_back(dispatch);
	return dispatch.block;
}

void FunctionLowering::place_eh_block(const ChainBlock& block)
{
	// A chain block of the contexts: a try's entry (its pops, the jump), a
	// handler's exit, a region's dispatch.
	switch (block.action)
	{
	case ChainAction::TryEntry:
		for (uint32_t i = 0; i < block.pops; ++i)
			emit_marker(lowir::Opcode::EhEnd, lowir::kNoId);
		if (block.tail == lowir::kNoId)
			terminate(lowir::make_instruction(lowir::Opcode::Resume, lowir::Type::scalar(lowir::TypeKind::Void)));
		else
			jump(block.tail);
		return;
	case ChainAction::HandlerExit:
		// The landing's record (when still pushed), the catch ended, the
		// handler's record.
		for (uint32_t i = 1; i < block.pops; ++i)
			emit_marker(lowir::Opcode::EhEnd, lowir::kNoId);
		call_end_catch();
		emit_marker(lowir::Opcode::EhEnd, lowir::kNoId);
		jump(block.tail);
		return;
	case ChainAction::LexicalHandlerExit:
	case ChainAction::LexicalTryEntry:
	case ChainAction::LexicalDispatch:
	case ChainAction::BodyEntry:
		place_lexical_block(block);
		return;
	case ChainAction::ReturnExit:
	case ChainAction::ReturnObject:
		place_return_block(block);
		return;
	case ChainAction::Dispatch:
		if (block.statement != nullptr)
			emit_try_clauses(*block.statement, block.chain_first, block.chain_count);
		emit_marker(lowir::Opcode::EhCleanup, lowir::kNoId);
		jump(block.tail);
		return;
	default:
		return;
	}
}

bool FunctionLowering::landing_try(EhContext& attempt) const
{
	// The try statement whose clauses an explicit landing (a new-
	// expression's element cleanup) dispatches on: the innermost context,
	// when a try and no other region encloses the landing's.
	if (region_open_ || eh_contexts_.empty() || !eh_contexts_.back().is_try)
		return false;
	attempt = eh_contexts_.back();
	return true;
}

void FunctionLowering::landing_dispatch(const EhContext& attempt)
{
	// The try's clauses and its enclosing ones, then the landing's marker
	// (a region landing's form: the try's entry dispatches on all of them,
	// 15.3p2 for each enclosing handler).
	emit_try_clauses(*attempt.statement, attempt.chain_first, attempt.chain_count);
	emit_marker(lowir::Opcode::EhCleanup, lowir::kNoId);
}

void FunctionLowering::landing_exit(const EhContext& attempt)
{
	// The unwinding reaches the try's entry, the marker popped and the try's
	// region too (a region landing's TryEntry pops).
	emit_marker(lowir::Opcode::EhEnd, lowir::kNoId);
	// (Through a function-try-block's subobjects' cleanup, see
	// leave_contexts.)
	if (function_try_subobjects())
	{
		terminate(lowir::make_instruction(lowir::Opcode::Resume, lowir::Type::scalar(lowir::TypeKind::Void)));
		return;
	}
	emit_marker(lowir::Opcode::EhEnd, lowir::kNoId);
	jump(attempt.entry);
}

// ------------------------------------------------------------------ throw

void FunctionLowering::throw_statement(const syntax::Expr* operand, uint32_t location)
{
	// 15.1: a throw-expression as the whole statement; the path ends there:
	// the course closes the open region and the enclosing try statements'
	// regions and returns zero.
	throw_object(operand, location, true);
	noreturn_exit();
}

void FunctionLowering::throw_expression(const syntax::ThrowExpr& expr)
{
	// A throw-expression inside a full-expression (5.16p2's operand, a comma
	// operand): its calls are the full-expression's; the code after it is
	// not reached, and continues in its block.
	throw_object(expr.operand, expr.location, false);
}

void FunctionLowering::throw_object(const syntax::Expr* operand, uint32_t location, bool statement)
{
	// 15.1p3: the exception object allocated and copy-initialized from the
	// operand, then thrown with its type's information and destructor;
	// `throw;` rethrows the handled exception (15.1p8). As a statement the
	// operand is a full-expression of its own, between the allocation's and
	// the throw's regions (opened before their operands).
	const lowir::Type pointer = lowir::Type::scalar(lowir::TypeKind::Ptr);
	const lowir::Type void_type = lowir::Type::scalar(lowir::TypeKind::Void);
	const bool nested = !statement && full_expression_;
	const bool expression = !statement;
	if (operand == nullptr)
	{
		// A rethrow statement where a throw's allocation would be staged
		// in a region of its own reserves that slot too (the course's
		// frame), unused.
		if (statement && !full_expression_ && !region_open_ && !chain_empty())
			generated_slot(lowir::SlotKind::ThrowAlloc, pointer);
		if (nested)
			begin_call(true);
		emit_call(void_type, lowir::Value::symbol(lowering_.runtime_function(RuntimeFunction::Rethrow)),
			arguments_.size(), lowir::kNoId, true);
		if (nested)
			end_call(true);
		if (expression)
			nested_throw_exit();
		return;
	}
	const syntax::Expr* inner = WithoutParentheses(operand);
	const sema::Conversion* conversion = inner->facts->conversion;
	const sema::Type* type = conversion != nullptr && conversion->result != nullptr ? conversion->result->unqualified
																					: inner->facts->type->unqualified;
	if (type->kind == sema::TypeKind::Array)
		type = lowering_.graph().types.pointer(type->element);
	else if (type->kind == sema::TypeKind::Function)
		type = lowering_.graph().types.pointer(type);
	if (nested)
		begin_call(true);
	size_t base = arguments_.size();
	arguments_.push_back(integer(lowir::Type::scalar(lowir::TypeKind::I64), lowering_.size_of(type, location)).value);
	lowir::Value object;
	if (expression)
	{
		// Inside a full-expression the throw's calls run in its region
		// (opened before them when objects are alive), the allocation's
		// result unspilled; a staged throw reserves its slot (unused, the
		// course's frame).
		if (nested)
			generated_slot(lowir::SlotKind::ThrowAlloc, pointer);
		if (nested)
			before_throwing_call();
		if (terminated_)
			place(new_block(lowir::BlockKind::Unreachable));
		++lowering_.counters().instructions;
		object = lowir::Value::temp(
			builder_.call(pointer, lowir::Value::symbol(lowering_.runtime_function(RuntimeFunction::AllocateException)),
				arguments_.data() + base, 1, lowir::kNoId, 0));
		arguments_.resize(base);
	}
	else
		object = throw_allocation(base, statement);
	if (nested)
		end_call(true);
	exception_object(*operand, type, object, statement, location);
	if (nested)
		begin_call(true);
	else if (statement && !full_expression_)
		open_region(false);
	base = arguments_.size();
	arguments_.push_back(object);
	arguments_.push_back(address_of_symbol(lowering_.exception_type_information(type)));
	lowering_.thrown_type(type);
	const sema::ClassEntity* entity = object_class(type);
	if (entity != nullptr && entity->destructor != nullptr && !entity->destructor->trivial)
	{
		lowering_.demand(entity->destructor);
		arguments_.push_back(address_of_symbol(lowering_.entry_symbol(entity->destructor, EntryVariant::Complete)));
	}
	else
		arguments_.push_back(integer(pointer, 0).value);
	emit_call(
		void_type, lowir::Value::symbol(lowering_.runtime_function(RuntimeFunction::Throw)), base, lowir::kNoId, true);
	if (nested)
		end_call(true);
	if (expression)
		nested_throw_exit();
}

void FunctionLowering::nested_throw_exit()
{
	// After a throw inside a full-expression the path ends: an innermost
	// try's region is left, then the function's zero return (the course's
	// no-return continuation); the full-expression's open region ends with
	// the block.
	if (!eh_contexts_.empty() && eh_contexts_.back().is_try)
		emit_marker(lowir::Opcode::EhEnd, lowir::kNoId);
	const bool segment = region_segment_;
	region_segment_ = false;
	zero_return();
	region_segment_ = segment;
	region_open_ = false;
	eager_pending_ = false;
}

void FunctionLowering::exception_object(
	const syntax::Expr& thrown, const sema::Type* type, lowir::Value object, bool statement, uint32_t location)
{
	// The operand copy-initializing the exception object (a class object's
	// prvalue initializing it directly); a statement's operand is a
	// full-expression, its temporaries destroyed before the throw.
	const size_t first = cleanups_.size();
	const bool began = statement && begin_full_expression(&thrown);
	const syntax::Expr* saved = statement ? full_root(&thrown) : nullptr;
	if (object_class(type) != nullptr)
		initialize_from(thrown, WithoutParentheses(&thrown)->facts->conversion, object, location);
	else
	{
		Place place = Place();
		place.storage = object;
		store(lowering_.storage_type(type), fit(operand(thrown, ConversionContext::Initialization), type), place);
	}
	if (!statement)
		return;
	choice_root_ = saved;
	destroy_temporaries(first);
	end_full_expression(began);
}

lowir::Value FunctionLowering::throw_allocation(size_t base, bool statement)
{
	// __cxa_allocate_exception; a statement's, when objects are alive, in a
	// region of its own, its result kept in a slot (the course's form).
	const lowir::Type pointer = lowir::Type::scalar(lowir::TypeKind::Ptr);
	const lowir::Value callee = lowir::Value::symbol(lowering_.runtime_function(RuntimeFunction::AllocateException));
	const bool region = statement && !full_expression_ && !region_open_ && !chain_empty();
	if (!region)
		return emit_call(pointer, callee, base, lowir::kNoId, true);
	open_region(false);
	RValue value;
	value.value = emit_call(pointer, callee, base, lowir::kNoId, true);
	value.type = pointer;
	Place spill = Place();
	spill.storage = lowir::Value::slot(generated_slot(lowir::SlotKind::ThrowAlloc, pointer));
	store(pointer, value, spill);
	region_end_kind_ = lowir::BlockKind::ThrowAllocUnwindEnd;
	close_region();
	region_end_kind_ = lowir::BlockKind::CallUnwindEnd;
	return load(pointer, spill).value;
}

void FunctionLowering::noreturn_exit()
{
	// After a call that does not return: the open region and each enclosing
	// try statement's region end, then the function's zero return (the
	// course's unreachable continuation).
	if (region_open_)
	{
		region_open_ = false;
		eager_pending_ = false;
		emit_marker(lowir::Opcode::EhEnd, lowir::kNoId);
	}
	// (The tries inside the innermost handler only: a handler's own
	// record stays, the course's form.)
	for (size_t c = eh_contexts_.size(); c-- > 0 && eh_contexts_[c].is_try;)
		emit_marker(lowir::Opcode::EhEnd, lowir::kNoId);
	zero_return();
}

void FunctionLowering::zero_return(bool fallback)
{
	// The function's zero result: an object's from a zeroed slot (after a
	// full-expression's region ends, as terminate ends it): the result
	// slot of the returns at a dead end, else (the continuation after a
	// call that does not return) a slot of its own, made with the
	// function's last slots (the course's frame; patch_fallback_slots).
	lowir::Instruction inst = lowir::make_instruction(lowir::Opcode::Return, result_type_);
	if (result_type_.kind() == lowir::TypeKind::Obj)
	{
		if (region_segment_)
			close_region();
		if (!fallback && return_slot_ == lowir::kNoId)
			return_slot_ = generated_slot(lowir::SlotKind::Retobj, result_type_);
		const lowir::SlotId slot = fallback ? 0 : return_slot_;
		lowir::Instruction zero = lowir::make_instruction(
			lowir::Opcode::ZeroInit, lowir::Type::scalar(lowir::TypeKind::Void));
		zero.ops[0] = lowir::Value::slot(slot);
		zero.immediate = static_cast<int64_t>(result_type_.bytes);
		zero.sub = static_cast<uint8_t>(result_type_.align_log2);
		const lowir::InstId zeroed = emit(zero);
		inst.ops[0] = lowir::Value::slot(slot);
		terminate(inst);
		if (fallback)
			fallback_patches_.push_back(FallbackPatch{ zeroed, builder_.last_instruction(builder_.insertion_block()) });
		return;
	}
	if (!result_type_.is_void())
		inst.ops[0] = zero_result();
	terminate(inst);
}

void FunctionLowering::patch_fallback_slots()
{
	// Each fallback's slot, made now (after every slot its code planned).
	lowir::FunctionBody& body = builder_.body();
	for (const FallbackPatch& patch : fallback_patches_)
	{
		const lowir::Value slot = lowir::Value::slot(generated_slot(lowir::SlotKind::Retobj, result_type_));
		body.instructions[patch.zero].ops[0] = slot;
		body.instructions[patch.ret].ops[0] = slot;
	}
	fallback_patches_.clear();
}

} // namespace lower
