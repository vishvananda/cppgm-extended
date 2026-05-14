#pragma once

#include <string>

namespace runtime_symbol_policy {

enum class RuntimeSymbolRole
{
  none,
  init,
  fini,
  eh_top,
  eh_value,
  eh_type,
  eh_unhandled,
  eh_allocate_exception,
  eh_begin_catch,
  eh_call_unexpected,
  eh_current_exception_type,
  eh_end_catch,
  eh_rethrow,
  eh_throw,
  eh_personality,
  eh_resume,
  builtin_operator_new,
  builtin_operator_new_aligned,
  builtin_operator_new_array,
  builtin_operator_new_array_aligned,
  builtin_operator_delete,
  builtin_operator_delete_sized,
  builtin_operator_delete_aligned,
  builtin_operator_delete_sized_aligned,
  builtin_operator_delete_array,
  builtin_operator_delete_array_sized,
  builtin_operator_delete_array_aligned,
  builtin_operator_delete_array_sized_aligned,
  builtin_memchr,
  builtin_memcmp,
  builtin_memcpy,
  builtin_memmove,
  builtin_strcmp,
  builtin_strchr,
  builtin_strlen,
  builtin_ceil,
  builtin_ceilf,
  builtin_ceill,
  builtin_fabs,
  builtin_fabsf,
  builtin_fabsl,
  builtin_inf,
  builtin_inff,
  builtin_infl,
  builtin_nans,
  builtin_nansf,
  builtin_nansl,
  builtin_flt_rounds,
  builtin_is_constant_evaluated,
  builtin_isfinite,
  builtin_isfinitef,
  builtin_isfinitel,
  builtin_isinf,
  builtin_isinff,
  builtin_isinfl,
  builtin_isnan,
  builtin_isnanf,
  builtin_isnanl,
  builtin_isnormal,
  builtin_isnormalf,
  builtin_isnormall,
  builtin_expect,
  builtin_unreachable
};

enum class RuntimeSymbolMigrationPolicy
{
  ordinary_symbol,
  private_runtime,
  host_abi,
  host_eh_runtime,
  host_libcall,
  reserved_internal
};

struct RuntimeSymbolInfo
{
  RuntimeSymbolRole role = RuntimeSymbolRole::none;
  RuntimeSymbolMigrationPolicy policy = RuntimeSymbolMigrationPolicy::ordinary_symbol;
  const char * object_symbol_alias = nullptr;
};

RuntimeSymbolInfo classify(const std::string & name);
std::string normalize_lookup_name(const std::string & name);
bool is_host_eh_runtime_role(RuntimeSymbolRole role);
bool is_host_eh_runtime_symbol(const std::string & name);
bool is_reserved_internal_symbol(const std::string & name);
std::string object_symbol_alias(const std::string & name);

}  // namespace runtime_symbol_policy
