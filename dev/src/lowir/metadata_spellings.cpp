#include "lowir/metadata_spellings.h"

#include <cstring>

#include "ir_symbol_model.h"
#include "lowir/program.h"

namespace lowir {

namespace {

const char* const kKeys[] = { "arity", "effects", "unwind", "return", "query", "result_classes", "storage", "section",
	"role", "linkage", "binding", "object", "tls_for", "keep_alias", "prefer_local", "object_root", "force_inline",
	"inline_hint", "no_inline", "replaceable" };
const char* const kEightbytes[] = { "none", "integer", "sse", "sseup" };

const char* const kRoles[] = { "", "entry", "init", "fini", "eh_allocate_exception", "eh_begin_catch", "eh_end_catch",
	"eh_rethrow", "eh_throw", "eh_personality", "eh_resume", "allocate_memory", "free_memory", "terminate",
	"pure_virtual", "dynamic_cast", "bad_cast", "bad_typeid", "rtti_class", "rtti_si", "rtti_vmi", "rtti_data",
	"bad_array_new_length", "rtti_pointer" };
const char* const kLinkages[] = { "", "c" };
const char* const kBindings[] = { "", "internal", "strong", "weak" };
const char* const kStorages[] = { "", "readonly", "thread_local" };
const char* const kArities[] = { "", "variadic" };
const char* const kEffects[] = { "", "readnone", "readonly" };
const char* const kUnwinds[] = { "", "no" };
const char* const kReturns[] = { "", "noreturn" };
const char* const kQueries[] = { "", "stable_prefix" };
const char* const kPasses[] = { "", "indirect_result", "by_address" };
const char* const kAliases[] = { "", "noalias" };
const char* const kProjections[] = { "", "array_element", "field" };

bool same(const char* spelling, const char* text, uint32_t length)
{
	return std::strlen(spelling) == length && std::memcmp(spelling, text, length) == 0;
}

} // namespace

#define LOWIR_TABLE(name, array) const SpellingTable name = { array, sizeof array / sizeof array[0] }
LOWIR_TABLE(kRoleSpellings, kRoles);
LOWIR_TABLE(kLinkageSpellings, kLinkages);
LOWIR_TABLE(kBindingSpellings, kBindings);
LOWIR_TABLE(kStorageSpellings, kStorages);
LOWIR_TABLE(kAritySpellings, kArities);
LOWIR_TABLE(kEffectsSpellings, kEffects);
LOWIR_TABLE(kUnwindSpellings, kUnwinds);
LOWIR_TABLE(kReturnSpellings, kReturns);
LOWIR_TABLE(kQuerySpellings, kQueries);
LOWIR_TABLE(kPassSpellings, kPasses);
LOWIR_TABLE(kAliasSpellings, kAliases);
LOWIR_TABLE(kProjectionSpellings, kProjections);
#undef LOWIR_TABLE

static_assert(sizeof kKeys / sizeof kKeys[0] == static_cast<unsigned>(MetadataKey::Count), "metadata keys");
static_assert(sizeof kRoles / sizeof kRoles[0] == ir_model::SR_RTTI_POINTER + 1, "roles");

const char* metadata_key_spelling(MetadataKey key)
{
	return kKeys[static_cast<unsigned>(key)];
}

MetadataKey find_metadata_key(const char* text, uint32_t length)
{
	for (unsigned i = 0; i < static_cast<unsigned>(MetadataKey::Count); ++i)
	{
		if (same(kKeys[i], text, length))
			return static_cast<MetadataKey>(i);
	}
	return MetadataKey::Count;
}

unsigned SpellingTable::find(const char* text, uint32_t length) const
{
	for (unsigned i = 1; i < count; ++i)
	{
		if (same(spellings[i], text, length))
			return i;
	}
	return 0;
}

void append_object_classes(std::string& out, uint8_t classes, uint64_t bytes)
{
	switch (classes)
	{
	case OC_STATED | OC_MEMORY:
		out += "memory";
		return;
	case OC_STATED | OC_X87:
		out += "x87";
		return;
	case OC_STATED | OC_COMPLEX_X87:
		out += "complex_x87";
		return;
	default:
		break;
	}
	if ((classes & ~(OC_STATED | 15)) != 0 || (classes & OC_STATED) == 0)
	{
		out += std::to_string(static_cast<unsigned>(classes));
		return;
	}
	out += kEightbytes[classes & 3];
	if (bytes > 8 || bytes == 0)
	{
		out += '_';
		out += kEightbytes[(classes >> 2) & 3];
	}
}

uint8_t find_object_classes(const char* text, uint32_t length, uint64_t bytes, bool extensions)
{
	if (same("memory", text, length))
		return OC_STATED | OC_MEMORY;
	if (extensions && same("x87", text, length))
		return OC_STATED | OC_X87;
	if (extensions && same("complex_x87", text, length))
		return OC_STATED | OC_COMPLEX_X87;
	// One name for each eightbyte (two when the size is unknown).
	const unsigned eightbytes = bytes == 0 || bytes > 16 ? 2u : static_cast<unsigned>((bytes + 7) / 8);
	uint8_t classes = OC_STATED;
	uint32_t at = 0;
	for (unsigned k = 0; k < eightbytes; ++k)
	{
		uint32_t end = at;
		while (end < length && text[end] != '_')
			++end;
		unsigned found = 4;
		for (unsigned cls = 0; cls < (extensions ? 4u : 3u); ++cls)
			if (same(kEightbytes[cls], text + at, end - at))
				found = cls;
		if (found == 4 || (end == length) != (k + 1 == eightbytes))
			return 0;
		classes = static_cast<uint8_t>(classes | found << (2 * k));
		at = end + 1;
	}
	return classes;
}

bool is_global_role(uint8_t role)
{
	return (role >= ir_model::SR_RTTI_CLASS && role <= ir_model::SR_RTTI_DATA) || role == ir_model::SR_RTTI_POINTER;
}

bool is_singleton_role(uint8_t role)
{
	return role != ir_model::SR_NONE && role != ir_model::SR_RTTI_DATA;
}

} // namespace lowir
