#include "lower/program_lowering.h"

#include <algorithm>
#include <cstdlib>
#include <cstring>
#include <string>

#include "lex/identifier_table.h"
#include "lower/function_lowering.h"
#include "lower/debug_info.h"
#include "preprocess/preprocessor.h"
#include "sema/aggregates.h"
#include "sema/analyzer.h"
#include "support/stack_budget.h"

namespace lower {

namespace {

bool HasExternSpecifier(const syntax::Slice<syntax::DeclSpecifier>& specifiers)
{
	for (const syntax::DeclSpecifier& specifier : specifiers)
	{
		if (specifier.kind == syntax::DeclSpecKind::Keyword && specifier.keyword == TokenKind::KW_EXTERN)
			return true;
	}
	return false;
}

// From this many direct bases a class with virtual bases has its
// non-virtual ones listed for the base searches (a few are scanned).
const uint32_t kListedBases = 8;

} // namespace

ProgramLowering::ProgramLowering(lowir::Program& program)
	: program_(program), graph_(nullptr), analyzer_(nullptr), local_identities_(0), init_symbol_(lowir::kNoId),
	  string_literals_(0), linkage_extern_(false), static_scope_(nullptr), slot_epoch_(0), stream_(nullptr),
	  stack_base_(0), stack_budget_(stack_budget_bytes())
{
	entry_name_ = intern("entry", 5);
	this_name_ = intern("this", 4);
	ret_name_ = intern("ret", 3);
	other_name_ = intern("other", 5);
	unnamed_slot_name_ = intern("__slot", 6);
	range_for_names_[0] = intern("__range", 7);
	range_for_names_[1] = intern("__begin", 7);
	range_for_names_[2] = intern("__end", 5);
	range_for_names_[3] = intern("__idx", 5);
}

ProgramLowering::~ProgramLowering()
{}

void ProgramLowering::lower_unit(
	const syntax::TranslationUnit& unit, sema::SemanticGraph& graph, sema::Analyzer& analyzer)
{
	// Entity identities are addresses in this unit's arena: the maps keyed
	// by them start empty for each unit.
	char marker;
	stack_base_ = reinterpret_cast<std::uintptr_t>(&marker);
	graph_ = &graph;
	analyzer_ = &analyzer;
	positions_ = debug_locations_ ? analyzer.source_positions() : nullptr;
	if (positions_ != nullptr && debug_types_)
	{
		if (!program_.debug)
			program_.debug.reset(new lowir::DebugInfo());
		source_debug_.reset(new DebugInfo(*this));
	}
	debug_file_.clear();
	debug_file_name_ = 0;
	symbols_.clear();
	identities_.clear();
	queued_.clear();
	queued_base_.clear();
	queued_deleting_.clear();
	base_symbols_.clear();
	deleting_symbols_.clear();
	vtables_.clear();
	typeinfos_.clear();
	typeinfo_names_.clear();
	type_informations_.clear();
	exception_informations_.clear();
	thrown_types_.clear();
	defined_informations_.clear();
	virtual_signatures_.clear();
	itanium_vbase_positions_.clear();
	itanium_row_lists_.clear();
	itanium_row_heads_.clear();
	itanium_row_store_.clear();
	itanium_vcall_positions_.clear();
	group_segments_.clear();
	caller_destroyed_types_.clear();
	element_temporaries_.clear();
	data_member_pointer_classes_.clear();
	changeable_classes_.clear();
	null_piece_spans_.clear();
	null_spans_.clear();
	null_piece_store_.clear();
	base_misses_.clear();
	constant_temporaries_.clear();
	tls_wrappers_.clear();
	inert_destructors_.clear();
	contained_destructors_.clear();
	direct_classes_.clear();
	object_classes_.clear();
	constructor_free_.clear();
	empty_list_constructs_.clear();
	rooted_destructors_.clear();
	aggregate_helpers_.clear();
	helper_index_.clear();
	pending_helper_ = 0;
	thunks_.clear();
	pending_thunk_ = 0;
	thunk_symbols_.clear();
	definition_thunk_classes_.clear();
	definition_thunk_seen_.clear();
	construction_sensitive_.clear();
	result_adjustments_.clear();
	definition_thunk_heads_.clear();
	definition_thunks_.clear();
	view_tables_.clear();
	view_symbols_.clear();
	offset_tables_.clear();
	segment_offsets_.clear();
	external_groups_.clear();
	external_points_.clear();
	vbase_rows_.clear();
	vbase_row_store_.clear();
	virtual_base_places_.clear();
	base_routes_.clear();
	nonvirtual_lists_.clear();
	nonvirtual_store_.clear();
	virtual_holders_.clear();
	cast_hints_.clear();
	hint_class_ = nullptr;
	virtual_orders_.clear();
	virtual_order_store_.clear();
	vtt_sizes_.clear();
	vtts_.clear();
	vtt_base_positions_.clear();
	vtt_segments_.clear();
	vtt_segment_store_.clear();
	vtt_orders_.clear();
	vtt_order_store_.clear();
	construction_tables_.clear();
	construction_symbols_.clear();
	construction_points_.clear();
	context_owners_.clear();
	context_owner_store_.clear();
	demanded_.clear();
	reference_index_.clear();
	reference_targets_.clear();
	overload_ordinals_.clear();
	call_signatures_.clear();
	nothrow_call_signatures_.clear();
	if (string_object_count_ != 0)
		string_objects_.assign(string_objects_.size(), StringObject{ 0, 0 });
	string_object_count_ = 0;
	abi_graph_.reset();
	encoder_.reset(new abi_mangle::AbiEncoder(abi_graph_));
	abi_types_.clear();
	class_names_.clear();
	placeholder_nodes_.clear();
	parameter_declarations_.clear();
	head_declaration_index_.clear();
	head_declarations_.clear();
	localities_.clear();
	unnamed_ordinals_.clear();
	anonymous_ordinals_.clear();
	anonymous_counts_.clear();
	anonymous_total_ = 0;
	local_discriminators_.clear();
	local_name_counts_.clear();
	local_class_heads_.clear();
	local_class_next_.clear();
	local_classes_linked_ = false;
	shared_locals_.clear();
	label_identities_.clear();
	class_spelling_index_.clear();
	class_spellings_.clear();
	constexpr_images_.clear();
	local_static_index_.clear();
	local_statics_.clear();
	if (init_)
		init_->begin_unit();
	++counters_.units;
	const size_t first_pending = pending_.size();
	for (const syntax::Decl* decl : unit.declarations)
	{
		declaration(*decl);
		if (graph.unnamed_member_definitions)
			name_deferred_members(*decl);
	}
	// A delegating constructor namespace-scope code odr-uses is emitted,
	// with the target its body calls (the course's form).
	for (const sema::DemandList* node = graph.root_demands; node != nullptr; node = node->next)
	{
		if (node->function->constructor && node->function->delegation != nullptr)
			demand(node->function);
	}
	// The course view includes functions named only as template arguments.
	// Object emission follows actual code/data references; semantic analysis
	// already instantiated and checked these argument-only definitions.
	// Explicit GNU used attributes still require a physical definition.
	for (uint32_t i = analyzer.argument_demand_head(nullptr); i != 0; i = analyzer.argument_demand(i).previous)
		if (!object_mode_ || analyzer.argument_demand(i).function->used)
			demand(analyzer.argument_demand(i).function);
	emit_demanded();
	// Members of explicitly instantiated classes (14.7.2p8, object roots),
	// with the entries the units seeing an explicit instantiation
	// declaration call (p10): a constructor's or destructor's base entry
	// that is code of its own (its class has virtual bases: Itanium
	// 5.1.4.3's C2/D2 taking the VTT) besides the complete entry;
	// instances' static data members named in the unit (defined by their
	// first reference), and what their definitions demand in turn.
	for (const sema::FunctionEntity* function : analyzer.explicit_instantiations())
	{
		if ((function->constructor || function->destructor) && HiddenBaseParameters(function, EntryVariant::Base))
			demand(function, EntryVariant::Base);
		demand(function);
	}
	// Their polymorphic classes' table groups, VTTs and type information
	// (Itanium 5.2.3: the explicit instantiation's unit; the units seeing
	// an explicit instantiation declaration refer to them), even with no
	// constructor or destructor emitted here to refer to them.
	for (const sema::ClassEntity* cls : analyzer.explicit_class_instantiations())
		class_table(cls);
	emit_demanded();
	for (size_t i = 0; i < analyzer.demanded_variables().size(); ++i)
	{
		// A host object defines a member read only as a constant where it
		// is named (3.2p2: no odr-use) or in a body it does not emit when
		// its code or data refers to it (variable_symbol), unless an
		// explicit instantiation names it (README PA31: demand-driven
		// emission; the course's form defines it where named).
		const sema::VariableEntity* variable = analyzer.demanded_variables()[i];
		if (host_objects_ && !variable->templ->explicit_definition &&
			variable->constant == sema::ConstantState::Value && !variable->type->is_reference())
			continue;
		variable_symbol(variable);
		emit_demanded();
	}
	for (const lowir::SymbolId symbol : held_definitions_)
		release_definition(symbol);
	held_definitions_.clear();
	state_pending_signatures(first_pending);
	if (source_debug_)
		source_debug_->finish(graph.global);
	source_debug_.reset();
	graph_ = nullptr;
	analyzer_ = nullptr;
	positions_ = nullptr;
}

lowir::DebugLocation ProgramLowering::debug_location(uint32_t location)
{
	// The presumed file name and line (16.4: as __FILE__ and __LINE__ give
	// them) and the byte column. lowir.md: the file is one LowIR token.
	const lowir::DebugLocation none = lowir::DebugLocation{ 0, 0, 0 };
	if (positions_ == nullptr)
		return none;
	std::string file = positions_->file_of(location);
	if (file != debug_file_)
	{
		debug_file_name_ = 0;
		bool token = !file.empty();
		for (const char c : file)
			token = token && c != ' ' && c != '\t' && c != '\n' && c != '\r' && c != ',' && c != '(' && c != ')';
		if (token)
			debug_file_name_ = intern(file.data(), static_cast<uint32_t>(file.size()));
		debug_file_.swap(file);
	}
	const uint64_t line = positions_->line_of(location);
	const uint64_t column = positions_->column_of(location);
	if (debug_file_name_ == 0 || line == 0 || column == 0 || line > 0xffffffffu || column > 0xffffffffu)
		return none;
	return lowir::DebugLocation{ debug_file_name_, static_cast<uint32_t>(line), static_cast<uint32_t>(column) };
}

void ProgramLowering::finish()
{
	// A declaration for each symbol referenced and never defined, in the
	// order of first reference.
	define_unit_thread_local_init();
	resolve_singleton_roles();
	for (const PendingDeclaration& pending : pending_)
	{
		lowir::Symbol& symbol = program_.symbols[pending.symbol];
		if (symbol.kind != lowir::SymbolKind::Unresolved)
			continue;
		if (pending.function)
			program_.declare_function(pending.symbol, lowir::SymbolKind::FunctionDeclaration, pending.signature);
		else
		{
			lowir::Global global = lowir::Global();
			global.typed = pending.typed;
			global.type = pending.type;
			global.init = lowir::GlobalInit::None;
			program_.declare_global(pending.symbol, lowir::SymbolKind::GlobalDeclaration, global);
		}
		program_.symbols[pending.symbol].metadata = pending.metadata;
		++counters_.declarations;
	}
	pending_.clear();
	// Object mode: a declaration binds as the object's symbol will. A
	// vague-linkage entity's (binding=weak in the course's LowIR) is a
	// reference some unit must define, a global undefined symbol (a weak
	// one would link as address 0); only the references Itanium makes weak
	// stay so (thread-local entries, weak_references_).
	if (object_mode_)
	{
		for (lowir::Symbol& symbol : program_.symbols)
		{
			if ((symbol.kind == lowir::SymbolKind::FunctionDeclaration ||
					symbol.kind == lowir::SymbolKind::GlobalDeclaration) &&
				symbol.metadata.binding == ir_model::SBM_WEAK)
				symbol.metadata.binding = ir_model::SBM_STRONG;
		}
		for (const lowir::SymbolId symbol : weak_references_)
		{
			if (program_.symbols[symbol].kind == lowir::SymbolKind::FunctionDeclaration)
				program_.symbols[symbol].metadata.binding = ir_model::SBM_WEAK;
		}
	}
	weak_references_.clear();
	// A constructor's or destructor's base entry is its complete entry's
	// code under a second object name, unless defined on its own (a call
	// that declared the base entry, an instantiated destructor's from a
	// derived class's, is bound to the alias).
	for (const EntryAlias& alias : entry_aliases_)
	{
		const lowir::SymbolId defined = alias.object < by_object_.size() ? by_object_[alias.object] : lowir::kNoId;
		if (defined != lowir::kNoId && defined != kLocalObject &&
			program_.symbols[defined].kind == lowir::SymbolKind::FunctionDefinition)
			continue;
		if (defined == kLocalObject)
			continue;
		program_.aliases.push_back(lowir::ObjectAlias{ alias.object, alias.target });
		if (by_object_.size() <= alias.object)
			by_object_.resize(std::max<size_t>(alias.object + 1, by_object_.size() * 2), lowir::kNoId);
		by_object_[alias.object] = kLocalObject;
	}
	entry_aliases_.clear();
	if (init_)
	{
		init_->finish_initializer();
		init_.reset();
		release_definition(init_symbol_);
	}
	finalizer_function();
	order_completed_definitions();
}

void ProgramLowering::order_completed_definitions()
{
	// A program kept whole lists its function definitions as they were
	// completed, as a streamed one (whose consumer had each at its
	// completion: the LowIR writer, an object back end) does, so a program
	// read back from its text is in the same order: the other entries stay.
	// The special-member families then take their order where their members
	// stand (order_special_member_families; not in an object's layout).
	if (completed_.empty())
		return;
	if (stream_ != nullptr)
	{
		// The stream's appends, one per completion.
		std::vector<lowir::SymbolId> order = completed_;
		order_special_member_families(order);
		if (order != completed_)
		{
			std::vector<uint32_t> append_index(program_.symbols.size(), 0);
			for (uint32_t i = static_cast<uint32_t>(completed_.size()); i-- > 0;)
				append_index[completed_[i]] = i;
			std::vector<uint32_t> appends(order.size());
			for (size_t i = 0; i < order.size(); ++i)
				appends[i] = append_index[order[i]];
			stream_->reorder(appends);
		}
		completed_.clear();
		return;
	}
	size_t definitions = 0;
	for (const lowir::SymbolId symbol : program_.order)
		definitions += program_.symbols[symbol].kind == lowir::SymbolKind::FunctionDefinition ? 1 : 0;
	std::vector<uint8_t> seen(program_.symbols.size(), 0);
	std::vector<lowir::SymbolId> completed;
	completed.reserve(definitions);
	for (const lowir::SymbolId symbol : completed_)
	{
		if (program_.symbols[symbol].kind != lowir::SymbolKind::FunctionDefinition || seen[symbol])
			continue;
		seen[symbol] = 1;
		completed.push_back(symbol);
	}
	if (completed.size() != definitions)
		return;
	order_special_member_families(completed);
	size_t next = 0;
	for (lowir::SymbolId& symbol : program_.order)
		if (program_.symbols[symbol].kind == lowir::SymbolKind::FunctionDefinition)
			symbol = completed[next++];
	completed_.clear();
}

void ProgramLowering::resolve_singleton_roles()
{
	// pa8/lowir.md: one symbol owns each singleton role. The runtime's
	// scalar allocation functions (or the program's definitions) own
	// allocate_memory and free_memory, std::terminate owns terminate; an
	// array form (18.6.1.2p4, p13: by default the scalar one's behavior)
	// or __builtin_abort (the default terminate handler aborts) still
	// undefined becomes an internal body calling the owner, and owns the
	// role only without one (a replaced array form never has it:
	// function_metadata). In object mode a unit is not the program: another
	// unit may replace the array forms (17.6.4.6p3, 18.6.1.2), so they stay
	// references to their own names, whose link-time defaults call the
	// scalar forms (18.6.1.2p4, p13), without the roles (function_metadata:
	// one owner per role, pa8/lowir.md); __builtin_abort is the C library's
	// abort there.
	const uint8_t roles[] = { ir_model::SR_ALLOCATE_MEMORY, ir_model::SR_FREE_MEMORY, ir_model::SR_TERMINATE };
	for (const uint8_t role : roles)
	{
		if (object_mode_ && role != ir_model::SR_TERMINATE)
			continue;
		lowir::SymbolId owner = lowir::kNoId;
		int owner_rank = -1;
		uint32_t count = 0;
		const auto consider = [&](lowir::SymbolId id, const lowir::SymbolMetadata& metadata, bool defined) {
			if (metadata.role != role)
				return;
			++count;
			const bool secondary = id < secondary_roles_.size() && secondary_roles_[id];
			const int rank = (defined ? 2 : 0) + (secondary ? 0 : 1);
			if (rank > owner_rank)
			{
				owner = id;
				owner_rank = rank;
			}
		};
		for (lowir::SymbolId id = 0; id < program_.symbols.size(); ++id)
		{
			const lowir::Symbol& symbol = program_.symbols[id];
			if (symbol.kind == lowir::SymbolKind::FunctionDefinition ||
				symbol.kind == lowir::SymbolKind::FunctionDeclaration)
				consider(id, symbol.metadata, symbol.kind == lowir::SymbolKind::FunctionDefinition);
		}
		for (const PendingDeclaration& pending : pending_)
		{
			if (program_.symbols[pending.symbol].kind == lowir::SymbolKind::Unresolved)
				consider(pending.symbol, pending.metadata, false);
		}
		if (count < 2)
			continue;
		for (PendingDeclaration& pending : pending_)
		{
			lowir::Symbol& symbol = program_.symbols[pending.symbol];
			if (pending.symbol == owner || pending.metadata.role != role ||
				symbol.kind != lowir::SymbolKind::Unresolved)
				continue;
			pending.metadata.role = ir_model::SR_NONE;
			pending.metadata.binding = ir_model::SBM_INTERNAL;
			pending.metadata.object = 0;
			program_.declare_function(pending.symbol, lowir::SymbolKind::FunctionDefinition, pending.signature);
			program_.symbols[pending.symbol].metadata = pending.metadata;
			{
				FunctionLowering body(*this, pending.symbol, nullptr);
				body.lower_forwarder(pending.symbol, owner);
			}
			release_definition(pending.symbol);
			++counters_.functions;
		}
		for (lowir::SymbolId id = 0; id < program_.symbols.size(); ++id)
		{
			lowir::Symbol& symbol = program_.symbols[id];
			if (id != owner && symbol.metadata.role == role && symbol.kind == lowir::SymbolKind::FunctionDefinition)
				symbol.metadata.role = ir_model::SR_NONE;
		}
	}
}

void ProgramLowering::release_definition(lowir::SymbolId symbol)
{
	// The order of completion; a stream's appends are ordered again at the
	// program's end (order_completed_definitions).
	if (stream_ == nullptr || !object_mode_)
		completed_.push_back(symbol);
	if (stream_ == nullptr)
		return;
	if (stream_->needs_boundaries())
		state_forward_boundaries(symbol);
	stream_->append(program_, symbol);
	program_.functions[program_.symbols[symbol].detail].body.reset();
}

void ProgramLowering::state_forward_boundaries(lowir::SymbolId symbol)
{
	// A pending callee's boundary: stated at its unit's end, else from its
	// entity while the unit's types exist (the same one finish declares).
	const lowir::FunctionBody& body = *program_.functions[program_.symbols[symbol].detail].body;
	for (const lowir::Instruction& inst : body.instructions)
	{
		if (inst.op != lowir::Opcode::Call || (inst.flags & lowir::IF_SIGNATURE) != 0 ||
			!inst.ops[0].is(lowir::ValueKind::Symbol))
			continue;
		const lowir::SymbolId callee = inst.ops[0].index;
		if (program_.symbols[callee].kind != lowir::SymbolKind::Unresolved ||
			program_.forward_boundary(callee) != lowir::kNoId)
			continue;
		const uint32_t index = callee < pending_index_.size() ? pending_index_[callee] : 0;
		if (index == 0 || !pending_[index - 1].function)
			continue;
		PendingDeclaration& pending = pending_[index - 1];
		if (pending.signature == lowir::kNoId && pending.entity != nullptr)
			pending.signature = function_signature(pending.entity, false, pending.variant);
		if (pending.signature != lowir::kNoId)
			program_.set_forward_boundary(callee, pending.signature);
	}
}

// ------------------------------------------------------------ declarations

void ProgramLowering::declarations(const syntax::Slice<syntax::Decl*>& list)
{
	for (const syntax::Decl* decl : list)
		declaration(*decl);
}

void ProgramLowering::declaration(const syntax::Decl& decl)
{
	switch (decl.kind)
	{
	case syntax::DeclKind::Asm:
		return asm_declaration(static_cast<const syntax::AsmDecl&>(decl));
	case syntax::DeclKind::Namespace:
		declarations(static_cast<const syntax::NamespaceDecl&>(decl).declarations);
		return;
	case syntax::DeclKind::Linkage:
	{
		// 7.5p7: a declaration directly in an unbraced linkage-specification
		// is treated as extern.
		const syntax::LinkageDecl& linkage = static_cast<const syntax::LinkageDecl&>(decl);
		const bool saved = linkage_extern_;
		linkage_extern_ = !linkage.braced;
		declarations(linkage.declarations);
		linkage_extern_ = saved;
		return;
	}
	case syntax::DeclKind::Simple:
		simple_declaration(static_cast<const syntax::SimpleDecl&>(decl));
		return;
	case syntax::DeclKind::Class:
		// An unnamed namespace's anonymous union need not be static (9.5p5):
		// its unnamed object.
		if (const sema::VariableEntity* object = anonymous_object(decl))
			define_variable(object, nullptr, decl.location);
		return;
	case syntax::DeclKind::Template:
	{
		// An explicit specialization (14.7.3) is an ordinary definition,
		// except that one of a static data member without an initializer is
		// a declaration (p13).
		const syntax::TemplateDecl& templ = static_cast<const syntax::TemplateDecl&>(decl);
		if (!templ.parameters->parameters.empty())
			return;
		if (templ.declaration->kind == syntax::DeclKind::Simple)
			simple_declaration(static_cast<const syntax::SimpleDecl&>(*templ.declaration), true);
		else
			declaration(*templ.declaration);
		return;
	}
	case syntax::DeclKind::Function:
	{
		const syntax::FunctionDecl& definition = static_cast<const syntax::FunctionDecl&>(decl);
		if (definition.declared == nullptr)
			return;
		const sema::FunctionEntity* function = static_cast<const sema::FunctionEntity*>(definition.declared->entity);
		if (emitted_at_definition(function))
			define_function(function, decl.location);
		return;
	}
	case syntax::DeclKind::SpecialMember:
	{
		// An out-of-class constructor or destructor definition (one in its
		// class is inline, emitted where odr-used).
		const syntax::SpecialMemberDecl& special = static_cast<const syntax::SpecialMemberDecl&>(decl);
		const bool defaulted = special.initializer != nullptr &&
			special.initializer->kind == syntax::InitializerKind::Default;
		if (!(special.definition || defaulted) || special.declared == nullptr)
			return;
		const sema::FunctionEntity* function = static_cast<const sema::FunctionEntity*>(special.declared->entity);
		if (emitted_at_definition(function))
			define_function(function, decl.location);
		return;
	}
	default:
		// Classes and enumerations (members wait for their stages), using
		// declarations, aliases, static assertions and templates (whose
		// specializations a later stage emits) define no code or data.
		return;
	}
}

const sema::VariableEntity* ProgramLowering::anonymous_object(const syntax::Decl& decl)
{
	// `static union { ... };` is a simple declaration, a block's
	// `union { ... };` a class declaration.
	if (decl.kind == syntax::DeclKind::Class)
	{
		const syntax::DeclSpecifier& specifier = static_cast<const syntax::ClassDecl&>(decl).specifier;
		if (specifier.kind == syntax::DeclSpecKind::Class && specifier.class_specifier->storage != nullptr)
			return static_cast<const sema::VariableEntity*>(specifier.class_specifier->storage->entity);
		return nullptr;
	}
	if (decl.kind != syntax::DeclKind::Simple)
		return nullptr;
	for (const syntax::DeclSpecifier& specifier : static_cast<const syntax::SimpleDecl&>(decl).specifiers)
	{
		if (specifier.kind == syntax::DeclSpecKind::Class && specifier.class_specifier->storage != nullptr)
			return static_cast<const sema::VariableEntity*>(specifier.class_specifier->storage->entity);
	}
	return nullptr;
}

void ProgramLowering::simple_declaration(const syntax::SimpleDecl& decl, bool explicit_specialization)
{
	const bool is_extern = linkage_extern_ || HasExternSpecifier(decl.specifiers);
	if (const sema::VariableEntity* object = anonymous_object(decl))
		define_variable(object, nullptr, decl.location);
	for (const syntax::InitDeclarator& init : decl.declarators)
	{
		const sema::Binding* binding = init.declared;
		if (binding != nullptr && binding->kind == sema::BindingKind::Function && init.initializer != nullptr &&
			init.initializer->kind == syntax::InitializerKind::Default)
		{
			// 8.4.2p5: a function explicitly defaulted after its first
			// declaration is defined here.
			const sema::FunctionEntity* function = static_cast<const sema::FunctionEntity*>(binding->entity);
			if (emitted_at_definition(function))
				define_function(function, decl.location);
			continue;
		}
		if (binding == nullptr || binding->kind != sema::BindingKind::Variable)
			continue;
		const sema::VariableEntity* variable = static_cast<const sema::VariableEntity*>(binding->entity);
		// Namespace-scope variables, and static data members defined outside
		// their class (9.4.2p2).
		if (variable->member || variable->owner == nullptr ||
			(variable->owner->kind != sema::ScopeKind::Namespace && variable->owner->kind != sema::ScopeKind::Class))
			continue;
		// 3.1p2: an extern declaration without an initializer only declares;
		// 14.7.3p13: so does a static data member's explicit specialization.
		if ((is_extern || (explicit_specialization && variable->owner->kind == sema::ScopeKind::Class)) &&
			init.initializer == nullptr)
			continue;
		const uint32_t location = init.declarator != nullptr ? init.declarator->location : decl.location;
		// A tuple-like structured binding's name is a reference variable
		// bound to the get<i>(e) the analysis made (C++17's 11.5p3; an
		// array's or class's names declare no variable).
		define_variable(variable, sema::VariableInitializer(variable, init.initializer), location);
	}
}

void ProgramLowering::define_variable(
	const sema::VariableEntity* variable, const syntax::Initializer* initializer, uint32_t location)
{
	const lowir::SymbolId symbol = variable_symbol(variable);
	if (program_.symbols[symbol].kind != lowir::SymbolKind::Unresolved)
		throw LoweringError(location, "redefinition of a variable across translation units");
	define_global(symbol, variable, initializer, location);
	++counters_.globals;
}

bool ProgramLowering::emitted_at_definition(const sema::FunctionEntity* function) const
{
	// An inline definition (7.1.2p4: every unit that odr-uses it defines
	// it) is emitted where odr-used; so is an internal always_inline one,
	// or one whose body calls a reserved builtin this compiler lacks (as
	// GCC and Clang emit an internal function: Clang's AMX intrinsics are
	// static always_inline functions, not inline, of builtins this
	// compiler lacks); every other definition is emitted.
	return !function->is_inline &&
		!(function->internal_linkage && (function->unanalyzed_body || function->always_inline));
}

void ProgramLowering::define_function(const sema::FunctionEntity* function, uint32_t location)
{
	const lowir::SymbolId symbol = function_symbol(function);
	if (program_.symbols[symbol].kind != lowir::SymbolKind::Unresolved)
	{
		// An inline function one unit already emitted; any other is defined
		// twice (3.2p3).
		if (function->is_inline)
			return;
		throw LoweringError(location, "redefinition of a function across translation units");
	}
	queued_.set(function, 1);
	// Clang names the local classes of an internal function before its
	// body, which it emits on first use; another's after it (see
	// name_local_classes).
	const bool deferred = unit_local(function);
	if (deferred)
		name_local_classes(function);
	// A constructor or destructor defined here defines its base entry too
	// (before the complete entry), not an alias of it; a virtual
	// destructor its deleting entry next.
	if ((function->constructor || function->destructor) && queued_base_.find(function) == PointerMap::kMissing)
	{
		queued_base_.set(function, 1);
		emit_definition(function, EntryVariant::Base, entry_symbol(function, EntryVariant::Base));
	}
	if (function->destructor && function->is_virtual && queued_deleting_.find(function) == PointerMap::kMissing)
	{
		queued_deleting_.set(function, 1);
		emit_definition(function, EntryVariant::Deleting, entry_symbol(function, EntryVariant::Deleting));
	}
	// A defaulted constructor that does nothing (trivial, never called)
	// defines its base entry only (the course's form).
	if (function->constructor && function->synthesized && function->trivial)
		return;
	emit_definition(function, EntryVariant::Complete, symbol);
	if (!deferred)
		name_local_classes(function);
	// Itanium 5.2.3: the key function's unit defines the class's table.
	const sema::ClassEntity* owner = function->member ? sema::ClassOf(function->owner->entity->type) : nullptr;
	if (owner != nullptr && owner->virtuals != nullptr && owner->virtuals->key_function == function)
		vtable_symbol(owner);
}

void ProgramLowering::emit_definition(
	const sema::FunctionEntity* function, EntryVariant variant, lowir::SymbolId symbol)
{
	const lowir::SignatureId signature = function_signature(function, true, variant);
	program_.declare_function(symbol, lowir::SymbolKind::FunctionDefinition, signature);
	program_.symbols[symbol].metadata = function_metadata(function, symbol);
	// An always_inline function's unit-local copy (inline_copy) is internal
	// from the start: the body streamed below carries its binding.
	const bool copy = inline_copies_.find(function) == symbol;
	if (copy)
		program_.symbols[symbol].metadata.binding = ir_model::SBM_INTERNAL;
	// An explicit instantiation definition roots what it instantiates
	// (14.7.2p5: a prior explicit specialization is not instantiated).
	if ((variant == EntryVariant::Base && !function->is_constexpr) ||
		rooted_destructors_.find(function) != PointerMap::kMissing ||
		(!copy && function->explicit_instantiation && (function->instantiated || !function->is_inline)))
		program_.symbols[symbol].metadata.flags |= lowir::SF_OBJECT_ROOT;
	// An internal virtual function is referenced by its class's table,
	// data no instruction reads: an object root.
	if (function->is_virtual && !function->destructor &&
		program_.symbols[symbol].metadata.binding == ir_model::SBM_INTERNAL)
		program_.symbols[symbol].metadata.flags |= lowir::SF_OBJECT_ROOT;
	// A base entry with the hidden parameters of a class with virtual bases
	// is code of its own, never the complete entry's.
	lowir::NameId base_object = 0;
	if (variant == EntryVariant::Complete && (function->constructor || function->destructor) &&
		!HiddenBaseParameters(function, EntryVariant::Base))
	{
		// An internal complete entry is its base entry's root too.
		if (program_.symbols[symbol].metadata.binding == ir_model::SBM_INTERNAL && !function->is_constexpr)
			program_.symbols[symbol].metadata.flags |= lowir::SF_OBJECT_ROOT;
		const std::string base = function_object(function, EntryVariant::Base);
		base_object = intern(base.data(), static_cast<uint32_t>(base.size()));
		entry_aliases_.push_back(EntryAlias{ base_object, symbol });
	}
	note_family_members(function, variant, symbol, base_object);
	if (function->unanalyzed_body)
		throw LoweringError(function->body->body->location,
			"an emitted inline function that calls a reserved builtin this compiler lacks");
	{
		FunctionLowering body(*this, symbol, function, variant);
		body.lower_definition();
	}
	if (function->is_virtual && variant == EntryVariant::Complete)
		definition_thunks(function, program_.symbols[symbol].metadata.binding);
	// Preserve the course view's argument-only definitions. In object mode
	// only an actual runtime reference demands their code (see lower_unit).
	for (uint32_t i = analyzer_->argument_demand_head(function); i != 0; i = analyzer_->argument_demand(i).previous)
		if (!object_mode_ || analyzer_->argument_demand(i).function->used)
			demand(analyzer_->argument_demand(i).function);
	// An inert destructor's complete entry is written at the unit's end: a
	// later body omitting a call of it makes it an object root (only
	// inert ones are omitted). A program kept whole completes it there too.
	if (function->destructor && variant == EntryVariant::Complete &&
		destructor_does_nothing(static_cast<const sema::ClassEntity*>(function->owner->entity)))
		held_definitions_.push_back(symbol);
	else
		release_definition(symbol);
	++counters_.functions;
}

lowir::SymbolId ProgramLowering::inline_copy(const sema::FunctionEntity* function)
{
	// GNU always_inline: a call runs the body this unit sees, as GCC's and
	// Clang's inlining makes it whatever another unit defines (a weak
	// definition replaced, an inline one's other copies): its unit-local
	// copy, emitted once (its address, its statics and other units' calls
	// are the function's own). Not for a function without a definition
	// here, a constructor or destructor (their entries), or a virtual one.
	// The copy replaces the function's own definition for calls (a unit
	// taking its address defines it too).
	if (function->denotes != nullptr)
		function = function->denotes;
	const bool copied = !function->constructor && !function->destructor && !function->is_virtual &&
		(function->is_inline || function->instantiated) && !sema::Analyzer::InstantiationSuppressed(function) &&
		(!function->instantiated || analyzer_->instantiate_definition(function)) && function->body != nullptr;
	if (!copied)
	{
		demand(function);
		return function_symbol(function);
	}
	const uint32_t known = inline_copies_.find(function);
	if (known != PointerMap::kMissing)
		return known;
	const std::string object = function_object(function, EntryVariant::Complete) + ".always_inline";
	const lowir::SymbolId symbol = new_symbol(function_spelling(function) + ".always_inline");
	symbol_objects_[symbol] = local_object(object, false);
	inline_copies_.set(function, symbol);
	PendingDeclaration pending = PendingDeclaration();
	pending.symbol = symbol;
	pending.function = true;
	pending.entity = function;
	pending.signature = lowir::kNoId;
	pending.metadata = function_metadata(function, symbol);
	pending.metadata.binding = ir_model::SBM_INTERNAL;
	set_pending(symbol, pending);
	pending_copies_.push_back(std::make_pair(function, symbol));
	return symbol;
}

void ProgramLowering::demand(const sema::FunctionEntity* function, EntryVariant variant)
{
	if (function->denotes != nullptr)
		function = function->denotes;
	// A constructor's base entry is its own definition, as a virtual
	// destructor's is; another destructor's base subobjects use the
	// complete entry.
	if (variant == EntryVariant::Base && !function->constructor && !(function->destructor && function->is_virtual) &&
		!HiddenBaseParameters(function, variant))
		variant = EntryVariant::Complete;
	demand_entry(function, variant);
}

void ProgramLowering::demand_destruction(const sema::FunctionEntity* destructor)
{
	// An automatic object's destruction demands a virtual destructor's
	// entries together: base, deleting, then complete (the course's
	// order, whichever a body calls first). A host object defines the
	// complete entry the destruction calls (README PA31); a table it emits
	// demands the deleting entry, a call of the base entry that entry.
	if (destructor->denotes != nullptr)
		destructor = destructor->denotes;
	if (host_objects_)
	{
		demand_entry(destructor, EntryVariant::Complete);
		return;
	}
	// So does a destructor whose base entry is a definition of its own
	// (its class has virtual bases).
	if (destructor->is_virtual || HiddenBaseParameters(destructor, EntryVariant::Base))
		demand_entry(destructor, EntryVariant::Base);
	if (destructor->is_virtual)
		demand_entry(destructor, EntryVariant::Deleting);
	demand_entry(destructor, EntryVariant::Complete);
}

void ProgramLowering::demand_base_entry(const sema::FunctionEntity* function)
{
	demand_entry(function->denotes != nullptr ? function->denotes : function, EntryVariant::Base);
}

void ProgramLowering::demand_vtable_entries(const sema::FunctionEntity* destructor)
{
	demand_entry(destructor, EntryVariant::Deleting);
	demand_entry(destructor, EntryVariant::Complete);
}

void ProgramLowering::demand_entry(const sema::FunctionEntity* function, EntryVariant variant)
{
	// Implicit and defaulted members are synthesized: constructors and
	// destructors that are not trivial, copy and move members called (the
	// course calls even a trivial assignment operator). One defaulted
	// inline outside its class (`inline T::T() = default;`, 8.4.2p5) is
	// defined where odr-used too (its definition emits nothing).
	const bool assignment = function->copy_kind == sema::CopyKind::CopyAssignment ||
		function->copy_kind == sema::CopyKind::MoveAssignment;
	const bool synthesized = (function->implicit || function->defaulted ||
								 (function->synthesized && function->is_inline)) &&
		(assignment || function->copy_kind != sema::CopyKind::None ||
			(!function->trivial && (function->constructor || function->destructor)));
	// GNU gnu_inline: an inline function's definition serves inlining only
	// (always_inline calls run it as a unit-local copy, see inline_copy);
	// GCC never compiles it on its own, so its address and other calls
	// name the external definition, which the declaration's object (its
	// asm label) denotes. Emitting it would define that name here: glibc's
	// C++ overloads of strchr wrap __builtin_strchr, a call of `strchr`.
	const sema::FunctionEntity* pattern = sema::PatternOf(function);
	if (function->gnu_inline || (function->primary != nullptr && function->primary->gnu_inline) ||
		(pattern != nullptr && pattern->gnu_inline))
		return;
	// A template's specialization (or an instance's member) is defined
	// where odr-used, its body instantiated on this demand (14.7.1p2).
	if (!synthesized && function->instantiated)
	{
		// One an explicit instantiation declaration names is defined
		// elsewhere (14.7.2p10).
		if (sema::Analyzer::InstantiationSuppressed(function) || !analyzer_->instantiate_definition(function))
			return;
	}
	else if (!synthesized && (emitted_at_definition(function) || !function->defined || function->body == nullptr))
		return;
	// A constructor's base entry is emitted where it is used even for an
	// out-of-class definition (the complete entry is the definition's).
	PointerMap& queued = variant == EntryVariant::Base ? queued_base_
		: variant == EntryVariant::Deleting            ? queued_deleting_
													   : queued_;
	if (queued.find(function) != PointerMap::kMissing)
		return;
	queued.set(function, 1);
	// A local class's member (a closure's included) whose body names its
	// enclosing function's block-scope statics reaches them through the
	// globals that function's lowering makes (see
	// FunctionLowering::variable_place): a function lowered on demand is
	// demanded first (only then: a captureless lambda's body lowered as
	// its invoker leaves the enclosing closure's operator unused).
	for (const sema::Scope* scope = function->owner; function->names_enclosing_statics && scope != nullptr &&
		scope->kind != sema::ScopeKind::Namespace && function->owner->kind == sema::ScopeKind::Class;
		scope = scope->parent)
	{
		if (scope->kind == sema::ScopeKind::Function && scope->entity != nullptr &&
			scope->entity->kind == sema::EntityKind::Function)
		{
			demand(static_cast<const sema::FunctionEntity*>(scope->entity));
			break;
		}
	}
	demanded_.push_back(Demanded{ function, variant });
}

bool ProgramLowering::function_nothrow(const sema::FunctionEntity* function)
{
	return function->noexcept_pending && analyzer_ != nullptr ? analyzer_->function_nothrow(function)
															  : function->nothrow;
}

bool ProgramLowering::callee_nothrow(const syntax::Expr& callee)
{
	return analyzer_ != nullptr && analyzer_->callee_nothrow(callee);
}

const sema::ConstValue* ProgramLowering::evaluated_value(const sema::VariableEntity* variable) const
{
	return analyzer_ != nullptr ? analyzer_->evaluated_value(variable) : nullptr;
}

bool ProgramLowering::read_through_storage(const sema::VariableEntity* variable) const
{
	return analyzer_ != nullptr && analyzer_->read_through_storage(variable);
}

bool ProgramLowering::constant_conversion(const syntax::Expr& condition, bool& value) const
{
	return analyzer_ != nullptr && analyzer_->constant_conversion(condition, value);
}

bool ProgramLowering::instantiate_definition(const sema::FunctionEntity* function)
{
	return analyzer_->instantiate_definition(function);
}

const sema::ClassEntity* ProgramLowering::call_naming_class(const syntax::CallExpr& call) const
{
	return analyzer_ != nullptr ? analyzer_->call_naming_class(call) : nullptr;
}

const sema::ClassEntity* ProgramLowering::member_naming_class(const syntax::Expr& access) const
{
	return analyzer_ != nullptr ? analyzer_->member_naming_class(access) : nullptr;
}

const syntax::Expr* ProgramLowering::default_argument(const sema::FunctionEntity* function, uint32_t index)
{
	return analyzer_->default_argument(function, index);
}

bool ProgramLowering::destructor_does_nothing(const sema::ClassEntity* entity, bool contained)
{
	const sema::FunctionEntity* destructor = entity->destructor;
	if (destructor == nullptr || destructor->trivial)
		return true;
	// A polymorphic class's destructor stores its table's address for the
	// virtual calls its bases' destructors may make (12.7p4): not without
	// effect as an object's own, but a subobject of an object destroyed
	// without effect stores tables nothing reads when its bases and members
	// are destroyed without effect too (the course's form omits its call);
	// a class with virtual bases but no virtual function reads that table
	// only to reach its virtual bases, which nothing does then either.
	if (entity->polymorphic && !contained)
		return false;
	PointerMap& memo = entity->polymorphic ? contained_destructors_ : inert_destructors_;
	const uint32_t known = memo.find(entity);
	if (known != PointerMap::kMissing)
		return known == 1;
	memo.set(entity, 2);
	// A union's implicit destructor destroys no variant member (12.4p8).
	const bool union_class = entity->key == TokenKind::KW_UNION;
	if (union_class && destructor->implicit)
	{
		memo.set(entity, 1);
		return true;
	}
	// A user-provided union destructor counts as a class's: its body and
	// its variant members' destructors (the course's form).
	bool inert = !destructor->deleted;
	if (inert && !destructor->implicit && !destructor->defaulted && !destructor->synthesized)
	{
		const syntax::FunctionBody* body = destructor->body;
		inert = body != nullptr && !body->try_block && body->body->items.empty();
	}
	for (const sema::BaseClass* base = entity->bases; inert && base != nullptr; base = base->next)
		inert = destructor_does_nothing(static_cast<const sema::ClassEntity*>(base->base), true);
	for (const sema::DataMember* member = entity->members; inert && member != nullptr; member = member->next)
	{
		const sema::Type* type = member->variable->type;
		while (type->kind == sema::TypeKind::Array)
			type = type->element;
		const sema::ClassEntity* cls = type->is_reference() ? nullptr : sema::ClassOf(type);
		inert = cls == nullptr || destructor_does_nothing(cls, true);
	}
	memo.set(entity, inert ? 1 : 2);
	return inert;
}

void ProgramLowering::root_destructor(const sema::FunctionEntity* destructor)
{
	demand(destructor);
	if (HiddenBaseParameters(destructor, EntryVariant::Base))
		demand_base_entry(destructor);
	rooted_destructors_.set(destructor, 1);
	const lowir::SymbolId symbol = function_symbol(destructor);
	if (program_.symbols[symbol].kind == lowir::SymbolKind::FunctionDefinition)
		program_.symbols[symbol].metadata.flags |= lowir::SF_OBJECT_ROOT;
}

lowir::SymbolId ProgramLowering::terminate_helper()
{
	if (terminate_helper_ != lowir::kNoId)
		return terminate_helper_;
	const lowir::Type pointer = lowir::Type::scalar(lowir::TypeKind::Ptr);
	// The runtime's __cxa_begin_catch (C linkage) and std::terminate.
	runtime_function(RuntimeFunction::BeginCatch);
	terminate_function();
	terminate_helper_ = new_symbol("cppgm_call_terminate");
	lowir::Signature helper = lowir::Signature();
	helper.first_parameter = static_cast<uint32_t>(program_.parameters.size());
	lowir::Parameter exception = lowir::Parameter();
	exception.name = intern("exception_object", 16);
	exception.type = pointer;
	program_.parameters.push_back(exception);
	helper.parameter_count = 1;
	helper.result = lowir::Type::scalar(lowir::TypeKind::Void);
	helper.boundary.unwind = ir_model::CUM_NO;
	helper.boundary.returns = ir_model::CRM_NORETURN;
	program_.declare_function(terminate_helper_, lowir::SymbolKind::FunctionDefinition, program_.add_signature(helper));
	program_.symbols[terminate_helper_].metadata.binding = ir_model::SBM_INTERNAL;
	program_.symbols[terminate_helper_].metadata.flags |= lowir::SF_NO_INLINE;
	terminate_pending_ = true;
	return terminate_helper_;
}

lowir::SymbolId ProgramLowering::terminate_function()
{
	if (std_terminate_ != lowir::kNoId)
		return std_terminate_;
	std_terminate_ = new_symbol("std_terminate");
	lowir::Signature ending = lowir::Signature();
	ending.first_parameter = static_cast<uint32_t>(program_.parameters.size());
	ending.result = lowir::Type::scalar(lowir::TypeKind::Void);
	ending.boundary.unwind = ir_model::CUM_NO;
	ending.boundary.returns = ir_model::CRM_NORETURN;
	program_.declare_function(std_terminate_, lowir::SymbolKind::FunctionDeclaration, program_.add_signature(ending));
	lowir::SymbolMetadata& terminating = program_.symbols[std_terminate_].metadata;
	terminating.role = ir_model::SR_TERMINATE;
	terminating.binding = ir_model::SBM_STRONG;
	terminating.object = intern("_ZSt9terminatev", 15);
	return std_terminate_;
}

void ProgramLowering::emit_demanded()
{
	// First-demand order; a definition may demand more (helpers first:
	// they were demanded by the definition just emitted). The thunks come
	// last, once the functions they adjust for are known to be defined here
	// or not.
	for (size_t i = 0; i < demanded_.size() || pending_helper_ < aggregate_helpers_.size() || terminate_pending_ ||
		pending_thunk_ < thunks_.size() || !pending_copies_.empty();)
	{
		if (!pending_copies_.empty())
		{
			// An always_inline function's unit-local copy (inline_copy).
			const std::pair<const sema::FunctionEntity*, lowir::SymbolId> copy = pending_copies_.back();
			pending_copies_.pop_back();
			emit_definition(copy.first, EntryVariant::Complete, copy.second);
			continue;
		}
		if (terminate_pending_)
		{
			terminate_pending_ = false;
			{
				FunctionLowering body(*this, terminate_helper_, nullptr);
				body.lower_terminate_helper(runtime_function(RuntimeFunction::BeginCatch), std_terminate_);
			}
			release_definition(terminate_helper_);
			++counters_.functions;
			continue;
		}
		if (pending_helper_ < aggregate_helpers_.size())
		{
			const AggregateHelper helper = aggregate_helpers_[pending_helper_++];
			emit_aggregate_helper(helper);
			continue;
		}
		if (i < demanded_.size())
		{
			const Demanded demanded = demanded_[i++];
			const lowir::SymbolId symbol = entry_symbol(demanded.function, demanded.variant);
			if (program_.symbols[symbol].kind != lowir::SymbolKind::Unresolved)
				continue;
			emit_definition(demanded.function, demanded.variant, symbol);
			continue;
		}
		const Thunk thunk = thunks_[pending_thunk_++];
		emit_thunk(thunk);
	}
	demanded_.clear();
}

lowir::SymbolId ProgramLowering::aggregate_helper(const sema::ClassEntity* entity, uint32_t count, unsigned cv)
{
	// The class's helpers, one per shape (and object cv-qualification).
	const uint32_t latest = helper_index_.find(entity);
	uint32_t shapes = 0;
	for (uint32_t index = latest == PointerMap::kMissing ? 0 : latest; index != 0;
		index = aggregate_helpers_[index - 1].previous)
	{
		if (aggregate_helpers_[index - 1].count == count && aggregate_helpers_[index - 1].cv == cv)
			return aggregate_helpers_[index - 1].symbol;
		++shapes;
	}
	std::string spelling;
	qualified_spelling(entity->owner, spelling);
	const char* label = entity->name != nullptr ? entity->name->spelling : entity->label;
	spelling += std::string(label) + "__" + label + "__aggregate";
	if (shapes != 0)
		spelling += "__sym" + std::to_string(shapes + 1);
	const lowir::SymbolId symbol = new_symbol(spelling);
	aggregate_helpers_.push_back(
		AggregateHelper{ entity, count, cv, symbol, latest == PointerMap::kMissing ? 0 : latest });
	helper_index_.set(entity, static_cast<uint32_t>(aggregate_helpers_.size()));
	// The signature now: calls name it before its definition is emitted.
	// Its object is a complete object of the class, a member of one or an
	// element of an array (an aggregate has no base classes, 8.5.1p1), as
	// a member function's `this` states its class's size.
	lowir::Signature signature = lowir::Signature();
	signature.first_parameter = static_cast<uint32_t>(program_.parameters.size());
	lowir::Parameter self = lowir::Parameter();
	self.name = this_name_;
	self.type = lowir::Type::scalar(lowir::TypeKind::Ptr);
	self.object_bytes = size_of(entity->type, 0);
	program_.parameters.push_back(self);
	signature.parameter_count = 1;
	uint32_t taken = 0;
	for (const sema::DataMember* member = entity->members; member != nullptr && taken < count; member = member->next)
	{
		const sema::Entity* variable = member->variable;
		if (variable->name == nullptr)
			continue;
		++taken;
		const lowir::NameId name = intern(
			variable->name->spelling, static_cast<uint32_t>(std::strlen(variable->name->spelling)));
		lowir::Parameter parameter = lowir::Parameter();
		parameter.name = name;
		parameter.type = variable->type->kind == sema::TypeKind::Array || variable->type->is_reference()
			? lowir::Type::scalar(lowir::TypeKind::Ptr)
			: storage_type(variable->type);
		// A reference member's referent is passed by its address, as a
		// reference parameter's is (with its complete object's size).
		if (variable->type->is_reference())
		{
			const lowir::Parameter reference = parameter_for(variable->type, name);
			parameter.pass = reference.pass;
			parameter.object_bytes = reference.object_bytes;
		}
		program_.parameters.push_back(parameter);
		++signature.parameter_count;
	}
	signature.result = lowir::Type::scalar(lowir::TypeKind::Void);
	// It takes no class member (FunctionLowering::helper_shape): it calls
	// nothing.
	signature.boundary.unwind = ir_model::CUM_NO;
	program_.declare_function(symbol, lowir::SymbolKind::FunctionDefinition, program_.add_signature(signature));
	program_.symbols[symbol].metadata.binding = ir_model::SBM_INTERNAL;
	return symbol;
}

void ProgramLowering::emit_aggregate_helper(const AggregateHelper& helper)
{
	{
		FunctionLowering body(*this, helper.symbol, nullptr);
		body.lower_aggregate_helper(helper.entity, helper.count);
	}
	release_definition(helper.symbol);
	++counters_.functions;
}

bool ProgramLowering::claim_slot_name(lowir::NameId name, uint32_t scope)
{
	if (slot_name_marks_.size() <= name)
		slot_name_marks_.resize(std::max<size_t>(name + 1, slot_name_marks_.size() * 2), 0);
	if (slot_name_marks_[name] == scope)
		return false;
	slot_name_marks_[name] = scope;
	return true;
}

lowir::NameId ProgramLowering::parameter_name(const Identifier* name)
{
	const char* spelling = name->spelling;
	const uint32_t length = name->length;
	uint32_t digits = length;
	while (digits > 0 && spelling[digits - 1] >= '0' && spelling[digits - 1] <= '9')
		--digits;
	if (digits == length || digits < 7 || std::memcmp(spelling + digits - 6, "__pack", 6) != 0)
		return intern(spelling, length);
	std::string text(spelling, digits - 6);
	const unsigned long index = std::strtoul(spelling + digits, nullptr, 10);
	if (index != 0)
		text += "__pack" + std::to_string(index + 1);
	return intern(text.data(), static_cast<uint32_t>(text.size()));
}

lowir::NameId ProgramLowering::argument_name(uint32_t index)
{
	while (argument_names_.size() <= index)
	{
		const std::string spelling = "arg" + std::to_string(argument_names_.size());
		argument_names_.push_back(intern(spelling.data(), static_cast<uint32_t>(spelling.size())));
	}
	return argument_names_[index];
}

// ----------------------------------------------------------------- types

lowir::Type ProgramLowering::storage_type(const sema::Type* type)
{
	using lowir::Type;
	using lowir::TypeKind;
	// An atomic class object takes its atomic size and alignment.
	const sema::Type* qualified = type;
	type = type->unqualified;
	switch (type->kind)
	{
	case sema::TypeKind::Fundamental:
		switch (type->fundamental)
		{
		case FundamentalType::Bool:
		case FundamentalType::UnsignedChar:
			return Type::scalar(TypeKind::U8);
		case FundamentalType::Char:
			return Type::scalar(type->plain_char_unsigned ? TypeKind::U8 : TypeKind::I8);
		case FundamentalType::SignedChar:
			return Type::scalar(TypeKind::I8);
		case FundamentalType::ShortInt:
			return Type::scalar(TypeKind::I16);
		case FundamentalType::UnsignedShortInt:
		case FundamentalType::Char16T:
			return Type::scalar(TypeKind::U16);
		case FundamentalType::Int:
		case FundamentalType::WcharT:
			return Type::scalar(TypeKind::I32);
		case FundamentalType::UnsignedInt:
		case FundamentalType::Char32T:
			return Type::scalar(TypeKind::U32);
		case FundamentalType::LongInt:
		case FundamentalType::LongLongInt:
		case FundamentalType::UnsignedLongInt:
		case FundamentalType::UnsignedLongLongInt:
			return Type::scalar(TypeKind::I64);
		case FundamentalType::Float:
		case FundamentalType::Float32:
			return Type::scalar(TypeKind::F32);
		case FundamentalType::Double:
		case FundamentalType::Float64:
		case FundamentalType::Float32x:
			return Type::scalar(TypeKind::F64);
		case FundamentalType::LongDouble:
		case FundamentalType::Float64x:
			return Type::scalar(TypeKind::F80);
		case FundamentalType::Float16:
		case FundamentalType::BFloat16:
			// No LowIR floating type holds binary16, bfloat16 or binary128
			// values: an object of their bytes
			// (lower/soft_float_lowering.cpp).
			return Type::object(2, 1);
		case FundamentalType::Float128:
		case FundamentalType::GnuFloat128:
			return Type::object(16, 4);
		case FundamentalType::Int128:
		case FundamentalType::UnsignedInt128:
			return Type::scalar(TypeKind::I128);
		case FundamentalType::NullptrT:
			// The course holds std::nullptr_t as a 64-bit integer.
			return Type::scalar(TypeKind::I64);
		case FundamentalType::Void:
			return Type::scalar(TypeKind::Void);
		}
		break;
	case sema::TypeKind::Enum:
	{
		const sema::EnumEntity* entity = static_cast<const sema::EnumEntity*>(type->entity);
		if (entity->underlying != nullptr)
			return storage_type(entity->underlying);
		break;
	}
	case sema::TypeKind::Pointer:
	case sema::TypeKind::BlockPointer:
	case sema::TypeKind::LValueReference:
	case sema::TypeKind::RValueReference:
	case sema::TypeKind::Function:
		return Type::scalar(TypeKind::Ptr);
	case sema::TypeKind::MemberPointer:
		// The course's Itanium forms: a data member's offset plus one (0 is
		// null), a member function's address and this-adjustment (i128).
		return Type::scalar(type->element->kind == sema::TypeKind::Function ? TypeKind::I128 : TypeKind::I64);
	case sema::TypeKind::Vector:
		// Its lanes' bytes (lower/vector_lowering.cpp).
	case sema::TypeKind::Complex:
		// Its two parts' bytes (lower/complex_lowering.cpp).
	case sema::TypeKind::Array:
	case sema::TypeKind::Class:
	{
		uint64_t size = 0, alignment = 0;
		if (!analyzer_->object_layout(qualified, size, alignment))
			break;
		unsigned log2 = 0;
		while ((uint64_t(1) << log2) < alignment)
			++log2;
		return Type::object(size, log2);
	}
	default:
		break;
	}
	throw LoweringError(0, "a type outside the procedural lowering slice");
}

uint64_t ProgramLowering::size_of(const sema::Type* type, uint32_t location)
{
	uint64_t size = 0, alignment = 0;
	if (type->is_reference())
		return 8;
	// A hosted compilation's flexible array member (see
	// sema::Analyzer::member_size) has no bytes of its own.
	if (type->kind == sema::TypeKind::Array && type->bound == 0 && analyzer_->hosted())
		return 0;
	if (!analyzer_->object_layout(type, size, alignment))
		throw LoweringError(location, "the size of an incomplete type");
	return size;
}

uint64_t ProgramLowering::alignment_of(const sema::Type* type, uint32_t location)
{
	uint64_t size = 0, alignment = 0;
	if (type->is_reference())
		return 8;
	if (!analyzer_->object_layout(type, size, alignment))
		throw LoweringError(location, "the alignment of an incomplete type");
	return alignment;
}

uint64_t ProgramLowering::member_bit_offset(const sema::VariableEntity* member, uint32_t location)
{
	uint64_t offset = 0;
	if (!analyzer_->member_offset(member, offset) || member->data_member == nullptr)
		throw LoweringError(location, "a member without an offset");
	return member->data_member->offset;
}

bool ProgramLowering::trivially_copied_elements(const sema::Type* type)
{
	// An array of class objects that a caller's temporary may build and
	// the helper copy by its bytes: a copy-initialization whose trivial
	// copy is not elided (12.8p31-p32; the course's form), for classes
	// trivially copied and destroyed.
	while (type->kind == sema::TypeKind::Array)
		type = type->element;
	const sema::ClassEntity* cls = type->is_reference() ? nullptr : sema::ClassOf(type);
	if (cls == nullptr)
		return true;
	analyzer_->implicit_copy_members(cls);
	return !cls->polymorphic && cls->copy_constructor != nullptr && cls->copy_constructor->trivial &&
		(cls->destructor == nullptr || cls->destructor->trivial);
}

uint64_t ProgramLowering::base_offset(
	const sema::ClassEntity* derived, const sema::ClassEntity* base, uint32_t location)
{
	uint64_t offset = 0;
	if (!base_path_offset(derived, base, offset, location))
		throw LoweringError(location, "a conversion to a class that is not a base");
	return offset;
}

bool ProgramLowering::base_path_offset(
	const sema::ClassEntity* derived, const sema::ClassEntity* base, uint64_t& offset, uint32_t location)
{
	// The base subobject's offset along the path of direct bases that leads
	// to it (the analysis made the path unique where it matters, 10.2,
	// 4.10p3): a class of one base is followed as a chain; at one of
	// several, a direct base is found by the analyzer's index, else each
	// base is searched, skipping the classes already found not to lead to
	// it (so a lattice of repeated bases costs its edges, not its paths).
	base_misses_.clear();
	return base_path_search(derived, base, offset, location);
}

bool ProgramLowering::base_path_search(
	const sema::ClassEntity* derived, const sema::ClassEntity* base, uint64_t& offset, uint32_t location)
{
	uint64_t total = 0;
	const sema::ClassEntity* cls = derived;
	while (cls != base)
	{
		++counters_.base_steps;
		const sema::BaseClass* step = cls != nullptr ? cls->bases : nullptr;
		if (step == nullptr)
			return false;
		size_of(cls->type, location);
		// A virtual base is no static offset away (see base_route).
		if (step->next == nullptr && step->is_virtual)
			return false;
		if (step->next != nullptr)
		{
			if (const sema::BaseClass* direct = analyzer_->direct_base(cls, base))
			{
				offset = total + direct->offset;
				return true;
			}
			// A wide class with virtual bases: its non-virtual direct bases'
			// list (read by index: the searches below may add lists).
			const bool listed = cls->base_count >= kListedBases && VirtualBaseCount(cls) != 0;
			uint32_t at = listed ? nonvirtual_bases(cls) : 0;
			for (const sema::BaseClass* edge = listed ? nonvirtual_store_[at] : step; edge != nullptr;
				edge = listed ? nonvirtual_store_[++at] : edge->next)
			{
				// The base itself, or a class without bases (no path, nothing
				// to remember), is decided without the memo.
				const sema::ClassEntity* next = static_cast<const sema::ClassEntity*>(edge->base);
				if (edge->is_virtual)
					continue;
				if (next == base)
				{
					offset = total + edge->offset;
					return true;
				}
				if (next == nullptr || next->bases == nullptr || base_misses_.find(next) != PointerMap::kMissing)
					continue;
				uint64_t inner = 0;
				if (base_path_search(next, base, inner, location))
				{
					offset = total + edge->offset + inner;
					return true;
				}
				base_misses_.set(next, 1);
			}
			return false;
		}
		total += step->offset;
		cls = static_cast<const sema::ClassEntity*>(step->base);
	}
	offset = total;
	return true;
}

uint32_t ProgramLowering::nonvirtual_bases(const sema::ClassEntity* cls)
{
	const uint32_t found = nonvirtual_lists_.find(cls);
	if (found != PointerMap::kMissing)
		return found;
	const uint32_t first = static_cast<uint32_t>(nonvirtual_store_.size());
	for (const sema::BaseClass* base = cls->bases; base != nullptr; base = base->next)
	{
		if (!base->is_virtual)
			nonvirtual_store_.push_back(base);
	}
	nonvirtual_store_.push_back(nullptr);
	nonvirtual_lists_.set(cls, first);
	return first;
}

uint64_t ProgramLowering::member_offset(const sema::VariableEntity* member, uint32_t location)
{
	uint64_t offset = 0;
	if (!analyzer_->member_offset(member, offset) || member->bit_field)
		throw LoweringError(location, "a member without a byte offset");
	return offset;
}

const sema::ClassEntity* ProgramLowering::member_home(
	const sema::VariableEntity* member, uint64_t& offset, uint32_t location)
{
	return member_holder(member, nullptr, offset, location);
}

const sema::ClassEntity* ProgramLowering::member_holder(
	const sema::VariableEntity* member, const sema::ClassEntity* within, uint64_t& offset, uint32_t location)
{
	offset = 0;
	const sema::ClassEntity* home = sema::ClassOf(member->owner->entity->type);
	while (home != within && home->anonymous && home->storage != nullptr &&
		home->storage->entity->owner->kind == sema::ScopeKind::Class)
	{
		const sema::VariableEntity* object = static_cast<const sema::VariableEntity*>(home->storage->entity);
		offset += member_offset(object, location);
		home = sema::ClassOf(object->owner->entity->type);
	}
	return home;
}

const sema::ClassEntity* ProgramLowering::anonymous_home(
	const sema::ClassEntity* cls, uint64_t& offset, uint32_t location)
{
	offset = 0;
	while (cls->anonymous && cls->storage != nullptr && cls->storage->entity->owner->kind == sema::ScopeKind::Class)
	{
		const sema::VariableEntity* object = static_cast<const sema::VariableEntity*>(cls->storage->entity);
		offset += member_offset(object, location);
		cls = sema::ClassOf(object->owner->entity->type);
	}
	return cls;
}

uint64_t ProgramLowering::member_home_offset(const sema::VariableEntity* member, uint32_t location)
{
	uint64_t anonymous = 0;
	member_home(member, anonymous, location);
	return anonymous + member_offset(member, location);
}

} // namespace lower
