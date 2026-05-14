#include "runtime_symbol_policy.h"

namespace runtime_symbol_policy {

namespace {

struct RuntimeSymbolTableEntry
{
  const char * name;
  RuntimeSymbolRole role;
  RuntimeSymbolMigrationPolicy policy;
  const char * object_symbol_alias;
};

#define RUNTIME_SYMBOL_TABLE(X) \
  X("@__cppgm_init", init, reserved_internal, nullptr) \
  X("@__cppgm_fini", fini, reserved_internal, nullptr) \
  X("@__cppgm_eh_top", eh_top, reserved_internal, "cppgm_priv_exc_top") \
  X("@__cppgm_eh_value", eh_value, reserved_internal, "cppgm_priv_exc_value") \
  X("@__cppgm_eh_type", eh_type, reserved_internal, "cppgm_priv_exc_type") \
  X("@__cppgm_eh_unhandled", eh_unhandled, reserved_internal, "cppgm_priv_exc_unhandled") \
  X("__cxa_allocate_exception", eh_allocate_exception, host_eh_runtime, nullptr) \
  X("__cxa_begin_catch", eh_begin_catch, host_eh_runtime, nullptr) \
  X("__cxa_call_unexpected", eh_call_unexpected, host_eh_runtime, nullptr) \
  X("__cxa_current_exception_type", eh_current_exception_type, host_eh_runtime, nullptr) \
  X("__cxa_end_catch", eh_end_catch, host_eh_runtime, nullptr) \
  X("__cxa_rethrow", eh_rethrow, host_eh_runtime, nullptr) \
  X("__cxa_throw", eh_throw, host_eh_runtime, nullptr) \
  X("__gxx_personality_v0", eh_personality, host_eh_runtime, nullptr) \
  X("_Unwind_Resume", eh_resume, host_eh_runtime, nullptr) \
  X("__emutls_get_address", none, host_abi, nullptr) \
  X("__tlv_bootstrap", none, host_abi, "_tlv_bootstrap") \
  X("__tlv_atexit", none, host_abi, "_tlv_atexit") \
  X("___dso_handle", none, host_abi, "__dso_handle") \
  X("cppgm_builtin_operator_new", builtin_operator_new, host_abi, "_Znwm") \
  X("cppgm_builtin_operator_new_aligned", builtin_operator_new_aligned, host_abi, "_ZnwmSt11align_val_t") \
  X("cppgm_builtin_operator_new_array", builtin_operator_new_array, host_abi, "_Znam") \
  X("cppgm_builtin_operator_new_array_aligned", builtin_operator_new_array_aligned, host_abi, "_ZnamSt11align_val_t") \
  X("cppgm_builtin_operator_delete", builtin_operator_delete, host_abi, "_ZdlPv") \
  X("cppgm_builtin_operator_delete_sized", builtin_operator_delete_sized, host_abi, "_ZdlPvm") \
  X("cppgm_builtin_operator_delete_aligned", builtin_operator_delete_aligned, host_abi, "_ZdlPvSt11align_val_t") \
  X("cppgm_builtin_operator_delete_sized_aligned", builtin_operator_delete_sized_aligned, host_abi, "_ZdlPvmSt11align_val_t") \
  X("cppgm_builtin_operator_delete_array", builtin_operator_delete_array, host_abi, "_ZdaPv") \
  X("cppgm_builtin_operator_delete_array_sized", builtin_operator_delete_array_sized, host_abi, "_ZdaPvm") \
  X("cppgm_builtin_operator_delete_array_aligned", builtin_operator_delete_array_aligned, host_abi, "_ZdaPvSt11align_val_t") \
  X("cppgm_builtin_operator_delete_array_sized_aligned", builtin_operator_delete_array_sized_aligned, host_abi, "_ZdaPvmSt11align_val_t") \
  X("cppgm_builtin_memchr", builtin_memchr, host_libcall, "memchr") \
  X("cppgm_builtin_memcmp", builtin_memcmp, host_libcall, "memcmp") \
  X("cppgm_builtin_memcpy", builtin_memcpy, host_libcall, "memcpy") \
  X("cppgm_builtin_memmove", builtin_memmove, host_libcall, "memmove") \
  X("cppgm_builtin_strcmp", builtin_strcmp, host_libcall, "strcmp") \
  X("cppgm_builtin_strchr", builtin_strchr, host_libcall, "strchr") \
  X("cppgm_builtin_strlen", builtin_strlen, host_libcall, "strlen") \
  X("cppgm_builtin_ceil", builtin_ceil, host_libcall, "ceil") \
  X("cppgm_builtin_ceilf", builtin_ceilf, host_libcall, "ceilf") \
  X("cppgm_builtin_ceill", builtin_ceill, host_libcall, "ceill") \
  X("cppgm_builtin_fabs", builtin_fabs, host_libcall, "fabs") \
  X("cppgm_builtin_fabsf", builtin_fabsf, host_libcall, "fabsf") \
  X("cppgm_builtin_fabsl", builtin_fabsl, host_libcall, "fabsl") \
  X("cppgm_builtin_i128_shl", none, host_libcall, "__ashlti3") \
  X("cppgm_builtin_i128_sar", none, host_libcall, "__ashrti3") \
  X("cppgm_builtin_i128_div", none, host_libcall, "__divti3") \
  X("cppgm_builtin_u128_shr", none, host_libcall, "__lshrti3") \
  X("cppgm_builtin_i128_mod", none, host_libcall, "__modti3") \
  X("cppgm_builtin_i128_mul", none, host_libcall, "__multi3") \
  X("cppgm_builtin_u128_div", none, host_libcall, "__udivti3") \
  X("cppgm_builtin_u128_mod", none, host_libcall, "__umodti3") \
  X("cppgm_builtin_inf", builtin_inf, host_libcall, nullptr) \
  X("cppgm_builtin_inff", builtin_inff, host_libcall, nullptr) \
  X("cppgm_builtin_infl", builtin_infl, host_libcall, nullptr) \
  X("cppgm_builtin_nans", builtin_nans, private_runtime, nullptr) \
  X("cppgm_builtin_nansf", builtin_nansf, private_runtime, nullptr) \
  X("cppgm_builtin_nansl", builtin_nansl, private_runtime, nullptr) \
  X("cppgm_builtin_flt_rounds", builtin_flt_rounds, private_runtime, nullptr) \
  X("cppgm_builtin_is_constant_evaluated", builtin_is_constant_evaluated, private_runtime, nullptr) \
  X("cppgm_builtin_isfinite", builtin_isfinite, private_runtime, nullptr) \
  X("cppgm_builtin_isfinitef", builtin_isfinitef, private_runtime, nullptr) \
  X("cppgm_builtin_isfinitel", builtin_isfinitel, private_runtime, nullptr) \
  X("cppgm_builtin_isinf", builtin_isinf, private_runtime, nullptr) \
  X("cppgm_builtin_isinff", builtin_isinff, private_runtime, nullptr) \
  X("cppgm_builtin_isinfl", builtin_isinfl, private_runtime, nullptr) \
  X("cppgm_builtin_isnan", builtin_isnan, private_runtime, nullptr) \
  X("cppgm_builtin_isnanf", builtin_isnanf, private_runtime, nullptr) \
  X("cppgm_builtin_isnanl", builtin_isnanl, private_runtime, nullptr) \
  X("cppgm_builtin_isnormal", builtin_isnormal, private_runtime, nullptr) \
  X("cppgm_builtin_isnormalf", builtin_isnormalf, private_runtime, nullptr) \
  X("cppgm_builtin_isnormall", builtin_isnormall, private_runtime, nullptr) \
  X("cppgm_builtin_expect", builtin_expect, private_runtime, nullptr) \
  X("cppgm_builtin_unreachable", builtin_unreachable, host_libcall, "abort") \
  X("cppgm_host_num_put_char_put_bool", none, host_libcall, "cppgm_host_num_put_char_put_bool") \
  X("cppgm_host_num_put_char_put_long", none, host_libcall, "cppgm_host_num_put_char_put_long") \
  X("cppgm_host_num_put_char_put_long_long", none, host_libcall, "cppgm_host_num_put_char_put_long_long") \
  X("cppgm_host_num_put_char_put_unsigned_long", none, host_libcall, "cppgm_host_num_put_char_put_unsigned_long") \
  X("cppgm_host_num_put_char_put_unsigned_long_long", none, host_libcall, "cppgm_host_num_put_char_put_unsigned_long_long") \
  X("cppgm_host_num_put_char_put_double", none, host_libcall, "cppgm_host_num_put_char_put_double") \
  X("cppgm_host_num_put_char_put_long_double", none, host_libcall, "cppgm_host_num_put_char_put_long_double") \
  X("cppgm_host_num_put_char_put_ptr", none, host_libcall, "cppgm_host_num_put_char_put_ptr")

#define RUNTIME_SYMBOL_ENTRY(name, role, policy, alias) \
  {name, RuntimeSymbolRole::role, RuntimeSymbolMigrationPolicy::policy, alias},
const RuntimeSymbolTableEntry kRuntimeSymbolTable[] = {
  RUNTIME_SYMBOL_TABLE(RUNTIME_SYMBOL_ENTRY)
};
#undef RUNTIME_SYMBOL_ENTRY

std::string normalize_runtime_lookup_name(std::string name)
{
  if(!name.empty() && name[0] == '@') {
    name.erase(0, 1);
  }
  const std::string external_runtime_internal_prefix = "__external_runtime__";
  const std::string external_runtime_text_prefix = "__external_runtime::";
  if(name.compare(0,
                  external_runtime_internal_prefix.size(),
                  external_runtime_internal_prefix) == 0) {
    name.erase(0, external_runtime_internal_prefix.size());
  }
  if(name.compare(0,
                  external_runtime_text_prefix.size(),
                  external_runtime_text_prefix) == 0) {
    name.erase(0, external_runtime_text_prefix.size());
  }
  if(name.size() > 1 && name[0] == '_' && name.compare(1, 6, "cppgm_") == 0) {
    name.erase(0, 1);
  }
  return name;
}

}  // namespace

RuntimeSymbolInfo classify(const std::string & name)
{
  const std::string normalized_name = normalize_runtime_lookup_name(name);
  for(size_t i = 0; i < sizeof(kRuntimeSymbolTable) / sizeof(kRuntimeSymbolTable[0]); ++i) {
    if(name == kRuntimeSymbolTable[i].name ||
        (kRuntimeSymbolTable[i].object_symbol_alias &&
        name == kRuntimeSymbolTable[i].object_symbol_alias) ||
       normalized_name == normalize_runtime_lookup_name(kRuntimeSymbolTable[i].name) ||
       (kRuntimeSymbolTable[i].object_symbol_alias &&
        normalized_name ==
            normalize_runtime_lookup_name(kRuntimeSymbolTable[i].object_symbol_alias))) {
      RuntimeSymbolInfo out;
      out.role = kRuntimeSymbolTable[i].role;
      out.policy = kRuntimeSymbolTable[i].policy;
      out.object_symbol_alias = kRuntimeSymbolTable[i].object_symbol_alias;
      return out;
    }
  }
  return RuntimeSymbolInfo();
}

std::string normalize_lookup_name(const std::string & name)
{
  return normalize_runtime_lookup_name(name);
}

bool is_host_eh_runtime_role(RuntimeSymbolRole role)
{
  return role == RuntimeSymbolRole::eh_allocate_exception ||
         role == RuntimeSymbolRole::eh_begin_catch ||
         role == RuntimeSymbolRole::eh_call_unexpected ||
         role == RuntimeSymbolRole::eh_current_exception_type ||
         role == RuntimeSymbolRole::eh_end_catch ||
         role == RuntimeSymbolRole::eh_rethrow ||
         role == RuntimeSymbolRole::eh_throw ||
         role == RuntimeSymbolRole::eh_personality ||
         role == RuntimeSymbolRole::eh_resume;
}

bool is_host_eh_runtime_symbol(const std::string & name)
{
  return is_host_eh_runtime_role(classify(name).role);
}

bool is_reserved_internal_symbol(const std::string & name)
{
  return classify(name).policy == RuntimeSymbolMigrationPolicy::reserved_internal;
}

std::string object_symbol_alias(const std::string & name)
{
  RuntimeSymbolInfo info = classify(name);
  return info.object_symbol_alias ? info.object_symbol_alias : std::string();
}

}  // namespace runtime_symbol_policy
