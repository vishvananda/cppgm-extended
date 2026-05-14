#include "eh_runtime.h"

#include <cstdlib>

extern "C" {
void * cppgm_priv_exc_top = nullptr;
void * cppgm_priv_exc_value = nullptr;
void * cppgm_priv_exc_type = nullptr;
void cppgm_priv_exc_unhandled(long long)
{
  std::abort();
}
}

namespace eh_runtime {

const char kEhTopSymbol[] = "@__cppgm_eh_top";
const char kEhValueSymbol[] = "@__cppgm_eh_value";
const char kEhTypeSymbol[] = "@__cppgm_eh_type";
const char kEhUnhandledSymbol[] = "@__cppgm_eh_unhandled";
const char kPureVirtualSymbol[] = "@__cxa_pure_virtual";
const char kEhTopObjectSymbol[] = "cppgm_priv_exc_top";
const char kEhValueObjectSymbol[] = "cppgm_priv_exc_value";
const char kEhTypeObjectSymbol[] = "cppgm_priv_exc_type";
const char kEhUnhandledObjectSymbol[] = "cppgm_priv_exc_unhandled";

bool is_reserved_symbol(const std::string & name)
{
  return name == kEhTopSymbol ||
         name == kEhTopObjectSymbol ||
         name == kEhValueSymbol ||
         name == kEhValueObjectSymbol ||
         name == kEhTypeSymbol ||
         name == kEhTypeObjectSymbol ||
         name == kEhUnhandledSymbol ||
         name == kEhUnhandledObjectSymbol ||
         name == kPureVirtualSymbol;
}

}  // namespace eh_runtime
