#include <cstddef>
#include <iostream>

#include "cpp_decl_model.h"
#include "resolved_source_semantics.h"
#include "semantic_model.h"
#include "template_instantiation_api.h"
#include "template_model.h"

int main()
{
  std::cout << "Type " << sizeof(cpp_decl::Type) << '\n';
  std::cout << "TemplateArgument "
            << sizeof(template_model::TemplateArgument) << '\n';
  std::cout << "ClassInfo " << sizeof(semantic_model::ClassInfo) << '\n';
  std::cout << "FunctionBinding "
            << sizeof(semantic_model::FunctionBinding) << '\n';
  std::cout << "ValueBinding " << sizeof(semantic_model::ValueBinding) << '\n';
  std::cout << "AliasTemplateDecl "
            << sizeof(semantic_model::AliasTemplateDecl) << '\n';
  std::cout << "TemplateLifecycleTransition "
            << sizeof(template_api::TemplateLifecycleTransition) << '\n';
  std::cout << "OutOfClassStaticMemberDecl "
            << sizeof(semantic_model::OutOfClassStaticMemberDecl) << '\n';
  std::cout << "OutOfClassMemberFunctionDecl "
            << sizeof(semantic_model::OutOfClassMemberFunctionDecl) << '\n';
  std::cout << "ResolvedClassTemplateIdView "
            << sizeof(resolved_source_semantics::ResolvedClassTemplateIdView)
            << '\n';
  std::cout << "ResolvedAliasTemplateId "
            << sizeof(resolved_source_semantics::ResolvedAliasTemplateId)
            << '\n';
  std::cout << "ResolvedQualifiedId "
            << sizeof(resolved_source_semantics::ResolvedQualifiedId)
            << '\n';
  std::cout << "ResolvedOwnerReference "
            << sizeof(resolved_source_semantics::ResolvedOwnerReference)
            << '\n';
  std::cout << "RetainedAliasClassUse "
            << sizeof(resolved_source_semantics::RetainedAliasClassUse)
            << '\n';
  return 0;
}
