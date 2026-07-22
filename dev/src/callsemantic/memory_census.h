#pragma once

#include <iosfwd>
#include <memory>
#include <vector>

#include "callsem_output.h"
#include "cppast_ast.h"
#include "semantic_cache.h"
#include "semantic_model.h"

namespace callsemantic {

struct MemoryCensusInput
{
  const semantic_model::Scope * root;
  const std::vector<std::unique_ptr<semantic_model::FunctionBinding> > & functions;
  const std::vector<std::unique_ptr<semantic_model::ClassInfo> > & classes;
  const std::vector<std::unique_ptr<semantic_model::FunctionTemplateDecl> > &
      function_templates;
  const std::vector<std::unique_ptr<semantic_model::ClassTemplateDecl> > & class_templates;
  const std::vector<std::unique_ptr<semantic_model::AliasTemplateDecl> > & alias_templates;
  const std::vector<std::unique_ptr<semantic_model::VariableTemplateDecl> > &
      variable_templates;
  const std::vector<std::unique_ptr<semantic_model::Scope> > & template_scopes;
  const std::vector<std::unique_ptr<semantic_model::Scope> > & captured_local_scopes;
  const std::vector<std::unique_ptr<semantic_model::Scope> > & durable_type_scopes;
  const std::vector<std::unique_ptr<CppAstNode> > & synthetic_ast_nodes;
  const std::vector<semantic_model::FunctionBinding *> & instantiated_functions;
  const std::vector<semantic_model::ClassInfo *> & instantiated_classes;
  const std::vector<semantic_model::FunctionBinding *> & required_function_definitions;
  const std::vector<semantic_model::FunctionBinding *> & late_required_class_methods;
  const std::vector<semantic_model::FunctionBinding *> &
      late_required_class_static_functions;
  const std::vector<semantic_model::FunctionBinding *> & synthetic_functions;
  const std::vector<semantic_model::FunctionBinding *> & deferred_constexpr_functions;
  const semantic_cache::SemanticCache & cache_state;
  const CallSemNode & translation_unit;
};

void dump_memory_census(std::ostream & out, const MemoryCensusInput & input);
void dump_source_ast_memory_census(std::ostream & out,
                                   const CppAstNode & source_ast);
void dump_callsem_duplicate_hash_census(std::ostream & out,
                                        const MemoryCensusInput & input);
void dump_callsem_provenance_census(std::ostream & out,
                                    const MemoryCensusInput & input);
void dump_callsem_retained_kind_census(std::ostream & out,
                                       const MemoryCensusInput & input);

}  // namespace callsemantic
