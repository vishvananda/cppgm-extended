#pragma once

#include <cstdint>

#include "sema/entity_records.h"

// The records of C++17's structured binding declarations (an extension
// here; sema/analyzer_bindings.cpp), which the entities mark only by
// VariableEntity::structured_binding.
namespace sema {

// A structured binding declaration (11.5): the unnamed variable e its
// initializer initializes (`object`), the type E of e (no reference), how
// its names bind and the names in order; kept beside e
// (SemanticGraph::decomposition), whose Itanium name lists them (5.1.4:
// DC <source-name>+ E).
enum class DecompositionForm : uint8_t
{
	// 11.5p2: E is an array, each name an element.
	Array,
	// p3: E is tuple-like (std::tuple_size<E> has its `value`): each name
	// is a reference variable bound to get<i>(e).
	Tuple,
	// p4: each name is a non-static data member of E, all of them direct
	// members of E or of one base class of E.
	Members
};

struct BindingEntity;

struct Decomposition
{
	VariableEntity* object;
	const Type* type;
	DecompositionForm form;
	uint32_t count;
	BindingEntity** bindings;
};

// A name of a structured binding declaration (VariableEntity::
// structured_binding): an lvalue designating its element of the object e
// (11.5p1). Its type is the referenced type (decltype's, 7.1.6.2p4), and
// for an array or a class it has no storage: naming it designates e's
// element `index` or member `member` (an lvalue of e's cv-qualification,
// a mutable member's without const, CWG 2312; a reference member's
// referent). A tuple-like E's name is the reference variable r_i (its
// type a reference to T_i, initialized by `initializer`), and its
// referenced type is T_i (`referenced`).
struct BindingEntity : VariableEntity
{
	const Decomposition* decomposition;
	uint32_t index;
	const Type* referenced;
	const VariableEntity* member;
	const syntax::Initializer* initializer;
};

// A structured binding named alone designates its element of e (no
// storage of its own): an array's or a class's binding, not a tuple-like
// type's reference variable.
inline const BindingEntity* BindingAlias(const Entity* entity)
{
	if (entity == nullptr || entity->kind != EntityKind::Variable ||
		!static_cast<const VariableEntity*>(entity)->structured_binding)
		return nullptr;
	const BindingEntity* binding = static_cast<const BindingEntity*>(entity);
	return binding->initializer == nullptr ? binding : nullptr;
}

// The initializer of the variable a declarator declares: the one written,
// or for a tuple-like type's structured binding (which writes none) the
// get<i>(e) the analysis made (11.5p3).
inline const syntax::Initializer* VariableInitializer(const Entity* entity, const syntax::Initializer* written)
{
	if (entity != nullptr && entity->kind == EntityKind::Variable &&
		static_cast<const VariableEntity*>(entity)->structured_binding)
		return static_cast<const BindingEntity*>(entity)->initializer;
	return written;
}

} // namespace sema
