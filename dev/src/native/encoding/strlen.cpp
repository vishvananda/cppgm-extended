#include "native/encoding/strlen.h"

#include "native/object/code_buffer.h"
#include "native/encoding/instructions.h"

namespace lowir_native
{
namespace strlen_detail
{

void plan_runtime(const lowir_model::LowirProgram & source,
                  lowir_model::SymbolId symbol, mir_model::MirProgram * target)
{
  if(!symbol.valid()) return;
  for(std::size_t i = 0; i < source.functions.size(); ++i)
    if(source.functions[i].symbol == symbol) return;
  for(std::size_t i = 0; i < source.function_declarations.size(); ++i) {
    const lowir_model::FunctionDeclaration & declaration =
      source.function_declarations[i];
    if(declaration.symbol != symbol || declaration.params.size() != 1 ||
       declaration.params[0].type.kind != lowir_model::LTK_PTR ||
       declaration.return_type.kind != lowir_model::LTK_I64 ||
       declaration.boundary.arity != lowir_model::CAM_FIXED) continue;
    mir_model::MirRuntimeFunction runtime;
    runtime.kind = mir_model::RuntimeFunction::RF_STRLEN;
    runtime.symbol = symbol;
    runtime.object_symbol = declaration.metadata.object_symbol;
    target->runtime_functions.push_back(runtime);
    return;
  }
}

void emit_runtime(elf_detail::CodeBuffer & out)
{
  const lowir_model::LocalLabelId loop = out.internal_label("strlen_loop");
  const lowir_model::LocalLabelId done = out.internal_label("strlen_done");
  emit_register_move(out, XR_RAX, XR_RDI);
  out.label(loop);
  emit_load(out, XR_RCX, XR_RAX, 0, 8);
  emit_test_register(out, XR_RCX);
  emit_condition_jump(out, XC_E, done);
  emit_lea(out, XR_RAX, XR_RAX, 1);
  out.byte(0xe9); out.relative32(loop);
  out.label(done);
  emit_register_alu(out, 0x29, XR_RAX, XR_RDI);
  out.byte(0xc3);
}

bool emit_prefix16_call(
    elf_detail::CodeBuffer & out,
    const mir_model::MirInstruction & instruction)
{
  if(instruction.call_encoding !=
     mir_model::MirInstruction::MCE_STRLEN_PREFIX16) return false;

  const lowir_model::LocalLabelId fallback =
    out.internal_label("builtin_strlen_fallback");
  const lowir_model::LocalLabelId done =
    out.internal_label("builtin_strlen_done");

  // Most compiler strings are tiny. Probe one page-contained SSE2 word
  // before paying the external-call cost, retaining libc as the long and
  // page-edge path.
  out.byte(0x48); out.byte(0x89); out.byte(0xf8);     // mov rax, rdi
  out.byte(0x25); out.little(0xfff, 4);               // and eax, 0xfff
  out.byte(0x3d); out.little(0xff0, 4);               // cmp eax, 0xff0
  emit_condition_jump(out, XC_A, fallback);
  out.byte(0x66); out.byte(0x0f); out.byte(0xef);     // pxor xmm0, xmm0
  out.byte(0xc0);
  out.byte(0xf3); out.byte(0x0f); out.byte(0x6f);     // movdqu xmm1, [rdi]
  out.byte(0x0f);
  out.byte(0x66); out.byte(0x0f); out.byte(0x74);     // pcmpeqb xmm1, xmm0
  out.byte(0xc8);
  out.byte(0x66); out.byte(0x0f); out.byte(0xd7);     // pmovmskb eax, xmm1
  out.byte(0xc1);
  out.byte(0x85); out.byte(0xc0);                     // test eax, eax
  emit_condition_jump(out, XC_E, fallback);
  out.byte(0x0f); out.byte(0xbc); out.byte(0xc0);     // bsf eax, eax
  if(!out.short_relative(0xeb, done)) {
    out.byte(0xe9);
    out.relative32(done);
  }
  out.label(fallback);
  out.byte(0xe8);
  out.relative32(instruction.operands[0].symbol);
  out.label(done);
  return true;
}

}
}
