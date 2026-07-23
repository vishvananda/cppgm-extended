#include "template_audit.h"

#include <cstdlib>
#include <iostream>

#include "semantic_model.h"
#include "semantic_trace.h"

namespace template_audit {

using semantic_model::ClassInfo;

namespace {

std::string effective_context(const std::string & fallback_context)
{
  const std::string frame = DiagnosticContext::current_frame();
  return frame.empty() ? fallback_context : frame;
}

}  // namespace

bool enabled()
{
  const char * value = std::getenv("CPPGM_TEMPLATE_AUDIT");
  return value && *value && std::string(value) != "0";
}

void set_creation_context(ClassInfo & info, const std::string & fallback_context)
{
  if(!enabled()) {
    return;
  }
  if(info.creation_context.empty()) {
    info.creation_context = effective_context(fallback_context);
  }
}

void log_upgrade(const ClassInfo & info, const std::string & upgrade_context)
{
  if(!enabled()) {
    return;
  }
  std::cerr << "UPGRADE: "
            << (info.qualified_name.empty() ? std::string("<unnamed-class>") : info.qualified_name)
            << " created_by="
            << (info.creation_context.empty() ? std::string("<unknown>") : info.creation_context)
            << " upgraded_at="
            << effective_context(upgrade_context)
            << '\n';
}

void log_upgrade_fail(const std::string & type_description,
                      const ClassInfo * info,
                      const std::string & failure_context)
{
  if(!enabled()) {
    return;
  }
  std::cerr << "UPGRADE_FAIL: " << type_description
            << " at=" << effective_context(failure_context);
  if(info) {
    std::cerr << " created_by="
              << (info->creation_context.empty() ? std::string("<unknown>")
                                                 : info->creation_context)
              << " qualified="
              << (info->qualified_name.empty() ? std::string("<unnamed-class>")
                                               : info->qualified_name)
              << " complete=" << (info->complete ? "yes" : "no")
              << " ref_members=" << (info->reference_members_collected ? "yes" : "no")
              << " in_progress=" << (info->template_instantiation_in_progress ? "yes" : "no");
  }
  std::cerr << '\n';
}

}  // namespace template_audit
