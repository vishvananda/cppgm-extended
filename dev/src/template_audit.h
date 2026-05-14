#pragma once

#include <string>

namespace semantic_model {
struct ClassInfo;
}

namespace template_audit {

bool enabled();
void set_creation_context(semantic_model::ClassInfo & info,
                          const std::string & fallback_context);
void log_upgrade(const semantic_model::ClassInfo & info,
                 const std::string & upgrade_context);
void log_upgrade_fail(const std::string & type_description,
                      const semantic_model::ClassInfo * info,
                      const std::string & failure_context);

}  // namespace template_audit
