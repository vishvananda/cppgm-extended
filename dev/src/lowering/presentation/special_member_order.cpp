#include "lowering/presentation/special_member_order.h"
#include "lowering/ir/model.h"

#include <algorithm>
#include <cstddef>
#include <map>
#include <string>
#include <utility>
#include <vector>

namespace cppgm
{
namespace lowering
{
namespace presentation
{

namespace
{

// One family a definition belongs to, and its rank inside it.  A constructor
// belongs to two: the copy/move family of its class and the entry family of
// its signature.
struct FamilyMembership
{
	std::string family;
	int rank;
};

// The families are stated in terms of the ABI entry points themselves, so
// the keys are read from the object name the mangler produced: the last
// `C1`/`C2`, `aS` or `D0`/`D1`/`D2` marker names the entry point, and the
// parameter encoding after it tells a copy form (`ERK`, a const lvalue
// reference to the class) from a move form (`EO`, an rvalue reference).
std::size_t LastMarker(const std::string& object, const char* first,
	const char* second)
{
	const std::size_t a = object.rfind(first);
	const std::size_t b = second ? object.rfind(second) : std::string::npos;
	if (a == std::string::npos) return b;
	if (b == std::string::npos) return a;
	return std::max(a, b);
}

void ConstructorMemberships(const std::string& object, std::size_t marker,
	std::vector<FamilyMembership>* out)
{
	const std::string owner = object.substr(0, marker);
	const char entry = object[marker + 1];
	const std::string suffix = object.substr(marker + 2);
	FamilyMembership form;
	form.family = owner + " constructor copy-move";
	if (suffix.compare(0, 3, "ERK") == 0) form.rank = 0;
	else if (suffix.compare(0, 2, "EO") == 0) form.rank = 1;
	else form.rank = -1;
	if (form.rank >= 0) out->push_back(form);
	FamilyMembership entry_point;
	entry_point.family = owner + " constructor entry " + suffix;
	entry_point.rank = entry == '2' ? 0 : 1;
	out->push_back(entry_point);
}

void Memberships(const std::string& object,
	std::vector<FamilyMembership>* out)
{
	out->clear();
	if (object.empty()) return;
	const std::size_t constructor = std::max(
		LastMarker(object, "C1", 0), LastMarker(object, "C2", 0));
	if (constructor != std::string::npos)
	{
		ConstructorMemberships(object, constructor, out);
		return;
	}
	const std::size_t assignment = object.rfind("aS");
	if (assignment != std::string::npos)
	{
		const std::string suffix = object.substr(assignment + 2);
		FamilyMembership form;
		form.family = object.substr(0, assignment) + " assignment copy-move";
		if (suffix.compare(0, 3, "ERK") == 0) form.rank = 30;
		else if (suffix.compare(0, 2, "EO") == 0) form.rank = 40;
		else return;
		out->push_back(form);
		return;
	}
	// The destructor marker is followed by the parameter list's `E`.
	std::size_t destructor = std::string::npos;
	for (std::size_t at = object.size(); at >= 3; --at)
	{
		const std::size_t candidate = at - 3;
		if (object[candidate] == 'D' &&
			(object[candidate + 1] == '0' || object[candidate + 1] == '1' ||
			 object[candidate + 1] == '2') &&
			object[candidate + 2] == 'E')
		{
			destructor = candidate;
			break;
		}
	}
	if (destructor == std::string::npos) return;
	FamilyMembership entry_point;
	entry_point.family = object.substr(0, destructor) + " destructor entry";
	const char entry = object[destructor + 1];
	entry_point.rank = entry == '2' ? 50 : entry == '0' ? 51 : 52;
	out->push_back(entry_point);
}

struct Member
{
	std::size_t definition;
	int rank;
};

// Sort the members of one family by rank, keeping their relative order
// within a rank, across the positions the family occupies in `order`.
void OrderFamily(const std::vector<Member>& members,
	std::vector<std::size_t>* order,
	std::vector<std::size_t>* position_of)
{
	std::vector<std::pair<std::size_t, int> > placed;
	placed.reserve(members.size());
	for (std::size_t i = 0; i < members.size(); ++i)
		placed.push_back(std::make_pair(
			(*position_of)[members[i].definition], members[i].rank));
	std::vector<std::size_t> positions;
	positions.reserve(placed.size());
	for (std::size_t i = 0; i < placed.size(); ++i)
		positions.push_back(placed[i].first);
	std::sort(positions.begin(), positions.end());
	std::vector<std::pair<std::pair<int, std::size_t>, std::size_t> > ranked;
	ranked.reserve(placed.size());
	for (std::size_t i = 0; i < members.size(); ++i)
		ranked.push_back(std::make_pair(
			std::make_pair(placed[i].second, placed[i].first),
			members[i].definition));
	std::sort(ranked.begin(), ranked.end());
	for (std::size_t i = 0; i < ranked.size(); ++i)
	{
		(*order)[positions[i]] = ranked[i].second;
		(*position_of)[ranked[i].second] = positions[i];
	}
}

}  // namespace

void OrderSpecialMemberFamilies(ir::Program* program)
{
	const std::size_t count = program->functions.size();
	std::vector<std::string> families;
	std::map<std::string, std::vector<Member> > members;
	std::vector<FamilyMembership> memberships;
	for (std::size_t i = 0; i < count; ++i)
	{
		const ir::Symbol& symbol =
			program->symbols[program->functions[i].symbol];
		if (!symbol.object_name.valid()) continue;
		Memberships(program->strings.get(symbol.object_name), &memberships);
		for (std::size_t m = 0; m < memberships.size(); ++m)
		{
			std::vector<Member>& family = members[memberships[m].family];
			if (family.empty()) families.push_back(memberships[m].family);
			Member member;
			member.definition = i;
			member.rank = memberships[m].rank;
			family.push_back(member);
		}
	}
	std::vector<std::size_t> order(count), position_of(count);
	for (std::size_t i = 0; i < count; ++i) order[i] = position_of[i] = i;
	bool moved = false;
	// Copy/move families first, then entry families: an entry family only
	// swaps two entries of the same form, so it cannot undo the first pass.
	for (int pass = 0; pass < 2; ++pass)
		for (std::size_t f = 0; f < families.size(); ++f)
		{
			const std::string& family = families[f];
			const bool entry_family =
				family.find(" entry") != std::string::npos;
			if (entry_family != (pass == 1)) continue;
			const std::vector<Member>& family_members = members[family];
			int last_rank = 0;
			bool monotone = true;
			// Members were collected in emission order, which is still their
			// order unless an earlier family moved one; check by position.
			std::vector<std::pair<std::size_t, int> > by_position;
			for (std::size_t i = 0; i < family_members.size(); ++i)
				by_position.push_back(std::make_pair(
					position_of[family_members[i].definition],
					family_members[i].rank));
			std::sort(by_position.begin(), by_position.end());
			for (std::size_t i = 0; i < by_position.size() && monotone; ++i)
			{
				if (i && by_position[i].second < last_rank) monotone = false;
				last_rank = by_position[i].second;
			}
			if (monotone) continue;
			OrderFamily(family_members, &order, &position_of);
			moved = true;
		}
	if (!moved) return;
	std::vector<ir::Function> reordered;
	reordered.reserve(count);
	for (std::size_t i = 0; i < count; ++i)
		reordered.push_back(program->functions[order[i]]);
	program->functions.swap(reordered);
}

}
}
}
