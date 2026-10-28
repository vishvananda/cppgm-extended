#pragma once

#include <cstdint>
#include <vector>

#include "sema/semantic_graph.h"
#include "support/memory/arena.h"
#include "support/memory/zeroed_allocator.h"

namespace sema {

// Interns the types of one translation unit in an open-addressed table
// keyed by structure, so each distinct type is created once and compared by
// address. Constructors apply the type rules that are properties of type
// formation itself: cv-qualifying an array qualifies its elements (8.3.4p1),
// cv-qualifiers on references and functions introduced through a typedef
// are ignored (8.3.2p1, 8.3.5p6), and references collapse (8.3.2p6).
// Validity (no pointer to reference, no array of functions, ...) is the
// caller's to check.
class TypeTable
{
public:
	explicit TypeTable(Arena& arena, bool plain_char_unsigned = false);

	const Type* fundamental(FundamentalType type) const { return fundamentals_[static_cast<unsigned>(type)]; }
	// (Adding no qualifier it lacks is the type itself, without a call.)
	const Type* qualified(const Type* type, uint8_t cv)
	{
		return cv == 0 || (type->cv | cv) == type->cv ? type : qualified_anew(type, cv);
	}
	const Type* qualified_anew(const Type* type, uint8_t cv);
	const Type* pointer(const Type* pointee);
	const Type* reference(const Type* referee, bool rvalue);
	const Type* member_pointer(const Type* owner, const Type* member);
	// `dependent_bound`: the placeholder of a bound that depends on
	// template parameters (`bound` 0).
	const Type* array(const Type* element, uint64_t bound, const Type* dependent_bound = nullptr);
	// `dependent_size`: the placeholder of a lane type or size that depends
	// on template parameters (`lanes` 0), a byte size if `bytes`.
	const Type* vector(const Type* lane, uint64_t lanes, const Type* dependent_size = nullptr, bool bytes = false);
	const Type* complex(const Type* element);
	const Type* block_pointer(const Type* function);
	const Type* function(
		const Type* result, const Type* const* params, uint32_t count, bool variadic, uint8_t cv, RefQualifier ref);
	// That function type if some declaration already formed it, else null
	// (nothing interned).
	const Type* find_function(const Type* result, const Type* const* params, uint32_t count, bool variadic, uint8_t cv,
		RefQualifier ref) const;
	// The type of a class, enumeration or template parameter entity.
	const Type* named(TypeKind kind, Entity* entity);
	// Template arguments: an integral constant of `type` (an
	// unqualified integral or enumeration type), an argument pack of
	// `count` elements, and a pack expansion of `pattern`.
	const Type* value(const Type* type, uint64_t bits);
	// A pointer or reference argument (14.3.2): of `type` (a pointer,
	// std::nullptr_t or lvalue reference type), designating `entity` (a
	// variable or function; null for a null pointer).
	const Type* address(const Type* type, Entity* entity);
	const Type* pack(const Type* const* elements, uint32_t count);
	const Type* expansion(const Type* pattern);
	// A bound parameter pack: the argument pack `pack` of the
	// positional parameter `parameter`, at `level` from the outermost
	// template.
	const Type* bound_pack(const Type* pack, Entity* parameter, uint32_t level);
	// A parameter's type in the function's signature (8.3.5p5).
	const Type* adjusted_parameter(const Type* type);

	uint32_t size() const { return count_; }

private:
	const Type* intern(const Type& key);
	const Type* insert(const Type& key, uint32_t hash, uint32_t index);
	const Type* canonical_of(const Type& type);
	static uint32_t hash_of(const Type& type);
	static bool same(const Type& a, const Type& b);
	void grow();

	struct Slot
	{
		uint32_t hash;
		const Type* type;
	};

	Arena& arena_;
	std::vector<Slot, ZeroedAllocator<Slot>> slots_;
	// slots_.size() - 1, the probes' index mask.
	uint32_t mask_ = 255;
	uint32_t count_;
	const Type* fundamentals_[kFundamentalTypeCount];
	// Parameter scratch for canonical function types.
	std::vector<const Type*> scratch_;
};

} // namespace sema
