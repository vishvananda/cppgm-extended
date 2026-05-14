#include "semantic_fallback_audit.h"

namespace semantic_fallback_audit {

void hard_fail(const char * category,
               const std::string & location,
               const std::string & detail)
{
  std::string message = "semantic fallback reached";
  message += " [category ";
  message += category ? category : "<unknown>";
  message += "]";
  if(!location.empty()) {
    message += " [location ";
    message += location;
    message += "]";
  }
  if(!detail.empty()) {
    message += " ";
    message += detail;
  }
  throw SemanticFallbackError(message);
}

void rethrow_fallback_error()
{
  try {
    throw;
  } catch(const SemanticFallbackError &) {
    throw;
  } catch(...) {
  }
}

}  // namespace semantic_fallback_audit
