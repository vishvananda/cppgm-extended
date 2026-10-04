#include "native/frame/epilogue.h"

#include "native/allocation/registers.h"
#include "native/errors.h"
#include "native/encoding/instructions.h"


namespace lowir_native {
namespace epilogue_detail {
namespace {

std::size_t stack_adjust_size(std::size_t bytes)
{
  if(!bytes) return 0;
  return bytes <= 127 ? 4 : 7;
}

std::size_t pop_size(X64Register reg)
{
  return reg >= XR_R8 ? 2 : 1;
}

bool has_call(const mir_model::MirFunction & function)
{
  for(std::size_t block = 0; block < function.blocks.size(); ++block)
    for(std::size_t index = 0;
        index < function.blocks[block].instructions.size(); ++index) {
      const mir_model::MirInstruction::Opcode opcode =
        function.blocks[block].instructions[index].opcode;
      if(opcode == mir_model::MirInstruction::MI_CALL ||
         opcode == mir_model::MirInstruction::MI_CALL_INDIRECT) return true;
    }
  return false;
}

std::size_t encoded_size(const mir_model::MirFunction & function)
{
  std::size_t result = 0;
  if(function.omit_frame_pointer) {
    result += stack_adjust_size(function_stack_adjustment(function));
  } else if(function.has_dynamic_stack || function.frame_alignment > 16) {
    result += 3;  // mov rsp, rbp
    result += stack_adjust_size(function.callee_saved_regs.size() * 8);
  } else {
    result += stack_adjust_size(function_stack_adjustment(function));
  }
  for(std::size_t i = 0; i < function.callee_saved_regs.size(); ++i)
    result += pop_size(function.callee_saved_regs[i]);
  return result + (function.omit_frame_pointer ? 1 : 2);
}

}  // namespace

bool is_return(const mir_model::MirInstruction & instruction)
{
  return instruction.opcode == mir_model::MirInstruction::MI_RET ||
    instruction.opcode == mir_model::MirInstruction::MI_FRET;
}

std::size_t function_stack_adjustment(
    const mir_model::MirFunction & function)
{
  if(function.omit_frame_pointer) {
    if(function.has_dynamic_stack)
      native_errors::ThrowInternal("dynamic stack cannot omit the frame pointer");
    // SysV enters with rsp == 8 (mod 16). An odd number of eight-byte saves
    // aligns a call naturally; an even number needs one padding word. Leaves
    // need no padding because they make no aligned-call promise of their own.
    return has_call(function) && function.callee_saved_regs.size() % 2 == 0 ?
      8 : 0;
  }
  const std::size_t preserved = function.callee_saved_regs.size() * 8;
  if(function.stack_size < preserved)
    native_errors::ThrowInternal("MIR stack reservation is smaller than its saves");
  const std::size_t bytes = function.stack_size - preserved;
  return function.frame_alignment > 16 ? (bytes + 15) / 16 * 16 : bytes;
}

Plan make_plan(const mir_model::MirFunction & function)
{
  Plan result;
  for(std::size_t i = 0; i < function.blocks.size(); ++i) {
    const std::vector<mir_model::MirInstruction> & instructions =
      function.blocks[i].instructions;
    for(std::size_t j = 0; j < instructions.size(); ++j)
      if(is_return(instructions[j])) ++result.return_count;
  }
  result.physical_epilogue_count = result.return_count;
  if(result.return_count < 2 || !function.share_epilogues)
    return result;

  if(!function.blocks.empty() &&
     !function.blocks.back().instructions.empty() &&
     is_return(function.blocks.back().instructions.back())) {
    result.final_return_falls_through = true;
    result.final_block = function.blocks.size() - 1;
    result.final_instruction = function.blocks.back().instructions.size() - 1;
  }

  const std::size_t teardown_bytes = encoded_size(function);
  const std::size_t branch_count = result.return_count -
    (result.final_return_falls_through ? 1 : 0);
  const std::size_t separate_bytes = result.return_count * teardown_bytes;
  const std::size_t shared_bytes = teardown_bytes + branch_count * 5;
  if(shared_bytes >= separate_bytes) return result;

  result.shared = true;
  result.physical_epilogue_count = 1;
  return result;
}

void emit_prologue(elf_detail::CodeBuffer & out, const mir_model::MirFunction & function)
{
  if(!function.omit_frame_pointer) {
    emit_push(out, XR_RBP);
    emit_register_move(out, XR_RBP, XR_RSP);
  }
  for(std::size_t i = 0; i < function.callee_saved_regs.size(); ++i)
    emit_push(out, function.callee_saved_regs[i]);
  if(function.frame_alignment > 16) {
    emit_register_move(out, XR_R12, XR_RSP);
    emit_immediate_move(out, XR_R11,
      ~(static_cast<std::uint64_t>(function.frame_alignment) - 1));
    emit_register_alu(out, 0x21, XR_R12, XR_R11);
    emit_register_move(out, XR_RSP, XR_R12);
  }
  emit_stack_adjust(out, true,
                    static_cast<unsigned>(
                      function_stack_adjustment(function)));
}
void emit_teardown(elf_detail::CodeBuffer & out,
                            const mir_model::MirFunction & function)
{
  if(function.omit_frame_pointer) {
    emit_stack_adjust(out, false,
      static_cast<unsigned>(
        function_stack_adjustment(function)));
  } else if(function.has_dynamic_stack || function.frame_alignment > 16) {
    emit_register_move(out, XR_RSP, XR_RBP);
    emit_stack_adjust(out, true,
      static_cast<unsigned>(function.callee_saved_regs.size() * 8));
  } else
    emit_stack_adjust(out, false,
                      static_cast<unsigned>(
                        function_stack_adjustment(function)));
  for(std::size_t i = function.callee_saved_regs.size(); i != 0; --i)
    emit_pop(out, function.callee_saved_regs[i - 1]);
  if(!function.omit_frame_pointer) emit_pop(out, XR_RBP);
}

}  // namespace epilogue_detail
}  // namespace lowir_native
