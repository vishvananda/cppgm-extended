#pragma once

// Facts about the entities local to a function (local classes, unnamed
// classes and closure types) that both the internal-identity pass and the
// ABI naming need: which function a local name is relative to, whether a
// closure type has an Itanium mangling number, and which declaration decides
// whether other units name the entity (Itanium 5.1.6, 5.1.8; N3485 3.5p8).

#include "semantic/model/program.h"

namespace cppgm
{
namespace semantic
{

// The function a local entity's name is relative to: a captureless
// closure's static invoker, which holds the lambda's body, stands for the
// closure's function call operator.
BindingId LocalContextFunction(const Program& program, BindingId function);

// An inline function, a template specialization or a member of one: every
// unit that uses it defines it.
bool IsInlineOrSpecialization(const Program& program, BindingId function);

// A closure type has an Itanium mangling number (`Ul<sig>E<n>_`) when its
// lambda is lexically inside an inline function or a template
// specialization (a lambda's own function call operator is inline, so a
// lambda nested in a lambda body is numbered), in a default member
// initializer, in a class template's static data member initializer, or in
// an in-class default argument.  Any other closure type is unit-local and
// named `$_<n>`.
bool ClosureHasManglingNumber(const Program& program, EntityId closure);

// The declaration whose linkage decides whether other units name the local
// entities of a function: the outermost enclosing function, or the data
// member or function a numbered member-context closure belongs to.  Sets
// *candidate when other units can name them at all (the outermost function
// is inline or a specialization, or the closure is such a member's); the
// anchor's own linkage decides the rest.
BindingId LocalVisibilityAnchor(const Program& program, BindingId function,
	bool* candidate);

// The same for a local class, unnamed local class or closure type itself.
// Returns kNoBinding with *candidate false for an entity that is not local
// to a function or member.
BindingId EntityVisibilityAnchor(const Program& program, EntityId entity,
	bool* candidate);

}
}
