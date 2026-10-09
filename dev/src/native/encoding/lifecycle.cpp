#include "native/encoding/lifecycle.h"
#include "native/encoding/instructions.h"

namespace lowir_native {
namespace lifecycle_detail {
namespace {
// Process-exit callbacks registered by atexit, and thread-exit callbacks
// registered by __cxa_thread_atexit; the latter run first, as glibc's exit
// runs the main thread's thread_local destructors before atexit handlers.
const char kCallbackHead[] = ".__cppgm_exit_callback_head";
const char kThreadCallbackHead[] = ".__cppgm_thread_exit_callback_head";

bool pointer_parameters(const lowir_model::FunctionDeclaration& declaration,
                        std::size_t count)
{
  if(declaration.params.size() != count ||
     declaration.return_type.kind != lowir_model::LTK_I32 ||
     declaration.boundary.arity != lowir_model::CAM_FIXED) return false;
  for(std::size_t i = 0; i < count; ++i)
    if(declaration.params[i].type.kind != lowir_model::LTK_PTR) return false;
  return true;
}

// A standalone program has one module; its __dso_handle is runtime storage
// whose only use is identity in a destructor registration.
void plan_dso_handle(const lowir_model::LowirProgram& source, mir_model::MirProgram* target)
{
  for(std::size_t i = 0; i < source.global_declarations.size(); ++i) {
    const lowir_model::GlobalDeclaration& declaration = source.global_declarations[i];
    if(!declaration.metadata.object_symbol.valid() ||
       source.strings.get(declaration.metadata.object_symbol) != "__dso_handle") continue;
    bool defined = false;
    for(std::size_t j = 0; j < source.globals.size(); ++j)
      defined = defined || source.globals[j].symbol == declaration.symbol ||
        source.globals[j].metadata.object_symbol == declaration.metadata.object_symbol;
    if(defined) continue;
    mir_model::MirRuntimeData data;
    data.symbol = declaration.symbol;
    data.object_symbol = declaration.metadata.object_symbol;
    target->runtime_data.push_back(data);
  }
}
}

void plan_runtime(const lowir_model::LowirProgram& source, mir_model::MirProgram* target)
{
  plan_dso_handle(source, target);
  for(std::size_t i = 0; i < source.function_declarations.size(); ++i) {
    const lowir_model::FunctionDeclaration& declaration = source.function_declarations[i];
    const bool has_object_symbol = declaration.metadata.object_symbol.valid();
    const std::string& object_name = has_object_symbol ?
      source.strings.get(declaration.metadata.object_symbol) :
      lowir_model::lowir_symbol_name(source, declaration.symbol);
    const bool process_exit = object_name == "atexit" && pointer_parameters(declaration, 1);
    if(!process_exit && !(object_name == "__cxa_thread_atexit" &&
                          pointer_parameters(declaration, 3))) continue;
    bool defined = false;
    for(std::size_t j = 0; j < source.functions.size(); ++j)
      defined = defined || source.functions[j].symbol == declaration.symbol ||
        (has_object_symbol &&
         source.functions[j].metadata.object_symbol == declaration.metadata.object_symbol);
    if(defined) continue;
    mir_model::MirRuntimeFunction runtime;
    runtime.kind = process_exit ? mir_model::RuntimeFunction::RF_ATEXIT :
      mir_model::RuntimeFunction::RF_THREAD_ATEXIT;
    runtime.symbol = declaration.symbol;
    runtime.object_symbol = declaration.metadata.object_symbol;
    target->runtime_functions.push_back(runtime);
  }
}

bool has_runtime(const mir_model::MirProgram& program)
{
  for(std::size_t i = 0; i < program.runtime_functions.size(); ++i)
    if(program.runtime_functions[i].kind == mir_model::RuntimeFunction::RF_ATEXIT ||
       program.runtime_functions[i].kind == mir_model::RuntimeFunction::RF_THREAD_ATEXIT)
      return true;
  return false;
}

// A callback record is one page: callback, next record, callback argument.
void emit_registration(elf_detail::CodeBuffer& out, mir_model::RuntimeFunction::Kind kind)
{
  const bool thread_exit = kind == mir_model::RuntimeFunction::RF_THREAD_ATEXIT;
  const lowir_model::LocalLabelId failed = out.internal_label("atexit_failed");
  out.byte(0x57); out.byte(0x56); // Preserve callback and argument across mmap.
  emit_immediate_move(out, XR_RAX, 9);
  emit_immediate_move(out, XR_RDI, 0);
  emit_immediate_move(out, XR_RSI, 4096);
  emit_immediate_move(out, XR_RDX, 3);
  emit_immediate_move(out, XR_R10, 0x22);
  emit_immediate_move(out, XR_R8, UINT64_MAX);
  emit_immediate_move(out, XR_R9, 0);
  out.byte(0x0f); out.byte(0x05); out.byte(0x5e); out.byte(0x5a);
  emit_test_register(out, XR_RAX);
  emit_condition_jump(out, XC_S, failed);
  emit_store(out, XR_RAX, 0, XR_RDX, 64);
  if(thread_exit) emit_store(out, XR_RAX, 16, XR_RSI, 64);
  emit_symbol_move(out, XR_RDI, thread_exit ? kThreadCallbackHead : kCallbackHead);
  emit_load(out, XR_RDX, XR_RDI, 0, 64);
  emit_store(out, XR_RAX, 8, XR_RDX, 64);
  emit_store(out, XR_RDI, 0, XR_RAX, 64);
  emit_immediate_move(out, XR_RAX, 0);
  out.byte(0xc3);
  out.label(failed);
  emit_immediate_move(out, XR_RAX, UINT64_MAX);
  out.byte(0xc3);
}

namespace {
void emit_list_drain(elf_detail::CodeBuffer& out, const char* head)
{
  const lowir_model::LocalLabelId loop = out.internal_label("atexit_drain");
  const lowir_model::LocalLabelId done = out.internal_label("atexit_done");
  out.label(loop);
  emit_symbol_move(out, XR_RDX, head);
  emit_load(out, XR_RDI, XR_RDX, 0, 64);
  emit_test_register(out, XR_RDI);
  emit_condition_jump(out, XC_E, done);
  emit_load(out, XR_R13, XR_RDI, 0, 64);
  emit_load(out, XR_R14, XR_RDI, 16, 64);
  emit_load(out, XR_RAX, XR_RDI, 8, 64);
  emit_store(out, XR_RDX, 0, XR_RAX, 64);
  // Pop before the call so callbacks may register more callbacks.
  emit_immediate_move(out, XR_RAX, 11);
  emit_immediate_move(out, XR_RSI, 4096);
  out.byte(0x0f); out.byte(0x05);
  emit_register_move(out, XR_RDI, XR_R14);
  emit_register_move(out, XR_RAX, XR_R13);
  out.byte(0xff); out.byte(0xd0);
  out.byte(0xe9); out.relative32(loop);
  out.label(done);
}
}

void emit_drain(elf_detail::CodeBuffer& out)
{
  emit_list_drain(out, kThreadCallbackHead);
  emit_list_drain(out, kCallbackHead);
}

void emit_data(elf_detail::CodeBuffer& out)
{
  out.align(8);
  out.label(kCallbackHead);
  out.zeros(8);
  out.label(kThreadCallbackHead);
  out.zeros(8);
}
}
}
