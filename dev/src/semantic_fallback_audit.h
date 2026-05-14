#pragma once

#include <stdexcept>
#include <string>

namespace semantic_fallback_audit {

struct SemanticFallbackError : std::runtime_error
{
  explicit SemanticFallbackError(const std::string & message)
    : std::runtime_error(message)
  {}
};

void hard_fail(const char * category,
               const std::string & location,
               const std::string & detail);

void rethrow_fallback_error();

}  // namespace semantic_fallback_audit
