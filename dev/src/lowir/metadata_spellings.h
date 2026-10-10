#pragma once

#include <cstdint>
#include <string>

// Spellings of the LowIR metadata vocabulary (lowir.md "Program Structure",
// "Functions" and "Memory and Addressing"). Each value table is indexed by
// the ir_model enumerator; entry 0 is the default, which has no spelling.
namespace lowir {

enum class MetadataKey : uint8_t
{
	// Boundary metadata, valid on functions and call signatures.
	Arity,
	Effects,
	Unwind,
	Return,
	Query,
	// Object mode (Program::object_facts): an object result's classes.
	ResultClasses,
	// Symbol metadata, in canonical presentation order.
	Storage,
	Section,
	Role,
	Linkage,
	Binding,
	Object,
	TlsFor,
	KeepAlias,
	PreferLocal,
	ObjectRoot,
	ForceInline,
	InlineHint,
	NoInline,
	// A GNU weak definition (SF_REPLACEABLE), with binding=weak.
	Replaceable,
	Count
};

inline bool is_boundary_key(MetadataKey key)
{
	return key <= MetadataKey::ResultClasses;
}

const char* metadata_key_spelling(MetadataKey key);
// MetadataKey::Count if unknown.
MetadataKey find_metadata_key(const char* text, uint32_t length);

struct SpellingTable
{
	const char* const* spellings;
	unsigned count;

	const char* operator[](unsigned index) const { return spellings[index]; }
	// The index of a spelling other than the default, or 0 if unknown.
	unsigned find(const char* text, uint32_t length) const;
};

extern const SpellingTable kRoleSpellings;
extern const SpellingTable kLinkageSpellings;
extern const SpellingTable kBindingSpellings;
extern const SpellingTable kStorageSpellings;
extern const SpellingTable kAritySpellings;
extern const SpellingTable kEffectsSpellings;
extern const SpellingTable kUnwindSpellings;
extern const SpellingTable kReturnSpellings;
extern const SpellingTable kQuerySpellings;
extern const SpellingTable kPassSpellings;
extern const SpellingTable kAliasSpellings;
extern const SpellingTable kProjectionSpellings;

// An ObjectClassBits value of an object of `bytes` bytes (0: unknown) as
// the text writes it (lowir.md "Object ABI Conventions"): `memory`, or the
// class of each eightbyte joined by `_` (`none`, `integer`, `sse`; two when
// the size is unknown); in host objects also `x87`, `complex_x87`, `sseup`
// and other values as their numbers.
void append_object_classes(std::string& out, uint8_t classes, uint64_t bytes);
// The value of such a word naming exactly the eightbytes of `bytes` (two
// if 0), the host objects' words only with `extensions`; 0 if it is none.
uint8_t find_object_classes(const char* text, uint32_t length, uint64_t bytes, bool extensions);

// Roles of global symbols (the rtti_* roles); the others are function roles.
bool is_global_role(uint8_t role);
// Every role but rtti_data belongs to at most one symbol of a program.
bool is_singleton_role(uint8_t role);

} // namespace lowir
