#include "lower/function_lowering.h"

#include <cstring>

#include "lex/identifier_table.h"
#include "sema/conversions.h"
#include "support/stack_budget.h"

// Construction helpers of one body: blocks placed in emission order, each
// instruction appended to the insertion block with its result's LowIR type
// derived from the operation (a `cmp` is i64), and the entry points for
// definitions and the dynamic-initialization function.
namespace lower {

FunctionLowering::FunctionLowering(
	ProgramLowering& lowering, lowir::SymbolId symbol, const sema::FunctionEntity* function, EntryVariant variant)
	: lowering_(lowering), program_(lowering.program()), function_(function), this_slot_(lowir::kNoId),
	  result_destination_(), builder_(program_, symbol), debug_(lowering.debug_locations()), terminated_(false),
	  unreachable_storage_(false), bit_unit_(0), bit_high_(0), switch_first_(0), resume_block_(lowir::kNoId),
	  return_terminal_(lowir::kNoId), return_slot_(lowir::kNoId), region_open_(false), region_segment_(false),
	  full_expression_(false), eager_pending_(false), segment_block_(lowir::kNoId), segment_after_(lowir::kNoId),
	  base_entry_(variant == EntryVariant::Base), deleting_(variant == EntryVariant::Deleting),
	  constructor_region_(false), constructor_cleanup_(lowir::kNoId), destructor_region_(false),
	  destructor_epilogue_(lowir::kNoId), destructor_cleanup_(lowir::kNoId), destructor_progress_(lowir::kNoId),
	  terminate_landing_(lowir::kNoId), exception_slot_(lowir::kNoId), guard_returns_(true)
{
	lowir::Function& definition = program_.functions[program_.symbols[symbol].detail];
	result_type_ = program_.signatures[definition.signature].result;
	is_main_ = program_.symbols[symbol].metadata.role == ir_model::SR_ENTRY;
	slot_scope_ = lowering.new_slot_scope();
	// A definition written in the source is where its declaration starts;
	// its prologue (the parameters' stores) and what no construct of its
	// own locates carry that location.
	if (debug_ && function != nullptr && function->definition_location != 0)
	{
		definition.location = lowering.debug_location(function->definition_location - 1);
		builder_.set_current_location(definition.location);
	}
	if (debug_)
	{
		debug_function_ = symbol;
		lowering.debug_function(function);
	}
	place(builder_.add_block(lowering.entry_name()));
}

void FunctionLowering::closure_captures()
{
	// 5.1.2p14, p18: each entity a lambda captured is named in its body
	// through the closure's member: its copy, or (by reference) the member
	// holding its address; a captured `this` through the pointer member.
	const sema::ClassEntity* owner = function_->member && !function_->is_static && function_->owner != nullptr
		? sema::ClassOf(function_->owner->entity->type)
		: nullptr;
	const sema::ClosureFacts* facts = owner != nullptr ? lowering_.graph().closure_facts(owner) : nullptr;
	// A templated lambda's call operator specialization is its body too.
	if (facts == nullptr ||
		(facts->call != function_ && (facts->lambda == nullptr || function_->primary != facts->call)))
		return;
	if (facts->this_member != nullptr)
	{
		closure_this_ = true;
		closure_this_offset_ = lowering_.member_offset(facts->this_member, 0);
		closure_this_class_ = sema::ClassOf(facts->this_member->type->element);
	}
	for (uint32_t i = 0; i < facts->capture_count; ++i)
	{
		Local local;
		local.slot = static_cast<lowir::SlotId>(lowering_.member_offset(facts->captures[i].member, 0));
		local.reference = facts->captures[i].member->type->is_reference();
		local.captured = true;
		local.address = lowir::Value();
		local.depth = 0;
		local_index_.set(facts->captures[i].entity, static_cast<uint32_t>(locals_.size()));
		locals_.push_back(local);
	}
}

void FunctionLowering::begin_unit()
{
	// Entities of the previous unit are gone; its locals were temporaries.
	local_index_.clear();
	locals_.clear();
	labels_.clear();
}

// ---------------------------------------------------------------- entries

void FunctionLowering::lower_definition()
{
	// An implicit (or defaulted) constructor or destructor has no body of
	// its own: its subobject initializations or destructions are its code.
	const syntax::FunctionBody* body = function_->body;
	// 15.5.1: a non-throwing function whose body may throw runs in a
	// region whose handler calls std::terminate (a deleting entry calling
	// the complete one has no code of its own: that entry has the region);
	// in a host object, one with a dynamic-exception-specification (15.4p9,
	// `throw()` too) in one whose handler calls std::unexpected (15.5.2)
	// through __cxa_call_unexpected, as GCC and Clang guard it.
	const sema::SemanticGraph::ThrowSpecification* specification = lowering_.host_objects()
		? lowering_.graph().throw_specification(function_)
		: nullptr;
	if ((specification != nullptr || lowering_.function_nothrow(function_)) && definition_may_throw() &&
		(!deleting_ || deleting_inlines_body(body != nullptr ? body->body->location : 0)))
	{
		unexpected_specification_ = specification;
		terminate_landing_ = new_block(lowir::BlockKind::FunctionExceptionLanding);
		emit_marker(lowir::Opcode::EhTry, terminate_landing_);
	}
	// 8.3.5: each parameter is an object of the function's frame: its
	// incoming value stored in a slot of its own; an indirect result's
	// destination first, then `this`.
	uint32_t index = 0;
	if (lowering_.indirect_result(function_))
		result_destination_ = lowir::Value::temp(builder_.parameter(index++));
	if (function_->member && !function_->is_static)
	{
		const lowir::Type pointer = lowir::Type::scalar(lowir::TypeKind::Ptr);
		this_slot_ = new_slot(lowering_.this_name(), pointer);
		RValue incoming;
		incoming.value = lowir::Value::temp(builder_.parameter(index));
		incoming.type = pointer;
		Place place = Place();
		place.storage = lowir::Value::slot(this_slot_);
		store(pointer, incoming, place);
		++index;
	}
	// An implicitly declared copy or move function's parameter is `other`
	// (as its signature names it).
	const bool implicit_copy = function_->implicit && function_->copy_kind != sema::CopyKind::None &&
		function_->type->canonical->param_count == 1;
	// A base-object entry of a class with virtual bases: its VTT next.
	const bool hidden = ProgramLowering::HiddenBaseParameters(function_,
		base_entry_     ? EntryVariant::Base
			: deleting_ ? EntryVariant::Deleting
						: EntryVariant::Complete);
	if (hidden)
		vtt_param_ = lowir::Value::temp(builder_.parameter(index++));
	for (const sema::Binding* binding = function_->scope != nullptr ? function_->scope->bindings : nullptr;
		binding != nullptr; binding = binding->next)
	{
		if (binding->kind != sema::BindingKind::Parameter)
			continue;
		const sema::Type* type = binding->type;
		Local local;
		local.reference = type->is_reference();
		const lowir::Type slot_type = local.reference
			? lowir::Type::scalar(lowir::TypeKind::Ptr)
			: lowering_.storage_type(type->kind == sema::TypeKind::Array || type->kind == sema::TypeKind::Function
					  ? lowering_.graph().types.adjusted_parameter(type)
					  : type);
		local.slot = binding->name != nullptr ? new_slot(lowering_.parameter_name(binding->name), slot_type)
			: implicit_copy                   ? new_slot(lowering_.other_name(), slot_type)
											  : parameter_slot(index, slot_type);
		local.address = lowir::Value();
		local.depth = 0;
		RValue incoming;
		incoming.value = lowir::Value::temp(builder_.parameter(index));
		incoming.type = slot_type;
		Place place = Place();
		place.storage = lowir::Value::slot(local.slot);
		const sema::ClassEntity* cls = local.reference ? nullptr : sema::ClassOf(type);
		if (cls != nullptr && !lowering_.passed_directly(cls))
		{
			// A class object passed by address is the caller's temporary
			// (its slot stays unused, the course's frame), which the caller
			// destroys (Itanium 3.1.2.3).
			local.address = incoming.value;
		}
		else if (cls != nullptr)
		{
			// A class object by value: its bytes copied into the slot (an
			// empty class has none to copy).
			if (!cls->empty)
				copy_object(type, incoming.value, address_of(place), 0);
		}
		else
			store(slot_type, incoming, place);
		local.depth = cleanups_.size();
		local_index_.set(binding->entity, static_cast<uint32_t>(locals_.size()));
		locals_.push_back(local);
		parameter_slots_.push_back(local.slot);
		++index;
	}
	if (function_->scope == nullptr)
	{
		// An implicit copy or move function's parameter and an inheriting
		// constructor's (12.9) have no declarations of their own: slots by
		// position (the first's named `other`, as its signature's).
		const sema::Type* type = function_->type->canonical;
		for (uint32_t i = 0; i < type->param_count; ++i)
		{
			const sema::Type* parameter = type->params[i];
			const lowir::Type slot_type = parameter->is_reference() ? lowir::Type::scalar(lowir::TypeKind::Ptr)
																	: lowering_.storage_type(parameter);
			const lowir::SlotId slot = implicit_copy ? new_slot(lowering_.other_name(), slot_type)
													 : parameter_slot(index, slot_type);
			RValue incoming;
			incoming.value = lowir::Value::temp(builder_.parameter(index));
			incoming.type = slot_type;
			Place place = Place();
			place.storage = lowir::Value::slot(slot);
			store(slot_type, incoming, place);
			parameter_slots_.push_back(slot);
			++index;
		}
	}
	closure_captures();
	const uint32_t location = body != nullptr ? body->body->location : 0;
	// 12.8p31: a local that every return statement returns is constructed
	// in an indirect result's destination.
	if (result_destination_.kind != lowir::ValueKind::None && body != nullptr && !function_->defaulted &&
		!body->try_block)
	{
		bool single = true;
		returned_local(*body->body, returned_local_, single);
		if (!single)
			returned_local_ = nullptr;
	}
	if (function_->copy_kind != sema::CopyKind::None && (function_->implicit || function_->synthesized))
	{
		// A defaulted copy or move member: its subobjects' copies.
		copy_members(location);
		finish_body();
		if (!lowering_.streaming())
			builder_.finish();
		return;
	}
	// A function-try-block guards a constructor's subobject initializations
	// too (15.3p4); the parameters outlive its handlers (15.3p11).
	size_t enclosing_try = SIZE_MAX;
	const bool function_try = body != nullptr && body->try_block && !deleting_;
	if (function_try && !function_->destructor)
		enclosing_try = begin_function_try();
	if (function_->constructor)
	{
		body_detached_ = detached_body();
		initialize_subobjects(location);
	}
	if (function_->destructor && lowering_.destructor_does_nothing(sema::ClassOf(function_->owner->entity->type)))
	{
		// No effect: nothing to destroy.
		finish_body();
		if (!lowering_.streaming())
			builder_.finish();
		return;
	}
	// A deleting entry (12.4p12) is the complete destructor, its code or a
	// call of its entry (the course's form when the body or a member's
	// destruction does anything), with the deallocation of the object as
	// the last subobject action.
	const bool calls_complete = deleting_ && !deleting_inlines_body(location);
	bool regions = function_->destructor && !calls_complete && destructor_regions(location);
	if (deleting_)
	{
		if (calls_complete)
			subobjects_.clear();
		subobjects_.push_back(Subobject{ nullptr, nullptr, -1, true });
		regions = true;
	}
	// A destructor's function-try-block guards its subobjects' destruction
	// too: their cleanup runs before the handler (15.3p13).
	if (function_try && function_->destructor)
		enclosing_try = begin_function_try();
	if (regions)
	{
		destructor_body(location, calls_complete);
		// The body's region ends: the function-try-block's handlers run
		// after the subobjects' destruction (15.3p11), and a return there
		// returns (15.3p15).
		if (function_try)
			end_try(enclosing_try);
		finish_body();
		place_late_blocks();
		place_terminate_landing();
		if (!lowering_.streaming())
			builder_.finish();
		return;
	}
	// A destructor here has no subobject to destroy (a union's members are
	// variant members, 12.4p8).
	if (function_->destructor && destructor_stores_tables())
		store_vptr(location);
	if (body != nullptr && !function_->defaulted)
		compound(*body->body);
	body_target_ = lowir::kNoId;
	// A constructor's function-try-block body leaves its subobjects'
	// cleanup region before the try's (the course's form).
	if (function_try && constructor_region_ && !terminated_ && cleanups_.empty())
	{
		emit_marker(lowir::Opcode::EhEnd, lowir::kNoId);
		constructor_region_ = false;
	}
	if (function_try)
		end_try(enclosing_try);
	finish_body();
	place_late_blocks();
	place_terminate_landing();
	// A body written and released at once keeps its growth slack.
	if (!lowering_.streaming())
		builder_.finish();
}

bool FunctionLowering::destructor_stores_tables() const
{
	// 12.7p4: a polymorphic class's destructor stores its tables for the
	// virtual calls made during destruction. A class dynamic only through
	// its virtual bases makes none: its destructor reaches its virtual bases
	// through the tables already stored (a base-object entry through its
	// VTT's), so it stores none (the course's form).
	return sema::ClassOf(function_->owner->entity->type)->polymorphic;
}

void FunctionLowering::destructor_body(uint32_t location, bool calls_complete)
{
	// 15.2p2: the body runs in a region whose cleanup destroys every
	// subobject; a return leaves it for the subobjects' destruction.
	const syntax::FunctionBody* body = function_->body;
	destructor_region_ = true;
	destructor_cleanup_ = new_block(lowir::BlockKind::DestructorCleanup);
	if (subobjects_.size() > 8)
	{
		const lowir::Type i64 = lowir::Type::scalar(lowir::TypeKind::I64);
		destructor_progress_ = generated_slot(lowir::SlotKind::DestructorProgress, i64);
		Place progress = Place();
		progress.storage = lowir::Value::slot(destructor_progress_);
		store(i64, integer(i64, 0), progress);
	}
	emit_marker(lowir::Opcode::EhCleanup, destructor_cleanup_);
	// A detached body unwinds to its subobjects' cleanup entry.
	body_detached_ = !calls_complete && detached_body();
	if (body_detached_)
	{
		body_target_ = new_block(lowir::BlockKind::DestructorCleanupEntry);
		body_depth_ = eh_contexts_.size();
	}
	if (calls_complete)
		call_complete_destructor(location);
	else
	{
		if (destructor_stores_tables())
			store_vptr(location);
		if (body != nullptr && !function_->defaulted)
			compound(*body->body);
	}
	body_entry_ = body_target_;
	body_target_ = lowir::kNoId;
	if (!terminated_)
	{
		emit_marker(lowir::Opcode::EhEnd, lowir::kNoId);
		if (destructor_epilogue_ != lowir::kNoId)
			jump(destructor_epilogue_);
	}
	if (destructor_epilogue_ != lowir::kNoId)
		place(destructor_epilogue_);
	destructor_epilogue(location);
	destructor_region_ = false;
}

void FunctionLowering::finish_body()
{
	finish_body_end();
	// The blocks still waiting for a placement (a lexical landing's at the
	// end) follow the body.
	if (!chain_blocks_.empty() && terminated_)
		place_chain_blocks();
	if (!lexical_pending_.empty() && terminated_)
		place_lexical_blocks();
	if (!pending_terminates_.empty() && terminated_)
		place_terminates();
	patch_fallback_slots();
}

void FunctionLowering::finish_body_end()
{
	if (terminated_)
	{
		flush_deferred();
		return;
	}
	// A constructor's last cleanup region ends with its body.
	if (constructor_region_)
		emit_marker(lowir::Opcode::EhEnd, lowir::kNoId);
	// 6.6.3p2: flowing off the end of a void function returns.
	if (result_type_.is_void())
	{
		terminate(lowir::make_instruction(lowir::Opcode::Return, result_type_));
		flush_deferred();
		return;
	}
	// 3.6.1p5: flowing off the end of main returns 0; of another non-void
	// function it is undefined (6.6.3p2): an end some branch reaches (a
	// constant loop's exit, a handler falling through) is unreachable; the
	// course returns a zero value from a dead end block (after a switch
	// whose paths all return).
	if (!is_main_)
	{
		if (!block_targeted(builder_.insertion_block()))
			return zero_return(false);
		terminate(lowir::make_instruction(lowir::Opcode::Unreachable, lowir::Type::scalar(lowir::TypeKind::Void)));
		return;
	}
	lowir::Instruction inst = lowir::make_instruction(lowir::Opcode::Return, result_type_);
	inst.ops[0] = integer(result_type_, 0).value;
	terminate(inst);
}

bool FunctionLowering::block_targeted(lowir::BlockId block)
{
	// Whether `block` has an incoming edge in the course's bookkeeping: a
	// branch, switch or region landing targets it from any placed block, a
	// jump from the entry or from a block with an incoming edge (a jump out
	// of a dead block, such as the end of a try whose paths all returned,
	// counts for nothing). A fixed point, for the rare non-void end flowed
	// into.
	const lowir::FunctionBody& body = builder_.body();
	std::vector<uint8_t> incoming(body.blocks.size(), 0);
	for (const lowir::BlockId folded : folded_targets_)
		incoming[folded] = 1;
	const lowir::BlockId entry = body.layout[0];
	const auto mark = [&](const lowir::Value& target, bool& changed) {
		if (target.is(lowir::ValueKind::Block) && incoming[target.index] == 0)
		{
			incoming[target.index] = 1;
			changed = true;
		}
	};
	for (bool changed = true; changed;)
	{
		changed = false;
		for (const lowir::BlockId at : body.layout)
		{
			const bool source = at == entry || incoming[at] != 0;
			for (lowir::InstId i = body.blocks[at].first; i != lowir::kNoId; i = body.instructions[i].next)
			{
				const lowir::Instruction& inst = body.instructions[i];
				switch (inst.op)
				{
				case lowir::Opcode::Jump:
					if (source)
						mark(inst.ops[0], changed);
					break;
				case lowir::Opcode::EhTry:
				case lowir::Opcode::EhCleanup:
					mark(inst.ops[0], changed);
					break;
				case lowir::Opcode::Branch:
					mark(inst.ops[1], changed);
					mark(inst.ops[2], changed);
					break;
				case lowir::Opcode::Switch:
					mark(inst.ops[1], changed);
					for (uint32_t e = 1; e < inst.extra_count; e += 2)
						mark(body.operands[inst.extra_first + e], changed);
					break;
				default:
					break;
				}
			}
		}
	}
	return incoming[block] != 0;
}

const syntax::Expr* FunctionLowering::reference_initializer(const syntax::Expr* single)
{
	// A reference's `{e}` binds `e` itself; another list (a prvalue) is the
	// temporary it initializes (8.5.4p3).
	if (single->kind != syntax::ExprKind::BracedInit || single->facts->category == sema::ValueCategory::PRValue)
		return single;
	return static_cast<const syntax::BracedInitList*>(single)->elements[0];
}

void FunctionLowering::lower_dynamic_initializer(lowir::SymbolId global, const sema::VariableEntity* variable,
	const syntax::Initializer* initializer, uint32_t location)
{
	// 3.6.2p2: the object was zero-initialized; its initializer runs here,
	// a full-expression whose other temporaries end with it (1.9p10,
	// 12.2p3).
	Place place = Place();
	place.storage = lowir::Value::symbol(global);
	qualify(place, variable->type);
	const sema::Type* type = variable->type;
	const size_t first = cleanups_.size();
	const syntax::Expr* single = initializer->kind == syntax::InitializerKind::Parenthesized
		? (initializer->arguments.size == 1 ? initializer->arguments[0] : nullptr)
		: initializer->expression;
	begin_static_initializer();
	const ProgramLowering::TemporaryNames names = lowering_.begin_temporary_names(variable, global);
	const syntax::Expr* saved = full_root(single);
	if (type->is_reference())
	{
		single = reference_initializer(single);
		// 12.2p5: the temporaries it binds persist (a thread's own
		// reference keeps them in frames, see temporary_storage).
		const bool persistent = !variable->is_thread_local;
		if (persistent)
			extension_path(single);
		RValue address;
		StaticTemporaries temporaries = StaticTemporaries{ true, std::vector<Finalizer>() };
		StaticTemporaries* const saved_temporaries = static_temporaries_;
		if (persistent)
			static_temporaries_ = &temporaries;
		address.value = bind_reference(*single, type, ConversionContext::Initialization, persistent);
		static_temporaries_ = saved_temporaries;
		persistent_path_.clear();
		address.type = lowir::Type::scalar(lowir::TypeKind::Ptr);
		store(address.type, address, place);
	}
	else
		initialize_storage(type, place, initializer, location);
	choice_root_ = saved;
	lowering_.end_temporary_names(names);
	if (!variable->is_thread_local || !type->is_reference())
		destroy_temporaries(first);
	static_initializer_ = false;
}

void FunctionLowering::begin_static_initializer()
{
	// An exception leaving a static or thread-local object's initialization
	// calls std::terminate (15.5.1p1); the stack is not unwound first (p2
	// leaves it implementation-defined; as g++): no cleanup regions.
	static_initializer_ = true;
}

void FunctionLowering::lower_temporary_initializer(
	lowir::SymbolId global, const sema::Type* type, const syntax::Expr& expr, uint32_t location)
{
	(void)location;
	Place place = Place();
	place.storage = lowir::Value::symbol(global);
	RValue value = operand(expr, ConversionContext::Initialization);
	value = fit(value, type);
	store(lowering_.storage_type(type), value, place);
}

void FunctionLowering::lower_thread_local_initializer(lowir::SymbolId global, lowir::SymbolId guard,
	const sema::VariableEntity* variable, const syntax::Initializer* initializer, uint32_t location, bool dynamic)
{
	// Once per thread: the guard tested, the object initialized (unless its
	// initialization is static), the guard set. Without a guard of its own
	// (an ordered variable in object mode), the unit's initialization
	// function guards it.
	const lowir::Type i64 = lowir::Type::scalar(lowir::TypeKind::I64);
	Place flag = Place();
	flag.storage = lowir::Value::symbol(guard);
	const lowir::BlockId run = guard != lowir::kNoId ? new_block(lowir::BlockKind::LocalStaticCtorRun) : lowir::kNoId;
	const lowir::BlockId done = new_block(lowir::BlockKind::LocalStaticCtorDone);
	if (guard != lowir::kNoId)
	{
		// The thread's flag is read through its address (storage=thread_local).
		Place reached = Place();
		reached.storage = address_of(flag);
		branch(compare(lowir::ComparePredicate::Ne, i64, load(i64, reached), integer(i64, 0)), done, run);
		place(run);
	}
	if (!dynamic)
		;
	else if (object_class(variable->type) != nullptr && !variable->type->is_reference())
		construct_global(global, variable, initializer, location);
	else
		lower_dynamic_initializer(global, variable, initializer, location);
	// Object mode: its destruction registered with the thread's exit list
	// now its initialization is complete (3.6.3p1).
	if (lowering_.object_mode() && needs_destruction(variable->type))
		register_destruction(global, variable->type, location, true);
	if (guard != lowir::kNoId)
		store(i64, integer(i64, 1), flag);
	jump(done);
	place(done);
	terminate(lowir::make_instruction(lowir::Opcode::Return, result_type_));
	if (!lowering_.streaming())
		builder_.finish();
}

void FunctionLowering::lower_thread_local_inits(lowir::SymbolId guard, const std::vector<lowir::SymbolId>& inits)
{
	// Once per thread, the guard set first (an initialization using
	// another of the unit's variables reaches this function again through
	// its wrapper, as g++'s and Clang's __tls_init): each variable's
	// initialization in definition order (3.6.2p2).
	const lowir::Type i8 = lowir::Type::scalar(lowir::TypeKind::I8);
	Place flag = Place();
	flag.storage = lowir::Value::symbol(guard);
	const lowir::BlockId run = new_block(lowir::BlockKind::LocalStaticCtorRun);
	const lowir::BlockId done = new_block(lowir::BlockKind::LocalStaticCtorDone);
	Place reached = Place();
	reached.storage = address_of(flag);
	branch(compare(lowir::ComparePredicate::Ne, i8, load(i8, reached), integer(i8, 0)), done, run);
	place(run);
	store(i8, integer(i8, 1), flag);
	for (const lowir::SymbolId init : inits)
	{
		++lowering_.counters().instructions;
		builder_.call(
			lowir::Type::scalar(lowir::TypeKind::Void), lowir::Value::symbol(init), nullptr, 0, lowir::kNoId, 0);
	}
	jump(done);
	place(done);
	terminate(lowir::make_instruction(lowir::Opcode::Return, result_type_));
	if (!lowering_.streaming())
		builder_.finish();
}

void FunctionLowering::lower_unordered_initializer(lowir::SymbolId global, lowir::SymbolId guard,
	lowir::SymbolId constructed, const sema::VariableEntity* variable, const syntax::Initializer* initializer,
	uint32_t location, bool dynamic, bool registered)
{
	// Every unit instantiating the member runs this: the first to reach it
	// sets the guard they share, then initializes the object and its own
	// unit's `constructed` flag, which its finalizer tests (in object mode,
	// registers its destruction instead: a statically initialized member's
	// registration alone, so one unit registers it).
	const lowir::Type i64 = lowir::Type::scalar(lowir::TypeKind::I64);
	Place flag = Place();
	flag.storage = lowir::Value::symbol(guard);
	const lowir::BlockId run = new_block(lowir::BlockKind::LocalStaticCtorRun);
	const lowir::BlockId done = new_block(lowir::BlockKind::LocalStaticCtorDone);
	branch(compare(lowir::ComparePredicate::Ne, i64, load(i64, flag), integer(i64, 0)), done, run);
	place(run);
	store(i64, integer(i64, 1), flag);
	// A class template specialization's member's temporaries (a
	// reference's, an aggregate's reference members') in host objects:
	// g++'s (see begin_owned_temporaries).
	if (dynamic)
		lowering_.begin_owned_temporaries(variable);
	if (!dynamic)
		;
	else if (object_class(variable->type) != nullptr && !variable->type->is_reference())
		construct_global(global, variable, initializer, location);
	else
		lower_dynamic_initializer(global, variable, initializer, location);
	if (dynamic)
		lowering_.end_owned_temporaries();
	if (constructed != lowir::kNoId)
	{
		const lowir::Type i8 = lowir::Type::scalar(lowir::TypeKind::I8);
		Place made = Place();
		made.storage = lowir::Value::symbol(constructed);
		store(i8, integer(i8, 1), made);
	}
	if (registered)
		register_destruction(global, variable->type, location);
	jump(done);
	place(done);
}

void FunctionLowering::lower_thread_local_wrapper(lowir::SymbolId init, lowir::SymbolId global)
{
	// The thread's object initialized (a dynamic initialization's `init`),
	// then its address. Another unit's entry is a weak reference, called
	// only where some unit defines it (Itanium 3.7: a unit initializing the
	// object statically, a g++ one included, defines none).
	if (init != lowir::kNoId)
	{
		const bool weak = lowering_.program().symbols[init].kind == lowir::SymbolKind::FunctionDeclaration;
		lowir::BlockId done = lowir::kNoId;
		if (weak)
		{
			const lowir::Type ptr = lowir::Type::scalar(lowir::TypeKind::Ptr);
			const lowir::BlockId run = new_block(lowir::BlockKind::LocalStaticCtorRun);
			done = new_block(lowir::BlockKind::LocalStaticCtorDone);
			branch(
				compare(lowir::ComparePredicate::Ne, ptr, RValue{ lowir::Value::symbol(init), ptr }, integer(ptr, 0)),
				run, done);
			place(run);
		}
		++lowering_.counters().instructions;
		builder_.call(
			lowir::Type::scalar(lowir::TypeKind::Void), lowir::Value::symbol(init), nullptr, 0, lowir::kNoId, 0);
		if (weak)
		{
			jump(done);
			place(done);
		}
	}
	Place place = Place();
	place.storage = lowir::Value::symbol(global);
	lowir::Instruction inst = lowir::make_instruction(lowir::Opcode::Return, result_type_);
	inst.ops[0] = address_of(place);
	terminate(inst);
	if (!lowering_.streaming())
		builder_.finish();
}

void FunctionLowering::lower_terminate_helper(lowir::SymbolId begin_catch, lowir::SymbolId terminate_function)
{
	// 15.5.1: the exception is caught (the handler it would have), then
	// std::terminate is called.
	lowir::Value exception = lowir::Value::temp(builder_.parameter(0));
	++lowering_.counters().instructions;
	builder_.call(
		lowir::Type::scalar(lowir::TypeKind::Ptr), lowir::Value::symbol(begin_catch), &exception, 1, lowir::kNoId, 0);
	++lowering_.counters().instructions;
	builder_.call(lowir::Type::scalar(lowir::TypeKind::Void), lowir::Value::symbol(terminate_function), nullptr, 0,
		lowir::kNoId, 0);
	terminate(lowir::make_instruction(lowir::Opcode::Return, result_type_));
	if (!lowering_.streaming())
		builder_.finish();
}

void FunctionLowering::lower_forwarder(lowir::SymbolId self, lowir::SymbolId target)
{
	const lowir::Function& definition = program_.functions[program_.symbols[self].detail];
	const lowir::Signature& signature = program_.signatures[definition.signature];
	for (uint32_t i = 0; i < signature.parameter_count; ++i)
		arguments_.push_back(lowir::Value::temp(builder_.parameter(i)));
	++lowering_.counters().instructions;
	const lowir::TempId result = builder_.call(result_type_, lowir::Value::symbol(target), arguments_.data(),
		static_cast<uint32_t>(arguments_.size()), lowir::kNoId, 0);
	arguments_.clear();
	lowir::Instruction inst = lowir::make_instruction(lowir::Opcode::Return, result_type_);
	if (!result_type_.is_void())
		inst.ops[0] = lowir::Value::temp(result);
	terminate(inst);
	if (!lowering_.streaming())
		builder_.finish();
}

void FunctionLowering::lower_thunk(lowir::SymbolId self, lowir::SymbolId target, uint32_t this_index,
	int64_t adjustment, int64_t vcall, const ProgramLowering::ResultAdjustment* result)
{
	// The parameters passed on, `this` moved first (by the fixed offset,
	// then by the vcall offset its table holds at `vcall`, if any); a
	// covariant result adjusted after (Itanium 2.5.3: a non-null pointer,
	// or a reference, through the virtual base's offset in the returned
	// object's table, then by the fixed offset, as a derived-to-base
	// conversion is).
	const lowir::Function& definition = program_.functions[program_.symbols[self].detail];
	const lowir::Signature& signature = program_.signatures[definition.signature];
	for (uint32_t i = 0; i < signature.parameter_count; ++i)
	{
		lowir::Value argument = lowir::Value::temp(builder_.parameter(i));
		if (i == this_index)
		{
			const lowir::Type i8 = lowir::Type::scalar(lowir::TypeKind::I8);
			const lowir::Type i64 = lowir::Type::scalar(lowir::TypeKind::I64);
			// (A virtual thunk without a fixed part reads the vcall offset
			// through `this` as it comes.)
			if (adjustment != 0 || vcall == 0)
				argument =
					index(i8, ir_model::IPK_NONE, argument, integer(i64, static_cast<uint64_t>(adjustment))).value;
			if (vcall != 0)
			{
				Place vptr = Place();
				vptr.storage = argument;
				Place offset = Place();
				offset.storage = index(i8, ir_model::IPK_NONE,
					load(lowir::Type::scalar(lowir::TypeKind::Ptr), vptr).value,
					integer(i64, static_cast<uint64_t>(vcall)))
									 .value;
				argument = index(i8, ir_model::IPK_NONE, argument, load(i64, offset)).value;
			}
		}
		arguments_.push_back(argument);
	}
	++lowering_.counters().instructions;
	const lowir::TempId returned = builder_.call(result_type_, lowir::Value::symbol(target), arguments_.data(),
		static_cast<uint32_t>(arguments_.size()), lowir::kNoId, 0);
	arguments_.clear();
	lowir::Instruction inst = lowir::make_instruction(lowir::Opcode::Return, result_type_);
	if (!result_type_.is_void())
		inst.ops[0] = lowir::Value::temp(returned);
	if (result != nullptr)
	{
		const lowir::Type pointer = lowir::Type::scalar(lowir::TypeKind::Ptr);
		const lowir::Type i64 = lowir::Type::scalar(lowir::TypeKind::I64);
		const lowir::Type i8 = lowir::Type::scalar(lowir::TypeKind::I8);
		RValue value;
		value.value = lowir::Value::temp(returned);
		value.type = pointer;
		if (!result->reference)
		{
			const lowir::BlockId null_block = new_block(lowir::BlockKind::ReturnNull);
			const lowir::BlockId adjust = new_block(lowir::BlockKind::AdjustReturn);
			branch(compare(lowir::ComparePredicate::Eq, pointer, value, integer(pointer, 0)), null_block, adjust);
			place(null_block);
			terminate(inst);
			place(adjust);
		}
		if (result->via != nullptr)
		{
			Place vptr = Place();
			vptr.storage = value.value;
			Place row = Place();
			row.storage = index(i8, ir_model::IPK_NONE, load(pointer, vptr).value,
				integer(i64, static_cast<uint64_t>(lowering_.vbase_row_offset(result->from, result->via))))
							  .value;
			value = index(i8, ir_model::IPK_NONE, value.value, load(i64, row));
		}
		if (result->offset != 0)
			value = index(i8, ir_model::IPK_NONE, value.value, integer(i64, static_cast<uint64_t>(result->offset)));
		inst.ops[0] = value.value;
	}
	terminate(inst);
	if (!lowering_.streaming())
		builder_.finish();
}

void FunctionLowering::finish_initializer()
{
	if (!terminated_)
		terminate(lowir::make_instruction(lowir::Opcode::Return, lowir::Type::scalar(lowir::TypeKind::Void)));
	if (!lexical_pending_.empty())
		place_lexical_blocks();
	if (!pending_terminates_.empty())
		place_terminates();
	if (!lowering_.streaming())
		builder_.finish();
}

void FunctionLowering::check_stack(uint32_t location) const
{
	char marker;
	if (stack_bytes_used(lowering_.stack_base(), &marker) > lowering_.stack_budget())
		throw LoweringError(location, "constructs nested too deeply");
}

// ----------------------------------------------------------------- blocks

lowir::BlockId FunctionLowering::new_block(lowir::BlockKind kind)
{
	++lowering_.counters().blocks;
	return builder_.add_block(0, kind);
}

void FunctionLowering::place(lowir::BlockId block)
{
	// A constructor's cleanup blocks follow the block that opened their
	// regions.
	if (!deferred_.empty() && terminated_)
		flush_deferred();
	if (!lexical_pending_.empty() && terminated_)
		place_lexical_blocks();
	if (!pending_terminates_.empty() && terminated_)
		place_terminates();
	builder_.place_block(block);
	builder_.set_insertion_block(block);
	terminated_ = false;
}

lowir::InstId FunctionLowering::emit(const lowir::Instruction& inst)
{
	// Code after a terminator that a label made reachable again starts a
	// block of its own.
	if (terminated_)
		place(new_block(lowir::BlockKind::Unreachable));
	++lowering_.counters().instructions;
	return builder_.append(inst);
}

void FunctionLowering::terminate(const lowir::Instruction& inst)
{
	// A full-expression's region ends before its control flow does; a
	// return leaves a terminate guard's region.
	paused_call_region(inst.op);
	if (region_segment_)
		close_region();
	if (inst.op == lowir::Opcode::Return && terminate_landing_ != lowir::kNoId && guard_returns_)
		emit_marker(lowir::Opcode::EhEnd, lowir::kNoId);
	emit(inst);
	terminated_ = true;
}

lowir::Value FunctionLowering::emit_value(lowir::Instruction inst, lowir::Type type)
{
	if (terminated_)
		place(new_block(lowir::BlockKind::Unreachable));
	++lowering_.counters().instructions;
	const lowir::Value result = lowir::Value::temp(builder_.append_value(inst, type, 0));
	if ((inst.op == lowir::Opcode::Index || inst.op == lowir::Opcode::Copy) && untyped_address(inst.ops[0]))
		mark_untyped(result);
	return result;
}

void FunctionLowering::mark_untyped(lowir::Value address)
{
	if (address.kind != lowir::ValueKind::Temp)
		return;
	if (address.index >= untyped_addresses_.size())
		untyped_addresses_.resize(address.index + 1, 0);
	untyped_addresses_[address.index] = 1;
}

bool FunctionLowering::union_member(const sema::VariableEntity* member)
{
	const sema::ClassEntity* cls = sema::ClassOf(member->owner->entity->type);
	while (cls->anonymous && cls->storage != nullptr && cls->storage->entity->owner->kind == sema::ScopeKind::Class)
	{
		if (cls->key == TokenKind::KW_UNION)
			return true;
		cls = sema::ClassOf(cls->storage->entity->owner->entity->type);
	}
	return cls->key == TokenKind::KW_UNION;
}

bool FunctionLowering::through_union(const sema::VariableEntity* member, const sema::ClassEntity* holder)
{
	for (const sema::ClassEntity* cls = sema::ClassOf(member->owner->entity->type); cls != nullptr;)
	{
		if (cls->key == TokenKind::KW_UNION)
			return true;
		if (cls == holder || !cls->anonymous || cls->storage == nullptr ||
			cls->storage->entity->owner->kind != sema::ScopeKind::Class)
			return false;
		cls = sema::ClassOf(cls->storage->entity->owner->entity->type);
	}
	return false;
}

lowir::SlotId FunctionLowering::new_slot(lowir::NameId name, lowir::Type type)
{
	// A source name labels one slot of the body; a shadowing declaration's
	// keeps it as the name its shadow's spelling derives from.
	if (name == 0)
		name = lowering_.unnamed_slot_name();
	lowir::SlotKind kind = lowir::SlotKind::Plain;
	if (lowering_.range_for_name(name))
		kind = lowir::SlotKind::RangeFor;
	else if (!lowering_.claim_slot_name(name, slot_scope_))
		kind = lowir::SlotKind::Shadow;
	++lowering_.counters().slots;
	return builder_.add_slot(name, type, kind);
}

lowir::SlotId FunctionLowering::generated_slot(lowir::SlotKind kind, lowir::Type type)
{
	++lowering_.counters().slots;
	return builder_.add_slot(0, type, kind);
}

lowir::SlotId FunctionLowering::parameter_slot(uint32_t index, lowir::Type type)
{
	// (Past the index a slot holds, by position as a generated one.)
	++lowering_.counters().slots;
	return index <= 0xffff ? builder_.add_slot(0, type, lowir::SlotKind::Parameter, static_cast<uint16_t>(index))
						   : builder_.add_slot(0, type, lowir::SlotKind::Arg);
}

void FunctionLowering::jump(lowir::BlockId target)
{
	lowir::Instruction inst = lowir::make_instruction(lowir::Opcode::Jump, lowir::Type::scalar(lowir::TypeKind::Void));
	inst.ops[0] = lowir::Value::block(target);
	terminate(inst);
}

void FunctionLowering::branch(const RValue& condition, lowir::BlockId if_true, lowir::BlockId if_false)
{
	lowir::Instruction inst = lowir::make_instruction(
		lowir::Opcode::Branch, lowir::Type::scalar(lowir::TypeKind::Void));
	inst.ops[0] = condition.value;
	inst.ops[1] = lowir::Value::block(if_true);
	inst.ops[2] = lowir::Value::block(if_false);
	terminate(inst);
}

// ----------------------------------------------------------- instructions

RValue FunctionLowering::integer(lowir::Type type, uint64_t bits)
{
	RValue value;
	value.value = builder_.literal(
		lowir::ValueKind::Integer, lowir::TypeKind::Void, lowir::integer_literal(static_cast<int64_t>(bits)));
	value.type = type;
	return value;
}

RValue FunctionLowering::integer(lowir::Type type, const lowir::Literal& bits)
{
	RValue value;
	value.value = builder_.literal(lowir::ValueKind::Integer, lowir::TypeKind::Void, bits);
	value.type = type;
	return value;
}

RValue FunctionLowering::integer(const sema::Type* type, uint64_t bits)
{
	return integer(lowering_.storage_type(type), IntegralLiteral(type, bits));
}

RValue FunctionLowering::floating(lowir::Type type, const lowir::Literal& bits)
{
	RValue value;
	value.value = builder_.literal(lowir::ValueKind::Float, type.kind(), bits);
	value.type = type;
	return value;
}

bool FunctionLowering::is_integer_literal(const RValue& value, uint64_t& bits)
{
	if (value.value.kind != lowir::ValueKind::Integer)
		return false;
	bits = builder_.body().literals[value.value.index].low;
	return true;
}

RValue FunctionLowering::constant(lowir::Type type, uint64_t bits)
{
	lowir::Instruction inst = lowir::make_instruction(lowir::Opcode::Const, type);
	inst.ops[0] = integer(type, bits).value;
	RValue value;
	value.value = emit_value(inst, type);
	value.type = type;
	return value;
}

void FunctionLowering::qualify(Place& place, const sema::Type* type)
{
	place.is_volatile = (type->cv & sema::kVolatile) != 0;
	place.is_atomic = (type->cv & sema::kAtomic) != 0;
}

// An access the optimizer may tell apart by its type (IF_TYPED): a pointer,
// or an integer or floating type wider than a byte (a byte type may be a
// character type, which accesses any object).
static bool typed_access(lowir::Type type)
{
	const lowir::TypeKind k = type.kind();
	return type.is_pointer() ||
		((type.is_integer() || type.is_float()) && k != lowir::TypeKind::I1 && k != lowir::TypeKind::I8 &&
			k != lowir::TypeKind::U8);
}

RValue FunctionLowering::load(lowir::Type type, const Place& place)
{
	const bool atomic = place.is_atomic && (type.is_integer() || type.kind() == lowir::TypeKind::Ptr);
	lowir::Type bits;
	if (place.is_atomic && !atomic && atomic_bits(type, bits) && bits.kind() != lowir::TypeKind::I128)
	{
		// An atomic floating (or binary16) object's bits, read atomically
		// as the integer of its size, then as its type through a slot (not
		// `typed`: the slot's bytes are read as another type).
		Place slot = Place();
		slot.storage = lowir::Value::slot(generated_slot(lowir::SlotKind::AtomicResult, bits));
		slot.untyped = true;
		store(bits, load(bits, place), slot);
		Place view = Place();
		view.storage = address_of(slot);
		view.untyped = true;
		return load(type, view);
	}
	lowir::Instruction inst = lowir::make_instruction(atomic ? lowir::Opcode::AtomicLoad : lowir::Opcode::Load, type);
	inst.ops[0] = place.storage;
	if (atomic)
		inst.order = lowir::MemoryOrder::SeqCst;
	if (place.is_volatile)
		inst.flags |= lowir::IF_VOLATILE;
	else if (!atomic && !place.untyped && typed_access(type) && !untyped_address(place.storage))
		inst.flags |= lowir::IF_TYPED;
	RValue value;
	value.value = emit_value(inst, type);
	value.type = type;
	return value;
}

void FunctionLowering::store(lowir::Type type, const RValue& value, const Place& place)
{
	const bool atomic = place.is_atomic && (type.is_integer() || type.kind() == lowir::TypeKind::Ptr);
	lowir::Type bits;
	if (place.is_atomic && !atomic && atomic_bits(type, bits) && bits.kind() != lowir::TypeKind::I128)
	{
		// An atomic floating (or binary16) object's new value stored as the
		// bits of the integer of its size (a sequentially consistent store,
		// C11 7.17.7.1).
		Place slot = Place();
		slot.storage = lowir::Value::slot(generated_slot(lowir::SlotKind::AtomicArg, type));
		slot.untyped = true;
		store(type, value, slot);
		Place view = Place();
		view.storage = address_of(slot);
		view.untyped = true;
		store(bits, load(bits, view), place);
		return;
	}
	lowir::Instruction inst = lowir::make_instruction(atomic ? lowir::Opcode::AtomicStore : lowir::Opcode::Store, type);
	inst.ops[0] = value.value;
	inst.ops[1] = place.storage;
	if (atomic)
		inst.order = lowir::MemoryOrder::SeqCst;
	if (place.is_volatile)
		inst.flags |= lowir::IF_VOLATILE;
	else if (!atomic && !place.untyped && typed_access(type) && !untyped_address(place.storage))
		inst.flags |= lowir::IF_TYPED;
	emit(inst);
}

RValue FunctionLowering::binary(
	lowir::BinaryOp op, lowir::Type type, const RValue& left, const RValue& right, uint8_t flags)
{
	lowir::Instruction inst = lowir::make_instruction(lowir::Opcode::Binary, type);
	inst.sub = static_cast<uint8_t>(op);
	inst.flags = flags;
	inst.ops[0] = left.value;
	inst.ops[1] = right.value;
	RValue value;
	value.value = emit_value(inst, type);
	value.type = type;
	return value;
}

RValue FunctionLowering::compare(
	lowir::ComparePredicate predicate, lowir::Type type, const RValue& left, const RValue& right)
{
	// lowir.md: a comparison's result is the canonical i64 truth value.
	lowir::Instruction inst = lowir::make_instruction(lowir::Opcode::Compare, type);
	inst.sub = static_cast<uint8_t>(predicate);
	inst.ops[0] = left.value;
	inst.ops[1] = right.value;
	RValue value;
	value.type = lowir::Type::scalar(lowir::TypeKind::I64);
	value.value = emit_value(inst, value.type);
	return value;
}

RValue FunctionLowering::unary(lowir::UnaryOp op, const RValue& operand)
{
	lowir::Instruction inst = lowir::make_instruction(lowir::Opcode::Unary, operand.type);
	inst.sub = static_cast<uint8_t>(op);
	inst.ops[0] = operand.value;
	RValue value;
	value.value = emit_value(inst, operand.type);
	value.type = operand.type;
	return value;
}

RValue FunctionLowering::convert_op(lowir::ConvertOp op, lowir::Type to, const RValue& value)
{
	lowir::Instruction inst = lowir::make_instruction(lowir::Opcode::Convert, to);
	inst.sub = static_cast<uint8_t>(op);
	inst.ops[0] = value.value;
	lowir::set_convert_source_type(inst, value.type);
	RValue result;
	result.value = emit_value(inst, to);
	result.type = to;
	return result;
}

RValue FunctionLowering::copy(lowir::Type to, const RValue& value)
{
	lowir::Instruction inst = lowir::make_instruction(lowir::Opcode::Copy, to);
	inst.ops[0] = value.value;
	RValue result;
	result.value = emit_value(inst, to);
	result.type = to;
	return result;
}

lowir::Value FunctionLowering::address_of(const Place& place)
{
	if (place.storage.kind != lowir::ValueKind::Slot && place.storage.kind != lowir::ValueKind::Symbol)
		return place.storage;
	lowir::Instruction inst = lowir::make_instruction(lowir::Opcode::Addr, lowir::Type::scalar(lowir::TypeKind::Ptr));
	inst.ops[0] = place.storage;
	return emit_value(inst, lowir::Type::scalar(lowir::TypeKind::Ptr));
}

RValue FunctionLowering::index(lowir::Type element, uint8_t projection, lowir::Value base, const RValue& offset)
{
	lowir::Instruction inst = lowir::make_instruction(lowir::Opcode::Index, element);
	inst.sub = projection;
	inst.ops[0] = base;
	inst.ops[1] = offset.value;
	RValue value;
	value.type = lowir::Type::scalar(lowir::TypeKind::Ptr);
	value.value = emit_value(inst, value.type);
	return value;
}

} // namespace lower
