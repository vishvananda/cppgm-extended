#include "callsemantic/template_body_checks.h"

#include "callsemantic_internal.h"
#include "callsemantic/template_source_utils.h"
#include "cpp_decl_ast.h"
#include "semantic_context.h"
#include "semantic_declaration.h"
#include "semantic_errors.h"
#include "semantic_fallback_audit.h"
#include "semantic_lookup.h"
#include "semantic_utils.h"

#include <unordered_map>
#include <unordered_set>

namespace callsemantic {

using callsemantic_internal::is_identifier_text;
using callsemantic_internal::find_child_kind;
using cpp_decl::QualifiedName;
using cpp_decl::TypePtr;
using cpp_decl::TemplateArgumentSyntax;
using cpp_decl::TemplateIdSyntax;
using semantic_model::ClassInfo;
using semantic_model::FunctionBinding;
using semantic_model::FunctionTemplateDecl;
using semantic_model::Scope;
using semantic_model::ValueBinding;
using semantic_utils::strip_trailing_top_level_template_arguments;
using semantic_utils::trim_space;
using template_model::TemplateParameterInfo;

class AtomNameSet
{
public:
  void insert(const std::string & name)
  {
    if(!name.empty()) {
      names.insert(text_intern::intern(name));
    }
  }

  bool contains(const std::string & name) const
  {
    if(name.empty()) {
      return false;
    }
    text_intern::Atom atom = text_intern::find(name);
    return atom && names.count(atom) != 0;
  }

  bool empty() const
  {
    return names.empty();
  }

private:
  std::unordered_set<text_intern::Atom> names;
};

typedef std::unordered_map<text_intern::Atom, TypePtr> TemplateBodyValueTypes;

static void record_template_body_value_type(TemplateBodyValueTypes & value_types,
                                            const std::string & name,
                                            const TypePtr & type)
{
  if(!name.empty()) {
    value_types[text_intern::intern(name)] = type;
  }
}

static TypePtr lookup_template_body_value_type(
    const TemplateBodyValueTypes & value_types,
    const std::string & name)
{
  text_intern::Atom atom = text_intern::find(name);
  if(!atom) {
    return TypePtr();
  }
  TemplateBodyValueTypes::const_iterator found = value_types.find(atom);
  return found == value_types.end() ? TypePtr() : found->second;
}

bool declarator_declared_identifier(const CppAstNode & node,
                                    std::string & out)
{
  if(node.kind == CppAstKind::identifier) {
    out = node.value;
    return !out.empty();
  }
  for(std::size_t i = 0; i < node.children.size(); ++i) {
    if(declarator_declared_identifier(node.children[i], out)) {
      return true;
    }
  }
  return false;
}

std::set<std::string> template_parameter_names(
    const std::vector<TemplateParameterInfo> & parameters)
{
  std::set<std::string> names;
  for(std::size_t i = 0; i < parameters.size(); ++i) {
    if(!parameters[i].name.empty()) {
      names.insert(parameters[i].name);
    }
  }
  return names;
}

static AtomNameSet template_parameter_atom_names(
    const std::vector<TemplateParameterInfo> & parameters)
{
  AtomNameSet names;
  for(std::size_t i = 0; i < parameters.size(); ++i) {
    names.insert(parameters[i].name);
  }
  return names;
}

static AtomNameSet non_type_template_parameter_names(
    const std::vector<TemplateParameterInfo> & parameters)
{
  AtomNameSet names;
  for(std::size_t i = 0; i < parameters.size(); ++i) {
    if(parameters[i].kind == TemplateParameterInfo::TP_NON_TYPE &&
       !parameters[i].name.empty()) {
      names.insert(parameters[i].name);
    }
  }
  return names;
}

static AtomNameSet type_template_parameter_names(
    const std::vector<TemplateParameterInfo> & parameters)
{
  AtomNameSet names;
  for(std::size_t i = 0; i < parameters.size(); ++i) {
    if(parameters[i].kind == TemplateParameterInfo::TP_TYPE &&
       !parameters[i].name.empty()) {
      names.insert(parameters[i].name);
    }
  }
  return names;
}

static void collect_declared_names_for_template_body(
    const CppAstNode & node,
    AtomNameSet & declared_names)
{
  if(node.kind == CppAstKind::declarator && node.value.empty()) {
    const CppAstNode * identifier = find_child_kind(node, CppAstKind::identifier);
    if(identifier && !identifier->value.empty()) {
      declared_names.insert(identifier->value);
    }
  }

  for(std::size_t i = 0; i < node.children.size(); ++i) {
    collect_declared_names_for_template_body(node.children[i], declared_names);
  }
}

static bool subtree_has_template_id_syntax(const CppAstNode & node)
{
  if(const TemplateIdSyntax * template_id = cppast_template_id_syntax(node)) {
    if(!template_id->arguments.empty() ||
       !template_id->argument_syntaxes.empty()) {
      return true;
    }
  }
  for(std::size_t i = 0; i < node.qualifier_template_id_syntaxes.size(); ++i) {
    const TemplateIdSyntax & template_id = node.qualifier_template_id_syntaxes[i];
    if(!template_id.arguments.empty() ||
       !template_id.argument_syntaxes.empty()) {
      return true;
    }
  }
  for(std::size_t i = 0; i < node.children.size(); ++i) {
    if(subtree_has_template_id_syntax(node.children[i])) {
      return true;
    }
  }
  return false;
}

static bool qualified_name_mentions_template_parameter(
    const QualifiedName & name,
    const AtomNameSet & parameter_names)
{
  if(parameter_names.contains(name.name)) {
    return true;
  }
  for(std::size_t i = 0; i < name.qualifiers.size(); ++i) {
    if(parameter_names.contains(name.qualifiers[i])) {
      return true;
    }
  }
  return false;
}

static bool qualified_name_mentions_atom_name(const QualifiedName & name,
                                              const AtomNameSet & names)
{
  if(names.contains(name.name)) {
    return true;
  }
  for(std::size_t i = 0; i < name.qualifiers.size(); ++i) {
    if(names.contains(name.qualifiers[i])) {
      return true;
    }
  }
  return false;
}

static bool template_argument_syntax_mentions_template_parameter(
    SemanticContext & ctx,
    const TemplateArgumentSyntax & syntax,
    const AtomNameSet & parameter_names);

static bool template_argument_syntax_mentions_template_dependency(
    SemanticContext & ctx,
    const TemplateArgumentSyntax & syntax,
    const AtomNameSet & parameter_names,
    const AtomNameSet & dependent_type_names);

static bool template_id_syntax_mentions_template_parameter(
    SemanticContext & ctx,
    const TemplateIdSyntax & syntax,
    const AtomNameSet & parameter_names)
{
  if(qualified_name_mentions_template_parameter(syntax.name, parameter_names)) {
    return true;
  }
  for(std::size_t i = 0; i < syntax.argument_syntaxes.size(); ++i) {
    if(template_argument_syntax_mentions_template_parameter(
           ctx,
           syntax.argument_syntaxes[i],
           parameter_names)) {
      return true;
    }
  }
  return false;
}

static bool template_id_syntax_mentions_template_dependency(
    SemanticContext & ctx,
    const TemplateIdSyntax & syntax,
    const AtomNameSet & parameter_names,
    const AtomNameSet & dependent_type_names)
{
  if(qualified_name_mentions_template_parameter(syntax.name, parameter_names) ||
     qualified_name_mentions_atom_name(syntax.name, dependent_type_names)) {
    return true;
  }
  for(std::size_t i = 0; i < syntax.argument_syntaxes.size(); ++i) {
    if(template_argument_syntax_mentions_template_dependency(
           ctx,
           syntax.argument_syntaxes[i],
           parameter_names,
           dependent_type_names)) {
      return true;
    }
  }
  return false;
}

static bool ast_mentions_template_parameter(
    SemanticContext & ctx,
    const CppAstNode & node,
    const AtomNameSet & parameter_names)
{
  if(node.kind == CppAstKind::pack_expansion_expression ||
     parameter_names.contains(node.value)) {
    return true;
  }
  if(node.semantic_type && ctx.type_depends_on_template_parameter(node.semantic_type)) {
    return true;
  }
  if(const QualifiedName * qualified = cppast_qualified_name_syntax(node)) {
    if(qualified_name_mentions_template_parameter(*qualified, parameter_names)) {
      return true;
    }
  }
  if(const TemplateIdSyntax * template_id = cppast_template_id_syntax(node)) {
    if(template_id_syntax_mentions_template_parameter(
           ctx,
           *template_id,
           parameter_names)) {
      return true;
    }
  }
  for(std::size_t i = 0; i < node.qualifier_template_id_syntaxes.size(); ++i) {
    if(template_id_syntax_mentions_template_parameter(
           ctx,
           node.qualifier_template_id_syntaxes[i],
           parameter_names)) {
      return true;
    }
  }
  if(node.conversion_type_id_syntax &&
     ast_mentions_template_parameter(
         ctx,
         *node.conversion_type_id_syntax,
         parameter_names)) {
    return true;
  }
  if(node.base_type_syntax &&
     ast_mentions_template_parameter(ctx, *node.base_type_syntax, parameter_names)) {
    return true;
  }
  for(std::size_t i = 0; i < node.exception_type_id_syntaxes.size(); ++i) {
    if(ast_mentions_template_parameter(
           ctx,
           node.exception_type_id_syntaxes[i],
           parameter_names)) {
      return true;
    }
  }
  for(std::size_t i = 0; i < node.children.size(); ++i) {
    if(ast_mentions_template_parameter(ctx, node.children[i], parameter_names)) {
      return true;
    }
  }
  return false;
}

static bool ast_mentions_template_dependency(
    SemanticContext & ctx,
    const CppAstNode & node,
    const AtomNameSet & parameter_names,
    const AtomNameSet & dependent_type_names)
{
  if(node.kind == CppAstKind::pack_expansion_expression ||
     parameter_names.contains(node.value) ||
     dependent_type_names.contains(node.value)) {
    return true;
  }
  if(node.semantic_type && ctx.type_depends_on_template_parameter(node.semantic_type)) {
    return true;
  }
  if(const QualifiedName * qualified = cppast_qualified_name_syntax(node)) {
    if(qualified_name_mentions_template_parameter(*qualified, parameter_names) ||
       qualified_name_mentions_atom_name(*qualified, dependent_type_names)) {
      return true;
    }
  }
  if(const TemplateIdSyntax * template_id = cppast_template_id_syntax(node)) {
    if(template_id_syntax_mentions_template_dependency(
           ctx,
           *template_id,
           parameter_names,
           dependent_type_names)) {
      return true;
    }
  }
  for(std::size_t i = 0; i < node.qualifier_template_id_syntaxes.size(); ++i) {
    if(template_id_syntax_mentions_template_dependency(
           ctx,
           node.qualifier_template_id_syntaxes[i],
           parameter_names,
           dependent_type_names)) {
      return true;
    }
  }
  if(node.conversion_type_id_syntax &&
     ast_mentions_template_dependency(ctx,
                                      *node.conversion_type_id_syntax,
                                      parameter_names,
                                      dependent_type_names)) {
    return true;
  }
  if(node.base_type_syntax &&
     ast_mentions_template_dependency(ctx,
                                      *node.base_type_syntax,
                                      parameter_names,
                                      dependent_type_names)) {
    return true;
  }
  for(std::size_t i = 0; i < node.exception_type_id_syntaxes.size(); ++i) {
    if(ast_mentions_template_dependency(ctx,
                                        node.exception_type_id_syntaxes[i],
                                        parameter_names,
                                        dependent_type_names)) {
      return true;
    }
  }
  for(std::size_t i = 0; i < node.children.size(); ++i) {
    if(ast_mentions_template_dependency(ctx,
                                        node.children[i],
                                        parameter_names,
                                        dependent_type_names)) {
      return true;
    }
  }
  return false;
}

static bool template_argument_syntax_mentions_template_parameter(
    SemanticContext & ctx,
    const TemplateArgumentSyntax & syntax,
    const AtomNameSet & parameter_names)
{
  if(syntax.dependent || syntax.pack_expansion) {
    return true;
  }
  if(syntax.resolved_type && ctx.type_depends_on_template_parameter(syntax.resolved_type)) {
    return true;
  }
  if(syntax.template_id &&
     template_id_syntax_mentions_template_parameter(
         ctx,
         *syntax.template_id,
         parameter_names)) {
    return true;
  }
  if(syntax.type_id &&
     ast_mentions_template_parameter(ctx, *syntax.type_id, parameter_names)) {
    return true;
  }
  if(syntax.expression &&
     ast_mentions_template_parameter(ctx, *syntax.expression, parameter_names)) {
    return true;
  }
  return false;
}

static bool template_argument_syntax_mentions_template_dependency(
    SemanticContext & ctx,
    const TemplateArgumentSyntax & syntax,
    const AtomNameSet & parameter_names,
    const AtomNameSet & dependent_type_names)
{
  if(syntax.dependent || syntax.pack_expansion) {
    return true;
  }
  if(syntax.resolved_type && ctx.type_depends_on_template_parameter(syntax.resolved_type)) {
    return true;
  }
  if(syntax.template_id &&
     template_id_syntax_mentions_template_dependency(
         ctx,
         *syntax.template_id,
         parameter_names,
         dependent_type_names)) {
    return true;
  }
  if(syntax.type_id &&
     ast_mentions_template_dependency(ctx,
                                      *syntax.type_id,
                                      parameter_names,
                                      dependent_type_names)) {
    return true;
  }
  if(syntax.expression &&
     ast_mentions_template_dependency(ctx,
                                      *syntax.expression,
                                      parameter_names,
                                      dependent_type_names)) {
    return true;
  }
  return false;
}

static bool parse_declared_value_type_for_template_body(
    SemanticContext & ctx,
    Scope & scope,
    const CppAstNode & declaration,
    const CppAstNode & init_decl,
    bool & is_typedef,
    TypePtr & type)
{
  const CppAstNode * specifiers =
      find_child_kind(declaration, CppAstKind::decl_specifier_seq);
  if(!specifiers || init_decl.kind != CppAstKind::init_declarator ||
     init_decl.children.empty()) {
    return false;
  }
  if(subtree_has_template_id_syntax(*specifiers)) {
    return false;
  }

  TypePtr base;
  try {
    if(!ctx.parse_decl_spec(*specifiers, scope, is_typedef, base) || !base) {
      return false;
    }
  } catch(const semantic_fallback_audit::SemanticFallbackError &) {
    throw;
  } catch(const std::exception &) {
    return false;
  }

  type = base;
  return true;
}

static void collect_declared_value_types_for_template_body(
    SemanticContext & ctx,
    Scope & scope,
    const CppAstNode & node,
    AtomNameSet & declared_names,
    TemplateBodyValueTypes & declared_value_types)
{
  if(node.kind != CppAstKind::simple_declaration &&
     node.kind != CppAstKind::for_init_statement &&
     node.kind != CppAstKind::condition) {
    return;
  }

  const CppAstNode * declarators =
      find_child_kind(node, CppAstKind::init_declarator_list);
  if(!declarators) {
    return;
  }

  for(std::size_t i = 0; i < declarators->children.size(); ++i) {
    const CppAstNode & init_decl = declarators->children[i];
    std::string declared_name;
    if(init_decl.children.empty() ||
       !declarator_declared_identifier(init_decl.children[0], declared_name) ||
       declared_name.empty()) {
      continue;
    }

    bool is_typedef = false;
    TypePtr type;
    if(!parse_declared_value_type_for_template_body(ctx,
                                                    scope,
                                                    node,
                                                    init_decl,
                                                    is_typedef,
                                                    type)) {
      continue;
    }
    declared_names.insert(declared_name);
    if(is_typedef) {
      scope.named_types[declared_name] = type;
    } else {
      record_template_body_value_type(declared_value_types, declared_name, type);
    }
  }
}

static bool declared_type_syntax_mentions_template_dependency(
    SemanticContext & ctx,
    const CppAstNode & declaration,
    const CppAstNode & declarator,
    const AtomNameSet & template_parameter_names,
    const AtomNameSet & dependent_type_names)
{
  const CppAstNode * specifiers =
      find_child_kind(declaration, CppAstKind::decl_specifier_seq);
  return (specifiers &&
          ast_mentions_template_dependency(ctx,
                                           *specifiers,
                                           template_parameter_names,
                                           dependent_type_names)) ||
         ast_mentions_template_dependency(ctx,
                                          declarator,
                                          template_parameter_names,
                                          dependent_type_names);
}

static void collect_dependent_type_names_from_template_body_declaration(
    SemanticContext & ctx,
    const CppAstNode & node,
    const AtomNameSet & template_parameter_names,
    AtomNameSet & dependent_type_names)
{
  if(node.kind == CppAstKind::alias_declaration) {
    if(!node.value.empty() &&
       ast_mentions_template_dependency(ctx,
                                        node,
                                        template_parameter_names,
                                        dependent_type_names)) {
      dependent_type_names.insert(node.value);
    }
    return;
  }

  if(node.kind != CppAstKind::simple_declaration &&
     node.kind != CppAstKind::for_init_statement &&
     node.kind != CppAstKind::condition) {
    return;
  }

  const CppAstNode * specifiers =
      find_child_kind(node, CppAstKind::decl_specifier_seq);
  if(!specifiers ||
     !cpp_decl::decl_spec_contains_token(*specifiers, KW_TYPEDEF)) {
    return;
  }

  const CppAstNode * declarators =
      find_child_kind(node, CppAstKind::init_declarator_list);
  if(!declarators) {
    return;
  }

  for(std::size_t i = 0; i < declarators->children.size(); ++i) {
    const CppAstNode & init_decl = declarators->children[i];
    if(init_decl.kind != CppAstKind::init_declarator ||
       init_decl.children.empty()) {
      continue;
    }
    std::string declared_name;
    if(!declarator_declared_identifier(init_decl.children[0], declared_name) ||
       declared_name.empty()) {
      continue;
    }
    if(declared_type_syntax_mentions_template_dependency(ctx,
                                                         node,
                                                         init_decl.children[0],
                                                         template_parameter_names,
                                                         dependent_type_names)) {
      dependent_type_names.insert(declared_name);
    }
  }
}

static void collect_dependent_type_names_for_class_template_body(
    SemanticContext & ctx,
    const CppAstNode & class_node,
    const AtomNameSet & template_parameter_names,
    AtomNameSet & dependent_type_names)
{
  for(std::size_t i = 0; i < class_node.children.size(); ++i) {
    const CppAstNode & child = class_node.children[i];
    if(child.kind == CppAstKind::template_declaration) {
      collect_dependent_type_names_for_class_template_body(ctx,
                                                           child,
                                                           template_parameter_names,
                                                           dependent_type_names);
      continue;
    }

    if((child.kind == CppAstKind::class_specifier ||
        child.kind == CppAstKind::class_forward_declaration) &&
       child.value.empty()) {
      collect_dependent_type_names_for_class_template_body(ctx,
                                                           child,
                                                           template_parameter_names,
                                                           dependent_type_names);
      continue;
    }

    collect_dependent_type_names_from_template_body_declaration(
        ctx,
        child,
        template_parameter_names,
        dependent_type_names);
  }
}

static void collect_dependent_declared_value_names_for_template_body(
    SemanticContext & ctx,
    Scope & scope,
    const CppAstNode & node,
    const AtomNameSet & template_parameter_names,
    const AtomNameSet & dependent_type_names,
    AtomNameSet & dependent_value_names)
{
  if(node.kind != CppAstKind::simple_declaration &&
     node.kind != CppAstKind::for_init_statement &&
     node.kind != CppAstKind::condition) {
    return;
  }

  const CppAstNode * declarators =
      find_child_kind(node, CppAstKind::init_declarator_list);
  if(!declarators) {
    return;
  }

  for(std::size_t i = 0; i < declarators->children.size(); ++i) {
    const CppAstNode & init_decl = declarators->children[i];
    if(init_decl.kind != CppAstKind::init_declarator ||
       init_decl.children.empty()) {
      continue;
    }

    const CppAstNode & declarator = init_decl.children[0];
    std::string declared_name;
    if(!declarator_declared_identifier(declarator, declared_name) ||
       declared_name.empty()) {
      continue;
    }

    bool is_typedef = false;
    TypePtr type;
    bool depends =
        parse_declared_value_type_for_template_body(ctx,
                                                    scope,
                                                    node,
                                                    init_decl,
                                                    is_typedef,
                                                    type) &&
        !is_typedef &&
        ctx.type_depends_on_template_parameter(type);
    if(!depends) {
      depends = declared_type_syntax_mentions_template_dependency(
          ctx,
          node,
          declarator,
          template_parameter_names,
          dependent_type_names);
    }
    if(depends) {
      dependent_value_names.insert(declared_name);
    }
  }
}

static void collect_dependent_parameter_names_for_template_body(
    SemanticContext & ctx,
    const CppAstNode & node,
    const AtomNameSet & template_parameter_names,
    const AtomNameSet & dependent_type_names,
    AtomNameSet & dependent_value_names)
{
  if(node.kind == CppAstKind::parameter_declaration) {
    const CppAstNode * declarator =
        find_child_kind(node, CppAstKind::declarator);
    std::string declared_name;
    if(declarator &&
       declarator_declared_identifier(*declarator, declared_name) &&
       !declared_name.empty() &&
       declared_type_syntax_mentions_template_dependency(ctx,
                                                        node,
                                                        *declarator,
                                                        template_parameter_names,
                                                        dependent_type_names)) {
      dependent_value_names.insert(declared_name);
    }
  }

  for(std::size_t i = 0; i < node.children.size(); ++i) {
    collect_dependent_parameter_names_for_template_body(ctx,
                                                       node.children[i],
                                                       template_parameter_names,
                                                       dependent_type_names,
                                                       dependent_value_names);
  }
}

static void collect_alias_type_name_for_template_body(const CppAstNode & node,
                                                      AtomNameSet & type_names)
{
  if(node.kind == CppAstKind::alias_declaration && !node.value.empty()) {
    type_names.insert(node.value);
  }
}

static void collect_enum_enumerator_names_for_template_body(
    const CppAstNode & node,
    AtomNameSet & names)
{
  if(node.kind == CppAstKind::enum_specifier) {
    for(std::size_t i = 0; i < node.children.size(); ++i) {
      const CppAstNode & enumerator = node.children[i];
      if(enumerator.kind == CppAstKind::enumerator &&
         !enumerator.value.empty()) {
        names.insert(enumerator.value);
      }
    }
  }

  for(std::size_t i = 0; i < node.children.size(); ++i) {
    collect_enum_enumerator_names_for_template_body(node.children[i], names);
  }
}

static void collect_class_member_names_for_template_body(
    const CppAstNode & class_node,
    AtomNameSet & names)
{
  for(std::size_t i = 0; i < class_node.children.size(); ++i) {
    const CppAstNode & child = class_node.children[i];
    if(child.kind == CppAstKind::template_declaration) {
      collect_class_member_names_for_template_body(child, names);
      continue;
    }

    if(child.kind == CppAstKind::alias_declaration && !child.value.empty()) {
      names.insert(child.value);
      continue;
    }

    if(child.kind == CppAstKind::using_declaration) {
      const CppAstNode * target = find_child_kind(child, CppAstKind::target);
      if(target && !target->value.empty()) {
        const QualifiedName * qualified = cppast_qualified_name_syntax(*target);
        if(qualified && !qualified->name.empty()) {
          names.insert(qualified->name);
        } else {
          const std::size_t pos = target->value.rfind("::");
          names.insert(pos == std::string::npos ? target->value
                                                : target->value.substr(pos + 2));
        }
      }
      continue;
    }

    if(child.kind == CppAstKind::enum_specifier) {
      collect_enum_enumerator_names_for_template_body(child, names);
      continue;
    }

    if((child.kind == CppAstKind::class_specifier ||
        child.kind == CppAstKind::class_forward_declaration) &&
       child.value.empty()) {
      collect_class_member_names_for_template_body(child, names);
      continue;
    }

    if(child.kind == CppAstKind::simple_declaration) {
      const CppAstNode * specifiers =
          find_child_kind(child, CppAstKind::decl_specifier_seq);
      if(specifiers) {
        collect_enum_enumerator_names_for_template_body(*specifiers, names);
      }
      const CppAstNode * declarators =
          find_child_kind(child, CppAstKind::init_declarator_list);
      if(!declarators) {
        continue;
      }
      for(std::size_t j = 0; j < declarators->children.size(); ++j) {
        const CppAstNode & init_decl = declarators->children[j];
        if(init_decl.kind != CppAstKind::init_declarator ||
           init_decl.children.empty()) {
          continue;
        }
        std::string declared_name;
        if(declarator_declared_identifier(init_decl.children[0],
                                          declared_name) &&
           !declared_name.empty()) {
          names.insert(declared_name);
        }
      }
      continue;
    }

    if((child.kind == CppAstKind::function_definition ||
        child.kind == CppAstKind::special_member_definition) &&
       find_child_kind(child, CppAstKind::declarator)) {
      std::string declared_name;
      if(declarator_declared_identifier(
             *find_child_kind(child, CppAstKind::declarator),
             declared_name) &&
         !declared_name.empty()) {
        names.insert(declared_name);
      }
      continue;
    }
  }
}

static void collect_class_member_type_names_for_template_body(
    const CppAstNode & class_node,
    AtomNameSet & names)
{
  for(std::size_t i = 0; i < class_node.children.size(); ++i) {
    const CppAstNode & child = class_node.children[i];
    if(child.kind == CppAstKind::template_declaration) {
      collect_class_member_type_names_for_template_body(child, names);
      continue;
    }

    if(child.kind == CppAstKind::alias_declaration && !child.value.empty()) {
      names.insert(child.value);
      continue;
    }

    if(child.kind == CppAstKind::simple_declaration) {
      const CppAstNode * specifiers =
          find_child_kind(child, CppAstKind::decl_specifier_seq);
      if(!specifiers ||
         !cpp_decl::decl_spec_contains_token(*specifiers, KW_TYPEDEF)) {
        continue;
      }
      const CppAstNode * declarators =
          find_child_kind(child, CppAstKind::init_declarator_list);
      if(!declarators) {
        continue;
      }
      for(std::size_t j = 0; j < declarators->children.size(); ++j) {
        const CppAstNode & init_decl = declarators->children[j];
        if(init_decl.kind != CppAstKind::init_declarator ||
           init_decl.children.empty()) {
          continue;
        }
        std::string declared_name;
        if(declarator_declared_identifier(init_decl.children[0],
                                          declared_name) &&
           !declared_name.empty()) {
          names.insert(declared_name);
        }
      }
      continue;
    }

    const CppAstNode * type_owner = nullptr;
    if(child.kind == CppAstKind::class_specifier ||
       child.kind == CppAstKind::class_forward_declaration) {
      type_owner = &child;
    } else if(child.kind == CppAstKind::template_declaration) {
      type_owner = find_child_kind(child, CppAstKind::class_specifier);
      if(!type_owner) {
        type_owner = find_child_kind(child, CppAstKind::class_forward_declaration);
      }
    }
    if(!type_owner) {
      continue;
    }

    if(!type_owner->value.empty()) {
      names.insert(type_owner->value);
    }
    collect_class_member_type_names_for_template_body(*type_owner, names);
  }
}

static bool dependent_named_type(const TypePtr & type)
{
  return type &&
         type->kind == cpp_decl::Type::TK_NAMED &&
         type->named_key.rfind("dependent ", 0) == 0;
}

static void collect_member_scope_names_for_template_body(
    const Scope & scope,
    AtomNameSet & member_names,
    AtomNameSet & type_names)
{
  for(std::map<std::string, ValueBinding>::const_iterator it =
          scope.values.begin();
      it != scope.values.end(); ++it) {
    member_names.insert(it->first);
  }
  for(std::map<std::string, std::vector<semantic_model::FunctionBinding *> >::
          const_iterator it = scope.function_sets.begin();
      it != scope.function_sets.end(); ++it) {
    member_names.insert(it->first);
  }
  for(std::map<std::string, std::vector<semantic_model::FunctionTemplateDecl *> >::
          const_iterator it = scope.function_templates.begin();
      it != scope.function_templates.end(); ++it) {
    member_names.insert(it->first);
  }
  for(auto it =
          scope.named_types.begin();
      it != scope.named_types.end(); ++it) {
    type_names.insert(it->first);
  }
  for(std::map<std::string, semantic_model::ClassTemplateDecl *>::const_iterator
          it = scope.class_templates.begin();
      it != scope.class_templates.end(); ++it) {
    type_names.insert(it->first);
  }
  for(std::map<std::string, semantic_model::AliasTemplateDecl *>::const_iterator
          it = scope.alias_templates.begin();
      it != scope.alias_templates.end(); ++it) {
    type_names.insert(it->first);
  }
}

static void collect_class_member_value_types_for_template_body(
    SemanticContext & ctx,
    Scope & lookup_scope,
    const CppAstNode & class_node,
    TemplateBodyValueTypes & value_types)
{
  for(std::size_t i = 0; i < class_node.children.size(); ++i) {
    const CppAstNode & child = class_node.children[i];
    if(child.kind == CppAstKind::template_declaration) {
      collect_class_member_value_types_for_template_body(ctx,
                                                         lookup_scope,
                                                         child,
                                                         value_types);
      continue;
    }

    if((child.kind == CppAstKind::class_specifier ||
        child.kind == CppAstKind::class_forward_declaration) &&
       child.value.empty()) {
      collect_class_member_value_types_for_template_body(ctx,
                                                         lookup_scope,
                                                         child,
                                                         value_types);
      continue;
    }

    if(child.kind != CppAstKind::simple_declaration) {
      continue;
    }

    const CppAstNode * declarators =
        find_child_kind(child, CppAstKind::init_declarator_list);
    if(!declarators) {
      continue;
    }
    for(std::size_t j = 0; j < declarators->children.size(); ++j) {
      const CppAstNode & init_decl = declarators->children[j];
      std::string declared_name;
      if(init_decl.children.empty() ||
         !declarator_declared_identifier(init_decl.children[0],
                                         declared_name) ||
         declared_name.empty()) {
        continue;
      }

      bool is_typedef = false;
      TypePtr type;
      if(!parse_declared_value_type_for_template_body(ctx,
                                                      lookup_scope,
                                                      child,
                                                      init_decl,
                                                      is_typedef,
                                                      type)) {
        continue;
      }
      if(is_typedef) {
        lookup_scope.named_types[declared_name] = type;
      } else {
        record_template_body_value_type(value_types, declared_name, type);
      }
    }
  }
}

static void collect_class_info_names_for_template_body(
    SemanticContext & ctx,
    ClassInfo & info,
    AtomNameSet & member_names,
    AtomNameSet & type_names,
    std::set<const ClassInfo *> & visited)
{
  if(visited.count(&info) != 0) {
    return;
  }
  visited.insert(&info);

  ctx.ensure_class_reference_members(info);
  if(info.member_scope) {
    collect_member_scope_names_for_template_body(*info.member_scope,
                                                 member_names,
                                                 type_names);
  }

  for(std::size_t i = 0; i < info.bases.size(); ++i) {
    if(info.bases[i].type) {
      collect_class_info_names_for_template_body(ctx,
                                                 *info.bases[i].type,
                                                 member_names,
                                                 type_names,
                                                 visited);
    }
  }
}

static void collect_base_class_names_for_template_body(
    SemanticContext & ctx,
    Scope & scope,
    const CppAstNode & class_node,
    AtomNameSet & member_names,
    AtomNameSet & type_names,
    const AtomNameSet & template_parameter_names)
{
  const CppAstNode * clause = find_child_kind(class_node,
                                              CppAstKind::base_clause);
  if(!clause) {
    return;
  }

  std::set<const ClassInfo *> visited;
  for(std::size_t i = 0; i < clause->children.size(); ++i) {
    const CppAstNode & specifier = clause->children[i];
    const CppAstNode * base_name = find_child_kind(specifier,
                                                   CppAstKind::base_name);
    if(!base_name || base_name->value.empty()) {
      continue;
    }
    if(ast_mentions_template_parameter(ctx, *base_name, template_parameter_names)) {
      continue;
    }

    TypePtr base_type;
    try {
      base_type = ctx.lookup_type_node(scope,
                                       *base_name,
                                       base_name->value,
                                       true);
    } catch(const semantic_fallback_audit::SemanticFallbackError &) {
      throw;
    } catch(const std::exception &) {
      continue;
    }

    if(!base_type || dependent_named_type(base_type)) {
      continue;
    }
    ClassInfo * base_info = ctx.class_info_for_type(base_type);
    if(!base_info) {
      continue;
    }
    collect_class_info_names_for_template_body(ctx,
                                               *base_info,
                                               member_names,
                                               type_names,
                                               visited);
  }
}

static bool id_expression_names_known_type_template(SemanticContext & ctx,
                                                    Scope & scope,
                                                    const CppAstNode & node)
{
  const TemplateIdSyntax * template_id = cppast_template_id_syntax(node);
  return template_id &&
         !template_id->name.name.empty() &&
         (ctx.lookup_class_template(scope, template_id->name) ||
          ctx.lookup_alias_template(scope, template_id->name));
}

static bool id_expression_names_ordinary_function(
    SemanticContext & ctx,
    Scope & scope,
    const std::string & name)
{
  return !ctx.lookup_functions(scope,
                               name,
                               semantic_policy::without_body_instantiation()).empty() ||
         !semantic_lookup::lookup_function_templates(ctx, scope, name).empty();
}

static bool id_expression_names_known_type_for_template_body(
    SemanticContext & ctx,
    Scope & scope,
    const CppAstNode & node)
{
  if(id_expression_names_known_type_template(ctx, scope, node)) {
    return true;
  }

  const QualifiedName * qualified = cppast_qualified_name_syntax(node);
  if(qualified &&
     (qualified->rooted || !qualified->qualifiers.empty()) &&
     !cppast_template_id_syntax(node) &&
     !node.qualifier_template_id_syntaxes.empty()) {
    return false;
  }

  try {
    return ctx.lookup_type_node(scope, node, node.value) != nullptr;
  } catch(const semantic_fallback_audit::SemanticFallbackError &) {
    throw;
  } catch(const TemplateSubstitutionFailure &) {
    return false;
  }
}

static bool id_expression_is_unqualified_name(const CppAstNode & node)
{
  if(node.kind != CppAstKind::id_expression) {
    return false;
  }
  const QualifiedName * qualified = cppast_qualified_name_syntax(node);
  return !qualified || (!qualified->rooted && qualified->qualifiers.empty());
}

static TypePtr template_body_expression_type(
    SemanticContext & ctx,
    Scope & scope,
    const CppAstNode & node,
    const TemplateBodyValueTypes & visible_value_types)
{
  if(node.kind == CppAstKind::id_expression) {
    const QualifiedName * qualified = cppast_qualified_name_syntax(node);
    if(!qualified || (!qualified->rooted && qualified->qualifiers.empty())) {
      TypePtr visible_type =
          lookup_template_body_value_type(visible_value_types, node.value);
      if(visible_type) {
        return visible_type;
      }
    }

    if(const ValueBinding * binding = ctx.lookup_value(scope, node.value)) {
      return binding->type;
    }
  }

  return TypePtr();
}

static bool call_expression_has_adl_candidate(
    SemanticContext & ctx,
    Scope & scope,
    const CppAstNode & node,
    const TemplateBodyValueTypes & visible_value_types)
{
  if(node.kind != CppAstKind::call_expression ||
     node.children.empty() ||
     node.children[0].kind != CppAstKind::id_expression) {
    return false;
  }

  const CppAstNode & callee = node.children[0];
  const QualifiedName * qualified = cppast_qualified_name_syntax(callee);
  if((qualified && (qualified->rooted || !qualified->qualifiers.empty())) ||
     cppast_template_id_syntax(callee) ||
     id_expression_names_ordinary_function(ctx, scope, callee.value)) {
    return false;
  }

  const CppAstNode * arguments = nullptr;
  for(std::size_t i = 1; i < node.children.size(); ++i) {
    if(node.children[i].kind == CppAstKind::argument_list ||
       node.children[i].kind == CppAstKind::paren_argument_list) {
      arguments = &node.children[i];
      break;
    }
  }
  if(!arguments) {
    return false;
  }

  std::vector<TypePtr> arg_types;
  for(std::size_t i = 0; i < arguments->children.size(); ++i) {
    TypePtr type = template_body_expression_type(ctx,
                                                 scope,
                                                 arguments->children[i],
                                                 visible_value_types);
    if(type) {
      arg_types.push_back(type);
    }
  }
  if(arg_types.empty()) {
    return false;
  }

  std::vector<Scope *> associated_scopes;
  std::vector<FunctionBinding *> associated_functions;
  std::vector<FunctionTemplateDecl *> associated_templates;
  for(std::size_t i = 0; i < arg_types.size(); ++i) {
    semantic_lookup::collect_associated_namespace_scopes_for_type(ctx,
                                                                  arg_types[i],
                                                                  associated_scopes);
    semantic_lookup::lookup_associated_friend_functions_for_type(ctx,
                                                                 arg_types[i],
                                                                 callee.value,
                                                                 associated_functions);
    semantic_lookup::lookup_associated_friend_function_templates_for_type(
        ctx,
        arg_types[i],
        callee.value,
        associated_templates);
  }
  semantic_lookup::lookup_functions_in_scopes(associated_scopes,
                                              callee.value,
                                              associated_functions);
  semantic_lookup::lookup_function_templates_in_scopes(associated_scopes,
                                                       callee.value,
                                                       associated_templates);
  return !associated_functions.empty() || !associated_templates.empty();
}

static bool template_body_expression_is_dependent_argument(
    SemanticContext & ctx,
    Scope & scope,
    const CppAstNode & node,
    const AtomNameSet & template_parameter_names,
    const AtomNameSet & dependent_type_names,
    const TemplateBodyValueTypes & visible_value_types,
    const AtomNameSet & dependent_value_names)
{
  if(ast_mentions_template_dependency(ctx,
                                      node,
                                      template_parameter_names,
                                      dependent_type_names)) {
    return true;
  }

  if(id_expression_is_unqualified_name(node) &&
     dependent_value_names.contains(node.value)) {
    return true;
  }

  TypePtr type = template_body_expression_type(ctx,
                                               scope,
                                               node,
                                               visible_value_types);
  if(type && ctx.type_depends_on_template_parameter(type)) {
    return true;
  }

  for(std::size_t i = 0; i < node.children.size(); ++i) {
    if(template_body_expression_is_dependent_argument(ctx,
                                                      scope,
                                                      node.children[i],
                                                      template_parameter_names,
                                                      dependent_type_names,
                                                      visible_value_types,
                                                      dependent_value_names)) {
      return true;
    }
  }
  return false;
}

static bool call_expression_has_dependent_argument(
    SemanticContext & ctx,
    Scope & scope,
    const CppAstNode & node,
    const AtomNameSet & template_parameter_names,
    const AtomNameSet & dependent_type_names,
    const TemplateBodyValueTypes & visible_value_types,
    const AtomNameSet & dependent_value_names)
{
  if(node.kind != CppAstKind::call_expression ||
     node.children.empty() ||
     !id_expression_is_unqualified_name(node.children[0])) {
    return false;
  }

  const CppAstNode & callee = node.children[0];
  if(cppast_template_id_syntax(callee) ||
     id_expression_names_ordinary_function(ctx, scope, callee.value)) {
    return false;
  }

  const CppAstNode * arguments = nullptr;
  for(std::size_t i = 1; i < node.children.size(); ++i) {
    if(node.children[i].kind == CppAstKind::argument_list ||
       node.children[i].kind == CppAstKind::paren_argument_list) {
      arguments = &node.children[i];
      break;
    }
  }
  if(!arguments) {
    return false;
  }

  for(std::size_t i = 0; i < arguments->children.size(); ++i) {
    if(template_body_expression_is_dependent_argument(ctx,
                                                      scope,
                                                      arguments->children[i],
                                                      template_parameter_names,
                                                      dependent_type_names,
                                                      visible_value_types,
                                                      dependent_value_names)) {
      return true;
    }
  }

  return false;
}

static bool collect_using_directive_for_template_body(
    SemanticContext & ctx,
    Scope & scope,
    const CppAstNode & node,
    const CppAstNode *& offending_node,
    std::string & offending_name)
{
  try {
    semantic_declaration::collect_using_directive(ctx, scope, node);
    return false;
  } catch(const semantic_fallback_audit::SemanticFallbackError &) {
    throw;
  } catch(const std::exception &) {
    offending_node = &node;
    offending_name.clear();
    const CppAstNode * target = find_child_kind(node, CppAstKind::target);
    if(target) {
      offending_node = target;
      const QualifiedName * qualified = cppast_qualified_name_syntax(*target);
      offending_name =
          qualified && !qualified->name.empty() ? qualified->name : target->value;
    }
    return true;
  }
}

static bool collect_using_declaration_for_template_body(
    SemanticContext & ctx,
    Scope & scope,
    const CppAstNode & node,
    const CppAstNode *& offending_node,
    std::string & offending_name)
{
  try {
    semantic_declaration::collect_using_declaration(ctx, scope, node);
    return false;
  } catch(const semantic_fallback_audit::SemanticFallbackError &) {
    throw;
  } catch(const std::exception &) {
    offending_node = &node;
    offending_name.clear();
    const CppAstNode * target = find_child_kind(node, CppAstKind::target);
    if(target) {
      offending_node = target;
      const QualifiedName * qualified = cppast_qualified_name_syntax(*target);
      offending_name =
          qualified && !qualified->name.empty() ? qualified->name : target->value;
    }
    return true;
  }
}

static bool template_body_has_invalid_nondependent_id_expression(
    SemanticContext & ctx,
    Scope & scope,
    const CppAstNode & node,
    const AtomNameSet & visible_names,
    const AtomNameSet & type_parameter_names,
    const AtomNameSet & template_parameter_names,
    const AtomNameSet & dependent_type_names,
    const TemplateBodyValueTypes & visible_value_types,
    const AtomNameSet & dependent_value_names,
    const CppAstNode *& offending_node,
    std::string & offending_name)
{
  if(node.kind == CppAstKind::lambda_expression ||
     node.kind == CppAstKind::class_specifier ||
     node.kind == CppAstKind::class_forward_declaration) {
    return false;
  }

  if(node.kind == CppAstKind::using_directive) {
    return collect_using_directive_for_template_body(ctx,
                                                     scope,
                                                     node,
                                                     offending_node,
                                                     offending_name);
  }

  if(node.kind == CppAstKind::using_declaration) {
    return collect_using_declaration_for_template_body(ctx,
                                                       scope,
                                                       node,
                                                       offending_node,
                                                       offending_name);
  }

  if(node.kind == CppAstKind::simple_declaration ||
     node.kind == CppAstKind::for_init_statement ||
     node.kind == CppAstKind::condition) {
    AtomNameSet extended = visible_names;
    TemplateBodyValueTypes extended_value_types = visible_value_types;
    AtomNameSet extended_dependent_type_names = dependent_type_names;
    AtomNameSet extended_dependent_value_names = dependent_value_names;
    collect_dependent_type_names_from_template_body_declaration(
        ctx,
        node,
        template_parameter_names,
        extended_dependent_type_names);
    collect_declared_names_for_template_body(node, extended);
    collect_declared_value_types_for_template_body(ctx,
                                                   scope,
                                                   node,
                                                   extended,
                                                   extended_value_types);
    collect_dependent_declared_value_names_for_template_body(
        ctx,
        scope,
        node,
        template_parameter_names,
        extended_dependent_type_names,
        extended_dependent_value_names);
    for(std::size_t i = 0; i < node.children.size(); ++i) {
      if(template_body_has_invalid_nondependent_id_expression(ctx,
                                                              scope,
                                                              node.children[i],
                                                              extended,
                                                              type_parameter_names,
                                                              template_parameter_names,
                                                              extended_dependent_type_names,
                                                              extended_value_types,
                                                              extended_dependent_value_names,
                                                              offending_node,
                                                              offending_name)) {
        return true;
      }
    }
    return false;
  }

  if(node.kind == CppAstKind::call_expression &&
     !node.children.empty() &&
     node.children[0].kind == CppAstKind::id_expression &&
     (type_parameter_names.contains(node.children[0].value) ||
      id_expression_names_known_type_for_template_body(ctx, scope, node.children[0]))) {
    for(std::size_t i = 1; i < node.children.size(); ++i) {
      if(template_body_has_invalid_nondependent_id_expression(ctx,
                                                              scope,
                                                              node.children[i],
                                                              visible_names,
                                                              type_parameter_names,
                                                              template_parameter_names,
                                                              dependent_type_names,
                                                              visible_value_types,
                                                              dependent_value_names,
                                                              offending_node,
                                                              offending_name)) {
        return true;
      }
    }
    return false;
  }

  if(node.kind == CppAstKind::call_expression &&
     call_expression_has_adl_candidate(ctx, scope, node, visible_value_types)) {
    for(std::size_t i = 1; i < node.children.size(); ++i) {
      if(template_body_has_invalid_nondependent_id_expression(ctx,
                                                              scope,
                                                              node.children[i],
                                                              visible_names,
                                                              type_parameter_names,
                                                              template_parameter_names,
                                                              dependent_type_names,
                                                              visible_value_types,
                                                              dependent_value_names,
                                                              offending_node,
                                                              offending_name)) {
        return true;
      }
    }
    return false;
  }

  if(node.kind == CppAstKind::call_expression &&
     call_expression_has_dependent_argument(ctx,
                                            scope,
                                            node,
                                            template_parameter_names,
                                            dependent_type_names,
                                            visible_value_types,
                                            dependent_value_names)) {
    for(std::size_t i = 1; i < node.children.size(); ++i) {
      if(template_body_has_invalid_nondependent_id_expression(ctx,
                                                              scope,
                                                              node.children[i],
                                                              visible_names,
                                                              type_parameter_names,
                                                              template_parameter_names,
                                                              dependent_type_names,
                                                              visible_value_types,
                                                              dependent_value_names,
                                                              offending_node,
                                                              offending_name)) {
        return true;
      }
    }
    return false;
  }

  if(node.kind == CppAstKind::compound_statement) {
    Scope sequential_scope(&scope, "", false);
    AtomNameSet sequential_visible = visible_names;
    AtomNameSet sequential_type_names = type_parameter_names;
    TemplateBodyValueTypes sequential_value_types = visible_value_types;
    AtomNameSet sequential_dependent_type_names = dependent_type_names;
    AtomNameSet sequential_dependent_value_names = dependent_value_names;
    for(std::size_t i = 0; i < node.children.size(); ++i) {
      if(template_body_has_invalid_nondependent_id_expression(ctx,
                                                              sequential_scope,
                                                              node.children[i],
                                                              sequential_visible,
                                                              sequential_type_names,
                                                              template_parameter_names,
                                                              sequential_dependent_type_names,
                                                              sequential_value_types,
                                                              sequential_dependent_value_names,
                                                              offending_node,
                                                              offending_name)) {
        return true;
      }
      if(node.children[i].kind == CppAstKind::simple_declaration) {
        collect_dependent_type_names_from_template_body_declaration(
            ctx,
            node.children[i],
            template_parameter_names,
            sequential_dependent_type_names);
        collect_declared_names_for_template_body(node.children[i],
                                                 sequential_visible);
        collect_declared_value_types_for_template_body(ctx,
                                                       sequential_scope,
                                                       node.children[i],
                                                       sequential_visible,
                                                       sequential_value_types);
        collect_dependent_declared_value_names_for_template_body(
            ctx,
            sequential_scope,
            node.children[i],
            template_parameter_names,
            sequential_dependent_type_names,
            sequential_dependent_value_names);
      }
      collect_alias_type_name_for_template_body(node.children[i],
                                                sequential_type_names);
    }
    return false;
  }

  if(node.kind == CppAstKind::range_for_statement) {
    if(node.children.size() != 3) {
      return false;
    }

    if(template_body_has_invalid_nondependent_id_expression(ctx,
                                                            scope,
                                                            node.children[0],
                                                            visible_names,
                                                            type_parameter_names,
                                                            template_parameter_names,
                                                            dependent_type_names,
                                                            visible_value_types,
                                                            dependent_value_names,
                                                            offending_node,
                                                            offending_name) ||
       template_body_has_invalid_nondependent_id_expression(ctx,
                                                            scope,
                                                            node.children[1],
                                                            visible_names,
                                                            type_parameter_names,
                                                            template_parameter_names,
                                                            dependent_type_names,
                                                            visible_value_types,
                                                            dependent_value_names,
                                                            offending_node,
                                                            offending_name)) {
      return true;
    }

    AtomNameSet body_visible = visible_names;
    AtomNameSet body_dependent_type_names = dependent_type_names;
    AtomNameSet body_dependent_value_names = dependent_value_names;
    collect_dependent_type_names_from_template_body_declaration(
        ctx,
        node.children[0],
        template_parameter_names,
        body_dependent_type_names);
    collect_declared_names_for_template_body(node.children[0], body_visible);
    collect_dependent_declared_value_names_for_template_body(
        ctx,
        scope,
        node.children[0],
        template_parameter_names,
        body_dependent_type_names,
        body_dependent_value_names);
    return template_body_has_invalid_nondependent_id_expression(ctx,
                                                               scope,
                                                               node.children[2],
                                                               body_visible,
                                                               type_parameter_names,
                                                               template_parameter_names,
                                                               body_dependent_type_names,
                                                               visible_value_types,
                                                               body_dependent_value_names,
                                                               offending_node,
                                                               offending_name);
  }

  if(node.kind == CppAstKind::for_statement ||
     node.kind == CppAstKind::if_statement ||
     node.kind == CppAstKind::while_statement ||
     node.kind == CppAstKind::switch_statement) {
    AtomNameSet control_visible = visible_names;
    AtomNameSet control_dependent_type_names = dependent_type_names;
    AtomNameSet control_dependent_value_names = dependent_value_names;
    for(std::size_t i = 0; i < node.children.size(); ++i) {
      const CppAstNode & child = node.children[i];
      if(template_body_has_invalid_nondependent_id_expression(ctx,
                                                              scope,
                                                              child,
                                                              control_visible,
                                                              type_parameter_names,
                                                              template_parameter_names,
                                                              control_dependent_type_names,
                                                              visible_value_types,
                                                              control_dependent_value_names,
                                                              offending_node,
                                                              offending_name)) {
        return true;
      }
      if(child.kind == CppAstKind::for_init_statement ||
         child.kind == CppAstKind::condition) {
        collect_dependent_type_names_from_template_body_declaration(
            ctx,
            child,
            template_parameter_names,
            control_dependent_type_names);
        collect_declared_names_for_template_body(child, control_visible);
        collect_dependent_declared_value_names_for_template_body(
            ctx,
            scope,
            child,
            template_parameter_names,
            control_dependent_type_names,
            control_dependent_value_names);
      }
    }
    return false;
  }

  if(node.kind == CppAstKind::id_expression) {
    if(node.value.empty() ||
       node.value == "this" ||
       node.value == "__null" ||
       node.value == "__func__" ||
       node.value == "__FUNCTION__" ||
       node.value == "__PRETTY_FUNCTION__") {
      return false;
    }

    const QualifiedName * qualified = cppast_qualified_name_syntax(node);
    if(qualified && (qualified->rooted || !qualified->qualifiers.empty())) {
      return false;
    }

    if(cppast_template_id_syntax(node)) {
      return false;
    }

    if(visible_names.contains(node.value) ||
       ctx.lookup_value(scope, node.value) ||
       id_expression_names_ordinary_function(ctx, scope, node.value)) {
      return false;
    }

    offending_node = &node;
    offending_name = node.value;
    return true;
  }

  for(std::size_t i = 0; i < node.children.size(); ++i) {
    if(template_body_has_invalid_nondependent_id_expression(ctx,
                                                            scope,
                                                            node.children[i],
                                                            visible_names,
                                                            type_parameter_names,
                                                            template_parameter_names,
                                                            dependent_type_names,
                                                            visible_value_types,
                                                            dependent_value_names,
                                                            offending_node,
                                                            offending_name)) {
      return true;
    }
  }
  return false;
}

bool class_member_body_has_invalid_nondependent_lookup(
    SemanticContext & ctx,
    Scope & scope,
    const CppAstNode & class_node,
    const std::vector<TemplateParameterInfo> & parameters,
    const CppAstNode *& offending_node,
    std::string & offending_name)
{
  AtomNameSet member_names =
      non_type_template_parameter_names(parameters);
  AtomNameSet type_names =
      type_template_parameter_names(parameters);
  const AtomNameSet parameter_names =
      template_parameter_atom_names(parameters);
  if(!class_node.value.empty()) {
    type_names.insert(class_node.value);
  }
  collect_class_member_names_for_template_body(class_node, member_names);
  collect_class_member_type_names_for_template_body(class_node, type_names);
  collect_base_class_names_for_template_body(ctx,
                                             scope,
                                             class_node,
                                             member_names,
                                             type_names,
                                             parameter_names);
  Scope lookup_scope(&scope, "", false);
  TemplateBodyValueTypes member_value_types;
  AtomNameSet dependent_type_names =
      type_template_parameter_names(parameters);
  collect_dependent_type_names_for_class_template_body(ctx,
                                                       class_node,
                                                       parameter_names,
                                                       dependent_type_names);
  collect_class_member_value_types_for_template_body(ctx,
                                                     lookup_scope,
                                                     class_node,
                                                     member_value_types);

  for(std::size_t i = 0; i < class_node.children.size(); ++i) {
    const CppAstNode & child = class_node.children[i];
    const CppAstNode * body = nullptr;
    const CppAstNode * declarator = nullptr;
    if(child.kind == CppAstKind::function_definition) {
      body = find_function_body_node(child);
      declarator = find_child_kind(child, CppAstKind::declarator);
    } else if(child.kind == CppAstKind::special_member_definition) {
      body = find_function_body_node(child);
      declarator = find_child_kind(child, CppAstKind::declarator);
    } else {
      continue;
    }

    if(!body) {
      continue;
    }

    AtomNameSet visible_names = member_names;
    AtomNameSet dependent_value_names;
    if(declarator) {
      collect_declared_names_for_template_body(*declarator, visible_names);
      collect_dependent_parameter_names_for_template_body(ctx,
                                                         *declarator,
                                                         parameter_names,
                                                         dependent_type_names,
                                                         dependent_value_names);
    }
    if(template_body_has_invalid_nondependent_id_expression(ctx,
                                                            lookup_scope,
                                                            *body,
                                                            visible_names,
                                                            type_names,
                                                            parameter_names,
                                                            dependent_type_names,
                                                            member_value_types,
                                                            dependent_value_names,
                                                            offending_node,
                                                            offending_name)) {
      return true;
    }
  }

  return false;
}

bool subtree_alias_redeclares_template_parameter(
    const CppAstNode & node,
    const std::set<std::string> & parameter_names,
    const CppAstNode *& offending_node,
    std::string & offending_name)
{
  if(parameter_names.empty()) {
    return false;
  }
  if(node.kind == CppAstKind::alias_declaration &&
     !node.value.empty() &&
     parameter_names.find(node.value) != parameter_names.end()) {
    offending_node = &node;
    offending_name = node.value;
    return true;
  }
  for(std::size_t i = 0; i < node.children.size(); ++i) {
    if(subtree_alias_redeclares_template_parameter(
           node.children[i], parameter_names, offending_node, offending_name)) {
      return true;
    }
  }
  return false;
}

static bool subtree_alias_redeclares_template_parameter_atom(
    const CppAstNode & node,
    const AtomNameSet & parameter_names,
    const CppAstNode *& offending_node,
    std::string & offending_name)
{
  if(parameter_names.empty()) {
    return false;
  }
  if(node.kind == CppAstKind::alias_declaration &&
     !node.value.empty() &&
     parameter_names.contains(node.value)) {
    offending_node = &node;
    offending_name = node.value;
    return true;
  }
  for(std::size_t i = 0; i < node.children.size(); ++i) {
    if(subtree_alias_redeclares_template_parameter_atom(
           node.children[i], parameter_names, offending_node, offending_name)) {
      return true;
    }
  }
  return false;
}

bool class_member_redeclares_template_parameter(
    const CppAstNode & class_node,
    const std::vector<TemplateParameterInfo> & parameters,
    const CppAstNode *& offending_node,
    std::string & offending_name)
{
  const AtomNameSet parameter_names = template_parameter_atom_names(parameters);
  if(parameter_names.empty()) {
    return false;
  }

  const auto note_match =
      [&](const CppAstNode & node, const std::string & name) -> bool
  {
    if(!parameter_names.contains(name)) {
      return false;
    }
    offending_node = &node;
    offending_name = name;
    return true;
  };

  for(std::size_t i = 0; i < class_node.children.size(); ++i) {
    const CppAstNode & child = class_node.children[i];
    if(child.kind == CppAstKind::simple_declaration) {
      const CppAstNode * declarators =
          find_child_kind(child, CppAstKind::init_declarator_list);
      if(!declarators) {
        continue;
      }
      for(std::size_t j = 0; j < declarators->children.size(); ++j) {
        const CppAstNode & init_decl = declarators->children[j];
        if(init_decl.kind != CppAstKind::init_declarator ||
           init_decl.children.empty()) {
          continue;
        }
        std::string declared_name;
        if(declarator_declared_identifier(init_decl.children[0],
                                          declared_name) &&
           note_match(init_decl.children[0], declared_name)) {
          return true;
        }
      }
      continue;
    }

    if(child.kind == CppAstKind::function_definition &&
       find_child_kind(child, CppAstKind::declarator)) {
      std::string declared_name;
      if(declarator_declared_identifier(
             *find_child_kind(child, CppAstKind::declarator),
             declared_name) &&
         note_match(child, declared_name)) {
        return true;
      }
      const CppAstNode * body = find_function_body_node(child);
      if(body &&
         subtree_alias_redeclares_template_parameter_atom(
             *body, parameter_names, offending_node, offending_name)) {
        return true;
      }
      continue;
    }

    if(child.kind == CppAstKind::special_member_definition) {
      const CppAstNode * body = find_function_body_node(child);
      if(body &&
         subtree_alias_redeclares_template_parameter_atom(
             *body, parameter_names, offending_node, offending_name)) {
        return true;
      }
      continue;
    }

    if((child.kind == CppAstKind::alias_declaration ||
        child.kind == CppAstKind::using_declaration ||
        child.kind == CppAstKind::class_specifier ||
        child.kind == CppAstKind::class_forward_declaration) &&
       !child.value.empty() &&
       note_match(child, child.value)) {
      return true;
    }
  }

  return false;
}

bool split_unqualified_template_head_text(const std::string & text,
                                          std::string & base_name)
{
  const std::string trimmed = trim_space(text);
  const std::string stripped =
      strip_trailing_top_level_template_arguments(trimmed);
  if(stripped.size() >= trimmed.size() ||
     stripped.empty() ||
     trimmed[stripped.size()] != '<' ||
     !is_identifier_text(stripped)) {
    return false;
  }
  base_name = stripped;
  return true;
}

}  // namespace callsemantic
