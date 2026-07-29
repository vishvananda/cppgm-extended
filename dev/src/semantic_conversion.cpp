#include "semantic_conversion.h"

#include <sstream>
#include <set>
#include <stdexcept>

#include "class_template_mangle_info.h"
#include "cpp_decl_ast.h"
#include "cpp_decl_bridge.h"
#include "cppast_dump.h"
#include "constructor_lifecycle_service.h"
#include "parser_trace.h"
#include "semantic_context.h"
#include "semantic_dependent_type.h"
#include "semantic_errors.h"
#include "semantic_hotspot.h"
#include "semantic_lookup.h"
#include "semantic_metrics.h"
#include "semantic_template_function.h"
#include "template_api.h"
#include "template_instantiation.h"

using namespace std;

namespace semantic_conversion {

using namespace cpp_decl;
using namespace semantic_lookup;

namespace {

struct ParsedSourceLocation
{
  bool valid = false;
  std::string file;
  int line = 0;
  int column = 0;
};

ParsedSourceLocation parse_source_location_text(const std::string & text)
{
  ParsedSourceLocation parsed;
  const std::size_t last_colon = text.rfind(':');
  if(last_colon == std::string::npos) {
    return parsed;
  }
  const std::size_t second_colon = text.rfind(':', last_colon - 1);
  if(second_colon == std::string::npos) {
    return parsed;
  }
  parsed.file = text.substr(0, second_colon);
  parsed.line = std::atoi(text.substr(second_colon + 1,
                                      last_colon - second_colon - 1).c_str());
  parsed.column = std::atoi(text.substr(last_colon + 1).c_str());
  parsed.valid = !parsed.file.empty();
  return parsed;
}

std::string callsem_node_source_location_text(const CallSemNode & node)
{
  if(!node.has_source_location()) {
    return std::string();
  }
  std::ostringstream out;
  out << callsem_source_file(node) << ":"
      << callsem_source_line(node) << ":"
      << callsem_source_column(node);
  return out.str();
}

std::string prefer_later_source_location_text(const std::string & first,
                                              const std::string & second)
{
  if(first.empty()) {
    return second;
  }
  if(second.empty()) {
    return first;
  }
  const ParsedSourceLocation parsed_first = parse_source_location_text(first);
  const ParsedSourceLocation parsed_second = parse_source_location_text(second);
  if(!parsed_first.valid || !parsed_second.valid ||
     parsed_first.file != parsed_second.file) {
    return first;
  }
  if(parsed_second.line > parsed_first.line) {
    return second;
  }
  if(parsed_second.line < parsed_first.line) {
    return first;
  }
  return parsed_second.column >= parsed_first.column ? second : first;
}

std::string constructor_probe_use_location(const ExprInfo & expr)
{
  return prefer_later_source_location_text(parser_trace::current_use_location(),
                                           callsem_node_source_location_text(expr.node));
}

TypePtr conversion_reference_target_object_type(const TypePtr & type)
{
  TypePtr base = strip_top_level_cv(remove_reference_type(type));
  if(!base) {
    base = strip_top_level_cv(type);
  }
  return base;
}

bool dependent_conversion_candidate_can_bind_known_argument(
    SemanticContext & ctx,
    const TypePtr & target,
    const ExprInfo & arg)
{
  if(!target || !arg.type) {
    return false;
  }
  const bool target_dependent = ctx.type_depends_on_template_parameter(target);
  const bool argument_dependent = ctx.type_depends_on_template_parameter(arg.type);
  if(!target_dependent && !argument_dependent) {
    return false;
  }
  if(argument_dependent) {
    return true;
  }

  TypePtr target_object = conversion_reference_target_object_type(target);
  if(!target_object || !ctx.type_depends_on_template_parameter(target_object)) {
    return true;
  }

  void * target_class_template = nullptr;
  vector<DependentAliasTemplateArgumentSyntax> target_arguments;
  if(!named_type_dependent_class_template(target_object,
                                          target_class_template,
                                          target_arguments) ||
     !target_class_template) {
    return true;
  }

  TypePtr argument_object = conversion_reference_target_object_type(arg.type);
  ClassInfo * argument_class =
      argument_object ? ctx.class_info_for_type(argument_object) : nullptr;
  if(!argument_class) {
    return true;
  }

  return argument_class->source_template &&
         static_cast<void *>(argument_class->source_template) ==
             target_class_template;
}

void record_conversion_function_source_use(SemanticContext & ctx,
                                           const ExprInfo & source_expr,
                                           FunctionBinding * binding,
                                           const std::string & source_use_location)
{
  std::string public_location = source_use_location;
  if(public_location.empty()) {
    public_location = callsem_node_source_location_text(source_expr.node);
  }
  if(public_location.empty()) {
    public_location = parser_trace::current_use_location();
  }

  semantic_template_function::FunctionTemplateCallSourceUseRequest request;
  request.binding = binding;
  request.use_location = public_location;
  request.candidates_built = 1;
  request.candidates_viable = 1;
  request.candidate_count = 1;
  semantic_template_function::emit_function_template_call_source_use(ctx, request);
}

bool try_fast_fundamental_exact_conversion(const TypePtr & target,
                                           const ExprInfo & expr,
                                           ConversionRank & rank)
{
  if(!target || !expr.type) {
    return false;
  }

  TypePtr target_base = strip_top_level_cv(target);
  if(!target_base ||
     target_base->kind == Type::TK_LVALUE_REFERENCE ||
     target_base->kind == Type::TK_RVALUE_REFERENCE ||
     target_base->kind != Type::TK_FUNDAMENTAL) {
    return false;
  }

  TypePtr source_base = strip_top_level_cv(remove_reference_type(expr.type));
  if(!source_base ||
     source_base->kind != Type::TK_FUNDAMENTAL ||
     source_base->fundamental != target_base->fundamental) {
    return false;
  }

  rank = CR_EXACT;
  return true;
}

}  // namespace

ExprInfo::ExprInfo() {}

ExprInfo::ExprInfo(const ExprInfo & other)
  : type(other.type),
    category(other.category),
    node(other.node),
    null_pointer_constant(other.null_pointer_constant)
{}

ExprInfo & ExprInfo::operator=(const ExprInfo & other)
{
  if(this == &other) {
    return *this;
  }
  type = other.type;
  category = other.category;
  node = other.node;
  null_pointer_constant = other.null_pointer_constant;
  return *this;
}

ExprInfo::ExprInfo(ExprInfo && other)
  : type(static_cast<TypePtr &&>(other.type)),
    category(other.category),
    node(static_cast<CallSemNode &&>(other.node)),
    null_pointer_constant(other.null_pointer_constant)
{}

ExprInfo & ExprInfo::operator=(ExprInfo && other)
{
  if(this == &other) {
    return *this;
  }
  type = static_cast<TypePtr &&>(other.type);
  category = other.category;
  node = static_cast<CallSemNode &&>(other.node);
  null_pointer_constant = other.null_pointer_constant;
  return *this;
}

namespace {

int integral_conversion_rank(EFundamentalType type)
{
  switch(type) {
  case FT_BOOL:
    return 0;
  case FT_CHAR:
  case FT_SIGNED_CHAR:
  case FT_UNSIGNED_CHAR:
  case FT_WCHAR_T:
  case FT_CHAR16_T:
  case FT_CHAR32_T:
    return 1;
  case FT_SHORT_INT:
  case FT_UNSIGNED_SHORT_INT:
    return 2;
  case FT_INT:
  case FT_UNSIGNED_INT:
    return 3;
  case FT_LONG_INT:
  case FT_UNSIGNED_LONG_INT:
    return 4;
  case FT_LONG_LONG_INT:
  case FT_UNSIGNED_LONG_LONG_INT:
    return 5;
  case FT_INT128:
  case FT_UINT128:
    return 6;
  default:
    return -1;
  }
}

EFundamentalType corresponding_unsigned_integral(EFundamentalType type)
{
  switch(type) {
  case FT_SIGNED_CHAR:
    return FT_UNSIGNED_CHAR;
  case FT_SHORT_INT:
    return FT_UNSIGNED_SHORT_INT;
  case FT_INT:
    return FT_UNSIGNED_INT;
  case FT_LONG_INT:
    return FT_UNSIGNED_LONG_INT;
  case FT_LONG_LONG_INT:
    return FT_UNSIGNED_LONG_LONG_INT;
  case FT_INT128:
    return FT_UINT128;
  case FT_UNSIGNED_CHAR:
  case FT_UNSIGNED_SHORT_INT:
  case FT_UNSIGNED_INT:
  case FT_UNSIGNED_LONG_INT:
  case FT_UNSIGNED_LONG_LONG_INT:
  case FT_UINT128:
    return type;
  default:
    return FT_INT;
  }
}

const Type * fundamental_base_type(const TypePtr & type)
{
  TypePtr base = strip_top_level_cv(type);
  return base && base->kind == Type::TK_FUNDAMENTAL ? base.get() : nullptr;
}

bool is_member_pointer_type(const TypePtr & type)
{
  TypePtr base = strip_top_level_cv(type);
  return base && base->kind == Type::TK_MEMBER_POINTER;
}

bool is_nullable_pointer_like_type(const TypePtr & type)
{
  TypePtr base = strip_top_level_cv(type);
  return is_pointer_type(base) || is_member_pointer_type(base);
}

TypePtr reference_binding_source_type(const ExprInfo & expr)
{
  // Reference binding must preserve top-level cv on the referred object so
  // non-const references do not bind through a const object type such as
  // `char * const&`. Only discard an invalid/spurious cv wrapper around the
  // reference itself, which can appear after alias substitution.
  TypePtr reference_type = strip_top_level_cv(expr.type);
  if(reference_type &&
     (reference_type->kind == Type::TK_LVALUE_REFERENCE ||
      reference_type->kind == Type::TK_RVALUE_REFERENCE)) {
    return remove_reference_type(reference_type);
  }
  return remove_reference_type(expr.type);
}

bool top_level_cv_allows_reference_binding(const TypePtr & target_referent,
                                           const TypePtr & source_object)
{
  TypePtr target_base;
  TypePtr source_base;
  bool target_const = false;
  bool target_volatile = false;
  bool source_const = false;
  bool source_volatile = false;
  if(!top_level_cv_flags(target_referent, target_base, target_const, target_volatile) ||
     !top_level_cv_flags(source_object, source_base, source_const, source_volatile)) {
    return false;
  }
  return (!source_const || target_const) && (!source_volatile || target_volatile);
}

bool reference_referents_are_same_ignoring_top_cv(const TypePtr & lhs,
                                                  const TypePtr & rhs)
{
  TypePtr lhs_base = strip_top_level_cv(lhs);
  TypePtr rhs_base = strip_top_level_cv(rhs);
  if(!lhs_base || !rhs_base) {
    return false;
  }
  if(lhs_base->kind == Type::TK_ARRAY || rhs_base->kind == Type::TK_ARRAY) {
    return lhs_base->kind == Type::TK_ARRAY &&
           rhs_base->kind == Type::TK_ARRAY &&
           lhs_base->has_bound == rhs_base->has_bound &&
           lhs_base->bound == rhs_base->bound &&
           lhs_base->bound_text == rhs_base->bound_text &&
           reference_referents_are_same_ignoring_top_cv(lhs_base->inner,
                                                        rhs_base->inner);
  }
  return type_equals(lhs_base, rhs_base);
}

bool class_template_specialization_metadata_has_same_identity(
    SemanticContext & ctx,
    const TypePtr & lhs,
    const TypePtr & rhs)
{
  std::shared_ptr<const ClassTemplateSpecializationMangleInfo> lhs_info =
      named_type_class_template_specialization_mangle_info_const(lhs);
  std::shared_ptr<const ClassTemplateSpecializationMangleInfo> rhs_info =
      named_type_class_template_specialization_mangle_info_const(rhs);
  if(!lhs_info ||
     !rhs_info ||
     !lhs_info->class_template_decl ||
     lhs_info->class_template_decl != rhs_info->class_template_decl ||
     lhs_info->arguments.empty() ||
     rhs_info->arguments.empty()) {
    return false;
  }
  if(ctx.type_depends_on_template_parameter(lhs) ||
     ctx.type_depends_on_template_parameter(rhs)) {
    return false;
  }

  const std::string lhs_key =
      template_instantiation::template_argument_key_for_instantiation(
          ctx,
          lhs_info->arguments);
  const std::string rhs_key =
      template_instantiation::template_argument_key_for_instantiation(
          ctx,
          rhs_info->arguments);
  return lhs_key == rhs_key;
}

bool class_object_types_have_same_semantic_identity(SemanticContext & ctx,
                                                    const TypePtr & lhs,
                                                    const TypePtr & rhs)
{
  TypePtr lhs_base = strip_top_level_cv(lhs);
  TypePtr rhs_base = strip_top_level_cv(rhs);
  if(!lhs_base || !rhs_base) {
    return false;
  }
  if(type_equals(lhs_base, rhs_base)) {
    return true;
  }
  if(lhs_base->kind != Type::TK_NAMED || rhs_base->kind != Type::TK_NAMED) {
    return false;
  }
  return class_template_specialization_metadata_has_same_identity(ctx,
                                                                  lhs_base,
                                                                  rhs_base);
}

bool same_type_with_compatible_top_cv_for_semantic_identity(
    SemanticContext & ctx,
    const TypePtr & target,
    const TypePtr & source)
{
  if(same_type_with_compatible_top_cv(target, source)) {
    return true;
  }

  TypePtr target_base;
  TypePtr source_base;
  bool target_const = false;
  bool target_volatile = false;
  bool source_const = false;
  bool source_volatile = false;
  if(!top_level_cv_flags(target, target_base, target_const, target_volatile) ||
     !top_level_cv_flags(source, source_base, source_const, source_volatile)) {
    return false;
  }
  if((source_const && !target_const) ||
     (source_volatile && !target_volatile)) {
    return false;
  }

  return class_object_types_have_same_semantic_identity(ctx, target_base, source_base);
}

bool pointer_pointee_cv_allows_base_conversion(const TypePtr & target_pointer,
                                               const TypePtr & source_pointer,
                                               TypePtr * target_pointee_base_out,
                                               TypePtr * source_pointee_base_out)
{
  if(!target_pointer ||
     !source_pointer ||
     target_pointer->kind != Type::TK_POINTER ||
     source_pointer->kind != Type::TK_POINTER) {
    return false;
  }

  TypePtr target_pointee_base;
  TypePtr source_pointee_base;
  bool target_const = false;
  bool target_volatile = false;
  bool source_const = false;
  bool source_volatile = false;
  if(!top_level_cv_flags(target_pointer->inner,
                         target_pointee_base,
                         target_const,
                         target_volatile) ||
     !top_level_cv_flags(source_pointer->inner,
                         source_pointee_base,
                         source_const,
                         source_volatile) ||
     (source_const && !target_const) ||
     (source_volatile && !target_volatile)) {
    return false;
  }

  if(target_pointee_base_out) {
    *target_pointee_base_out = target_pointee_base;
  }
  if(source_pointee_base_out) {
    *source_pointee_base_out = source_pointee_base;
  }
  return true;
}

bool is_class_or_union_object_type(const TypePtr & type)
{
  TypePtr base = strip_top_level_cv(type);
  if(!base || base->kind != Type::TK_NAMED) {
    return false;
  }
  return base->named_key.compare(0, 6, "class ") == 0 ||
         base->named_key.compare(0, 7, "struct ") == 0 ||
         base->named_key.compare(0, 6, "union ") == 0;
}

bool reference_referent_accepts_temporary(const TypePtr & referent)
{
  return referent && type_is_const_object(referent);
}

bool nonvolatile_const_object_parameter(const TypePtr & implicit_object_parameter)
{
  TypePtr parameter = strip_top_level_cv(implicit_object_parameter);
  if(parameter && parameter->kind == Type::TK_POINTER) {
    parameter = parameter->inner;
  }
  TypePtr object_base;
  bool object_const = false;
  bool object_volatile = false;
  return top_level_cv_flags(parameter, object_base, object_const, object_volatile) &&
         object_const &&
         !object_volatile;
}

bool is_scoped_enum_key(const std::string & key)
{
  return key.rfind("enum class ", 0) == 0 || key.rfind("enum struct ", 0) == 0;
}

bool is_unscoped_enum_type_impl(const TypePtr & type)
{
  TypePtr base = strip_top_level_cv(type);
  return base && base->kind == Type::TK_NAMED && base->named_key.rfind("enum ", 0) == 0 &&
         !is_scoped_enum_key(base->named_key);
}

bool reference_target_accepts_result_category(const TypePtr & target,
                                              ValueCategory result_category)
{
  TypePtr base = strip_top_level_cv(target);
  if(!base) {
    return false;
  }
  if(base->kind == Type::TK_LVALUE_REFERENCE) {
    return result_category == VC_LVALUE ||
           reference_referent_accepts_temporary(base->inner);
  }
  if(base->kind == Type::TK_RVALUE_REFERENCE) {
    return result_category != VC_LVALUE;
  }
  return true;
}

TypePtr reference_binding_converted_pointer_target(const TypePtr & target,
                                                   const ExprInfo & expr)
{
  TypePtr target_base = strip_top_level_cv(target);
  if(!target_base) {
    return TypePtr();
  }

  const bool const_lvalue_reference =
      target_base->kind == Type::TK_LVALUE_REFERENCE &&
      reference_referent_accepts_temporary(target_base->inner);
  const bool rvalue_reference =
      target_base->kind == Type::TK_RVALUE_REFERENCE &&
      expr.category != VC_LVALUE;
  if(!const_lvalue_reference && !rvalue_reference) {
    return TypePtr();
  }

  TypePtr referred_base = strip_top_level_cv(target_base->inner);
  if(!referred_base ||
     (referred_base->kind != Type::TK_POINTER &&
      referred_base->kind != Type::TK_MEMBER_POINTER)) {
    return TypePtr();
  }
  return referred_base;
}

ExprInfo unwrap_atomic_value_expr(const ExprInfo & expr)
{
  ExprInfo result = expr;
  TypePtr converted = value_conversion_type(expr);
  TypePtr converted_base = strip_top_level_cv(converted);
  if(converted_base && converted_base->kind == Type::TK_ATOMIC) {
    result.type = converted_base->inner;
    result.category = VC_PRVALUE;
  }
  return result;
}

void collect_conversion_function_names(SemanticContext & ctx,
                                       ClassInfo & info,
                                       set<ClassInfo *> & visited,
                                       set<ClassInfo *> & visited_virtual,
                                       set<string> & out)
{
  if(!visited.insert(&info).second) {
    return;
  }

  for(map<string, vector<FunctionBinding *> >::const_iterator it = info.methods.begin();
      it != info.methods.end(); ++it) {
    if(ctx.is_conversion_function_name(it->first)) {
      out.insert(it->first);
    }
  }

  if(info.member_scope) {
    for(map<string, vector<FunctionTemplateDecl *> >::const_iterator it =
            info.member_scope->function_templates.begin();
        it != info.member_scope->function_templates.end();
        ++it) {
      if(ctx.is_conversion_function_name(it->first) && !it->second.empty()) {
        out.insert(it->first);
      }
    }
  }

  for(size_t i = 0; i < info.bases.size(); ++i) {
    BaseInfo & base = info.bases[i];
    if(base.is_virtual && !visited_virtual.insert(base.type).second) {
      continue;
    }
    collect_conversion_function_names(ctx, *base.type, visited, visited_virtual, out);
  }
}

TypePtr conversion_function_result_type(FunctionBinding * binding)
{
  if(!binding) {
    return TypePtr();
  }
  TypePtr function_type = strip_top_level_cv(binding->type);
  if(!function_type || function_type->kind != Type::TK_FUNCTION) {
    function_type = strip_top_level_cv(binding->declared_type);
  }
  if(!function_type || function_type->kind != Type::TK_FUNCTION) {
    return TypePtr();
  }
  return function_type->inner;
}

bool conversion_result_type_seen(const vector<TypePtr> & seen,
                                 const TypePtr & result_type)
{
  for(size_t i = 0; i < seen.size(); ++i) {
    if(type_equals(seen[i], result_type)) {
      return true;
    }
  }
  return false;
}

struct VisibleConversionFunctionGroup
{
  MemberFunctionLookupResult result;
  TypePtr result_type;
};

void append_direct_conversion_function_groups(
    SemanticContext & ctx,
    ClassInfo & info,
    vector<VisibleConversionFunctionGroup> & out,
    vector<TypePtr> & direct_result_types)
{
  set<FunctionBinding *> seen_bindings;
  const auto append_binding = [&](FunctionBinding * binding)
  {
    // Function-template specializations are retained in the class method set
    // for output and definition acquisition, but they are not independent
    // declarations found by conversion-function lookup. The primary template
    // is considered separately against the current target type below.
    if(!binding || binding->source_template ||
       !seen_bindings.insert(binding).second) {
      return;
    }
    TypePtr result_type = conversion_function_result_type(binding);
    if(!result_type) {
      return;
    }
    MemberFunctionLookupResult result;
    result.functions.push_back(binding);
    result.declared_in = &info;
    result.path_access = MA_PUBLIC;
    result.path_offset = 0;

    VisibleConversionFunctionGroup group;
    group.result = result;
    group.result_type = result_type;
    out.push_back(group);
    if(!conversion_result_type_seen(direct_result_types, result_type)) {
      direct_result_types.push_back(result_type);
    }
  };

  for(map<string, vector<FunctionBinding *> >::const_iterator it = info.methods.begin();
      it != info.methods.end(); ++it) {
    if(!ctx.is_conversion_function_name(it->first)) {
      continue;
    }
    for(size_t i = 0; i < it->second.size(); ++i) {
      append_binding(it->second[i]);
    }
  }

  if(info.member_scope) {
    for(map<string, vector<FunctionBinding *> >::const_iterator it =
            info.member_scope->function_sets.begin();
        it != info.member_scope->function_sets.end();
        ++it) {
      if(!ctx.is_conversion_function_name(it->first)) {
        continue;
      }
      for(size_t i = 0; i < it->second.size(); ++i) {
        append_binding(it->second[i]);
      }
    }
  }
}

void collect_visible_conversion_function_groups(
    SemanticContext & ctx,
    ClassInfo & info,
    set<ClassInfo *> & visited,
    set<ClassInfo *> & visited_virtual,
    vector<VisibleConversionFunctionGroup> & out)
{
  if(!visited.insert(&info).second) {
    return;
  }

  vector<TypePtr> direct_result_types;
  append_direct_conversion_function_groups(ctx, info, out, direct_result_types);

  for(size_t i = 0; i < info.bases.size(); ++i) {
    BaseInfo & base = info.bases[i];
    if(!base.type) {
      continue;
    }
    if(base.is_virtual && !visited_virtual.insert(base.type).second) {
      continue;
    }

    vector<VisibleConversionFunctionGroup> base_groups;
    collect_visible_conversion_function_groups(ctx,
                                               *base.type,
                                               visited,
                                               visited_virtual,
                                               base_groups);
    for(size_t j = 0; j < base_groups.size(); ++j) {
      if(conversion_result_type_seen(direct_result_types,
                                     base_groups[j].result_type)) {
        continue;
      }
      base_groups[j].result.path_access =
          combine_member_access(base.access, base_groups[j].result.path_access);
      base_groups[j].result.path_offset += base.offset;
      out.push_back(base_groups[j]);
    }
  }
}

vector<MemberFunctionLookupResult> collect_visible_conversion_functions(SemanticContext & ctx,
                                                                       ClassInfo & info)
{
  set<ClassInfo *> visited;
  set<ClassInfo *> visited_virtual;
  vector<VisibleConversionFunctionGroup> groups;
  collect_visible_conversion_function_groups(ctx,
                                             info,
                                             visited,
                                             visited_virtual,
                                             groups);

  vector<MemberFunctionLookupResult> out;
  out.reserve(groups.size());
  for(size_t i = 0; i < groups.size(); ++i) {
    if(!groups[i].result.functions.empty()) {
      out.push_back(groups[i].result);
    }
  }
  return out;
}

vector<MemberFunctionTemplateLookupResult> collect_visible_conversion_function_templates(
    SemanticContext & ctx,
    ClassInfo & info)
{
  set<ClassInfo *> visited;
  set<ClassInfo *> visited_virtual;
  set<string> names;
  collect_conversion_function_names(ctx, info, visited, visited_virtual, names);

  vector<MemberFunctionTemplateLookupResult> out;
  for(set<string>::const_iterator it = names.begin(); it != names.end(); ++it) {
    MemberFunctionTemplateLookupResult visible =
        lookup_visible_member_function_templates(info, *it);
    if(!visible.templates.empty()) {
      out.push_back(visible);
    }
  }
  return out;
}

TypePtr build_conversion_function_template_target_type(SemanticContext & ctx,
                                                       Scope & use_scope,
                                                       FunctionTemplateDecl & decl,
                                                       const TypePtr & target)
{
  TypePtr function_type = strip_top_level_cv(decl.type_pattern);
  if(!function_type || function_type->kind != Type::TK_FUNCTION) {
    return TypePtr();
  }

  vector<TypePtr> params;
  params.reserve(function_type->params.size());
  for(size_t i = 0; i < function_type->params.size(); ++i) {
    TypePtr resolved = function_type->params[i];
    if(function_type->params[i] &&
       semantic_dependent_type::resolve_instantiated_dependent_type(ctx, use_scope, function_type->params[i], resolved) &&
       resolved) {
      params.push_back(resolved);
    } else {
      params.push_back(function_type->params[i]);
    }
  }

  return make_function(target,
                       params,
                       function_type->variadic,
                       function_type->function_const,
                       function_type->function_volatile,
                       function_type->prototype_relaxed,
                       function_type->function_ref_qualifier);
}

TypePtr conversion_function_template_deduction_target_type(const TypePtr & target)
{
  TypePtr base = strip_top_level_cv(target);
  if(base &&
     (base->kind == Type::TK_LVALUE_REFERENCE ||
      base->kind == Type::TK_RVALUE_REFERENCE)) {
    return target;
  }
  return base ? base : target;
}

vector<TypePtr> conversion_function_template_deduction_target_types(
    const TypePtr & target)
{
  vector<TypePtr> targets;
  if(target) {
    targets.push_back(conversion_function_template_deduction_target_type(target));
  }

  TypePtr base = strip_top_level_cv(target);
  if(base &&
     (base->kind == Type::TK_LVALUE_REFERENCE ||
      base->kind == Type::TK_RVALUE_REFERENCE)) {
    TypePtr object_target = strip_top_level_cv(base->inner);
    if(object_target) {
      bool duplicate = false;
      for(size_t i = 0; i < targets.size(); ++i) {
        if(type_equals(targets[i], object_target)) {
          duplicate = true;
          break;
        }
      }
      if(!duplicate) {
        targets.push_back(object_target);
      }
    }
  }

  return targets;
}

void update_conversion_function_template_binding_result(
    SemanticContext & ctx,
    FunctionBinding & binding,
    const TypePtr & result_type)
{
  TypePtr function_type = strip_top_level_cv(binding.type);
  if(!result_type ||
     !function_type ||
     function_type->kind != Type::TK_FUNCTION ||
     !ctx.type_depends_on_template_parameter(function_type->inner)) {
    return;
  }

  binding.type = make_function(result_type,
                               function_type->params,
                               function_type->variadic,
                               function_type->function_const,
                               function_type->function_volatile,
                               function_type->prototype_relaxed,
                               function_type->function_ref_qualifier);
}

ClassInfo * ensure_complete_class_info(SemanticContext & ctx, const TypePtr & type)
{
  ClassInfo * info = ctx.class_info_for_type(type);
  if(info && info->complete) {
    return info;
  }
  return ctx.complete_class_type(type);
}

void ensure_reference_inheritance_graph(SemanticContext & ctx,
                                        ClassInfo & info,
                                        set<ClassInfo *> & visited)
{
  if(!visited.insert(&info).second) {
    return;
  }
  // Speculative overload screening still needs transitive base paths, but it
  // must not force full class-member materialization.
  ctx.ensure_class_reference_type_members(info);
  for(size_t i = 0; i < info.bases.size(); ++i) {
    if(info.bases[i].type) {
      ensure_reference_inheritance_graph(ctx, *info.bases[i].type, visited);
    }
  }
}

ClassInfo * class_info_for_inheritance_conversion(SemanticContext & ctx,
                                                  const TypePtr & type,
                                                  bool materialize)
{
  if(materialize) {
    return ensure_complete_class_info(ctx, type);
  }

  ClassInfo * info = ctx.class_info_for_type(type);
  if(!info) {
    return ctx.complete_class_type(type);
  }
  if(!info->complete &&
     !info->reference_type_members_collected &&
     !info->reference_members_collected &&
     !info->reference_type_member_collection_in_progress &&
     !info->reference_member_collection_in_progress &&
     !info->full_member_collection_in_progress) {
    ctx.ensure_class_reference_type_members(*info);
    if(ClassInfo * refreshed = ctx.class_info_for_type(type)) {
      info = refreshed;
    }
  }
  if(info) {
    set<ClassInfo *> visited;
    ensure_reference_inheritance_graph(ctx, *info, visited);
  }
  return info;
}

bool binding_declares_explicit_function(const FunctionBinding & binding)
{
  if(binding.is_explicit) {
    return true;
  }

  const auto node_declares_explicit =
      [](const CppAstNode * node) -> bool
      {
        if(!node) {
          return false;
        }
        const CppAstNode * specifiers = find_child(*node, CppAstKind::member_specifiers);
        if(!specifiers) {
          specifiers = find_child(*node, CppAstKind::decl_specifier_seq);
        }
        if(specifiers && decl_spec_contains_token(*specifiers, KW_EXPLICIT)) {
          return true;
        }
        if(!specifiers) {
          return false;
        }
        for(size_t i = 0; i < specifiers->children.size(); ++i) {
          if((specifiers->children[i].kind == CppAstKind::decl_specifier ||
              specifiers->children[i].kind == CppAstKind::specifier) &&
             specifiers->children[i].value == "explicit") {
            return true;
          }
        }
        return false;
      };

  return node_declares_explicit(binding.declaration_node) ||
         node_declares_explicit(binding.definition_node);
}

}  // namespace

bool try_builtin_pointer_operand_conversion(SemanticContext & ctx,
                                            Scope & scope,
                                            const ExprInfo & expr,
                                            ExprInfo & out,
                                            TypePtr & pointer_type,
                                            const ArgumentConversionOptions & options)
{
  pointer_type = TypePtr();
  if(!expr.type) {
    return false;
  }

  TypePtr source_class_type = strip_top_level_cv(remove_reference_type(expr.type));
  ClassInfo * source_class = ensure_complete_class_info(ctx, source_class_type);
  if(!source_class) {
    return false;
  }

  vector<TypePtr> target_pointer_types;
  const auto append_target =
      [&](const TypePtr & result_type)
      {
        TypePtr result_base = strip_top_level_cv(result_type);
        if(!result_base || result_base->kind != Type::TK_POINTER) {
          return;
        }
        for(size_t i = 0; i < target_pointer_types.size(); ++i) {
          if(type_equals(target_pointer_types[i], result_base)) {
            return;
          }
        }
        target_pointer_types.push_back(result_base);
      };

  if(source_class->is_lambda_closure && source_class->fields.empty()) {
    map<string, vector<FunctionBinding *> >::const_iterator call_operator =
        source_class->methods.find("operator()");
    if(call_operator != source_class->methods.end()) {
      for(size_t i = 0; i < call_operator->second.size(); ++i) {
        FunctionBinding * binding = call_operator->second[i];
        if(binding && binding->declared_type) {
          append_target(make_pointer(binding->declared_type));
        }
      }
    }
  }

  vector<MemberFunctionLookupResult> conversion_sets =
      collect_visible_conversion_functions(ctx, *source_class);
  for(size_t set_index = 0; set_index < conversion_sets.size(); ++set_index) {
    const MemberFunctionLookupResult & visible = conversion_sets[set_index];
    for(size_t i = 0; i < visible.functions.size(); ++i) {
      append_target(conversion_function_result_type(visible.functions[i]));
    }
  }
  if(target_pointer_types.empty()) {
    return false;
  }

  struct Candidate
  {
    ExprInfo converted;
    TypePtr pointer_type;
    ConversionRank rank = CR_BAD;
  };

  vector<Candidate> candidates;
  for(size_t i = 0; i < target_pointer_types.size(); ++i) {
    ExprInfo converted;
    ConversionRank rank = CR_BAD;
    if(!try_argument_conversion(ctx,
                                scope,
                                target_pointer_types[i],
                                expr,
                                converted,
                                rank,
                                options)) {
      continue;
    }
    TypePtr converted_type = strip_top_level_cv(value_conversion_type(converted));
    if(!converted_type || converted_type->kind != Type::TK_POINTER) {
      continue;
    }
    bool duplicate = false;
    for(size_t j = 0; j < candidates.size(); ++j) {
      if(type_equals(candidates[j].pointer_type, converted_type)) {
        duplicate = true;
        break;
      }
    }
    if(duplicate) {
      continue;
    }

    Candidate candidate;
    candidate.converted = converted;
    candidate.pointer_type = converted_type;
    candidate.rank = rank;
    candidates.push_back(candidate);
  }
  if(candidates.empty()) {
    return false;
  }

  size_t best = 0;
  bool ambiguous = false;
  for(size_t i = 1; i < candidates.size(); ++i) {
    bool current_better = false;
    bool best_better = false;
    if(candidates[i].rank < candidates[best].rank) {
      current_better = true;
    } else if(candidates[i].rank > candidates[best].rank) {
      best_better = true;
    } else {
      int qual_pref = compare_parameter_qualification_preference(
          candidates[i].pointer_type,
          candidates[best].pointer_type);
      if(qual_pref < 0) {
        current_better = true;
      } else if(qual_pref > 0) {
        best_better = true;
      }
    }

    if(current_better && !best_better) {
      best = i;
      ambiguous = false;
    } else if((current_better && best_better) ||
              (!current_better && !best_better)) {
      ambiguous = true;
    }
  }
  if(ambiguous) {
    return false;
  }

  out = candidates[best].converted;
  pointer_type = candidates[best].pointer_type;
  return true;
}

bool member_pointer_exact_qualification_compatible(const TypePtr & target,
                                                   const TypePtr & source);
bool member_pointer_inheritance_conversion(SemanticContext & ctx,
                                           const TypePtr & target,
                                           const TypePtr & source,
                                           size_t * out_offset);

bool ref_qualifier_accepts_implicit_object(RefQualifier ref_qualifier,
                                           const TypePtr & implicit_object_parameter,
                                           ValueCategory category)
{
  switch(ref_qualifier) {
  case RQ_NONE:
    return true;
  case RQ_LVALUE:
    return category == VC_LVALUE ||
           nonvolatile_const_object_parameter(implicit_object_parameter);
  case RQ_RVALUE:
    return category == VC_PRVALUE || category == VC_XVALUE;
  }

  return false;
}

bool member_pointer_ref_qualifier_rejects_object(
    FunctionTypeRefQualifier qualifier,
    ValueCategory category)
{
  switch(qualifier) {
  case FTRQ_NONE: return false;
  case FTRQ_LVALUE: return category != VC_LVALUE;
  case FTRQ_RVALUE: return category == VC_LVALUE;
  }
  return false;
}

TypePtr value_conversion_type(const ExprInfo & expr)
{
  if(!expr.type) {
    return TypePtr();
  }

  TypePtr base = strip_top_level_cv(expr.type);
  if(base->kind == Type::TK_LVALUE_REFERENCE ||
     base->kind == Type::TK_RVALUE_REFERENCE) {
    base = strip_top_level_cv(base->inner);
  }

  if(base->kind == Type::TK_ARRAY && expr.category != VC_PRVALUE) {
    return make_pointer(base->inner);
  }
  if(base->kind == Type::TK_FUNCTION && expr.category == VC_LVALUE) {
    return make_pointer(base);
  }
  return base;
}

ConversionRank standard_conversion_rank_non_reference(const TypePtr & target,
                                                      const ExprInfo & expr)
{
  if(!target || !expr.type) {
    return CR_BAD;
  }

  TypePtr target_base = strip_top_level_cv(target);
  TypePtr source_type = remove_reference_type(expr.type);
  if(target_base &&
     target_base->kind == Type::TK_ARRAY &&
     expr.category != VC_PRVALUE &&
     same_type_with_compatible_top_cv(target, source_type)) {
    return CR_EXACT;
  }

  TypePtr converted = value_conversion_type(expr);
  if(!converted) {
    return CR_BAD;
  }

  if(type_equals(target_base, converted)) {
    return CR_EXACT;
  }

  if(target_base->kind == Type::TK_ATOMIC) {
    return standard_conversion_rank_non_reference(target_base->inner,
                                                  unwrap_atomic_value_expr(expr));
  }

  TypePtr converted_base = strip_top_level_cv(converted);

  if(target_base->kind == Type::TK_FUNDAMENTAL &&
     target_base->fundamental == FT_NULLPTR_T &&
     expr.null_pointer_constant) {
    return CR_CONVERSION;
  }

  if(target_base->kind == Type::TK_FUNDAMENTAL &&
     target_base->fundamental == FT_BOOL) {
    if(expr.null_pointer_constant ||
       (converted_base && converted_base->kind == Type::TK_FUNDAMENTAL &&
        converted_base->fundamental == FT_NULLPTR_T) ||
       is_nullable_pointer_like_type(converted_base)) {
      return CR_CONVERSION;
    }
  }

  TypePtr promoted = promoted_integral_type(converted);
  if(promoted && type_equals(target_base, promoted)) {
    return CR_PROMOTION;
  }

  if(is_floating_type(target_base) && is_floating_type(converted)) {
    TypePtr converted_base = strip_top_level_cv(converted);
    if(converted_base && converted_base->kind == Type::TK_FUNDAMENTAL &&
       target_base->kind == Type::TK_FUNDAMENTAL &&
       converted_base->fundamental == FT_FLOAT &&
       target_base->fundamental == FT_DOUBLE) {
      return CR_PROMOTION;
    }
    return CR_CONVERSION;
  }

  if(is_integral_type(target_base) && is_integral_or_unscoped_enum_type(converted)) {
    return CR_CONVERSION;
  }

  if((is_integral_type(target_base) && is_floating_type(converted)) ||
     (is_floating_type(target_base) && is_integral_or_unscoped_enum_type(converted))) {
    return CR_CONVERSION;
  }

  if(is_nullable_pointer_like_type(target_base)) {
    if(expr.null_pointer_constant ||
       (converted_base && converted_base->kind == Type::TK_FUNDAMENTAL &&
        converted_base->fundamental == FT_NULLPTR_T)) {
      return CR_CONVERSION;
    }
    if(converted_base && converted_base->kind == Type::TK_MEMBER_POINTER) {
      if(member_pointer_exact_qualification_compatible(target_base, converted_base)) {
        return CR_EXACT;
      }
    }
    if(converted_base && converted_base->kind == Type::TK_POINTER) {
      if(same_type_with_compatible_top_cv(target_base, converted_base)) {
        return CR_EXACT;
      }
      TypePtr target_pointee;
      TypePtr actual_pointee;
      if(pointer_pointee_cv_allows_base_conversion(target_base,
                                                   converted_base,
                                                   &target_pointee,
                                                   &actual_pointee) &&
         is_void_type(target_pointee) &&
         actual_pointee->kind != Type::TK_FUNCTION) {
        return CR_CONVERSION;
      }
    }
  }

  return CR_BAD;
}

ConversionRank standard_conversion_rank(const TypePtr & target, const ExprInfo & expr)
{
  if(!target || !expr.type) {
    return CR_BAD;
  }

  TypePtr base_target = strip_top_level_cv(target);
  if(base_target->kind == Type::TK_LVALUE_REFERENCE) {
    TypePtr source_type = reference_binding_source_type(expr);
    if((expr.category == VC_LVALUE ||
        reference_referent_accepts_temporary(base_target->inner)) &&
       same_type_with_compatible_top_cv(base_target->inner, source_type)) {
      return CR_EXACT;
    }

    TypePtr referred_base = strip_top_level_cv(base_target->inner);
    if(reference_referent_accepts_temporary(base_target->inner) &&
       base_target->inner != referred_base &&
       base_target->inner->kind == Type::TK_CV) {
      ConversionRank rank = standard_conversion_rank_non_reference(referred_base, expr);
      if(expr.category == VC_LVALUE && rank == CR_EXACT) {
        // This path binds through a converted temporary, not a direct
        // reference-compatible binding, so it must stay worse than an actual
        // cv-correct lvalue reference match.
        rank = CR_CONVERSION;
      }
      return rank == CR_BAD ? CR_BAD : rank;
    }

    return CR_BAD;
  }

  if(base_target->kind == Type::TK_RVALUE_REFERENCE) {
    TypePtr source_type = reference_binding_source_type(expr);
    if(reference_referents_are_same_ignoring_top_cv(base_target->inner, source_type)) {
      if(expr.category == VC_LVALUE) {
        return CR_BAD;
      }
      if(!same_type_with_compatible_top_cv(base_target->inner, source_type)) {
        return CR_BAD;
      }
      return CR_EXACT;
    }
    if(is_class_or_union_object_type(source_type)) {
      return CR_BAD;
    }
    return standard_conversion_rank_non_reference(base_target->inner, expr);
  }

  return standard_conversion_rank_non_reference(target, expr);
}

bool try_semantic_exact_reference_binding(SemanticContext & ctx,
                                          const TypePtr & target,
                                          const ExprInfo & expr,
                                          ExprInfo & out,
                                          ConversionRank & rank)
{
  rank = CR_BAD;
  if(!target || !expr.type) {
    return false;
  }

  TypePtr base_target = strip_top_level_cv(target);
  if(!base_target ||
     (base_target->kind != Type::TK_LVALUE_REFERENCE &&
      base_target->kind != Type::TK_RVALUE_REFERENCE)) {
    return false;
  }

  TypePtr source_type = reference_binding_source_type(expr);
  bool binds = false;
  if(base_target->kind == Type::TK_LVALUE_REFERENCE) {
    binds =
        (expr.category == VC_LVALUE ||
         reference_referent_accepts_temporary(base_target->inner)) &&
        same_type_with_compatible_top_cv_for_semantic_identity(ctx,
                                                              base_target->inner,
                                                              source_type);
  } else {
    binds =
        expr.category != VC_LVALUE &&
        same_type_with_compatible_top_cv_for_semantic_identity(ctx,
                                                              base_target->inner,
                                                              source_type);
  }

  if(!binds) {
    return false;
  }

  out = expr;
  rank = CR_EXACT;
  return true;
}

static void set_standard_converted_prvalue_result(SemanticContext & ctx,
                                                  const TypePtr & converted_type,
                                                  const ExprInfo & expr,
                                                  ExprInfo & out)
{
  out = expr;
  out.type = converted_type;
  out.category = VC_PRVALUE;
  out.null_pointer_constant = false;
  out.node = make_dump_node(CallSemKind::cast_expression);
  ctx.set_expr_info_metadata(out, out.type, out.category);
  set_callsem_conversion_source_type(out.node, expr.type);
  out.node.children.push_back(expr.node);
}

void apply_standard_conversion_result_metadata(SemanticContext & ctx,
                                               const TypePtr & target,
                                               const ExprInfo & expr,
                                               ExprInfo & out)
{
  out = expr;
  TypePtr target_base = strip_top_level_cv(target);
  if(target_base &&
     (target_base->kind == Type::TK_LVALUE_REFERENCE ||
      target_base->kind == Type::TK_RVALUE_REFERENCE)) {
    TypePtr referent_type = target_base->inner;
    if(target_base->kind == Type::TK_LVALUE_REFERENCE) {
      TypePtr source_type = reference_binding_source_type(expr);
      const bool direct_binding =
          (expr.category == VC_LVALUE ||
           reference_referent_accepts_temporary(referent_type)) &&
          same_type_with_compatible_top_cv(referent_type, source_type);
      TypePtr referred_base = strip_top_level_cv(referent_type);
      if(!direct_binding &&
         reference_referent_accepts_temporary(referent_type) &&
         referred_base &&
         standard_conversion_rank_non_reference(referred_base, expr) != CR_BAD) {
        set_standard_converted_prvalue_result(ctx, referent_type, expr, out);
      }
    } else {
      TypePtr source_type = reference_binding_source_type(expr);
      const bool direct_binding =
          expr.category != VC_LVALUE &&
          reference_referents_are_same_ignoring_top_cv(referent_type, source_type);
      if(!direct_binding &&
         standard_conversion_rank_non_reference(referent_type, expr) != CR_BAD) {
        set_standard_converted_prvalue_result(ctx, referent_type, expr, out);
      }
    }
    target_base = strip_top_level_cv(referent_type);
  }

  TypePtr expr_base = strip_top_level_cv(remove_reference_type(expr.type));
  if(target_base && target_base->kind == Type::TK_FUNDAMENTAL &&
     target_base->fundamental == FT_NULLPTR_T &&
     expr.null_pointer_constant && expr_base && is_integral_type(expr_base)) {
    ctx.set_expr_info_metadata(out, target_base, VC_PRVALUE);
  }
  if(target_base && is_nullable_pointer_like_type(target_base) &&
     expr.null_pointer_constant && expr_base && is_integral_type(expr_base)) {
    ctx.set_expr_info_metadata(out, target_base, out.category);
  }
}

ConversionRank inheritance_conversion_rank(SemanticContext & ctx,
                                           const TypePtr & target,
                                           const ExprInfo & arg)
{
  TypePtr target_base = strip_top_level_cv(target);
  TypePtr arg_base = strip_top_level_cv(remove_reference_type(arg.type));
  TypePtr arg_object_type = reference_binding_source_type(arg);
  if(!target_base || !arg_base) {
    return CR_BAD;
  }

  if(target_base->kind == Type::TK_LVALUE_REFERENCE &&
     (arg.category == VC_LVALUE ||
      reference_referent_accepts_temporary(target_base->inner))) {
    if(!top_level_cv_allows_reference_binding(target_base->inner, arg_object_type)) {
      return CR_BAD;
    }
    ClassInfo * target_class =
        ensure_complete_class_info(ctx, strip_top_level_cv(target_base->inner));
    ClassInfo * arg_class = ensure_complete_class_info(ctx, arg_object_type);
    if(target_class && arg_class && is_same_or_derived(arg_class, target_class)) {
      return CR_CONVERSION;
    }
  }

  if(target_base->kind == Type::TK_RVALUE_REFERENCE && arg.category != VC_LVALUE) {
    if(!top_level_cv_allows_reference_binding(target_base->inner, arg_object_type)) {
      return CR_BAD;
    }
    ClassInfo * target_class =
        ensure_complete_class_info(ctx, strip_top_level_cv(target_base->inner));
    ClassInfo * arg_class = ensure_complete_class_info(ctx, arg_object_type);
    if(target_class && arg_class && is_same_or_derived(arg_class, target_class)) {
      return CR_CONVERSION;
    }
  }

  if(target_base->kind == Type::TK_POINTER && arg_base->kind == Type::TK_POINTER) {
    TypePtr target_pointee_base;
    TypePtr arg_pointee_base;
    if(pointer_pointee_cv_allows_base_conversion(target_base,
                                                 arg_base,
                                                 &target_pointee_base,
                                                 &arg_pointee_base)) {
      ClassInfo * target_class = ctx.class_info_for_type(target_pointee_base);
      ClassInfo * arg_class = ctx.class_info_for_type(arg_pointee_base);
      if(!target_class) {
        target_class = ensure_complete_class_info(ctx, target_pointee_base);
      }
      if(!arg_class) {
        arg_class = ensure_complete_class_info(ctx, arg_pointee_base);
      }
      if(target_class && arg_class && is_same_or_derived(arg_class, target_class)) {
        return CR_CONVERSION;
      }
    }
  }

  if(target_base->kind == Type::TK_MEMBER_POINTER &&
     arg_base->kind == Type::TK_MEMBER_POINTER &&
     member_pointer_inheritance_conversion(ctx, target_base, arg_base, nullptr)) {
    return CR_CONVERSION;
  }

  if(target_base->kind == Type::TK_NAMED) {
    ClassInfo * target_class = ensure_complete_class_info(ctx, target_base);
    ClassInfo * arg_class = ensure_complete_class_info(ctx, arg_object_type);
    if(target_class && arg_class && is_same_or_derived(arg_class, target_class)) {
      return CR_CONVERSION;
    }
  }
  return CR_BAD;
}

ConversionRank conversion_rank(SemanticContext & ctx,
                               const TypePtr & target,
                               const ExprInfo & arg)
{
  ConversionRank rank = standard_conversion_rank(target, arg);
  if(rank != CR_BAD) {
    return rank;
  }
  ExprInfo ignored;
  if(try_semantic_exact_reference_binding(ctx, target, arg, ignored, rank)) {
    return rank;
  }
  if(target &&
     arg.type &&
     (ctx.type_depends_on_template_parameter(target) ||
      ctx.type_depends_on_template_parameter(arg.type))) {
    if(!dependent_conversion_candidate_can_bind_known_argument(ctx, target, arg)) {
      return CR_BAD;
    }
    return CR_CONVERSION;
  }
  TypePtr reference_conversion_target =
      reference_binding_converted_pointer_target(target, arg);
  if(reference_conversion_target) {
    ConversionRank rank =
        inheritance_conversion_rank(ctx, reference_conversion_target, arg);
    if(rank != CR_BAD) {
      return rank;
    }
  }
  return inheritance_conversion_rank(ctx, target, arg);
}

bool can_copy_initialize(SemanticContext & ctx,
                         const TypePtr & target,
                         const ExprInfo & expr)
{
  return conversion_rank(ctx, target, expr) != CR_BAD;
}

bool supports_non_reference_explicit_cast(SemanticContext & ctx,
                                          const TypePtr & target,
                                          const ExprInfo & expr,
                                          bool allow_reinterpret_like)
{
  TypePtr target_base = strip_top_level_cv(target);
  TypePtr operand_base = value_conversion_type(expr);
  if(!target_base || !operand_base) {
    return false;
  }

  bool supported =
      is_void_type(target_base) ||
      (is_integral_type(target_base) && is_integral_type(operand_base)) ||
      (is_integral_type(target_base) && is_named_enum_type(ctx, operand_base)) ||
      (is_named_enum_type(ctx, target_base) &&
       (is_integral_type(operand_base) || is_named_enum_type(ctx, operand_base))) ||
      (is_nullable_pointer_like_type(target_base) &&
       operand_base->kind == Type::TK_FUNDAMENTAL &&
       operand_base->fundamental == FT_NULLPTR_T) ||
      (is_pointer_type(target_base) && is_pointer_type(operand_base));
  if(!supported && allow_reinterpret_like) {
    supported =
        (is_integral_type(target_base) && is_pointer_type(operand_base)) ||
        (is_pointer_type(target_base) &&
         (is_integral_type(operand_base) || is_named_enum_type(ctx, operand_base)));
  }
  return supported;
}

bool supports_reinterpret_like_reference_cast(const TypePtr & target,
                                              const ExprInfo & expr)
{
  TypePtr target_base = strip_top_level_cv(target);
  if(!target_base ||
     (target_base->kind != Type::TK_LVALUE_REFERENCE &&
      target_base->kind != Type::TK_RVALUE_REFERENCE) ||
     !target_base->inner ||
     is_void_type(target_base->inner) ||
     is_function_type(target_base->inner)) {
    return false;
  }

  if(target_base->kind == Type::TK_LVALUE_REFERENCE) {
    return expr.category == VC_LVALUE;
  }
  return expr.category != VC_PRVALUE;
}

bool top_level_cv_flags(const TypePtr & type,
                        TypePtr & base,
                        bool & cv_const,
                        bool & cv_volatile)
{
  if(!type) {
    return false;
  }

  if(type->kind == Type::TK_CV) {
    base = type->inner;
    cv_const = type->cv_const;
    cv_volatile = type->cv_volatile;
    return true;
  }

  base = type;
  cv_const = false;
  cv_volatile = false;
  return true;
}

struct QualificationLevel
{
  Type::Kind kind = Type::TK_POINTER;
  TypePtr owner;
  bool cv_const = false;
  bool cv_volatile = false;
};

bool same_array_type_with_compatible_element_cv(const TypePtr & target,
                                                const TypePtr & source)
{
  TypePtr target_base = strip_top_level_cv(target);
  TypePtr source_base = strip_top_level_cv(source);
  if(!target_base ||
     !source_base ||
     target_base->kind != Type::TK_ARRAY ||
     source_base->kind != Type::TK_ARRAY ||
     target_base->has_bound != source_base->has_bound ||
     target_base->bound != source_base->bound ||
     target_base->bound_text != source_base->bound_text) {
    return false;
  }
  return same_type_with_compatible_top_cv(target_base->inner, source_base->inner);
}

void fold_array_top_cv_into_element(TypePtr & base,
                                    bool & cv_const,
                                    bool & cv_volatile)
{
  if(!base || base->kind != Type::TK_ARRAY || (!cv_const && !cv_volatile)) {
    return;
  }
  base = apply_cv(base, cv_const, cv_volatile);
  cv_const = false;
  cv_volatile = false;
}

bool collect_pointer_qualification_signature(const TypePtr & type,
                                             std::vector<QualificationLevel> & levels,
                                             TypePtr & leaf)
{
  levels.clear();
  TypePtr current = strip_top_level_cv(type);
  while(current &&
        (current->kind == Type::TK_POINTER ||
         current->kind == Type::TK_MEMBER_POINTER)) {
    QualificationLevel level;
    level.kind = current->kind;
    if(current->kind == Type::TK_MEMBER_POINTER) {
      level.owner = strip_top_level_cv(current->owner);
    }
    TypePtr inner_base;
    if(!top_level_cv_flags(current->inner,
                           inner_base,
                           level.cv_const,
                           level.cv_volatile)) {
      return false;
    }
    levels.push_back(level);
    current = strip_top_level_cv(inner_base);
  }
  leaf = current;
  return leaf != nullptr;
}

bool pointer_qualification_conversion_compatible(const TypePtr & target,
                                                 const TypePtr & source)
{
  std::vector<QualificationLevel> target_levels;
  std::vector<QualificationLevel> source_levels;
  TypePtr target_leaf;
  TypePtr source_leaf;
  if(!collect_pointer_qualification_signature(target, target_levels, target_leaf) ||
     !collect_pointer_qualification_signature(source, source_levels, source_leaf) ||
     target_levels.size() != source_levels.size() ||
     (!type_equals(target_leaf, source_leaf) &&
      !same_array_type_with_compatible_element_cv(target_leaf, source_leaf))) {
    return false;
  }

  bool all_previous_target_const = true;
  for(std::size_t i = 0; i < target_levels.size(); ++i) {
    const QualificationLevel & target_level = target_levels[i];
    const QualificationLevel & source_level = source_levels[i];
    if(target_level.kind != source_level.kind) {
      return false;
    }
    if(target_level.kind == Type::TK_MEMBER_POINTER &&
       !type_equals(target_level.owner, source_level.owner)) {
      return false;
    }
    if((source_level.cv_const && !target_level.cv_const) ||
       (source_level.cv_volatile && !target_level.cv_volatile)) {
      return false;
    }
    const bool different =
        target_level.cv_const != source_level.cv_const ||
        target_level.cv_volatile != source_level.cv_volatile;
    if(i != 0 && different && !all_previous_target_const) {
      return false;
    }
    all_previous_target_const = all_previous_target_const && target_level.cv_const;
  }
  return true;
}

bool member_pointer_exact_qualification_compatible(const TypePtr & target,
                                                   const TypePtr & source)
{
  TypePtr target_base = strip_top_level_cv(target);
  TypePtr source_base = strip_top_level_cv(source);
  return target_base && source_base &&
         target_base->kind == Type::TK_MEMBER_POINTER &&
         source_base->kind == Type::TK_MEMBER_POINTER &&
         type_equals(strip_top_level_cv(target_base->owner),
                     strip_top_level_cv(source_base->owner)) &&
         same_type_with_compatible_top_cv(target_base->inner, source_base->inner);
}

bool member_pointer_inheritance_conversion(SemanticContext & ctx,
                                           const TypePtr & target,
                                           const TypePtr & source,
                                           size_t * out_offset)
{
  TypePtr target_base = strip_top_level_cv(target);
  TypePtr source_base = strip_top_level_cv(source);
  if(!target_base || !source_base ||
     target_base->kind != Type::TK_MEMBER_POINTER ||
     source_base->kind != Type::TK_MEMBER_POINTER ||
     !same_type_with_compatible_top_cv(target_base->inner, source_base->inner)) {
    return false;
  }

  ClassInfo * target_owner =
      ensure_complete_class_info(ctx, strip_top_level_cv(target_base->owner));
  ClassInfo * source_owner =
      ensure_complete_class_info(ctx, strip_top_level_cv(source_base->owner));
  size_t offset = 0;
  MemberAccess access = MA_PUBLIC;
  if(!target_owner || !source_owner || target_owner == source_owner ||
     !find_unique_base_path(*target_owner, source_owner, offset, access) ||
     access != MA_PUBLIC) {
    return false;
  }
  if(out_offset) {
    *out_offset = offset;
  }
  return true;
}

bool same_type_with_compatible_top_cv(const TypePtr & target, const TypePtr & source)
{
  TypePtr target_base;
  TypePtr source_base;
  bool target_const = false;
  bool target_volatile = false;
  bool source_const = false;
  bool source_volatile = false;

  if(!top_level_cv_flags(target, target_base, target_const, target_volatile) ||
     !top_level_cv_flags(source, source_base, source_const, source_volatile)) {
    return false;
  }
  fold_array_top_cv_into_element(target_base, target_const, target_volatile);
  fold_array_top_cv_into_element(source_base, source_const, source_volatile);

  if((source_const && !target_const) ||
     (source_volatile && !target_volatile)) {
    return false;
  }

  if(type_equals(target_base, source_base)) {
    return true;
  }

  if(same_array_type_with_compatible_element_cv(target_base, source_base)) {
    return true;
  }

  if(target_base && source_base &&
     (target_base->kind == Type::TK_POINTER ||
      target_base->kind == Type::TK_MEMBER_POINTER) &&
     (source_base->kind == Type::TK_POINTER ||
      source_base->kind == Type::TK_MEMBER_POINTER)) {
    return pointer_qualification_conversion_compatible(target_base, source_base);
  }

  return false;
}

bool class_value_transfer_prefers_nonconst_move(
    const TypePtr & target,
    const TypePtr & source,
    CallValueCategory source_category)
{
  if(source_category == CVC_NONE || source_category == CVC_LVALUE) {
    return false;
  }

  TypePtr target_object = strip_top_level_cv(remove_reference_type(target));
  TypePtr source_outer = strip_top_level_cv(source);
  TypePtr source_object =
      source_outer && is_reference_type(source_outer) ?
          remove_reference_type(source_outer) : source;
  return target_object &&
         source_object &&
         same_type_with_compatible_top_cv(target_object, source_object);
}

bool is_const_object_type(const TypePtr & type)
{
  TypePtr base;
  bool cv_const = false;
  bool cv_volatile = false;
  return top_level_cv_flags(type, base, cv_const, cv_volatile) && cv_const;
}


bool is_unscoped_enum_type(const TypePtr & type)
{
  return is_unscoped_enum_type_impl(type);
}

bool is_integral_or_unscoped_enum_type(const TypePtr & type)
{
  TypePtr base = strip_top_level_cv(type);
  return is_integral_type(base) || is_unscoped_enum_type_impl(base);
}

TypePtr promoted_integral_type(const TypePtr & type)
{
  TypePtr base = strip_top_level_cv(type);
  if(is_unscoped_enum_type_impl(base)) {
    return make_fundamental(FT_INT);
  }
  if(!base || base->kind != Type::TK_FUNDAMENTAL) {
    return TypePtr();
  }

  switch(base->fundamental) {
  case FT_BOOL:
  case FT_CHAR:
  case FT_SIGNED_CHAR:
  case FT_UNSIGNED_CHAR:
  case FT_SHORT_INT:
  case FT_UNSIGNED_SHORT_INT:
  case FT_CHAR16_T:
    return make_fundamental(FT_INT);
  case FT_WCHAR_T:
    if(type_is_signed(base->fundamental) ||
       type_to_size(base->fundamental) < type_to_size(FT_INT)) {
      return make_fundamental(FT_INT);
    }
    return make_fundamental(FT_UNSIGNED_INT);
  case FT_CHAR32_T:
    return make_fundamental(FT_UNSIGNED_INT);
  default:
    return TypePtr();
  }
}

bool is_condition_test_type(const TypePtr & type)
{
  return is_integral_or_unscoped_enum_type(type) || is_floating_type(type) ||
         is_nullable_pointer_like_type(type);
}

bool try_condition_test_conversion(SemanticContext & ctx,
                                   Scope & scope,
                                   ExprInfo & expr)
{
  TypePtr converted = value_conversion_type(expr);
  if(converted && is_condition_test_type(converted)) {
    return true;
  }

  ExprInfo bool_expr;
  ConversionRank rank = CR_BAD;
  if(!ctx.try_argument_conversion(scope,
                                  make_fundamental(FT_BOOL),
                                  expr,
                                  bool_expr,
                                  rank,
                                  semantic_policy::allow_explicit_argument_conversion())) {
    return false;
  }

  converted = value_conversion_type(bool_expr);
  if(!converted || !is_condition_test_type(converted)) {
    return false;
  }

  expr = bool_expr;
  return true;
}

int compare_standard_conversion_preference(const TypePtr & lhs_param,
                                           const ExprInfo & lhs_arg,
                                           const TypePtr & rhs_param,
                                           const ExprInfo & rhs_arg)
{
  const ConversionRank lhs_rank = standard_conversion_rank(lhs_param, lhs_arg);
  const ConversionRank rhs_rank = standard_conversion_rank(rhs_param, rhs_arg);
  if(lhs_rank != CR_BAD && rhs_rank != CR_BAD && lhs_rank != rhs_rank) {
    return lhs_rank < rhs_rank ? -1 : 1;
  }

  const auto is_pointer_like_to_bool_conversion =
      [](const TypePtr & param, const ExprInfo & arg) -> bool
      {
        TypePtr target = strip_top_level_cv(remove_reference_type(param));
        if(!target || target->kind != Type::TK_FUNDAMENTAL ||
           target->fundamental != FT_BOOL) {
          return false;
        }

        if(arg.null_pointer_constant) {
          return true;
        }

        TypePtr source = value_conversion_type(arg);
        TypePtr source_base = strip_top_level_cv(source);
        return source_base &&
               ((source_base->kind == Type::TK_FUNDAMENTAL &&
                 source_base->fundamental == FT_NULLPTR_T) ||
                is_nullable_pointer_like_type(source_base));
      };

  const bool lhs_pointer_to_bool = is_pointer_like_to_bool_conversion(lhs_param, lhs_arg);
  const bool rhs_pointer_to_bool = is_pointer_like_to_bool_conversion(rhs_param, rhs_arg);
  if(lhs_pointer_to_bool && !rhs_pointer_to_bool) {
    return 1;
  }
  if(rhs_pointer_to_bool && !lhs_pointer_to_bool) {
    return -1;
  }
  return 0;
}

int compare_reference_binding_preference(const TypePtr & lhs_param,
                                         const ExprInfo & lhs_arg,
                                         const TypePtr & rhs_param,
                                         const ExprInfo & rhs_arg)
{
  if(lhs_arg.category != rhs_arg.category ||
     lhs_arg.category == VC_LVALUE) {
    return 0;
  }

  TypePtr lhs_base = strip_top_level_cv(lhs_param);
  TypePtr rhs_base = strip_top_level_cv(rhs_param);
  const bool lhs_lref = lhs_base && lhs_base->kind == Type::TK_LVALUE_REFERENCE;
  const bool lhs_rref = lhs_base && lhs_base->kind == Type::TK_RVALUE_REFERENCE;
  const bool rhs_lref = rhs_base && rhs_base->kind == Type::TK_LVALUE_REFERENCE;
  const bool rhs_rref = rhs_base && rhs_base->kind == Type::TK_RVALUE_REFERENCE;

  if(lhs_rref && rhs_lref) {
    return -1;
  }
  if(lhs_lref && rhs_rref) {
    return 1;
  }
  return 0;
}

int compare_parameter_qualification_preference(const TypePtr & lhs,
                                               const TypePtr & rhs)
{
  TypePtr lhs_base;
  TypePtr rhs_base;
  bool lhs_const = false;
  bool lhs_volatile = false;
  bool rhs_const = false;
  bool rhs_volatile = false;
  if(!top_level_cv_flags(lhs, lhs_base, lhs_const, lhs_volatile) ||
     !top_level_cv_flags(rhs, rhs_base, rhs_const, rhs_volatile)) {
    return 0;
  }

  TypePtr lhs_stripped = strip_top_level_cv(lhs_base);
  TypePtr rhs_stripped = strip_top_level_cv(rhs_base);
  if(!lhs_stripped || !rhs_stripped) {
    return 0;
  }

  if(lhs_stripped->kind == Type::TK_POINTER &&
     rhs_stripped->kind == Type::TK_POINTER) {
    int inner_pref =
        compare_parameter_qualification_preference(lhs_stripped->inner, rhs_stripped->inner);
    if(inner_pref != 0) {
      return inner_pref;
    }
  } else if(lhs_stripped->kind == Type::TK_ARRAY &&
            rhs_stripped->kind == Type::TK_ARRAY) {
    if(lhs_stripped->has_bound != rhs_stripped->has_bound ||
       (lhs_stripped->has_bound && lhs_stripped->bound != rhs_stripped->bound)) {
      return 0;
    }
    int inner_pref =
        compare_parameter_qualification_preference(lhs_stripped->inner, rhs_stripped->inner);
    if(inner_pref != 0) {
      return inner_pref;
    }
  } else if(!type_equals(lhs_stripped, rhs_stripped)) {
    return 0;
  }

  const bool lhs_more_qualified =
      (!rhs_const || lhs_const) &&
      (!rhs_volatile || lhs_volatile) &&
      (lhs_const != rhs_const || lhs_volatile != rhs_volatile);
  const bool rhs_more_qualified =
      (!lhs_const || rhs_const) &&
      (!lhs_volatile || rhs_volatile) &&
      (lhs_const != rhs_const || lhs_volatile != rhs_volatile);
  if(lhs_more_qualified && !rhs_more_qualified) {
    return 1;
  }
  if(rhs_more_qualified && !lhs_more_qualified) {
    return -1;
  }
  return 0;
}

int compare_qualification_conversion_preference(const TypePtr & lhs_param,
                                                const ExprInfo & lhs_arg,
                                                const TypePtr & rhs_param,
                                                const ExprInfo & rhs_arg)
{
  (void)lhs_arg;
  (void)rhs_arg;
  TypePtr lhs_base = strip_top_level_cv(lhs_param);
  TypePtr rhs_base = strip_top_level_cv(rhs_param);
  if(lhs_base && rhs_base &&
     (lhs_base->kind == Type::TK_LVALUE_REFERENCE ||
      lhs_base->kind == Type::TK_RVALUE_REFERENCE) &&
     (rhs_base->kind == Type::TK_LVALUE_REFERENCE ||
      rhs_base->kind == Type::TK_RVALUE_REFERENCE)) {
    return compare_parameter_qualification_preference(lhs_base->inner, rhs_base->inner);
  }
  return compare_parameter_qualification_preference(lhs_base, rhs_base);
}

bool pointer_equality_operands_compatible(const TypePtr & lhs, const TypePtr & rhs)
{
  TypePtr lhs_base = strip_top_level_cv(lhs);
  TypePtr rhs_base = strip_top_level_cv(rhs);
  if(!lhs_base || !rhs_base) {
    return false;
  }
  const bool lhs_nullptr =
      lhs_base->kind == Type::TK_FUNDAMENTAL && lhs_base->fundamental == FT_NULLPTR_T;
  const bool rhs_nullptr =
      rhs_base->kind == Type::TK_FUNDAMENTAL && rhs_base->fundamental == FT_NULLPTR_T;
  if((is_nullable_pointer_like_type(lhs_base) && rhs_nullptr) ||
     (is_nullable_pointer_like_type(rhs_base) && lhs_nullptr)) {
    return true;
  }
  if(lhs_base->kind == Type::TK_POINTER && rhs_base->kind == Type::TK_POINTER) {
    if(same_type_with_compatible_top_cv(lhs_base->inner, rhs_base->inner) ||
       same_type_with_compatible_top_cv(rhs_base->inner, lhs_base->inner)) {
      return true;
    }

    TypePtr lhs_pointee = strip_top_level_cv(lhs_base->inner);
    TypePtr rhs_pointee = strip_top_level_cv(rhs_base->inner);
    if((lhs_pointee && rhs_pointee) &&
       ((is_void_type(lhs_pointee) && rhs_pointee->kind != Type::TK_FUNCTION) ||
        (is_void_type(rhs_pointee) && lhs_pointee->kind != Type::TK_FUNCTION))) {
      return true;
    }
    return false;
  }
  if(lhs_base->kind != Type::TK_MEMBER_POINTER || rhs_base->kind != Type::TK_MEMBER_POINTER) {
    return false;
  }
  return type_equals(lhs_base->owner, rhs_base->owner) &&
         (same_type_with_compatible_top_cv(lhs_base->inner, rhs_base->inner) ||
          same_type_with_compatible_top_cv(rhs_base->inner, lhs_base->inner));
}

bool pointer_subtraction_operands_compatible(const TypePtr & lhs, const TypePtr & rhs)
{
  TypePtr lhs_base = strip_top_level_cv(lhs);
  TypePtr rhs_base = strip_top_level_cv(rhs);
  if(!lhs_base || !rhs_base ||
     lhs_base->kind != Type::TK_POINTER ||
     rhs_base->kind != Type::TK_POINTER) {
    return false;
  }
  return same_type_with_compatible_top_cv(lhs_base->inner, rhs_base->inner) ||
         same_type_with_compatible_top_cv(rhs_base->inner, lhs_base->inner);
}

TypePtr promoted_integral_result_type(const TypePtr & type)
{
  TypePtr base = strip_top_level_cv(type);
  TypePtr promoted = promoted_integral_type(base);
  return promoted ? promoted : base;
}

TypePtr common_integral_result_type(const TypePtr & lhs, const TypePtr & rhs)
{
  TypePtr lhs_type = promoted_integral_result_type(lhs);
  TypePtr rhs_type = promoted_integral_result_type(rhs);
  if(type_equals(lhs_type, rhs_type)) {
    return lhs_type;
  }

  const Type * lhs_base = fundamental_base_type(lhs_type);
  const Type * rhs_base = fundamental_base_type(rhs_type);
  if(!lhs_base || !rhs_base) {
    return make_fundamental(FT_INT);
  }

  const EFundamentalType lhs_ft = lhs_base->fundamental;
  const EFundamentalType rhs_ft = rhs_base->fundamental;
  const bool lhs_unsigned = !type_is_signed(lhs_ft);
  const bool rhs_unsigned = !type_is_signed(rhs_ft);
  const int lhs_rank = integral_conversion_rank(lhs_ft);
  const int rhs_rank = integral_conversion_rank(rhs_ft);

  if(lhs_unsigned == rhs_unsigned) {
    if(lhs_rank > rhs_rank) {
      return lhs_type;
    }
    if(rhs_rank > lhs_rank) {
      return rhs_type;
    }
    return type_to_size(lhs_ft) >= type_to_size(rhs_ft) ? lhs_type : rhs_type;
  }

  const EFundamentalType signed_ft = lhs_unsigned ? rhs_ft : lhs_ft;
  const EFundamentalType unsigned_ft = lhs_unsigned ? lhs_ft : rhs_ft;
  const TypePtr & signed_type = lhs_unsigned ? rhs_type : lhs_type;
  const TypePtr & unsigned_type = lhs_unsigned ? lhs_type : rhs_type;
  const int signed_rank = lhs_unsigned ? rhs_rank : lhs_rank;
  const int unsigned_rank = lhs_unsigned ? lhs_rank : rhs_rank;

  if(unsigned_rank >= signed_rank) {
    return unsigned_type;
  }
  if(type_to_size(signed_ft) > type_to_size(unsigned_ft)) {
    return signed_type;
  }
  return make_fundamental(corresponding_unsigned_integral(signed_ft));
}

TypePtr common_arithmetic_result_type(const TypePtr & lhs, const TypePtr & rhs)
{
  TypePtr lhs_type = strip_top_level_cv(lhs);
  TypePtr rhs_type = strip_top_level_cv(rhs);
  if(is_floating_type(lhs_type) || is_floating_type(rhs_type)) {
    const EFundamentalType lhs_ft =
        lhs_type && lhs_type->kind == Type::TK_FUNDAMENTAL ? lhs_type->fundamental : FT_VOID;
    const EFundamentalType rhs_ft =
        rhs_type && rhs_type->kind == Type::TK_FUNDAMENTAL ? rhs_type->fundamental : FT_VOID;
    if(lhs_ft == FT_LONG_DOUBLE || rhs_ft == FT_LONG_DOUBLE) {
      return make_fundamental(FT_LONG_DOUBLE);
    }
    if(lhs_ft == FT_DOUBLE || rhs_ft == FT_DOUBLE) {
      return make_fundamental(FT_DOUBLE);
    }
    return make_fundamental(FT_FLOAT);
  }
  return common_integral_result_type(lhs_type, rhs_type);
}

void set_unmaterialized_inheritance_conversion_result(SemanticContext & ctx,
                                                      const TypePtr & target,
                                                      const ExprInfo & expr,
                                                      const ClassInfo * target_class,
                                                      ExprInfo & out)
{
  out = expr;
  TypePtr target_base = strip_top_level_cv(target);
  if(target_base &&
     (target_base->kind == Type::TK_LVALUE_REFERENCE ||
      target_base->kind == Type::TK_RVALUE_REFERENCE)) {
    out.type = target_base->inner ?
        target_base->inner :
        (target_class ? target_class->type : expr.type);
    out.category = expr.category;
  } else {
    out.type = target ? target : (target_class ? target_class->type : expr.type);
    out.category =
        target_base && target_base->kind == Type::TK_POINTER ? VC_PRVALUE : expr.category;
  }
  ctx.set_expr_info_metadata(out, out.type, out.category);
}

ExprInfo make_unmaterialized_address_of_expr(SemanticContext & ctx,
                                             const ExprInfo & operand)
{
  ExprInfo result = operand;
  TypePtr pointee = remove_reference_type(operand.type);
  if(!pointee) {
    pointee = operand.type;
  }
  result.type = make_pointer(pointee);
  result.category = VC_PRVALUE;
  result.null_pointer_constant = false;
  ctx.set_expr_info_metadata(result, result.type, result.category);
  return result;
}

ExprInfo make_unmaterialized_constructor_result(SemanticContext & ctx,
                                                const TypePtr & result_type,
                                                const ExprInfo & source)
{
  ExprInfo result = source;
  result.type = result_type;
  result.category = VC_PRVALUE;
  result.null_pointer_constant = false;
  ctx.set_expr_info_metadata(result, result.type, result.category);
  return result;
}

ExprInfo make_unmaterialized_direct_call_result(SemanticContext & ctx,
                                                FunctionBinding & function,
                                                const ExprInfo & source)
{
  TypePtr function_type = strip_top_level_cv(function.type);
  if(!function_type || function_type->kind != Type::TK_FUNCTION) {
    throw logic_error("direct call expression requires function type");
  }
  ValueCategory category = VC_PRVALUE;
  if(!result_value_category_for_function_result(function_type->inner, category)) {
    throw logic_error("invalid direct call expression result");
  }
  ExprInfo result = source;
  result.type = expression_type_for_function_result(function_type->inner);
  result.category = category;
  result.null_pointer_constant = false;
  ctx.set_expr_info_metadata(result, result.type, result.category);
  return result;
}

bool try_apply_inheritance_conversion_impl(SemanticContext & ctx,
                                           const TypePtr & target,
                                           const ExprInfo & expr,
                                           ExprInfo & out,
                                           bool materialize)
{
  TypePtr target_base = strip_top_level_cv(target);
  TypePtr expr_base = strip_top_level_cv(remove_reference_type(expr.type));
  TypePtr expr_object_type = reference_binding_source_type(expr);
  if(!target_base || !expr_base) {
    return false;
  }

  if(target_base->kind == Type::TK_LVALUE_REFERENCE &&
     (expr.category == VC_LVALUE ||
      reference_referent_accepts_temporary(target_base->inner))) {
    if(!top_level_cv_allows_reference_binding(target_base->inner, expr_object_type)) {
      return false;
    }
    ClassInfo * target_class = class_info_for_inheritance_conversion(
        ctx, strip_top_level_cv(target_base->inner), materialize);
    ClassInfo * source_class = class_info_for_inheritance_conversion(
        ctx, expr_object_type, materialize);
    size_t offset = 0;
    MemberAccess access = MA_PUBLIC;
    if(target_class && source_class && target_class == source_class) {
      set_unmaterialized_inheritance_conversion_result(ctx, target, expr, target_class, out);
      return true;
    }
    if(target_class && source_class &&
       find_unique_base_path(*source_class, target_class, offset, access)) {
      if(materialize) {
        out = ctx.apply_base_subobject_adjustment(expr, target, *target_class, offset);
      } else {
        set_unmaterialized_inheritance_conversion_result(ctx, target, expr, target_class, out);
      }
      return true;
    }
  }

  if(target_base->kind == Type::TK_RVALUE_REFERENCE && expr.category != VC_LVALUE) {
    if(!top_level_cv_allows_reference_binding(target_base->inner, expr_object_type)) {
      return false;
    }
    ClassInfo * target_class = class_info_for_inheritance_conversion(
        ctx, strip_top_level_cv(target_base->inner), materialize);
    ClassInfo * source_class = class_info_for_inheritance_conversion(
        ctx, expr_object_type, materialize);
    size_t offset = 0;
    MemberAccess access = MA_PUBLIC;
    if(target_class && source_class && target_class == source_class) {
      set_unmaterialized_inheritance_conversion_result(ctx, target, expr, target_class, out);
      return true;
    }
    if(target_class && source_class &&
       find_unique_base_path(*source_class, target_class, offset, access)) {
      if(materialize) {
        out = ctx.apply_base_subobject_adjustment(expr, target, *target_class, offset);
      } else {
        set_unmaterialized_inheritance_conversion_result(ctx, target, expr, target_class, out);
      }
      return true;
    }
  }

  if(target_base->kind == Type::TK_POINTER && expr_base->kind == Type::TK_POINTER) {
    if(type_equals(target_base, expr_base)) {
      out = expr;
      ctx.set_expr_info_metadata(out, target, out.category);
      return true;
    }
    TypePtr target_pointee_base;
    TypePtr expr_pointee_base;
    if(!pointer_pointee_cv_allows_base_conversion(target_base,
                                                  expr_base,
                                                  &target_pointee_base,
                                                  &expr_pointee_base)) {
      return false;
    }
    ClassInfo * target_class = class_info_for_inheritance_conversion(
        ctx, target_pointee_base, materialize);
    ClassInfo * source_class = class_info_for_inheritance_conversion(
        ctx, expr_pointee_base, materialize);
    size_t offset = 0;
    MemberAccess access = MA_PUBLIC;
    if(target_class && source_class && target_class == source_class) {
      set_unmaterialized_inheritance_conversion_result(ctx, target, expr, target_class, out);
      return true;
    }
    if(target_class && source_class &&
       find_unique_base_path(*source_class, target_class, offset, access)) {
      if(materialize) {
        out = ctx.apply_base_subobject_adjustment(expr, target, *target_class, offset);
      } else {
        set_unmaterialized_inheritance_conversion_result(ctx, target, expr, target_class, out);
      }
      return true;
    }
  }

  if(target_base->kind == Type::TK_MEMBER_POINTER &&
     expr_base->kind == Type::TK_MEMBER_POINTER) {
    size_t offset = 0;
    if(member_pointer_inheritance_conversion(ctx, target_base, expr_base, &offset)) {
      out = expr;
      if(materialize &&
         offset != 0 &&
         !is_function_type(target_base->inner) &&
         out.node.kind == CallSemKind::unary_expression &&
         out.node.has_uint_value) {
        set_callsem_uint_value(out.node, callsem_uint_value(out.node) + offset);
      }
      ctx.set_expr_info_metadata(out, target, out.category);
      return true;
    }
  }

  if(target_base->kind == Type::TK_NAMED) {
    ClassInfo * target_class = class_info_for_inheritance_conversion(
        ctx, target_base, materialize);
    ClassInfo * source_class = class_info_for_inheritance_conversion(
        ctx, expr_object_type, materialize);
    size_t offset = 0;
    MemberAccess access = MA_PUBLIC;
    if(target_class && source_class && target_class == source_class) {
      set_unmaterialized_inheritance_conversion_result(ctx, target, expr, target_class, out);
      return true;
    }
    if(target_class && source_class &&
       find_unique_base_path(*source_class, target_class, offset, access)) {
      if(materialize) {
        out = ctx.apply_base_subobject_adjustment(expr, target, *target_class, offset);
      } else {
        set_unmaterialized_inheritance_conversion_result(ctx, target, expr, target_class, out);
      }
      return true;
    }
  }

  return false;
}

bool try_apply_inheritance_conversion(SemanticContext & ctx,
                                      const TypePtr & target,
                                      const ExprInfo & expr,
                                      ExprInfo & out)
{
  return try_apply_inheritance_conversion_impl(ctx, target, expr, out, true);
}

bool try_apply_unmaterialized_inheritance_conversion(SemanticContext & ctx,
                                                     const TypePtr & target,
                                                     const ExprInfo & expr,
                                                     ExprInfo & out)
{
  return try_apply_inheritance_conversion_impl(ctx, target, expr, out, false);
}

static FunctionBinding * ensure_captureless_lambda_conversion_target(
    SemanticContext & ctx,
    Scope & use_scope,
    ClassInfo & closure,
    const TypePtr & target)
{
  TypePtr target_base = strip_top_level_cv(target);
  if(!closure.is_lambda_closure ||
     !closure.fields.empty() ||
     !target_base ||
     target_base->kind != Type::TK_POINTER ||
     !is_function_type(target_base->inner)) {
    return nullptr;
  }

  FunctionBinding * call_operator = nullptr;
  map<string, vector<FunctionBinding *> >::const_iterator found =
      closure.methods.find("operator()");
  if(found != closure.methods.end()) {
    for(size_t i = 0; i < found->second.size(); ++i) {
      FunctionBinding * candidate = found->second[i];
      if(candidate &&
         candidate->declared_type &&
         type_equals(strip_top_level_cv(candidate->declared_type),
                     strip_top_level_cv(target_base->inner))) {
        call_operator = candidate;
        break;
      }
    }
  }
  if(!call_operator) {
    return nullptr;
  }

  if(closure.captureless_lambda_conversion_target) {
    return closure.captureless_lambda_conversion_target;
  }

  vector<pair<string, TypePtr> > params;
  vector<const CppAstNode *> default_arguments;
  for(size_t i = 1; i < call_operator->params.size(); ++i) {
    params.push_back(call_operator->params[i]);
    default_arguments.push_back(
        i < call_operator->default_arguments.size() ?
            call_operator->default_arguments[i] : nullptr);
  }
  Scope & lambda_scope = closure.enclosing_scope ?
      *closure.enclosing_scope : use_scope;
  closure.captureless_lambda_conversion_target =
      ctx.create_synthetic_lambda_function(lambda_scope,
                                           call_operator->declared_type,
                                           params,
                                           default_arguments,
                                           call_operator->declaration_node,
                                           call_operator->body);
  return closure.captureless_lambda_conversion_target;
}

static ExprInfo make_captureless_lambda_conversion_result(
    SemanticContext & ctx,
    FunctionBinding & target)
{
  ExprInfo result;
  result.type = target.type;
  result.category = VC_LVALUE;
  result.node = make_dump_node(CallSemKind::id_expression, target.name);
  ctx.set_expr_info_metadata(result, result.type, result.category);
  return result;
}

bool try_argument_conversion(SemanticContext & ctx,
                             Scope & scope,
                             const TypePtr & target,
                             const ExprInfo & expr,
                             ExprInfo & out,
                             ConversionRank & rank,
                             const ArgumentConversionOptions & options)
{
  if(semantic_metrics::AnalyzerCounters * counters = ctx.performance_counters()) {
    ++counters->conversion_attempts;
  }
  if(semantic_hotspot::enabled()) {
    std::ostringstream query;
    query << "target="
          << (target ? describe_type(target) : std::string("<null>"))
          << " expr="
          << (expr.type ? describe_type(expr.type) : std::string("<null>"))
          << "/" << static_cast<int>(expr.category)
          << " allow_ud=" << (options.allow_user_defined ? "yes" : "no");
    semantic_hotspot::note_semantic_query("try_argument_conversion", query.str());
  }
  out = expr;
  if(try_fast_fundamental_exact_conversion(target, expr, rank)) {
    return true;
  }
  if(target &&
     expr.type &&
     (ctx.type_depends_on_template_parameter(target) ||
      ctx.type_depends_on_template_parameter(expr.type))) {
    if(!dependent_conversion_candidate_can_bind_known_argument(ctx, target, expr)) {
      rank = CR_BAD;
      return false;
    }
    rank = CR_CONVERSION;
    TypePtr target_base = strip_top_level_cv(target);
    const bool target_is_reference =
        target_base &&
        (target_base->kind == Type::TK_LVALUE_REFERENCE ||
         target_base->kind == Type::TK_RVALUE_REFERENCE);
    ctx.set_expr_info_metadata(out,
                               target,
                               target_is_reference ? expr.category : VC_PRVALUE);
    return true;
  }
  rank = standard_conversion_rank(target, expr);
  if(rank != CR_BAD) {
    apply_standard_conversion_result_metadata(ctx, target, expr, out);
    return true;
  }
  if(try_semantic_exact_reference_binding(ctx, target, expr, out, rank)) {
    return true;
  }

  TypePtr ref_target_base = strip_top_level_cv(target);
  if(ref_target_base &&
     ref_target_base->kind == Type::TK_LVALUE_REFERENCE &&
     expr.category == VC_LVALUE &&
     reference_referent_accepts_temporary(ref_target_base->inner)) {
    TypePtr referred_base = strip_top_level_cv(ref_target_base->inner);
    TypePtr source_type = reference_binding_source_type(expr);
    TypePtr source_base;
    bool source_const = false;
    bool source_volatile = false;
    TypePtr target_type_with_cv_base;
    bool target_const = false;
    bool target_volatile = false;
    if(referred_base &&
       referred_base->kind == Type::TK_POINTER &&
       top_level_cv_flags(source_type, source_base, source_const, source_volatile) &&
       top_level_cv_flags(ref_target_base->inner,
                          target_type_with_cv_base,
                          target_const,
                          target_volatile) &&
       (!source_const || target_const) &&
       (!source_volatile || target_volatile)) {
      rank = standard_conversion_rank_non_reference(referred_base, expr);
      if(rank != CR_BAD) {
        return true;
      }
    }
  }

  TypePtr reference_conversion_target =
      reference_binding_converted_pointer_target(target, expr);
  if(reference_conversion_target) {
    ExprInfo converted_reference_source;
    const bool converted_ok =
        options.materialize_standard_adjustments ?
            try_apply_inheritance_conversion(ctx,
                                             reference_conversion_target,
                                             expr,
                                             converted_reference_source) :
            try_apply_unmaterialized_inheritance_conversion(ctx,
                                                            reference_conversion_target,
                                                            expr,
                                                            converted_reference_source);
    if(converted_ok) {
      out = converted_reference_source;
      rank = inheritance_conversion_rank(ctx, reference_conversion_target, expr);
      return true;
    }
  }

  ExprInfo inherited;
  const bool inherited_ok =
      options.materialize_standard_adjustments ?
          try_apply_inheritance_conversion(ctx, target, expr, inherited) :
          try_apply_unmaterialized_inheritance_conversion(ctx, target, expr, inherited);
  if(inherited_ok) {
    out = inherited;
    rank = inheritance_conversion_rank(ctx, target, expr);
    return true;
  }

  if(!options.allow_user_defined || !target || !expr.type) {
    return false;
  }

  struct UserDefinedCandidate
  {
    enum Kind
    {
      PREMATERIALIZED,
      CONSTRUCTOR,
      CONVERSION_FUNCTION
    };

    Kind kind = PREMATERIALIZED;
    ExprInfo expr;
    ExprInfo source_expr;
    ClassInfo * constructor_target_class = nullptr;
    FunctionBinding * constructor = nullptr;
    vector<ExprInfo> constructor_args;
    FunctionBinding * conversion_function = nullptr;
    const ClassInfo * conversion_declared_in = nullptr;
    std::size_t conversion_path_offset = 0;
    TypePtr conversion_implicit_object_param;
    TypePtr initial_parameter;
    ExprInfo initial_argument;
    ConversionRank initial_rank = CR_EXACT;
    bool direct_reference_binding = false;
    ConversionRank second_rank = CR_BAD;
  };

  vector<UserDefinedCandidate> candidates;

  TypePtr target_base = strip_top_level_cv(target);
  const bool target_is_reference =
      target_base &&
      (target_base->kind == Type::TK_LVALUE_REFERENCE ||
       target_base->kind == Type::TK_RVALUE_REFERENCE);
  TypePtr target_class_type = target_base;
  if(target_is_reference) {
    target_class_type = strip_top_level_cv(target_base->inner);
  }

  // `std::initializer_list<T>` arguments are only formed from an actual
  // initializer_list object or from braced-init-list target-aware analysis
  // earlier in the pipeline. Avoid exploring impossible constructor-based
  // conversions from arbitrary single expressions such as `pair<int, int>`.
  if(target_class_type && ctx.is_initializer_list_type(target_class_type, nullptr, nullptr)) {
    TypePtr expr_class_type = strip_top_level_cv(remove_reference_type(expr.type));
    if(!ctx.is_initializer_list_type(expr_class_type, nullptr, nullptr)) {
      return false;
    }
  }

  if(ClassInfo * target_class = ensure_complete_class_info(ctx, target_class_type)) {
    TypePtr expr_object_type = strip_top_level_cv(remove_reference_type(expr.type));
    ClassInfo * source_class = ctx.class_info_for_type(expr_object_type);
    bool same_or_derived_reference_source = false;
    if(target_is_reference && source_class) {
      same_or_derived_reference_source = source_class == target_class;
      if(!same_or_derived_reference_source) {
        std::size_t ignored_offset = 0;
        MemberAccess ignored_access = MA_PUBLIC;
        same_or_derived_reference_source =
            find_unique_base_path(*source_class,
                                  target_class,
                                  ignored_offset,
                                  ignored_access);
      }
    }
    if(!same_or_derived_reference_source &&
       reference_target_accepts_result_category(target, VC_PRVALUE)) {
    vector<ExprInfo> ctor_source_args(1, expr);
    vector<ExprInfo> ctor_call_args;
    vector<ConversionRank> ctor_param_ranks;
    FunctionBinding * ctor = nullptr;
    try
    {
      ConstructorSelectionOptions ctor_options =
          constructor_lifecycle_service::selection_options_for(
              constructor_lifecycle_service::user_defined_conversion_constructor_probe_profile(
                  "user-defined conversion constructor",
                  false));
      ctor_options.emit_source_witness_without_body_instantiation =
          options.instantiate_user_defined_bodies;
      ctor_options.use_location = constructor_probe_use_location(expr);
      ctor = ctx.select_constructor_from_exprs(scope,
                                               *target_class,
                                               ctor_source_args,
                                               ctor_call_args,
                                               &ctor_param_ranks,
                                               ctor_options);
    }
    catch(const logic_error &)
    {
      ctor = nullptr;
    }
    if(ctor) {
      ExprInfo ctor_expr =
          make_unmaterialized_constructor_result(ctx, target_class->type, expr);
      ExprInfo adjusted = ctor_expr;
      ConversionRank second = standard_conversion_rank(target, ctor_expr);
      if(second == CR_BAD &&
         try_apply_unmaterialized_inheritance_conversion(ctx, target, ctor_expr, inherited)) {
        adjusted = inherited;
        second = inheritance_conversion_rank(ctx, target, ctor_expr);
      }
      if(second != CR_BAD) {
        UserDefinedCandidate candidate;
        candidate.kind = UserDefinedCandidate::CONSTRUCTOR;
        candidate.expr = adjusted;
        candidate.source_expr = expr;
        candidate.constructor_target_class = target_class;
        candidate.constructor = ctor;
        candidate.constructor_args = ctor_call_args;
        const std::size_t explicit_offset =
            function_binding_explicit_parameter_offset(*ctor);
        TypePtr ctor_function_type = strip_top_level_cv(ctor->type);
        if(ctor_function_type &&
           ctor_function_type->kind == Type::TK_FUNCTION &&
           explicit_offset < ctor_function_type->params.size()) {
          candidate.initial_parameter = ctor_function_type->params[explicit_offset];
        }
        candidate.initial_argument = expr;
        candidate.initial_rank = !ctor_param_ranks.empty() ?
            ctor_param_ranks[0] :
            (candidate.initial_parameter ?
                 conversion_rank(ctx, candidate.initial_parameter, expr) :
                 CR_EXACT);
        candidate.second_rank = second;
        candidates.push_back(candidate);
      }
    }
    }
  }

  TypePtr source_class_type = strip_top_level_cv(remove_reference_type(expr.type));
  if(ClassInfo * source_class = ensure_complete_class_info(ctx, source_class_type)) {
    FunctionBinding * lambda_target =
        ensure_captureless_lambda_conversion_target(ctx,
                                                    scope,
                                                    *source_class,
                                                    target);
    if(lambda_target) {
      ExprInfo lambda_result =
          make_captureless_lambda_conversion_result(ctx, *lambda_target);
      ConversionRank second = standard_conversion_rank(target, lambda_result);
      if(second != CR_BAD) {
        UserDefinedCandidate lambda_candidate;
        lambda_candidate.kind = UserDefinedCandidate::PREMATERIALIZED;
        lambda_candidate.expr = lambda_result;
        lambda_candidate.source_expr = expr;
        lambda_candidate.initial_parameter = expr.type;
        lambda_candidate.initial_argument = expr;
        lambda_candidate.initial_rank = CR_EXACT;
        lambda_candidate.second_rank = second;
        candidates.push_back(lambda_candidate);
      }
    }

    ExprInfo conversion_function_implicit_object_arg;
    bool conversion_function_implicit_object_ready = false;
    const auto get_conversion_function_implicit_object_arg = [&]() -> const ExprInfo &
    {
      if(!conversion_function_implicit_object_ready) {
        ScopedCallSemConstructionPath construction_path(
            "conversion.conversion-function-implicit-object");
        conversion_function_implicit_object_arg =
            make_unmaterialized_address_of_expr(ctx, expr);
        conversion_function_implicit_object_ready = true;
      }
      return conversion_function_implicit_object_arg;
    };
    std::set<FunctionBinding *> seen_conversion_bindings;
    const auto maybe_add_conversion_function_candidate =
        [&](FunctionBinding * candidate,
            const ClassInfo * declared_in,
            MemberAccess path_access,
            std::size_t path_offset)
    {
      if(!candidate || !candidate->is_method ||
         !ctx.is_conversion_function_name(candidate->name)) {
        return;
      }
      if(!seen_conversion_bindings.insert(candidate).second) {
        return;
      }
      if(binding_declares_explicit_function(*candidate) && !options.allow_explicit) {
        return;
      }
      if(!member_access_allowed(&scope,
                                current_class_scope(scope),
                                current_function_scope(scope),
                                declared_in ? declared_in : candidate->owner_class,
                                candidate->access,
                                path_access)) {
        return;
      }

      TypePtr function_type = strip_top_level_cv(candidate->type);
      if(!function_type || function_type->kind != Type::TK_FUNCTION ||
         function_type->params.size() != 1) {
        return;
      }
      if(!ref_qualifier_accepts_implicit_object(candidate->ref_qualifier,
                                                function_type->params[0],
                                                expr.category)) {
        return;
      }

      const ExprInfo & implicit_object_arg =
          get_conversion_function_implicit_object_arg();
      ExprInfo adjusted_this = implicit_object_arg;
      ExprInfo converted_this;
      if(try_apply_unmaterialized_inheritance_conversion(ctx,
                                                         function_type->params[0],
                                                         implicit_object_arg,
                                                         converted_this)) {
        adjusted_this = converted_this;
      } else if(path_offset != 0 && declared_in) {
        set_unmaterialized_inheritance_conversion_result(ctx,
                                                         function_type->params[0],
                                                         implicit_object_arg,
                                                         declared_in,
                                                         adjusted_this);
      }
      ConversionRank implicit_object_rank =
          implicit_object_conversion_rank(ctx, function_type->params[0], adjusted_this);
      if(implicit_object_rank == CR_BAD) {
        return;
      }

      ValueCategory conversion_result_category = VC_PRVALUE;
      if(!result_value_category_for_function_result(function_type->inner,
                                                    conversion_result_category) ||
         !reference_target_accepts_result_category(target, conversion_result_category)) {
        return;
      }

      ExprInfo call_expr =
          make_unmaterialized_direct_call_result(ctx, *candidate, expr);
      ExprInfo adjusted = call_expr;
      ConversionRank second = standard_conversion_rank(target, call_expr);
      if(second == CR_BAD &&
         try_apply_unmaterialized_inheritance_conversion(ctx, target, call_expr, inherited)) {
        adjusted = inherited;
        second = inheritance_conversion_rank(ctx, target, call_expr);
      }
      if(second != CR_BAD) {
        UserDefinedCandidate ud_candidate;
        ud_candidate.kind = UserDefinedCandidate::CONVERSION_FUNCTION;
        ud_candidate.expr = adjusted;
        ud_candidate.source_expr = expr;
        ud_candidate.conversion_function = candidate;
        ud_candidate.conversion_declared_in = declared_in;
        ud_candidate.conversion_path_offset = path_offset;
        ud_candidate.conversion_implicit_object_param = function_type->params[0];
        TypePtr object_param = strip_top_level_cv(function_type->params[0]);
        if(object_param && object_param->kind == Type::TK_POINTER) {
          ud_candidate.initial_parameter =
              expr.category == VC_LVALUE ?
                  make_lvalue_reference_raw(object_param->inner) :
                  make_rvalue_reference_raw(object_param->inner);
        } else {
          ud_candidate.initial_parameter = function_type->params[0];
        }
        ud_candidate.initial_argument = expr;
        ud_candidate.initial_rank = implicit_object_rank;
        ud_candidate.direct_reference_binding =
            target_is_reference && conversion_result_category != VC_PRVALUE;
        ud_candidate.second_rank = second;
        candidates.push_back(ud_candidate);
      }
    };

    vector<MemberFunctionLookupResult> conversion_sets =
        collect_visible_conversion_functions(ctx, *source_class);
    for(size_t set_index = 0; set_index < conversion_sets.size(); ++set_index) {
      const MemberFunctionLookupResult & visible = conversion_sets[set_index];
      const ClassInfo * declared_in = visible.declared_in;
      for(size_t i = 0; i < visible.functions.size(); ++i) {
        maybe_add_conversion_function_candidate(visible.functions[i],
                                                declared_in,
                                                visible.path_access,
                                                visible.path_offset);
      }
    }

    vector<MemberFunctionTemplateLookupResult> conversion_template_sets =
        collect_visible_conversion_function_templates(ctx, *source_class);
    for(size_t set_index = 0; set_index < conversion_template_sets.size(); ++set_index) {
      const MemberFunctionTemplateLookupResult & visible = conversion_template_sets[set_index];
      const ClassInfo * declared_in = visible.declared_in;
      for(size_t i = 0; i < visible.templates.size(); ++i) {
        FunctionTemplateDecl * decl = visible.templates[i];
        if(!decl || !ctx.is_conversion_function_name(decl->name)) {
          continue;
        }
        if(!member_access_allowed(&scope,
                                  current_class_scope(scope),
                                  current_function_scope(scope),
                                  declared_in,
                                  decl->access,
                                  visible.path_access)) {
          continue;
        }

        Scope template_use_scope(&scope, "", false);
        template_use_scope.class_info = source_class;
        template_use_scope.function = scope.function;
        if(source_class->member_scope) {
          semantic_template_function::overlay_instantiation_use_scope_bindings(
              template_use_scope,
              *source_class->member_scope,
              decl->declaring_scope);
        }

        vector<TypePtr> conversion_targets =
            conversion_function_template_deduction_target_types(target);
        for(size_t target_index = 0;
            target_index < conversion_targets.size();
            ++target_index) {
          TypePtr conversion_target = conversion_targets[target_index];
          TypePtr target_function_type =
              build_conversion_function_template_target_type(ctx,
                                                             template_use_scope,
                                                             *decl,
                                                             conversion_target);
          if(!target_function_type) {
            continue;
          }

          semantic_template_function::FunctionTemplateDeduction result;
          if(!semantic_template_function::deduce_function_template_from_target_type(
                 ctx, *decl, target_function_type, &template_use_scope, result)) {
            continue;
          }

          FunctionBinding * binding = nullptr;
          try
          {
            binding = semantic_template_function::acquire_function_template_binding(
                ctx,
                *decl,
                result.arguments,
                &template_use_scope,
                result.pack_sizes.empty() ? nullptr : &result.pack_sizes,
                false);
          }
          catch(const TemplateSubstitutionFailure &)
          {
            binding = nullptr;
          }
          if(!binding) {
            continue;
          }
          update_conversion_function_template_binding_result(ctx,
                                                             *binding,
                                                             conversion_target);

          maybe_add_conversion_function_candidate(
              binding,
              declared_in ? declared_in : binding->owner_class,
              visible.path_access,
              visible.path_offset);
          break;
        }
      }
    }
  }

  if(candidates.empty()) {
    return false;
  }

  size_t best = 0;
  bool ambiguous = false;
  for(size_t i = 1; i < candidates.size(); ++i) {
    bool current_better = false;
    bool best_better = false;
    if(candidates[i].direct_reference_binding !=
       candidates[best].direct_reference_binding) {
      current_better = candidates[i].direct_reference_binding;
      best_better = candidates[best].direct_reference_binding;
    } else if(candidates[i].initial_rank < candidates[best].initial_rank) {
      current_better = true;
    } else if(candidates[i].initial_rank > candidates[best].initial_rank) {
      best_better = true;
    } else {
      int initial_ref_pref =
          compare_reference_binding_preference(candidates[i].initial_parameter,
                                               candidates[i].initial_argument,
                                               candidates[best].initial_parameter,
                                               candidates[best].initial_argument);
      if(initial_ref_pref < 0) {
        current_better = true;
      } else if(initial_ref_pref > 0) {
        best_better = true;
      } else {
        int initial_qual_pref =
            compare_qualification_conversion_preference(
                candidates[i].initial_parameter,
                candidates[i].initial_argument,
                candidates[best].initial_parameter,
                candidates[best].initial_argument);
        if(initial_qual_pref < 0) {
          current_better = true;
        } else if(initial_qual_pref > 0) {
          best_better = true;
        }
      }
    }
    if(!current_better && !best_better &&
       candidates[i].second_rank < candidates[best].second_rank) {
      current_better = true;
    } else if(!current_better && !best_better &&
              candidates[i].second_rank > candidates[best].second_rank) {
      best_better = true;
    } else if(!current_better && !best_better) {
      int ref_pref = compare_reference_binding_preference(target, candidates[i].expr,
                                                          target, candidates[best].expr);
      if(ref_pref < 0) {
        current_better = true;
      } else if(ref_pref > 0) {
        best_better = true;
      } else {
        int qual_pref = compare_qualification_conversion_preference(target,
                                                                    candidates[i].expr,
                                                                    target,
                                                                    candidates[best].expr);
        if(qual_pref < 0) {
          current_better = true;
        } else if(qual_pref > 0) {
          best_better = true;
        } else if(candidates[i].conversion_function &&
                  candidates[best].conversion_function) {
          int object_qual_pref =
              compare_qualification_conversion_preference(
                  candidates[i].conversion_implicit_object_param,
                  candidates[i].source_expr,
                  candidates[best].conversion_implicit_object_param,
                  candidates[best].source_expr);
          if(object_qual_pref < 0) {
            current_better = true;
          } else if(object_qual_pref > 0) {
            best_better = true;
          }
        }
      }
    }

    if(!current_better && !best_better) {
      FunctionBinding * current_function =
          candidates[i].constructor ? candidates[i].constructor :
                                      candidates[i].conversion_function;
      FunctionBinding * best_function =
          candidates[best].constructor ? candidates[best].constructor :
                                         candidates[best].conversion_function;
      if(current_function &&
         best_function &&
         (current_function->source_template != nullptr) !=
             (best_function->source_template != nullptr)) {
        current_better = current_function->source_template == nullptr;
        best_better = best_function->source_template == nullptr;
      }
    }

    if(current_better && !best_better) {
      best = i;
      ambiguous = false;
    } else if((current_better && best_better) ||
              (!current_better && !best_better)) {
      ambiguous = true;
    }
  }

  if(ambiguous) {
    return false;
  }

  const UserDefinedCandidate & selected = candidates[best];
  FunctionBinding * selected_conversion_function =
      selected.conversion_function;
  out = selected.expr;
  if(selected.kind == UserDefinedCandidate::CONSTRUCTOR) {
    if(!selected.constructor || !selected.constructor_target_class) {
      return false;
    }
    FunctionBinding * selected_constructor = selected.constructor;
    if(options.instantiate_user_defined_bodies) {
      selected_constructor =
          semantic_template_function::acquire_function_definition_binding(
              ctx,
              selected_constructor,
              scope);
      if(!selected_constructor) {
        return false;
      }
    }
    ExprInfo ctor_expr =
        ctx.make_constructor_conversion_expr(*selected_constructor,
                                             selected.constructor_target_class->type,
                                             selected.constructor_args,
                                             options.materialize_user_defined_output);
    out = ctor_expr;
    if(standard_conversion_rank(target, ctor_expr) == CR_BAD) {
      ExprInfo inherited_result;
      if(!try_apply_inheritance_conversion(ctx, target, ctor_expr, inherited_result)) {
        return false;
      }
      out = inherited_result;
    }
  } else if(selected.kind == UserDefinedCandidate::CONVERSION_FUNCTION) {
    if(!selected.conversion_function) {
      return false;
    }
    if(options.instantiate_user_defined_bodies) {
      selected_conversion_function =
          semantic_template_function::acquire_function_definition_binding(
              ctx,
              selected_conversion_function,
              scope);
      if(!selected_conversion_function) {
        return false;
      }
    }
    if(options.instantiate_user_defined_bodies &&
       ctx.expand_output_closure_enabled() &&
       ctx.template_witness_context().session != nullptr &&
       is_bool_type(strip_top_level_cv(target))) {
      semantic_template_function::
          note_ensured_function_definition_materialized_by_lifecycle(
              ctx,
              selected_conversion_function);
    }
    ExprInfo implicit_object_arg = ctx.make_address_of_expr(selected.source_expr);
    ExprInfo adjusted_this = implicit_object_arg;
    ExprInfo converted_this;
    if(try_apply_inheritance_conversion(ctx,
                                        selected.conversion_implicit_object_param,
                                        implicit_object_arg,
                                        converted_this)) {
      adjusted_this = converted_this;
    } else if(selected.conversion_path_offset != 0 &&
              selected.conversion_declared_in) {
      adjusted_this =
          ctx.apply_base_subobject_adjustment(implicit_object_arg,
                                              selected.conversion_implicit_object_param,
                                              *selected.conversion_declared_in,
                                              selected.conversion_path_offset);
    }
    vector<ExprInfo> call_args(1, adjusted_this);
    ExprInfo call_expr =
        ctx.make_direct_call_expr(*selected_conversion_function,
                                  call_args,
                                  options.materialize_user_defined_output);
    out = call_expr;
    if(standard_conversion_rank(target, call_expr) == CR_BAD) {
      ExprInfo inherited_result;
      if(!try_apply_inheritance_conversion(ctx, target, call_expr, inherited_result)) {
        return false;
      }
      out = inherited_result;
    }
  }
  if(options.instantiate_user_defined_bodies) {
    record_conversion_function_source_use(ctx,
                                          selected.source_expr,
                                          selected_conversion_function,
                                          options.source_use_location);
  }
  rank = CR_USER_DEFINED;
  return true;
}

bool is_modifiable_lvalue(const ExprInfo & expr)
{
  return expr.category == VC_LVALUE &&
         expr.type &&
         !type_is_const_object(expr.type);
}

ConversionRank implicit_object_conversion_rank(SemanticContext & ctx,
                                               const TypePtr & target,
                                               const ExprInfo & arg)
{
  TypePtr target_base = strip_top_level_cv(target);
  TypePtr converted = value_conversion_type(arg);
  TypePtr converted_base = strip_top_level_cv(converted);
  if(target_base && converted_base &&
     target_base->kind == Type::TK_POINTER &&
     converted_base->kind == Type::TK_POINTER) {
    if(type_equals(target_base, converted_base)) {
      return CR_EXACT;
    }
    if(same_type_with_compatible_top_cv(target_base->inner, converted_base->inner)) {
      return CR_EXACT;
    }
  }
  return conversion_rank(ctx, target, arg);
}

bool result_value_category_for_function_result(const TypePtr & result_type,
                                               semantic_conversion::ValueCategory & out)
{
  if(!result_type) {
    return false;
  }

  TypePtr base = strip_top_level_cv(result_type);
  if(base->kind == Type::TK_LVALUE_REFERENCE) {
    out = VC_LVALUE;
    return true;
  }
  if(base->kind == Type::TK_RVALUE_REFERENCE) {
    out = VC_XVALUE;
    return true;
  }

  out = VC_PRVALUE;
  return true;
}

TypePtr expression_type_for_function_result(const TypePtr & result_type)
{
  TypePtr expression_type = remove_reference_type(result_type);
  return expression_type ? expression_type : result_type;
}

}  // namespace semantic_conversion
