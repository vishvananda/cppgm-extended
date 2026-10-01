#pragma once

#include "semantic/model/graph.h"

#include <cstddef>
#include <cstdint>
#include <unordered_map>
#include <vector>

namespace cppgm
{
namespace semantic
{

struct ExceptionControlContextFact
{
	std::uint32_t parent, depth, region, cleanup_root;
	ExceptionControlContextFact(std::uint32_t parent_value,
		std::uint32_t depth_value)
		: parent(parent_value), depth(depth_value), region(kNoDumpEdge), cleanup_root(0) {}
};
struct GotoLifetimeSnapshot
{
	ScopeId scope;
	std::size_t count;
	GotoLifetimeSnapshot(ScopeId scope_value, std::size_t count_value)
		: scope(scope_value), count(count_value) {}
};
struct PendingGotoControlFact
{
	std::uint32_t node;
	ScopeId scope;
	std::uint32_t exception_context;
	std::vector<GotoLifetimeSnapshot> lifetimes;
	PendingGotoControlFact(std::uint32_t node_value, ScopeId scope_value,
		std::uint32_t context_value)
		: node(node_value), scope(scope_value),
		  exception_context(context_value) {}
};
struct LabelControlFact
{
	ScopeId scope;
	std::uint32_t exception_context;
	std::vector<GotoLifetimeSnapshot> lifetimes;
	LabelControlFact()
		: scope(kNoScope), exception_context(0) {}
	LabelControlFact(ScopeId scope_value, std::uint32_t context_value)
		: scope(scope_value), exception_context(context_value) {}
};
struct FunctionControlFlowFactState
{
	std::vector<ExceptionControlContextFact> contexts;
	std::uint32_t current_context;
	std::unordered_map<NameId, LabelControlFact> labels;
	std::unordered_multimap<NameId, PendingGotoControlFact> pending_gotos;
	FunctionControlFlowFactState() : current_context(0) {}
};

}
}
