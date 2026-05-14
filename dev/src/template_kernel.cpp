#include "template_kernel.h"

#include <cctype>
#include <fstream>
#include <map>
#include <set>
#include <sstream>
#include <string>
#include <vector>

namespace template_kernel {
namespace {

struct TypeExpr
{
  enum RefKind
  {
    RK_NONE,
    RK_LVALUE,
    RK_RVALUE
  };

  std::string name;
  bool is_const = false;
  bool is_volatile = false;
  int pointer_depth = 0;
  RefKind ref_kind = RK_NONE;
  std::vector<TypeExpr> template_args;
};

struct TemplateParam
{
  enum Kind
  {
    PK_TYPE,
    PK_VALUE
  };

  Kind kind = PK_TYPE;
  std::string name;
  std::string value_type_name;
  bool has_default = false;
  TypeExpr default_type;
};

struct ClassTemplateDecl
{
  std::string name;
  std::vector<TemplateParam> parameters;
};

struct VariableTemplateDecl
{
  std::string name;
  std::vector<TemplateParam> parameters;
};

struct AliasTemplateDecl
{
  std::string name;
  std::vector<TemplateParam> parameters;
  TypeExpr target;
};

struct PartialClassDecl
{
  std::string primary_name;
  std::string id;
  std::vector<TemplateParam> parameters;
  std::vector<TypeExpr> patterns;
};

struct PartialVariableDecl
{
  std::string primary_name;
  std::string id;
  std::vector<TemplateParam> parameters;
  std::vector<TypeExpr> patterns;
};

struct ExplicitClassDecl
{
  std::string primary_name;
  std::string id;
  std::vector<TypeExpr> arguments;
};

struct ExplicitVariableDecl
{
  std::string primary_name;
  std::string id;
  std::vector<TypeExpr> arguments;
};

struct Requirement
{
  bool present = false;
  std::string left_parameter;
  TypeExpr right_type;
};

struct FunctionTemplateDecl
{
  std::string name;
  std::string id;
  std::vector<TemplateParam> parameters;
  std::vector<TypeExpr> parameter_patterns;
  TypeExpr return_type;
  Requirement requirement;
};

struct Query
{
  enum Kind
  {
    QK_SELECT_CLASS,
    QK_SELECT_VARIABLE,
    QK_EXPAND_ALIAS,
    QK_BIND_DEFAULTS,
    QK_DEDUCE_CALL,
    QK_STATS
  };

  enum TargetKind
  {
    TK_NONE,
    TK_CLASS,
    TK_VARIABLE,
    TK_ALIAS
  };

  Kind kind = QK_SELECT_CLASS;
  TargetKind target_kind = TK_NONE;
  std::string target_name;
  std::vector<TypeExpr> arguments;
};

struct CandidateResult
{
  std::string id;
  bool viable = false;
  std::map<std::string, TypeExpr> deduced;
  std::vector<std::string> applied_defaults;
  std::string failure_reason;
  int conversion_penalty = 0;
  int specialization_score = 0;
};

struct Metrics
{
  int queries_executed = 0;
  int stats_queries = 0;
  int class_select_queries = 0;
  int variable_select_queries = 0;
  int alias_expand_queries = 0;
  int default_binding_queries = 0;
  int function_deduction_queries = 0;
  int candidate_evaluations = 0;
  int defaults_applied = 0;
  int text_fallbacks = 0;
};

struct Program
{
  std::map<std::string, ClassTemplateDecl> class_templates;
  std::map<std::string, VariableTemplateDecl> variable_templates;
  std::map<std::string, AliasTemplateDecl> alias_templates;
  std::vector<PartialClassDecl> partial_classes;
  std::vector<PartialVariableDecl> partial_variables;
  std::vector<ExplicitClassDecl> explicit_classes;
  std::vector<ExplicitVariableDecl> explicit_variables;
  std::map<std::string, std::vector<FunctionTemplateDecl> > function_templates;
  std::vector<Query> queries;
};

struct BindingResult
{
  bool ok = false;
  std::string reason;
  std::vector<TypeExpr> resolved_arguments;
  std::map<std::string, TypeExpr> bindings;
  std::vector<std::string> applied_defaults;
};

struct SelectionResult
{
  enum SelectionKind
  {
    SK_PRIMARY,
    SK_PARTIAL,
    SK_EXPLICIT
  };

  bool ok = false;
  std::string reason;
  SelectionKind selection_kind = SK_PRIMARY;
  std::string selected_id;
  BindingResult primary_binding;
  std::map<std::string, TypeExpr> specialization_bindings;
};

struct PendingCheck
{
  TypeExpr pattern;
  TypeExpr actual;
};

std::string trim(const std::string & text)
{
  std::size_t begin = 0;
  while(begin < text.size() && std::isspace(static_cast<unsigned char>(text[begin]))) {
    ++begin;
  }
  std::size_t end = text.size();
  while(end > begin && std::isspace(static_cast<unsigned char>(text[end - 1]))) {
    --end;
  }
  return text.substr(begin, end - begin);
}

bool starts_with(const std::string & text, const std::string & prefix)
{
  return text.size() >= prefix.size() &&
         text.compare(0, prefix.size(), prefix) == 0;
}

std::string consume_token(std::string & text)
{
  text = trim(text);
  std::size_t end = 0;
  while(end < text.size() && !std::isspace(static_cast<unsigned char>(text[end]))) {
    ++end;
  }
  const std::string token = text.substr(0, end);
  text = trim(text.substr(end));
  return token;
}

bool consume_prefix(std::string & text, const std::string & prefix)
{
  text = trim(text);
  if(!starts_with(text, prefix)) {
    return false;
  }
  text = trim(text.substr(prefix.size()));
  return true;
}

bool consume_group(std::string & text, char open, char close, std::string & out)
{
  text = trim(text);
  if(text.empty() || text[0] != open) {
    return false;
  }

  int depth = 0;
  for(std::size_t i = 0; i < text.size(); ++i) {
    if(text[i] == open) {
      ++depth;
    } else if(text[i] == close) {
      --depth;
      if(depth == 0) {
        out = text.substr(1, i - 1);
        text = trim(text.substr(i + 1));
        return true;
      }
    }
  }
  return false;
}

std::vector<std::string> split_top_level(const std::string & text, char delimiter)
{
  std::vector<std::string> out;
  int angle_depth = 0;
  int paren_depth = 0;
  std::size_t current = 0;
  for(std::size_t i = 0; i < text.size(); ++i) {
    if(text[i] == '<') {
      ++angle_depth;
    } else if(text[i] == '>') {
      --angle_depth;
    } else if(text[i] == '(') {
      ++paren_depth;
    } else if(text[i] == ')') {
      --paren_depth;
    } else if(text[i] == delimiter && angle_depth == 0 && paren_depth == 0) {
      out.push_back(trim(text.substr(current, i - current)));
      current = i + 1;
    }
  }
  out.push_back(trim(text.substr(current)));
  return out;
}

bool parse_type_expr(const std::string & text, TypeExpr & out, std::string & error);
bool type_equals(const TypeExpr & left, const TypeExpr & right);

bool merge_template_param_lists(std::vector<TemplateParam> & existing,
                                const std::vector<TemplateParam> & incoming,
                                const std::string & kind,
                                const std::string & name,
                                std::string & error)
{
  if(existing.size() != incoming.size()) {
    error = "mismatched_template_parameter_arity_for_redeclaration " +
            kind + " " + name;
    return false;
  }

  for(std::size_t i = 0; i < existing.size(); ++i) {
    if(existing[i].kind != incoming[i].kind ||
       existing[i].value_type_name != incoming[i].value_type_name) {
      error = "mismatched_template_parameter_kind_for_redeclaration " +
              kind + " " + name + " parameter " + std::to_string(i + 1);
      return false;
    }

    if(existing[i].name != incoming[i].name) {
      error = "mismatched_template_parameter_name_for_redeclaration " +
              kind + " " + name + " parameter " + std::to_string(i + 1);
      return false;
    }

    if(existing[i].has_default && incoming[i].has_default &&
       !type_equals(existing[i].default_type, incoming[i].default_type)) {
      error = "conflicting_default_template_argument_for_redeclaration " +
              kind + " " + name + " parameter " + existing[i].name;
      return false;
    }

    if(!existing[i].has_default && incoming[i].has_default) {
      existing[i].has_default = true;
      existing[i].default_type = incoming[i].default_type;
    }
  }

  return true;
}

bool parse_type_list_payload(const std::string & payload,
                             std::vector<TypeExpr> & out,
                             std::string & error)
{
  const std::string trimmed = trim(payload);
  if(trimmed.empty()) {
    out.clear();
    return true;
  }

  const std::vector<std::string> parts = split_top_level(trimmed, ',');
  out.clear();
  for(std::size_t i = 0; i < parts.size(); ++i) {
    if(parts[i].empty()) {
      error = "empty type entry";
      return false;
    }
    std::string current = parts[i];
    if(!consume_prefix(current, "type ")) {
      error = "expected 'type' prefix in type list entry: " + parts[i];
      return false;
    }
    TypeExpr parsed;
    if(!parse_type_expr(current, parsed, error)) {
      return false;
    }
    out.push_back(parsed);
  }
  return true;
}

std::size_t find_top_level_char(const std::string & text, char ch)
{
  int angle_depth = 0;
  int paren_depth = 0;
  for(std::size_t i = 0; i < text.size(); ++i) {
    if(text[i] == '<') {
      ++angle_depth;
    } else if(text[i] == '>') {
      --angle_depth;
    } else if(text[i] == '(') {
      ++paren_depth;
    } else if(text[i] == ')') {
      --paren_depth;
    } else if(text[i] == ch && angle_depth == 0 && paren_depth == 0) {
      return i;
    }
  }
  return std::string::npos;
}

bool parse_template_param_list_payload(const std::string & payload,
                                       std::vector<TemplateParam> & out,
                                       std::string & error)
{
  const std::string trimmed = trim(payload);
  if(trimmed.empty()) {
    out.clear();
    return true;
  }

  const std::vector<std::string> parts = split_top_level(trimmed, ',');
  out.clear();
  for(std::size_t i = 0; i < parts.size(); ++i) {
    const std::size_t eq = find_top_level_char(parts[i], '=');
    std::string left = eq == std::string::npos ? parts[i] : parts[i].substr(0, eq);
    std::string right = eq == std::string::npos ? std::string() : parts[i].substr(eq + 1);
    left = trim(left);
    right = trim(right);

    TemplateParam param;
    if(starts_with(left, "type ")) {
      param.kind = TemplateParam::PK_TYPE;
      param.name = trim(left.substr(5));
    } else if(starts_with(left, "value ")) {
      param.kind = TemplateParam::PK_VALUE;
      std::string rest = trim(left.substr(6));
      param.value_type_name = consume_token(rest);
      param.name = trim(rest);
      if(param.value_type_name != "int" && param.value_type_name != "bool") {
        error = "unsupported value parameter type in parameter list entry: " + parts[i];
        return false;
      }
    } else {
      error = "expected 'type' or 'value' prefix in parameter list entry: " + parts[i];
      return false;
    }

    if(param.name.empty()) {
      error = "missing template parameter name";
      return false;
    }

    if(eq != std::string::npos) {
      param.has_default = true;
      if(starts_with(right, "type ")) {
        right = trim(right.substr(5));
      } else if(starts_with(right, "value ")) {
        right = trim(right.substr(6));
      }
      if(!parse_type_expr(right, param.default_type, error)) {
        return false;
      }
    }

    out.push_back(param);
  }
  return true;
}

bool parse_type_expr(const std::string & text, TypeExpr & out, std::string & error)
{
  std::string current = trim(text);
  if(current.empty()) {
    error = "empty type expression";
    return false;
  }

  out = TypeExpr();
  if(current.size() >= 2 && current.substr(current.size() - 2) == "&&") {
    out.ref_kind = TypeExpr::RK_RVALUE;
    current = trim(current.substr(0, current.size() - 2));
  } else if(!current.empty() && current[current.size() - 1] == '&') {
    out.ref_kind = TypeExpr::RK_LVALUE;
    current = trim(current.substr(0, current.size() - 1));
  }

  while(!current.empty() && current[current.size() - 1] == '*') {
    ++out.pointer_depth;
    current = trim(current.substr(0, current.size() - 1));
  }

  bool consumed_qualifier = true;
  while(consumed_qualifier) {
    consumed_qualifier = false;
    if(starts_with(current, "const ")) {
      out.is_const = true;
      current = trim(current.substr(6));
      consumed_qualifier = true;
    }
    if(starts_with(current, "volatile ")) {
      out.is_volatile = true;
      current = trim(current.substr(9));
      consumed_qualifier = true;
    }
  }

  const std::size_t angle = current.find('<');
  if(angle == std::string::npos) {
    out.name = trim(current);
    if(out.name.empty()) {
      error = "missing type name";
      return false;
    }
    return true;
  }

  if(current[current.size() - 1] != '>') {
    error = "unterminated template-id type expression: " + text;
    return false;
  }

  out.name = trim(current.substr(0, angle));
  if(out.name.empty()) {
    error = "missing type name before template arguments";
    return false;
  }

  const std::string arg_payload =
      current.substr(angle + 1, current.size() - angle - 2);
  const std::vector<std::string> arg_parts = split_top_level(arg_payload, ',');
  for(std::size_t i = 0; i < arg_parts.size(); ++i) {
    TypeExpr arg;
    if(!parse_type_expr(arg_parts[i], arg, error)) {
      return false;
    }
    out.template_args.push_back(arg);
  }
  return true;
}

bool is_nondeduced_wrapper(const TypeExpr & type)
{
  return type.name == "nondeduced" && type.pointer_depth == 0 && type.template_args.size() == 1;
}

std::string format_type(const TypeExpr & type)
{
  std::ostringstream out;
  if(type.is_const) {
    out << "const ";
  }
  if(type.is_volatile) {
    out << "volatile ";
  }
  out << type.name;
  if(!type.template_args.empty()) {
    out << "<";
    for(std::size_t i = 0; i < type.template_args.size(); ++i) {
      if(i != 0) {
        out << ", ";
      }
      out << format_type(type.template_args[i]);
    }
    out << ">";
  }
  for(int i = 0; i < type.pointer_depth; ++i) {
    out << "*";
  }
  if(type.ref_kind == TypeExpr::RK_LVALUE) {
    out << "&";
  } else if(type.ref_kind == TypeExpr::RK_RVALUE) {
    out << "&&";
  }
  return out.str();
}

std::string format_type_list(const std::vector<TypeExpr> & types)
{
  std::ostringstream out;
  for(std::size_t i = 0; i < types.size(); ++i) {
    if(i != 0) {
      out << ", ";
    }
    out << format_type(types[i]);
  }
  return out.str();
}

bool type_equals(const TypeExpr & left, const TypeExpr & right)
{
  if(left.name != right.name ||
     left.is_const != right.is_const ||
     left.is_volatile != right.is_volatile ||
     left.pointer_depth != right.pointer_depth ||
     left.ref_kind != right.ref_kind ||
     left.template_args.size() != right.template_args.size()) {
    return false;
  }
  for(std::size_t i = 0; i < left.template_args.size(); ++i) {
    if(!type_equals(left.template_args[i], right.template_args[i])) {
      return false;
    }
  }
  return true;
}

TypeExpr strip_reference(const TypeExpr & type)
{
  TypeExpr out = type;
  out.ref_kind = TypeExpr::RK_NONE;
  return out;
}

TypeExpr::RefKind collapse_ref_kind(TypeExpr::RefKind left, TypeExpr::RefKind right)
{
  if(left == TypeExpr::RK_LVALUE || right == TypeExpr::RK_LVALUE) {
    return TypeExpr::RK_LVALUE;
  }
  if(left == TypeExpr::RK_RVALUE || right == TypeExpr::RK_RVALUE) {
    return TypeExpr::RK_RVALUE;
  }
  return TypeExpr::RK_NONE;
}

bool is_forwarding_reference_parameter(const TypeExpr & pattern)
{
  return pattern.ref_kind == TypeExpr::RK_RVALUE &&
         !pattern.is_const &&
         !pattern.is_volatile &&
         pattern.pointer_depth == 0 &&
         pattern.template_args.empty();
}

int pattern_specificity_score(const TypeExpr & pattern,
                              const std::set<std::string> & parameter_names)
{
  if(is_nondeduced_wrapper(pattern)) {
    return pattern_specificity_score(pattern.template_args[0], parameter_names);
  }

  int score = 0;
  if(!(pattern.template_args.empty() && parameter_names.count(pattern.name) != 0)) {
    score += 1;
  }
  if(pattern.is_const) {
    score += 1;
  }
  if(pattern.is_volatile) {
    score += 1;
  }
  score += pattern.pointer_depth;
  if(pattern.ref_kind != TypeExpr::RK_NONE) {
    score += 1;
  }
  for(std::size_t i = 0; i < pattern.template_args.size(); ++i) {
    score += pattern_specificity_score(pattern.template_args[i], parameter_names);
  }
  return score;
}

int pattern_specificity_score(const std::vector<TypeExpr> & patterns,
                              const std::vector<TemplateParam> & parameters)
{
  std::set<std::string> parameter_names;
  for(std::size_t i = 0; i < parameters.size(); ++i) {
    parameter_names.insert(parameters[i].name);
  }
  int score = 0;
  for(std::size_t i = 0; i < patterns.size(); ++i) {
    score += pattern_specificity_score(patterns[i], parameter_names);
  }
  return score;
}

void substitute_type(const TypeExpr & input,
                     const std::map<std::string, TypeExpr> & bindings,
                     TypeExpr & out)
{
  if(is_nondeduced_wrapper(input)) {
    out = input;
    substitute_type(input.template_args[0], bindings, out.template_args[0]);
    return;
  }

  if(input.template_args.empty()) {
    const std::map<std::string, TypeExpr>::const_iterator found = bindings.find(input.name);
    if(found != bindings.end()) {
      out = found->second;
      out.is_const = out.is_const || input.is_const;
      out.is_volatile = out.is_volatile || input.is_volatile;
      out.pointer_depth += input.pointer_depth;
      out.ref_kind = collapse_ref_kind(out.ref_kind, input.ref_kind);
      return;
    }
  }

  out = input;
  for(std::size_t i = 0; i < input.template_args.size(); ++i) {
    substitute_type(input.template_args[i], bindings, out.template_args[i]);
  }
}

void record_default_application(std::vector<std::string> & applied_defaults,
                                const std::string & name,
                                Metrics & metrics)
{
  for(std::size_t i = 0; i < applied_defaults.size(); ++i) {
    if(applied_defaults[i] == name) {
      return;
    }
  }
  applied_defaults.push_back(name);
  ++metrics.defaults_applied;
}

bool resolve_template_arguments(const std::vector<TemplateParam> & parameters,
                                const std::vector<TypeExpr> & provided_arguments,
                                BindingResult & out,
                                Metrics & metrics)
{
  out = BindingResult();
  if(provided_arguments.size() > parameters.size()) {
    out.reason = "too_many_template_arguments";
    return false;
  }

  for(std::size_t i = 0; i < provided_arguments.size(); ++i) {
    out.resolved_arguments.push_back(provided_arguments[i]);
    out.bindings[parameters[i].name] = provided_arguments[i];
  }

  for(std::size_t i = provided_arguments.size(); i < parameters.size(); ++i) {
    if(!parameters[i].has_default) {
      out.reason = "missing_template_argument " + parameters[i].name;
      return false;
    }

    TypeExpr substituted_default;
    substitute_type(parameters[i].default_type, out.bindings, substituted_default);
    out.resolved_arguments.push_back(substituted_default);
    out.bindings[parameters[i].name] = substituted_default;
    record_default_application(out.applied_defaults, parameters[i].name, metrics);
  }

  out.ok = true;
  return true;
}

bool can_bind_pattern_reference(const TypeExpr & pattern,
                                const TypeExpr & actual,
                                std::string & reason)
{
  if(pattern.ref_kind == TypeExpr::RK_NONE) {
    return true;
  }
  if(pattern.ref_kind == TypeExpr::RK_LVALUE) {
    if(actual.ref_kind == TypeExpr::RK_LVALUE) {
      return true;
    }
    if(actual.ref_kind == TypeExpr::RK_RVALUE && pattern.is_const) {
      return true;
    }
    reason = "non_matching_reference_kind";
    return false;
  }
  if(actual.ref_kind == TypeExpr::RK_RVALUE) {
    return true;
  }
  reason = "non_matching_reference_kind";
  return false;
}

TypeExpr strip_pattern_wrappers_for_deduction(const TypeExpr & pattern,
                                              const TypeExpr & actual)
{
  TypeExpr bound = actual;
  if(pattern.ref_kind != TypeExpr::RK_NONE || bound.ref_kind != TypeExpr::RK_NONE) {
    bound = strip_reference(bound);
  }
  bound.pointer_depth -= pattern.pointer_depth;
  if(pattern.is_const && bound.is_const) {
    bound.is_const = false;
  }
  if(pattern.is_volatile && bound.is_volatile) {
    bound.is_volatile = false;
  }
  return bound;
}

bool match_type_pattern(const TypeExpr & pattern,
                        const TypeExpr & actual,
                        const std::set<std::string> & parameter_names,
                        std::map<std::string, TypeExpr> & deduced,
                        std::vector<PendingCheck> & pending,
                        std::string & reason,
                        bool allow_deduction,
                        bool allow_pending)
{
  if(is_nondeduced_wrapper(pattern)) {
    return match_type_pattern(pattern.template_args[0],
                              actual,
                              parameter_names,
                              deduced,
                              pending,
                              reason,
                              false,
                              allow_pending);
  }

  const bool direct_parameter =
      pattern.template_args.empty() && parameter_names.count(pattern.name) != 0;
  if(direct_parameter) {
    if(allow_deduction) {
      TypeExpr bound;
      if(is_forwarding_reference_parameter(pattern) &&
         actual.ref_kind == TypeExpr::RK_LVALUE) {
        bound = actual;
      } else {
        if(!can_bind_pattern_reference(pattern, actual, reason)) {
          return false;
        }
        if(actual.pointer_depth < pattern.pointer_depth) {
          reason = "non_matching_parameter_type";
          return false;
        }
        bound = strip_pattern_wrappers_for_deduction(pattern, actual);
      }
      const std::map<std::string, TypeExpr>::const_iterator found = deduced.find(pattern.name);
      if(found == deduced.end()) {
        deduced[pattern.name] = bound;
        return true;
      }
      if(type_equals(found->second, bound)) {
        return true;
      }
      reason = "conflicting_deduction " + pattern.name + "=" + format_type(found->second) +
               " vs " + pattern.name + "=" + format_type(bound);
      return false;
    }

    const std::map<std::string, TypeExpr>::const_iterator found = deduced.find(pattern.name);
    if(found == deduced.end()) {
      if(allow_pending) {
        PendingCheck check;
        check.pattern = pattern;
        check.actual = actual;
        pending.push_back(check);
        return true;
      }
      reason = "parameter_only_in_nondeduced_context " + pattern.name;
      return false;
    }

    TypeExpr expected = found->second;
    expected.is_const = expected.is_const || pattern.is_const;
    expected.is_volatile = expected.is_volatile || pattern.is_volatile;
    expected.pointer_depth += pattern.pointer_depth;
    expected.ref_kind = collapse_ref_kind(expected.ref_kind, pattern.ref_kind);
    if(type_equals(expected, actual)) {
      return true;
    }
    reason = "non_matching_parameter_type";
    return false;
  }

  TypeExpr normalized_actual = actual;
  if(pattern.ref_kind == TypeExpr::RK_NONE) {
    normalized_actual = strip_reference(normalized_actual);
  }

  if(pattern.name != normalized_actual.name ||
     pattern.is_const != normalized_actual.is_const ||
     pattern.is_volatile != normalized_actual.is_volatile ||
     pattern.pointer_depth != normalized_actual.pointer_depth ||
     pattern.ref_kind != normalized_actual.ref_kind ||
     pattern.template_args.size() != normalized_actual.template_args.size()) {
    reason = "non_matching_parameter_type";
    return false;
  }

  for(std::size_t i = 0; i < pattern.template_args.size(); ++i) {
    if(!match_type_pattern(pattern.template_args[i],
                           normalized_actual.template_args[i],
                           parameter_names,
                           deduced,
                           pending,
                           reason,
                           allow_deduction,
                           allow_pending)) {
      return false;
    }
  }
  return true;
}

bool apply_remaining_defaults_to_deduced(const std::vector<TemplateParam> & parameters,
                                         std::map<std::string, TypeExpr> & deduced,
                                         std::vector<std::string> & applied_defaults,
                                         Metrics & metrics,
                                         std::string & reason)
{
  for(std::size_t i = 0; i < parameters.size(); ++i) {
    if(deduced.find(parameters[i].name) != deduced.end()) {
      continue;
    }
    if(!parameters[i].has_default) {
      reason = "missing_deduced_template_argument " + parameters[i].name;
      return false;
    }
    TypeExpr substituted_default;
    substitute_type(parameters[i].default_type, deduced, substituted_default);
    deduced[parameters[i].name] = substituted_default;
    record_default_application(applied_defaults, parameters[i].name, metrics);
  }
  return true;
}

bool evaluate_requirement(const Requirement & requirement,
                          const std::map<std::string, TypeExpr> & deduced,
                          std::string & reason)
{
  if(!requirement.present) {
    return true;
  }

  const std::map<std::string, TypeExpr>::const_iterator left =
      deduced.find(requirement.left_parameter);
  if(left == deduced.end()) {
    reason = "requirement_undeduced_parameter " + requirement.left_parameter;
    return false;
  }

  TypeExpr substituted_right;
  substitute_type(requirement.right_type, deduced, substituted_right);
  if(type_equals(left->second, substituted_right)) {
    return true;
  }

  reason = "requirement_not_satisfied " + requirement.left_parameter + "=" +
           format_type(left->second) + " vs " + format_type(substituted_right);
  return false;
}

bool parameter_matches_actual(const TypeExpr & parameter,
                              const TypeExpr & actual,
                              int & penalty,
                              std::string & reason)
{
  if(is_nondeduced_wrapper(parameter)) {
    return parameter_matches_actual(parameter.template_args[0], actual, penalty, reason);
  }

  penalty = 0;

  TypeExpr normalized_parameter = parameter;
  TypeExpr normalized_actual = actual;
  if(normalized_parameter.ref_kind == TypeExpr::RK_NONE) {
    normalized_actual = strip_reference(normalized_actual);
  } else if(normalized_parameter.ref_kind == TypeExpr::RK_LVALUE) {
    if(normalized_actual.ref_kind == TypeExpr::RK_LVALUE) {
      normalized_parameter.ref_kind = TypeExpr::RK_NONE;
      normalized_actual.ref_kind = TypeExpr::RK_NONE;
    } else if(normalized_actual.ref_kind == TypeExpr::RK_RVALUE &&
              normalized_parameter.is_const) {
      normalized_parameter.ref_kind = TypeExpr::RK_NONE;
      normalized_actual.ref_kind = TypeExpr::RK_NONE;
      ++penalty;
    } else {
      reason = "non_matching_reference_kind";
      return false;
    }
  } else {
    if(normalized_actual.ref_kind != TypeExpr::RK_RVALUE) {
      reason = "non_matching_reference_kind";
      return false;
    }
    normalized_parameter.ref_kind = TypeExpr::RK_NONE;
    normalized_actual.ref_kind = TypeExpr::RK_NONE;
  }

  if(normalized_parameter.name != normalized_actual.name ||
     normalized_parameter.pointer_depth != normalized_actual.pointer_depth ||
     normalized_parameter.template_args.size() != normalized_actual.template_args.size()) {
    reason = "non_matching_parameter_type";
    return false;
  }

  for(std::size_t i = 0; i < normalized_parameter.template_args.size(); ++i) {
    if(!type_equals(normalized_parameter.template_args[i], normalized_actual.template_args[i])) {
      reason = "non_matching_parameter_type";
      return false;
    }
  }

  if(normalized_actual.is_const && !normalized_parameter.is_const) {
    reason = "non_matching_parameter_type";
    return false;
  }
  if(normalized_actual.is_volatile && !normalized_parameter.is_volatile) {
    reason = "non_matching_parameter_type";
    return false;
  }
  if(normalized_parameter.is_const && !normalized_actual.is_const) {
    ++penalty;
  }
  if(normalized_parameter.is_volatile && !normalized_actual.is_volatile) {
    ++penalty;
  }

  return true;
}

bool compute_conversion_penalty(const FunctionTemplateDecl & decl,
                                const std::map<std::string, TypeExpr> & deduced,
                                const std::vector<TypeExpr> & actuals,
                                int & penalty,
                                std::string & reason)
{
  penalty = 0;
  if(decl.parameter_patterns.size() != actuals.size()) {
    reason = "arity_mismatch";
    return false;
  }

  for(std::size_t i = 0; i < decl.parameter_patterns.size(); ++i) {
    TypeExpr substituted;
    substitute_type(decl.parameter_patterns[i], deduced, substituted);
    int parameter_penalty = 0;
    if(!parameter_matches_actual(substituted, actuals[i], parameter_penalty, reason)) {
      return false;
    }
    penalty += parameter_penalty;
  }
  return true;
}

bool match_template_pattern_arguments(const std::vector<TypeExpr> & patterns,
                                      const std::vector<TemplateParam> & parameters,
                                      const std::vector<TypeExpr> & actuals,
                                      std::map<std::string, TypeExpr> & deduced,
                                      std::vector<std::string> & applied_defaults,
                                      Metrics & metrics,
                                      std::string & reason)
{
  deduced.clear();
  applied_defaults.clear();
  if(patterns.size() != actuals.size()) {
    reason = "arity_mismatch";
    return false;
  }

  std::set<std::string> parameter_names;
  for(std::size_t i = 0; i < parameters.size(); ++i) {
    parameter_names.insert(parameters[i].name);
  }

  std::vector<PendingCheck> pending;
  for(std::size_t i = 0; i < patterns.size(); ++i) {
    if(!match_type_pattern(patterns[i],
                           actuals[i],
                           parameter_names,
                           deduced,
                           pending,
                           reason,
                           true,
                           true)) {
      return false;
    }
  }

  if(!apply_remaining_defaults_to_deduced(parameters, deduced, applied_defaults, metrics, reason)) {
    return false;
  }

  for(std::size_t i = 0; i < pending.size(); ++i) {
    std::vector<PendingCheck> ignored_pending;
    if(!match_type_pattern(pending[i].pattern,
                           pending[i].actual,
                           parameter_names,
                           deduced,
                           ignored_pending,
                           reason,
                           false,
                           false)) {
      return false;
    }
  }

  return true;
}

std::string format_instantiation(const std::string & name, const std::vector<TypeExpr> & arguments)
{
  std::ostringstream out;
  out << name << "<" << format_type_list(arguments) << ">";
  return out.str();
}

std::string target_kind_name(Query::TargetKind kind)
{
  if(kind == Query::TK_CLASS) {
    return "class";
  }
  if(kind == Query::TK_VARIABLE) {
    return "variable";
  }
  if(kind == Query::TK_ALIAS) {
    return "alias";
  }
  return "unknown";
}

void write_primary_bindings(std::ostream & out,
                            const std::vector<TemplateParam> & parameters,
                            const std::map<std::string, TypeExpr> & bindings)
{
  for(std::size_t i = 0; i < parameters.size(); ++i) {
    const std::map<std::string, TypeExpr>::const_iterator found =
        bindings.find(parameters[i].name);
    if(found == bindings.end()) {
      continue;
    }
    out << "bind " << parameters[i].name << " = " << format_type(found->second) << "\n";
  }
}

void write_defaults(std::ostream & out,
                    const std::vector<std::string> & defaults,
                    const std::map<std::string, TypeExpr> & bindings)
{
  for(std::size_t i = 0; i < defaults.size(); ++i) {
    const std::map<std::string, TypeExpr>::const_iterator found = bindings.find(defaults[i]);
    if(found == bindings.end()) {
      continue;
    }
    out << "default " << defaults[i] << " = " << format_type(found->second) << "\n";
  }
}

void write_specialization_bindings(std::ostream & out,
                                   const std::vector<TemplateParam> & parameters,
                                   const std::map<std::string, TypeExpr> & bindings)
{
  for(std::size_t i = 0; i < parameters.size(); ++i) {
    const std::map<std::string, TypeExpr>::const_iterator found =
        bindings.find(parameters[i].name);
    if(found == bindings.end()) {
      continue;
    }
    out << "specialize " << parameters[i].name << " = " << format_type(found->second) << "\n";
  }
}

bool parse_requirement(std::string & text, Requirement & requirement, std::string & error)
{
  requirement = Requirement();
  text = trim(text);
  if(text.empty()) {
    return true;
  }

  if(!consume_prefix(text, "when ")) {
    error = "unexpected trailing tokens after declaration";
    return false;
  }

  if(!consume_prefix(text, "same")) {
    error = "only when same(...) requirements are supported";
    return false;
  }

  std::string payload;
  if(!consume_group(text, '(', ')', payload)) {
    error = "malformed same(...) requirement";
    return false;
  }

  const std::vector<std::string> parts = split_top_level(payload, ',');
  if(parts.size() != 2) {
    error = "same(...) requires exactly two operands";
    return false;
  }

  requirement.present = true;
  requirement.left_parameter = trim(parts[0]);
  std::string right = trim(parts[1]);
  if(starts_with(right, "type ")) {
    right = trim(right.substr(5));
  }
  if(!parse_type_expr(right, requirement.right_type, error)) {
    return false;
  }

  if(!text.empty()) {
    error = "unexpected trailing tokens after requirement";
    return false;
  }
  return true;
}

bool parse_program(const std::string & input_path, Program & program, std::string & error)
{
  std::ifstream input(input_path.c_str());
  if(!input) {
    error = "failed to open input file: " + input_path;
    return false;
  }

  std::string line;
  bool header_seen = false;
  int line_number = 0;
  while(std::getline(input, line)) {
    ++line_number;
    line = trim(line);
    if(line.empty() || starts_with(line, "#")) {
      continue;
    }

    if(!header_seen) {
      if(line != "template_kernel_v1") {
        error = "expected template_kernel_v1 header";
        return false;
      }
      header_seen = true;
      continue;
    }

    if(starts_with(line, "class_template ")) {
      std::string rest = line.substr(std::string("class_template ").size());
      ClassTemplateDecl decl;
      decl.name = consume_token(rest);
      std::string group;
      if(decl.name.empty() ||
         !consume_group(rest, '<', '>', group) ||
         !parse_template_param_list_payload(group, decl.parameters, error) ||
         !rest.empty()) {
        if(error.empty()) {
          error = "malformed class_template on line " + std::to_string(line_number);
        }
        return false;
      }
      std::map<std::string, ClassTemplateDecl>::iterator found =
          program.class_templates.find(decl.name);
      if(found == program.class_templates.end()) {
        program.class_templates[decl.name] = decl;
      } else if(!merge_template_param_lists(found->second.parameters,
                                            decl.parameters,
                                            "class_template",
                                            decl.name,
                                            error)) {
        return false;
      }
      continue;
    }

    if(starts_with(line, "variable_template ")) {
      std::string rest = line.substr(std::string("variable_template ").size());
      VariableTemplateDecl decl;
      decl.name = consume_token(rest);
      std::string group;
      if(decl.name.empty() ||
         !consume_group(rest, '<', '>', group) ||
         !parse_template_param_list_payload(group, decl.parameters, error) ||
         !rest.empty()) {
        if(error.empty()) {
          error = "malformed variable_template on line " + std::to_string(line_number);
        }
        return false;
      }
      std::map<std::string, VariableTemplateDecl>::iterator found =
          program.variable_templates.find(decl.name);
      if(found == program.variable_templates.end()) {
        program.variable_templates[decl.name] = decl;
      } else if(!merge_template_param_lists(found->second.parameters,
                                            decl.parameters,
                                            "variable_template",
                                            decl.name,
                                            error)) {
        return false;
      }
      continue;
    }

    if(starts_with(line, "alias_template ")) {
      std::string rest = line.substr(std::string("alias_template ").size());
      AliasTemplateDecl decl;
      decl.name = consume_token(rest);
      std::string group;
      if(decl.name.empty() ||
         !consume_group(rest, '<', '>', group) ||
         !parse_template_param_list_payload(group, decl.parameters, error) ||
         !consume_prefix(rest, "= ")) {
        if(error.empty()) {
          error = "malformed alias_template on line " + std::to_string(line_number);
        }
        return false;
      }
      if(starts_with(rest, "type ")) {
        rest = trim(rest.substr(5));
      }
      if(!parse_type_expr(rest, decl.target, error)) {
        return false;
      }
      program.alias_templates[decl.name] = decl;
      continue;
    }

    if(starts_with(line, "partial_class ")) {
      std::string rest = line.substr(std::string("partial_class ").size());
      PartialClassDecl decl;
      decl.primary_name = consume_token(rest);
      decl.id = consume_token(rest);
      std::string params_group;
      std::string args_group;
      if(decl.primary_name.empty() || decl.id.empty() ||
         !consume_group(rest, '<', '>', params_group) ||
         !parse_template_param_list_payload(params_group, decl.parameters, error) ||
         !consume_prefix(rest, "args ") ||
         !consume_group(rest, '<', '>', args_group) ||
         !parse_type_list_payload(args_group, decl.patterns, error) ||
         !rest.empty()) {
        if(error.empty()) {
          error = "malformed partial_class on line " + std::to_string(line_number);
        }
        return false;
      }
      program.partial_classes.push_back(decl);
      continue;
    }

    if(starts_with(line, "partial_variable ")) {
      std::string rest = line.substr(std::string("partial_variable ").size());
      PartialVariableDecl decl;
      decl.primary_name = consume_token(rest);
      decl.id = consume_token(rest);
      std::string params_group;
      std::string args_group;
      if(decl.primary_name.empty() || decl.id.empty() ||
         !consume_group(rest, '<', '>', params_group) ||
         !parse_template_param_list_payload(params_group, decl.parameters, error) ||
         !consume_prefix(rest, "args ") ||
         !consume_group(rest, '<', '>', args_group) ||
         !parse_type_list_payload(args_group, decl.patterns, error) ||
         !rest.empty()) {
        if(error.empty()) {
          error = "malformed partial_variable on line " + std::to_string(line_number);
        }
        return false;
      }
      program.partial_variables.push_back(decl);
      continue;
    }

    if(starts_with(line, "explicit_class ")) {
      std::string rest = line.substr(std::string("explicit_class ").size());
      ExplicitClassDecl decl;
      decl.primary_name = consume_token(rest);
      decl.id = consume_token(rest);
      std::string args_group;
      if(decl.primary_name.empty() || decl.id.empty() ||
         !consume_prefix(rest, "args ") ||
         !consume_group(rest, '<', '>', args_group) ||
         !parse_type_list_payload(args_group, decl.arguments, error) ||
         !rest.empty()) {
        if(error.empty()) {
          error = "malformed explicit_class on line " + std::to_string(line_number);
        }
        return false;
      }
      program.explicit_classes.push_back(decl);
      continue;
    }

    if(starts_with(line, "explicit_variable ")) {
      std::string rest = line.substr(std::string("explicit_variable ").size());
      ExplicitVariableDecl decl;
      decl.primary_name = consume_token(rest);
      decl.id = consume_token(rest);
      std::string args_group;
      if(decl.primary_name.empty() || decl.id.empty() ||
         !consume_prefix(rest, "args ") ||
         !consume_group(rest, '<', '>', args_group) ||
         !parse_type_list_payload(args_group, decl.arguments, error) ||
         !rest.empty()) {
        if(error.empty()) {
          error = "malformed explicit_variable on line " + std::to_string(line_number);
        }
        return false;
      }
      program.explicit_variables.push_back(decl);
      continue;
    }

    if(starts_with(line, "function_template ")) {
      std::string rest = line.substr(std::string("function_template ").size());
      FunctionTemplateDecl decl;
      decl.name = consume_token(rest);
      decl.id = consume_token(rest);
      std::string params_group;
      std::string args_group;
      if(decl.name.empty() || decl.id.empty() ||
         !consume_group(rest, '<', '>', params_group) ||
         !parse_template_param_list_payload(params_group, decl.parameters, error) ||
         !consume_prefix(rest, "params ") ||
         !consume_group(rest, '(', ')', args_group) ||
         !parse_type_list_payload(args_group, decl.parameter_patterns, error) ||
         !consume_prefix(rest, "return ")) {
        if(error.empty()) {
          error = "malformed function_template on line " + std::to_string(line_number);
        }
        return false;
      }
      std::size_t when_pos = rest.find(" when ");
      std::string return_text = when_pos == std::string::npos ? rest : rest.substr(0, when_pos);
      if(!parse_type_expr(return_text, decl.return_type, error)) {
        return false;
      }
      if(when_pos != std::string::npos) {
        rest = trim(rest.substr(when_pos));
      } else {
        rest.clear();
      }
      if(!parse_requirement(rest, decl.requirement, error)) {
        return false;
      }
      program.function_templates[decl.name].push_back(decl);
      continue;
    }

    if(starts_with(line, "query select_class ")) {
      std::string rest = line.substr(std::string("query select_class ").size());
      Query query;
      query.kind = Query::QK_SELECT_CLASS;
      query.target_name = consume_token(rest);
      std::string group;
      if(query.target_name.empty() ||
         !consume_group(rest, '<', '>', group) ||
         !parse_type_list_payload(group, query.arguments, error) ||
         !rest.empty()) {
        if(error.empty()) {
          error = "malformed select_class query on line " + std::to_string(line_number);
        }
        return false;
      }
      program.queries.push_back(query);
      continue;
    }

    if(starts_with(line, "query select_variable ")) {
      std::string rest = line.substr(std::string("query select_variable ").size());
      Query query;
      query.kind = Query::QK_SELECT_VARIABLE;
      query.target_name = consume_token(rest);
      std::string group;
      if(query.target_name.empty() ||
         !consume_group(rest, '<', '>', group) ||
         !parse_type_list_payload(group, query.arguments, error) ||
         !rest.empty()) {
        if(error.empty()) {
          error = "malformed select_variable query on line " + std::to_string(line_number);
        }
        return false;
      }
      program.queries.push_back(query);
      continue;
    }

    if(starts_with(line, "query expand_alias ")) {
      std::string rest = line.substr(std::string("query expand_alias ").size());
      Query query;
      query.kind = Query::QK_EXPAND_ALIAS;
      query.target_name = consume_token(rest);
      std::string group;
      if(query.target_name.empty() ||
         !consume_group(rest, '<', '>', group) ||
         !parse_type_list_payload(group, query.arguments, error) ||
         !rest.empty()) {
        if(error.empty()) {
          error = "malformed expand_alias query on line " + std::to_string(line_number);
        }
        return false;
      }
      program.queries.push_back(query);
      continue;
    }

    if(starts_with(line, "query bind_defaults ")) {
      std::string rest = line.substr(std::string("query bind_defaults ").size());
      Query query;
      query.kind = Query::QK_BIND_DEFAULTS;
      const std::string kind = consume_token(rest);
      query.target_kind = kind == "class" ? Query::TK_CLASS :
                          (kind == "variable" ? Query::TK_VARIABLE :
                           (kind == "alias" ? Query::TK_ALIAS : Query::TK_NONE));
      query.target_name = consume_token(rest);
      std::string group;
      if(query.target_kind == Query::TK_NONE || query.target_name.empty() ||
         !consume_group(rest, '<', '>', group) ||
         !parse_type_list_payload(group, query.arguments, error) ||
         !rest.empty()) {
        if(error.empty()) {
          error = "malformed bind_defaults query on line " + std::to_string(line_number);
        }
        return false;
      }
      program.queries.push_back(query);
      continue;
    }

    if(starts_with(line, "query deduce_call ")) {
      std::string rest = line.substr(std::string("query deduce_call ").size());
      Query query;
      query.kind = Query::QK_DEDUCE_CALL;
      query.target_name = consume_token(rest);
      std::string group;
      if(query.target_name.empty() ||
         !consume_prefix(rest, "args ") ||
         !consume_group(rest, '(', ')', group) ||
         !parse_type_list_payload(group, query.arguments, error) ||
         !rest.empty()) {
        if(error.empty()) {
          error = "malformed deduce_call query on line " + std::to_string(line_number);
        }
        return false;
      }
      program.queries.push_back(query);
      continue;
    }

    if(line == "query stats") {
      Query query;
      query.kind = Query::QK_STATS;
      program.queries.push_back(query);
      continue;
    }

    error = "unrecognized line " + std::to_string(line_number) + ": " + line;
    return false;
  }

  if(!header_seen) {
    error = "missing template_kernel_v1 header";
    return false;
  }

  return true;
}

bool select_class_specialization(const Program & program,
                                 const std::string & name,
                                 const std::vector<TypeExpr> & provided_arguments,
                                 SelectionResult & out,
                                 Metrics & metrics)
{
  out = SelectionResult();
  const std::map<std::string, ClassTemplateDecl>::const_iterator primary_found =
      program.class_templates.find(name);
  if(primary_found == program.class_templates.end()) {
    out.reason = "unknown_class_template";
    return false;
  }

  if(!resolve_template_arguments(primary_found->second.parameters,
                                 provided_arguments,
                                 out.primary_binding,
                                 metrics)) {
    out.reason = out.primary_binding.reason;
    return false;
  }

  int explicit_matches = 0;
  ExplicitClassDecl chosen_explicit;
  for(std::size_t i = 0; i < program.explicit_classes.size(); ++i) {
    if(program.explicit_classes[i].primary_name != name) {
      continue;
    }
    ++metrics.candidate_evaluations;
    if(program.explicit_classes[i].arguments.size() != out.primary_binding.resolved_arguments.size()) {
      continue;
    }
    bool match = true;
    for(std::size_t arg = 0; arg < program.explicit_classes[i].arguments.size(); ++arg) {
      if(!type_equals(program.explicit_classes[i].arguments[arg],
                      out.primary_binding.resolved_arguments[arg])) {
        match = false;
        break;
      }
    }
    if(match) {
      ++explicit_matches;
      chosen_explicit = program.explicit_classes[i];
    }
  }

  if(explicit_matches > 1) {
    out.reason = "ambiguous_explicit_class_specialization";
    return false;
  }
  if(explicit_matches == 1) {
    out.ok = true;
    out.selection_kind = SelectionResult::SK_EXPLICIT;
    out.selected_id = chosen_explicit.id;
    return true;
  }

  CandidateResult best_partial;
  bool have_partial = false;
  bool ambiguous = false;
  for(std::size_t i = 0; i < program.partial_classes.size(); ++i) {
    const PartialClassDecl & partial = program.partial_classes[i];
    if(partial.primary_name != name) {
      continue;
    }
    ++metrics.candidate_evaluations;
    CandidateResult current;
    current.id = partial.id;
    current.specialization_score = pattern_specificity_score(partial.patterns, partial.parameters);
    current.viable = match_template_pattern_arguments(partial.patterns,
                                                      partial.parameters,
                                                      out.primary_binding.resolved_arguments,
                                                      current.deduced,
                                                      current.applied_defaults,
                                                      metrics,
                                                      current.failure_reason);
    if(!current.viable) {
      continue;
    }

    if(!have_partial || current.specialization_score > best_partial.specialization_score) {
      best_partial = current;
      have_partial = true;
      ambiguous = false;
    } else if(current.specialization_score == best_partial.specialization_score) {
      ambiguous = true;
    }
  }

  if(ambiguous) {
    out.reason = "ambiguous_partial_class_specialization";
    return false;
  }

  out.ok = true;
  if(have_partial) {
    out.selection_kind = SelectionResult::SK_PARTIAL;
    out.selected_id = best_partial.id;
    out.specialization_bindings = best_partial.deduced;
  } else {
    out.selection_kind = SelectionResult::SK_PRIMARY;
    out.selected_id = name;
  }
  return true;
}

bool select_variable_specialization(const Program & program,
                                    const std::string & name,
                                    const std::vector<TypeExpr> & provided_arguments,
                                    SelectionResult & out,
                                    Metrics & metrics)
{
  out = SelectionResult();
  const std::map<std::string, VariableTemplateDecl>::const_iterator primary_found =
      program.variable_templates.find(name);
  if(primary_found == program.variable_templates.end()) {
    out.reason = "unknown_variable_template";
    return false;
  }

  if(!resolve_template_arguments(primary_found->second.parameters,
                                 provided_arguments,
                                 out.primary_binding,
                                 metrics)) {
    out.reason = out.primary_binding.reason;
    return false;
  }

  int explicit_matches = 0;
  ExplicitVariableDecl chosen_explicit;
  for(std::size_t i = 0; i < program.explicit_variables.size(); ++i) {
    if(program.explicit_variables[i].primary_name != name) {
      continue;
    }
    ++metrics.candidate_evaluations;
    if(program.explicit_variables[i].arguments.size() != out.primary_binding.resolved_arguments.size()) {
      continue;
    }
    bool match = true;
    for(std::size_t arg = 0; arg < program.explicit_variables[i].arguments.size(); ++arg) {
      if(!type_equals(program.explicit_variables[i].arguments[arg],
                      out.primary_binding.resolved_arguments[arg])) {
        match = false;
        break;
      }
    }
    if(match) {
      ++explicit_matches;
      chosen_explicit = program.explicit_variables[i];
    }
  }

  if(explicit_matches > 1) {
    out.reason = "ambiguous_explicit_variable_specialization";
    return false;
  }
  if(explicit_matches == 1) {
    out.ok = true;
    out.selection_kind = SelectionResult::SK_EXPLICIT;
    out.selected_id = chosen_explicit.id;
    return true;
  }

  CandidateResult best_partial;
  bool have_partial = false;
  bool ambiguous = false;
  for(std::size_t i = 0; i < program.partial_variables.size(); ++i) {
    const PartialVariableDecl & partial = program.partial_variables[i];
    if(partial.primary_name != name) {
      continue;
    }
    ++metrics.candidate_evaluations;
    CandidateResult current;
    current.id = partial.id;
    current.specialization_score = pattern_specificity_score(partial.patterns, partial.parameters);
    current.viable = match_template_pattern_arguments(partial.patterns,
                                                      partial.parameters,
                                                      out.primary_binding.resolved_arguments,
                                                      current.deduced,
                                                      current.applied_defaults,
                                                      metrics,
                                                      current.failure_reason);
    if(!current.viable) {
      continue;
    }

    if(!have_partial || current.specialization_score > best_partial.specialization_score) {
      best_partial = current;
      have_partial = true;
      ambiguous = false;
    } else if(current.specialization_score == best_partial.specialization_score) {
      ambiguous = true;
    }
  }

  if(ambiguous) {
    out.reason = "ambiguous_partial_variable_specialization";
    return false;
  }

  out.ok = true;
  if(have_partial) {
    out.selection_kind = SelectionResult::SK_PARTIAL;
    out.selected_id = best_partial.id;
    out.specialization_bindings = best_partial.deduced;
  } else {
    out.selection_kind = SelectionResult::SK_PRIMARY;
    out.selected_id = name;
  }
  return true;
}

bool expand_alias(const Program & program,
                  const std::string & name,
                  const std::vector<TypeExpr> & provided_arguments,
                  BindingResult & binding,
                  TypeExpr & expanded,
                  Metrics & metrics)
{
  const std::map<std::string, AliasTemplateDecl>::const_iterator found =
      program.alias_templates.find(name);
  if(found == program.alias_templates.end()) {
    binding = BindingResult();
    binding.reason = "unknown_alias_template";
    return false;
  }

  if(!resolve_template_arguments(found->second.parameters, provided_arguments, binding, metrics)) {
    return false;
  }

  substitute_type(found->second.target, binding.bindings, expanded);
  return true;
}

void write_binding_result(std::ostream & out,
                          Query::TargetKind kind,
                          const std::string & name,
                          const std::vector<TemplateParam> & parameters,
                          const BindingResult & binding)
{
  out << "resolved " << target_kind_name(kind) << " "
      << format_instantiation(name, binding.resolved_arguments) << "\n";
  write_primary_bindings(out, parameters, binding.bindings);
  write_defaults(out, binding.applied_defaults, binding.bindings);
}

void write_select_class_result(const Program & program,
                               const Query & query,
                               int query_index,
                               Metrics & metrics,
                               std::ostream & out)
{
  ++metrics.class_select_queries;
  out << "query " << query_index << " select_class "
      << format_instantiation(query.target_name, query.arguments) << "\n";

  SelectionResult selection;
  if(!select_class_specialization(program, query.target_name, query.arguments, selection, metrics)) {
    out << "status error\n";
    out << "reason " << selection.reason << "\n";
    return;
  }

  const ClassTemplateDecl & primary = program.class_templates.find(query.target_name)->second;
  out << "status ok\n";
  if(selection.selection_kind == SelectionResult::SK_PRIMARY) {
    out << "selected primary_class " << query.target_name << "\n";
  } else if(selection.selection_kind == SelectionResult::SK_PARTIAL) {
    out << "selected partial_class " << selection.selected_id << "\n";
  } else {
    out << "selected explicit_class " << selection.selected_id << "\n";
  }
  write_binding_result(out,
                       Query::TK_CLASS,
                       query.target_name,
                       primary.parameters,
                       selection.primary_binding);
  if(selection.selection_kind == SelectionResult::SK_PARTIAL) {
    for(std::size_t i = 0; i < program.partial_classes.size(); ++i) {
      if(program.partial_classes[i].primary_name == query.target_name &&
         program.partial_classes[i].id == selection.selected_id) {
        write_specialization_bindings(out,
                                      program.partial_classes[i].parameters,
                                      selection.specialization_bindings);
        break;
      }
    }
  }
}

void write_select_variable_result(const Program & program,
                                  const Query & query,
                                  int query_index,
                                  Metrics & metrics,
                                  std::ostream & out)
{
  ++metrics.variable_select_queries;
  out << "query " << query_index << " select_variable "
      << format_instantiation(query.target_name, query.arguments) << "\n";

  SelectionResult selection;
  if(!select_variable_specialization(program, query.target_name, query.arguments, selection, metrics)) {
    out << "status error\n";
    out << "reason " << selection.reason << "\n";
    return;
  }

  const VariableTemplateDecl & primary = program.variable_templates.find(query.target_name)->second;
  out << "status ok\n";
  if(selection.selection_kind == SelectionResult::SK_PRIMARY) {
    out << "selected primary_variable " << query.target_name << "\n";
  } else if(selection.selection_kind == SelectionResult::SK_PARTIAL) {
    out << "selected partial_variable " << selection.selected_id << "\n";
  } else {
    out << "selected explicit_variable " << selection.selected_id << "\n";
  }
  write_binding_result(out,
                       Query::TK_VARIABLE,
                       query.target_name,
                       primary.parameters,
                       selection.primary_binding);
  if(selection.selection_kind == SelectionResult::SK_PARTIAL) {
    for(std::size_t i = 0; i < program.partial_variables.size(); ++i) {
      if(program.partial_variables[i].primary_name == query.target_name &&
         program.partial_variables[i].id == selection.selected_id) {
        write_specialization_bindings(out,
                                      program.partial_variables[i].parameters,
                                      selection.specialization_bindings);
        break;
      }
    }
  }
}

void write_expand_alias_result(const Program & program,
                               const Query & query,
                               int query_index,
                               Metrics & metrics,
                               std::ostream & out)
{
  ++metrics.alias_expand_queries;
  out << "query " << query_index << " expand_alias "
      << format_instantiation(query.target_name, query.arguments) << "\n";

  BindingResult binding;
  TypeExpr expanded;
  if(!expand_alias(program, query.target_name, query.arguments, binding, expanded, metrics)) {
    out << "status error\n";
    out << "reason " << binding.reason << "\n";
    return;
  }

  const AliasTemplateDecl & alias = program.alias_templates.find(query.target_name)->second;
  out << "status ok\n";
  write_binding_result(out,
                       Query::TK_ALIAS,
                       query.target_name,
                       alias.parameters,
                       binding);
  out << "expanded type " << format_type(expanded) << "\n";
}

void write_bind_defaults_result(const Program & program,
                                const Query & query,
                                int query_index,
                                Metrics & metrics,
                                std::ostream & out)
{
  ++metrics.default_binding_queries;
  out << "query " << query_index << " bind_defaults "
      << target_kind_name(query.target_kind) << " "
      << format_instantiation(query.target_name, query.arguments) << "\n";

  BindingResult binding;
  if(query.target_kind == Query::TK_CLASS) {
    const std::map<std::string, ClassTemplateDecl>::const_iterator found =
        program.class_templates.find(query.target_name);
    if(found == program.class_templates.end()) {
      out << "status error\nreason unknown_class_template\n";
      return;
    }
    if(!resolve_template_arguments(found->second.parameters, query.arguments, binding, metrics)) {
      out << "status error\nreason " << binding.reason << "\n";
      return;
    }
    out << "status ok\n";
    write_binding_result(out, Query::TK_CLASS, query.target_name, found->second.parameters, binding);
    return;
  }

  if(query.target_kind == Query::TK_VARIABLE) {
    const std::map<std::string, VariableTemplateDecl>::const_iterator found =
        program.variable_templates.find(query.target_name);
    if(found == program.variable_templates.end()) {
      out << "status error\nreason unknown_variable_template\n";
      return;
    }
    if(!resolve_template_arguments(found->second.parameters, query.arguments, binding, metrics)) {
      out << "status error\nreason " << binding.reason << "\n";
      return;
    }
    out << "status ok\n";
    write_binding_result(out, Query::TK_VARIABLE, query.target_name, found->second.parameters, binding);
    return;
  }

  const std::map<std::string, AliasTemplateDecl>::const_iterator found =
      program.alias_templates.find(query.target_name);
  if(found == program.alias_templates.end()) {
    out << "status error\nreason unknown_alias_template\n";
    return;
  }
  if(!resolve_template_arguments(found->second.parameters, query.arguments, binding, metrics)) {
    out << "status error\nreason " << binding.reason << "\n";
    return;
  }
  out << "status ok\n";
  write_binding_result(out, Query::TK_ALIAS, query.target_name, found->second.parameters, binding);
}

void write_deduce_call_result(const Program & program,
                              const Query & query,
                              int query_index,
                              Metrics & metrics,
                              std::ostream & out)
{
  ++metrics.function_deduction_queries;
  out << "query " << query_index << " deduce_call "
      << query.target_name << "(" << format_type_list(query.arguments) << ")\n";

  const std::map<std::string, std::vector<FunctionTemplateDecl> >::const_iterator found =
      program.function_templates.find(query.target_name);
  if(found == program.function_templates.end()) {
    out << "status error\n";
    out << "reason unknown_function_template\n";
    return;
  }

  std::vector<CandidateResult> candidates;
  for(std::size_t i = 0; i < found->second.size(); ++i) {
    ++metrics.candidate_evaluations;
    const FunctionTemplateDecl & decl = found->second[i];
    CandidateResult current;
    current.id = decl.id;
    current.specialization_score = pattern_specificity_score(decl.parameter_patterns,
                                                             decl.parameters);
    current.viable = match_template_pattern_arguments(decl.parameter_patterns,
                                                      decl.parameters,
                                                      query.arguments,
                                                      current.deduced,
                                                      current.applied_defaults,
                                                      metrics,
                                                      current.failure_reason);
    if(current.viable &&
       !evaluate_requirement(decl.requirement, current.deduced, current.failure_reason)) {
      current.viable = false;
    }
    if(current.viable &&
       !compute_conversion_penalty(decl,
                                   current.deduced,
                                   query.arguments,
                                   current.conversion_penalty,
                                   current.failure_reason)) {
      current.viable = false;
    }
    candidates.push_back(current);
  }

  int best_index = -1;
  bool ambiguous = false;
  for(std::size_t i = 0; i < candidates.size(); ++i) {
    if(!candidates[i].viable) {
      continue;
    }
    if(best_index < 0 ||
       candidates[i].conversion_penalty < candidates[best_index].conversion_penalty ||
       (candidates[i].conversion_penalty == candidates[best_index].conversion_penalty &&
        candidates[i].specialization_score > candidates[best_index].specialization_score)) {
      best_index = static_cast<int>(i);
      ambiguous = false;
    } else if(candidates[i].conversion_penalty ==
                 candidates[best_index].conversion_penalty &&
              candidates[i].specialization_score ==
                 candidates[best_index].specialization_score) {
      ambiguous = true;
    }
  }

  if(best_index >= 0 && !ambiguous) {
    out << "status ok\n";
    out << "selected function_template " << candidates[best_index].id << "\n";
    write_primary_bindings(out, found->second[best_index].parameters, candidates[best_index].deduced);
    write_defaults(out, candidates[best_index].applied_defaults, candidates[best_index].deduced);
    for(std::size_t i = 0; i < candidates.size(); ++i) {
      if(static_cast<int>(i) == best_index) {
        continue;
      }
      std::string drop_reason = candidates[i].failure_reason;
      if(candidates[i].viable) {
        drop_reason = candidates[i].conversion_penalty >
                              candidates[best_index].conversion_penalty ?
                          "worse_conversion_than_selected_candidate" :
                          "less_specialized_than_selected_candidate";
      }
      out << "drop " << candidates[i].id << " reason "
          << drop_reason
          << "\n";
    }
    return;
  }

  out << "status error\n";
  out << "reason " << (ambiguous ? "ambiguous_function_template" : "no_viable_function_template")
      << "\n";
  for(std::size_t i = 0; i < candidates.size(); ++i) {
    out << "drop " << candidates[i].id << " reason "
        << (candidates[i].viable ? "ambiguous_with_other_viable_candidate" :
                                   candidates[i].failure_reason)
        << "\n";
  }
}

void write_stats_result(int query_index, const Metrics & metrics, std::ostream & out)
{
  out << "query " << query_index << " stats\n";
  out << "status ok\n";
  out << "queries_executed " << metrics.queries_executed << "\n";
  out << "stats_queries " << metrics.stats_queries << "\n";
  out << "class_select_queries " << metrics.class_select_queries << "\n";
  out << "variable_select_queries " << metrics.variable_select_queries << "\n";
  out << "alias_expand_queries " << metrics.alias_expand_queries << "\n";
  out << "default_binding_queries " << metrics.default_binding_queries << "\n";
  out << "function_deduction_queries " << metrics.function_deduction_queries << "\n";
  out << "candidate_evaluations " << metrics.candidate_evaluations << "\n";
  out << "defaults_applied " << metrics.defaults_applied << "\n";
  out << "text_fallbacks " << metrics.text_fallbacks << "\n";
}

}  // namespace

bool run_file(const std::string & input_path,
              const std::string & output_path,
              std::string & error)
{
  Program program;
  if(!parse_program(input_path, program, error)) {
    return false;
  }

  std::ofstream output(output_path.c_str());
  if(!output) {
    error = "failed to open output file: " + output_path;
    return false;
  }

  Metrics metrics;
  output << "template_kernel_output_v1\n\n";
  for(std::size_t i = 0; i < program.queries.size(); ++i) {
    ++metrics.queries_executed;
    if(i != 0) {
      output << "\n";
    }

    if(program.queries[i].kind == Query::QK_SELECT_CLASS) {
      write_select_class_result(program, program.queries[i], static_cast<int>(i + 1), metrics, output);
    } else if(program.queries[i].kind == Query::QK_SELECT_VARIABLE) {
      write_select_variable_result(program, program.queries[i], static_cast<int>(i + 1), metrics, output);
    } else if(program.queries[i].kind == Query::QK_EXPAND_ALIAS) {
      write_expand_alias_result(program, program.queries[i], static_cast<int>(i + 1), metrics, output);
    } else if(program.queries[i].kind == Query::QK_BIND_DEFAULTS) {
      write_bind_defaults_result(program, program.queries[i], static_cast<int>(i + 1), metrics, output);
    } else if(program.queries[i].kind == Query::QK_DEDUCE_CALL) {
      write_deduce_call_result(program, program.queries[i], static_cast<int>(i + 1), metrics, output);
    } else {
      ++metrics.stats_queries;
      write_stats_result(static_cast<int>(i + 1), metrics, output);
    }
  }

  return true;
}

}  // namespace template_kernel
