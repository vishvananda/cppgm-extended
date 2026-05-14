#include "template_function_deduction_api.h"

#include "template_api_internal.h"

namespace template_api {

bool deduce_function_template(
    SemanticContext & ctx,
    const TemplateFunctionDeductionRequest & request,
    TemplateFunctionDeductionResult & out)
{
  out.arguments.clear();
  out.pack_sizes.clear();
  if(!request.decl) {
    return false;
  }

  if(request.target_type && request.explicit_arguments) {
    if(!request.resolution_scope) {
      return false;
    }
    return deduce_function_template_arguments_from_target_type_with_explicit(
        ctx,
        *request.decl,
        *request.resolution_scope,
        *request.explicit_arguments,
        request.target_type,
        out.arguments,
        &out.pack_sizes);
  }

  if(request.target_type) {
    return deduce_function_template_arguments_from_target_type(ctx,
                                                               *request.decl,
                                                               request.target_type,
                                                               out.arguments,
                                                               request.use_scope,
                                                               &out.pack_sizes);
  }

  if(request.explicit_arguments) {
    if(!request.args || !request.resolution_scope) {
      return false;
    }
    return deduce_function_template_arguments_with_explicit(ctx,
                                                            *request.decl,
                                                            *request.resolution_scope,
                                                            *request.explicit_arguments,
                                                            *request.args,
                                                            out.arguments,
                                                            &out.pack_sizes);
  }

  if(!request.args) {
    return false;
  }
  return deduce_function_template_arguments(
      ctx, *request.decl, *request.args, out.arguments, request.use_scope, &out.pack_sizes);
}

}  // namespace template_api
