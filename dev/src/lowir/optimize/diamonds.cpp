// The closing O3 passes over two-arm diamonds: a phi that selects between
// the operands of its own comparison, and a diamond that repeats a
// dominating diamond's condition and pure arms.  Split from boolean_cfg.cpp,
// whose memory predicates they share.
#include "lowir/optimize/boolean_cfg.h"
#include "lowir/optimize/errors.h"
#include "lowir/analysis/function.h"
#include "lowir/optimize/pipeline.h"
#include "lowir/optimize/support.h"
#include "lowir/analysis/phi_edges.h"
#include "lowir/optimize/scalar_rules.h"
#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <utility>
#include <vector>

namespace lowir_opt {
namespace {

using lowir_analysis::Graph;
using lowir_analysis::build_graph;
using lowir_model::Block;
using lowir_model::Function;
using lowir_model::Instruction;
using lowir_model::LowOperation;
using lowir_model::Operand;

}  // namespace

namespace {

// A two-arm choice: `parent` branches on `condition` to `then_arm` and
// `else_arm`, each with that one predecessor, each ending in a jump to
// `merge`, which has exactly those two predecessors.
struct Diamond
{
  std::size_t parent;
  std::size_t then_arm;
  std::size_t else_arm;
  std::size_t merge;
  Operand condition;
};

bool arm_jumps_to(const Block & arm, lowir_model::BlockId merge)
{
  if(arm.instructions.empty()) return false;
  const Instruction & terminal = arm.instructions.back();
  return terminal.kind == Instruction::IK_JUMP &&
    terminal.first.kind == Operand::OP_LABEL &&
    terminal.first.block == merge;
}

// Every diamond of the function whose arms hold nothing but pure values and
// nonvolatile loads.
std::vector<Diamond> collect_diamonds(const Function & function,
                                      const Graph & graph)
{
  std::vector<Diamond> diamonds;
  for(std::size_t merge = 0; merge < function.blocks.size(); ++merge) {
    const Block & merge_block = function.blocks[merge];
    if(graph.predecessors[merge].size() != 2 ||
       graph.eh_targets[static_cast<std::uint32_t>(merge_block.id)])
      continue;
    const std::size_t left = graph.predecessors[merge][0];
    const std::size_t right = graph.predecessors[merge][1];
    if(left == right || graph.predecessors[left].size() != 1 ||
       graph.predecessors[right].size() != 1 ||
       graph.predecessors[left][0] != graph.predecessors[right][0])
      continue;
    const std::size_t parent = graph.predecessors[left][0];
    if(parent == merge || parent == left || parent == right ||
       function.blocks[parent].instructions.empty())
      continue;
    const Block & left_block = function.blocks[left];
    const Block & right_block = function.blocks[right];
    if(graph.eh_targets[static_cast<std::uint32_t>(left_block.id)] ||
       graph.eh_targets[static_cast<std::uint32_t>(right_block.id)] ||
       !arm_jumps_to(left_block, merge_block.id) ||
       !arm_jumps_to(right_block, merge_block.id))
      continue;
    bool pure_arms = true;
    for(std::size_t arm = 0; arm < 2 && pure_arms; ++arm) {
      const Block & block = arm == 0 ? left_block : right_block;
      for(std::size_t i = 0; i + 1 < block.instructions.size(); ++i)
        if(block.instructions[i].kind == Instruction::IK_PHI ||
           !predicate_keeps_memory(block.instructions[i]) ||
           !block.instructions[i].dest.valid()) {
          pure_arms = false;
          break;
        }
    }
    if(!pure_arms) continue;
    const Instruction & branch = function.blocks[parent].instructions.back();
    if(branch.kind != Instruction::IK_BRANCH ||
       branch.first.kind != Operand::OP_TEMP ||
       branch.second.kind != Operand::OP_LABEL ||
       branch.third.kind != Operand::OP_LABEL)
      continue;
    Diamond diamond;
    diamond.parent = parent;
    diamond.merge = merge;
    diamond.condition = branch.first;
    if(branch.second.block == left_block.id &&
       branch.third.block == right_block.id) {
      diamond.then_arm = left;
      diamond.else_arm = right;
    } else if(branch.second.block == right_block.id &&
              branch.third.block == left_block.id) {
      diamond.then_arm = right;
      diamond.else_arm = left;
    } else {
      continue;
    }
    diamonds.push_back(diamond);
  }
  return diamonds;
}

bool phi_arm_inputs(const Instruction & phi, lowir_model::BlockId then_id,
                    lowir_model::BlockId else_id, Operand * then_value,
                    Operand * else_value)
{
  if(phi.kind != Instruction::IK_PHI || phi.args.size() != 4) return false;
  bool have_then = false, have_else = false;
  for(std::size_t i = 0; i + 1 < phi.args.size(); i += 2) {
    if(phi.args[i].kind != Operand::OP_LABEL) return false;
    if(phi.args[i].block == then_id) {
      *then_value = phi.args[i + 1];
      have_then = true;
    } else if(phi.args[i].block == else_id) {
      *else_value = phi.args[i + 1];
      have_else = true;
    }
  }
  return have_then && have_else;
}

bool integer_all_ones(const Operand & operand, const lowir_model::LowType & type)
{
  if(operand.kind != Operand::OP_INTEGER || !operand.has_int_value)
    return false;
  const std::size_t width = lowir_model::lowir_type_bit_width(type);
  const std::uint64_t mask = width >= 64 ? ~std::uint64_t(0) :
    (std::uint64_t(1) << width) - 1;
  return (static_cast<std::uint64_t>(operand.int_value) & mask) == mask;
}

// The value a diamond's phi selects when the parent's comparison decides
// it: `x <u y ? x : y` is the unsigned minimum, which is `x` when `y` is
// the type's largest value; `x >u y ? x : y` the maximum, `x` when `y` is
// zero; and a choice between the two operands of an equality is whichever
// operand both arms name once the equal edge is folded.
bool select_identity(const Instruction & compare, const Operand & then_value,
                     const Operand & else_value, Operand * result)
{
  if(compare.kind != Instruction::IK_CMP) return false;
  const Operand & x = compare.first;
  const Operand & y = compare.second;
  const bool then_x = same_operand(then_value, x);
  const bool then_y = same_operand(then_value, y);
  const bool else_x = same_operand(else_value, x);
  const bool else_y = same_operand(else_value, y);
  const LowOperation::Kind op = compare.op.kind;
  if(op == LowOperation::LOP_EQ) {
    // On the true edge x == y, so an arm naming either operand names the
    // false arm's value.
    if(else_x && (then_x || then_y)) { *result = x; return true; }
    if(else_y && (then_x || then_y)) { *result = y; return true; }
    return false;
  }
  if(op == LowOperation::LOP_NE) {
    if(then_x && (else_x || else_y)) { *result = x; return true; }
    if(then_y && (else_x || else_y)) { *result = y; return true; }
    return false;
  }
  // Unsigned minimum and maximum: the operand selected when the
  // comparison holds is the minimum for `<`/`<=` and the maximum for
  // `>`/`>=`; the other arm holds the other operand.
  const bool minimum_of_x_y = ((op == LowOperation::LOP_ULT ||
                                op == LowOperation::LOP_ULE) &&
                               then_x && else_y) ||
    ((op == LowOperation::LOP_UGT || op == LowOperation::LOP_UGE) &&
     then_y && else_x);
  const bool maximum_of_x_y = ((op == LowOperation::LOP_UGT ||
                                op == LowOperation::LOP_UGE) &&
                               then_x && else_y) ||
    ((op == LowOperation::LOP_ULT || op == LowOperation::LOP_ULE) &&
     then_y && else_x);
  if(minimum_of_x_y) {
    if(integer_all_ones(y, compare.type)) { *result = x; return true; }
    if(integer_all_ones(x, compare.type)) { *result = y; return true; }
    if(lowir_opt::is_zero(x) || lowir_opt::is_zero(y)) {
      *result = lowir_opt::is_zero(x) ? x : y;
      return true;
    }
    return false;
  }
  if(maximum_of_x_y) {
    if(lowir_opt::is_zero(y)) { *result = x; return true; }
    if(lowir_opt::is_zero(x)) { *result = y; return true; }
    if(integer_all_ones(x, compare.type) ||
       integer_all_ones(y, compare.type)) {
      *result = integer_all_ones(x, compare.type) ? x : y;
      return true;
    }
    return false;
  }
  return false;
}

// Matching state for two diamonds: the blocks of the earlier diamond's
// loads that the later one recomputes, which memory must not have changed
// since.
struct DiamondMatch
{
  const Function * function;
  const lowir_analysis::ValueIndex * values;
  std::vector<std::size_t> earlier_load_blocks;
};

bool same_instruction_shape(const Instruction & a, const Instruction & b)
{
  return a.kind == b.kind && a.op.kind == b.op.kind &&
    a.type == b.type && a.source_type == b.source_type &&
    a.byte_count == b.byte_count &&
    a.byte_alignment == b.byte_alignment &&
    a.volatile_access == b.volatile_access &&
    a.index_projection == b.index_projection &&
    a.args.size() == b.args.size();
}

bool operands_correspond(const Operand & first, const Operand & second,
                         DiamondMatch * match, std::size_t depth);

// Two values agree when they are one value or pure computations of the
// same shape over agreeing operands; a nonvolatile load agrees with a load
// of an agreeing address provided memory holds between them, which the
// caller checks from the recorded block.
bool values_correspond(lowir_model::ValueId first, lowir_model::ValueId second,
                       DiamondMatch * match, std::size_t depth)
{
  if(first == second) return true;
  if(depth == 0) return false;
  const lowir_analysis::ValueDefinition a = match->values->definition(first);
  const lowir_analysis::ValueDefinition b = match->values->definition(second);
  // A copy names its source.
  if(a.kind == lowir_analysis::ValueDefinition::INSTRUCTION) {
    const Instruction & x =
      match->function->blocks[a.block].instructions[a.instruction];
    if(x.kind == Instruction::IK_COPY && x.first.kind == Operand::OP_TEMP)
      return values_correspond(x.first.value, second, match, depth - 1);
  }
  if(b.kind == lowir_analysis::ValueDefinition::INSTRUCTION) {
    const Instruction & y =
      match->function->blocks[b.block].instructions[b.instruction];
    if(y.kind == Instruction::IK_COPY && y.first.kind == Operand::OP_TEMP)
      return values_correspond(first, y.first.value, match, depth - 1);
  }
  if(a.kind != lowir_analysis::ValueDefinition::INSTRUCTION ||
     b.kind != lowir_analysis::ValueDefinition::INSTRUCTION)
    return false;
  const Instruction & x =
    match->function->blocks[a.block].instructions[a.instruction];
  const Instruction & y =
    match->function->blocks[b.block].instructions[b.instruction];
  if(x.kind == Instruction::IK_PHI || !predicate_keeps_memory(x) ||
     !same_instruction_shape(x, y))
    return false;
  if(!operands_correspond(x.first, y.first, match, depth - 1) ||
     !operands_correspond(x.second, y.second, match, depth - 1) ||
     !operands_correspond(x.third, y.third, match, depth - 1))
    return false;
  for(std::size_t i = 0; i < x.args.size(); ++i)
    if(!operands_correspond(x.args[i], y.args[i], match, depth - 1))
      return false;
  if(x.kind == Instruction::IK_LOAD) match->earlier_load_blocks.push_back(a.block);
  return true;
}

bool operands_correspond(const Operand & first, const Operand & second,
                         DiamondMatch * match, std::size_t depth)
{
  if(first.kind == Operand::OP_TEMP && second.kind == Operand::OP_TEMP)
    return values_correspond(first.value, second.value, match, depth);
  if(first.kind == Operand::OP_TEMP || second.kind == Operand::OP_TEMP)
    return false;
  return same_operand(first, second);
}

const std::size_t kDiamondMatchDepth = 8;

// Whether memory is untouched on every path from `from` to `to`, both
// included.
bool memory_stable_between(const Function & function, const Graph & graph,
                           std::size_t from, std::size_t to)
{
  std::vector<unsigned char> forward(function.blocks.size(), 0);
  std::vector<std::size_t> work(1, from);
  forward[from] = 1;
  while(!work.empty()) {
    const std::size_t block = work.back();
    work.pop_back();
    for(std::size_t i = 0; i < graph.successors[block].size(); ++i) {
      const std::size_t successor = graph.successors[block][i];
      if(!forward[successor]) {
        forward[successor] = 1;
        work.push_back(successor);
      }
    }
  }
  if(!forward[to]) return false;
  std::vector<unsigned char> backward(function.blocks.size(), 0);
  work.assign(1, to);
  backward[to] = 1;
  while(!work.empty()) {
    const std::size_t block = work.back();
    work.pop_back();
    for(std::size_t i = 0; i < graph.predecessors[block].size(); ++i) {
      const std::size_t predecessor = graph.predecessors[block][i];
      if(!backward[predecessor]) {
        backward[predecessor] = 1;
        work.push_back(predecessor);
      }
    }
  }
  for(std::size_t block = 0; block < function.blocks.size(); ++block)
    if(forward[block] && backward[block] &&
       !block_memory_stable(function.blocks[block]))
      return false;
  return true;
}

}  // namespace

bool fold_diamond_select_identities(Function * function, Stats * stats)
{
  if(!function_has_phi(*function)) return false;
  const Graph graph = build_graph(*function, stats);
  const std::vector<Diamond> diamonds = collect_diamonds(*function, graph);
  bool changed = false;
  for(std::size_t d = 0; d < diamonds.size(); ++d) {
    const Diamond & diamond = diamonds[d];
    const Block & then_block = function->blocks[diamond.then_arm];
    const Block & else_block = function->blocks[diamond.else_arm];
    if(then_block.instructions.size() != 1 ||
       else_block.instructions.size() != 1)
      continue;
    const Instruction * compare = local_definition(
      function->blocks[diamond.parent], diamond.condition.value);
    if(!compare || compare->kind != Instruction::IK_CMP) continue;
    Block & merge_block = function->blocks[diamond.merge];
    bool converted = false;
    for(std::size_t i = 0; i < merge_block.instructions.size(); ++i) {
      Instruction & phi = merge_block.instructions[i];
      if(phi.kind != Instruction::IK_PHI) break;
      Operand then_value, else_value, result;
      if(!phi_arm_inputs(phi, then_block.id, else_block.id,
                         &then_value, &else_value) ||
         !select_identity(*compare, then_value, else_value, &result))
        continue;
      phi.args.clear();
      if(result.kind == Operand::OP_TEMP) {
        phi.kind = Instruction::IK_COPY;
        phi.first = result;
      } else {
        phi.kind = Instruction::IK_CONST;
        result.literal_type = phi.type;
        phi.first = result;
      }
      converted = true;
      if(stats) ++stats->diamond_select_identities;
    }
    if(!converted) continue;
    // The converted phis are copies now; phis lead a block.
    std::stable_partition(
      merge_block.instructions.begin(), merge_block.instructions.end(),
      [](const Instruction & ins) { return ins.kind == Instruction::IK_PHI; });
    changed = true;
  }
  return changed;
}

bool value_number_diamonds(Function * function, Stats * stats)
{
  if(!function_has_phi(*function)) return false;
  const Graph graph = build_graph(*function, stats);
  const std::vector<Diamond> diamonds = collect_diamonds(*function, graph);
  if(diamonds.size() < 2) return false;
  const lowir_analysis::DominatorTree dom =
    lowir_analysis::dominators(graph, stats);
  const lowir_analysis::ValueIndex values(*function, stats);
  std::vector<std::pair<lowir_model::ValueId, Operand> > replacements;
  std::vector<std::size_t> condition_rewrites;
  for(std::size_t second = 0; second < diamonds.size(); ++second) {
    const Diamond & later = diamonds[second];
    bool condition_rewritten = false;
    for(std::size_t first = 0; first < diamonds.size(); ++first) {
      if(first == second) continue;
      const Diamond & earlier = diamonds[first];
      if(!dom.dominates(earlier.merge, later.parent)) continue;
      DiamondMatch match;
      match.function = function;
      match.values = &values;
      if(!operands_correspond(earlier.condition, later.condition, &match,
                              kDiamondMatchDepth))
        continue;
      bool condition_stable = true;
      for(std::size_t k = 0;
          k < match.earlier_load_blocks.size() && condition_stable; ++k)
        condition_stable = memory_stable_between(
          *function, graph, match.earlier_load_blocks[k], later.parent);
      if(!condition_stable) continue;
      // The later diamond branches on the earliest such condition itself;
      // its own recomputation is dead once nothing else reads it.
      if(!condition_rewritten &&
         !same_operand(earlier.condition, later.condition)) {
        function->blocks[later.parent].instructions.back().first =
          earlier.condition;
        condition_rewrites.push_back(later.parent);
        condition_rewritten = true;
      }
      const Block & earlier_then = function->blocks[earlier.then_arm];
      const Block & earlier_else = function->blocks[earlier.else_arm];
      const Block & later_then = function->blocks[later.then_arm];
      const Block & later_else = function->blocks[later.else_arm];
      // Each phi of the later merge whose arm inputs are the same
      // computations as an earlier phi's is that earlier phi, provided the
      // memory the earlier computations read still holds.
      const Block & earlier_merge = function->blocks[earlier.merge];
      const Block & later_merge = function->blocks[later.merge];
      std::size_t matched = 0;
      for(std::size_t i = 0; i < later_merge.instructions.size(); ++i) {
        const Instruction & phi = later_merge.instructions[i];
        if(phi.kind != Instruction::IK_PHI) break;
        Operand later_then_value, later_else_value;
        if(!phi_arm_inputs(phi, later_then.id, later_else.id,
                           &later_then_value, &later_else_value))
          continue;
        for(std::size_t j = 0; j < earlier_merge.instructions.size(); ++j) {
          const Instruction & candidate = earlier_merge.instructions[j];
          if(candidate.kind != Instruction::IK_PHI) break;
          Operand earlier_then_value, earlier_else_value;
          DiamondMatch phi_match = match;
          if(!(candidate.type == phi.type) ||
             !phi_arm_inputs(candidate, earlier_then.id, earlier_else.id,
                             &earlier_then_value, &earlier_else_value) ||
             !operands_correspond(earlier_then_value, later_then_value,
                                  &phi_match, kDiamondMatchDepth) ||
             !operands_correspond(earlier_else_value, later_else_value,
                                  &phi_match, kDiamondMatchDepth))
            continue;
          bool stable = true;
          for(std::size_t k = 0;
              k < phi_match.earlier_load_blocks.size() && stable; ++k)
            stable = memory_stable_between(
              *function, graph, phi_match.earlier_load_blocks[k],
              later.parent);
          if(!stable) continue;
          Operand replacement;
          replacement.kind = Operand::OP_TEMP;
          replacement.value = candidate.dest;
          replacement.literal_type = candidate.type;
          replacements.push_back(std::make_pair(phi.dest, replacement));
          ++matched;
          break;
        }
      }
      if(matched) break;
    }
  }
  if(stats) stats->diamond_condition_reuses += condition_rewrites.size();
  if(replacements.empty()) return !condition_rewrites.empty();
  std::vector<unsigned char> replaced(function->value_names.size(), 0);
  std::vector<Operand> replacement_of(function->value_names.size());
  for(std::size_t i = 0; i < replacements.size(); ++i) {
    replaced[replacements[i].first] = 1;
    replacement_of[replacements[i].first] = replacements[i].second;
  }
  // A replacement may itself name a replaced phi when three diamonds
  // agree; follow it to the surviving value.
  for(std::size_t i = 0; i < replacements.size(); ++i) {
    Operand & target = replacement_of[replacements[i].first];
    for(int depth = 0; depth < 8 && target.kind == Operand::OP_TEMP &&
        replaced[target.value]; ++depth)
      target = replacement_of[target.value];
  }
  const auto replace = [&](Operand * operand) {
    if(operand->kind == Operand::OP_TEMP &&
       static_cast<std::uint32_t>(operand->value) < replaced.size() &&
       replaced[operand->value])
      *operand = replacement_of[operand->value];
  };
  for(std::size_t b = 0; b < function->blocks.size(); ++b)
    for(std::size_t i = 0; i < function->blocks[b].instructions.size(); ++i) {
      Instruction & ins = function->blocks[b].instructions[i];
      if(ins.dest.valid() && replaced[ins.dest]) continue;
      replace(&ins.first);
      replace(&ins.second);
      replace(&ins.third);
      for(std::size_t j = 0; j < ins.args.size(); ++j) replace(&ins.args[j]);
    }
  if(stats) stats->diamond_value_numberings += replacements.size();
  return true;
}

}  // namespace lowir_opt
