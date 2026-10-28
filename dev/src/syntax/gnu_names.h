#pragma once

#include <cstdint>
#include <cstring>
#include <vector>

#include "lex/gnu_builtins.h"
#include "lex/identifier_table.h"

namespace syntax {

// The GNU names the grammar treats specially (type traits, `__typeof__`
// `__attribute__` and `__constinit`), by identifier id: one table lookup
// per identifier.
class GnuNames
{
public:
	// The kinds of `__typeof__` and `__typeof`, of `__attribute__` and
	// `__attribute`, of the complex type specifiers (`_Complex`, ...) and of
	// the part operators `__real__` and `__imag__` (with `__real`,
	// `__imag`); a trait's is 1 + GnuTrait, below kFirstSpecial.
	static const uint8_t kTypeof = 0xff;
	static const uint8_t kAttribute = 0xfe;
	static const uint8_t kComplex = 0xfd;
	static const uint8_t kRealPart = 0xfc;
	static const uint8_t kImagPart = 0xfb;
	static const uint8_t kConstinit = 0xfa;
	static const uint8_t kFirstSpecial = 0xfa;

	explicit GnuNames(IdentifierTable& identifiers)
	{
		for (uint32_t i = 0; i < static_cast<uint32_t>(GnuTrait::Declval); ++i)
			mark(identifiers, GnuTraitName(static_cast<GnuTrait>(i)), static_cast<uint8_t>(i + 1));
		mark(identifiers, "__typeof__", kTypeof);
		mark(identifiers, "__typeof", kTypeof);
		mark(identifiers, "__attribute__", kAttribute);
		mark(identifiers, "__attribute", kAttribute);
		for (const char* spelling : { "_Complex", "__complex__", "__complex" })
			mark(identifiers, spelling, kComplex);
		mark(identifiers, "__real__", kRealPart);
		mark(identifiers, "__real", kRealPart);
		mark(identifiers, "__imag__", kImagPart);
		mark(identifiers, "__imag", kImagPart);
		mark(identifiers, "__constinit", kConstinit);
	}

	// 0 for an identifier of no special GNU name.
	uint8_t kind(const Identifier* identifier) const
	{
		return identifier->id < table_.size() ? table_[identifier->id] : 0;
	}

private:
	void mark(IdentifierTable& identifiers, const char* spelling, uint8_t kind)
	{
		const Identifier* identifier = identifiers.intern(spelling, static_cast<uint32_t>(std::strlen(spelling)));
		if (table_.size() <= identifier->id)
			table_.resize(identifier->id + 1, 0);
		table_[identifier->id] = kind;
	}

	std::vector<uint8_t> table_;
};

} // namespace syntax
