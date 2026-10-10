#include "sema/analyzer.h"

#include <algorithm>

// The Itanium C++ ABI's primary bases (2.2, 2.4 II.3): a nearly empty
// class, the base sharing a class's table pointer (a nearly empty virtual
// base included), and where an indirect primary base lies (with the
// subobject it is the primary base of, 2.4 IV, as Clang's
// BaseSubobjectInfo claims them). Every layout follows them (host
// objects, course objects and LowIR alike). Each class's facts are
// computed once; the walks over the inheritance graph visit each
// class once.
namespace sema {

namespace {

const ClassEntity* BaseClassOf(const BaseClass* base)
{
	return static_cast<const ClassEntity*>(base->base);
}

} // namespace

bool Analyzer::itanium_nearly_empty(const ClassEntity* cls)
{
	// Itanium 2.2: a dynamic class with no data but its table pointer: no
	// non-static data member but zero-width bit-fields, every non-virtual
	// direct base empty or nearly empty (at most one nearly empty), none
	// placed past offset 0; its virtual bases do not count. That is a
	// non-virtual size of one pointer, as Clang tests it (GCC's
	// check_bases, then layout_empty_base clearing it for an empty base
	// placed elsewhere). The class is laid out first.
	if (!cls->dynamic)
		return false;
	const uint32_t known = itanium_nearly_empty_.find(cls);
	if (known != PointerMap::kMissing)
		return known == 2;
	uint64_t size = 0, alignment = 0;
	const bool nearly = type_size(cls->type, size, alignment, 0) && cls->nvsize == 8;
	itanium_nearly_empty_.set(cls, nearly ? 2 : 1);
	return nearly;
}

const ClassEntity* Analyzer::itanium_primary(const ClassEntity* cls, bool& is_virtual)
{
	// Itanium 2.4 II.3: the first non-virtual dynamic direct base; else the
	// first nearly empty virtual base in inheritance graph order that is no
	// other base's primary base (an indirect primary base), else the first
	// nearly empty one of those. A class with a dynamic base is dynamic, so
	// the bases alone decide it.
	is_virtual = false;
	const uint32_t known = itanium_primary_index_.find(cls);
	if (known != PointerMap::kMissing)
	{
		is_virtual = (known & 1) != 0;
		return (known >> 1) == 0 ? nullptr : itanium_primaries_[(known >> 1) - 1];
	}
	const ClassEntity* primary = nullptr;
	bool virtual_below = false;
	for (const BaseClass* base = cls->bases; base != nullptr && primary == nullptr; base = base->next)
	{
		virtual_below = virtual_below || base->is_virtual || VirtualBaseCount(BaseClassOf(base)) != 0;
		if (!base->is_virtual && BaseClassOf(base)->dynamic)
			primary = BaseClassOf(base);
	}
	if (primary == nullptr && virtual_below)
	{
		primary = itanium_virtual_primary(cls);
		is_virtual = primary != nullptr;
	}
	uint32_t index = 0;
	if (primary != nullptr)
	{
		itanium_primaries_.push_back(primary);
		index = static_cast<uint32_t>(itanium_primaries_.size());
	}
	itanium_primary_index_.set(cls, (index << 1) | (is_virtual ? 1 : 0));
	return primary;
}

const ClassEntity* Analyzer::itanium_virtual_primary(const ClassEntity* cls)
{
	// The classes below `cls`, each once; the virtual primary bases of
	// those with virtual bases are the indirect primary bases. Their own
	// primary bases and the virtual bases' nearly emptiness are computed
	// after the walk and before the preorder walk below: either may lay a
	// class out, which may come back here or to indirect_primaries and
	// reuse the scratch.
	std::vector<const ClassEntity*> below;
	std::vector<const ClassEntity*> reached_virtually;
	{
		PointerMap& seen = itanium_seen_;
		seen.clear();
		std::vector<const ClassEntity*>& stack = itanium_stack_;
		stack.assign(1, cls);
		while (!stack.empty())
		{
			const ClassEntity* top = stack.back();
			stack.pop_back();
			for (const BaseClass* base = top->bases; base != nullptr; base = base->next)
			{
				const ClassEntity* next = BaseClassOf(base);
				const uint32_t mark = seen.find(next);
				const uint32_t now = (mark == PointerMap::kMissing ? 0 : mark) | (base->is_virtual ? 2 : 1);
				if (base->is_virtual && (mark == PointerMap::kMissing || (mark & 2) == 0))
					reached_virtually.push_back(next);
				seen.set(next, now);
				if (mark != PointerMap::kMissing)
					continue;
				below.push_back(next);
				stack.push_back(next);
			}
		}
	}
	for (const ClassEntity* next : reached_virtually)
		itanium_nearly_empty(next);
	std::vector<const ClassEntity*> indirect;
	for (const ClassEntity* next : below)
	{
		bool virtual_primary = false;
		const ClassEntity* own = VirtualBaseCount(next) != 0 ? itanium_primary(next, virtual_primary) : nullptr;
		if (own != nullptr && virtual_primary)
			indirect.push_back(own);
	}
	PointerMap& indirect_set = itanium_seen_;
	indirect_set.clear();
	for (const ClassEntity* entry : indirect)
		indirect_set.set(entry, 1);
	// Preorder over the inheritance graph's edges (each base, then below
	// it), a class's bases walked at its first visit only.
	PointerMap& expanded = itanium_expanded_;
	expanded.clear();
	const ClassEntity* first = nullptr;
	std::vector<const BaseClass*>& walk = itanium_walk_;
	walk.clear();
	for (const BaseClass* base = cls->bases; base != nullptr; base = base->next)
		walk.push_back(base);
	std::reverse(walk.begin(), walk.end());
	while (!walk.empty())
	{
		const BaseClass* base = walk.back();
		walk.pop_back();
		const ClassEntity* next = BaseClassOf(base);
		if (base->is_virtual && itanium_nearly_empty(next))
		{
			if (indirect_set.find(next) == PointerMap::kMissing)
				return next;
			if (first == nullptr)
				first = next;
		}
		if (expanded.find(next) != PointerMap::kMissing)
			continue;
		expanded.set(next, 1);
		const size_t mark = walk.size();
		for (const BaseClass* step = next->bases; step != nullptr; step = step->next)
			walk.push_back(step);
		std::reverse(walk.begin() + static_cast<std::ptrdiff_t>(mark), walk.end());
	}
	return first;
}

const BaseClass* Analyzer::layout_primary(const ClassEntity* entity)
{
	// Itanium's: the first non-virtual dynamic base, else a nearly empty
	// virtual one as the class's entry for it (its base-specifier when
	// direct; the class's virtual bases are listed).
	for (const BaseClass* base = entity->bases; base != nullptr; base = base->next)
	{
		if (!base->is_virtual && BaseClassOf(base)->dynamic)
			return base;
	}
	bool is_virtual = false;
	const ClassEntity* primary = itanium_primary(entity, is_virtual);
	for (uint32_t i = 0; primary != nullptr && i < VirtualBaseCount(entity); ++i)
	{
		if (VirtualBases(entity)[i].base->base == primary)
			return VirtualBases(entity)[i].base;
	}
	return nullptr;
}

bool Analyzer::indirect_primaries(const ClassEntity* entity)
{
	// Itanium 2.4 IV's indirect primary bases of a class: the virtual
	// primary bases of the classes below with virtual bases (marked in
	// indirect_primary_set_; their primary bases computed after the walk,
	// which may reuse its scratch); whether there is one.
	std::vector<const ClassEntity*> below;
	PointerMap& seen = itanium_seen_;
	seen.clear();
	std::vector<const ClassEntity*>& stack = itanium_stack_;
	stack.assign(1, entity);
	while (!stack.empty())
	{
		const ClassEntity* top = stack.back();
		stack.pop_back();
		for (const BaseClass* base = top->bases; base != nullptr; base = base->next)
		{
			const ClassEntity* next = BaseClassOf(base);
			if (VirtualBaseCount(next) == 0 || seen.find(next) != PointerMap::kMissing)
				continue;
			seen.set(next, 1);
			stack.push_back(next);
			below.push_back(next);
		}
	}
	std::vector<const ClassEntity*> primaries;
	for (const ClassEntity* next : below)
	{
		bool is_virtual = false;
		const ClassEntity* primary = itanium_primary(next, is_virtual);
		if (primary != nullptr && is_virtual)
			primaries.push_back(primary);
	}
	indirect_primary_set_.clear();
	for (const ClassEntity* primary : primaries)
		indirect_primary_set_.set(primary, 1);
	return !primaries.empty();
}

bool Analyzer::has_indirect_primaries(const ClassEntity* entity)
{
	// Whether a class has an indirect primary base (indirect_primaries'
	// set is not empty): a base with virtual bases has a virtual primary
	// base or one of its own. Once per class, its bases' first (an explicit
	// stack: chains of any length).
	std::vector<const ClassEntity*>& stack = has_indirect_stack_;
	stack.assign(1, entity);
	while (!stack.empty())
	{
		const ClassEntity* top = stack.back();
		if (has_indirect_.find(top) != PointerMap::kMissing)
		{
			stack.pop_back();
			continue;
		}
		const size_t waiting = stack.size();
		bool any = false;
		for (const BaseClass* base = top->bases; base != nullptr; base = base->next)
		{
			const ClassEntity* next = BaseClassOf(base);
			if (VirtualBaseCount(next) == 0)
				continue;
			const uint32_t known = has_indirect_.find(next);
			if (known == PointerMap::kMissing)
			{
				stack.push_back(next);
				continue;
			}
			bool is_virtual = false;
			itanium_primary(next, is_virtual);
			any = any || known == 2 || is_virtual;
		}
		if (stack.size() != waiting)
			continue;
		has_indirect_.set(top, any ? 2 : 1);
		stack.pop_back();
	}
	return has_indirect_.find(entity) == 2;
}

void Analyzer::claim_primaries(const ClassEntity* entity, uint32_t location)
{
	// The class's base subobjects depth first, bases in declaration order,
	// a virtual base's at its first meeting (Clang's
	// ComputeBaseSubobjectInfo): a subobject whose class has a virtual
	// primary base claims it when it meets it unclaimed, else (met first
	// below the subobject) once its bases are walked, over any claim made
	// below it. The class's own virtual primary base is its own (offset
	// 0, kMissing).
	claim_nodes_.clear();
	claim_frames_.clear();
	claim_virtual_nodes_.clear();
	claimants_.clear();
	const auto enter = [&](const BaseClass* edge, uint32_t parent) {
		const ClassEntity* cls = BaseClassOf(edge);
		if (edge->is_virtual && claim_virtual_nodes_.find(cls) != PointerMap::kMissing)
			return;
		const uint32_t node = static_cast<uint32_t>(claim_nodes_.size());
		claim_nodes_.push_back(ClaimNode{ cls, edge, parent });
		if (edge->is_virtual)
			claim_virtual_nodes_.set(cls, node);
		const ClassEntity* claim_after = nullptr;
		bool is_virtual = false;
		const ClassEntity* primary = VirtualBaseCount(cls) != 0 ? itanium_primary(cls, is_virtual) : nullptr;
		if (primary != nullptr && is_virtual)
		{
			if (claim_virtual_nodes_.find(primary) == PointerMap::kMissing)
				claim_after = primary;
			else if (claimants_.find(primary) == PointerMap::kMissing)
				claimants_.set(primary, node);
		}
		claim_frames_.push_back(ClaimFrame{ node, cls->bases, claim_after });
	};
	for (const BaseClass* base = entity->bases; base != nullptr; base = base->next)
	{
		enter(base, PointerMap::kMissing);
		while (!claim_frames_.empty())
		{
			ClaimFrame& frame = claim_frames_.back();
			if (frame.next != nullptr)
			{
				const BaseClass* edge = frame.next;
				frame.next = edge->next;
				spend_layout_work(location);
				enter(edge, frame.node);
				continue;
			}
			if (frame.claim_after != nullptr)
				claimants_.set(frame.claim_after, frame.node);
			claim_frames_.pop_back();
		}
	}
	bool is_virtual = false;
	const ClassEntity* own = itanium_primary(entity, is_virtual);
	if (own != nullptr && is_virtual)
		claimants_.set(own, PointerMap::kMissing);
}

void Analyzer::plan_claims(const ClassEntity* entity, const BaseClass* primary)
{
	// Each indirect primary base's place relative to the placement that
	// decides it (its resolver): up from its claimant through non-virtual
	// base-specifiers to one of the class's direct non-virtual bases, or
	// to a virtual base the layout allocates (an indirect primary one met
	// on the way is where its own claimant is), or to the class's virtual
	// primary base. Each is listed under its resolver's base-specifier
	// (the class's entry for a virtual base), placed and recorded with it.
	claim_vbases_.clear();
	for (uint32_t i = 0; i < VirtualBaseCount(entity); ++i)
		claim_vbases_.set(VirtualBases(entity)[i].base->base, i);
	claim_heads_.clear();
	claim_entries_.clear();
	for (uint32_t i = 0; i < VirtualBaseCount(entity); ++i)
	{
		BaseClass* entry = VirtualBases(entity)[i].base;
		const ClassEntity* shared = BaseClassOf(entry);
		const uint32_t claimant = claimants_.find(shared);
		if (entry == primary || indirect_primary_set_.find(shared) == PointerMap::kMissing ||
			claimant == PointerMap::kMissing)
			continue;
		uint64_t relative = 0;
		const BaseClass* resolver = nullptr;
		for (uint32_t node = claimant, steps = 0; resolver == nullptr && steps <= claim_nodes_.size(); ++steps)
		{
			const ClaimNode& item = claim_nodes_[node];
			if (!item.edge->is_virtual)
			{
				relative += item.edge->offset;
				if (item.parent == PointerMap::kMissing)
					resolver = item.edge;
				else
					node = item.parent;
				continue;
			}
			const BaseClass* holder = VirtualBases(entity)[claim_vbases_.find(item.cls)].base;
			const uint32_t next = claimants_.find(item.cls);
			if (holder == primary || indirect_primary_set_.find(item.cls) == PointerMap::kMissing ||
				next == PointerMap::kMissing)
				resolver = holder;
			else
				node = next;
		}
		if (resolver == nullptr)
			continue;
		const uint32_t head = claim_heads_.find(resolver);
		claim_heads_.set(resolver, static_cast<uint32_t>(claim_entries_.size()));
		claim_entries_.push_back(ClaimEntry{ entry, relative, head });
	}
}

bool Analyzer::claimed_conflict(const BaseClass* resolver, uint64_t offset, uint32_t location)
{
	// Clang's CanPlaceBaseAtOffset walks a base with the primary virtual
	// bases it claims.
	for (uint32_t k = claim_heads_.find(resolver); k != PointerMap::kMissing; k = claim_entries_[k].next)
	{
		if (empty_subobjects(BaseClassOf(claim_entries_[k].vbase), offset + claim_entries_[k].relative,
				EmptyWalk::Conflict, location))
			return true;
	}
	return false;
}

void Analyzer::record_claimed(const BaseClass* resolver, uint64_t offset, EmptyWalk walk, uint32_t location)
{
	// The resolver placed at `offset`: its claimed primary bases' offsets and
	// their empty subobjects.
	for (uint32_t k = claim_heads_.find(resolver); k != PointerMap::kMissing; k = claim_entries_[k].next)
	{
		ClaimEntry& item = claim_entries_[k];
		item.vbase->offset = offset + item.relative;
		empty_subobjects(BaseClassOf(item.vbase), item.vbase->offset, walk, location);
	}
}

} // namespace sema
