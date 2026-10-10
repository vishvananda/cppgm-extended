#include <algorithm>
#include <string>

#include "lower/program_lowering.h"
#include "sema/analyzer.h"

// The order of a program's function definitions (lowir.md "Program
// Structure"): definitions follow first demand, except within the families
// of special members the document orders: for one class, copy forms before
// move forms (its copy and move constructors; its copy and move assignment
// operators), and for one constructor its entries base before complete, for
// one destructor base, deleting, then complete (as for the thunks adjusting
// `this` alike for them). Each family's members take the positions its
// members occupy, in that order; every other definition stays where it is.

namespace lower {

namespace {

uint8_t EntryRank(EntryVariant variant)
{
	switch (variant)
	{
	case EntryVariant::Base:
		return 0;
	case EntryVariant::Deleting:
		return 1;
	case EntryVariant::Complete:
		break;
	}
	return 2;
}

// A family member at its position: grouped by family, then by position.
struct Placed
{
	uint32_t identity;
	uint8_t kind;
	uint8_t rank;
	int64_t offset;
	int64_t vcall;
	uint32_t position;
	lowir::SymbolId symbol;

	bool same_family(const Placed& other) const
	{
		return identity == other.identity && kind == other.kind && offset == other.offset && vcall == other.vcall;
	}
	bool operator<(const Placed& other) const
	{
		if (identity != other.identity)
			return identity < other.identity;
		if (kind != other.kind)
			return kind < other.kind;
		if (offset != other.offset)
			return offset < other.offset;
		if (vcall != other.vcall)
			return vcall < other.vcall;
		return position < other.position;
	}
};

bool ByRank(const Placed& a, const Placed& b)
{
	return a.rank != b.rank ? a.rank < b.rank : a.position < b.position;
}

} // namespace

void ProgramLowering::note_family_members(
	const sema::FunctionEntity* function, EntryVariant variant, lowir::SymbolId symbol, lowir::NameId base_object)
{
	// An object's layout keeps the completion order: its LowIR half reads
	// back as the program the direct compile encodes as it goes.
	if (object_mode_)
		return;
	const sema::CopyKind copy = function->copy_kind;
	const bool entries = function->constructor || function->destructor;
	const bool assignment = copy == sema::CopyKind::CopyAssignment || copy == sema::CopyKind::MoveAssignment;
	if (!entries && !assignment)
		return;
	if (copy != sema::CopyKind::None)
	{
		const bool move = copy == sema::CopyKind::MoveConstructor || copy == sema::CopyKind::MoveAssignment;
		family_members_.push_back(
			FamilyMember{ symbol, class_identity(static_cast<const sema::ClassEntity*>(function->owner->entity)),
				assignment ? FamilyKind::AssignmentForm : FamilyKind::ConstructorForm,
				static_cast<uint8_t>(move ? 1 : 0), 0, 0 });
	}
	if (entries)
		family_members_.push_back(FamilyMember{ symbol, function_identity(function, variant, symbol, base_object),
			FamilyKind::Entries, EntryRank(variant), 0, 0 });
}

void ProgramLowering::note_thunk_family(const Thunk& thunk)
{
	// A destructor's thunks adjusting `this` alike are a family of their
	// own: the deleting entry's before the complete entry's.
	if (object_mode_ || !thunk.function->destructor)
		return;
	family_members_.push_back(
		FamilyMember{ thunk.symbol, function_identity(thunk.function, thunk.variant, lowir::kNoId, 0),
			FamilyKind::Entries, EntryRank(thunk.variant), thunk.adjustment.offset, thunk.adjustment.vcall });
}

uint32_t ProgramLowering::function_identity(
	const sema::FunctionEntity* function, EntryVariant variant, lowir::SymbolId symbol, lowir::NameId base_object)
{
	// A constructor's or destructor's entries across units: its base
	// entry's object name, which each entry's definition knows or names.
	const uint32_t known = identities_.find(function);
	if (known != PointerMap::kMissing)
		return known;
	uint32_t identity = 0;
	if (unit_local(function))
		identity = kLocalIdentity + local_identities_++;
	else
	{
		if (base_object == 0 && variant == EntryVariant::Base && symbol != lowir::kNoId)
			base_object = symbol_objects_[symbol];
		if (base_object == 0)
		{
			const uint32_t base = base_symbols_.find(function);
			if (base != PointerMap::kMissing)
				base_object = symbol_objects_[base];
		}
		if (base_object == 0)
		{
			const std::string object = function_object(function, EntryVariant::Base);
			base_object = intern(object.data(), static_cast<uint32_t>(object.size()));
		}
		identity = base_object;
	}
	identities_.set(function, identity);
	return identity;
}

uint32_t ProgramLowering::class_identity(const sema::ClassEntity* entity)
{
	// A class across units: its type's encoding.
	const uint32_t known = identities_.find(entity);
	if (known != PointerMap::kMissing)
		return known;
	uint32_t identity = 0;
	if (entity_locality(entity) != Locality::External)
		identity = kLocalIdentity + local_identities_++;
	else
	{
		const std::string object = class_object(entity, abi_mangle::ABI_TARGET_TYPE);
		identity = intern(object.data(), static_cast<uint32_t>(object.size()));
	}
	identities_.set(entity, identity);
	return identity;
}

void ProgramLowering::order_special_member_families(std::vector<lowir::SymbolId>& definitions)
{
	if (family_members_.empty())
		return;
	const uint32_t kAbsent = 0xffffffffu;
	std::vector<uint32_t> position(program_.symbols.size(), kAbsent);
	for (uint32_t i = 0; i < definitions.size(); ++i)
	{
		if (position[definitions[i]] == kAbsent)
			position[definitions[i]] = i;
	}
	// The copy/move families first: an entry family then permutes entries
	// of one form only, keeping their order.
	std::vector<Placed> placed;
	std::vector<uint32_t> slots;
	for (int pass = 0; pass < 2; ++pass)
	{
		placed.clear();
		for (const FamilyMember& member : family_members_)
		{
			if ((member.kind == FamilyKind::Entries) != (pass == 1) || member.symbol >= position.size() ||
				position[member.symbol] == kAbsent)
				continue;
			placed.push_back(Placed{ member.identity, static_cast<uint8_t>(member.kind), member.rank, member.offset,
				member.vcall, position[member.symbol], member.symbol });
		}
		std::sort(placed.begin(), placed.end());
		for (size_t first = 0; first < placed.size();)
		{
			size_t last = first + 1;
			bool ordered = true;
			for (; last < placed.size() && placed[last].same_family(placed[first]); ++last)
				ordered = ordered && placed[last - 1].rank <= placed[last].rank;
			if (!ordered)
			{
				// The family's positions, ascending, receive its members by
				// rank (and position within a rank).
				slots.clear();
				for (size_t i = first; i < last; ++i)
					slots.push_back(placed[i].position);
				std::sort(placed.begin() + first, placed.begin() + last, ByRank);
				for (size_t i = first; i < last; ++i)
				{
					definitions[slots[i - first]] = placed[i].symbol;
					position[placed[i].symbol] = slots[i - first];
				}
			}
			first = last;
		}
	}
	family_members_.clear();
}

} // namespace lower
