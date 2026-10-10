#include "preprocess/macro/hide_sets.h"

#include <vector>

namespace {

const uint32_t kInitialSlots = 256;

} // namespace

HideSets::HideSets()
	: nodes_(1, Node{ 0, 0, 0, nullptr, 0 }), slots_(kInitialSlots, 0), merged_set_(0), merged_other_(0),
	  merged_result_(0), index_nodes_(1, IndexNode{ { 0, 0 } })
{}

uint32_t HideSets::add(uint32_t set, const Identifier* name)
{
	uint32_t mask = static_cast<uint32_t>(slots_.size()) - 1;
	uint32_t index = SlotHash(set, name) & mask;
	for (; slots_[index] != 0; index = (index + 1) & mask)
	{
		const Node& node = nodes_[slots_[index]];
		if (node.parent == set && node.name == name)
			return slots_[index];
	}

	// The skew-binary jump pointer: two equal jumps below combine into one.
	const Node& parent = nodes_[set];
	const Node& parent_jump = nodes_[parent.jump];
	const uint32_t jump = set != 0 &&
			parent.depth - parent_jump.depth == parent_jump.depth - nodes_[parent_jump.jump].depth
		? parent_jump.jump
		: set;
	const uint32_t id = static_cast<uint32_t>(nodes_.size());
	nodes_.push_back(Node{ set, parent.depth + 1, jump, name, parent.summary | SummaryBit(name) });
	if (2 * nodes_.size() > slots_.size())
	{
		grow();
		return id;
	}
	slots_[index] = id;
	return id;
}

uint32_t HideSets::merge_slow(uint32_t set, uint32_t other)
{
	if (set == merged_set_ && other == merged_other_)
		return merged_result_;
	merged_set_ = set;
	merged_other_ = other;
	// The names of `other` from its common ancestor with `set` down are
	// already in `set`; if `set` is that ancestor, `other` is the union.
	const uint32_t shared = common_ancestor(set, other);
	if (shared == set)
		return merged_result_ = other;
	merge_names_.clear();
	for (; other != shared; other = nodes_[other].parent)
		merge_names_.push_back(nodes_[other].name);
	for (auto name = merge_names_.rbegin(); name != merge_names_.rend(); ++name)
	{
		if (!contains(set, *name))
			set = add(set, *name);
	}
	return merged_result_ = set;
}

uint32_t HideSets::ancestor_at(uint32_t set, uint32_t depth) const
{
	while (nodes_[set].depth > depth)
		set = nodes_[nodes_[set].jump].depth >= depth ? nodes_[set].jump : nodes_[set].parent;
	return set;
}

uint32_t HideSets::common_ancestor(uint32_t a, uint32_t b) const
{
	// At equal depths the jump pointers go to equal depths.
	if (nodes_[a].depth > nodes_[b].depth)
		a = ancestor_at(a, nodes_[b].depth);
	else
		b = ancestor_at(b, nodes_[a].depth);
	while (a != b)
	{
		if (nodes_[a].jump != nodes_[b].jump)
		{
			a = nodes_[a].jump;
			b = nodes_[b].jump;
		}
		else
		{
			a = nodes_[a].parent;
			b = nodes_[b].parent;
		}
	}
	return a;
}

bool HideSets::indexed_contains(uint32_t set, const Identifier* name) const
{
	// Bit `level` of the id chooses the child at depth `level`.
	uint32_t ref = index_of(set);
	for (unsigned level = 0; ref != 0 && (ref & kIndexLeaf) == 0; ++level)
		ref = index_nodes_[ref].child[(name->id >> level) & 1];
	return ref == (kIndexLeaf | name->id);
}

uint32_t HideSets::index_of(uint32_t set) const
{
	// Built on the nearest ancestor that has an index (or the empty set), so
	// each node's index costs one insertion and shares the rest. A node
	// always holds a name, so its index is never the empty reference 0.
	if (index_roots_.size() < nodes_.size())
		index_roots_.resize(nodes_.size(), 0);
	index_path_.clear();
	uint32_t base = set;
	while (base != 0 && index_roots_[base] == 0)
	{
		index_path_.push_back(base);
		base = nodes_[base].parent;
	}
	uint32_t root = index_roots_[base];
	for (auto node = index_path_.rbegin(); node != index_path_.rend(); ++node)
	{
		root = index_insert(root, nodes_[*node].name->id, 0);
		index_roots_[*node] = root;
	}
	return root;
}

uint32_t HideSets::index_insert(uint32_t ref, uint32_t id, unsigned level) const
{
	// Copies the path to the key's place, sharing every other subtree. The
	// recursion is bounded by the 31 bits of an id.
	if (ref == 0)
		return kIndexLeaf | id;
	IndexNode node;
	if ((ref & kIndexLeaf) != 0)
	{
		if (ref == (kIndexLeaf | id))
			return ref;
		// Two keys: branch on this level's bit, deeper while they agree.
		node.child[0] = 0;
		node.child[1] = 0;
		node.child[((ref & ~kIndexLeaf) >> level) & 1] = ref;
	}
	else
	{
		node = index_nodes_[ref];
	}
	const unsigned bit = (id >> level) & 1;
	node.child[bit] = index_insert(node.child[bit], id, level + 1);
	index_nodes_.push_back(node);
	return static_cast<uint32_t>(index_nodes_.size() - 1);
}

void HideSets::grow()
{
	// Rebuilds the index, including the node just appended.
	slots_.assign(slots_.size() * 2, 0);
	const uint32_t mask = static_cast<uint32_t>(slots_.size()) - 1;
	for (uint32_t id = 1; id < nodes_.size(); ++id)
	{
		uint32_t index = SlotHash(nodes_[id].parent, nodes_[id].name) & mask;
		while (slots_[index] != 0)
			index = (index + 1) & mask;
		slots_[index] = id;
	}
}
