#include "native/eh/catch_matching.h"

#include "native/eh/lowering.h"
#include "native/encoding/instructions.h"

#include <cstdint>

namespace lowir_native {
namespace eh {

const char kCatchMatcher[] = ".__cppgm_eh_match";

namespace {

using elf_detail::CodeBuffer;
typedef lowir_model::LocalLabelId Label;

const char kDynamicCastFind[] = ".__cppgm_dynamic_cast_find";
const char kClassBase[] = ".__cppgm_eh_class_base";

// Type identities come from the Itanium type_info layout (2.9.5): the name
// is the mangled type, so "v" is void, "Dn" is std::nullptr_t, a leading
// 'F' a function type and a leading 'M' a pointer to member. A
// __pbase_type_info keeps its cv flags at +16 and its pointee at +24; a
// pointer to member also keeps its class at +32.
void emit_name_byte(CodeBuffer & out, X64Register destination,
                    X64Register type_info, long long index)
{
  emit_load(out, XR_RAX, type_info, 8, 64);
  emit_normalized_load(out, destination, XR_RAX, index, 8, false);
}

void emit_require_name_byte(CodeBuffer & out, X64Register type_info,
                            long long index, unsigned value, Label otherwise)
{
  emit_name_byte(out, XR_RCX, type_info, index);
  if(value == 0) emit_test_register(out, XR_RCX);
  else emit_compare_immediate(out, XR_RCX, value);
  emit_condition_jump(out, XC_NE, otherwise);
}

// Falls through when the type_info is a __pointer_type_info.
void emit_require_pointer(CodeBuffer & out, X64Register type_info,
                          lowir_model::SymbolId pointer_vtable, Label otherwise)
{
  if(!pointer_vtable.valid()) {
    emit_unconditional_jump(out, otherwise);
    return;
  }
  emit_load(out, XR_RAX, type_info, 0, 64);
  emit_symbol_move(out, XR_RCX, pointer_vtable,
                   mir_model::MirOperand::ADDRESS_PREEMPTIBLE);
  emit_lea(out, XR_RCX, XR_RCX, 16);
  emit_register_alu(out, 0x39, XR_RAX, XR_RCX);
  emit_condition_jump(out, XC_NE, otherwise);
}

void emit_call(CodeBuffer & out, const char * target)
{
  out.byte(0xe8);
  out.relative32(target);
}

// rdi: thrown class type_info, rsi: handler class type_info; rax is 1 when
// the handler's class is the thrown class or one of its public bases. It
// serves a null pointer, which has no object to adjust.
void emit_class_base(CodeBuffer & out,
                     const std::vector<mir_model::MirRuntimeData> & data)
{
  const Label yes = out.internal_label("eh_class_base_yes");
  const Label no = out.internal_label("eh_class_base_no");
  const Label vmi = out.internal_label("eh_class_base_vmi");
  const lowir_model::SymbolId si_type =
    runtime_data_symbol(data, mir_model::RuntimeData::RD_RTTI_SI);
  const lowir_model::SymbolId vmi_type =
    runtime_data_symbol(data, mir_model::RuntimeData::RD_RTTI_VMI);
  out.label(kClassBase);
  emit_register_alu(out, 0x39, XR_RDI, XR_RSI);
  emit_condition_jump(out, XC_E, yes);
  emit_load(out, XR_RAX, XR_RDI, 0, 64);
  if(si_type.valid()) {
    emit_symbol_move(out, XR_RCX, si_type,
                     mir_model::MirOperand::ADDRESS_PREEMPTIBLE);
    emit_lea(out, XR_RCX, XR_RCX, 16);
    emit_register_alu(out, 0x39, XR_RAX, XR_RCX);
    emit_condition_jump(out, XC_NE, vmi);
    emit_load(out, XR_RDI, XR_RDI, 16, 64);
    emit_unconditional_jump(out, kClassBase);
  }
  out.label(vmi);
  if(vmi_type.valid()) {
    const Label loop = out.internal_label("eh_class_base_loop");
    const Label skip = out.internal_label("eh_class_base_skip");
    const Label found = out.internal_label("eh_class_base_found");
    const Label missing = out.internal_label("eh_class_base_missing");
    emit_symbol_move(out, XR_RCX, vmi_type,
                     mir_model::MirOperand::ADDRESS_PREEMPTIBLE);
    emit_lea(out, XR_RCX, XR_RCX, 16);
    emit_register_alu(out, 0x39, XR_RAX, XR_RCX);
    emit_condition_jump(out, XC_NE, no);
    emit_push(out, XR_RBX); emit_push(out, XR_R12); emit_push(out, XR_R13);
    emit_load(out, XR_RBX, XR_RDI, 20, 32);
    emit_lea(out, XR_R12, XR_RDI, 24);
    emit_register_move(out, XR_R13, XR_RSI);
    out.label(loop);
    emit_test_register(out, XR_RBX); emit_condition_jump(out, XC_E, missing);
    emit_load(out, XR_RCX, XR_R12, 8, 64);
    emit_test_immediate(out, XR_RCX, 2); emit_condition_jump(out, XC_E, skip);
    emit_load(out, XR_RDI, XR_R12, 0, 64);
    emit_register_move(out, XR_RSI, XR_R13);
    emit_call(out, kClassBase);
    emit_test_register(out, XR_RAX); emit_condition_jump(out, XC_NE, found);
    out.label(skip);
    emit_immediate_alu(out, XR_R12, 0, 16);
    emit_immediate_alu(out, XR_RBX, 5, 1);
    emit_unconditional_jump(out, loop);
    out.label(found);
    emit_pop(out, XR_R13); emit_pop(out, XR_R12); emit_pop(out, XR_RBX);
    emit_unconditional_jump(out, yes);
    out.label(missing);
    emit_pop(out, XR_R13); emit_pop(out, XR_R12); emit_pop(out, XR_RBX);
  }
  out.label(no);
  emit_immediate_move(out, XR_RAX, 0);
  out.byte(0xc3);
  out.label(yes);
  emit_immediate_move(out, XR_RAX, 1);
  out.byte(0xc3);
}

// rbx: object or pointer, r12: thrown class, r13: handler class.
void emit_upcast(CodeBuffer & out, Label yes, Label no)
{
  emit_register_move(out, XR_RDI, XR_RBX);
  emit_register_move(out, XR_RSI, XR_R12);
  emit_register_move(out, XR_RDX, XR_R13);
  emit_call(out, kDynamicCastFind);
  emit_test_register(out, XR_RAX); emit_condition_jump(out, XC_E, no);
  emit_immediate_move(out, XR_RCX, UINT64_MAX);
  emit_register_alu(out, 0x39, XR_RAX, XR_RCX);
  emit_condition_jump(out, XC_E, no);
  emit_register_move(out, XR_RBX, XR_RAX);
  emit_unconditional_jump(out, yes);
}

}  // namespace

// Follows libsupc++'s __pbase_type_info::__do_catch. r14 is 1 for a pointer
// handler (rbx then holds the thrown pointer), r15 the outer-level state:
// bit 0 while every outer level is const, plus 2 per pointer level.
void emit_catch_matcher(CodeBuffer & out,
                        const std::vector<mir_model::MirRuntimeData> & data)
{
  const lowir_model::SymbolId pointer_type =
    runtime_data_symbol(data, mir_model::RuntimeData::RD_RTTI_POINTER);
  const Label yes = out.internal_label("eh_match_yes");
  const Label no = out.internal_label("eh_match_no");
  const Label done = out.internal_label("eh_match_done");
  const Label top_object = out.internal_label("eh_match_top_object");
  const Label not_null = out.internal_label("eh_match_not_null");
  const Label null_member = out.internal_label("eh_match_null_member");
  const Label member_function = out.internal_label("eh_match_member_function");
  const Label level = out.internal_label("eh_match_level");
  const Label member_level = out.internal_label("eh_match_member_level");
  const Label kind_known = out.internal_label("eh_match_kind_known");
  const Label const_level = out.internal_label("eh_match_const_level");
  const Label no_context = out.internal_label("eh_match_no_context");
  const Label no_void = out.internal_label("eh_match_no_void");
  const Label null_class = out.internal_label("eh_match_null_class");
  const Label null_data = out.internal_label("eh_match_null_data");
  const Label null_function = out.internal_label("eh_match_null_function");
  out.label(kCatchMatcher);
  emit_push(out, XR_RBX); emit_push(out, XR_R12); emit_push(out, XR_R13);
  emit_push(out, XR_R14); emit_push(out, XR_R15);
  emit_register_move(out, XR_RBX, XR_RDX);
  emit_register_move(out, XR_R12, XR_RDI);
  emit_register_move(out, XR_R13, XR_RSI);
  emit_immediate_move(out, XR_R14, 0);
  emit_immediate_move(out, XR_R15, 1);
  // 15.3p3: a pointer is caught by value, so a pointer handler sees it.
  emit_require_pointer(out, XR_R13, pointer_type, top_object);
  emit_immediate_move(out, XR_R14, 1);
  emit_load(out, XR_RBX, XR_RBX, 0, 64);
  out.label(top_object);
  emit_register_alu(out, 0x39, XR_R12, XR_R13);
  emit_condition_jump(out, XC_E, yes);
  // A thrown std::nullptr_t matches any pointer or pointer-to-member handler.
  emit_require_name_byte(out, XR_R12, 0, 'D', not_null);
  emit_require_name_byte(out, XR_R12, 1, 'n', not_null);
  emit_require_name_byte(out, XR_R12, 2, 0, not_null);
  emit_test_register(out, XR_R14); emit_condition_jump(out, XC_E, null_member);
  emit_immediate_move(out, XR_RBX, 0);
  emit_unconditional_jump(out, yes);
  out.label(null_member);
  emit_require_name_byte(out, XR_R13, 0, 'M', no);
  emit_load(out, XR_RDX, XR_R13, 24, 64);
  emit_name_byte(out, XR_RCX, XR_RDX, 0);
  emit_compare_immediate(out, XR_RCX, 'F');
  emit_condition_jump(out, XC_E, member_function);
  emit_symbol_move(out, XR_RBX, null_data);
  emit_unconditional_jump(out, yes);
  out.label(member_function);
  emit_symbol_move(out, XR_RBX, null_function);
  emit_unconditional_jump(out, yes);
  out.label(not_null);
  // A class or other object handler: the exact type or a public base.
  emit_test_register(out, XR_R14); emit_condition_jump(out, XC_NE, level);
  emit_name_byte(out, XR_RCX, XR_R13, 0);
  emit_compare_immediate(out, XR_RCX, 'M');
  emit_condition_jump(out, XC_E, level);
  emit_upcast(out, yes, no);
  // One pointer level: equal kinds, no qualification lost, and cv added
  // below the top only where every outer level is const (4.4).
  out.label(level);
  emit_immediate_move(out, XR_R10, 1);
  emit_require_pointer(out, XR_R13, pointer_type, member_level);
  emit_require_pointer(out, XR_R12, pointer_type, no);
  emit_unconditional_jump(out, kind_known);
  out.label(member_level);
  emit_require_name_byte(out, XR_R13, 0, 'M', no);
  emit_require_name_byte(out, XR_R12, 0, 'M', no);
  emit_immediate_move(out, XR_R10, 0);
  out.label(kind_known);
  emit_test_immediate(out, XR_R15, 1); emit_condition_jump(out, XC_E, no);
  emit_load(out, XR_RAX, XR_R12, 16, 32);
  emit_load(out, XR_RCX, XR_R13, 16, 32);
  emit_immediate_alu(out, XR_RAX, 4, 7);
  emit_immediate_alu(out, XR_RCX, 4, 7);
  emit_register_alu(out, 0x09, XR_RAX, XR_RCX);
  emit_register_alu(out, 0x39, XR_RAX, XR_RCX);
  emit_condition_jump(out, XC_NE, no);
  emit_test_immediate(out, XR_RCX, 1); emit_condition_jump(out, XC_NE, const_level);
  emit_immediate_alu(out, XR_R15, 4, 0xfe);
  out.label(const_level);
  emit_test_register(out, XR_R10); emit_condition_jump(out, XC_NE, no_context);
  emit_load(out, XR_RAX, XR_R12, 32, 64);
  emit_load(out, XR_RCX, XR_R13, 32, 64);
  emit_register_alu(out, 0x39, XR_RAX, XR_RCX);
  emit_condition_jump(out, XC_NE, no);
  out.label(no_context);
  // 4.10p2: any object pointer converts to cv void* at the first level.
  emit_test_register(out, XR_R10); emit_condition_jump(out, XC_E, no_void);
  emit_compare_immediate(out, XR_R15, 2); emit_condition_jump(out, XC_AE, no_void);
  emit_load(out, XR_RDX, XR_R13, 24, 64);
  emit_require_name_byte(out, XR_RDX, 0, 'v', no_void);
  emit_require_name_byte(out, XR_RDX, 1, 0, no_void);
  emit_load(out, XR_RDX, XR_R12, 24, 64);
  emit_name_byte(out, XR_RCX, XR_RDX, 0);
  emit_compare_immediate(out, XR_RCX, 'F');
  emit_condition_jump(out, XC_E, no);
  emit_unconditional_jump(out, yes);
  out.label(no_void);
  emit_immediate_alu(out, XR_R15, 0, 2);
  emit_load(out, XR_R12, XR_R12, 24, 64);
  emit_load(out, XR_R13, XR_R13, 24, 64);
  emit_register_alu(out, 0x39, XR_R12, XR_R13);
  emit_condition_jump(out, XC_E, yes);
  // 4.10p3: at a pointer handler's first level, an unambiguous public base.
  emit_test_register(out, XR_R14); emit_condition_jump(out, XC_E, level);
  emit_compare_immediate(out, XR_R15, 4); emit_condition_jump(out, XC_AE, level);
  emit_name_byte(out, XR_RCX, XR_R13, 0);
  emit_compare_immediate(out, XR_RCX, 'P');
  emit_condition_jump(out, XC_E, level);
  emit_compare_immediate(out, XR_RCX, 'M');
  emit_condition_jump(out, XC_E, level);
  emit_test_register(out, XR_RBX); emit_condition_jump(out, XC_E, null_class);
  emit_upcast(out, yes, no);
  out.label(null_class);
  emit_register_move(out, XR_RDI, XR_R12);
  emit_register_move(out, XR_RSI, XR_R13);
  emit_call(out, kClassBase);
  emit_test_register(out, XR_RAX); emit_condition_jump(out, XC_E, no);
  out.label(yes);
  emit_register_move(out, XR_RAX, XR_RBX);
  emit_immediate_move(out, XR_RDX, 1);
  emit_unconditional_jump(out, done);
  out.label(no);
  emit_immediate_move(out, XR_RAX, 0);
  emit_immediate_move(out, XR_RDX, 0);
  out.label(done);
  emit_pop(out, XR_R15); emit_pop(out, XR_R14); emit_pop(out, XR_R13);
  emit_pop(out, XR_R12); emit_pop(out, XR_RBX);
  out.byte(0xc3);
  // Null pointers to members (2.3): a data member's is -1, a function's 0.
  out.align(8);
  out.label(null_data);
  out.little(UINT64_MAX, 8);
  out.label(null_function);
  out.zeros(16);
  emit_class_base(out, data);
}

void emit_runtime_data_placeholder(CodeBuffer & out,
                                   const std::string & object_name)
{
  if(object_name.compare(0, 4, "_ZTI") != 0) {
    out.zeros(32);
    return;
  }
  const Label name = out.internal_label("eh_type_info_name");
  out.zeros(8);
  out.absolute64(name);
  out.zeros(16);
  out.label(name);
  for(std::size_t i = 4; i < object_name.size(); ++i)
    out.byte(static_cast<unsigned char>(object_name[i]));
  out.byte(0);
}

}  // namespace eh
}  // namespace lowir_native
