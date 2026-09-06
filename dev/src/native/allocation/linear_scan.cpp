#include "native/allocation/planning_seam.h"
#include "native/driver/stats.h"

#include <algorithm>
#include <cstdint>
#include <cstdlib>
#include <cstdio>
#include <vector>

// A linear-scan allocator at the planning seam.  Implemented against
// planning_seam.h and the PA38 README; see doc/backend-review/newcomer-notes.md.
//
// Every plannable value becomes one interval [begin, end] over the
// function's instruction positions (a loop-carried phi begins at 0, any
// other value at its definition; the end is the last use, extended over
// every layout backedge or backward exception span that contains it).  The
// intervals are visited in order of their begin; each takes the first
// register of its pool whose previous occupant ended before it, or, when
// the pool is full, evicts the occupant that ends last if that occupant
// ends after the newcomer.  An evicted or unplaced value is simply left to
// the reactive walk, which is always correct.
namespace lowir_native {
namespace location_planning {

namespace {

enum { kRegisterCount = 16, kNoCandidate = -1 };

struct Candidate
{
  std::size_t begin;
  std::size_t end;
  std::uint32_t value;
  // Loop-carried phi: begins at 0, never evicted.
  bool phi;
  // The interval contains a call: only the callee-saved pool may host it.
  bool crosses_call;
  // Phis, call-crossing values and loop invariants try the callee-saved
  // pool first; everything else tries the caller pool first.
  bool prefer_callee;
  int reg;
};

bool plannable_type(const lowir_model::LowType & type)
{
  switch(type.kind) {
  case lowir_model::LTK_I1:
  case lowir_model::LTK_I8:
  case lowir_model::LTK_U8:
  case lowir_model::LTK_I16:
  case lowir_model::LTK_U16:
  case lowir_model::LTK_I32:
  case lowir_model::LTK_U32:
  case lowir_model::LTK_I64:
  case lowir_model::LTK_PTR:
    return true;
  default:
    return false;
  }
}

// True when a sorted position list has an entry inside [begin, end].
bool any_position_in(const std::vector<std::size_t> & positions,
                     std::size_t begin, std::size_t end)
{
  std::vector<std::size_t>::const_iterator it =
    std::lower_bound(positions.begin(), positions.end(), begin);
  return it != positions.end() && *it <= end;
}

bool candidate_order(const Candidate & left, const Candidate & right)
{
  if(left.begin != right.begin) return left.begin < right.begin;
  if(left.phi != right.phi) return left.phi;
  if(left.end != right.end) return left.end > right.end;
  return left.value < right.value;
}

class LinearScan
{
public:
  explicit LinearScan(const analysis::FunctionFacts & facts)
    : facts_(facts)
  {
    for(int i = 0; i < kRegisterCount; ++i) {
      occupant_[i] = kNoCandidate;
      occupant_end_[i] = 0;
    }
    callee_pool_.push_back(XR_RBX);
    callee_pool_.push_back(XR_R12);
    callee_pool_.push_back(XR_R13);
    callee_pool_.push_back(XR_R14);
    callee_pool_.push_back(XR_R15);
    caller_pool_.push_back(XR_R8);
    caller_pool_.push_back(XR_R9);
  }

  void run(std::vector<Candidate> & candidates)
  {
    for(std::size_t i = 0; i < candidates.size(); ++i) {
      const bool prefer_callee = candidates[i].prefer_callee;
      const std::vector<X64Register> & first =
        prefer_callee ? callee_pool_ : caller_pool_;
      const std::vector<X64Register> & second =
        prefer_callee ? caller_pool_ : callee_pool_;
      if(place(candidates, i, first)) continue;
      if(place(candidates, i, second)) continue;
      if(evict_and_place(candidates, i, first)) continue;
      evict_and_place(candidates, i, second);
    }
  }

private:
  bool register_allowed(const Candidate & candidate, X64Register reg) const
  {
    if(reg == XR_RBX && facts_.has_i128_atomic) return false;
    // The caller pool only over call-free intervals.
    if(!allocation::is_callee_saved(reg) && candidate.crosses_call)
      return false;
    if(static_cast<std::size_t>(reg) < facts_.clobber_positions.size() &&
       any_position_in(facts_.clobber_positions[reg],
                       candidate.begin, candidate.end))
      return false;
    if(candidate.value < facts_.live_across_clobbers.size() &&
       (facts_.live_across_clobbers[candidate.value] &
        analysis::register_mask(reg)) != 0)
      return false;
    return true;
  }

  bool place(std::vector<Candidate> & candidates, std::size_t index,
             const std::vector<X64Register> & pool)
  {
    Candidate & candidate = candidates[index];
    for(std::size_t p = 0; p < pool.size(); ++p) {
      X64Register reg = pool[p];
      if(!register_allowed(candidate, reg)) continue;
      if(occupant_[reg] != kNoCandidate &&
         occupant_end_[reg] >= candidate.begin) continue;
      assign(candidates, index, reg);
      return true;
    }
    return false;
  }

  bool evict_and_place(std::vector<Candidate> & candidates,
                       std::size_t index,
                       const std::vector<X64Register> & pool)
  {
    Candidate & candidate = candidates[index];
    if(candidate.phi) return false;
    int victim_reg = -1;
    std::size_t victim_end = candidate.end;
    for(std::size_t p = 0; p < pool.size(); ++p) {
      X64Register reg = pool[p];
      if(!register_allowed(candidate, reg)) continue;
      int holder = occupant_[reg];
      if(holder == kNoCandidate) continue;
      if(candidates[holder].phi) continue;
      if(occupant_end_[reg] > victim_end) {
        victim_end = occupant_end_[reg];
        victim_reg = reg;
      }
    }
    if(victim_reg < 0) return false;
    candidates[occupant_[victim_reg]].reg = -1;
    assign(candidates, index, static_cast<X64Register>(victim_reg));
    return true;
  }

  void assign(std::vector<Candidate> & candidates, std::size_t index,
              X64Register reg)
  {
    candidates[index].reg = reg;
    occupant_[reg] = static_cast<int>(index);
    occupant_end_[reg] = candidates[index].end;
  }

  const analysis::FunctionFacts & facts_;
  std::vector<X64Register> callee_pool_;
  std::vector<X64Register> caller_pool_;
  int occupant_[kRegisterCount];
  std::size_t occupant_end_[kRegisterCount];
};

}  // namespace

FunctionLocationTimeline plan_value_locations_linear_scan(
    const lowir_model::LowirFunction & function,
    const analysis::FunctionFacts & facts,
    const LayoutScan & layout,
    std::vector<std::pair<std::size_t, std::size_t> > * register_spans,
    Stats * stats)
{
  const std::size_t value_count = function.value_names.size();
  FunctionLocationTimeline timeline(value_count);
  const std::size_t missing = analysis::FunctionFacts::missing_position();

  // Which values are phis, from the instruction stream.
  std::vector<unsigned char> is_phi(value_count, 0);
  for(std::size_t b = 0; b < function.blocks.size(); ++b) {
    const lowir_model::Block & block = function.blocks[b];
    for(std::size_t i = 0; i < block.instructions.size(); ++i) {
      const lowir_model::Instruction & instruction = block.instructions[i];
      if(instruction.kind == lowir_model::Instruction::IK_PHI &&
         instruction.dest.valid() &&
         static_cast<std::size_t>(instruction.dest) < value_count)
        is_phi[instruction.dest] = 1;
    }
  }

  std::vector<std::size_t> calls(facts.calls);
  std::sort(calls.begin(), calls.end());

  std::vector<Candidate> candidates;
  for(std::size_t v = 0; v < value_count; ++v) {
    if(v >= facts.definition.size() || v >= facts.last_use.size() ||
       v >= facts.value_flags.size() || v >= function.value_types.size())
      continue;
    if(!plannable_type(function.value_types[v])) continue;
    const unsigned flags = facts.value_flags[v];
    if(flags & (analysis::FunctionFacts::VF_PARAMETER |
                analysis::FunctionFacts::VF_ONLY_STORAGE_ADDRESS))
      continue;
    const std::size_t definition = facts.definition[v];
    const std::size_t last_use = facts.last_use[v];
    if(definition == missing || last_use == missing) continue;

    Candidate candidate;
    candidate.value = static_cast<std::uint32_t>(v);
    candidate.phi = is_phi[v] != 0;
    candidate.reg = -1;
    if(candidate.phi) {
      // Merge phis are written by every predecessor before their own
      // position; only loop-carried phis are plannable, from position 0.
      if(v >= layout.phi_loop_carried.size() || !layout.phi_loop_carried[v])
        continue;
      candidate.begin = 0;
    } else {
      candidate.begin = definition;
    }
    candidate.end = last_use;
    if(candidate.phi || (flags & analysis::FunctionFacts::VF_EDGE_LIVE)) {
      bool extended = true;
      while(extended) {
        extended = false;
        for(std::size_t s = 0; s < layout.spans.size(); ++s) {
          const std::pair<std::size_t, std::size_t> & span = layout.spans[s];
          if(span.first <= candidate.end && candidate.end < span.second) {
            candidate.end = span.second;
            extended = true;
          }
        }
      }
    }
    if(candidate.end < candidate.begin) continue;
    candidate.crosses_call =
      any_position_in(calls, candidate.begin, candidate.end);
    candidate.prefer_callee = candidate.crosses_call || candidate.phi ||
      (flags & analysis::FunctionFacts::VF_LOOP_INVARIANT) != 0;
    candidates.push_back(candidate);
  }

  std::sort(candidates.begin(), candidates.end(), candidate_order);

  LinearScan scan(facts);
  scan.run(candidates);

  for(std::size_t i = 0; i < candidates.size(); ++i) {
    const Candidate & candidate = candidates[i];
    if(candidate.reg < 0) continue;
    timeline[candidate.value].push_back(PlannedLocationSegment(
      candidate.begin, candidate.end, PLK_GPR,
      static_cast<unsigned>(candidate.reg)));
    if(register_spans && allocation::is_callee_saved(
         static_cast<X64Register>(candidate.reg)))
      register_spans[candidate.reg].push_back(
        std::make_pair(candidate.begin, candidate.end));
    if(stats) {
      ++stats->planned_value_registers;
      if(candidate.phi) ++stats->planned_phi_registers;
    }
  }
  if(register_spans)
    for(int r = 0; r < kRegisterCount; ++r)
      std::sort(register_spans[r].begin(), register_spans[r].end());
  return timeline;
}

}  // namespace location_planning
}  // namespace lowir_native
