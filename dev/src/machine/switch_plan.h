#pragma once

#include <cstdint>
#include <utility>
#include <vector>

namespace machine {

struct CaseCluster
{
	int64_t lo, hi;
	uint32_t target;
};

// Canonical integer values and logical destinations; physical branches remain
// the consumer's responsibility. Repeated values retain their first target.
void cluster_switch_cases(std::vector<std::pair<int64_t, uint32_t>>& cases, std::vector<CaseCluster>& clusters,
	std::vector<uint32_t>& targets, uint32_t target_limit);

bool dense_switch(const std::vector<std::pair<int64_t, uint32_t>>& cases, uint32_t maximum_entries, uint32_t spread,
	uint32_t minimum_cases);

} // namespace machine
