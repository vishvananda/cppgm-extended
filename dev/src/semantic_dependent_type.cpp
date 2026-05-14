#include "semantic_dependent_type.h"

#include "semantic_context.h"
#include "template_api.h"

namespace semantic_dependent_type {

bool resolve_instantiated_dependent_type(SemanticContext & ctx,
                                         semantic_model::Scope & scope,
                                         const cpp_decl::TypePtr & type,
                                         cpp_decl::TypePtr & out)
{
  return template_api::type::resolve_instantiated_dependent_type(ctx, scope, type, out);
}

std::string lookup_type_argument_text(SemanticContext & ctx,
                                      const cpp_decl::TypePtr & type)
{
  return template_api::type::lookup_text_for_type_argument(ctx, type);
}

}  // namespace semantic_dependent_type
