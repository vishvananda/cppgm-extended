#include "lowir/optimize/loop_simplify.h"

#include "lowir/optimize/pipeline.h"
#include "lowir/model/identity.h"

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <string>
#include <utility>
#include <vector>

namespace lowir_opt {
namespace {

using lowir_model::Function;
using lowir_model::Instruction;
using lowir_model::LowOperation;
using lowir_model::LowType;
using lowir_model::Operand;

const std::size_t kNoIndex = static_cast<std::size_t>(-1);

struct Induction
{
  std::size_t phi_instruction;
  std::size_t update_block;
  std::size_t update_instruction;
  lowir_model::ValueId value;
  long long initial;
  long long step;
};

bool pure_loop_instruction(const Instruction & instruction)
{
  const Instruction::Kind kind = instruction.kind;
  if(kind == Instruction::IK_BINARY &&
     (instruction.op.kind == LowOperation::LOP_DIV ||
      instruction.op.kind == LowOperation::LOP_UDIV ||
      instruction.op.kind == LowOperation::LOP_MOD ||
      instruction.op.kind == LowOperation::LOP_UMOD)) return false;
  return kind == Instruction::IK_CONST || kind == Instruction::IK_COPY ||
    kind == Instruction::IK_PHI || kind == Instruction::IK_ADDR ||
    kind == Instruction::IK_INDEX || kind == Instruction::IK_UNARY ||
    kind == Instruction::IK_BINARY || kind == Instruction::IK_CMP ||
    kind == Instruction::IK_CONVERT || kind == Instruction::IK_JUMP ||
    kind == Instruction::IK_BRANCH;
}

bool find_induction(Function * function,
                    const lowir_analysis::NaturalLoop & loop,
                    const lowir_analysis::ValueIndex & values,
                    lowir_model::ValueId value, Induction * result)
{
  const lowir_analysis::ValueDefinition phi_definition =
    values.definition(value);
  if(phi_definition.kind != lowir_analysis::ValueDefinition::INSTRUCTION ||
     phi_definition.block != loop.header) return false;
  const std::size_t phi_index = phi_definition.instruction;
  Instruction & phi =
    function->blocks[loop.header].instructions[phi_index];
  if(phi.kind != Instruction::IK_PHI ||
     phi.type.kind != lowir_model::LTK_I64 || loop.latches.size() != 1)
    return false;
  Operand initial;
  Operand updated;
  bool has_initial = false;
  bool has_updated = false;
  const lowir_model::BlockId preheader_id =
    function->blocks[loop.preheader].id;
  const lowir_model::BlockId latch_id =
    function->blocks[loop.latches[0]].id;
  for(std::size_t incoming = 0;
      incoming + 1 < phi.args.size(); incoming += 2) {
    if(phi.args[incoming].block == preheader_id) {
      initial = phi.args[incoming + 1];
      has_initial = true;
    } else if(phi.args[incoming].block == latch_id) {
      updated = phi.args[incoming + 1];
      has_updated = true;
    }
  }
  if(!has_initial || !initial.has_int_value ||
     initial.kind != Operand::OP_INTEGER || !has_updated ||
     updated.kind != Operand::OP_TEMP) return false;
  if(initial.int_high !=
     (initial.int_value < 0 ? ~UINT64_C(0) : UINT64_C(0))) return false;
  const lowir_analysis::ValueDefinition update_definition =
    values.definition(updated.value);
  if(update_definition.kind !=
       lowir_analysis::ValueDefinition::INSTRUCTION ||
     update_definition.block != loop.latches[0]) return false;
  Instruction * update = &function->blocks[update_definition.block]
    .instructions[update_definition.instruction];
  if(update->kind != Instruction::IK_BINARY ||
     update->type.kind != lowir_model::LTK_I64) return false;
  long long step = 0;
  if(update->op.kind == LowOperation::LOP_ADD &&
     update->first.kind == Operand::OP_TEMP &&
     update->first.value == value &&
     update->second.kind == Operand::OP_INTEGER &&
     update->second.has_int_value &&
     update->second.int_high ==
       (update->second.int_value < 0 ? ~UINT64_C(0) : UINT64_C(0)))
    step = update->second.int_value;
  else if(update->op.kind == LowOperation::LOP_SUB &&
          update->first.kind == Operand::OP_TEMP &&
          update->first.value == value &&
          update->second.kind == Operand::OP_INTEGER &&
          update->second.has_int_value &&
          update->second.int_high ==
            (update->second.int_value < 0 ? ~UINT64_C(0) : UINT64_C(0)) &&
          update->second.int_value != std::numeric_limits<long long>::min())
    step = -update->second.int_value;
  else return false;
  if(step == 0) return false;
  result->phi_instruction = phi_index;
  result->update_block = update_definition.block;
  result->update_instruction = update_definition.instruction;
  result->value = value;
  result->initial = initial.int_value;
  result->step = step;
  return true;
}

LowOperation::Kind negate_compare(LowOperation::Kind kind)
{
  switch(kind) {
  case LowOperation::LOP_EQ: return LowOperation::LOP_NE;
  case LowOperation::LOP_NE: return LowOperation::LOP_EQ;
  case LowOperation::LOP_LT: return LowOperation::LOP_GE;
  case LowOperation::LOP_LE: return LowOperation::LOP_GT;
  case LowOperation::LOP_GT: return LowOperation::LOP_LE;
  case LowOperation::LOP_GE: return LowOperation::LOP_LT;
  case LowOperation::LOP_ULT: return LowOperation::LOP_UGE;
  case LowOperation::LOP_ULE: return LowOperation::LOP_UGT;
  case LowOperation::LOP_UGT: return LowOperation::LOP_ULE;
  case LowOperation::LOP_UGE: return LowOperation::LOP_ULT;
  default: return LowOperation::LOP_NONE;
  }
}

bool proves_termination(long long initial, long long step, long long bound,
                        LowOperation::Kind condition)
{
  typedef __int128 Wide;
  const Wide first = initial;
  const Wide increment = step;
  const Wide limit = bound;
  Wide trips = 0;
  if(condition == LowOperation::LOP_LT && increment > 0) {
    if(first >= limit) return true;
    trips = (limit - first + increment - 1) / increment;
  } else if(condition == LowOperation::LOP_LE && increment > 0) {
    if(first > limit) return true;
    trips = (limit - first) / increment + 1;
  } else if(condition == LowOperation::LOP_GT && increment < 0) {
    if(first <= limit) return true;
    const Wide positive = -increment;
    trips = (first - limit + positive - 1) / positive;
  } else if(condition == LowOperation::LOP_GE && increment < 0) {
    if(first < limit) return true;
    trips = (first - limit) / (-increment) + 1;
  } else if(condition == LowOperation::LOP_NE) {
    const Wide difference = limit - first;
    if(difference == 0) return true;
    if((difference > 0) != (increment > 0) ||
       difference % increment != 0) return false;
    trips = difference / increment;
  } else return false;
  if(trips < 0 || trips > static_cast<Wide>(UINT64_C(1000000000)))
    return false;
  const Wide final_value = first + trips * increment;
  return final_value >= std::numeric_limits<long long>::min() &&
    final_value <= std::numeric_limits<long long>::max();
}

bool power_of_two(long long value, unsigned * shift)
{
  if(value <= 1) return false;
  const std::uint64_t bits = static_cast<std::uint64_t>(value);
  if((bits & (bits - 1)) != 0) return false;
  unsigned result = 0;
  for(std::uint64_t cursor = bits; cursor > 1; cursor >>= 1) ++result;
  *shift = result;
  return true;
}

bool has_outside_value_use(const Function & function,
                           const lowir_analysis::NaturalLoop & loop)
{
  std::vector<unsigned char> defined(function.value_names.size(), 0);
  for(std::size_t member = 0; member < loop.blocks.size(); ++member) {
    const std::vector<Instruction> & instructions =
      function.blocks[loop.blocks[member]].instructions;
    for(std::size_t i = 0; i < instructions.size(); ++i)
      if(instructions[i].dest.valid()) defined[instructions[i].dest] = 1;
  }
  const auto outside_use = [&defined](const Operand & operand) {
    return operand.kind == Operand::OP_TEMP && defined[operand.value];
  };
  for(std::size_t block = 0; block < function.blocks.size(); ++block) {
    if(loop.contains(block)) continue;
    for(std::size_t i = 0; i < function.blocks[block].instructions.size(); ++i) {
      const Instruction & ins = function.blocks[block].instructions[i];
      if(outside_use(ins.first) || outside_use(ins.second) ||
         outside_use(ins.third)) return true;
      for(std::size_t arg = 0; arg < ins.args.size(); ++arg)
        if(outside_use(ins.args[arg])) return true;
    }
  }
  return false;
}

void erase_loop_blocks(Function * function,
                       const lowir_analysis::NaturalLoop & loop)
{
  std::vector<unsigned char> erase(function->blocks.size(), 0);
  for(std::size_t i = 0; i < loop.blocks.size(); ++i)
    erase[loop.blocks[i]] = 1;
  std::size_t kept = 0;
  for(std::size_t block = 0; block < function->blocks.size(); ++block)
    if(!erase[block]) {
      if(kept != block)
        function->blocks[kept] = std::move(function->blocks[block]);
      ++kept;
    }
  function->blocks.resize(kept);
}


// An instruction whose removal changes nothing the program can observe: the
// pure loop set plus a non-volatile load.
bool effect_free_instruction(const Instruction & instruction)
{
  if(instruction.kind == Instruction::IK_LOAD)
    return !instruction.volatile_access;
  return pure_loop_instruction(instruction);
}

// Rewire the exit block's phis for the loss of every loop predecessor: the
// one incoming edge that remains is from the preheader.  Every loop-defined
// value is already known to be unused outside, so the surviving operands are
// invariant; when several loop blocks fed the same phi they must have fed the
// same operand.
bool retarget_exit_phis(Function * function,
                        const lowir_analysis::NaturalLoop & loop,
                        std::size_t exit)
{
  const lowir_model::BlockId preheader_id =
    function->blocks[loop.preheader].id;
  std::vector<unsigned char> loop_block_ids;
  for(std::size_t i = 0; i < function->blocks.size(); ++i) {
    const lowir_model::BlockId id = function->blocks[i].id;
    if(id >= loop_block_ids.size()) loop_block_ids.resize(id + 1, 0);
    if(loop.contains(i)) loop_block_ids[id] = 1;
  }
  std::vector<Instruction> & instructions = function->blocks[exit].instructions;
  for(std::size_t i = 0; i < instructions.size(); ++i) {
    Instruction & phi = instructions[i];
    if(phi.kind != Instruction::IK_PHI) break;
    bool have_survivor = false;
    Operand survivor;
    for(std::size_t arg = 0; arg + 1 < phi.args.size(); arg += 2) {
      const lowir_model::BlockId from = phi.args[arg].block;
      if(from >= loop_block_ids.size() || !loop_block_ids[from]) continue;
      if(!have_survivor) { survivor = phi.args[arg + 1]; have_survivor = true; }
      else if(survivor.kind != phi.args[arg + 1].kind ||
              (survivor.kind == Operand::OP_TEMP &&
               survivor.value != phi.args[arg + 1].value) ||
              (survivor.kind == Operand::OP_INTEGER &&
               (survivor.int_value != phi.args[arg + 1].int_value ||
                survivor.int_high != phi.args[arg + 1].int_high)) ||
              (survivor.kind != Operand::OP_TEMP &&
               survivor.kind != Operand::OP_INTEGER))
        return false;
    }
  }
  for(std::size_t i = 0; i < instructions.size(); ++i) {
    Instruction & phi = instructions[i];
    if(phi.kind != Instruction::IK_PHI) break;
    std::vector<Operand> kept;
    bool placed = false;
    for(std::size_t arg = 0; arg + 1 < phi.args.size(); arg += 2) {
      const lowir_model::BlockId from = phi.args[arg].block;
      const bool from_loop = from < loop_block_ids.size() && loop_block_ids[from];
      if(!from_loop) {
        kept.push_back(phi.args[arg]);
        kept.push_back(phi.args[arg + 1]);
      } else if(!placed) {
        Operand label = phi.args[arg];
        label.block = preheader_id;
        kept.push_back(label);
        kept.push_back(phi.args[arg + 1]);
        placed = true;
      }
    }
    phi.args.swap(kept);
  }
  return true;
}

// Two header phis with the same incoming operands are the same value.
bool twin_header_phi(const Function & function,
                     const lowir_analysis::NaturalLoop & loop,
                     const lowir_analysis::ValueIndex & values,
                     lowir_model::ValueId first, lowir_model::ValueId second)
{
  const lowir_analysis::ValueDefinition a = values.definition(first);
  const lowir_analysis::ValueDefinition b = values.definition(second);
  if(a.kind != lowir_analysis::ValueDefinition::INSTRUCTION ||
     b.kind != lowir_analysis::ValueDefinition::INSTRUCTION ||
     a.block != loop.header || b.block != loop.header) return false;
  const Instruction & x = function.blocks[a.block].instructions[a.instruction];
  const Instruction & y = function.blocks[b.block].instructions[b.instruction];
  if(x.kind != Instruction::IK_PHI || y.kind != Instruction::IK_PHI ||
     x.args.size() != y.args.size()) return false;
  for(std::size_t i = 0; i < x.args.size(); ++i) {
    const Operand & p = x.args[i];
    const Operand & q = y.args[i];
    if(p.kind != q.kind) return false;
    if(p.kind == Operand::OP_LABEL && p.block != q.block) return false;
    if(p.kind == Operand::OP_TEMP && p.value != q.value) return false;
    if(p.kind == Operand::OP_INTEGER &&
       (p.int_value != q.int_value || p.int_high != q.int_high)) return false;
    if(p.kind != Operand::OP_LABEL && p.kind != Operand::OP_TEMP &&
       p.kind != Operand::OP_INTEGER) return false;
  }
  return true;
}

// Every edge that leaves the loop is taken on an equality test between a
// pointer the loop walks by a constant stride and a pointer from outside it.
// N3485 5.7/5 makes pointer arithmetic that leaves an object undefined, so a
// walk that compares for equality with another pointer reaches it or is not
// a program at all; that, not the forward-progress rule, is the licence to
// delete the walk.  An integer count with an unproven bound, or a loop that
// spins on a phi, is left alone.
bool exits_on_pointer_walk(const Function & function,
                           const lowir_analysis::NaturalLoop & loop,
                           const lowir_analysis::ValueIndex & values,
                           std::size_t exit)
{
  const lowir_model::BlockId exit_id = function.blocks[exit].id;
  const lowir_model::BlockId preheader_id =
    function.blocks[loop.preheader].id;
  const auto defined_in_loop = [&](const Operand & operand) {
    if(operand.kind != Operand::OP_TEMP) return false;
    const lowir_analysis::ValueDefinition definition =
      values.definition(operand.value);
    return definition.kind == lowir_analysis::ValueDefinition::INSTRUCTION &&
      loop.contains(definition.block);
  };
  const auto walks = [&](const Operand & operand) {
    if(operand.kind != Operand::OP_TEMP) return false;
    const lowir_analysis::ValueDefinition definition =
      values.definition(operand.value);
    if(definition.kind != lowir_analysis::ValueDefinition::INSTRUCTION ||
       definition.block != loop.header) return false;
    const Instruction & phi =
      function.blocks[definition.block].instructions[definition.instruction];
    if(phi.kind != Instruction::IK_PHI || phi.type.kind != lowir_model::LTK_PTR ||
       phi.args.size() != 4) return false;
    for(std::size_t incoming = 0; incoming + 1 < phi.args.size(); incoming += 2) {
      if(phi.args[incoming].block == preheader_id) continue;
      const Operand & step = phi.args[incoming + 1];
      if(step.kind != Operand::OP_TEMP) return false;
      const lowir_analysis::ValueDefinition step_definition =
        values.definition(step.value);
      if(step_definition.kind != lowir_analysis::ValueDefinition::INSTRUCTION ||
         !loop.contains(step_definition.block)) return false;
      const Instruction & advance = function.blocks[step_definition.block]
        .instructions[step_definition.instruction];
      if(advance.kind != Instruction::IK_INDEX ||
         advance.first.kind != Operand::OP_TEMP ||
         advance.second.kind != Operand::OP_INTEGER ||
         !advance.second.has_int_value || advance.second.int_value == 0)
        return false;
      // The step may advance a twin of this phi -- a header phi with the
      // same incoming operands, which promotion of a field that mirrored
      // the pointer leaves behind (libc++'s split_buffer destroys its old
      // elements walking one twin down and comparing the other).
      if(advance.first.value != operand.value &&
         !twin_header_phi(function, loop, values, operand.value,
                          advance.first.value))
        return false;
    }
    return true;
  };
  bool saw_exit = false;
  for(std::size_t member = 0; member < loop.blocks.size(); ++member) {
    const std::vector<Instruction> & instructions =
      function.blocks[loop.blocks[member]].instructions;
    if(instructions.empty()) return false;
    const Instruction & terminator = instructions.back();
    bool leaves = false;
    if(terminator.kind == Instruction::IK_BRANCH)
      leaves = terminator.second.block == exit_id ||
        terminator.third.block == exit_id;
    else if(terminator.kind == Instruction::IK_JUMP)
      leaves = terminator.first.block == exit_id;
    else if(terminator.kind == Instruction::IK_SWITCH)
      return false;
    if(!leaves) continue;
    saw_exit = true;
    if(terminator.kind != Instruction::IK_BRANCH ||
       terminator.first.kind != Operand::OP_TEMP) return false;
    const lowir_analysis::ValueDefinition condition =
      values.definition(terminator.first.value);
    if(condition.kind != lowir_analysis::ValueDefinition::INSTRUCTION ||
       !loop.contains(condition.block)) return false;
    const Instruction & compare =
      function.blocks[condition.block].instructions[condition.instruction];
    if(compare.kind != Instruction::IK_CMP ||
       compare.type.kind != lowir_model::LTK_PTR ||
       (compare.op.kind != LowOperation::LOP_EQ &&
        compare.op.kind != LowOperation::LOP_NE)) return false;
    const bool first_walks = walks(compare.first);
    const bool second_walks = walks(compare.second);
    if(first_walks == second_walks) return false;
    const Operand & bound = first_walks ? compare.second : compare.first;
    if(defined_in_loop(bound)) return false;
  }
  return saw_exit;
}

// The loop shape libc++ leaves behind for vector(n, value) and its kin once
// the allocator layers are inlined: a header that is exactly a pointer phi,
// an equality test against an invariant end, and a branch; a latch that
// stores one fixed-width value through the pointer, advances it by that
// width, and jumps back.  The end is `start + n` from an index instruction,
// which is how a sized construction spells it; a first/last pair is not
// taken yet.
struct FillLoop
{
  std::size_t header;
  std::size_t latch;
  std::size_t exit;
  // The walked pointer's phi and any phi with identical incoming values:
  // promotion of a field that mirrored the pointer leaves such a twin.
  std::vector<lowir_model::ValueId> pointers;
  Operand start;
  Operand bound;
  Operand length;
  Operand value;
  Operand load_address;
  bool value_loaded;
  // A value wider than a byte reloaded each iteration: the fill is exact
  // only when the source lies outside the range or on an element boundary
  // inside it, which a runtime test in the preheader decides, keeping the
  // loop as the other arm.
  bool alias_checked;
  LowType load_type;
  std::size_t width;
};

bool invariant_operand(const Operand & operand,
                       const lowir_analysis::NaturalLoop & loop,
                       const lowir_analysis::ValueIndex & values)
{
  if(operand.kind == Operand::OP_INTEGER) return operand.has_int_value;
  if(operand.kind == Operand::OP_SLOT || operand.kind == Operand::OP_GLOBAL)
    return true;
  if(operand.kind != Operand::OP_TEMP) return false;
  const lowir_analysis::ValueDefinition definition =
    values.definition(operand.value);
  if(definition.kind == lowir_analysis::ValueDefinition::PARAMETER)
    return true;
  return definition.kind == lowir_analysis::ValueDefinition::INSTRUCTION &&
    !loop.contains(definition.block);
}

bool same_temp(const Operand & a, const Operand & b)
{
  return a.kind == Operand::OP_TEMP && b.kind == Operand::OP_TEMP &&
    a.value == b.value;
}

bool is_pointer_id(const std::vector<lowir_model::ValueId> & pointers,
                   lowir_model::ValueId value)
{
  for(std::size_t i = 0; i < pointers.size(); ++i)
    if(pointers[i] == value) return true;
  return false;
}

std::size_t store_width(const LowType & type)
{
  switch(type.kind) {
  case lowir_model::LTK_I8: case lowir_model::LTK_U8: return 1;
  case lowir_model::LTK_I16: case lowir_model::LTK_U16: return 2;
  case lowir_model::LTK_I32: case lowir_model::LTK_U32: return 4;
  case lowir_model::LTK_I64: return 8;
  default: return 0;
  }
}

// The byte the fill writes, when every byte of the stored constant is the
// same one.
// A fill the backend can do in one `rep stos`: bytes, or 2-, 4- and 8-byte
// units for a value that is not a byte splat -- `vector<unsigned>(n, v)`,
// `resize(n, v)`, `vector<bool>`'s word fills.
bool unit_fill_width(std::size_t width)
{
  return width == 1 || width == 2 || width == 4 || width == 8;
}

// Below this many units the loop stays: `rep stos` starts in tens of cycles.
const long long kUnitFillMinimumCount = 16;

bool splat_byte(const Operand & constant, std::size_t width, long long * byte)
{
  if(constant.kind != Operand::OP_INTEGER || !constant.has_int_value)
    return false;
  const std::uint64_t bits = static_cast<std::uint64_t>(constant.int_value);
  const std::uint64_t low = bits & 0xff;
  for(std::size_t i = 1; i < width; ++i)
    if(((bits >> (8 * i)) & 0xff) != low) return false;
  *byte = static_cast<long long>(low);
  return true;
}

bool match_fill_loop(const Function & function,
                     const lowir_analysis::NaturalLoop & loop,
                     const lowir_analysis::ValueIndex & values,
                     const lowir_analysis::Graph & graph,
                     FillLoop * out)
{
  if(loop.preheader == kNoIndex || loop.has_eh || loop.latches.size() != 1 ||
     loop.blocks.size() != 2 || loop.exits.size() != 1) return false;
  const std::size_t latch = loop.latches[0];
  const std::size_t exit = loop.exits[0];
  if(latch == loop.header || loop.contains(exit)) return false;
  const lowir_model::BlockId preheader_id = function.blocks[loop.preheader].id;
  const lowir_model::BlockId latch_id = function.blocks[latch].id;
  const lowir_model::BlockId header_id = function.blocks[loop.header].id;
  const lowir_model::BlockId exit_id = function.blocks[exit].id;

  const std::vector<Instruction> & header =
    function.blocks[loop.header].instructions;
  std::size_t phi_count = 0;
  while(phi_count < header.size() &&
        header[phi_count].kind == Instruction::IK_PHI) ++phi_count;
  if(phi_count == 0 || header.size() != phi_count + 2 ||
     header[phi_count].kind != Instruction::IK_CMP ||
     header[phi_count + 1].kind != Instruction::IK_BRANCH) return false;
  const Instruction & compare = header[phi_count];
  const Instruction & branch = header[phi_count + 1];
  if(compare.type.kind != lowir_model::LTK_PTR ||
     (compare.op.kind != LowOperation::LOP_NE &&
      compare.op.kind != LowOperation::LOP_EQ) ||
     branch.first.kind != Operand::OP_TEMP ||
     branch.first.value != compare.dest) return false;
  // The walked pointer is the phi the comparison reads; every other phi
  // must be its twin, with the same incoming values from the same blocks.
  std::size_t phi_index = phi_count;
  for(std::size_t i = 0; i < phi_count; ++i)
    if((compare.first.kind == Operand::OP_TEMP &&
        compare.first.value == header[i].dest) ||
       (compare.second.kind == Operand::OP_TEMP &&
        compare.second.value == header[i].dest)) { phi_index = i; break; }
  if(phi_index == phi_count) return false;
  const Instruction & phi = header[phi_index];
  if(phi.type.kind != lowir_model::LTK_PTR || phi.args.size() != 4) return false;
  std::vector<lowir_model::ValueId> pointers;
  pointers.push_back(phi.dest);
  for(std::size_t i = 0; i < phi_count; ++i) {
    if(i == phi_index) continue;
    const Instruction & twin = header[i];
    if(twin.type.kind != lowir_model::LTK_PTR ||
       twin.args.size() != phi.args.size()) return false;
    for(std::size_t a = 0; a < phi.args.size(); ++a) {
      const Operand & mine = phi.args[a];
      const Operand & theirs = twin.args[a];
      if(mine.kind != theirs.kind) return false;
      if(mine.kind == Operand::OP_LABEL && mine.block != theirs.block) return false;
      if(mine.kind == Operand::OP_TEMP && mine.value != theirs.value) return false;
      if(mine.kind == Operand::OP_INTEGER &&
         (mine.int_value != theirs.int_value || mine.int_high != theirs.int_high))
        return false;
      if(mine.kind != Operand::OP_LABEL && mine.kind != Operand::OP_TEMP &&
         mine.kind != Operand::OP_INTEGER) return false;
    }
    pointers.push_back(twin.dest);
  }
  Operand start;
  Operand next;
  bool have_start = false;
  bool have_next = false;
  for(std::size_t incoming = 0; incoming + 1 < phi.args.size(); incoming += 2) {
    if(phi.args[incoming].block == preheader_id) {
      start = phi.args[incoming + 1]; have_start = true;
    } else if(phi.args[incoming].block == latch_id) {
      next = phi.args[incoming + 1]; have_next = true;
    }
  }
  if(!have_start || !have_next || next.kind != Operand::OP_TEMP ||
     !invariant_operand(start, loop, values)) return false;
  const bool first_is_phi = compare.first.kind == Operand::OP_TEMP &&
    is_pointer_id(pointers, compare.first.value);
  const bool second_is_phi = compare.second.kind == Operand::OP_TEMP &&
    is_pointer_id(pointers, compare.second.value);
  if(first_is_phi == second_is_phi) return false;
  const Operand & bound = first_is_phi ? compare.second : compare.first;
  if(!invariant_operand(bound, loop, values)) return false;
  const lowir_model::BlockId continue_target =
    compare.op.kind == LowOperation::LOP_NE ? branch.second.block : branch.third.block;
  const lowir_model::BlockId leave_target =
    compare.op.kind == LowOperation::LOP_NE ? branch.third.block : branch.second.block;
  if(continue_target != latch_id || leave_target != exit_id) return false;
  (void)graph;

  // The bound is start + n from an index instruction over the same start.
  if(bound.kind != Operand::OP_TEMP) return false;
  const lowir_analysis::ValueDefinition bound_definition =
    values.definition(bound.value);
  if(bound_definition.kind != lowir_analysis::ValueDefinition::INSTRUCTION)
    return false;
  const Instruction & bound_index = function.blocks[bound_definition.block]
    .instructions[bound_definition.instruction];
  if(bound_index.kind != Instruction::IK_INDEX ||
     bound_index.index_projection != lowir_model::IPK_NONE ||
     !same_temp(bound_index.first, start) ||
     bound_index.type.kind != lowir_model::LTK_I8) return false;
  const Operand length = bound_index.second;
  if(!invariant_operand(length, loop, values)) return false;

  // The latch: [load] [copy] store advance jump, each at most once.  A copy
  // of an invariant operand may also sit in the body -- invariant motion has
  // not run yet at this point -- and names an alias of that operand.
  const std::vector<Instruction> & body = function.blocks[latch].instructions;
  const Instruction * load = 0;
  const Instruction * copy = 0;
  const Instruction * store = 0;
  const Instruction * advance = 0;
  std::vector<std::pair<lowir_model::ValueId, Operand> > aliases;
  const auto alias_source = [&aliases](const Operand & operand,
                                       Operand * source) {
    if(operand.kind != Operand::OP_TEMP) return false;
    for(std::size_t i = 0; i < aliases.size(); ++i)
      if(aliases[i].first == operand.value) { *source = aliases[i].second; return true; }
    return false;
  };
  for(std::size_t i = 0; i < body.size(); ++i) {
    const Instruction & ins = body[i];
    if(i + 1 == body.size()) {
      if(ins.kind != Instruction::IK_JUMP || ins.first.block != header_id)
        return false;
      continue;
    }
    if(ins.kind == Instruction::IK_LOAD && !load && !store &&
       !ins.volatile_access) load = &ins;
    else if(ins.kind == Instruction::IK_COPY && !copy && !store &&
            ins.type.kind == lowir_model::LTK_PTR &&
            ins.first.kind == Operand::OP_TEMP &&
            is_pointer_id(pointers, ins.first.value))
      copy = &ins;
    else if(ins.kind == Instruction::IK_COPY && !store &&
            ins.type.kind == lowir_model::LTK_PTR &&
            invariant_operand(ins.first, loop, values))
      aliases.push_back(std::make_pair(ins.dest, ins.first));
    else if(ins.kind == Instruction::IK_STORE && !store &&
            !ins.volatile_access) store = &ins;
    else if(ins.kind == Instruction::IK_INDEX && !advance && store &&
            ins.index_projection == lowir_model::IPK_NONE &&
            ins.type.kind == lowir_model::LTK_I8) advance = &ins;
    else return false;
  }
  if(!store || !advance) return false;
  const std::size_t width = store_width(store->type);
  if(width == 0) return false;
  const Operand & address = store->second;
  const bool through_phi =
    address.kind == Operand::OP_TEMP && is_pointer_id(pointers, address.value);
  const bool through_copy =
    copy && address.kind == Operand::OP_TEMP && address.value == copy->dest;
  if(!through_phi && !through_copy) return false;
  if(advance->first.kind != Operand::OP_TEMP ||
     !is_pointer_id(pointers, advance->first.value) ||
     advance->second.kind != Operand::OP_INTEGER ||
     !advance->second.has_int_value || advance->second.int_high != 0 ||
     advance->second.int_value != static_cast<long long>(width) ||
     advance->dest != next.value) return false;

  const Operand & stored = store->first;
  out->value_loaded = false;
  out->alias_checked = false;
  if(load) {
    // A value reloaded through an invariant address each iteration.  For a
    // one-byte store the reload can be lifted out even if the address is
    // inside the range: the one write that could reach it writes the byte it
    // read.  A wider store overlapping its source partially would change
    // the bytes later iterations read, so a wider fill is guarded by a
    // runtime test (see recognize_fill_loops); libc++'s
    // `vector<unsigned>(n, const unsigned&)` is this shape.
    Operand load_address = load->first;
    if(!invariant_operand(load_address, loop, values) &&
       !alias_source(load->first, &load_address)) return false;
    if(stored.kind != Operand::OP_TEMP || stored.value != load->dest ||
       store_width(load->type) != width) return false;
    if(width != 1) {
      if(!unit_fill_width(width)) return false;
      out->alias_checked = true;
    }
    out->value_loaded = true;
    out->load_address = load_address;
    out->load_type = load->type;
  } else if(!invariant_operand(stored, loop, values)) return false;
  else if(stored.kind == Operand::OP_TEMP) {
    if(!unit_fill_width(width)) return false;
  } else if(stored.kind == Operand::OP_INTEGER) {
    long long byte = 0;
    if(!splat_byte(stored, width, &byte) && !unit_fill_width(width))
      return false;
  } else return false;

  out->header = loop.header;
  out->latch = latch;
  out->exit = exit;
  out->pointers = pointers;
  out->start = start;
  out->bound = bound;
  out->length = length;
  out->value = stored;
  out->width = width;
  return true;
}

// The loop leaves only when the walked pointer equals the bound, so a use of
// the pointer after the loop is a use of the bound.  Any other loop-defined
// value used outside keeps the loop.  With apply false this only checks.
bool rewrite_outside_pointer_uses(Function * function,
                                  const lowir_analysis::NaturalLoop & loop,
                                  const std::vector<lowir_model::ValueId> & pointers,
                                  const Operand & bound, bool apply)
{
  std::vector<unsigned char> defined(function->value_names.size(), 0);
  for(std::size_t member = 0; member < loop.blocks.size(); ++member) {
    const std::vector<Instruction> & instructions =
      function->blocks[loop.blocks[member]].instructions;
    for(std::size_t i = 0; i < instructions.size(); ++i)
      if(instructions[i].dest.valid()) defined[instructions[i].dest] = 1;
  }
  const auto visit = [&](Operand & operand) {
    if(operand.kind != Operand::OP_TEMP || !defined[operand.value]) return true;
    if(!is_pointer_id(pointers, operand.value)) return false;
    if(apply) operand = bound;
    return true;
  };
  for(std::size_t block = 0; block < function->blocks.size(); ++block) {
    if(loop.contains(block)) continue;
    std::vector<Instruction> & instructions = function->blocks[block].instructions;
    for(std::size_t i = 0; i < instructions.size(); ++i) {
      Instruction & ins = instructions[i];
      if(!visit(ins.first) || !visit(ins.second) || !visit(ins.third))
        return false;
      for(std::size_t arg = 0; arg < ins.args.size(); ++arg)
        if(!visit(ins.args[arg])) return false;
    }
  }
  return true;
}

// The fill builtin the native backend lowers inline (see
// native/lowering/memcpy.h, fill_detail): declared once per program with the
// object symbol the backend matches, so the call never reaches a linker.
lowir_model::SymbolId fill_symbol(lowir_model::LowirProgram * program)
{
  const std::string object_name = "cppgm_builtin_fill_bytes";
  for(std::size_t i = 0; i < program->function_declarations.size(); ++i) {
    const lowir_model::FunctionDeclaration & declaration =
      program->function_declarations[i];
    if(declaration.metadata.object_symbol.valid() &&
       program->strings.get(declaration.metadata.object_symbol) == object_name)
      return declaration.symbol;
  }
  const lowir_model::SymbolId symbol =
    lowir_model::append_lowir_symbol(*program, "__builtin_fill_bytes");
  lowir_model::FunctionDeclaration declaration;
  declaration.symbol = symbol;
  const lowir_model::LowTypeKind param_kinds[3] = {
    lowir_model::LTK_PTR, lowir_model::LTK_I32, lowir_model::LTK_I64 };
  static const char * const param_names[3] = { "arg0", "arg1", "arg2" };
  for(std::size_t i = 0; i < 3; ++i) {
    lowir_model::Parameter parameter;
    parameter.name = program->strings.intern(param_names[i]);
    parameter.type = lowir_model::builtin_lowir_type(param_kinds[i]);
    declaration.params.push_back(parameter);
  }
  declaration.return_type = lowir_model::builtin_lowir_type(lowir_model::LTK_PTR);
  declaration.boundary.arity = lowir_model::CAM_FIXED;
  declaration.boundary.unwind = lowir_model::CUM_NO;
  declaration.metadata.binding = lowir_model::SBM_STRONG;
  declaration.metadata.object_symbol = program->strings.intern(object_name);
  program->function_declarations.push_back(declaration);
  return symbol;
}

// `call @__builtin_fill_units(dst, value, count, unit)` on the object symbol
// cppgm_builtin_fill_units: `count` units of `unit` bytes (2, 4 or 8) each
// holding the low `unit` bytes of `value`.  The backend lowers it inline
// to `rep stosw`, `rep stosd` or `rep stosq` when its result is unused.
lowir_model::SymbolId fill_units_symbol(lowir_model::LowirProgram * program)
{
  const std::string object_name = "cppgm_builtin_fill_units";
  for(std::size_t i = 0; i < program->function_declarations.size(); ++i) {
    const lowir_model::FunctionDeclaration & declaration =
      program->function_declarations[i];
    if(declaration.metadata.object_symbol.valid() &&
       program->strings.get(declaration.metadata.object_symbol) == object_name)
      return declaration.symbol;
  }
  const lowir_model::SymbolId symbol =
    lowir_model::append_lowir_symbol(*program, "__builtin_fill_units");
  lowir_model::FunctionDeclaration declaration;
  declaration.symbol = symbol;
  static const char * const param_names[4] = { "arg0", "arg1", "arg2", "arg3" };
  for(std::size_t i = 0; i < 4; ++i) {
    lowir_model::Parameter parameter;
    parameter.name = program->strings.intern(param_names[i]);
    parameter.type = lowir_model::builtin_lowir_type(
      i == 0 ? lowir_model::LTK_PTR : lowir_model::LTK_I64);
    declaration.params.push_back(parameter);
  }
  declaration.return_type = lowir_model::builtin_lowir_type(lowir_model::LTK_PTR);
  declaration.boundary.arity = lowir_model::CAM_FIXED;
  declaration.boundary.unwind = lowir_model::CUM_NO;
  declaration.metadata.binding = lowir_model::SBM_STRONG;
  declaration.metadata.object_symbol = program->strings.intern(object_name);
  program->function_declarations.push_back(declaration);
  return symbol;
}

// A label for the guarded fill's fast block, on the preheader pass's pattern.
lowir_model::StringId fresh_fill_label(lowir_model::LowirProgram * program,
                                       const Function & function)
{
  if(program->presentation_policy == lowir_model::PRESENTATION_OBJECT_ONLY)
    return lowir_model::StringId();
  std::size_t ordinal = function.next_block_id;
  for(;;) {
    const lowir_model::StringId candidate = program->strings.intern(
      "__fill_fast_" + std::to_string(ordinal++));
    if(std::find(function.block_labels.begin(), function.block_labels.end(),
                 candidate) == function.block_labels.end())
      return candidate;
  }
}

Operand integer_operand(long long value, lowir_model::LowTypeKind kind)
{
  Operand result;
  result.kind = Operand::OP_INTEGER;
  result.has_int_value = true;
  result.int_value = value;
  result.int_high = value < 0 ? ~UINT64_C(0) : UINT64_C(0);
  result.literal_type = lowir_model::builtin_lowir_type(kind);
  return result;
}

Operand temp_operand(lowir_model::ValueId value)
{
  Operand result;
  result.kind = Operand::OP_TEMP;
  result.value = value;
  return result;
}

}  // namespace

bool simplify_counted_loops(Function * function,
                            lowir_analysis::FunctionAnalysis * analysis,
                            Stats * stats)
{
  const lowir_analysis::LoopForest & forest = analysis->loop_forest();
  if(forest.loops.empty()) return false;
  const lowir_analysis::ValueIndex & values = analysis->value_index();

  bool changed = false;
  bool cfg_changed = false;
  for(std::size_t loop_index = 0;
      loop_index < forest.loops.size(); ++loop_index) {
    const lowir_analysis::NaturalLoop & loop = forest.loops[loop_index];
    if(loop.preheader == kNoIndex || loop.has_eh || loop.latches.size() != 1)
      continue;
    std::vector<Instruction> & header =
      function->blocks[loop.header].instructions;
    if(header.empty() || header.back().kind != Instruction::IK_BRANCH ||
       header.back().first.kind != Operand::OP_TEMP) continue;
    Instruction & branch = header.back();
    const lowir_analysis::ValueDefinition compare_definition =
      values.definition(branch.first.value);
    if(compare_definition.kind !=
         lowir_analysis::ValueDefinition::INSTRUCTION ||
       compare_definition.block != loop.header) continue;
    Instruction * compare = &function->blocks[compare_definition.block]
      .instructions[compare_definition.instruction];
    if(compare->kind != Instruction::IK_CMP ||
       compare->first.kind != Operand::OP_TEMP ||
       compare->second.kind != Operand::OP_INTEGER ||
       !compare->second.has_int_value ||
       compare->type.kind != lowir_model::LTK_I64 ||
       compare->second.int_high !=
         (compare->second.int_value < 0 ? ~UINT64_C(0) : UINT64_C(0)))
      continue;
    Induction induction;
    if(!find_induction(function, loop, values, compare->first.value,
                       &induction)) continue;
    if(stats) ++stats->induction_variables;

    const std::size_t true_target = analysis->graph().find(branch.second.block);
    const std::size_t false_target = analysis->graph().find(branch.third.block);
    const bool true_inside = loop.contains(true_target);
    const bool false_inside = loop.contains(false_target);
    if(true_inside == false_inside) continue;
    LowOperation::Kind condition = compare->op.kind;
    std::size_t exit = false_target;
    if(!true_inside) {
      condition = negate_compare(condition);
      exit = true_target;
      if(condition != LowOperation::LOP_NONE &&
         values.use_count(compare->dest) == 1) {
        compare->op.kind = condition;
        std::swap(branch.second, branch.third);
        changed = true;
        if(stats) { ++stats->loop_exits_canonicalized; ++stats->rewrites; }
      }
    }

    for(std::size_t member = 0; member < loop.blocks.size(); ++member) {
      std::vector<Instruction> & instructions =
        function->blocks[loop.blocks[member]].instructions;
      for(std::size_t instruction = 0;
          instruction < instructions.size(); ++instruction) {
        Instruction & candidate = instructions[instruction];
        if(candidate.kind != Instruction::IK_BINARY ||
           candidate.op.kind != LowOperation::LOP_MUL) continue;
        Operand * induction_operand = &candidate.first;
        Operand * factor = &candidate.second;
        if(induction_operand->kind != Operand::OP_TEMP ||
           induction_operand->value != induction.value)
          std::swap(induction_operand, factor);
        unsigned shift = 0;
        if(induction_operand->kind != Operand::OP_TEMP ||
           induction_operand->value != induction.value ||
           factor->kind != Operand::OP_INTEGER || !factor->has_int_value ||
           factor->int_high != 0 ||
           !power_of_two(factor->int_value, &shift)) continue;
        const Operand induction_value = *induction_operand;
        const Operand factor_value = *factor;
        candidate.first = induction_value;
        candidate.second = factor_value;
        candidate.second.int_value = shift;
        candidate.second.int_high = 0;
        candidate.second.has_spelling = false;
        candidate.op.kind = LowOperation::LOP_SHL;
        changed = true;
        if(stats) { ++stats->induction_strength_reductions; ++stats->rewrites; }
      }
    }

    if(exit == kNoIndex || loop.exits.size() != 1 || loop.exits[0] != exit ||
       !proves_termination(induction.initial, induction.step,
                           compare->second.int_value, condition) ||
       has_outside_value_use(*function, loop)) continue;
    bool pure = true;
    for(std::size_t member = 0; member < loop.blocks.size() && pure; ++member)
      for(std::size_t instruction = 0;
          instruction < function->blocks[loop.blocks[member]].instructions.size();
          ++instruction)
        pure = pure && pure_loop_instruction(
          function->blocks[loop.blocks[member]].instructions[instruction]);
    if(!pure || (!function->blocks[exit].instructions.empty() &&
       function->blocks[exit].instructions.front().kind == Instruction::IK_PHI))
      continue;
    Instruction & preheader =
      function->blocks[loop.preheader].instructions.back();
    if(preheader.kind != Instruction::IK_JUMP ||
       preheader.first.block != function->blocks[loop.header].id) continue;
    preheader.first.block = function->blocks[exit].id;
    erase_loop_blocks(function, loop);
    changed = true;
    cfg_changed = true;
    if(stats) { ++stats->dead_loops_removed; ++stats->rewrites; }
    // The cached loop forest describes the old CFG.  Later cleanup removes
    // this loop; leave any other loop for the next optimizer invocation.
    break;
  }
  if(cfg_changed) analysis->invalidate_cfg();
  return changed;
}

// Delete a loop that does nothing: every instruction in it is effect-free,
// no value it defines is used after it, it has one exit, and it leaves on an
// equality test between a pointer it walks by a constant stride and a
// pointer from outside -- the range walk, whose termination N3485 5.7/5
// guarantees for any defined program (see exits_on_pointer_walk).  libc++
// presents one at every destruction of a vector of a trivially destructible
// element (__base_destruct_at_end), which is why this runs at -O1: the loop
// is not slow code, it is no code.  Integer-counted loops stay with
// simplify_counted_loops and its termination proof.
bool delete_effect_free_loops(Function * function,
                              lowir_analysis::FunctionAnalysis * analysis,
                              Stats * stats)
{
  bool changed = false;
  for(;;) {
    const lowir_analysis::LoopForest & forest = analysis->loop_forest();
    if(forest.loops.empty()) break;
    const lowir_analysis::ValueIndex & values = analysis->value_index();
    bool deleted = false;
    for(std::size_t loop_index = 0;
        loop_index < forest.loops.size() && !deleted; ++loop_index) {
      const lowir_analysis::NaturalLoop & loop = forest.loops[loop_index];
      if(loop.preheader == kNoIndex || loop.has_eh ||
         loop.exits.size() != 1) continue;
      const std::size_t exit = loop.exits[0];
      if(loop.contains(exit)) continue;
      Instruction & preheader =
        function->blocks[loop.preheader].instructions.back();
      if(preheader.kind != Instruction::IK_JUMP ||
         preheader.first.block != function->blocks[loop.header].id) continue;
      bool pure = true;
      for(std::size_t member = 0; member < loop.blocks.size() && pure; ++member) {
        const std::vector<Instruction> & instructions =
          function->blocks[loop.blocks[member]].instructions;
        for(std::size_t i = 0; i < instructions.size() && pure; ++i)
          pure = effect_free_instruction(instructions[i]);
      }
      if(!pure || has_outside_value_use(*function, loop) ||
         !exits_on_pointer_walk(*function, loop, values, exit)) continue;
      if(!retarget_exit_phis(function, loop, exit)) continue;
      preheader.first.block = function->blocks[exit].id;
      erase_loop_blocks(function, loop);
      deleted = true;
      changed = true;
      if(stats) { ++stats->effect_free_loops_removed; ++stats->rewrites; }
    }
    if(!deleted) break;
    // The forest described the old CFG; rebuild before looking again.
    analysis->invalidate_cfg();
    analysis->invalidate_values();
  }
  return changed;
}

// Replace a fill loop with one call to the fill builtin, which the native
// backend lowers to rep stosb.  libc++ writes vector(n, v) as a per-element
// construct loop and leaves the fill to the compiler, where libstdc++ calls
// memset itself; without this the libc++ cell fills a byte at a time.  Only
// the sized-construction shape (end = start + n) is taken, with a constant
// whose bytes are all alike, or a one-byte value whether held or reloaded.
// A use of the walked pointer after the loop becomes a use of the bound.
// The value a fill stores, as the fill builtin takes it: for a unit fill
// the low `width` bytes of the value as an i64, for a byte fill the splat
// byte as an i32.  A value the loop loaded is loaded again first; the
// instructions this needs go to `added`.
Operand fill_byte_value(Function * function, const FillLoop & fill,
                        bool unit_fill, std::vector<Instruction> * added)
{
  Operand byte_value;
  if(unit_fill) {
    // The value's low `width` bytes, as an i64, and the unit count.
    if(fill.value.kind == Operand::OP_INTEGER) {
      const unsigned long long mask = fill.width == 8 ?
        ~0ull : (1ull << (8 * fill.width)) - 1;
      byte_value = integer_operand(
        static_cast<long long>(
          static_cast<unsigned long long>(fill.value.int_value) & mask),
        lowir_model::LTK_I64);
    } else {
      Operand narrow = fill.value;
      if(fill.value_loaded) {
        Instruction load;
        load.kind = Instruction::IK_LOAD;
        load.type = fill.load_type;
        load.first = fill.load_address;
        load.dest = lowir_model::append_lowir_fresh_generated_value(
          *function, fill.load_type);
        added->push_back(load);
        narrow = temp_operand(load.dest);
      }
      if(fill.width == 8) byte_value = narrow;
      else {
      Instruction widen;
      widen.kind = Instruction::IK_CONVERT;
      widen.op.kind = LowOperation::LOP_ZEXT;
      widen.type = lowir_model::builtin_lowir_type(lowir_model::LTK_I64);
      widen.source_type = fill.value_loaded ? fill.load_type :
        function->value_types[fill.value.value];
      widen.first = narrow;
      widen.dest = lowir_model::append_lowir_fresh_generated_value(
        *function, widen.type);
      added->push_back(widen);
      byte_value = temp_operand(widen.dest);
      }
    }
  } else if(fill.value.kind == Operand::OP_INTEGER) {
    long long byte = 0;
    splat_byte(fill.value, fill.width, &byte);
    byte_value = integer_operand(byte, lowir_model::LTK_I32);
  } else {
    Operand narrow = fill.value;
    if(fill.value_loaded) {
      Instruction load;
      load.kind = Instruction::IK_LOAD;
      load.type = fill.load_type;
      load.first = fill.load_address;
      load.dest = lowir_model::append_lowir_fresh_generated_value(
        *function, fill.load_type);
      added->push_back(load);
      narrow = temp_operand(load.dest);
    }
    Instruction widen;
    widen.kind = Instruction::IK_CONVERT;
    widen.op.kind = LowOperation::LOP_ZEXT;
    widen.type = lowir_model::builtin_lowir_type(lowir_model::LTK_I32);
    widen.source_type = fill.value_loaded ? fill.load_type :
      function->value_types[fill.value.value];
    widen.first = narrow;
    widen.dest = lowir_model::append_lowir_fresh_generated_value(
      *function, widen.type);
    added->push_back(widen);
    byte_value = temp_operand(widen.dest);
  }
  return byte_value;
}

bool recognize_fill_loops(lowir_model::LowirProgram * program,
                          Function * function,
                          lowir_analysis::FunctionAnalysis * analysis,
                          Stats * stats)
{
  bool changed = false;
  for(;;) {
    const lowir_analysis::LoopForest & forest = analysis->loop_forest();
    if(forest.loops.empty()) break;
    const lowir_analysis::ValueIndex & values = analysis->value_index();
    bool rewritten = false;
    for(std::size_t loop_index = 0;
        loop_index < forest.loops.size() && !rewritten; ++loop_index) {
      const lowir_analysis::NaturalLoop & loop = forest.loops[loop_index];
      FillLoop fill;
      if(!match_fill_loop(*function, loop, values, analysis->graph(), &fill))
        continue;
      if(!rewrite_outside_pointer_uses(function, loop, fill.pointers,
                                       fill.bound, false)) continue;
      Instruction & preheader_jump =
        function->blocks[loop.preheader].instructions.back();
      if(preheader_jump.kind != Instruction::IK_JUMP ||
         preheader_jump.first.block != function->blocks[loop.header].id)
        continue;
      long long splat_probe = 0;
      const bool unit_fill = fill.width != 1 &&
        !(fill.value.kind == Operand::OP_INTEGER &&
          splat_byte(fill.value, fill.width, &splat_probe));
      // A unit fill keeps its loop for short counts: `rep stos` pays a
      // startup of tens of cycles, and the compiler's own
      // `resize(id + 1, 0)` fills one element at a time.
      const bool versioned = fill.alias_checked || unit_fill;
      if(!versioned && !retarget_exit_phis(function, loop, fill.exit)) continue;
      rewrite_outside_pointer_uses(function, loop, fill.pointers, fill.bound,
                                   true);

      std::vector<Instruction> & preheader =
        function->blocks[loop.preheader].instructions;
      std::vector<Instruction> added;
      const Operand byte_value = fill_byte_value(function, fill, unit_fill, &added);
      Instruction call;
      call.kind = Instruction::IK_CALL;
      call.type = lowir_model::builtin_lowir_type(lowir_model::LTK_PTR);
      // The builtin returns its destination like memset; the value is not
      // used, which is what lets the backend lower the call inline.
      call.dest = lowir_model::append_lowir_fresh_generated_value(
        *function, call.type);
      call.first.kind = Operand::OP_GLOBAL;
      call.first.symbol = unit_fill ? fill_units_symbol(program) :
        fill_symbol(program);
      call.first.address_binding = Operand::ADDRESS_PREEMPTIBLE;
      call.call_boundary.unwind = lowir_model::CUM_NO;
      call.args.push_back(fill.start);
      call.args.push_back(byte_value);
      std::vector<Instruction> guard;
      Operand unit_count;
      if(unit_fill) {
        Instruction units;
        units.kind = Instruction::IK_BINARY;
        units.op.kind = LowOperation::LOP_USHR;
        units.type = lowir_model::builtin_lowir_type(lowir_model::LTK_I64);
        units.first = fill.length;
        units.second = integer_operand(
          fill.width == 8 ? 3 : fill.width == 4 ? 2 : 1, lowir_model::LTK_I64);
        units.dest = lowir_model::append_lowir_fresh_generated_value(
          *function, units.type);
        guard.push_back(units);
        unit_count = temp_operand(units.dest);
        call.args.push_back(unit_count);
        call.args.push_back(integer_operand(
          static_cast<long long>(fill.width), lowir_model::LTK_I64));
      } else call.args.push_back(fill.length);
      added.push_back(call);
      if(versioned) {
        // Version: the preheader tests whether the source lies outside the
        // range, or inside it on an element boundary (then every iteration
        // rewrites the element it read from, and the fill is the same), and
        // takes the fast block; otherwise the loop runs as written.
        const lowir_model::BlockId header_id = function->blocks[loop.header].id;
        const lowir_model::BlockId exit_id = function->blocks[fill.exit].id;
        lowir_model::Block fast;
        fast.id = lowir_model::allocate_lowir_block_id(
          *function, fresh_fill_label(program, *function));
        fast.instructions = added;
        Instruction to_exit;
        to_exit.kind = Instruction::IK_JUMP;
        to_exit.first.kind = Operand::OP_LABEL;
        to_exit.first.block = exit_id;
        fast.instructions.push_back(to_exit);
        const LowType i64 = lowir_model::builtin_lowir_type(lowir_model::LTK_I64);
        const LowType i1 = lowir_model::builtin_lowir_type(lowir_model::LTK_I1);
        const LowType ptr = lowir_model::builtin_lowir_type(lowir_model::LTK_PTR);
        Operand take_fast;
        if(fill.alias_checked) {
        Instruction source_end;
        source_end.kind = Instruction::IK_INDEX;
        source_end.type = lowir_model::builtin_lowir_type(lowir_model::LTK_I8);
        source_end.first = fill.load_address;
        source_end.second = integer_operand(
          static_cast<long long>(fill.width), lowir_model::LTK_I64);
        source_end.dest = lowir_model::append_lowir_fresh_generated_value(*function, ptr);
        guard.push_back(source_end);
        Instruction before;
        before.kind = Instruction::IK_CMP;
        before.op.kind = LowOperation::LOP_ULE;
        before.type = ptr;
        before.first = temp_operand(source_end.dest);
        before.second = fill.start;
        before.dest = lowir_model::append_lowir_fresh_generated_value(*function, i1);
        guard.push_back(before);
        Instruction after;
        after.kind = Instruction::IK_CMP;
        after.op.kind = LowOperation::LOP_UGE;
        after.type = ptr;
        after.first = fill.load_address;
        after.second = fill.bound;
        after.dest = lowir_model::append_lowir_fresh_generated_value(*function, i1);
        guard.push_back(after);
        Instruction outside;
        outside.kind = Instruction::IK_BINARY;
        outside.op.kind = LowOperation::LOP_OR;
        outside.type = i1;
        outside.first = temp_operand(before.dest);
        outside.second = temp_operand(after.dest);
        outside.dest = lowir_model::append_lowir_fresh_generated_value(*function, i1);
        guard.push_back(outside);
        Instruction delta;
        delta.kind = Instruction::IK_BINARY;
        delta.op.kind = LowOperation::LOP_SUB;
        delta.type = ptr;
        delta.first = fill.load_address;
        delta.second = fill.start;
        delta.dest = lowir_model::append_lowir_fresh_generated_value(*function, i64);
        guard.push_back(delta);
        Instruction misaligned;
        misaligned.kind = Instruction::IK_BINARY;
        misaligned.op.kind = LowOperation::LOP_AND;
        misaligned.type = i64;
        misaligned.first = temp_operand(delta.dest);
        misaligned.second = integer_operand(
          static_cast<long long>(fill.width - 1), lowir_model::LTK_I64);
        misaligned.dest = lowir_model::append_lowir_fresh_generated_value(*function, i64);
        guard.push_back(misaligned);
        Instruction aligned;
        aligned.kind = Instruction::IK_CMP;
        aligned.op.kind = LowOperation::LOP_EQ;
        aligned.type = i64;
        aligned.first = temp_operand(misaligned.dest);
        aligned.second = integer_operand(0, lowir_model::LTK_I64);
        aligned.dest = lowir_model::append_lowir_fresh_generated_value(*function, i1);
        guard.push_back(aligned);
        Instruction fast_path;
        fast_path.kind = Instruction::IK_BINARY;
        fast_path.op.kind = LowOperation::LOP_OR;
        fast_path.type = i1;
        fast_path.first = temp_operand(outside.dest);
        fast_path.second = temp_operand(aligned.dest);
        fast_path.dest = lowir_model::append_lowir_fresh_generated_value(*function, i1);
        guard.push_back(fast_path);
        take_fast = temp_operand(fast_path.dest);
        }
        if(unit_fill) {
          Instruction enough;
          enough.kind = Instruction::IK_CMP;
          enough.op.kind = LowOperation::LOP_UGE;
          enough.type = i64;
          enough.first = unit_count;
          enough.second = integer_operand(kUnitFillMinimumCount, lowir_model::LTK_I64);
          enough.dest = lowir_model::append_lowir_fresh_generated_value(*function, i1);
          guard.push_back(enough);
          if(fill.alias_checked) {
            Instruction both;
            both.kind = Instruction::IK_BINARY;
            both.op.kind = LowOperation::LOP_AND;
            both.type = i1;
            both.first = take_fast;
            both.second = temp_operand(enough.dest);
            both.dest = lowir_model::append_lowir_fresh_generated_value(*function, i1);
            guard.push_back(both);
            take_fast = temp_operand(both.dest);
          } else take_fast = temp_operand(enough.dest);
        }
        Instruction branch;
        branch.kind = Instruction::IK_BRANCH;
        branch.first = take_fast;
        branch.second.kind = Operand::OP_LABEL;
        branch.second.block = fast.id;
        branch.third.kind = Operand::OP_LABEL;
        branch.third.block = header_id;
        preheader.pop_back();
        preheader.insert(preheader.end(), guard.begin(), guard.end());
        preheader.push_back(branch);
        // The exit block's phis gain the fast edge, carrying what the loop
        // edge carries: an invariant, or the walked pointer already rewritten
        // to the bound.
        std::vector<unsigned char> in_loop;
        for(std::size_t i = 0; i < function->blocks.size(); ++i) {
          const lowir_model::BlockId id = function->blocks[i].id;
          if(id >= in_loop.size()) in_loop.resize(id + 1, 0);
          if(loop.contains(i)) in_loop[id] = 1;
        }
        std::vector<Instruction> & exit_block = function->blocks[fill.exit].instructions;
        for(std::size_t i = 0; i < exit_block.size(); ++i) {
          Instruction & phi = exit_block[i];
          if(phi.kind != Instruction::IK_PHI) break;
          for(std::size_t arg = 0; arg + 1 < phi.args.size(); arg += 2) {
            const lowir_model::BlockId from = phi.args[arg].block;
            if(from >= in_loop.size() || !in_loop[from]) continue;
            Operand label = phi.args[arg];
            label.block = fast.id;
            phi.args.push_back(label);
            phi.args.push_back(phi.args[arg + 1]);
            break;
          }
        }
        function->blocks.push_back(std::move(fast));
      } else {
        preheader.insert(preheader.end() - 1, added.begin(), added.end());
        preheader.back().first.block = function->blocks[fill.exit].id;
        erase_loop_blocks(function, loop);
      }
      rewritten = true;
      changed = true;
      if(stats) { ++stats->fill_loops_recognized; ++stats->rewrites; }
    }
    if(!rewritten) break;
    analysis->invalidate_cfg();
    analysis->invalidate_values();
  }
  return changed;
}

}  // namespace lowir_opt
