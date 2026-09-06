#include "preprocess/macros/paint_table.h"
#include "support/exception_types.h"
#include <algorithm>
#include <functional>

namespace cppgm
{
namespace macro_detail
{
namespace
{
using detail::MixedHash;

__attribute__((cold, noinline, noreturn))
void ThrowPreprocessingResourceLimit(const char* message) { throw ResourceLimitError(message, CompilerErrorDomain::PREPROCESSING); }
__attribute__((cold, noinline, noreturn))
void ThrowPreprocessingInternalError(const char* message) { throw InternalCompilerError(message, CompilerErrorDomain::PREPROCESSING); }
}

SpellingTable::SpellingTable(MacroProcessingStats* stats)
	: slots_(16, 0), stats_(stats), identifier_bytes_(0)
{
	spellings_.push_back(std::string());
}

SpellingId SpellingTable::Intern(const std::string& spelling)
{
	std::size_t position = FindPosition(spelling);
	if (slots_[position] != 0)
		return slots_[position];
	// The high bit is reserved for allocation-free singleton paint sets.
	if (spellings_.size() >= static_cast<std::size_t>(kSingletonPaint))
		ThrowPreprocessingResourceLimit("too many distinct preprocessing spellings");
	if ((spellings_.size() + 1) * 10 >= slots_.size() * 7)
	{
		Rehash(slots_.size() * 2);
		position = FindPosition(spelling);
	}
	const SpellingId id = static_cast<SpellingId>(spellings_.size());
	spellings_.push_back(spelling);
	slots_[position] = id;
	identifier_bytes_ += spelling.size();
	if (stats_)
	{
		stats_->interned_identifiers = spellings_.size() - 1;
		stats_->interned_identifier_bytes = identifier_bytes_;
	}
	return id;
}

const std::string& SpellingTable::Get(SpellingId id) const
{
	if (id >= spellings_.size())
		ThrowPreprocessingInternalError("invalid interned spelling ID");
	return spellings_[id];
}

std::size_t SpellingTable::FindPosition(const std::string& spelling) const
{
	std::size_t position = MixedHash(std::hash<std::string>()(spelling)) &
		(slots_.size() - 1);
	while (slots_[position] != 0 &&
		spellings_[slots_[position]] != spelling)
		position = (position + 1) & (slots_.size() - 1);
	return position;
}

void SpellingTable::Rehash(std::size_t capacity)
{
	slots_.assign(capacity, 0);
	for (SpellingId id = 1; id < spellings_.size(); ++id)
		slots_[FindPosition(spellings_[id])] = id;
}

PaintTable::PaintTable(MacroProcessingStats* stats)
	: stats_(stats), singleton_count_(0)
{
	// Node 0 is the empty trie and node 1 is the terminal membership
	// marker. Internal nodes form persistent paths in one contiguous arena;
	// complete add/merge transitions are memoized at their compact roots.
	nodes_.push_back(Node());
	nodes_.push_back(Node());
	root_count_ = 0;
	if (stats_)
	{
		root_flags_.push_back(1);
		root_flags_.push_back(0);
		root_count_ = 1;
	}
	UpdateStats();
}

bool PaintTable::Contains(PaintId paint, SpellingId macro_name) const
{
	Validate(paint);
	if (IsSingleton(paint))
		return SingletonName(paint) == macro_name;
	PaintId node = paint;
	for (int bit = 31; bit >= 0; --bit)
	{
		if (node == 0 || node == 1)
			return false;
		node = ((macro_name >> bit) & 1U) ?
			nodes_[node].one : nodes_[node].zero;
	}
	return node == 1;
}

PaintId PaintTable::Add(PaintId paint, SpellingId macro_name)
{
	const std::uint64_t key = PairKey(paint, macro_name);
	const PaintId* cached = add_cache_.Find(key);
	if (cached)
		return *cached;
	Validate(paint);
	PaintId id;
	if (paint == 0)
		id = MakeSingleton(macro_name);
	else if (IsSingleton(paint) && SingletonName(paint) == macro_name)
		id = paint;
	else
	{
		if (IsSingleton(paint))
			paint = MaterializeSingleton(SingletonName(paint));
		id = AddAt(paint, macro_name, 31);
	}
	add_cache_.Insert(key, id);
	RegisterRoot(id);
	return id;
}

PaintId PaintTable::Merge(PaintId first, PaintId second)
{
	Validate(first);
	Validate(second);
	if (first == second || second == 0)
	{
		RegisterRoot(first);
		return first;
	}
	if (first == 0)
	{
		RegisterRoot(second);
		return second;
	}
	if (IsSingleton(first))
		first = MaterializeSingleton(SingletonName(first));
	if (IsSingleton(second))
		second = MaterializeSingleton(SingletonName(second));
	const PaintId id = MergeAt(first, second, 31);
	RegisterRoot(id);
	return id;
}

bool PaintTable::IsSingleton(PaintId paint)
{
	return (paint & kSingletonPaint) != 0;
}

SpellingId PaintTable::SingletonName(PaintId paint)
{
	return paint & ~kSingletonPaint;
}

PaintId PaintTable::MakeSingleton(SpellingId name)
{
	if (name == 0 || (name & kSingletonPaint) != 0)
		ThrowPreprocessingInternalError("invalid singleton paint name");
	return kSingletonPaint | name;
}

void PaintTable::Validate(PaintId paint) const
{
	if (IsSingleton(paint))
	{
		if (SingletonName(paint) == 0)
			ThrowPreprocessingInternalError("invalid singleton paint ID");
		return;
	}
	if (paint >= nodes_.size())
		ThrowPreprocessingInternalError("invalid macro paint ID");
}

PaintId PaintTable::InternNode(PaintId zero, PaintId one)
{
	if (zero == 0 && one == 0)
		return 0;
	if (nodes_.size() >= static_cast<std::size_t>(kSingletonPaint))
		ThrowPreprocessingResourceLimit("too many macro paint trie nodes");
	const PaintId id = static_cast<PaintId>(nodes_.size());
	nodes_.push_back(Node(zero, one));
	if (stats_)
		root_flags_.push_back(0);
	UpdateStats();
	return id;
}

PaintId PaintTable::MaterializeSingleton(SpellingId name)
{
	const PaintId* cached = singleton_tries_.Find(name);
	if (cached)
		return *cached;
	const PaintId result = AddAt(0, name, 31);
	singleton_tries_.Insert(name, result);
	return result;
}

PaintId PaintTable::AddAt(PaintId node, SpellingId name, int bit)
{
	if (bit < 0)
		return 1;
	const PaintId zero = node == 0 ? 0 : nodes_[node].zero;
	const PaintId one = node == 0 ? 0 : nodes_[node].one;
	if ((name >> bit) & 1U)
	{
		const PaintId added = AddAt(one, name, bit - 1);
		return added == one ? node : InternNode(zero, added);
	}
	const PaintId added = AddAt(zero, name, bit - 1);
	return added == zero ? node : InternNode(added, one);
}

PaintId PaintTable::MergeAt(PaintId first, PaintId second, int bit)
{
	if (first == second || second == 0)
		return first;
	if (first == 0)
		return second;
	if (bit < 0)
		return 1;
	const PaintId low = std::min(first, second);
	const PaintId high = std::max(first, second);
	const std::uint64_t key = PairKey(low, high);
	const PaintId* cached = merge_cache_.Find(key);
	if (cached)
		return *cached;
	const PaintId zero = MergeAt(nodes_[first].zero,
		nodes_[second].zero, bit - 1);
	const PaintId one = MergeAt(nodes_[first].one,
		nodes_[second].one, bit - 1);
	PaintId result;
	if (zero == nodes_[first].zero && one == nodes_[first].one)
		result = first;
	else if (zero == nodes_[second].zero && one == nodes_[second].one)
		result = second;
	else
		result = InternNode(zero, one);
	merge_cache_.Insert(key, result);
	return result;
}

void PaintTable::RegisterRoot(PaintId root)
{
	if (!stats_)
		return;
	if (IsSingleton(root))
	{
		const SpellingId name = SingletonName(root);
		if (!singleton_roots_.Find(name))
		{
			singleton_roots_.Insert(name, 1);
			++singleton_count_;
			++root_count_;
			UpdateStats();
		}
		return;
	}
	if (!root_flags_[root])
	{
		root_flags_[root] = 1;
		++root_count_;
		UpdateStats();
	}
}

void PaintTable::UpdateStats()
{
	if (!stats_)
		return;
	stats_->paint_roots = root_count_;
	stats_->paint_singletons = singleton_count_;
	stats_->paint_nodes = nodes_.size();
}
}
}
