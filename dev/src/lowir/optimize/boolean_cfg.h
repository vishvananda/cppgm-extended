#pragma once

#include "lowir/model/program.h"

#include <vector>

namespace lowir_opt {

struct Stats;

struct CleanupCfgScratch
{
  std::vector<unsigned char> branch_values;
};

bool fold_boolean_phi_branch(
  lowir_model::Function * function, Stats * stats);
// Bypass an acyclic two-arm choice that only materializes opposite u8
// Boolean constants for an immediately following branch.  This is an O3
// pipeline dose; ordinary CFG cleanup retains its loop-oriented policy.
bool fold_trivial_boolean_phi_diamond(
  lowir_model::Function * function, Stats * stats);
// Thread an exact Boolean value through an i64-phi/u8-trunc forwarding pair
// into its consuming branch.  The incoming values must be comparison results
// or literal zero/one, so bypassing the truncation preserves truth semantics.
// This is an O3-only late CFG cleanup.
bool fold_forwarded_boolean_phi_branch(
  lowir_model::Function * function, Stats * stats);
bool fold_edge_known_branches(
  lowir_model::Function * function, Stats * stats);
bool fold_edge_known_branches(
  lowir_model::Function * function, Stats * stats,
  CleanupCfgScratch * scratch);
// The O3 form also lets an equality established by the sole predecessor edge
// decide a local equality/inequality branch on the same typed SSA value.
bool fold_edge_known_branches(
  lowir_model::Function * function, Stats * stats,
  CleanupCfgScratch * scratch, bool allow_integer_equality);
// Substitute an integer value inside the region reached only through an edge
// which proves that value equal to a literal.  SSA immutability makes the
// equality valid throughout the dominated region; phi operands remain edge
// uses and are deliberately left alone.
bool propagate_edge_integer_equalities(
  lowir_model::Function * function, Stats * stats);
// Redirect a loop latch around a pure phi/compare header when that latch's
// literal phi input decides the header branch.  Target phis are excluded so
// the new edge needs no value translation.
bool thread_constant_loop_phi_edge(
  lowir_model::Function * function, Stats * stats);
// Fold the unsigned x-1 underflow test on an edge where a preceding branch
// establishes that the same SSA value, or a stable reload of it, is nonzero.
bool fold_nonzero_underflow_branches(
  lowir_model::Function * function, Stats * stats);
// Replace a two-edge signed rejection of x < 0 followed by x > C with the
// equivalent single unsigned x > C branch.  The intermediate block must be
// private and otherwise contain only its comparison and branch.
bool fold_zero_bounded_signed_branch(
  lowir_model::Function * function, Stats * stats);
// Thread a two-input scalar phi through a short terminal scalar chain or a
// branch with a direct-return successor.  This is an O3 pipeline dose;
// ordinary CFG cleanup does not invoke it.
bool thread_terminal_phi_returns(
  lowir_model::Function * function, Stats * stats);
// Terminal folding, unreachable-block removal with dead-edge value
// rematerialization, jump bypassing, and forward block merging.  Functions
// containing phis stop after the two folds above; their CFG stays stable
// until an edge-aware transform requests repair.
// The native lowering requires every non-phi use to follow its definition
// in block order.  Reorders a function that violates that into reverse
// postorder; returns whether it moved anything.
bool restore_definition_order(lowir_model::Function * function);
// A phi that selects between the operands of its diamond's comparison is
// one of them when the comparison fixes the other: the unsigned minimum
// with the type's largest value, the unsigned maximum with zero, and either
// operand of an equality.  Closing O3 pass.
bool fold_diamond_select_identities(
  lowir_model::Function * function, Stats * stats);
// A diamond on the same condition as a dominating diamond, with arms that
// compute the same pure values (loads only when memory is untouched in
// between), yields the dominating diamond's phis.  Closing O3 pass.
bool value_number_diamonds(
  lowir_model::Function * function, Stats * stats);

bool cleanup_cfg(lowir_model::Function * function, Stats * stats);
bool cleanup_cfg(lowir_model::Function * function, Stats * stats,
                 CleanupCfgScratch * scratch);

// Shared by the CFG folds and the diamond passes (diamonds.cpp): an
// instruction that neither writes memory nor observes a write (a volatile
// load does), a block made only of those, and whether a function has a phi.
bool predicate_keeps_memory(const lowir_model::Instruction & instruction);
bool block_memory_stable(const lowir_model::Block & block);
bool function_has_phi(const lowir_model::Function & function);
// The last instruction of `block` defining `value`, or null.
const lowir_model::Instruction * local_definition(
    const lowir_model::Block & block, lowir_model::ValueId value);

}  // namespace lowir_opt
