#pragma once

#include <cstdint>
#include <vector>

#include "lowir/program.h"
#include "opt/optimizer.h"

// The direct-call graph of a program's function definitions (PA32
// inlining and pruning): dense function indices in definition order, call
// edges from typed call operands, one byte of non-call observation per
// function, and strongly connected components (Tarjan) giving recursion
// and a callee-first order. O(instructions + edges); symbols are compared
// by id, never by spelling.
namespace opt {

struct CallGraph
{
	std::vector<lowir::SymbolId> functions;      // definitions, in program order
	std::vector<uint32_t> index_of;              // per symbol: function index or kNoId
	std::vector<uint32_t> callee_first, callees; // per function: direct call sites' callees
	std::vector<uint32_t> calls_in;              // direct call sites naming each function
	std::vector<uint32_t> callers_in;            // distinct functions those sites lie in
	std::vector<uint8_t> observed;               // a non-call use: address, data, alias, role, root
	std::vector<uint8_t> recursive;              // on a call cycle (or calls itself)
	std::vector<uint32_t> component;             // SCC id
	std::vector<uint32_t> bottom_up;             // callees before callers; ties in program order
};

void build_call_graph(const lowir::Program& program, CallGraph& graph);

// Whether a definition may disappear when nothing calls or names it: weak
// or internal, no role, not an object root, not GNU-replaceable and not a
// thread-local wrapper.
bool discardable(const lowir::Program& program, lowir::SymbolId symbol);

// Whether a definition is an Itanium this-adjusting thunk (5.1.4.2: its
// object spelling begins `_ZTh` or `_ZTv`). Its body is the forwarding
// call an object back end encodes as an adjustment and a jump
// (Program::jump_thunks, recognized again in LowIR text by the call's
// shape): the optimizer leaves the body as lowered (nothing inlined into
// it, nothing simplified), so it keeps naming its target in that shape.
bool this_adjusting_thunk(const lowir::Program& program, lowir::SymbolId symbol);

// Removes discardable definitions unreachable from the program's roots
// (other definitions, roles, object roots, globals' data and aliases)
// through call and address operands. Returns the number removed.
uint32_t prune_unreachable(lowir::Program& program, OptimizerStats& stats);

} // namespace opt
