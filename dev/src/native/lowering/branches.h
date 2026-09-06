#pragma once
#include "native/analysis/function.h"
#include "native/mir/block_labels.h"
#include "native/mir/construction.h"
#include "native/lowering/wide.h"
#include <vector>
namespace lowir_native {
namespace branch_detail {
// The two-way and multi-way branches: the condition (or a wide condition's
// two words, or-ed) compared with zero, then a conditional jump per target
// and the fall-through jump.  A switch compares the selector against each
// case in rax; past 31 cases an immediate case is compared directly.
template <class Derived>
class BranchLowering
{
protected:
  void emit_branch(const lowir_model::Instruction & instruction, std::vector<mir_model::MirInstruction> & out)
  {
    using namespace build;
    Derived & lowerer = static_cast<Derived &>(*this);
    const lowir_model::LowType & condition_type = lowerer.operand_type(instruction.first);
    if(wide::is_integer(condition_type)) {
      const wide::Value condition = lowerer.wide_value(instruction.first);
      wide::append_word_to_register(condition, 0, XR_RAX, XR_R11, out);
      wide::append_word_to_register(condition, 1, XR_RDX, XR_R11, out);
      mir_model::MirInstruction combine = machine_instruction(mir_model::MirInstruction::MI_OR, machine_type(lowir_model::LTK_I64));
      append_operand(combine, reg_operand(XR_RAX));
      append_operand(combine, reg_operand(XR_RDX));
      out.push_back(combine);
    } else if(instruction.first.kind != lowir_model::Operand::OP_TEMP ||
              !lowerer.facts_.has(instruction.first.value,
                          analysis::FunctionFacts::VF_DIRECT_BRANCH_CALL_RESULT))
      lowerer.move_value_to_register(out, XR_RAX, lowerer.resolve(instruction.first),
                             condition_type);
    mir_model::MirInstruction compare = machine_instruction(mir_model::MirInstruction::MI_CMP, machine_type(lowir_model::LTK_I64));
    append_operand(compare, reg_operand(XR_RAX));
    append_operand(compare, immediate(0));
    out.push_back(compare);
    mir_model::MirInstruction branch = machine_instruction(mir_model::MirInstruction::MI_JCC);
    branch.condition = XC_NE;
    append_operand(branch, native_block_operand(lowerer.source_, instruction.second));
    out.push_back(branch);
    mir_model::MirInstruction jump = machine_instruction(mir_model::MirInstruction::MI_JMP);
    append_operand(jump, native_block_operand(lowerer.source_, instruction.third));
    out.push_back(jump);
    lowerer.consume(instruction.first);
  }
  void emit_switch(const lowir_model::Instruction & instruction, std::vector<mir_model::MirInstruction> & out)
  {
    using namespace build;
    Derived & lowerer = static_cast<Derived &>(*this);
    mir_model::MirOperand source = lowerer.resolve(instruction.first);
    if(instruction.first.kind == lowir_model::Operand::OP_TEMP &&
       lowerer.facts_.first_use[instruction.first.value] == lowerer.position_ &&
       lowerer.incoming_parameter_register_known_[instruction.first.value]) {
      const X64Register incoming =
        lowerer.incoming_parameter_registers_[instruction.first.value];
      if(lowerer.incoming_parameter_register_is_intact(
           instruction.first.value, incoming))
        source = reg_operand(incoming);
    }
    lowerer.move_value_to_register(out, XR_RAX, source,
                           lowerer.operand_type(instruction.first));
    for(std::size_t i = 0; i < instruction.args.size(); i += 2) {
      const mir_model::MirOperand case_value = lowerer.resolve(instruction.args[i]);
      if(instruction.args.size() < 32 || case_value.kind != mir_model::MirOperand::OP_IMM) lowerer.move_value_to_register(out, XR_RCX, case_value, lowerer.operand_type(instruction.args[i]));
      mir_model::MirInstruction compare = machine_instruction(mir_model::MirInstruction::MI_CMP, machine_type(lowir_model::LTK_I64));
      append_operand(compare, reg_operand(XR_RAX));
      append_operand(compare, instruction.args.size() >= 32 && case_value.kind == mir_model::MirOperand::OP_IMM ? case_value : reg_operand(XR_RCX));
      out.push_back(compare);
      mir_model::MirInstruction branch = machine_instruction(mir_model::MirInstruction::MI_JCC);
      branch.condition = XC_E;
      append_operand(branch,
        native_block_operand(lowerer.source_, instruction.args[i + 1]));
      out.push_back(branch);
      lowerer.consume(instruction.args[i]);
    }
    mir_model::MirInstruction jump = machine_instruction(mir_model::MirInstruction::MI_JMP);
    append_operand(jump, native_block_operand(lowerer.source_, instruction.second));
    out.push_back(jump);
    lowerer.consume(instruction.first);
  }
};
}  // namespace branch_detail
}  // namespace lowir_native
