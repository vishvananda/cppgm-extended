#include <cstddef>
#include <iostream>

#include "cpp_decl_model.h"
#include "semantic_model.h"
#include "template_model.h"

int main()
{
  std::cout << "Type " << sizeof(cpp_decl::Type) << '\n';
  std::cout << "TemplateArgument "
            << sizeof(template_model::TemplateArgument) << '\n';
  std::cout << "ClassInfo " << sizeof(semantic_model::ClassInfo) << '\n';
  std::cout << "OutOfClassStaticMemberDecl "
            << sizeof(semantic_model::OutOfClassStaticMemberDecl) << '\n';
  std::cout << "OutOfClassMemberFunctionDecl "
            << sizeof(semantic_model::OutOfClassMemberFunctionDecl) << '\n';
  return 0;
}
