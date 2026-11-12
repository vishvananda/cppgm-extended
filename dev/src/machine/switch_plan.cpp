#include "machine/switch_plan.h"

#include <algorithm>
#include <climits>
#include "support/flat_map.h"

namespace machine {

namespace {
struct TargetHash
{
	size_t operator()(uint32_t target) const { return target; }
};
} // namespace

void cluster_switch_cases(std::vector<std::pair<int64_t, uint32_t>>& cases, std::vector<CaseCluster>& clusters,
	std::vector<uint32_t>& targets, uint32_t target_limit)
{
	std::stable_sort(cases.begin(), cases.end(),
		[](const std::pair<int64_t, uint32_t>& a, const std::pair<int64_t, uint32_t>& b) { return a.first < b.first; });
	clusters.clear();
	targets.clear();
	FlatMap<uint32_t, uint8_t, TargetHash> seen;
	for (const auto& c : cases)
	{
		if (!clusters.empty() && clusters.back().hi == c.first)
			continue;
		if (!clusters.empty() && clusters.back().target == c.second && clusters.back().hi != INT64_MAX &&
			clusters.back().hi + 1 == c.first)
			clusters.back().hi = c.first;
		else
			clusters.push_back(CaseCluster{ c.first, c.first, c.second });
		if (targets.size() > target_limit)
			continue;
		if (target_limit <= 8)
		{
			if (std::find(targets.begin(), targets.end(), c.second) == targets.end())
				targets.push_back(c.second);
		}
		else if (seen.find(c.second) == nullptr)
		{
			seen.insert(c.second, 1);
			targets.push_back(c.second);
		}
	}
}

bool dense_switch(const std::vector<std::pair<int64_t, uint32_t>>& cases, uint32_t maximum_entries, uint32_t spread,
	uint32_t minimum_cases)
{
	if (cases.empty() || cases.size() < minimum_cases)
		return false;
	const uint64_t span = static_cast<uint64_t>(cases.back().first) - static_cast<uint64_t>(cases.front().first);
	return span < maximum_entries && span + 1 <= uint64_t(spread) * cases.size();
}

} // namespace machine
