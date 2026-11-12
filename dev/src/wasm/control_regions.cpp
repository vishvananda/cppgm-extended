#include "wasm/control.h"

#include <algorithm>
#include <functional>
#include <queue>

namespace wasm {
namespace {

using lowir::kNoId;
using Graph = std::vector<std::vector<uint32_t>>;

std::vector<uint32_t> topological(const Graph& graph, const std::vector<uint32_t>& rank)
{
	std::vector<uint32_t> incoming(graph.size(), 0), result;
	for (const auto& edges : graph)
		for (uint32_t to : edges)
			++incoming[to];
	using Ranked = std::pair<uint32_t, uint32_t>;
	std::priority_queue<Ranked, std::vector<Ranked>, std::greater<Ranked>> ready;
	for (uint32_t i = 0; i < graph.size(); ++i)
		if (incoming[i] == 0)
			ready.emplace(rank[i], i);
	while (!ready.empty())
	{
		const uint32_t from = ready.top().second;
		ready.pop();
		result.push_back(from);
		for (uint32_t to : graph[from])
			if (--incoming[to] == 0)
				ready.emplace(rank[to], to);
	}
	return result;
}

// Iterative Tarjan: O(vertices + edges), without a host-stack depth bound.
std::vector<std::vector<uint32_t>> components(const Graph& graph, std::vector<uint32_t>& component)
{
	std::vector<uint32_t> number(graph.size(), kNoId), low(graph.size(), 0), stack;
	std::vector<uint8_t> active(graph.size(), 0);
	std::vector<std::pair<uint32_t, uint32_t>> frames;
	std::vector<std::vector<uint32_t>> result;
	component.resize(graph.size());
	uint32_t next = 0;
	for (uint32_t root = 0; root < graph.size(); ++root)
	{
		if (number[root] != kNoId)
			continue;
		auto enter = [&](uint32_t node) {
			number[node] = low[node] = next++;
			active[node] = 1;
			stack.push_back(node);
			frames.emplace_back(node, 0);
		};
		enter(root);
		while (!frames.empty())
		{
			const uint32_t node = frames.back().first;
			uint32_t& edge = frames.back().second;
			if (edge < graph[node].size())
			{
				const uint32_t to = graph[node][edge++];
				if (number[to] == kNoId)
					enter(to);
				else if (active[to])
					low[node] = std::min(low[node], number[to]);
				continue;
			}
			frames.pop_back();
			if (!frames.empty())
				low[frames.back().first] = std::min(low[frames.back().first], low[node]);
			if (low[node] != number[node])
				continue;
			result.emplace_back();
			uint32_t member;
			do
			{
				member = stack.back();
				stack.pop_back();
				active[member] = 0;
				component[member] = static_cast<uint32_t>(result.size() - 1);
				result.back().push_back(member);
			} while (member != node);
		}
	}
	return result;
}

uint32_t dispatch_group(ControlPlan& plan, const std::vector<uint32_t>& members, const std::vector<uint32_t>& children,
	const std::vector<uint32_t>& rank)
{
	std::vector<uint32_t> ordered = members;
	std::sort(ordered.begin(), ordered.end(), [&](uint32_t a, uint32_t b) { return rank[a] < rank[b]; });
	if (plan.dispatch_region.empty())
	{
		plan.dispatch_region.assign(plan.block_count, kNoId);
		plan.dispatch_case.assign(plan.block_count, kNoId);
	}
	const uint32_t id = static_cast<uint32_t>(plan.regions.size());
	plan.regions.emplace_back();
	plan.regions[id].dispatch = true;
	plan.regions[id].header = plan.entry(children[ordered.front()]);
	for (uint32_t i = 0; i < ordered.size(); ++i)
	{
		const uint32_t child = children[ordered[i]], entry = plan.entry(child);
		plan.dispatch_region[entry] = id;
		plan.dispatch_case[entry] = i;
		plan.regions[id].children.push_back(child);
	}
	plan.dispatch_children += static_cast<uint32_t>(members.size());
	return plan.block_count + id - 1;
}

} // namespace

bool order_control_region(ControlPlan& plan, uint32_t region, const std::vector<std::pair<uint32_t, uint32_t>>& edges,
	const opt::Cfg& cfg, std::vector<uint32_t>& indices)
{
	const auto children = plan.regions[region].children;
	Graph graph(children.size());
	std::vector<uint32_t> rank;
	for (uint32_t i = 0; i < children.size(); ++i)
	{
		indices[children[i]] = i;
		rank.push_back(cfg.rpo_index[plan.entry(children[i])]);
	}
	for (const auto& edge : edges)
		graph[indices[edge.first]].push_back(indices[edge.second]);
	auto order = topological(graph, rank);
	if (order.size() == children.size())
	{
		for (uint32_t i = 0; i < order.size(); ++i)
			plan.regions[region].children[i] = children[order[i]];
		return true;
	}
	std::vector<uint32_t> component;
	const auto groups = components(graph, component);
	// Keep the fallback bounded and emit each child exactly once. Large
	// cycles retain the whole-function table; no branch-tail duplication.
	uint32_t total = plan.dispatch_children;
	for (const auto& group : groups)
		if (group.size() > 1)
		{
			if (group.size() > 32 || (total += static_cast<uint32_t>(group.size())) > 128)
				return false;
		}
	Graph condensed(groups.size());
	std::vector<uint32_t> nodes, ranks;
	for (const auto& group : groups)
	{
		const uint32_t child = group.size() == 1 ? children[group.front()]
												 : dispatch_group(plan, group, children, rank);
		nodes.push_back(child);
		ranks.push_back(cfg.rpo_index[plan.entry(child)]);
	}
	for (uint32_t from = 0; from < graph.size(); ++from)
		for (uint32_t to : graph[from])
			if (component[from] != component[to])
				condensed[component[from]].push_back(component[to]);
	order = topological(condensed, ranks);
	plan.regions[region].children.clear();
	for (uint32_t i : order)
		plan.regions[region].children.push_back(nodes[i]);
	return order.size() == groups.size();
}

} // namespace wasm
