#include "lower/function_lowering.h"

#include <cstring>
#include <string>

#include "lex/identifier_table.h"
#include "sema/conversions.h"
#include "sema/entity_records.h"

// Function-local objects of static storage duration (6.7p4, PA16): one
// global per variable (internal, or weak in an inline or instantiated
// function), constant-initialized when its initialization is a constant
// expression (3.6.2p2); otherwise initialized the first time control
// passes through its declaration, behind an i64 guard of its own, its
// destructor then registered with atexit (3.6.3p1). The initialization is
// not made thread-safe (the course's single-threaded form) except in host
// objects, where concurrent entry waits for it (6.7p4) through Itanium's
// one-time construction API (3.3.2), as g++'s units sharing the guard do.
// A block-scope thread_local object (3.7.2) is each thread's: the object
// and its guard are thread-local (storage=thread_local), its destructor
// registered with the thread's exit list in object mode.
namespace lower {

lowir::SymbolId ProgramLowering::local_static(const sema::VariableEntity* variable,
	const sema::FunctionEntity* function, uint32_t ordinal, const syntax::Initializer* initializer, uint32_t location,
	lowir::SymbolId& guard, const syntax::InitDeclarator* tokens)
{
	const uint32_t found = local_static_index_.find(variable);
	if (found != PointerMap::kMissing)
	{
		guard = local_statics_[found].guard;
		return local_statics_[found].symbol;
	}
	// `__local_static__<function>__decl<ordinal>__<name>`: an instance's
	// function named by its object spelling's bytes (its source spelling is
	// every instance's), as is a local class's or a closure's member every
	// unit shares (its presentation label is numbered in its unit).
	const lowir::SymbolId owner = function_symbol(function);
	// The function's binding, also while only its always_inline copy is
	// defined (the function's own symbol not yet declared).
	const uint8_t owner_binding = program_.symbols[owner].kind == lowir::SymbolKind::Unresolved
		? function_metadata(function, owner).binding
		: program_.symbols[owner].metadata.binding;
	std::string spelling = "__local_static__";
	const sema::Scope* home = function->owner;
	while (home != nullptr && home->kind == sema::ScopeKind::Class &&
		closure_call(static_cast<const sema::ClassEntity*>(home->entity)) == nullptr)
		home = home->parent;
	const bool local_member = home != nullptr && home != function->owner &&
		(home->kind == sema::ScopeKind::Block || home->kind == sema::ScopeKind::Function);
	const bool closure_member = home != nullptr && home->kind == sema::ScopeKind::Class;
	if (function->instantiated || (owner_binding == ir_model::SBM_WEAK && (local_member || closure_member)))
	{
		static const char kHex[] = "0123456789abcdef";
		spelling += "function_symbol_";
		for (const unsigned char c : function_object(function, EntryVariant::Complete))
		{
			spelling += kHex[c >> 4];
			spelling += kHex[c & 15];
		}
	}
	else
		spelling += program_.spelling(program_.symbols[owner].name);
	const char* name = variable->name != nullptr ? variable->name->spelling : "object";
	// One of a strong function whose initializer makes a temporary is
	// named by its init-declarator's token span (the course's form).
	if (tokens != nullptr && owner_binding != ir_model::SBM_WEAK && !function->instantiated &&
		tokens->end_token > tokens->first_token)
	{
		spelling += "__";
		spelling += name;
		spelling += "__tokens" + std::to_string(tokens->first_token) + "_" + std::to_string(tokens->end_token);
	}
	else
	{
		spelling += "__decl" + std::to_string(ordinal) + "__";
		spelling += name;
	}
	const lowir::SymbolId symbol = new_symbol(spelling);
	lowir::SymbolMetadata metadata = lowir::SymbolMetadata();
	metadata.binding = owner_binding == ir_model::SBM_WEAK ? ir_model::SBM_WEAK : ir_model::SBM_INTERNAL;
	if (variable->is_thread_local)
		metadata.storage = ir_model::GSM_THREAD_LOCAL;
	// Its Itanium name, the units' (a host compiler's included) of an
	// inline or instantiated function shared: Z <function encoding> E
	// <name> [<discriminator>] (5.1.7), its guard's _ZGV and that name; an
	// internal one's names its unit's local symbol. The LowIR spelling
	// stays the course's.
	abi_mangle::AbiNode abi_name = abi_mangle::ABI_NONE;
	if (variable->name != nullptr || graph_->decomposition(variable) != nullptr)
	{
		abi_name = local_static_name(variable);
		const std::string object = special_object(abi_mangle::ABI_TARGET_ENTITY, abi_name);
		metadata.object = metadata.binding == ir_model::SBM_INTERNAL
			? local_object(object, false)
			: intern(object.data(), static_cast<uint32_t>(object.size()));
	}
	metadata.section = variable_section(variable);
	// A GNU asm label names its object (as for other variables).
	if (const char* asm_label = graph_->asm_label(variable))
	{
		const std::string label = asm_label;
		metadata.object = metadata.binding == ir_model::SBM_INTERNAL
			? local_object(label, false)
			: intern(label.data(), static_cast<uint32_t>(label.size()));
	}
	static_scope_ = variable->owner;
	const TemporaryNames names = begin_temporary_names(variable, symbol);
	const StaticOutcome outcome = static_global(symbol, variable, initializer, location, metadata);
	end_temporary_names(names);
	static_scope_ = nullptr;
	++counters_.globals;
	// A constant-initialized class object's constructor is still emitted
	// (the course's form); one whose destructor must run is initialized on
	// first use, when the destructor is registered.
	const bool destroyed = FunctionLowering::needs_destruction(variable->type) && !variable->type->is_reference();
	if (outcome.evaluated && !destroyed)
	{
		if (variable->constructor != nullptr && !variable->constructor->trivial)
			demand(variable->constructor);
		demand_constructions(initializer);
	}
	guard = lowir::kNoId;
	if (outcome.dynamic || destroyed)
	{
		guard = new_symbol(spelling + "__guard");
		lowir::Global flag = lowir::Global();
		flag.typed = true;
		flag.type = lowir::Type::scalar(lowir::TypeKind::I64);
		flag.init = lowir::GlobalInit::Zero;
		program_.declare_global(guard, lowir::SymbolKind::GlobalDefinition, flag);
		program_.symbols[guard].metadata.binding = metadata.binding;
		if (variable->is_thread_local)
			program_.symbols[guard].metadata.storage = ir_model::GSM_THREAD_LOCAL;
		if (abi_name != abi_mangle::ABI_NONE)
		{
			const std::string object = special_object(abi_mangle::ABI_TARGET_GUARD_VARIABLE, abi_name);
			program_.symbols[guard].metadata.object = metadata.binding == ir_model::SBM_INTERNAL
				? local_object(object, false)
				: intern(object.data(), static_cast<uint32_t>(object.size()));
		}
		++counters_.globals;
	}
	local_static_index_.set(variable, static_cast<uint32_t>(local_statics_.size()));
	local_statics_.push_back(LocalStatic{ symbol, guard });
	return symbol;
}

abi_mangle::AbiNode ProgramLowering::local_static_name(const sema::VariableEntity* variable)
{
	// Z <function encoding> E <name> [<discriminator>] (5.1.7); a
	// structured binding declaration's object is named by its names
	// (5.1.4: _ZZ1fvEDC1x1yE).
	const abi_mangle::AbiNode scope = abi_scope(variable->owner);
	const abi_mangle::AbiNode name = tagged_name(scope, variable_source_name(variable), variable,
		local_discriminator(variable, graph_->local_discriminator(variable)), false);
	return with_implicit_tags(name, variable->type);
}

lowir::SymbolId ProgramLowering::atexit_function()
{
	// The C library's atexit (7.20.4.2 of C99, 18.5p5), declared once.
	if (atexit_ != lowir::kNoId)
		return atexit_;
	atexit_ = new_symbol("__cppgm_runtime_atexit");
	lowir::Signature signature = lowir::Signature();
	signature.first_parameter = static_cast<uint32_t>(program_.parameters.size());
	lowir::Parameter function = lowir::Parameter();
	function.name = intern("arg0", 4);
	function.type = lowir::Type::scalar(lowir::TypeKind::Ptr);
	program_.parameters.push_back(function);
	signature.parameter_count = 1;
	signature.result = lowir::Type::scalar(lowir::TypeKind::I32);
	signature.boundary.unwind = ir_model::CUM_NO;
	program_.declare_function(atexit_, lowir::SymbolKind::FunctionDeclaration, program_.add_signature(signature));
	lowir::SymbolMetadata& metadata = program_.symbols[atexit_].metadata;
	metadata.linkage = ir_model::LLM_C;
	metadata.binding = ir_model::SBM_STRONG;
	metadata.object = intern("atexit", 6);
	return atexit_;
}

lowir::SymbolId ProgramLowering::guard_function(GuardCall call)
{
	lowir::SymbolId& symbol = guard_functions_[static_cast<uint32_t>(call)];
	if (symbol != lowir::kNoId)
		return symbol;
	static const char* const kNames[] = { "__cxa_guard_acquire", "__cxa_guard_release", "__cxa_guard_abort" };
	const char* name = kNames[static_cast<uint32_t>(call)];
	symbol = new_symbol(std::string("__cppgm_runtime_") + (name + 2));
	lowir::Signature signature = lowir::Signature();
	signature.first_parameter = static_cast<uint32_t>(program_.parameters.size());
	lowir::Parameter guard = lowir::Parameter();
	guard.name = argument_name(0);
	guard.type = lowir::Type::scalar(lowir::TypeKind::Ptr);
	program_.parameters.push_back(guard);
	signature.parameter_count = 1;
	signature.result = lowir::Type::scalar(call == GuardCall::Acquire ? lowir::TypeKind::I32 : lowir::TypeKind::Void);
	program_.declare_function(symbol, lowir::SymbolKind::FunctionDeclaration, program_.add_signature(signature));
	lowir::SymbolMetadata& metadata = program_.symbols[symbol].metadata;
	metadata.linkage = ir_model::LLM_C;
	metadata.binding = ir_model::SBM_STRONG;
	metadata.object = intern(name, static_cast<uint32_t>(std::strlen(name)));
	return symbol;
}

lowir::SymbolId ProgramLowering::thread_atexit_function()
{
	// Itanium's __cxa_thread_atexit(destructor, object, dso handle): a
	// thread's exit functions, run before atexit's (3.6.3p1), declared once.
	if (thread_atexit_ != lowir::kNoId)
		return thread_atexit_;
	thread_atexit_ = new_symbol("__cppgm_runtime_cxa_thread_atexit");
	lowir::Signature signature = lowir::Signature();
	signature.first_parameter = static_cast<uint32_t>(program_.parameters.size());
	for (uint32_t i = 0; i < 3; ++i)
	{
		lowir::Parameter parameter = lowir::Parameter();
		parameter.name = argument_name(i);
		parameter.type = lowir::Type::scalar(lowir::TypeKind::Ptr);
		program_.parameters.push_back(parameter);
	}
	signature.parameter_count = 3;
	signature.result = lowir::Type::scalar(lowir::TypeKind::I32);
	program_.declare_function(
		thread_atexit_, lowir::SymbolKind::FunctionDeclaration, program_.add_signature(signature));
	lowir::SymbolMetadata& metadata = program_.symbols[thread_atexit_].metadata;
	metadata.linkage = ir_model::LLM_C;
	metadata.binding = ir_model::SBM_STRONG;
	metadata.object = intern("__cxa_thread_atexit", 19);
	return thread_atexit_;
}

lowir::SymbolId ProgramLowering::dso_handle()
{
	if (dso_handle_ != lowir::kNoId)
		return dso_handle_;
	dso_handle_ = new_symbol("__cppgm_runtime_dso_handle");
	lowir::Global global = lowir::Global();
	global.init = lowir::GlobalInit::None;
	program_.declare_global(dso_handle_, lowir::SymbolKind::GlobalDeclaration, global);
	lowir::SymbolMetadata& metadata = program_.symbols[dso_handle_].metadata;
	metadata.linkage = ir_model::LLM_C;
	metadata.binding = ir_model::SBM_STRONG;
	metadata.object = intern("__dso_handle", 12);
	return dso_handle_;
}

lowir::SymbolId ProgramLowering::local_static_destructor(
	lowir::SymbolId global, const sema::Type* type, uint32_t location)
{
	// An internal function destroying the object (an array's elements last
	// to first), which atexit calls at program end.
	return local_static_destructor(global, std::vector<Finalizer>(1, finalizer_for(global, type, location)));
}

lowir::SymbolId ProgramLowering::local_static_destructor(
	lowir::SymbolId global, const std::vector<Finalizer>& temporaries)
{
	const lowir::SymbolId symbol = new_symbol(
		"__cppgm_local_static_destructor__" + std::string(program_.spelling(program_.symbols[global].name)));
	lowir::Signature signature = lowir::Signature();
	signature.first_parameter = static_cast<uint32_t>(program_.parameters.size());
	signature.result = lowir::Type::scalar(lowir::TypeKind::Void);
	signature.boundary.unwind = ir_model::CUM_NO;
	program_.declare_function(symbol, lowir::SymbolKind::FunctionDefinition, program_.add_signature(signature));
	program_.symbols[symbol].metadata.binding = ir_model::SBM_INTERNAL;
	{
		FunctionLowering body(*this, symbol, nullptr);
		for (size_t i = temporaries.size(); i-- > 0;)
			body.finalize_global(temporaries[i]);
		body.finish_initializer();
	}
	release_definition(symbol);
	++counters_.functions;
	return symbol;
}

bool FunctionLowering::initializer_temporaries(const sema::VariableEntity* variable, const syntax::Initializer* init)
{
	// Whether the initializer materializes a temporary: a reference bound
	// to a prvalue (or a list's object), a std::initializer_list's array, a
	// class temporary its full-expression destroys.
	if (init == nullptr || init->kind == syntax::InitializerKind::Default)
		return false;
	const syntax::Expr* single = init->kind == syntax::InitializerKind::Parenthesized
		? (init->arguments.size == 1 ? init->arguments[0] : nullptr)
		: init->expression;
	if (variable->type->is_reference())
	{
		if (single == nullptr)
			return false;
		const syntax::Expr* bound = reference_initializer(single);
		const syntax::Expr* inner = bound;
		while (inner->kind == syntax::ExprKind::Paren)
			inner = static_cast<const syntax::ParenExpr*>(inner)->inner;
		if (inner->facts != nullptr && inner->facts->category == sema::ValueCategory::PRValue)
			return true;
		return binding_temporary(*bound) != nullptr || creates_temporaries(bound);
	}
	if (single != nullptr && list_object(*single) != nullptr)
		return true;
	if (init->kind == syntax::InitializerKind::Parenthesized)
	{
		for (const syntax::Expr* argument : init->arguments)
		{
			if (creates_temporaries(argument))
				return true;
		}
		return false;
	}
	return single != nullptr && creates_temporaries(single);
}

void FunctionLowering::local_static_variable(const sema::VariableEntity* variable, const syntax::InitDeclarator& init)
{
	// The object is its global: the name designates it; a first pass
	// through the declaration initializes it unless its initialization
	// was constant.
	const uint32_t location = init.declarator != nullptr ? init.declarator->location : 0;
	lowir::SymbolId guard = lowir::kNoId;
	const lowir::SymbolId global = lowering_.local_static(variable, function_, local_static_ordinal_++,
		init.initializer, location, guard, initializer_temporaries(variable, init.initializer) ? &init : nullptr);
	Local local;
	local.reference = variable->type->is_reference();
	local.slot = lowir::kNoId;
	local.address = lowir::Value::symbol(global);
	local.depth = 0;
	local_index_.set(variable, static_cast<uint32_t>(locals_.size()));
	locals_.push_back(local);
	if (guard == lowir::kNoId || unreachable_storage_)
		return;
	const lowir::Type i64 = lowir::Type::scalar(lowir::TypeKind::I64);
	Place flag = Place();
	flag.storage = lowir::Value::symbol(guard);
	const lowir::BlockId ready = new_block(lowir::BlockKind::LocalStaticReady);
	const lowir::BlockId initialize = new_block(lowir::BlockKind::LocalStaticInit);
	// Host objects: thread-safe (6.7p4), the guard's first byte tested
	// first as g++'s units do (the other bytes are __cxa_guard_acquire's),
	// then acquired; an exception leaving the initialization aborts it
	// (the guard's cleanup, under the initializer's temporaries).
	const bool thread_safe = lowering_.host_objects() && !variable->is_thread_local;
	size_t aborting = SIZE_MAX;
	if (thread_safe)
	{
		const lowir::Type i8 = lowir::Type::scalar(lowir::TypeKind::I8);
		const lowir::Type i32 = lowir::Type::scalar(lowir::TypeKind::I32);
		const lowir::BlockId acquire = new_block(lowir::BlockKind::LocalStaticCtorRun);
		Place first = Place();
		first.storage = address_of(flag);
		branch(compare(lowir::ComparePredicate::Ne, i8, load(i8, first), integer(i8, 0)), ready, acquire);
		place(acquire);
		const RValue acquired = guard_call(ProgramLowering::GuardCall::Acquire, guard);
		branch(compare(lowir::ComparePredicate::Ne, i32, acquired, integer(i32, 0)), initialize, ready);
		place(initialize);
		aborting = cleanups_.size();
		cleanups_.push_back(
			Cleanup{ lowir::Value::symbol(guard), lowering_.graph().types.fundamental(FundamentalType::Void), nullptr,
				lowir::kNoId, lowir::kNoId, lowir::kNoId, false });
		cleanups_.back().aborts_guard = true;
		split_region();
	}
	else
	{
		branch(compare(lowir::ComparePredicate::Ne, i64, load(i64, flag), integer(i64, 0)), ready, initialize);
		place(initialize);
	}
	const sema::Type* type = variable->type;
	Place storage = Place();
	storage.storage = lowir::Value::symbol(global);
	Place object = Place();
	object.storage = address_of(storage);
	qualify(object, type);
	const bool saved = static_local_;
	static_local_ = true;
	const ProgramLowering::TemporaryNames names = lowering_.begin_temporary_names(variable, global);
	// (A reference's temporaries, or a std::initializer_list's array: the
	// course LowIR destroys them with the static.)
	StaticTemporaries temporaries = StaticTemporaries{ local.reference, std::vector<Finalizer>() };
	StaticTemporaries* const saved_temporaries = static_temporaries_;
	const syntax::BracedInitList* listed = list_variable(variable, init.initializer);
	if (local.reference || listed != nullptr)
		static_temporaries_ = &temporaries;
	// The course plans a frame slot for a std::initializer_list's array
	// though the static's is a global.
	if (listed != nullptr && !listed->elements.empty() && !lowering_.object_mode())
		generated_slot(lowir::SlotKind::Initlist, lowering_.storage_type(listed->facts->target));
	const StaticInitialization initialization{ variable, global, guard, location, aborting, thread_safe, temporaries };
	if (!local.reference && object_class(type) != nullptr)
		local_object(variable, object, init.initializer, location, nullptr, &initialization);
	else if (init.initializer != nullptr)
	{
		// A reference's temporaries persist with it (12.2p5); the
		// initializer is a full-expression (1.9p10).
		const syntax::Initializer* written = init.initializer;
		const syntax::Expr* single = written->kind == syntax::InitializerKind::Parenthesized
			? (written->arguments.size == 1 ? written->arguments[0] : nullptr)
			: written->expression;
		size_t first = cleanups_.size();
		const bool region = begin_full_expression(single);
		const syntax::Expr* saved_root = full_root(single);
		if (local.reference)
		{
			single = reference_initializer(single);
			extension_path(single);
			RValue address;
			address.value = bind_reference(*single, type, ConversionContext::Initialization, true);
			persistent_path_.clear();
			address.type = lowir::Type::scalar(lowir::TypeKind::Ptr);
			store(address.type, address, object);
		}
		else
			initialize_storage(type, object, written, location);
		choice_root_ = saved_root;
		complete_static_initialization(initialization, first);
		destroy_temporaries(first);
		end_full_expression(region);
	}
	else
	{
		size_t first = cleanups_.size();
		complete_static_initialization(initialization, first);
	}
	lowering_.end_temporary_names(names);
	static_local_ = saved;
	static_temporaries_ = saved_temporaries;
	jump(ready);
	place(ready);
}

void FunctionLowering::complete_static_initialization(const StaticInitialization& initialization, size_t& first)
{
	// 6.7p4: the variable is initialized before its initializer's temporary
	// destructors run (12.2p3). Their failure must neither retry construction
	// nor abort a released host guard; destruction is already registered.
	if (tracked_.active)
		finish_tracking();
	const size_t aborting = initialization.aborting;
	if (aborting != SIZE_MAX)
	{
		close_region();
		cleanups_.erase(cleanups_.begin() + static_cast<std::ptrdiff_t>(aborting));
		--first;
		if (tracked_.active && tracked_.floor > aborting)
			--tracked_.floor;
		if (construction_depth_ > aborting)
			--construction_depth_;
		if (segment_floor_ > aborting)
			--segment_floor_;
		for (size_t i = aborting; i < cleanups_.size(); ++i)
		{
			cleanups_[i].unwind = cleanups_[i].shared_return = cleanups_[i].conditional = lowir::kNoId;
			cleanups_[i].eh_marked = cleanups_[i].eh_unmarked = 0;
		}
	}
	register_static_temporaries(initialization.global, initialization.temporaries);
	const sema::VariableEntity* variable = initialization.variable;
	if (needs_destruction(variable->type) && !variable->type->is_reference())
		register_destruction(initialization.global, variable->type, initialization.location,
			variable->is_thread_local && lowering_.object_mode());
	if (initialization.thread_safe)
		guard_call(ProgramLowering::GuardCall::Release, initialization.guard);
	else
	{
		const lowir::Type i64 = lowir::Type::scalar(lowir::TypeKind::I64);
		Place flag = Place();
		flag.storage = lowir::Value::symbol(initialization.guard);
		store(i64, integer(i64, 1), flag);
	}
}

RValue FunctionLowering::guard_call(ProgramLowering::GuardCall call, lowir::SymbolId guard)
{
	Place flag = Place();
	flag.storage = lowir::Value::symbol(guard);
	const size_t base = arguments_.size();
	arguments_.push_back(address_of(flag));
	RValue result;
	result.type = lowir::Type::scalar(
		call == ProgramLowering::GuardCall::Acquire ? lowir::TypeKind::I32 : lowir::TypeKind::Void);
	result.value = emit_call(
		result.type, lowir::Value::symbol(lowering_.guard_function(call)), base, lowir::kNoId, false);
	return result;
}

bool FunctionLowering::static_temporary(lowir::SymbolId storage, const sema::Type* type, uint32_t location)
{
	// A temporary of the course LowIR's static reference (12.2p5), or the
	// array of a block-scope static std::initializer_list variable (8.5.4p6),
	// constructed: a reference's flag is set now, and it is destroyed if
	// made, last first, by the finalizer (a namespace-scope reference's) or
	// by the one function the static registers once its initialization
	// completes (3.6.3p1, as the course destroys them).
	if (static_temporaries_ == nullptr || lowering_.object_mode())
		return false;
	const lowir::Type i8 = lowir::Type::scalar(lowir::TypeKind::I8);
	Place flag = Place();
	if (!static_local_)
	{
		flag.storage = lowir::Value::symbol(lowering_.static_temporary_constructed(storage, type, location));
		store(i8, integer(i8, 1), flag);
		return true;
	}
	Finalizer made = lowering_.finalizer_for(storage, type, location);
	if (static_temporaries_->reference)
	{
		made.reference_temporary = true;
		made.constructed = lowering_.constructed_flag(storage);
		flag.storage = lowir::Value::symbol(made.constructed);
		store(i8, integer(i8, 1), flag);
	}
	static_temporaries_->made.push_back(made);
	return true;
}

void FunctionLowering::register_static_temporaries(lowir::SymbolId global, const StaticTemporaries& temporaries)
{
	if (temporaries.made.empty())
		return;
	const size_t base = arguments_.size();
	arguments_.push_back(lowir::Value::symbol(lowering_.local_static_destructor(global, temporaries.made)));
	emit_call(lowir::Type::scalar(lowir::TypeKind::I32), lowir::Value::symbol(lowering_.atexit_function()), base,
		lowir::kNoId, false);
}

void FunctionLowering::register_destruction(
	lowir::SymbolId object, const sema::Type* type, uint32_t location, bool thread)
{
	// 3.6.3p1: objects of static storage duration are destroyed in the
	// reverse order of the completion of their construction, so one
	// constructed on a first pass has its destructor registered now; so are
	// a thread's objects, all before any of static storage duration: on the
	// thread's own list (the destructor finds the thread's object itself).
	const size_t base = arguments_.size();
	arguments_.push_back(lowir::Value::symbol(lowering_.local_static_destructor(object, type, location)));
	if (thread)
	{
		// The object (the destructor finds it) and the module's handle:
		// glibc keeps the module loaded while a thread's destructors are
		// pending and needs a real one (a null handle crashes it).
		const lowir::Type ptr = lowir::Type::scalar(lowir::TypeKind::Ptr);
		arguments_.push_back(integer(ptr, 0).value);
		arguments_.push_back(lowir::Value::symbol(lowering_.dso_handle()));
	}
	const lowir::SymbolId registrar = thread ? lowering_.thread_atexit_function() : lowering_.atexit_function();
	emit_call(lowir::Type::scalar(lowir::TypeKind::I32), lowir::Value::symbol(registrar), base, lowir::kNoId, false);
}

} // namespace lower
