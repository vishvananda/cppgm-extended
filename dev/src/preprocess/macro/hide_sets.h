#pragma once

#include <cstdint>
#include <vector>

#include "lex/identifier_table.h"

// Sets of macro names that a token may not invoke (the per-token
// "blacklists" of the course's recursion rule, macros.md). A set is a
// compact id and 0 is the empty set. Sets only grow by one name at a time,
// set ∪ {name}; each (set, name) pair is interned, so the invocations that
// share a head hide set and macro share the result.
//
// Membership walks the set's chain of names from the newest. Every node also
// keeps a 64-bit summary of the names on its chain (one bit per name, chosen
// by the identifier's hash), and the walk stops at the first node whose
// summary lacks the name's bit, so most negative queries take one test. A
// walk that would pass kWalkLimit nodes asks the node's index instead: a
// persistent binary trie of the chain's identifier ids, built on first use
// from the nearest ancestor's index with one insertion per node, so a query
// on a deep set costs at most one step per id bit.
//
// Sets form a tree by their last name; each node keeps its depth and a
// skew-binary jump pointer, so ancestor and common-ancestor queries take
// O(log depth) steps, and a union with an ancestor set is free.
class HideSets
{
public:
	HideSets();

	uint32_t add(uint32_t set, const Identifier* name);
	// set ∪ other: free if either is an ancestor of the other, otherwise in
	// time proportional to the names of `other` above their common
	// ancestor, each tested for membership. The last union is remembered.
	uint32_t merge(uint32_t set, uint32_t other)
	{
		if (other == 0 || other == set)
			return set;
		return set == 0 ? other : merge_slow(set, other);
	}
	// The set before its last add(), and the name added last.
	uint32_t without_last(uint32_t set) const { return nodes_[set].parent; }
	const Identifier* last(uint32_t set) const { return nodes_[set].name; }

	bool contains(uint32_t set, const Identifier* name) const
	{
		const uint64_t bit = SummaryBit(name);
		for (uint32_t steps = 0; set != 0 && (nodes_[set].summary & bit) != 0; ++steps)
		{
			if (nodes_[set].name == name)
				return true;
			if (steps == kWalkLimit)
				return indexed_contains(set, name);
			set = nodes_[set].parent;
		}
		return false;
	}

	uint32_t size() const { return static_cast<uint32_t>(nodes_.size() - 1); }

private:
	struct Node
	{
		uint32_t parent;
		uint32_t depth;
		uint32_t jump;
		const Identifier* name;
		uint64_t summary;
	};

	// A trie node: a child reference per bit value. A reference is 0 for
	// no key, kIndexLeaf | id for the one key below, or a node index.
	struct IndexNode
	{
		uint32_t child[2];
	};

	static const uint32_t kWalkLimit = 32;
	static const uint32_t kIndexLeaf = 0x80000000u;

	uint32_t merge_slow(uint32_t set, uint32_t other);
	uint32_t ancestor_at(uint32_t set, uint32_t depth) const;
	uint32_t common_ancestor(uint32_t a, uint32_t b) const;
	bool indexed_contains(uint32_t set, const Identifier* name) const;
	uint32_t index_of(uint32_t set) const;
	uint32_t index_insert(uint32_t ref, uint32_t id, unsigned level) const;

	static uint64_t SummaryBit(const Identifier* name) { return uint64_t(1) << (name->hash & 63); }
	static uint32_t SlotHash(uint32_t set, const Identifier* name) { return (set * 0x9E3779B1u) ^ name->hash; }

	void grow();

	std::vector<Node> nodes_;
	// Open-addressed index of nodes by (parent, name); 0 marks a free slot.
	std::vector<uint32_t> slots_;
	// The last merge() and its result.
	uint32_t merged_set_;
	uint32_t merged_other_;
	uint32_t merged_result_;
	std::vector<const Identifier*> merge_names_;
	// Membership indexes of deep sets, a cache owned by the set table: the
	// root reference of each node's index (0 until built) and the trie
	// nodes, shared between the indexes.
	mutable std::vector<uint32_t> index_roots_;
	mutable std::vector<IndexNode> index_nodes_;
	mutable std::vector<uint32_t> index_path_;
};
