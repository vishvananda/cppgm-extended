#pragma once

#include <cstddef>
#include <functional>
#include <map>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

#include "cpp_decl_model.h"
#include "cppast_ast.h"
#include "semantic_model.h"

namespace callsemantic {

struct TypeRegistryState
{
  std::vector<std::unique_ptr<semantic_model::ClassInfo> > & classes;
  semantic_model::ClassIndexMap & classes_by_key;
  std::size_t & classes_by_key_version;
  std::unordered_map<std::string, std::size_t> & classes_by_key_epochs;
  semantic_model::TypeScopeIndexMap & type_scopes_by_key;
  std::vector<semantic_model::ClassInfo *> & instantiated_classes;
};

struct TypeRegistryCallbacks
{
  std::function<cpp_decl::TypePtr(semantic_model::Scope &, const std::string &)>
      direct_named_type;
  std::function<semantic_model::Scope &(semantic_model::Scope &)>
      persistent_enclosing_scope;
  std::function<std::string(const CppAstNode *)> current_location_note;
  std::function<std::string(const char *,
                            const semantic_model::ClassInfo *)>
      previous_class_location_note;
};

semantic_model::ClassInfo * class_info_for_type(
    const TypeRegistryState & state,
    const cpp_decl::TypePtr & type);

const semantic_model::ClassIndexMap &
template_named_class_index(const TypeRegistryState & state);

semantic_model::Scope * scope_for_type(const TypeRegistryState & state,
                                       const cpp_decl::TypePtr & type);

bool is_initializer_list_type(const TypeRegistryState & state,
                              const cpp_decl::TypePtr & type,
                              cpp_decl::TypePtr * element_type = nullptr,
                              semantic_model::ClassInfo ** info_out = nullptr);

std::size_t stable_function_local_type_fingerprint(
    const semantic_model::Scope & current_scope,
    const std::string & local_name,
    const CppAstNode & node);

semantic_model::ClassInfo * create_class_info(
    TypeRegistryState & state,
    const TypeRegistryCallbacks & callbacks,
    semantic_model::Scope & scope,
    const std::string & class_kind,
    const std::string & name,
    const CppAstNode * class_node = nullptr);

semantic_model::ClassInfo * create_instantiated_class_info_with_internal_name(
    TypeRegistryState & state,
    const TypeRegistryCallbacks & callbacks,
    semantic_model::Scope & scope,
    const std::string & class_kind,
    const std::string & template_name,
    const std::string & specialization_name,
    const std::string & internal_specialization_name,
    semantic_model::ClassTemplateDecl * source_template,
    const CppAstNode * output_node,
    bool track_output = true);

semantic_model::ClassInfo * create_instantiated_class_info(
    TypeRegistryState & state,
    const TypeRegistryCallbacks & callbacks,
    semantic_model::Scope & scope,
    const std::string & class_kind,
    const std::string & template_name,
    const std::string & specialization_name,
    semantic_model::ClassTemplateDecl * source_template,
    const CppAstNode * output_node,
    bool track_output = true);

void track_instantiated_class(TypeRegistryState & state,
                              semantic_model::ClassInfo * info);

}  // namespace callsemantic
