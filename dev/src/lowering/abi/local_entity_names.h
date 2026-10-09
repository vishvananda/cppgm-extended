#pragma once

// The names Itanium leaves to the implementation, as clang gives them: the
// `$_<n>` name of a unit-local unnamed class or closure type, and the
// discriminators of a function's local classes and static variables where
// no other unit names them.  clang assigns both as it first mangles an
// entity; LocalNames follows that order for the ordinary cases with one
// pass over the unit's functions in emission order (Prepare) and assigns
// anything it did not reach at its first mangling.

#include "semantic/model/graph.h"
#include "semantic/model/local_contexts.h"
#include "semantic/model/program.h"
#include "support/containers/flat_hash_map.h"

#include <cstdint>
#include <vector>

namespace cppgm
{
namespace lowering
{
namespace abi
{

using semantic::ClosureHasManglingNumber;
using semantic::LocalContextFunction;

// A data member initializer's numbered closure type, named in the member,
// `<class> <member> M Ul<sig>E<n>_`.
bool ClosureIsMemberLambda(const semantic::Program& program,
	semantic::EntityId closure);

// A closure type named `$_<n>` outside a function: one of a namespace-scope
// lambda, of a namespace function's default argument, or of a non-template
// class's static data member initializer.
bool ClosureIsNamespaceScope(const semantic::Program& program,
	semantic::EntityId closure);

// The scope such a `$_<n>` closure type is named in: a static data member
// initializer's is its class (`_ZN1P3$_08__invokeEv`), any other's its
// namespace.
semantic::ScopeId ClosureNamingScope(const semantic::Program& program,
	semantic::EntityId closure);

// Whether other units name a function's local entities: its outermost
// enclosing function is inline or a template specialization and has
// external linkage (clang's externally visible no-linkage entities).
bool LocalContextIsVisible(const semantic::Program& program,
	semantic::BindingId function);

// Whether other units name an unnamed class (`Ut<n>_`): a member of a class
// they name, or a local class of a visible function.  Otherwise it is
// unit-local and named `$_<n>`.
bool UnnamedTypeIsVisible(const semantic::Program& program,
	semantic::EntityId entity);

// An unnamed class without a typedef name for linkage purposes.
bool IsUnnamedClass(const semantic::Program& program, semantic::EntityId entity);

class LocalNames
{
public:
	explicit LocalNames(const semantic::Program& program);

	void Prepare(const semantic::SemanticGraphView& graph);

	// The `$_<n>` number of a unit-local unnamed class or closure type: a
	// local one counts its function's types numbered before it, any other
	// all the unit numbered before it, local ones included.
	std::uint32_t AnonymousNumber(semantic::EntityId type);
	// The discriminator ordinal (0 for the first, which takes none) of a
	// named local class or of the local static of an action.
	std::uint32_t ClassDiscriminator(semantic::EntityId entity);
	std::uint32_t StaticDiscriminator(std::uint32_t action);
	// The first action of the declaration of an action's static variable:
	// the actions of one declaration are one object.
	std::uint32_t StaticDeclaration(std::uint32_t action) const;

private:
	struct DeferredFunction
	{
		std::uint32_t node;
		semantic::BindingId function;
	};

	std::uint32_t LazyDiscriminator(semantic::BindingId function,
		semantic::NameId name);
	semantic::BindingId StaticContext(
		const semantic::LocalStaticObjectAction& action) const;
	std::uint64_t StaticDeclarationKey(
		const semantic::LocalStaticObjectAction& action) const;
	void WalkTop(std::uint32_t node, std::vector<DeferredFunction>* deferred);
	void WalkFunction(std::uint32_t node, semantic::BindingId function);
	void WalkBody(std::uint32_t node);
	void NameReference(semantic::BindingId binding);
	void NameType(semantic::TypeId type, std::size_t depth);
	void NameEntity(semantic::EntityId entity, std::size_t depth);

	const semantic::Program& program_;
	const semantic::SemanticGraphView* graph_;
	std::vector<std::uint32_t> anonymous_numbers_;
	std::vector<std::uint32_t> class_discriminators_;
	std::vector<std::uint32_t> static_discriminators_;
	std::vector<std::uint32_t> static_first_actions_;
	detail::FlatHashMap<semantic::BindingId, std::uint32_t>
		function_anonymous_counts_;
	detail::FlatHashMap<std::uint64_t, std::uint32_t> discriminator_counts_;
	detail::FlatHashMap<std::uint32_t, std::uint32_t> static_action_by_node_;
	detail::FlatHashMap<std::uint64_t, std::uint32_t> static_declarations_;
	std::vector<std::uint8_t> defined_member_owner_;
	detail::FlatHashMap<semantic::BindingId, std::vector<semantic::EntityId> >
		local_classes_;
	std::uint32_t anonymous_total_;
};

}  // namespace abi
}  // namespace lowering
}  // namespace cppgm
