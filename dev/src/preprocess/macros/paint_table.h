#ifndef CPPGM_PREPROCESS_MACROS_PAINT_TABLE_H
#define CPPGM_PREPROCESS_MACROS_PAINT_TABLE_H
#include "preprocess/macros/macro_processor.h"
#include "support/containers/flat_hash_map.h"
#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>
namespace cppgm
{
namespace macro_detail
{
// Interned spellings, and the sets of macro names a token is painted with
// (the names whose expansion produced it, which it may not re-expand).
typedef std::uint32_t SpellingId;
typedef std::uint32_t PaintId;
// The high bit of a paint id marks an allocation-free singleton set.
const PaintId kSingletonPaint = UINT32_C(0x80000000);

inline std::uint64_t PairKey(std::uint32_t first, std::uint32_t second)
{
	return (static_cast<std::uint64_t>(first) << 32) | second;
}

class SpellingTable
{
public:
	explicit SpellingTable(MacroProcessingStats* stats);
	SpellingId Intern(const std::string& spelling);
	const std::string& Get(SpellingId id) const;

private:
	std::size_t FindPosition(const std::string& spelling) const;
	void Rehash(std::size_t capacity);

	std::vector<std::string> spellings_;
	std::vector<SpellingId> slots_;
	MacroProcessingStats* stats_;
	std::size_t identifier_bytes_;
};

// Paint sets as a persistent binary trie over the 32 bits of a spelling id;
// adds and merges are memoized at their roots.
class PaintTable
{
public:
	explicit PaintTable(MacroProcessingStats* stats);
	bool Contains(PaintId paint, SpellingId macro_name) const;
	PaintId Add(PaintId paint, SpellingId macro_name);
	PaintId Merge(PaintId first, PaintId second);

private:
	struct Node
	{
		PaintId zero;
		PaintId one;

		Node() : zero(0), one(0) {}
		Node(PaintId zero_child, PaintId one_child)
			: zero(zero_child), one(one_child)
		{}
	};

	static bool IsSingleton(PaintId paint);
	static SpellingId SingletonName(PaintId paint);
	static PaintId MakeSingleton(SpellingId name);
	void Validate(PaintId paint) const;
	PaintId InternNode(PaintId zero, PaintId one);
	PaintId MaterializeSingleton(SpellingId name);
	PaintId AddAt(PaintId node, SpellingId name, int bit);
	PaintId MergeAt(PaintId first, PaintId second, int bit);
	void RegisterRoot(PaintId root);
	void UpdateStats();

	MacroProcessingStats* stats_;
	std::vector<Node> nodes_;
	std::vector<unsigned char> root_flags_;
	std::size_t root_count_;
	std::size_t singleton_count_;
	detail::FlatHashMap<std::uint64_t, PaintId> add_cache_;
	detail::FlatHashMap<std::uint64_t, PaintId> merge_cache_;
	detail::FlatHashMap<SpellingId, PaintId> singleton_tries_;
	detail::FlatHashMap<SpellingId, unsigned char> singleton_roots_;
};
}
}
#endif
