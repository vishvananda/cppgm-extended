#include "lowering/abi/local_entity_names.h"

#include "lowering/support/errors.h"

#include <algorithm>

namespace cppgm
{
namespace lowering
{
namespace abi
{

namespace
{

using namespace semantic;

const std::uint32_t kUnassigned = 0xffffffffu;

bool HasTemplateSpecializationOwner(const Program& program, EntityId owner)
{
	for (std::size_t depth = 0; owner != kNoEntity &&
		owner < program.entities.size() && depth < program.entities.size();
		++depth)
	{
		if (program.entities[owner].template_argument_begin != kNoBinding)
			return true;
		owner = program.entities[owner].enclosing_class;
	}
	return false;
}

bool IsInlineOrSpecialization(const Program& program, BindingId function)
{
	const BindingRecord& binding =
		program.bindings[program.bindings[function].canonical];
	return binding.inline_function || binding.weak_odr ||
		binding.template_argument_count != 0 ||
		binding.function_template_specialization ||
		HasTemplateSpecializationOwner(program, binding.member_owner);
}

bool HasInternalLinkage(const Program& program, const BindingRecord& function)
{
	return (function.storage_class == STORAGE_CLASS_STATIC &&
			function.member_owner == kNoEntity) ||
		function.unnamed_namespace_linkage ||
		program.HasInternalLinkageScope(function.owner);
}

// The function whose body lexically holds a local class's or a closure's
// member function, or the function itself.
BindingId OutermostFunction(const Program& program, BindingId function)
{
	for (std::size_t depth = 0; depth < program.bindings.size(); ++depth)
	{
		function = LocalContextFunction(program, function);
		const EntityId owner = program.bindings[function].member_owner;
		if (owner == kNoEntity || owner >= program.entities.size())
			return function;
		const BindingId context = program.entities[owner].local_context;
		if (context == kNoBinding || context >= program.bindings.size())
			return function;
		function = context;
	}
	ThrowLoweringInternal("local function context chain is cyclic");
}

bool ClassIsVisible(const Program& program, EntityId entity, std::size_t depth)
{
	if (entity == kNoEntity || entity >= program.entities.size() ||
		depth > program.entities.size())
		return false;
	const EntityRecord& record = program.entities[entity];
	if (record.lambda_closure) return false;
	if (record.local_context != kNoBinding)
		return LocalContextIsVisible(program, record.local_context);
	if (IsUnnamedClass(program, entity))
		return record.enclosing_class != kNoEntity &&
			ClassIsVisible(program, record.enclosing_class, depth + 1);
	if (program.HasInternalLinkageScope(record.owner)) return false;
	return record.enclosing_class == kNoEntity ||
		ClassIsVisible(program, record.enclosing_class, depth + 1);
}

EntityId NamedEntity(const Program& program, TypeId type)
{
	for (std::size_t depth = 0; type != kNoType && depth < 64; ++depth)
	{
		const TypeRecord& record = program.types.Get(type);
		if (record.kind == TYPE_NAMED) return record.entity;
		if (record.kind != TYPE_QUALIFIED && record.kind != TYPE_ARRAY)
			return kNoEntity;
		type = record.child;
	}
	return kNoEntity;
}

}  // namespace

BindingId LocalContextFunction(const Program& program, BindingId function)
{
	function = program.bindings[function].canonical;
	const BindingRecord& binding = program.bindings[function];
	if (!binding.lambda_invocation ||
		binding.lambda_invocation_owner >= program.entities.size())
		return function;
	const BindingId call =
		program.entities[binding.lambda_invocation_owner].lambda_call_operator;
	return call == kNoBinding || call >= program.bindings.size() ? function :
		program.bindings[call].canonical;
}

bool ClosureHasManglingNumber(const Program& program, EntityId closure)
{
	if (closure == kNoEntity || closure >= program.entities.size()) return false;
	const BindingId context = program.entities[closure].local_context;
	return context != kNoBinding && context < program.bindings.size() &&
		IsInlineOrSpecialization(program, LocalContextFunction(program, context));
}

bool LocalContextIsVisible(const Program& program, BindingId function)
{
	if (function == kNoBinding || function >= program.bindings.size())
		return false;
	const BindingId outer = OutermostFunction(program, function);
	return !HasInternalLinkage(program, program.bindings[outer]) &&
		IsInlineOrSpecialization(program, outer);
}

bool UnnamedTypeIsVisible(const Program& program, EntityId entity)
{
	if (entity == kNoEntity || entity >= program.entities.size()) return false;
	const EntityRecord& record = program.entities[entity];
	if (record.local_context != kNoBinding)
		return LocalContextIsVisible(program, record.local_context);
	return record.enclosing_class != kNoEntity &&
		ClassIsVisible(program, record.enclosing_class, 0);
}

bool IsUnnamedClass(const Program& program, EntityId entity)
{
	return entity != kNoEntity && entity < program.entities.size() &&
		program.entities[entity].unnamed_class &&
		!program.entities[entity].lambda_closure;
}

LocalNames::LocalNames(const Program& program)
	: program_(program), graph_(0), anonymous_total_(0)
{
}

std::uint32_t LocalNames::AnonymousNumber(EntityId type)
{
	if (type == kNoEntity || type >= program_.entities.size())
		ThrowLoweringInternal("unnamed ABI type has no entity");
	if (anonymous_numbers_.size() <= type)
		anonymous_numbers_.resize(program_.entities.size(), kUnassigned);
	if (anonymous_numbers_[type] != kUnassigned) return anonymous_numbers_[type];
	const BindingId context = program_.entities[type].local_context;
	std::uint32_t number = anonymous_total_;
	if (context != kNoBinding && context < program_.bindings.size())
	{
		std::uint32_t* count = function_anonymous_counts_.Insert(
			LocalContextFunction(program_, context), 0);
		number = (*count)++;
	}
	++anonymous_total_;
	anonymous_numbers_[type] = number;
	return number;
}

// The function a static variable belongs to: a constructor's or
// destructor's complete and base entries are one function.
BindingId LocalNames::StaticContext(const LocalStaticObjectAction& action) const
{
	const BindingId function = LocalContextFunction(program_, action.function);
	const BindingId base = program_.bindings[function].lifecycle_base_entry;
	return base == kNoBinding || base >= program_.bindings.size() ? function :
		program_.bindings[base].canonical;
}

std::uint64_t LocalNames::StaticDeclarationKey(
	const LocalStaticObjectAction& action) const
{
	return (static_cast<std::uint64_t>(StaticContext(action)) << 32) |
		action.source_token_first;
}

std::uint32_t LocalNames::StaticDeclaration(std::uint32_t action) const
{
	if (action >= static_first_actions_.size())
		ThrowLoweringInternal("local static ABI action is invalid");
	return static_first_actions_[action];
}

std::uint32_t LocalNames::LazyDiscriminator(BindingId function, NameId name)
{
	const std::uint64_t key = (static_cast<std::uint64_t>(
		LocalContextFunction(program_, function)) << 32) | name;
	std::uint32_t* count = discriminator_counts_.Insert(key, 0);
	return (*count)++;
}

std::uint32_t LocalNames::ClassDiscriminator(EntityId entity)
{
	if (entity == kNoEntity || entity >= program_.entities.size())
		ThrowLoweringInternal("local ABI class has no entity");
	const EntityRecord& record = program_.entities[entity];
	if (record.local_context == kNoBinding ||
		LocalContextIsVisible(program_, record.local_context))
		return record.local_name_ordinal;
	if (class_discriminators_.size() <= entity)
		class_discriminators_.resize(program_.entities.size(), kUnassigned);
	if (class_discriminators_[entity] == kUnassigned)
		class_discriminators_[entity] =
			LazyDiscriminator(record.local_context, record.identity_name);
	return class_discriminators_[entity];
}

std::uint32_t LocalNames::StaticDiscriminator(std::uint32_t action)
{
	if (graph_ == 0 || action >= graph_->local_static_objects.size() ||
		action >= static_discriminators_.size())
		ThrowLoweringInternal("local static ABI action is invalid");
	if (static_discriminators_[action] == kUnassigned)
	{
		const LocalStaticObjectAction& object =
			graph_->local_static_objects[action];
		std::uint32_t* declared = static_declarations_.Insert(
			StaticDeclarationKey(object), kUnassigned);
		if (*declared == kUnassigned)
			*declared = LazyDiscriminator(StaticContext(object),
				program_.bindings[object.object].name);
		static_discriminators_[action] = *declared;
	}
	return static_discriminators_[action];
}

void LocalNames::Prepare(const SemanticGraphView& graph)
{
	graph_ = &graph;
	anonymous_numbers_.assign(program_.entities.size(), kUnassigned);
	class_discriminators_.assign(program_.entities.size(), kUnassigned);
	static_discriminators_.assign(graph.local_static_objects.size(), kUnassigned);
	static_first_actions_.resize(graph.local_static_objects.size());
	// Where other units name a function's static variables, a discriminator
	// counts the earlier ones of its name in declaration order, as clang's
	// Sema numbers them.
	detail::FlatHashMap<std::uint64_t, std::uint32_t> declared, first;
	for (std::size_t i = 0; i < graph.local_static_objects.size(); ++i)
	{
		const LocalStaticObjectAction& action = graph.local_static_objects[i];
		static_action_by_node_.Insert(action.variable, static_cast<std::uint32_t>(i));
		// A body analyzed more than once declares each static once: a
		// constructor's or destructor's complete and base entries, a
		// captureless closure's call operator and static invoker, and a
		// function whose definition is analyzed again.
		static_first_actions_[i] = *first.Insert(StaticDeclarationKey(action),
			static_cast<std::uint32_t>(i));
		if (!LocalContextIsVisible(program_, action.function)) continue;
		const std::uint64_t key = (static_cast<std::uint64_t>(
			StaticContext(action)) << 32) | program_.bindings[action.object].name;
		std::uint32_t* ordinal = static_declarations_.Insert(
			StaticDeclarationKey(action), kUnassigned);
		if (*ordinal == kUnassigned)
		{
			std::uint32_t* count = declared.Insert(key, 0);
			*ordinal = (*count)++;
		}
		static_discriminators_[i] = *ordinal;
	}
	defined_member_owner_.assign(program_.entities.size(), 0);
	for (std::size_t node = 0; node < graph.arena.nodes.size(); ++node)
	{
		const DumpNode& record = graph.arena.nodes[node];
		if (record.kind == DUMP_FUNCTION_DEFINITION &&
			record.binding != kNoBinding &&
			record.binding < program_.bindings.size() &&
			program_.bindings[record.binding].member_owner <
				defined_member_owner_.size())
			defined_member_owner_[program_.bindings[record.binding].member_owner] = 1;
	}
	for (EntityId entity = 0; entity < program_.entities.size(); ++entity)
	{
		const EntityRecord& record = program_.entities[entity];
		if (defined_member_owner_[entity] && !record.lambda_closure &&
			record.local_context != kNoBinding &&
			record.local_context < program_.bindings.size())
			local_classes_.Insert(LocalContextFunction(program_, record.local_context),
				std::vector<EntityId>())->push_back(entity);
	}
	std::vector<DeferredFunction> deferred;
	WalkTop(graph.root, &deferred);
	for (std::size_t i = 0; i < deferred.size(); ++i)
		WalkFunction(deferred[i].node, deferred[i].function);
}

void LocalNames::WalkTop(std::uint32_t node,
	std::vector<DeferredFunction>* deferred)
{
	// clang emits an external non-inline function where it is defined and
	// names the entities its body uses as it emits them; it emits the other
	// definitions (inline, internal, template and local ones) later.  The
	// member functions an unnamed class defines are named at the end of the
	// declaration holding the class.
	const DumpArena& arena = graph_->arena;
	for (std::uint32_t edge = arena.nodes[node].first_edge;
		edge != kNoDumpEdge; edge = arena.edges[edge].next)
	{
		const std::uint32_t child = arena.edges[edge].child;
		const DumpNode& record = arena.nodes[child];
		if (record.kind == DUMP_NAMESPACE || record.kind == DUMP_TRANSLATION_UNIT)
			WalkTop(child, deferred);
		else if (record.kind == DUMP_FUNCTION_DEFINITION &&
			record.binding != kNoBinding && record.binding < program_.bindings.size())
		{
			const BindingId function = program_.bindings[record.binding].canonical;
			const BindingRecord& binding = program_.bindings[function];
			const bool local = binding.lambda_invocation ||
				(binding.member_owner != kNoEntity &&
				 binding.member_owner < program_.entities.size() &&
				 program_.entities[binding.member_owner].local_context != kNoBinding);
			DeferredFunction item;
			item.node = child;
			item.function = function;
			if (!local && !IsInlineOrSpecialization(program_, function) &&
				!HasInternalLinkage(program_, binding))
				WalkFunction(child, function);
			else deferred->push_back(item);
		}
		else if (record.kind == DUMP_VARIABLE)
		{
			const EntityId type = NamedEntity(program_, record.type);
			if (IsUnnamedClass(program_, type) && defined_member_owner_[type] &&
				!UnnamedTypeIsVisible(program_, type))
				AnonymousNumber(type);
		}
	}
}

void LocalNames::WalkFunction(std::uint32_t node, BindingId function)
{
	NameReference(function);
	const DumpArena& arena = graph_->arena;
	std::vector<std::uint32_t> pending(1, node);
	while (!pending.empty())
	{
		const std::uint32_t current = pending.back();
		pending.pop_back();
		const DumpNode& record = arena.nodes[current];
		if (record.kind == DUMP_VARIABLE)
		{
			const std::uint32_t* action = static_action_by_node_.Find(current);
			if (action) StaticDiscriminator(*action);
		}
		else if (current != node && record.binding != kNoBinding &&
			record.binding < program_.bindings.size() &&
			program_.bindings[record.binding].kind == BIND_FUNCTION)
			NameReference(record.binding);
		if (record.selected_binding != kNoBinding &&
			record.selected_binding < program_.bindings.size() &&
			program_.bindings[record.selected_binding].kind == BIND_FUNCTION)
			NameReference(record.selected_binding);
		// Children in source order: push them, then reverse the pushed run.
		const std::size_t first = pending.size();
		for (std::uint32_t edge = record.first_edge; edge != kNoDumpEdge;
			edge = arena.edges[edge].next)
			pending.push_back(arena.edges[edge].child);
		std::reverse(pending.begin() + first, pending.end());
	}
	// The local classes whose member functions clang defers to the end of
	// the function's declaration, in definition order.
	const std::vector<EntityId>* classes = local_classes_.Find(function);
	if (classes)
		for (std::size_t i = 0; i < classes->size(); ++i)
			NameEntity((*classes)[i], 0);
}

void LocalNames::NameReference(BindingId binding)
{
	const BindingRecord& record = program_.bindings[binding];
	const EntityId owner = record.lambda_invocation ?
		record.lambda_invocation_owner : record.member_owner;
	if (owner != kNoEntity) NameEntity(owner, 0);
	NameType(record.type, 0);
	const std::size_t first = record.template_argument_begin;
	for (std::size_t i = 0; i < record.template_argument_count &&
		first + i < program_.template_arguments.size(); ++i)
		if (first + i >= program_.canonical_template_arguments.size() ||
			program_.canonical_template_arguments[first + i].kind ==
				TEMPLATE_ARGUMENT_TYPE)
			NameType(program_.template_arguments[first + i], 0);
}

void LocalNames::NameType(TypeId type, std::size_t depth)
{
	if (type == kNoType || depth > 8) return;
	const TypeRecord& record = program_.types.Get(type);
	switch (record.kind)
	{
	case TYPE_QUALIFIED: case TYPE_POINTER: case TYPE_LVALUE_REFERENCE:
	case TYPE_RVALUE_REFERENCE: case TYPE_ARRAY:
		NameType(record.child, depth + 1);
		return;
	case TYPE_FUNCTION:
	{
		NameType(record.child, depth + 1);
		const TypeId* parameters = program_.types.Parameters(type);
		for (std::size_t i = 0; i < record.parameter_count; ++i)
			NameType(parameters[i], depth + 1);
		return;
	}
	case TYPE_NAMED:
		NameEntity(record.entity, depth + 1);
		return;
	default:
		return;
	}
}

void LocalNames::NameEntity(EntityId entity, std::size_t depth)
{
	for (; entity != kNoEntity && entity < program_.entities.size() &&
		depth < 16; ++depth)
	{
		const EntityRecord& record = program_.entities[entity];
		if (record.lambda_closure)
		{
			if (!ClosureHasManglingNumber(program_, entity)) AnonymousNumber(entity);
			return;
		}
		if (record.local_context != kNoBinding)
		{
			if (IsUnnamedClass(program_, entity))
			{
				if (!UnnamedTypeIsVisible(program_, entity)) AnonymousNumber(entity);
			}
			else ClassDiscriminator(entity);
			return;
		}
		if (IsUnnamedClass(program_, entity) &&
			!UnnamedTypeIsVisible(program_, entity))
			AnonymousNumber(entity);
		const std::size_t first = record.template_argument_begin;
		if (first != kNoBinding)
			for (std::size_t i = 0; i < record.template_argument_count &&
				first + i < program_.template_arguments.size(); ++i)
				if (first + i >= program_.canonical_template_arguments.size() ||
					program_.canonical_template_arguments[first + i].kind ==
						TEMPLATE_ARGUMENT_TYPE)
					NameType(program_.template_arguments[first + i], depth + 1);
		entity = record.enclosing_class;
	}
}

}  // namespace abi
}  // namespace lowering
}  // namespace cppgm
