#pragma once

#include "cpp_decl_ast.h"
#include "semantic_context.h"

namespace callsemantic_phase_bridge {

cpp_decl::AstDeclHooks make_decl_hooks(SemanticContext & ctx,
                                       semantic_model::Scope & scope,
                                       bool reference_class_templates_only = false);

}  // namespace callsemantic_phase_bridge
