#pragma once

#include <cstdint>

#include "sema/entity_records.h"

// A polymorphic class's virtual facts (10.3, Itanium C++ ABI 2.5.2): its
// table's slots and its table group, the semantic facts its tables are
// emitted from (see Analyzer::virtual_facts and vtable_group).
namespace sema {

struct BaseClass;
struct ClassEntity;
struct FunctionEntity;
struct VtableGroup;

// A virtual base of a class (10.1p4: one subobject however often the
// hierarchy names it): the class's base-specifier for it when direct
// (`direct`), else one made for it; its offset is the subobject's in a
// complete object of the class, once laid out.
struct VirtualBase
{
	BaseClass* base;
	bool direct;
};

// One slot of a polymorphic class's virtual table (10.3, Itanium C++ ABI
// 2.5.2): the final overrider in the class of the virtual function the
// slot stands for; a virtual destructor has two, its complete-object and
// deleting entries. `introducer` is the function that introduced the slot
// (the first of its primary chain to take it: one without an overridden
// function, or a covariant overrider whose result needs adjustment);
// `adjusted` the function a covariant result in it is adjusted to: the one
// sharing the slot nearest the class (calls through every class of the
// chain sharing it expect its result, at offset zero from theirs).
enum class SlotKind : uint8_t
{
	Function,
	CompleteDestructor,
	DeletingDestructor
};

struct VirtualSlot
{
	FunctionEntity* function;
	SlotKind kind;
	const FunctionEntity* introducer;
	const FunctionEntity* adjusted;
};

// A polymorphic class's virtual facts, recorded at its completion: its
// virtual table (the primary base's slots first, then those of the
// virtual functions it introduces in declaration order; built on demand,
// see Analyzer::virtual_slots) and its size, the primary base sharing its
// virtual table pointer (Itanium 2.4: the first polymorphic base), the
// count of virtual functions whose final overrider is pure (10.4p2: the
// class is abstract when some is), the key function whose definition
// emits the table (Itanium 5.2.3; null for none), a virtual destructor's
// deallocation function (12.4p12), whether a class of the primary
// chain has another polymorphic base (secondary tables), and then the
// class's table group (null for a primary chain alone).
struct VirtualFacts
{
	VirtualSlot* vtable;
	uint32_t vtable_size;
	uint32_t pure_count;
	const BaseClass* primary_base;
	FunctionEntity* key_function;
	const FunctionEntity* deleting_deallocator;
	bool secondary_bases;
	const VtableGroup* group;
	// The class's virtual bases, direct and indirect, in inheritance graph
	// order (Itanium 2.4: depth first, left to right, first occurrence),
	// recorded at completion, and its non-virtual alignment once laid out
	// (a class with a virtual base is dynamic: only such a class has them).
	const VirtualBase* virtual_bases;
	uint32_t virtual_base_count;
	uint64_t nvalign;
};

// A dynamic base subobject off a complete class's primary chain, in
// subobject order (the non-virtual ones depth first, bases in declaration
// order, then each virtual base's in inheritance graph order): its class,
// the base-specifier reaching it from its parent subobject's class (the
// parent's offset plus that base's gives its own; a virtual base's parent
// is the complete class, its base-specifier's offset its offset there),
// the parent's segment (0: the complete class, at offset 0 with its
// primary chain), its entries (a slot of its class's table each), whether
// it shares its table pointer with its parent's (its parent's primary
// base), and the segment of the virtual base it is (or is in), 0 for none.
struct VtableSegment
{
	const ClassEntity* cls;
	const BaseClass* base;
	uint32_t parent;
	uint32_t first_entry;
	bool shared;
	uint32_t within;
};

// A segment's slot: the final overrider in the complete class (10.3p2;
// a pure one when the final overrider is pure) and the segment whose
// subobject (the overrider's class's) its `this` designates.
struct VtableEntry
{
	FunctionEntity* function;
	SlotKind kind;
	uint32_t overrider;
};

// The virtual table group of a class with polymorphic subobjects off its
// primary chain (Itanium C++ ABI 2.5.2): segment 0 is the class's own
// table (virtual_slots), then a segment per such subobject.
struct VtableGroup
{
	const VtableSegment* segments;
	uint32_t segment_count;
	const VtableEntry* entries;
	uint32_t entry_count;
};

// A class's virtual bases (none for a class without virtual facts), and
// the alignment its subobject is placed by as a base (Itanium 2.4: its
// non-virtual alignment).
inline uint32_t VirtualBaseCount(const ClassEntity* cls)
{
	return cls->virtuals != nullptr ? cls->virtuals->virtual_base_count : 0;
}

inline const VirtualBase* VirtualBases(const ClassEntity* cls)
{
	return cls->virtuals != nullptr ? cls->virtuals->virtual_bases : nullptr;
}

inline uint64_t NonVirtualAlignment(const ClassEntity* cls)
{
	return VirtualBaseCount(cls) != 0 ? cls->virtuals->nvalign : cls->alignment;
}

} // namespace sema
