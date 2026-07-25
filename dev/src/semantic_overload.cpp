#include "semantic_overload.h"

#include <algorithm>
#include <cctype>
#include <cstdlib>
#include <map>
#include <memory>
#include <set>
#include <sstream>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

#include "callsem_output.h"
#include "callsemantic_internal.h"
#include "class_template_mangle_info.h"
#include "constructor_lifecycle_service.h"
#include "cpp_decl_bridge.h"
#include "cppast_dump.h"
#include "parser_trace.h"
#include "semantic_builtins.h"
#include "semantic_class_model.h"
#include "semantic_conversion.h"
#include "semantic_consteval.h"
#include "semantic_dependent_type.h"
#include "semantic_errors.h"
#include "semantic_expression.h"
#include "semantic_hotspot.h"
#include "semantic_lifetime.h"
#include "semantic_lookup.h"
#include "semantic_metrics.h"
#include "semantic_output.h"
#include "semantic_scope_mutation.h"
#include "semantic_template_class.h"
#include "semantic_template_function.h"
#include "semantic_trace.h"
#include "semantic_utils.h"
#include "symbol_linkage.h"
#include "template_api.h"
#include "template_model.h"
#include "template_witness.h"

using namespace std;

namespace semantic_overload {

using namespace cpp_decl;
using namespace semantic_model;
using namespace semantic_conversion;
using namespace semantic_lookup;
using template_model::TemplateArgument;
using template_model::TemplateParameterInfo;
using template_model::find_template_parameter;

namespace {

typedef std::unordered_map<std::size_t, std::vector<FunctionBinding *> >
    FunctionCandidateBucketMap;

struct ScopedTemplateUseLocation
{
  explicit ScopedTemplateUseLocation(const std::string & location)
  {
    parser_trace::push_use_location(location);
  }

  ~ScopedTemplateUseLocation()
  {
    parser_trace::pop_use_location();
  }

  ScopedTemplateUseLocation(const ScopedTemplateUseLocation &) = delete;
  ScopedTemplateUseLocation & operator=(const ScopedTemplateUseLocation &) = delete;
};

struct ScopedSuppressedTemplateUseLocation
{
  ScopedSuppressedTemplateUseLocation()
  {
    parser_trace::push_use_location("\x1d");
  }

  ~ScopedSuppressedTemplateUseLocation()
  {
    parser_trace::pop_use_location();
  }

  ScopedSuppressedTemplateUseLocation(const ScopedSuppressedTemplateUseLocation &) = delete;
  ScopedSuppressedTemplateUseLocation & operator=(
      const ScopedSuppressedTemplateUseLocation &) = delete;
};

struct ParsedSourceLocation
{
  bool valid = false;
  std::string file;
  int line = 0;
  int column = 0;
};

ParsedSourceLocation parse_source_location(const std::string & text)
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

std::string trailing_identifier_token(const std::string & text)
{
  std::size_t end = text.size();
  while(end > 0 &&
        std::isspace(static_cast<unsigned char>(text[end - 1]))) {
    --end;
  }
  std::size_t begin = end;
  while(begin > 0) {
    const unsigned char ch = static_cast<unsigned char>(text[begin - 1]);
    if(!std::isalnum(ch) && ch != '_') {
      break;
    }
    --begin;
  }
  if(begin == end) {
    return std::string();
  }
  return text.substr(begin, end - begin);
}

bool source_location_points_at_identifier(SemanticContext & ctx,
                                          const std::string & location,
                                          const std::string & identifier)
{
  return template_api::template_witness_detail::
      source_location_points_at_identifier_token(
          ctx.template_witness_context(),
          location,
          identifier);
}

bool source_location_identifier_followed_by(SemanticContext & ctx,
                                            const std::string & location,
                                            const std::string & identifier,
                                            char ch)
{
  return template_api::template_witness_detail::
      source_location_identifier_token_followed_by(
          ctx.template_witness_context(),
          location,
          identifier,
          std::string(1, ch));
}

bool source_location_has_identifier_on_or_after(SemanticContext & ctx,
                                                const std::string & location,
                                                const std::string & identifier)
{
  return !template_api::template_witness_detail::
      source_location_for_identifier_token_on_or_after(
          ctx.template_witness_context(),
          location,
          identifier,
          true).empty();
}

std::string source_location_for_name_in_subtree(SemanticContext & ctx,
                                                const CppAstNode & node,
                                                const std::string & name,
                                                bool prefer_last)
{
  if(prefer_last) {
    for(size_t i = node.children.size(); i > 0; --i) {
      const std::string child_location =
          source_location_for_name_in_subtree(ctx, node.children[i - 1], name, true);
      if(!child_location.empty()) {
        return child_location;
      }
    }
    return ctx.source_location_for_name_in_node(node, name, true);
  }

  const std::string direct_location =
      ctx.source_location_for_name_in_node(node, name, false);
  if(!direct_location.empty()) {
    return direct_location;
  }
  for(size_t i = 0; i < node.children.size(); ++i) {
    const std::string child_location =
        source_location_for_name_in_subtree(ctx, node.children[i], name, false);
    if(!child_location.empty()) {
      return child_location;
    }
  }
  return std::string();
}

std::string call_expression_source_anchor_identifier(const CppAstNode & node)
{
  if(node.kind != CppAstKind::call_expression || node.children.empty()) {
    return std::string();
  }

  const CppAstNode * callee = &node.children[0];
  while(callee->kind == CppAstKind::parenthesized_expression &&
        callee->children.size() == 1) {
    callee = &callee->children[0];
  }

  if(const TemplateIdSyntax * template_id = cppast_template_id_syntax(*callee)) {
    const std::string unqualified =
        semantic_utils::unqualified_member_name(template_id->name.name);
    if(!unqualified.empty()) {
      return unqualified;
    }
  }

  if((callee->kind == CppAstKind::id_expression ||
      callee->kind == CppAstKind::member_expression) &&
     !callee->value.empty()) {
    const std::string unqualified =
        semantic_utils::trim_space(
            semantic_utils::unqualified_member_name(
                semantic_utils::strip_trailing_top_level_template_arguments(
                    callee->value)));
    if(unqualified.compare(0, 8, "operator") == 0) {
      return "operator";
    }
    return trailing_identifier_token(unqualified);
  }

  return std::string();
}

std::string refine_fragment_use_location(SemanticContext & ctx,
                                         const CppAstNode & node,
                                         const std::string & base_location)
{
  if(base_location.empty()) {
    return std::string();
  }
  if(!witness::source_capture_enabled(ctx.template_witness_context())) {
    return base_location;
  }

  const std::string anchor_identifier =
      call_expression_source_anchor_identifier(node);
  if(anchor_identifier.empty()) {
    return base_location;
  }

  const std::string token_location =
      template_api::template_witness_detail::
          source_location_for_identifier_token_on_or_after(
              ctx.template_witness_context(),
              base_location,
              anchor_identifier,
              true);
  if(!token_location.empty()) {
    return token_location;
  }
  return base_location;
}

std::string prefer_later_source_location(const std::string & first,
                                         const std::string & second)
{
  if(first.empty()) {
    return second;
  }
  if(second.empty()) {
    return first;
  }
  const ParsedSourceLocation parsed_first = parse_source_location(first);
  const ParsedSourceLocation parsed_second = parse_source_location(second);
  if(!parsed_first.valid || !parsed_second.valid ||
     parsed_first.file != parsed_second.file) {
    return first;
  }
  if(parsed_second.line > parsed_first.line) {
    return second;
  }
  if(parsed_second.line == parsed_first.line &&
     parsed_second.column > parsed_first.column) {
    return second;
  }
  return first;
}

std::string normalize_template_witness_location(const std::string & location);

bool source_location_is_strictly_later_in_same_file(const std::string & first,
                                                    const std::string & second)
{
  const ParsedSourceLocation parsed_first =
      parse_source_location(normalize_template_witness_location(first));
  const ParsedSourceLocation parsed_second =
      parse_source_location(normalize_template_witness_location(second));
  if(!parsed_first.valid || !parsed_second.valid ||
     parsed_first.file != parsed_second.file) {
    return false;
  }
  if(parsed_first.line > parsed_second.line) {
    return true;
  }
  return parsed_first.line == parsed_second.line &&
         parsed_first.column > parsed_second.column;
}

bool source_location_in_template_body_range(SemanticContext & ctx,
                                            const std::string & location)
{
  const ParsedSourceLocation parsed =
      parse_source_location(normalize_template_witness_location(location));
  if(!parsed.valid || ctx.template_witness_context().session == nullptr) {
    return false;
  }
  const std::vector<template_api::TemplateWitnessSourceRange> & ranges =
      ctx.template_witness_context().session->template_body_ranges;
  for(std::size_t i = 0; i < ranges.size(); ++i) {
    const ParsedSourceLocation range_file =
        parse_source_location(normalize_template_witness_location(
            ranges[i].file + ":1:1"));
    const std::string normalized_range_file =
        range_file.valid ? range_file.file : ranges[i].file;
    if(normalized_range_file != parsed.file) {
      continue;
    }
    if(parsed.line < ranges[i].begin_line || parsed.line > ranges[i].end_line) {
      continue;
    }
    if(parsed.line == ranges[i].begin_line &&
       ranges[i].first_body_column > 1 &&
       parsed.column > 0 &&
       parsed.column < ranges[i].first_body_column) {
      continue;
    }
    return true;
  }
  return false;
}

std::string coarse_assignment_drop_reason(const std::string & rejection)
{
  if(rejection == "implicit object conversion failed" ||
     rejection == "argument conversion failed") {
    return "bad_conversion";
  }
  if(rejection == "argument analysis failed") {
    return "substitution_failure";
  }
  if(rejection == "member access not allowed") {
    return "access_denied";
  }
  return "not_viable";
}

std::string template_use_or_fallback_location(const std::string & fallback)
{
  return fallback;
}

bool same_function_candidate_entity(FunctionBinding * lhs, FunctionBinding * rhs);

semantic_metrics::AnalyzerCounters * performance_counters(SemanticContext & ctx)
{
  return ctx.performance_counters();
}

void note_overload_candidate_set(SemanticContext & ctx)
{
  if(semantic_metrics::AnalyzerCounters * counters = performance_counters(ctx)) {
    ++counters->overload_candidate_sets;
  }
}

void note_overload_candidate_attempt(SemanticContext & ctx)
{
  if(semantic_metrics::AnalyzerCounters * counters = performance_counters(ctx)) {
    ++counters->overload_candidate_attempts;
  }
}

void note_overload_viable_candidate(SemanticContext & ctx)
{
  if(semantic_metrics::AnalyzerCounters * counters = performance_counters(ctx)) {
    ++counters->overload_viable_candidates;
  }
}

void note_overload_candidate_refresh(SemanticContext & ctx, bool succeeded)
{
  if(semantic_metrics::AnalyzerCounters * counters = performance_counters(ctx)) {
    ++counters->overload_candidate_refresh_attempts;
    if(succeeded) {
      ++counters->overload_candidate_refresh_successes;
    }
  }
}

ClassInfo * complete_class_type_for_lookup(SemanticContext & ctx,
                                           const TypePtr & type)
{
  TypePtr base = strip_top_level_cv(type);
  if(!base) {
    return nullptr;
  }
  ClassInfo * info = ctx.class_info_for_type(base);
  if(info && info->class_kind == "enum") {
    return nullptr;
  }
  if(info && info->complete) {
    return info;
  }
  return info ? ctx.complete_class_type(base) : nullptr;
}

bool constructor_template_has_trailing_parameter_pack_fast(FunctionTemplateDecl & decl);
bool function_template_has_trailing_parameter_pack_fast(FunctionTemplateDecl & decl);
bool is_forwarding_reference_pattern(const vector<TemplateParameterInfo> & parameters,
                                     const TypePtr & pattern);
std::string candidate_primary_location(SemanticContext & ctx,
                                       FunctionBinding * binding);

bool template_witness_source_capture_enabled_for_calls(SemanticContext & ctx)
{
  return witness::function_call_source_capture_enabled() &&
         witness::source_capture_enabled(ctx.template_witness_context());
}

std::string normalize_template_witness_location(const std::string & location)
{
  return template_api::normalize_template_witness_source_location(location);
}

struct SourceTokenRef
{
  std::size_t index = 0;
  std::string source;
  uint32_t location_id = 0;
  int column = 0;
  bool identifier = false;
  bool close_angle = false;
};

std::string source_location_for_token_id(
    const template_api::TemplateWitnessContext & witness_ctx,
    uint32_t location_id)
{
  if(!(witness_ctx.source_locations && location_id != 0)) {
    return std::string();
  }
  return normalize_template_witness_location(
      std::string(" at ") +
      template_api::template_witness_detail::source_location_for_location_id_raw(
          *witness_ctx.source_locations,
          location_id));
}

std::vector<SourceTokenRef> same_line_source_tokens(
    SemanticContext & ctx,
    const ParsedSourceLocation & base)
{
  std::vector<SourceTokenRef> out;
  const template_api::TemplateWitnessContext witness_ctx =
      ctx.template_witness_context();
  if(!base.valid ||
     !(witness_ctx.token_sequence && witness_ctx.source_locations)) {
    return out;
  }
  const std::size_t token_count = witness_ctx.token_sequence->size();
  for(std::size_t i = 0; i < token_count; ++i) {
    const RecogToken & token = witness_ctx.token_sequence->peek(i);
    if(token.is_eof()) {
      break;
    }
    if(token.location_id == 0 ||
       token.location_id >= witness_ctx.source_locations->locations.size()) {
      continue;
    }
    const SourceLocation & location =
        witness_ctx.source_locations->locations[token.location_id];
    if(location.file_index >= witness_ctx.source_locations->files.size() ||
       witness_ctx.source_locations->files[location.file_index] != base.file ||
       static_cast<int>(location.line) != base.line) {
      continue;
    }
    SourceTokenRef ref;
    ref.index = i;
    ref.source = token.source;
    ref.location_id = token.location_id;
    ref.column = static_cast<int>(location.column);
    ref.identifier = token.is_identifier();
    ref.close_angle = token.is_close_angle_bracket();
    out.push_back(ref);
  }
  return out;
}

std::string overloaded_operator_token(const std::string & selected_name)
{
  static const std::pair<const char *, const char *> operators[] = {
      {"operator>>", ">>"},
      {"operator<<", "<<"},
      {"operator<=", "<="},
      {"operator>=", ">="},
      {"operator==", "=="},
      {"operator!=", "!="},
      {"operator+=", "+="},
      {"operator-=", "-="},
      {"operator*=", "*="},
      {"operator/=", "/="},
      {"operator%=", "%="},
      {"operator&=", "&="},
      {"operator|=", "|="},
      {"operator^=", "^="},
      {"operator+", "+"},
      {"operator-", "-"},
      {"operator*", "*"},
      {"operator/", "/"},
      {"operator%", "%"},
      {"operator<", "<"},
      {"operator>", ">"},
      {"operator=", "="},
      {"operator&", "&"},
      {"operator|", "|"},
      {"operator^", "^"},
      {"operator!", "!"},
      {"operator~", "~"},
      {"operator,", ","}};
  for(std::size_t i = 0; i < sizeof(operators) / sizeof(operators[0]); ++i) {
    const std::string suffix = operators[i].first;
    if(selected_name.size() >= suffix.size() &&
       selected_name.compare(selected_name.size() - suffix.size(),
                             suffix.size(),
                             suffix) == 0) {
      return operators[i].second;
    }
  }
  return std::string();
}

std::string refine_operator_call_source_location(SemanticContext & ctx,
                                                 const std::string & selected_name,
                                                 const std::string & base_location)
{
  const std::string token_text = overloaded_operator_token(selected_name);
  if(token_text.empty()) {
    return base_location;
  }
  const ParsedSourceLocation base =
      parse_source_location(normalize_template_witness_location(base_location));
  if(!base.valid) {
    return base_location;
  }
  const template_api::TemplateWitnessContext witness_ctx =
      ctx.template_witness_context();
  const std::vector<SourceTokenRef> tokens = same_line_source_tokens(ctx, base);
  int chosen_before = -1;
  int first_after = -1;
  for(std::size_t i = 0; i < tokens.size(); ++i) {
    if(tokens[i].source != token_text) {
      continue;
    }
    if(tokens[i].column <= base.column) {
      chosen_before = static_cast<int>(i);
    } else if(first_after < 0) {
      first_after = static_cast<int>(i);
    }
  }
  const int chosen =
      chosen_before >= 0 ? chosen_before :
      (first_after >= 0 ? first_after : -1);
  if(chosen < 0) {
    return base_location;
  }
  const std::string refined =
      source_location_for_token_id(witness_ctx,
                                   tokens[static_cast<std::size_t>(chosen)].location_id);
  return refined.empty() ? base_location : refined;
}

std::string constructor_source_class_name(const std::string & selected_name,
                                          const std::string & template_name)
{
  const std::size_t split = selected_name.rfind("::");
  if(split != std::string::npos) {
    const std::string owner = selected_name.substr(0, split);
    const std::string member =
        semantic_utils::strip_trailing_top_level_template_arguments(
            selected_name.substr(split + 2));
    const std::string owner_class =
        semantic_utils::unqualified_member_name(
            semantic_utils::strip_trailing_top_level_template_arguments(owner));
    if(!member.empty() && member == owner_class) {
      return member;
    }
  }
  return semantic_utils::unqualified_member_name(
      semantic_utils::strip_trailing_top_level_template_arguments(template_name));
}

std::string refine_constructor_call_source_location(
    SemanticContext & ctx,
    const std::string & selected_name,
    const std::string & template_name,
    const std::string & base_location,
    bool * matched_source_syntax = nullptr)
{
  if(matched_source_syntax) {
    *matched_source_syntax = false;
  }
  const std::string class_name =
      constructor_source_class_name(selected_name, template_name);
  if(class_name.empty()) {
    return base_location;
  }
  const ParsedSourceLocation base =
      parse_source_location(normalize_template_witness_location(base_location));
  if(!base.valid) {
    return base_location;
  }
  const template_api::TemplateWitnessContext witness_ctx =
      ctx.template_witness_context();
  const std::vector<SourceTokenRef> tokens = same_line_source_tokens(ctx, base);
  if(tokens.empty()) {
    return base_location;
  }

  int paren = -1;
  int forward_angle_depth = 0;
  for(std::size_t i = 0; i < tokens.size(); ++i) {
    if(tokens[i].source == "<") {
      ++forward_angle_depth;
      continue;
    }
    if(tokens[i].close_angle && forward_angle_depth > 0) {
      --forward_angle_depth;
      continue;
    }
    if(forward_angle_depth != 0) {
      continue;
    }
    if(tokens[i].source != "(") {
      continue;
    }
    if(paren < 0) {
      paren = static_cast<int>(i);
    }
    if(tokens[i].column >= base.column) {
      paren = static_cast<int>(i);
      break;
    }
    paren = static_cast<int>(i);
  }
  if(paren < 0) {
    return base_location;
  }

  const int before_paren = paren - 1;
  if(before_paren >= 0 && tokens[static_cast<std::size_t>(before_paren)].identifier) {
    const SourceTokenRef & before =
        tokens[static_cast<std::size_t>(before_paren)];
    if(before.source == class_name) {
      const std::string refined =
          source_location_for_token_id(witness_ctx, before.location_id);
      if(matched_source_syntax && !refined.empty()) {
        *matched_source_syntax = true;
      }
      return refined.empty() ? base_location : refined;
    }
    bool have_type_name_before_identifier = false;
    for(int i = before_paren - 1; i >= 0; --i) {
      const SourceTokenRef & candidate = tokens[static_cast<std::size_t>(i)];
      if(candidate.identifier && candidate.source == class_name) {
        have_type_name_before_identifier = true;
        break;
      }
    }
    if(have_type_name_before_identifier) {
      const std::string refined =
          source_location_for_token_id(witness_ctx, before.location_id);
      if(matched_source_syntax && !refined.empty()) {
        *matched_source_syntax = true;
      }
      return refined.empty() ? base_location : refined;
    }
  }

  int angle_depth = 0;
  for(int i = before_paren; i >= 0; --i) {
    const SourceTokenRef & candidate = tokens[static_cast<std::size_t>(i)];
    if(candidate.close_angle) {
      ++angle_depth;
      continue;
    }
    if(candidate.source == "<" && angle_depth > 0) {
      --angle_depth;
      continue;
    }
    if(candidate.identifier && candidate.source == class_name) {
      const std::string refined =
          source_location_for_token_id(witness_ctx, candidate.location_id);
      if(matched_source_syntax && !refined.empty()) {
        *matched_source_syntax = true;
      }
      return refined.empty() ? base_location : refined;
    }
  }
  return base_location;
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

std::string prefer_later_expr_source_location(const std::string & location,
                                              const std::vector<ExprInfo> & args)
{
  std::string best = location;
  for(std::size_t i = 0; i < args.size(); ++i) {
    best = prefer_later_source_location(best,
                                        callsem_node_source_location_text(args[i].node));
  }
  return best;
}

std::string constructor_selection_use_location(
    const ConstructorSelectionOptions & options)
{
  return !options.use_location.empty() ? options.use_location :
                                         parser_trace::current_use_location();
}

std::string constructor_witness_source_location(
    const ConstructorSelectionOptions & options,
    const std::string & fallback,
    const std::vector<ExprInfo> & args)
{
  if(!options.source_witness_location.empty()) {
    return options.source_witness_location;
  }
  const std::string configured = constructor_selection_use_location(options);
  if(!configured.empty()) {
    return configured;
  }
  return prefer_later_expr_source_location(fallback, args);
}

bool constructor_selection_is_speculative_user_defined_conversion_probe(
    const ConstructorSelectionOptions & options)
{
  return options.context &&
         std::string(options.context) == "user-defined conversion constructor" &&
         !options.instantiate_bodies;
}

const CppAstNode * default_argument_payload(const CppAstNode * default_arg)
{
  if(!default_arg) {
    return nullptr;
  }
  if(default_arg->kind != CppAstKind::default_argument) {
    return default_arg;
  }
  if(default_arg->children.size() != 1) {
    return nullptr;
  }
  return &default_arg->children[0];
}

std::size_t function_template_required_argument_count_fast(const FunctionTemplateDecl & decl)
{
  std::size_t required_count = decl.params_pattern.size();
  const bool has_trailing_pack =
      decl.has_trailing_function_parameter_pack ||
      function_template_has_trailing_parameter_pack_fast(
          const_cast<FunctionTemplateDecl &>(decl));
  if(has_trailing_pack && required_count > 0) {
    --required_count;
  }
  while(required_count > 0 &&
        required_count - 1 < decl.default_arguments_pattern.size() &&
        decl.default_arguments_pattern[required_count - 1]) {
    --required_count;
  }
  return required_count;
}

std::string function_template_argument_count_drop_reason(const FunctionTemplateDecl & decl,
                                                         std::size_t argument_count)
{
  const std::size_t required_count = function_template_required_argument_count_fast(decl);
  if(argument_count < required_count) {
    return "too_few_arguments";
  }
  TypePtr function_type = strip_top_level_cv(decl.type_pattern);
  if(function_type &&
     function_type->kind == Type::TK_FUNCTION &&
     (function_type->variadic || function_type->prototype_relaxed)) {
    return std::string();
  }
  const bool has_trailing_pack =
      decl.has_trailing_function_parameter_pack ||
      function_template_has_trailing_parameter_pack_fast(
          const_cast<FunctionTemplateDecl &>(decl));
  if(has_trailing_pack &&
     argument_count + 1 >= decl.params_pattern.size()) {
    return std::string();
  }
  return argument_count > decl.params_pattern.size() ? "too_many_arguments" :
                                                       "substitution_failure";
}

string named_template_base_name(const TypePtr & type);
bool template_base_names_compatible(const string & pattern,
                                    const string & actual);

std::string constructor_template_argument_count_drop_reason(const FunctionTemplateDecl & decl,
                                                            std::size_t argument_count)
{
  const bool has_trailing_pack =
      constructor_template_has_trailing_parameter_pack_fast(
          const_cast<FunctionTemplateDecl &>(decl));
  TypePtr function_type = strip_top_level_cv(decl.type_pattern);
  if(function_type &&
     function_type->kind == Type::TK_FUNCTION &&
     (function_type->variadic || function_type->prototype_relaxed) &&
     argument_count >= decl.params_pattern.size()) {
    return std::string();
  }
  if(has_trailing_pack &&
     argument_count + 1 >= decl.params_pattern.size()) {
    return std::string();
  }
  return argument_count > decl.params_pattern.size() ? "too_many_arguments" :
                                                       "too_few_arguments";
}

bool function_template_specialization_retained_dependent_parameter(
    SemanticContext & ctx,
    const FunctionBinding & binding,
    const TypePtr & function_type)
{
  if(!binding.source_template ||
     (!binding.has_instantiation_arguments &&
      binding.instantiation_arguments.empty()) ||
     template_api::function_binding_instantiation_arguments_dependent(ctx, binding)) {
    return false;
  }
  if(binding.owner_class &&
     (binding.owner_class->dependent_instantiation ||
      ctx.type_depends_on_template_parameter(binding.owner_class->type))) {
    return false;
  }

  TypePtr base = strip_top_level_cv(function_type);
  const std::size_t offset = function_binding_explicit_parameter_offset(binding);
  if(base && base->kind == Type::TK_FUNCTION) {
    for(std::size_t i = offset; i < base->params.size(); ++i) {
      if(ctx.type_depends_on_template_parameter(base->params[i])) {
        return true;
      }
    }
  }
  for(std::size_t i = offset; i < binding.params.size(); ++i) {
    if(ctx.type_depends_on_template_parameter(binding.params[i].second)) {
      return true;
    }
  }
  return false;
}

TypePtr collapse_lvalue_reference_type_for_deduction_reason(const TypePtr & inner)
{
  if(!inner) {
    return inner;
  }
  TypePtr base = strip_top_level_cv(inner);
  if(base && base->kind == Type::TK_LVALUE_REFERENCE) {
    return make_lvalue_reference_raw(base->inner);
  }
  if(base && base->kind == Type::TK_RVALUE_REFERENCE) {
    return make_lvalue_reference_raw(base->inner);
  }
  return make_lvalue_reference_raw(inner);
}

const TemplateParameterInfo * direct_type_template_parameter_for_reason(
    const std::vector<TemplateParameterInfo> & parameters,
    const TypePtr & type)
{
  TypePtr base = strip_top_level_cv(type);
  if(!base || base->kind != Type::TK_NAMED) {
    return nullptr;
  }

  const TemplateParameterInfo * parameter =
      find_template_parameter(parameters, base);
  return parameter && parameter->kind == TemplateParameterInfo::TP_TYPE ?
             parameter :
             nullptr;
}

bool record_reason_deduction(std::map<std::string, TypePtr> & deduced,
                             const TemplateParameterInfo & parameter,
                             const TypePtr & actual,
                             bool & inconsistent)
{
  if(parameter.name.empty() || !actual) {
    return false;
  }
  auto found = deduced.find(parameter.name);
  if(found == deduced.end()) {
    deduced[parameter.name] = actual;
    return true;
  }
  if(!type_equals(found->second, actual)) {
    inconsistent = true;
  }
  return true;
}

bool collect_template_deduction_reason_deductions(
    const std::vector<TemplateParameterInfo> & parameters,
    const TypePtr & pattern,
    const TypePtr & actual,
    std::map<std::string, TypePtr> & deduced,
    bool & inconsistent)
{
  if(!pattern || !actual) {
    return false;
  }

  TypePtr pattern_base = strip_top_level_cv(pattern);
  TypePtr actual_base = strip_top_level_cv(actual);
  if(!pattern_base || !actual_base) {
    return false;
  }

  const TemplateParameterInfo * direct =
      direct_type_template_parameter_for_reason(parameters, pattern_base);
  if(direct) {
    return record_reason_deduction(deduced, *direct, actual, inconsistent);
  }

  if(pattern->kind == Type::TK_CV) {
    TypePtr actual_inner;
    bool actual_const = false;
    bool actual_volatile = false;
    if(top_level_cv_flags(actual, actual_inner, actual_const, actual_volatile) &&
       ((!pattern->cv_const || actual_const) &&
        (!pattern->cv_volatile || actual_volatile))) {
      return collect_template_deduction_reason_deductions(parameters,
                                                          pattern->inner,
                                                          actual_inner,
                                                          deduced,
                                                          inconsistent);
    }
    return collect_template_deduction_reason_deductions(parameters,
                                                        pattern->inner,
                                                        actual_base,
                                                        deduced,
                                                        inconsistent);
  }

  if(actual->kind == Type::TK_CV && pattern->kind != Type::TK_POINTER &&
     pattern->kind != Type::TK_BLOCK_POINTER &&
     pattern->kind != Type::TK_MEMBER_POINTER) {
    return collect_template_deduction_reason_deductions(parameters,
                                                        pattern,
                                                        actual->inner,
                                                        deduced,
                                                        inconsistent);
  }

  if(pattern_base->kind == Type::TK_LVALUE_REFERENCE ||
     pattern_base->kind == Type::TK_RVALUE_REFERENCE) {
    TypePtr actual_inner = actual_base;
    if(actual_base->kind == Type::TK_LVALUE_REFERENCE ||
       actual_base->kind == Type::TK_RVALUE_REFERENCE) {
      actual_inner = actual_base->inner;
    }
    return collect_template_deduction_reason_deductions(parameters,
                                                        pattern_base->inner,
                                                        actual_inner,
                                                        deduced,
                                                        inconsistent);
  }

  if(pattern_base->kind != actual_base->kind) {
    return false;
  }

  switch(pattern_base->kind) {
  case Type::TK_POINTER:
  case Type::TK_BLOCK_POINTER:
  case Type::TK_ARRAY:
  case Type::TK_ATOMIC:
    return collect_template_deduction_reason_deductions(parameters,
                                                        pattern_base->inner,
                                                        actual_base->inner,
                                                        deduced,
                                                        inconsistent);

  case Type::TK_MEMBER_POINTER:
  {
    const bool owner_ok = collect_template_deduction_reason_deductions(parameters,
                                                                       pattern_base->owner,
                                                                       actual_base->owner,
                                                                       deduced,
                                                                       inconsistent);
    const bool inner_ok = collect_template_deduction_reason_deductions(parameters,
                                                                       pattern_base->inner,
                                                                       actual_base->inner,
                                                                       deduced,
                                                                       inconsistent);
    return owner_ok || inner_ok;
  }

  case Type::TK_FUNCTION:
  {
    bool any = collect_template_deduction_reason_deductions(parameters,
                                                           pattern_base->inner,
                                                           actual_base->inner,
                                                           deduced,
                                                           inconsistent);
    const std::size_t count = std::min(pattern_base->params.size(),
                                       actual_base->params.size());
    for(std::size_t i = 0; i < count; ++i) {
      any = collect_template_deduction_reason_deductions(parameters,
                                                         pattern_base->params[i],
                                                         actual_base->params[i],
                                                         deduced,
                                                         inconsistent) ||
            any;
    }
    return any;
  }

  case Type::TK_FUNDAMENTAL:
  case Type::TK_NAMED:
  case Type::TK_CV:
  case Type::TK_LVALUE_REFERENCE:
  case Type::TK_RVALUE_REFERENCE:
    return false;
  }

  return false;
}

bool type_contains_deducible_type_template_parameter(
    const std::vector<TemplateParameterInfo> & parameters,
    const TypePtr & type)
{
  TypePtr base = strip_top_level_cv(type);
  if(!base) {
    return false;
  }

  if(direct_type_template_parameter_for_reason(parameters, base)) {
    return true;
  }

  switch(base->kind) {
  case Type::TK_POINTER:
  case Type::TK_BLOCK_POINTER:
  case Type::TK_LVALUE_REFERENCE:
  case Type::TK_RVALUE_REFERENCE:
  case Type::TK_ARRAY:
  case Type::TK_ATOMIC:
  case Type::TK_CV:
    return type_contains_deducible_type_template_parameter(parameters, base->inner);

  case Type::TK_MEMBER_POINTER:
    return type_contains_deducible_type_template_parameter(parameters, base->owner) ||
           type_contains_deducible_type_template_parameter(parameters, base->inner);

  case Type::TK_FUNCTION:
    if(type_contains_deducible_type_template_parameter(parameters, base->inner)) {
      return true;
    }
    for(std::size_t i = 0; i < base->params.size(); ++i) {
      if(type_contains_deducible_type_template_parameter(parameters,
                                                         base->params[i])) {
        return true;
      }
    }
    return false;

  case Type::TK_FUNDAMENTAL:
  case Type::TK_NAMED:
    return false;
  }

  return false;
}

bool template_deduction_reason_has_shape_mismatch(
    const std::vector<TemplateParameterInfo> & parameters,
    const TypePtr & pattern,
    const TypePtr & actual)
{
  TypePtr pattern_base = strip_top_level_cv(pattern);
  TypePtr actual_base = strip_top_level_cv(actual);
  if(!pattern_base || !actual_base) {
    return false;
  }

  if(direct_type_template_parameter_for_reason(parameters, pattern_base)) {
    return false;
  }

  if(pattern_base->kind != actual_base->kind) {
    return type_contains_deducible_type_template_parameter(parameters,
                                                          pattern_base);
  }

  switch(pattern_base->kind) {
  case Type::TK_POINTER:
  case Type::TK_BLOCK_POINTER:
  case Type::TK_LVALUE_REFERENCE:
  case Type::TK_RVALUE_REFERENCE:
  case Type::TK_ARRAY:
  case Type::TK_ATOMIC:
  case Type::TK_CV:
    return template_deduction_reason_has_shape_mismatch(parameters,
                                                        pattern_base->inner,
                                                        actual_base->inner);

  case Type::TK_MEMBER_POINTER:
    return template_deduction_reason_has_shape_mismatch(parameters,
                                                        pattern_base->owner,
                                                        actual_base->owner) ||
           template_deduction_reason_has_shape_mismatch(parameters,
                                                        pattern_base->inner,
                                                        actual_base->inner);

  case Type::TK_FUNCTION:
  {
    if(pattern_base->params.size() != actual_base->params.size() &&
       type_contains_deducible_type_template_parameter(parameters,
                                                       pattern_base)) {
      return true;
    }
    if(template_deduction_reason_has_shape_mismatch(parameters,
                                                    pattern_base->inner,
                                                    actual_base->inner)) {
      return true;
    }
    const std::size_t count = std::min(pattern_base->params.size(),
                                       actual_base->params.size());
    for(std::size_t i = 0; i < count; ++i) {
      if(template_deduction_reason_has_shape_mismatch(parameters,
                                                      pattern_base->params[i],
                                                      actual_base->params[i])) {
        return true;
      }
    }
    return false;
  }

  case Type::TK_FUNDAMENTAL:
  case Type::TK_NAMED:
    return false;
  }

  return false;
}

void adjusted_template_deduction_reason_pair(FunctionTemplateDecl & decl,
                                             const ExprInfo & arg,
                                             std::size_t parameter_index,
                                             TypePtr & pattern,
                                             TypePtr & actual)
{
  pattern = parameter_index < decl.params_pattern.size() ?
                decl.params_pattern[parameter_index].second :
                TypePtr();
  actual = arg.type ? remove_reference_type(arg.type) : TypePtr();
  if(!actual) {
    actual = arg.type;
  }

  TypePtr pattern_base = strip_top_level_cv(pattern);
  if(pattern_base &&
     pattern_base->kind == Type::TK_RVALUE_REFERENCE &&
     is_forwarding_reference_pattern(decl.parameters, pattern) &&
     arg.category == VC_LVALUE) {
    pattern = pattern_base->inner;
    actual = collapse_lvalue_reference_type_for_deduction_reason(actual);
  } else if(pattern_base &&
            (pattern_base->kind == Type::TK_LVALUE_REFERENCE ||
             pattern_base->kind == Type::TK_RVALUE_REFERENCE)) {
    pattern = pattern_base->inner;
  } else {
    if(pattern) {
      pattern = normalize_parameter_type(pattern);
    }
    if(actual) {
      actual = normalize_parameter_type(actual);
    }
  }

  TypePtr pattern_cv_inner;
  TypePtr actual_cv_inner;
  bool pattern_const = false;
  bool pattern_volatile = false;
  bool actual_const = false;
  bool actual_volatile = false;
  if(top_level_cv_flags(pattern, pattern_cv_inner, pattern_const, pattern_volatile) &&
     top_level_cv_flags(actual, actual_cv_inner, actual_const, actual_volatile) &&
     ((pattern_const && actual_const) ||
      (pattern_volatile && actual_volatile))) {
    pattern = pattern_cv_inner;
    actual = actual_cv_inner;
  }
}

bool template_deduction_reason_is_inconsistent(FunctionTemplateDecl & decl,
                                               const std::vector<ExprInfo> & args)
{
  std::map<std::string, TypePtr> deduced;
  bool inconsistent = false;
  const std::size_t count = std::min(args.size(), decl.params_pattern.size());
  for(std::size_t i = 0; i < count; ++i) {
    TypePtr pattern;
    TypePtr actual;
    adjusted_template_deduction_reason_pair(decl, args[i], i, pattern, actual);
    collect_template_deduction_reason_deductions(decl.parameters,
                                                 pattern,
                                                 actual,
                                                 deduced,
                                                 inconsistent);
    if(inconsistent) {
      return true;
    }
  }
  return false;
}

bool template_argument_text_mentions_deduction_parameter(
    const std::vector<TemplateParameterInfo> & parameters,
    const std::string & text)
{
  for(std::size_t i = 0; i < parameters.size(); ++i) {
    if(!parameters[i].name.empty() &&
       callsemantic_internal::contains_identifier_token(text, parameters[i].name)) {
      return true;
    }
  }
  return false;
}

bool template_argument_mentions_deduction_parameter(
    SemanticContext & ctx,
    const std::vector<TemplateParameterInfo> & parameters,
    const TemplateArgument & argument);

bool type_mentions_deduction_parameter(
    SemanticContext & ctx,
    const std::vector<TemplateParameterInfo> & parameters,
    const TypePtr & type)
{
  if(!type) {
    return false;
  }
  TypePtr base = strip_top_level_cv(type);
  if(!base) {
    return false;
  }

  switch(base->kind) {
  case Type::TK_NAMED:
  {
    if(find_template_parameter(parameters, base)) {
      return true;
    }
    ClassInfo * info = ctx.class_info_for_type(base);
    if(info) {
      for(std::size_t i = 0; i < info->instantiation_arguments.size(); ++i) {
        if(template_argument_mentions_deduction_parameter(
               ctx, parameters, info->instantiation_arguments[i])) {
          return true;
        }
      }
    }
    if(std::shared_ptr<const cpp_decl::ClassTemplateSpecializationMangleInfo>
           mangle_info =
               cpp_decl::named_type_class_template_specialization_mangle_info_const(
                   base)) {
      for(std::size_t i = 0; i < mangle_info->arguments.size(); ++i) {
        if(template_argument_mentions_deduction_parameter(
               ctx, parameters, mangle_info->arguments[i])) {
          return true;
        }
      }
    }
    return false;
  }

  case Type::TK_CV:
  case Type::TK_ATOMIC:
  case Type::TK_POINTER:
  case Type::TK_BLOCK_POINTER:
  case Type::TK_LVALUE_REFERENCE:
  case Type::TK_RVALUE_REFERENCE:
  case Type::TK_ARRAY:
    return type_mentions_deduction_parameter(ctx, parameters, base->inner);

  case Type::TK_MEMBER_POINTER:
    return type_mentions_deduction_parameter(ctx, parameters, base->owner) ||
           type_mentions_deduction_parameter(ctx, parameters, base->inner);

  case Type::TK_FUNCTION:
    if(type_mentions_deduction_parameter(ctx, parameters, base->inner)) {
      return true;
    }
    for(std::size_t i = 0; i < base->params.size(); ++i) {
      if(type_mentions_deduction_parameter(ctx, parameters, base->params[i])) {
        return true;
      }
    }
    return false;

  default:
    return false;
  }
}

bool template_argument_mentions_deduction_parameter(
    SemanticContext & ctx,
    const std::vector<TemplateParameterInfo> & parameters,
    const TemplateArgument & argument)
{
  if(argument.kind == TemplateArgument::TA_TYPE &&
     argument.type &&
     type_mentions_deduction_parameter(ctx, parameters, argument.type)) {
    return true;
  }
  return !argument.text.empty() &&
         template_argument_text_mentions_deduction_parameter(
             parameters, argument.text);
}

bool template_deduction_reason_has_nondeduced_mismatch(SemanticContext & ctx,
                                                       FunctionTemplateDecl & decl,
                                                       const std::vector<ExprInfo> & args)
{
  const std::size_t count = std::min(args.size(), decl.params_pattern.size());
  for(std::size_t i = 0; i < count; ++i) {
    TypePtr pattern;
    TypePtr actual;
    adjusted_template_deduction_reason_pair(decl, args[i], i, pattern, actual);
    if(!pattern ||
       !actual ||
       !type_mentions_deduction_parameter(ctx, decl.parameters, pattern)) {
      continue;
    }
    if(template_deduction_reason_has_shape_mismatch(decl.parameters,
                                                    pattern,
                                                    actual)) {
      return true;
    }
    const std::string pattern_template = named_template_base_name(pattern);
    if(pattern_template.empty()) {
      continue;
    }
    const std::string actual_template = named_template_base_name(actual);
    if(actual_template.empty() ||
       !template_base_names_compatible(pattern_template, actual_template)) {
      return true;
    }
  }
  return false;
}

std::string function_template_deduction_failure_drop_reason(
    SemanticContext & ctx,
    FunctionTemplateDecl & decl,
    const std::vector<ExprInfo> & args,
    bool has_explicit_args)
{
  if(has_explicit_args) {
    for(std::size_t i = 0; i < decl.parameters.size(); ++i) {
      const TemplateParameterInfo & parameter = decl.parameters[i];
      if(parameter.kind == TemplateParameterInfo::TP_NON_TYPE &&
         parameter.default_argument &&
         ctx.type_depends_on_template_parameter(parameter.value_type)) {
        return "substitution_failure";
      }
    }
  }
  if(template_deduction_reason_is_inconsistent(decl, args)) {
    return "inconsistent";
  }
  if(template_deduction_reason_has_nondeduced_mismatch(ctx, decl, args)) {
    return "non_deduced_mismatch";
  }
  return "substitution_failure";
}

std::string function_template_witness_name(
    SemanticContext & ctx,
    const FunctionTemplateDecl * decl)
{
  if(!decl) {
    return std::string();
  }
  if(decl->declaring_scope) {
    if(decl->declaring_scope->class_info) {
      return template_api::class_witness_output_qualified_name(
                 ctx,
                 *decl->declaring_scope->class_info) +
          "::" + decl->name;
    }
    return semantic_lookup::scope_symbol_qualified_name(*decl->declaring_scope, decl->name);
  }
  return decl->name;
}

struct FunctionWitnessDeclAnchor
{
  std::string location;
  witness::TemplateWitnessSourceAnchorKind kind =
      witness::TemplateWitnessSourceAnchorKind::None;
};

FunctionWitnessDeclAnchor function_template_witness_decl_anchor(
    SemanticContext & ctx,
    const FunctionTemplateDecl * decl)
{
  FunctionWitnessDeclAnchor anchor;
  const semantic_model::SourceDeclAnchorCache & decl_anchor =
      semantic_trace::function_template_decl_anchor(ctx, decl);
  if(!decl_anchor.name_location.empty()) {
    anchor.location = normalize_template_witness_location(
        decl_anchor.name_location);
    anchor.kind = witness::TemplateWitnessSourceAnchorKind::DeclarationName;
    return anchor;
  }
  anchor.location = normalize_template_witness_location(
      decl_anchor.approximate_location);
  if(!anchor.location.empty()) {
    anchor.kind = witness::TemplateWitnessSourceAnchorKind::ApproximateDeclaration;
  }
  return anchor;
}

std::string function_template_witness_decl_location(SemanticContext & ctx,
                                                    const FunctionTemplateDecl * decl)
{
  return function_template_witness_decl_anchor(ctx, decl).location;
}

FunctionWitnessDeclAnchor constructor_template_witness_decl_anchor(
    SemanticContext & ctx,
    const FunctionTemplateDecl * decl)
{
  FunctionWitnessDeclAnchor anchor;
  if(!decl) {
    return anchor;
  }
  if(decl->definition_inner) {
    anchor.location = normalize_template_witness_location(
        ctx.source_location_for_node(*decl->definition_inner));
    anchor.kind = anchor.location.empty() ?
        witness::TemplateWitnessSourceAnchorKind::None :
        witness::TemplateWitnessSourceAnchorKind::ApproximateDeclaration;
    return anchor;
  }
  if(decl->inner) {
    anchor.location = normalize_template_witness_location(
        ctx.source_location_for_node(*decl->inner));
    anchor.kind = anchor.location.empty() ?
        witness::TemplateWitnessSourceAnchorKind::None :
        witness::TemplateWitnessSourceAnchorKind::ApproximateDeclaration;
    return anchor;
  }
  if(decl->definition_node) {
    anchor.location = normalize_template_witness_location(
        ctx.source_location_for_node(*decl->definition_node));
    anchor.kind = anchor.location.empty() ?
        witness::TemplateWitnessSourceAnchorKind::None :
        witness::TemplateWitnessSourceAnchorKind::ApproximateDeclaration;
    return anchor;
  }
  if(decl->declaration_node) {
    anchor.location = normalize_template_witness_location(
        ctx.source_location_for_node(*decl->declaration_node));
    anchor.kind = anchor.location.empty() ?
        witness::TemplateWitnessSourceAnchorKind::None :
        witness::TemplateWitnessSourceAnchorKind::ApproximateDeclaration;
    return anchor;
  }
  return function_template_witness_decl_anchor(ctx, decl);
}

const ClassInfo * constructor_owner_class(const FunctionBinding * binding)
{
  if(binding == nullptr) {
    return nullptr;
  }
  if(binding->owner_class != nullptr) {
    return binding->owner_class;
  }
  if(binding->declaration_scope != nullptr &&
     binding->declaration_scope->class_info != nullptr) {
    return binding->declaration_scope->class_info;
  }
  if(binding->source_template != nullptr &&
     binding->source_template->declaring_scope != nullptr &&
     binding->source_template->declaring_scope->class_info != nullptr) {
    return binding->source_template->declaring_scope->class_info;
  }
  return nullptr;
}

std::string constructor_binding_name_location(SemanticContext & ctx,
                                              const FunctionBinding * binding)
{
  if(!(binding && binding->is_constructor)) {
    return std::string();
  }
  const CppAstNode * node =
      binding->definition_node ? binding->definition_node : binding->declaration_node;
  if(node == nullptr) {
    return std::string();
  }
  const std::string unqualified_name =
      binding->name.rfind("::") == std::string::npos ?
          binding->name : binding->name.substr(binding->name.rfind("::") + 2);
  return normalize_template_witness_location(
      ctx.source_location_for_name_in_node(*node, unqualified_name, true));
}

std::string implicit_constructor_witness_decl_location(
    SemanticContext & ctx,
    const FunctionBinding * binding)
{
  const ClassInfo * owner = constructor_owner_class(binding);
  if(!(binding && binding->is_constructor &&
       binding->declaration_node == nullptr &&
       binding->definition_node == nullptr &&
       owner != nullptr)) {
    return std::string();
  }
  const semantic_model::SourceDeclAnchorCache & owner_anchor =
      semantic_trace::class_decl_anchor(ctx, owner);
  return normalize_template_witness_location(
      semantic_model::source_decl_anchor_location(owner_anchor));
}

std::string constructor_owner_witness_decl_location(
    SemanticContext & ctx,
    const FunctionBinding * binding)
{
  const ClassInfo * owner = constructor_owner_class(binding);
  if(owner == nullptr) {
    return std::string();
  }
  const semantic_model::SourceDeclAnchorCache & owner_anchor =
      semantic_trace::class_decl_anchor(ctx, owner);
  return normalize_template_witness_location(
      semantic_model::source_decl_anchor_location(owner_anchor));
}

enum class FunctionWitnessDeclLocationKind
{
  Selected,
  CandidateDrop,
};

FunctionWitnessDeclAnchor function_binding_witness_decl_anchor(
    SemanticContext & ctx,
    const FunctionBinding * binding,
    FunctionWitnessDeclLocationKind kind =
        FunctionWitnessDeclLocationKind::Selected);

std::string function_binding_witness_name(SemanticContext & ctx,
                                          const FunctionBinding * binding)
{
  return template_api::function_binding_witness_entity(ctx, binding);
}

std::string function_binding_witness_decl_location(SemanticContext & ctx,
                                                   const FunctionBinding * binding,
                                                   FunctionWitnessDeclLocationKind kind =
                                                       FunctionWitnessDeclLocationKind::Selected)
{
  return function_binding_witness_decl_anchor(ctx, binding, kind).location;
}

FunctionWitnessDeclAnchor function_binding_witness_decl_anchor(
    SemanticContext & ctx,
    const FunctionBinding * binding,
    FunctionWitnessDeclLocationKind kind)
{
  FunctionWitnessDeclAnchor anchor;
  if(!binding) {
    return anchor;
  }
  if(binding->is_constructor) {
    if(binding->source_template) {
      const FunctionWitnessDeclAnchor template_decl =
          constructor_template_witness_decl_anchor(ctx, binding->source_template);
      if(!template_decl.location.empty()) {
        return template_decl;
      }
    }
    if(kind == FunctionWitnessDeclLocationKind::CandidateDrop) {
      const std::string name_decl = constructor_binding_name_location(ctx, binding);
      if(!name_decl.empty()) {
        anchor.location = name_decl;
        anchor.kind = witness::TemplateWitnessSourceAnchorKind::DeclarationName;
        return anchor;
      }
    }
    const std::string implicit_decl =
        implicit_constructor_witness_decl_location(ctx, binding);
    if(!implicit_decl.empty()) {
      anchor.location = implicit_decl;
      anchor.kind = witness::TemplateWitnessSourceAnchorKind::ApproximateDeclaration;
      return anchor;
    }
  }
  if(binding->source_template) {
    const FunctionWitnessDeclAnchor source_decl =
        function_template_witness_decl_anchor(ctx, binding->source_template);
    if(!source_decl.location.empty()) {
      return source_decl;
    }
  }
  if(kind == FunctionWitnessDeclLocationKind::CandidateDrop &&
     !binding->name.empty() &&
     binding->name.find("operator") != std::string::npos) {
    if(binding->declaration_node) {
      const std::string operator_decl = normalize_template_witness_location(
          ctx.source_location_for_name_in_node(*binding->declaration_node,
                                               "operator",
                                               true));
      if(!operator_decl.empty()) {
        anchor.location = operator_decl;
        anchor.kind = witness::TemplateWitnessSourceAnchorKind::DeclarationName;
        return anchor;
      }
    }
    if(binding->definition_node) {
      const std::string operator_decl = normalize_template_witness_location(
          ctx.source_location_for_name_in_node(*binding->definition_node,
                                               "operator",
                                               true));
      if(!operator_decl.empty()) {
        anchor.location = operator_decl;
        anchor.kind = witness::TemplateWitnessSourceAnchorKind::DeclarationName;
        return anchor;
      }
    }
  }
  if(kind == FunctionWitnessDeclLocationKind::CandidateDrop &&
     binding->owner_class &&
     binding->declaration_node == nullptr &&
     binding->definition_node == nullptr &&
     (binding->is_copy_assignment || binding->is_move_assignment)) {
    const semantic_model::SourceDeclAnchorCache & owner_anchor =
        semantic_trace::class_decl_anchor(ctx, binding->owner_class);
    anchor.location = normalize_template_witness_location(
        semantic_model::source_decl_anchor_location(owner_anchor));
    if(!anchor.location.empty()) {
      anchor.kind = witness::TemplateWitnessSourceAnchorKind::ApproximateDeclaration;
      return anchor;
    }
  }
  const semantic_model::SourceDeclAnchorCache & decl_anchor =
      semantic_trace::function_binding_decl_anchor(ctx, binding);
  anchor.location = normalize_template_witness_location(
      semantic_model::source_decl_anchor_location(decl_anchor));
  if(semantic_model::source_decl_anchor_has_name_location(decl_anchor)) {
    anchor.kind = witness::TemplateWitnessSourceAnchorKind::DeclarationName;
    return anchor;
  }
  if(!anchor.location.empty()) {
    anchor.kind = witness::TemplateWitnessSourceAnchorKind::ApproximateDeclaration;
    return anchor;
  }
  anchor.location = normalize_template_witness_location(
      candidate_primary_location(ctx, const_cast<FunctionBinding *>(binding)));
  anchor.kind = anchor.location.empty() ?
      witness::TemplateWitnessSourceAnchorKind::None :
      witness::TemplateWitnessSourceAnchorKind::ApproximateDeclaration;
  return anchor;
}

std::string function_candidate_rejection_drop_reason(const std::string & rejection)
{
  if(rejection.empty()) {
    return std::string();
  }
  if(rejection.find("too many args") != std::string::npos ||
     rejection.find("argument count mismatch") != std::string::npos ||
     rejection.find("member argument count mismatch") != std::string::npos) {
    return "too_many_arguments";
  }
  if(rejection.find("conversion failed") != std::string::npos ||
     rejection.find("non-forwarding-rvalue") != std::string::npos) {
    return "bad_conversion";
  }
  if(rejection.find("arg analysis failed") != std::string::npos ||
     rejection.find("argument analysis failed") != std::string::npos ||
     rejection.find("default analysis failed") != std::string::npos ||
     rejection.find("substitution") != std::string::npos ||
     rejection.find("missing defaults") != std::string::npos) {
    return "substitution_failure";
  }
  return "substitution_failure";
}

bool constructor_is_class_copy_or_move_candidate(const ClassInfo & info,
                                                 const FunctionBinding * binding)
{
  if(!(binding && binding->is_constructor && binding->params.size() == 2)) {
    return false;
  }
  TypePtr param = strip_top_level_cv(binding->params[1].second);
  if(!param ||
     (param->kind != Type::TK_LVALUE_REFERENCE &&
      param->kind != Type::TK_RVALUE_REFERENCE)) {
    return false;
  }
  return same_type_with_compatible_top_cv(param->inner, info.type);
}

bool has_materialized_copy_or_move_constructor_candidate(const ClassInfo & info)
{
  std::map<std::string, std::vector<FunctionBinding *> >::const_iterator found =
      info.methods.find(info.name);
  if(found == info.methods.end()) {
    return false;
  }
  for(std::size_t i = 0; i < found->second.size(); ++i) {
    if(constructor_is_class_copy_or_move_candidate(info, found->second[i])) {
      return true;
    }
  }
  return false;
}

void append_unmaterialized_copy_move_constructor_arity_drop(
    SemanticContext & ctx,
    const ClassInfo & info,
    std::size_t arg_count,
    const ConstructorSelectionOptions & options,
    const FunctionBinding * chosen,
    std::vector<witness::TemplateWitnessSourceDrop> & out)
{
  if(!template_witness_source_capture_enabled_for_calls(ctx) ||
     arg_count == 1 ||
     !options.synthesize_implicit_copy_move ||
     !(chosen && chosen->is_constructor && chosen->source_template) ||
     has_materialized_copy_or_move_constructor_candidate(info)) {
    return;
  }
  // Do not materialize the implicit binding here; that changes overload suffixes
  // in emitted LowIR. This records the semantic candidate that arity rejects.
  witness::append_source_drop(out,
                              function_binding_witness_name(ctx, chosen),
                              constructor_owner_witness_decl_location(ctx, chosen),
                              arg_count == 0 ? "too_few_arguments" :
                                               "too_many_arguments");
}

void note_owner_class_use_source_event(SemanticContext & ctx,
                                       const std::string & use_location,
                                       const FunctionBinding * chosen)
{
  if(!template_witness_source_capture_enabled_for_calls(ctx) ||
     !chosen ||
     chosen->is_constructor ||
     !chosen->owner_class ||
     !chosen->owner_class->source_template ||
     chosen->owner_class->instantiation_arguments.empty()) {
    return;
  }

  const ClassInfo & owner = *chosen->owner_class;
  semantic_template_class::emit_instantiated_class_template_use_source(
      ctx,
      owner,
      use_location,
      witness::SourceUseRole::QualifierUse);
}

void append_template_function_candidate_drop(
    SemanticContext & ctx,
    const FunctionTemplateDecl * decl,
    const std::string & reason,
    std::vector<witness::TemplateWitnessSourceDrop> * out)
{
  if(!(out && template_witness_source_capture_enabled_for_calls(ctx) && decl) ||
     reason.empty()) {
    return;
  }
  witness::append_source_drop(
      *out,
      function_template_witness_name(ctx, decl),
      function_template_witness_decl_location(ctx, decl),
      reason);
}

void hash_combine(std::size_t & seed, std::size_t value)
{
  seed ^= value + 0x9e3779b97f4a7c15ULL + (seed << 6) + (seed >> 2);
}

void hash_combine(std::size_t & seed, const std::string & value)
{
  hash_combine(seed, std::hash<std::string>()(value));
}

std::size_t function_candidate_bucket_key(FunctionBinding * binding)
{
  if(!binding) {
    return 0;
  }

  std::size_t seed = 0;
  hash_combine(seed, canonical_function_lookup_name(binding->name));

  if(binding->source_template) {
    hash_combine(seed,
                 inline_namespace_collapsed_scope_name(
                     binding->source_template->declaring_scope));
    if(binding->type) {
      // Function-template overloads can share a template-instantiation key while
      // still producing different callable signatures.
      hash_combine(seed, describe_type(binding->type));
    }
    hash_combine(seed, binding->template_instantiation_key);
    return seed;
  }

  const bool have_symbol_identity =
      !binding->symbol.internal_symbol.empty() || !binding->symbol.object_symbol.empty();
  if(have_symbol_identity) {
    hash_combine(seed, binding->symbol.internal_symbol);
    hash_combine(seed, binding->symbol.object_symbol);
    hash_combine(seed, static_cast<std::size_t>(binding->symbol.linkage));
    return seed;
  }

  if(binding->declaration_node) {
    hash_combine(seed, reinterpret_cast<std::size_t>(binding->declaration_node));
    return seed;
  }

  if(binding->declaration_scope) {
    hash_combine(seed, reinterpret_cast<std::size_t>(binding->declaration_scope));
    return seed;
  }

  if(binding->type) {
    hash_combine(seed, reinterpret_cast<std::size_t>(binding->type.get()));
  }
  if(!binding->template_instantiation_key.empty()) {
    hash_combine(seed, binding->template_instantiation_key);
  }
  return seed;
}

bool contains_equivalent_function_candidate(const FunctionCandidateBucketMap & buckets,
                                            FunctionBinding * binding,
                                            SemanticContext * ctx = nullptr)
{
  const std::size_t key = function_candidate_bucket_key(binding);
  FunctionCandidateBucketMap::const_iterator found = buckets.find(key);
  if(found == buckets.end()) {
    return false;
  }
  for(size_t i = 0; i < found->second.size(); ++i) {
    if(ctx && !ctx->function_binding_is_live(found->second[i])) {
      continue;
    }
    if(same_function_candidate_entity(found->second[i], binding)) {
      return true;
    }
  }
  return false;
}

void note_function_candidate_bucket(FunctionCandidateBucketMap & buckets,
                                    FunctionBinding * binding)
{
  buckets[function_candidate_bucket_key(binding)].push_back(binding);
}

struct FunctionCandidateRefreshKey
{
  ClassInfo * owner_class = nullptr;
  std::string name;
  TypePtr type;
  RefQualifier ref_qualifier = RQ_NONE;
  bool has_source_template = false;
  const CppAstNode * source_template_declaration = nullptr;
  std::string template_instantiation_key;
};

FunctionCandidateRefreshKey function_candidate_refresh_key(
    const FunctionBinding & binding)
{
  FunctionCandidateRefreshKey key;
  key.owner_class = binding.owner_class;
  key.name = semantic_utils::unqualified_member_name(
      canonical_function_lookup_name(binding.name));
  key.type = binding.type;
  key.ref_qualifier = binding.ref_qualifier;
  key.has_source_template = binding.source_template != nullptr;
  key.source_template_declaration =
      binding.source_template ? binding.source_template->declaration_node : nullptr;
  key.template_instantiation_key = binding.template_instantiation_key;
  return key;
}

FunctionBinding * refresh_invalidated_member_candidate(
    SemanticContext & ctx,
    const FunctionCandidateRefreshKey & key)
{
  if(!key.owner_class || key.name.empty() || !key.type) {
    return nullptr;
  }

  if(!key.has_source_template) {
    FunctionBinding * refreshed =
        ctx.find_equivalent_class_function(*key.owner_class,
                                           key.name,
                                           key.type,
                                           key.ref_qualifier);
    return ctx.function_binding_is_live(refreshed) ? refreshed : nullptr;
  }

  std::map<std::string, std::vector<FunctionBinding *> >::iterator found =
      key.owner_class->methods.find(key.name);
  if(found == key.owner_class->methods.end()) {
    return nullptr;
  }
  for(std::size_t i = 0; i < found->second.size(); ++i) {
    FunctionBinding * candidate = found->second[i];
    if(!ctx.function_binding_is_live(candidate) ||
       !candidate->source_template ||
       candidate->source_template->declaration_node !=
           key.source_template_declaration ||
       candidate->template_instantiation_key != key.template_instantiation_key ||
       candidate->ref_qualifier != key.ref_qualifier ||
       !type_equals(candidate->type, key.type)) {
      continue;
    }
    return candidate;
  }
  return nullptr;
}

bool function_candidate_matches_refresh_key(
    SemanticContext & ctx,
    FunctionBinding * candidate,
    const FunctionCandidateRefreshKey & key)
{
  if(!key.owner_class ||
     !ctx.function_binding_is_live(candidate) ||
     candidate->owner_class != key.owner_class ||
     semantic_utils::unqualified_member_name(
         canonical_function_lookup_name(candidate->name)) != key.name ||
     candidate->ref_qualifier != key.ref_qualifier ||
     (candidate->source_template != nullptr) != key.has_source_template ||
     candidate->template_instantiation_key != key.template_instantiation_key ||
     !type_equals(candidate->type, key.type)) {
    return false;
  }
  return !key.has_source_template ||
         candidate->source_template->declaration_node ==
             key.source_template_declaration;
}

bool source_template_is_in_lookup_set(const FunctionBinding * binding,
                                      const vector<FunctionTemplateDecl *> & templates)
{
  if(!binding ||
     !binding->source_template ||
     binding->is_explicit_specialization) {
    return false;
  }
  for(size_t i = 0; i < templates.size(); ++i) {
    if(same_inline_namespace_function_template_entity(binding->source_template,
                                                      templates[i])) {
      return true;
    }
  }
  return false;
}

void suppress_implicit_template_instantiation_lookup_candidates(
    SemanticContext & ctx,
    vector<FunctionBinding *> & candidates,
    const vector<FunctionTemplateDecl *> & templates)
{
  if(candidates.empty()) {
    return;
  }
  candidates.erase(
      remove_if(candidates.begin(),
                candidates.end(),
                [&](FunctionBinding * binding) -> bool
                {
                  if(!ctx.function_binding_is_live(binding)) {
                    return true;
                  }
                  return source_template_is_in_lookup_set(binding, templates);
                }),
      candidates.end());
}

bool scope_is_within(const Scope & scope, const Scope * ancestor)
{
  for(const Scope * current = &scope; current; current = current->parent) {
    if(current == ancestor) {
      return true;
    }
  }
  return false;
}

TypePtr collapse_rvalue_reference_type(const TypePtr & inner)
{
  if(!inner) {
    return inner;
  }
  TypePtr base = strip_top_level_cv(inner);
  if(base && base->kind == Type::TK_LVALUE_REFERENCE) {
    return make_lvalue_reference_raw(base->inner);
  }
  if(base && base->kind == Type::TK_RVALUE_REFERENCE) {
    return make_rvalue_reference_raw(base->inner);
  }
  return make_rvalue_reference_raw(inner);
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

bool try_analyze_declval_call_expression(SemanticContext & ctx,
                                         Scope & scope,
                                         const CppAstNode & node,
                                         const std::string & use_location,
                                         ExprInfo & out)
{
  if(node.children.empty() || node.children[0].kind != CppAstKind::id_expression) {
    return false;
  }

  const CppAstNode * argument_list = cpp_decl::find_child(node, CppAstKind::argument_list);
  if(!argument_list) {
    argument_list = cpp_decl::find_child(node, CppAstKind::paren_argument_list);
  }
  if(!argument_list || !argument_list->children.empty()) {
    return false;
  }

  const TemplateIdSyntax * template_id = cppast_template_id_syntax(node.children[0]);
  if(!template_id ||
     template_id->name.name != "declval" ||
     template_id->arguments.size() != 1) {
    return false;
  }

  TypePtr declval_type;
  const TemplateArgumentSyntax * arg_syntax =
      template_id->argument_syntaxes.size() == 1 ?
          &template_id->argument_syntaxes[0] :
          nullptr;
  if(!arg_syntax ||
     !template_api::type::resolve_type_argument_input(ctx,
                                                      scope,
                                                      arg_syntax,
                                                      true,
                                                      declval_type) ||
     !declval_type) {
    return false;
  }

  if(witness::function_call_recording_enabled(
         ctx.template_witness_context(),
         witness::FunctionCallEmissionOrigin::DeclvalCall) &&
     template_api::template_witness_declval_call_source_capture_enabled() &&
     !ctx.type_depends_on_template_parameter(declval_type)) {
    std::string public_location =
        normalize_template_witness_location(use_location);
    const std::string node_declval_location =
        normalize_template_witness_location(
            source_location_for_name_in_subtree(ctx, node, "declval", false));
    if(source_location_points_at_identifier(ctx,
                                            node_declval_location,
                                            "declval") &&
       source_location_identifier_followed_by(ctx,
                                              node_declval_location,
                                              "declval",
                                              '<')) {
      public_location = node_declval_location;
    }
    if(!public_location.empty() &&
       !source_location_points_at_identifier(ctx, public_location, "declval")) {
      const std::string same_line_declval_location =
          template_api::template_witness_detail::
              source_location_for_identifier_token_on_or_after(
                  ctx.template_witness_context(),
                  public_location,
                  "declval",
                  true);
      if(!same_line_declval_location.empty()) {
        public_location = normalize_template_witness_location(
            same_line_declval_location);
      }
    }
    if(!source_location_points_at_identifier(ctx, public_location, "declval")) {
      public_location.clear();
    }
    if(!source_location_identifier_followed_by(ctx,
                                               public_location,
                                               "declval",
                                               '<')) {
      public_location.clear();
    }
    if(!public_location.empty()) {
      witness::FunctionCallSourceDecision decision;
      decision.origin = witness::FunctionCallEmissionOrigin::DeclvalCall;
      witness::set_use_anchor(decision.location,
                              decision.use_anchor,
                              public_location);
      decision.template_name = "declval";
      decision.selected = "declval";
      decision.selection = witness::SourceSelectionKind::Instantiation;
      witness::TemplateWitnessSourceBinding binding;
      binding.param = "$1";
      binding.arg = semantic_dependent_type::lookup_type_argument_text(ctx, declval_type);
      if(binding.arg.empty()) {
        binding.arg = template_id->arguments[0];
      }
      binding.source = "explicit";
      binding.type_like = true;
      decision.bindings.push_back(binding);
      witness::emit_function_call(ctx.template_witness_context(), decision);
    }
  }

  TypePtr result_type = is_void_type(strip_top_level_cv(declval_type)) ?
                            declval_type :
                            collapse_rvalue_reference_type(declval_type);
  ValueCategory result_category = VC_PRVALUE;
  if(!result_value_category_for_function_result(result_type, result_category)) {
    result_category = VC_PRVALUE;
  }

  out = ExprInfo();
  out.node = make_dump_node(CallSemKind::call_expression);
  ctx.set_expr_info_metadata(out, result_type, result_category);

  CallSemNode resolved_callee = make_dump_node(CallSemKind::callee, node.children[0].value);
  resolved_callee.semantic_type = make_function(result_type, vector<TypePtr>(), false);
  resolved_callee.value_category = CVC_PRVALUE;
  out.node.children.push_back(resolved_callee);
  return true;
}

bool should_use_target_aware_argument_analysis(const CppAstNode & node,
                                               const TypePtr & target);
bool target_has_unknown_array_bound(const TypePtr & target);

struct CachedArgumentAnalysis
{
  enum State { UNKNOWN, VALUE, ERROR } state = UNKNOWN;
  ExprInfo value;
  string error;
};

struct CachedReferenceSourceAnalysis
{
  enum State { UNKNOWN, VALUE, NONE } state = UNKNOWN;
  ExprInfo value;
};

struct SharedCallArgumentAnalyzer
{
  SemanticContext & ctx;
  Scope & scope;
  const vector<const CppAstNode *> & arg_nodes;
  const CallAnalysisOptions & options;
  const CallAnalysisHints * hints;
  bool allow_unknown_bound_array_placeholder;
  vector<CachedArgumentAnalysis> generic_arg_cache;
  vector<CachedReferenceSourceAnalysis> reference_source_cache;

  SharedCallArgumentAnalyzer(SemanticContext & ctx_in,
                             Scope & scope_in,
                             const vector<const CppAstNode *> & arg_nodes_in,
                             const CallAnalysisOptions & options_in,
                             bool allow_unknown_bound_array_placeholder_in = false)
      : ctx(ctx_in),
        scope(scope_in),
        arg_nodes(arg_nodes_in),
        options(options_in),
        hints(options_in.hints),
        allow_unknown_bound_array_placeholder(allow_unknown_bound_array_placeholder_in),
        generic_arg_cache(arg_nodes_in.size()),
        reference_source_cache(arg_nodes_in.size())
  {}

  void set_hints(const CallAnalysisHints * new_hints)
  {
    hints = new_hints;
  }

  ExprInfo analyze_subexpression(const CppAstNode & child) const
  {
    if(!options.instantiate_bodies && child.kind == CppAstKind::call_expression) {
      CallAnalysisHints nested_hints;
      const CallAnalysisHints * nested_hints_ptr = nullptr;
      if(hints && !hints->use_location.empty()) {
        nested_hints.use_location = refine_fragment_use_location(ctx,
                                                                child,
                                                                hints->use_location);
        if(nested_hints.use_location.empty()) {
          nested_hints.use_location = hints->use_location;
        }
        nested_hints_ptr = &nested_hints;
      }
      return analyze_call_expression(
          ctx, scope, child, semantic_policy::without_body_instantiation(nested_hints_ptr));
    }
    return ctx.analyze_expression(scope, child);
  }

  bool analyze_generic_arg(size_t index, ExprInfo & out_arg, string & error)
  {
    if(hints &&
       index < hints->args.size() &&
       hints->args[index]) {
      out_arg = *hints->args[index];
      error.clear();
      return true;
    }

    CachedArgumentAnalysis & cached = generic_arg_cache[index];
    if(cached.state == CachedArgumentAnalysis::VALUE) {
      out_arg = cached.value;
      error.clear();
      return true;
    }
    if(cached.state == CachedArgumentAnalysis::ERROR) {
      error = cached.error;
      return false;
    }

    try
    {
      ScopedCallSemConstructionPath construction_path("overload.arg.generic-cache-miss");
      cached.value = analyze_subexpression(*arg_nodes[index]);
      cached.state = CachedArgumentAnalysis::VALUE;
      out_arg = cached.value;
      error.clear();
      return true;
    }
    catch(const logic_error & e)
    {
      cached.state = CachedArgumentAnalysis::ERROR;
      cached.error = e.what();
      error = cached.error;
      return false;
    }
  }

  bool analyze_reference_source(size_t index, ExprInfo & out_arg)
  {
    CachedReferenceSourceAnalysis & cached = reference_source_cache[index];
    if(cached.state == CachedReferenceSourceAnalysis::VALUE) {
      out_arg = cached.value;
      return true;
    }
    if(cached.state == CachedReferenceSourceAnalysis::NONE) {
      return false;
    }

    ScopedCallSemConstructionPath construction_path("overload.template.reference-source");
    if(!semantic_expression::try_analyze_reference_binding_source_expression(
           ctx, scope, *arg_nodes[index], cached.value)) {
      cached.state = CachedReferenceSourceAnalysis::NONE;
      return false;
    }
    cached.state = CachedReferenceSourceAnalysis::VALUE;
    out_arg = cached.value;
    return true;
  }

  ExprInfo analyze_argument(size_t index,
                            const TypePtr & target,
                            bool use_target)
  {
    const CppAstNode & child = *arg_nodes[index];
    ExprInfo expr;
    TypePtr target_base;
    ClassInfo * target_class = nullptr;
    if(target) {
      target_base = strip_top_level_cv(remove_reference_type(target));
      target_class = ctx.class_info_for_type(target_base);
    }
    const bool prefer_lambda_closure_target =
        child.kind == CppAstKind::lambda_expression &&
        target_class &&
        target_class->is_lambda_closure;
    const auto note_argument_path =
        [&](const char * mode, const char * reason) -> void
        {
          if(!parser_trace::enabled("overload")) {
            return;
          }
          ostringstream trace;
          trace << "action=arg-analysis"
                << " index=" << index
                << " mode=" << mode
                << " reason=" << reason
                << " node-kind=" << cppast_kind_text(child.kind);
          if(target) {
            trace << " target=" << describe_type(target);
          }
          parser_trace::note("overload", std::string(), trace.str());
        };
    if(!prefer_lambda_closure_target &&
       hints &&
       index < hints->args.size() &&
       hints->args[index]) {
      return *hints->args[index];
    }
    if(child.kind == CppAstKind::lambda_expression) {
      if((target_class && target_class->is_lambda_closure) ||
         !target ||
         ctx.type_depends_on_template_parameter(target)) {
        note_argument_path("lambda-closure",
                           prefer_lambda_closure_target ? "lambda-target" :
                           (!target ? "no-target" : "dependent-target"));
        ScopedCallSemConstructionPath construction_path("overload.arg.lambda-closure");
        return semantic_expression::analyze_lambda_expression_as_closure(ctx, scope, child);
      }
      note_argument_path("generic", "non-closure-target");
    }
    if(use_target &&
       should_use_target_aware_argument_analysis(child, target) &&
       ctx.try_analyze_target_aware_expression(scope, child, target, expr)) {
      if(child.kind == CppAstKind::lambda_expression) {
        note_argument_path("target-aware", "target-expression");
      }
      semantic_lifetime::require_reference_bound_temporary_destructor_if_needed(ctx,
                                                                                target,
                                                                                expr);
      return expr;
    }

    string error;
    if(!analyze_generic_arg(index, expr, error)) {
      if(child.kind == CppAstKind::braced_init_list &&
         target &&
         (ctx.type_depends_on_template_parameter(target) ||
          (allow_unknown_bound_array_placeholder &&
           target_has_unknown_array_bound(target)))) {
        ExprInfo placeholder;
        placeholder.type = TypePtr();
        placeholder.category = VC_LVALUE;
        placeholder.node = make_dump_node(CallSemKind::braced_init_list);
        ctx.set_expr_info_metadata(placeholder, placeholder.type, placeholder.category);
        note_argument_path("generic-placeholder", "dependent-braced-init-list");
        return placeholder;
      }
      throw logic_error(error);
    }
    return expr;
  }
};

bool looks_like_operator_function_name(const string & name)
{
  return name.size() > 8 &&
         name.compare(0, 8, "operator") == 0 &&
         !std::isalnum(static_cast<unsigned char>(name[8])) &&
         name[8] != '_';
}

struct CandidateMatch
{
  FunctionBinding * function = nullptr;
  vector<ConversionRank> ranks;
  vector<ExprInfo> args;
  vector<ExprInfo> call_args;
  vector<ExprInfo> source_args;
  vector<string> source_arg_locations;
  vector<TypePtr> params;
  vector<bool> list_initialization_args;
  vector<vector<ConversionRank> > list_initialization_element_ranks;
  vector<bool> needs_rematerialization;
  size_t explicit_arg_count = static_cast<size_t>(-1);
};

struct BestCandidateSelection
{
  size_t index = 0;
  bool ambiguous = false;
};

bool collect_initializer_list_element_conversion_ranks(
    SemanticContext & ctx,
    Scope & scope,
    const CppAstNode & argument,
    const TypePtr & target,
    vector<ConversionRank> & out)
{
  out.clear();
  TypePtr element_type;
  TypePtr target_base = strip_top_level_cv(remove_reference_type(target));
  if(argument.kind != CppAstKind::braced_init_list ||
     !ctx.is_initializer_list_type(target_base, &element_type, nullptr) ||
     !element_type) {
    return false;
  }

  vector<unique_ptr<CppAstNode> > expanded_storage;
  vector<const CppAstNode *> elements;
  for(size_t i = 0; i < argument.children.size(); ++i) {
    const CppAstNode & child = argument.children[i];
    if(child.kind != CppAstKind::pack_expansion_expression) {
      elements.push_back(&child);
      continue;
    }
    vector<CppAstNode> expanded;
    if(!ctx.expand_pack_argument_node(scope, child, expanded)) {
      out.clear();
      return false;
    }
    for(size_t j = 0; j < expanded.size(); ++j) {
      expanded_storage.emplace_back(new CppAstNode(expanded[j]));
      elements.push_back(expanded_storage.back().get());
    }
  }

  for(size_t i = 0; i < elements.size(); ++i) {
    ExprInfo source;
    try
    {
      if(!should_use_target_aware_argument_analysis(*elements[i], element_type) ||
         !ctx.try_analyze_target_aware_expression(scope,
                                                  *elements[i],
                                                  element_type,
                                                  source)) {
        source = ctx.analyze_expression(scope, *elements[i]);
      }
      ExprInfo converted;
      ConversionRank rank = CR_BAD;
      ArgumentConversionOptions options =
          semantic_policy::without_user_defined_body_instantiation();
      options.materialize_standard_adjustments = false;
      if(!ctx.try_argument_conversion(scope,
                                      element_type,
                                      source,
                                      converted,
                                      rank,
                                      options)) {
        out.clear();
        return false;
      }
      out.push_back(rank);
    }
    catch(const logic_error &)
    {
      out.clear();
      return false;
    }
  }
  return true;
}

int compare_candidate_match_preference(SemanticContext & ctx,
                                       const CandidateMatch & current,
                                       const CandidateMatch & best);
int compare_function_template_partial_order_preference(SemanticContext & ctx,
                                                       const CandidateMatch & current,
                                                       const CandidateMatch & best);

bool overload_source_arg_is_function_like(const ExprInfo & arg)
{
  TypePtr base = strip_top_level_cv(arg.type);
  if(!base) {
    return false;
  }
  if(base->kind == Type::TK_FUNCTION) {
    return true;
  }
  if((base->kind == Type::TK_LVALUE_REFERENCE ||
      base->kind == Type::TK_RVALUE_REFERENCE ||
      base->kind == Type::TK_POINTER ||
      base->kind == Type::TK_BLOCK_POINTER) &&
     base->inner) {
    TypePtr inner = strip_top_level_cv(base->inner);
    return inner && inner->kind == Type::TK_FUNCTION;
  }
  return false;
}

bool copy_move_constructor_drop_is_bad_source_conversion(
    const CandidateMatch & match,
    const CandidateMatch & selected)
{
  return match.function &&
         selected.function &&
         match.function->is_constructor &&
         selected.function->is_constructor &&
         (match.function->is_copy_constructor ||
          match.function->is_move_constructor) &&
         !selected.function->is_copy_constructor &&
         !selected.function->is_move_constructor &&
         match.function->owner_class == selected.function->owner_class &&
         match.source_args.size() == 1 &&
         overload_source_arg_is_function_like(match.source_args.front());
}

std::string function_candidate_rank_drop_reason(const CandidateMatch & match,
                                                SemanticContext & ctx,
                                                const CandidateMatch & selected)
{
  const int rank_comparison = compare_candidate_match_preference(ctx, match, selected);
  if(rank_comparison == 0) {
    const int partial_order =
        compare_function_template_partial_order_preference(ctx, match, selected);
    if(partial_order != 0) {
      return "better_candidate_selected";
    }
  }
  for(std::size_t i = 0; i < match.ranks.size(); ++i) {
    if(match.ranks[i] == CR_BAD) {
      return "bad_conversion";
    }
  }
  if(copy_move_constructor_drop_is_bad_source_conversion(match, selected)) {
    return "bad_conversion";
  }
  return "worse_conversion";
}

bool function_template_has_separate_member_definition(
    const FunctionTemplateDecl * decl)
{
  return decl &&
         decl->declaring_scope &&
         decl->declaring_scope->class_info &&
         decl->definition_node &&
         decl->definition_node != decl->declaration_node;
}

bool function_call_source_drops_follow_out_of_class_member_definition(
    const FunctionBinding * chosen)
{
  return chosen &&
         chosen->source_template &&
         !chosen->is_constructor &&
         function_template_has_separate_member_definition(chosen->source_template);
}

bool function_call_drop_is_duplicate_template_materialization(
    const FunctionBinding * chosen,
    const FunctionBinding * dropped)
{
  return chosen &&
         dropped &&
         chosen != dropped &&
         chosen->source_template &&
         dropped->source_template &&
         chosen->source_template == dropped->source_template;
}

bool drop_reason_is_argument_count_mismatch(const std::string & reason)
{
  return reason == "too_few_arguments" || reason == "too_many_arguments";
}

bool source_drop_survives_out_of_class_member_definition_suppression(
    const std::string & reason)
{
  return reason == "bad_conversion";
}

bool function_call_source_drop_is_internal_member_template_arity_detail(
    const FunctionBinding * chosen,
    const template_api::TemplateWitnessSourceDrop & drop)
{
  return chosen &&
         chosen->source_template &&
         chosen->owner_class &&
         chosen->owner_class->source_template &&
         !chosen->is_constructor &&
         drop_reason_is_argument_count_mismatch(drop.reason);
}

void note_function_call_source_event(
    SemanticContext & ctx,
    const std::string & use_location,
    const std::string & template_name,
    const CppAstNode * callee_node,
    FunctionBinding * chosen,
    const std::vector<FunctionBinding *> & built_candidates,
    const std::vector<std::string> & candidate_rejections,
    const std::vector<CandidateMatch> & matches,
    const BestCandidateSelection & selection,
    const std::vector<template_api::TemplateWitnessSourceDrop> & initial_drops,
    bool constructor_source_is_direct_construction,
    std::size_t explicit_arg_count,
    std::size_t built_candidate_count)
{
  const bool trace_source_decision =
      parser_trace::enabled("witness.call");
  const auto trace_return =
      [&](const char * reason,
          const std::string & detail_location = std::string()) -> void
  {
    if(!trace_source_decision) {
      return;
    }
    std::ostringstream trace;
    trace << "function-call-source-skip reason=" << reason
          << " use=" << use_location
          << " detail=" << detail_location
          << " template=" << template_name;
    if(chosen) {
      trace << " chosen=" << function_binding_witness_name(ctx, chosen)
            << " source-template=" << (chosen->source_template ? "yes" : "no")
            << " constructor=" << (chosen->is_constructor ? "yes" : "no");
    }
    parser_trace::note("witness.call", use_location, trace.str());
  };
  if(!template_witness_source_capture_enabled_for_calls(ctx) ||
     !chosen) {
    trace_return("disabled-or-no-chosen");
    return;
  }

  const bool template_related = chosen->source_template != nullptr;
  const bool owner_template_related =
      chosen->owner_class != nullptr &&
      chosen->owner_class->source_template != nullptr &&
      !chosen->owner_class->instantiation_arguments.empty();
  if(witness::template_witness_source_type_lookup_active()) {
    trace_return("type-lookup-active");
    return;
  }
  const std::string selected_name = function_binding_witness_name(ctx, chosen);
  const std::string unqualified_selected =
      selected_name.substr(selected_name.rfind("::") == std::string::npos ?
                               0 :
                               selected_name.rfind("::") + 2);
  const std::string unqualified_template =
      template_name.substr(template_name.rfind("::") == std::string::npos ?
                               0 :
                               template_name.rfind("::") + 2);
  if(unqualified_template == "__declval" ||
     unqualified_selected == "__declval") {
    trace_return("declval");
    return;
  }

  std::string public_location = normalize_template_witness_location(use_location);
  if(callee_node &&
     (callee_node->kind == CppAstKind::member_expression ||
      callee_node->kind == CppAstKind::id_expression)) {
    std::string exact_name_location =
        normalize_template_witness_location(
            source_location_for_name_in_subtree(ctx,
                                                *callee_node,
                                                unqualified_selected,
                                                true));
    if(exact_name_location.empty()) {
      exact_name_location = normalize_template_witness_location(
          template_api::template_witness_detail::
              source_location_for_identifier_token_on_or_after(
                  ctx.template_witness_context(),
                  use_location,
                  unqualified_selected,
                  true));
    }
    bool should_use_exact_name_location =
        callee_node->kind == CppAstKind::member_expression;
    if(callee_node->kind == CppAstKind::id_expression) {
      const ParsedSourceLocation parsed_public =
          parse_source_location(public_location);
      const ParsedSourceLocation parsed_exact =
          parse_source_location(exact_name_location);
      should_use_exact_name_location =
          parsed_public.valid &&
          parsed_exact.valid &&
          parsed_public.file == parsed_exact.file &&
          parsed_exact.line > parsed_public.line;
    }
    if(should_use_exact_name_location && !exact_name_location.empty()) {
      public_location = exact_name_location;
    }
  }
  bool constructor_source_syntax = false;
  if(chosen->is_constructor) {
    public_location = refine_constructor_call_source_location(ctx,
                                                              selected_name,
                                                              template_name,
                                                              public_location,
                                                              &constructor_source_syntax);
  } else {
    public_location = refine_operator_call_source_location(ctx,
                                                           selected_name,
                                                           public_location);
  }
  if(public_location.empty()) {
    trace_return("empty-public-location");
    return;
  }
  if(!witness::source_location_capture_enabled(ctx.template_witness_context(),
                                               public_location)) {
    trace_return("location-capture-disabled", public_location);
    return;
  }
  if(chosen->is_constructor &&
     !constructor_source_syntax &&
     !constructor_source_is_direct_construction) {
    trace_return("constructor-not-source-syntax", public_location);
    return;
  }
  if(!chosen->is_constructor &&
     unqualified_selected.compare(0, 8, "operator") != 0) {
    bool source_call_syntax =
        source_location_points_at_identifier(ctx,
                                             public_location,
                                             unqualified_selected);
    if(!source_call_syntax) {
      source_call_syntax =
          source_location_has_identifier_on_or_after(ctx,
                                                     public_location,
                                                     unqualified_selected);
    }
    if(!source_call_syntax) {
      const std::string::size_type split = selected_name.rfind("::");
      if(split != std::string::npos) {
        std::string owner = selected_name.substr(0, split);
        std::size_t component_begin = 0;
        while(component_begin <= owner.size()) {
          const std::size_t component_end = owner.find("::", component_begin);
          std::string component =
              owner.substr(component_begin,
                           component_end == std::string::npos ?
                               std::string::npos :
                               component_end - component_begin);
          component =
              semantic_utils::strip_trailing_top_level_template_arguments(
                  component);
          if(!component.empty() &&
             source_location_points_at_identifier(ctx,
                                                  public_location,
                                                  component)) {
            source_call_syntax = true;
            break;
          }
          if(component_end == std::string::npos) {
            break;
          }
          component_begin = component_end + 2;
        }
      }
    }
    if(!source_call_syntax) {
      const std::string::size_type split = template_name.rfind("::");
      if(split != std::string::npos) {
        std::string owner = template_name.substr(0, split);
        std::size_t component_begin = 0;
        while(component_begin <= owner.size()) {
          const std::size_t component_end = owner.find("::", component_begin);
          std::string component =
              owner.substr(component_begin,
                           component_end == std::string::npos ?
                               std::string::npos :
                               component_end - component_begin);
          component =
              semantic_utils::strip_trailing_top_level_template_arguments(
                  component);
          if(!component.empty() &&
             source_location_points_at_identifier(ctx,
                                                  public_location,
                                                  component)) {
            source_call_syntax = true;
            break;
          }
          if(component_end == std::string::npos) {
            break;
          }
          component_begin = component_end + 2;
        }
      }
    }
    if(!source_call_syntax) {
      trace_return("not-source-call-syntax", public_location);
      return;
    }
  }
  if(trace_source_decision) {
    std::ostringstream trace;
    trace << "function-call-source-record"
          << " use=" << use_location
          << " public=" << public_location
          << " template=" << template_name
          << " selected=" << selected_name;
    parser_trace::note("witness.call", public_location, trace.str());
  }
  const FunctionWitnessDeclAnchor selected_decl_anchor =
      function_binding_witness_decl_anchor(ctx, chosen);
  const std::string selected_decl_location =
      normalize_template_witness_location(selected_decl_anchor.location);
  const bool source_constructor_template_call_requires_definition =
      chosen->is_constructor &&
      !chosen->is_explicit_specialization &&
      !template_api::class_is_explicit_specialization(chosen->owner_class) &&
      !template_api::class_is_explicit_specialization(chosen->lexical_access_class) &&
      template_related &&
      template_api::current_template_witness_entry_context().origin ==
          template_api::TemplateWitnessOrigin::Source &&
     !selected_decl_location.empty() &&
     normalize_template_witness_location(public_location) != selected_decl_location &&
     (chosen->is_constexpr ||
      !source_location_in_template_body_range(ctx, public_location));
  if(source_constructor_template_call_requires_definition) {
    chosen->template_definition_required_by_public_source_call = true;
  }
  if(!chosen->is_constructor &&
     !chosen->is_explicit_specialization &&
     !template_api::class_is_explicit_specialization(chosen->owner_class) &&
     !template_api::class_is_explicit_specialization(chosen->lexical_access_class) &&
     !template_related &&
     (chosen->has_definition || chosen->body || chosen->definition_node) &&
     template_api::function_binding_has_template_identity(chosen) &&
     template_api::current_template_witness_entry_context().origin ==
         template_api::TemplateWitnessOrigin::Source &&
     !selected_decl_location.empty() &&
     normalize_template_witness_location(public_location) != selected_decl_location &&
     (chosen->is_constexpr ||
      !source_location_in_template_body_range(ctx, public_location))) {
    chosen->template_definition_required_by_public_source_call = true;
  }
  if(owner_template_related) {
    note_owner_class_use_source_event(ctx, public_location, chosen);
  }
  if(!template_related) {
    return;
  }
  semantic_template_function::FunctionTemplateCallSourceUseRequest source_use;
  source_use.binding = chosen;
  source_use.use_location = public_location;
  source_use.template_name = template_name;
  source_use.selected = selected_name;
  source_use.selected_decl_anchor.location = selected_decl_anchor.location;
  source_use.selected_decl_anchor.kind = selected_decl_anchor.kind;
  source_use.explicit_arg_count = explicit_arg_count;
  source_use.candidates_viable = static_cast<int>(matches.size());
  const bool suppress_source_drops =
      function_call_source_drops_follow_out_of_class_member_definition(chosen);

  witness::SourceDropSet seen_drops;
  const std::string selected_owner_decl =
      chosen->is_constructor ? constructor_owner_witness_decl_location(ctx, chosen) :
                               std::string();
  const auto append_drop =
      [&](const std::string & candidate,
          const std::string & location,
          const std::string & reason)
  {
    std::string effective_location = location;
    if(effective_location.empty() &&
       chosen->is_constructor &&
       candidate == selected_name) {
      effective_location = selected_owner_decl;
    }
    if(effective_location.empty() &&
       !chosen->is_constructor &&
       candidate == selected_name) {
      effective_location = selected_decl_location;
    }
    witness::append_unique_source_drop(
        seen_drops,
        source_use.drops,
        candidate,
        effective_location,
        reason);
  };

  for(std::size_t i = 0; i < initial_drops.size(); ++i) {
    if((suppress_source_drops &&
        !source_drop_survives_out_of_class_member_definition_suppression(
            initial_drops[i].reason)) ||
       function_call_source_drop_is_internal_member_template_arity_detail(
           chosen,
           initial_drops[i])) {
      continue;
    }
    append_drop(initial_drops[i].candidate,
                initial_drops[i].location,
                initial_drops[i].reason);
  }

  std::set<FunctionBinding *> viable_bindings;
  for(std::size_t i = 0; i < matches.size(); ++i) {
    viable_bindings.insert(matches[i].function);
  }
  const auto implicit_assignment_drop_is_represented_by_selected_owner =
      [&](FunctionBinding * binding, const std::string & reason) -> bool
  {
    if(!binding ||
       !binding->synthesized ||
       (!binding->is_copy_assignment && !binding->is_move_assignment) ||
       !binding->owner_class ||
       !chosen->source_template ||
       !chosen->owner_class ||
       !type_equals(binding->owner_class->type, chosen->owner_class->type)) {
      return false;
    }
    for(std::size_t i = 0; i < source_use.drops.size(); ++i) {
      if(source_use.drops[i].candidate == selected_name &&
         source_use.drops[i].reason == reason) {
        return true;
      }
    }
    return false;
  };
  for(std::size_t i = 0;
      i < built_candidates.size() && i < candidate_rejections.size();
      ++i) {
    if(!built_candidates[i] || candidate_rejections[i].empty() ||
       viable_bindings.count(built_candidates[i]) != 0 ||
       function_call_drop_is_duplicate_template_materialization(chosen,
                                                                built_candidates[i])) {
      continue;
    }
    const std::string reason =
        function_candidate_rejection_drop_reason(candidate_rejections[i]);
    if(implicit_assignment_drop_is_represented_by_selected_owner(
           built_candidates[i], reason)) {
      continue;
    }
    if(suppress_source_drops &&
       !source_drop_survives_out_of_class_member_definition_suppression(
           reason)) {
      continue;
    }
    append_drop(function_binding_witness_name(ctx, built_candidates[i]),
                function_binding_witness_decl_location(
                    ctx,
                    built_candidates[i],
                    FunctionWitnessDeclLocationKind::CandidateDrop),
                reason);
  }
  std::vector<std::size_t> nonselected_match_indices;
  for(std::size_t i = 0; i < matches.size(); ++i) {
    if(i == selection.index || !matches[i].function || matches[i].function == chosen) {
      continue;
    }
    nonselected_match_indices.push_back(i);
  }
  std::sort(nonselected_match_indices.begin(),
            nonselected_match_indices.end(),
            [&](std::size_t lhs, std::size_t rhs)
            {
              int comparison =
                  compare_candidate_match_preference(ctx, matches[lhs], matches[rhs]);
              if(comparison == 0) {
                comparison = compare_function_template_partial_order_preference(
                    ctx,
                    matches[lhs],
                    matches[rhs]);
              }
              if(comparison != 0) {
                return comparison < 0;
              }
              return lhs < rhs;
            });
  for(std::size_t i = 0; i < nonselected_match_indices.size(); ++i) {
    const std::size_t match_index = nonselected_match_indices[i];
    if(function_call_drop_is_duplicate_template_materialization(
           chosen,
           matches[match_index].function)) {
      continue;
    }
    const std::string reason =
        function_candidate_rank_drop_reason(matches[match_index],
                                            ctx,
                                            matches[selection.index]);
    if(implicit_assignment_drop_is_represented_by_selected_owner(
           matches[match_index].function, reason)) {
      continue;
    }
    if(suppress_source_drops &&
       !source_drop_survives_out_of_class_member_definition_suppression(
           reason)) {
      continue;
    }
    append_drop(function_binding_witness_name(ctx, matches[match_index].function),
                function_binding_witness_decl_location(
                    ctx,
                    matches[match_index].function,
                    FunctionWitnessDeclLocationKind::CandidateDrop),
                reason);
  }
  if(chosen->is_constructor &&
     !constructor_source_syntax &&
     !constructor_source_is_direct_construction) {
    for(std::size_t i = 0; i < source_use.drops.size(); ++i) {
      if(source_use.drops[i].reason == "bad_conversion") {
        return;
      }
    }
  }
  std::size_t visible_candidate_count = 0;
  if(chosen->is_constructor) {
    visible_candidate_count = built_candidate_count + initial_drops.size();
    if(visible_candidate_count < source_use.drops.size() + 1) {
      visible_candidate_count = source_use.drops.size() + 1;
    }
  } else {
    std::set<std::pair<std::string, std::string> > seen_candidates;
    for(std::size_t i = 0; i < built_candidates.size(); ++i) {
      if(!built_candidates[i]) {
        continue;
      }
      seen_candidates.insert(
          std::make_pair(function_binding_witness_name(ctx, built_candidates[i]),
                         function_binding_witness_decl_location(
                             ctx,
                             built_candidates[i],
                             FunctionWitnessDeclLocationKind::CandidateDrop)));
    }
    for(std::size_t i = 0; i < initial_drops.size(); ++i) {
      if(initial_drops[i].candidate.empty() || initial_drops[i].location.empty()) {
        continue;
      }
      seen_candidates.insert(
          std::make_pair(initial_drops[i].candidate,
                         initial_drops[i].location));
    }
    visible_candidate_count = seen_candidates.size();
    if(visible_candidate_count < source_use.drops.size() + 1) {
      visible_candidate_count = source_use.drops.size() + 1;
    }
  }
  source_use.candidate_count = static_cast<int>(visible_candidate_count);
  source_use.candidates_built = static_cast<int>(visible_candidate_count);
  semantic_template_function::emit_function_template_call_source_use(ctx, source_use);
}

CppAstNode make_operator_identifier_node(const string & operator_name)
{
  CppAstNode identifier;
  identifier.kind = CppAstKind::identifier;
  identifier.value = operator_name;
  QualifiedName name;
  name.name = operator_name;
  set_cppast_qualified_name_syntax(identifier, name);
  return identifier;
}

string qualified_name_lookup_text(const QualifiedName & qualified)
{
  string out;
  if(qualified.rooted) {
    out += "::";
  }
  for(size_t i = 0; i < qualified.qualifiers.size(); ++i) {
    if(!out.empty() && out != "::") {
      out += "::";
    }
    out += qualified.qualifiers[i];
  }
  if(!out.empty() && out != "::") {
    out += "::";
  }
  out += qualified.name;
  return out;
}

std::string value_binding_lookup_declaration_location(SemanticContext & ctx,
                                                      const ValueBinding & binding)
{
  const CppAstNode * node =
      binding.declaration_node ? binding.declaration_node : binding.definition_node;
  if(node == nullptr) {
    return std::string();
  }

  const std::string location_id_location =
      source_location_for_token_id(ctx.template_witness_context(),
                                   node->source_location_id);
  if(!location_id_location.empty()) {
    return location_id_location;
  }
  if(!binding.name.empty()) {
    const std::string name_location =
        ctx.source_location_for_name_in_node(*node, binding.name, true);
    if(!name_location.empty()) {
      return name_location;
    }
  }
  return ctx.source_location_for_node(*node);
}

std::string ast_node_start_location(SemanticContext & ctx,
                                    const CppAstNode & node)
{
  const std::string location_id_location =
      source_location_for_token_id(ctx.template_witness_context(),
                                   node.source_location_id);
  if(!location_id_location.empty()) {
    return location_id_location;
  }
  return ctx.source_location_for_node(node);
}

bool value_binding_visible_at_call_source(SemanticContext & ctx,
                                          const CppAstNode & use_node,
                                          const ValueBinding & binding)
{
  if(binding.owner_class || binding.kind == ValueBinding::VK_FIELD ||
     (binding.declaration_scope && binding.declaration_scope->class_info)) {
    return true;
  }

  const std::string use_location = ast_node_start_location(ctx, use_node);
  const std::string effective_use_location =
      !use_location.empty() ? use_location : parser_trace::current_order_use_location();
  const std::string declaration_location =
      value_binding_lookup_declaration_location(ctx, binding);
  if(!effective_use_location.empty() &&
     !declaration_location.empty() &&
     source_location_is_strictly_later_in_same_file(declaration_location,
                                                    effective_use_location)) {
    return false;
  }

  const CppAstNode * declaration_node =
      binding.declaration_node ? binding.declaration_node : binding.definition_node;
  if(declaration_node &&
     use_node.source_location_id != 0 &&
     declaration_node->source_location_id != 0 &&
     declaration_node->source_location_id > use_node.source_location_id) {
    return false;
  }
  if(declaration_node &&
     declaration_node->token_end > declaration_node->token_start &&
     use_node.token_end > use_node.token_start &&
     declaration_node->token_start > use_node.token_start) {
    return false;
  }
  return true;
}

const ValueBinding * lookup_id_expression_value_binding_for_call(
    SemanticContext & ctx,
    Scope & scope,
    const CppAstNode & node)
{
  const QualifiedName * qualified = cppast_qualified_name_syntax(node);
  if(!qualified || (!qualified->rooted && qualified->qualifiers.empty())) {
    const ValueBinding * binding = ctx.lookup_value(scope, node.value);
    return binding && value_binding_visible_at_call_source(ctx, node, *binding) ?
        binding :
        nullptr;
  }

  const bool structured_qualified_lookup =
      !node.qualifier_template_id_syntaxes.empty() ||
      !node.qualifier_type_syntaxes.empty();
  if(structured_qualified_lookup) {
    const ValueBinding * binding =
        lookup_qualified_value_binding_node(ctx, scope, *qualified, node);
    return binding && value_binding_visible_at_call_source(ctx, node, *binding) ?
        binding :
        nullptr;
  }

  if(!node.qualifier_template_id_syntaxes.empty()) {
    Scope * qualifier_scope =
        ctx.resolve_qualified_scope_for_node(scope, *qualified, node, false);
    if(qualifier_scope && qualifier_scope->class_info) {
      ClassInfo * qualifier_info = qualifier_scope->class_info;
      if(!qualifier_info->complete && qualifier_info->type) {
        if(ClassInfo * completed =
               complete_class_type_for_lookup(ctx, qualifier_info->type)) {
          qualifier_info = completed;
        }
      }
      if(qualifier_info->member_scope) {
        map<string, ValueBinding>::const_iterator found =
            qualifier_info->member_scope->values.find(qualified->name);
        if(found != qualifier_info->member_scope->values.end()) {
          return value_binding_visible_at_call_source(ctx, node, found->second) ?
              &found->second :
              nullptr;
        }
        MemberValueLookupResult member =
            lookup_member_value(*qualifier_info, qualified->name);
        if(member.binding && member.binding->kind != ValueBinding::VK_FIELD) {
          return value_binding_visible_at_call_source(ctx, node, *member.binding) ?
              member.binding :
              nullptr;
        }
        return nullptr;
      }
    }
  }

  const ValueBinding * binding =
      lookup_qualified_value_binding_node(ctx, scope, *qualified, node);
  return binding && value_binding_visible_at_call_source(ctx, node, *binding) ?
      binding :
      nullptr;
}

CppAstNode make_dot_member_operator_callee(const CppAstNode & operand,
                                           const string & operator_name)
{
  CppAstNode callee;
  callee.kind = CppAstKind::member_expression;
  callee.has_token = true;
  callee.token_kind = RT_SIMPLE;
  callee.simple_type = OP_DOT;
  callee.value = ".";
  callee.children.push_back(operand);
  callee.children.push_back(make_operator_identifier_node(operator_name));
  return callee;
}

string overloaded_assignment_operator_name(const CppAstNode & node)
{
  if(node_has_simple_type(node, OP_ASS)) {
    return "operator=";
  }
  if(node_has_simple_type(node, OP_PLUSASS)) {
    return "operator+=";
  }
  if(node_has_simple_type(node, OP_MINUSASS)) {
    return "operator-=";
  }
  if(node_has_simple_type(node, OP_STARASS)) {
    return "operator*=";
  }
  if(node_has_simple_type(node, OP_DIVASS)) {
    return "operator/=";
  }
  if(node_has_simple_type(node, OP_MODASS)) {
    return "operator%=";
  }
  if(node_has_simple_type(node, OP_XORASS)) {
    return "operator^=";
  }
  if(node_has_simple_type(node, OP_BANDASS)) {
    return "operator&=";
  }
  if(node_has_simple_type(node, OP_BORASS)) {
    return "operator|=";
  }
  if(node_has_simple_type(node, OP_LSHIFTASS)) {
    return "operator<<=";
  }
  if(node_has_simple_type(node, OP_RSHIFTASS)) {
    return "operator>>=";
  }
  return string();
}

bool is_scalar_pseudo_destructor_name(const string & name)
{
  return name.size() > 1 && name[0] == '~';
}

bool destructor_implicit_object_cv_compatible(const TypePtr & target,
                                              const ExprInfo & arg)
{
  TypePtr target_base = strip_top_level_cv(target);
  TypePtr object_type = value_conversion_type(arg);
  TypePtr object_base = strip_top_level_cv(object_type);
  if(!target_base ||
     !object_base ||
     target_base->kind != Type::TK_POINTER ||
     object_base->kind != Type::TK_POINTER) {
    return false;
  }

  TypePtr target_object = strip_top_level_cv(target_base->inner);
  TypePtr actual_object = strip_top_level_cv(object_base->inner);
  return target_object && actual_object && type_equals(target_object, actual_object);
}

bool constructor_candidates_match_deferred_functional_cast(
    SemanticContext & ctx,
    const TypePtr & deferred_functional_cast_type,
    const vector<FunctionBinding *> & candidates)
{
  if(!deferred_functional_cast_type || candidates.empty()) {
    return false;
  }

  TypePtr target_type =
      strip_top_level_cv(remove_reference_type(deferred_functional_cast_type));
  if(!target_type) {
    return false;
  }

  ClassInfo * target_class = complete_class_type_for_lookup(ctx, target_type);
  if(!target_class) {
    return false;
  }

  bool saw_constructor = false;
  for(size_t i = 0; i < candidates.size(); ++i) {
    FunctionBinding * candidate = candidates[i];
    if(!candidate || !candidate->is_constructor || !candidate->owner_class ||
       !candidate->owner_class->type) {
      return false;
    }
    TypePtr candidate_owner_type =
        strip_top_level_cv(remove_reference_type(candidate->owner_class->type));
    if(!candidate_owner_type || !type_equals(candidate_owner_type, target_type)) {
      return false;
    }
    saw_constructor = true;
  }

  return saw_constructor;
}

bool try_analyze_scalar_pseudo_destructor_call(SemanticContext & ctx,
                                               Scope & scope,
                                               const CppAstNode & call_node,
                                               const CppAstNode & callee_node,
                                               const ExprInfo & base,
                                               ExprInfo & out)
{
  if(callee_node.children.size() != 2 ||
     callee_node.children[1].kind != CppAstKind::identifier ||
     !is_scalar_pseudo_destructor_name(callee_node.children[1].value) ||
     call_node.children.size() < 2 ||
     call_node.children[1].kind != CppAstKind::argument_list ||
     !call_node.children[1].children.empty()) {
    return false;
  }

  TypePtr object_type;
  TypePtr base_type = strip_top_level_cv(remove_reference_type(base.type));
  if(node_has_simple_type(callee_node, OP_ARROW)) {
    if(!base_type || base_type->kind != Type::TK_POINTER) {
      return false;
    }
    object_type = strip_top_level_cv(base_type->inner);
  } else if(node_has_simple_type(callee_node, OP_DOT)) {
    object_type = base_type;
  } else {
    return false;
  }

  if(!object_type) {
    return false;
  }
  if(ctx.class_info_for_type(object_type) ||
     complete_class_type_for_lookup(ctx, object_type)) {
    return false;
  }

  TypePtr named_type = ctx.lookup_type(scope, callee_node.children[1].value.substr(1));
  if(!named_type || !type_equals(strip_top_level_cv(named_type), object_type)) {
    return false;
  }

  out.type = make_fundamental(FT_VOID);
  out.category = VC_PRVALUE;
  out.node = make_dump_node(CallSemKind::call_expression);
  out.node.semantic_type = out.type;
  out.node.value_category = CVC_PRVALUE;
  set_dump_token(out.node, call_node);

  CallSemNode callee = make_dump_node(CallSemKind::callee, "__pseudo_destructor");
  callee.semantic_type = make_function(make_fundamental(FT_VOID),
                                       vector<TypePtr>(1, base.type),
                                       false);
  out.node.children.push_back(callee);
  out.node.children.push_back(base.node);
  return true;
}

bool rematerialize_candidate_match_args(SemanticContext & ctx,
                                        Scope & scope,
                                        CandidateMatch & match,
                                        const ArgumentConversionOptions & conversion_options,
                                        bool rematerialize_call_args)
{
  bool already_materialized = match.args.size() >= match.params.size();
  if(already_materialized && rematerialize_call_args &&
     match.call_args.size() < match.params.size()) {
    already_materialized = false;
  }
  if(already_materialized) {
    for(size_t i = 0; i < match.params.size(); ++i) {
      if(match.ranks.size() <= i ||
         (match.ranks[i] != CR_EXACT && match.ranks[i] != CR_ELLIPSIS) ||
         !match.args[i].type) {
        already_materialized = false;
        break;
      }
      if(i < match.needs_rematerialization.size() &&
         match.needs_rematerialization[i]) {
        already_materialized = false;
        break;
      }
      if(rematerialize_call_args) {
        if(!match.call_args[i].type ||
           match.call_args[i].category != match.args[i].category ||
           match.call_args[i].null_pointer_constant != match.args[i].null_pointer_constant ||
           !type_equals(match.call_args[i].type, match.args[i].type)) {
          already_materialized = false;
          break;
        }
      }
    }
  }
  if(already_materialized) {
    if(rematerialize_call_args) {
      if(match.call_args.size() < match.args.size()) {
        match.call_args = match.args;
      } else {
        for(size_t i = 0; i < match.args.size(); ++i) {
          match.call_args[i] = match.args[i];
        }
      }
    }
    return true;
  }

  if(match.source_args.size() < match.params.size()) {
    return false;
  }
  for(size_t i = 0; i < match.params.size(); ++i) {
    if(!match.params[i]) {
      continue;
    }
    ExprInfo rematerialized;
    ConversionRank rank = CR_BAD;
    ArgumentConversionOptions effective_conversion_options = conversion_options;
    if(i < match.source_arg_locations.size() &&
       !match.source_arg_locations[i].empty()) {
      effective_conversion_options.source_use_location =
          match.source_arg_locations[i];
    }
    if(!ctx.try_argument_conversion(scope,
                                    match.params[i],
                                    match.source_args[i],
                                    rematerialized,
                                    rank,
                                    effective_conversion_options)) {
      return false;
    }
    if(i < match.args.size()) {
      match.args[i] = rematerialized;
    }
    if(i < match.ranks.size()) {
      match.ranks[i] = rank;
    }
    if(rematerialize_call_args) {
      if(i < match.call_args.size()) {
        match.call_args[i] = rematerialized;
      } else {
        match.call_args.push_back(rematerialized);
      }
    }
    if(i < match.needs_rematerialization.size()) {
      match.needs_rematerialization[i] = false;
    }
  }
  return true;
}

bool candidate_match_is_all_exact(const CandidateMatch & match)
{
  if(match.ranks.empty()) {
    return true;
  }
  for(size_t i = 0; i < match.ranks.size(); ++i) {
    if(match.ranks[i] != CR_EXACT && match.ranks[i] != CR_ELLIPSIS) {
      return false;
    }
  }
  return true;
}

int compare_function_template_partial_order_preference(SemanticContext & ctx,
                                                       const CandidateMatch & current,
                                                       const CandidateMatch & best);

vector<const CppAstNode *> initializer_argument_nodes(const CppAstNode & node);

CallValueCategory to_call_value_category(ValueCategory category)
{
  switch(category) {
  case VC_LVALUE: return CVC_LVALUE;
  case VC_PRVALUE: return CVC_PRVALUE;
  case VC_XVALUE: return CVC_XVALUE;
  }

  throw_internal_error("unknown value category", std::string(), "overload");
}

string join_string_list(const vector<string> & items, const char * sep)
{
  string out;
  for(size_t i = 0; i < items.size(); ++i) {
    if(i) {
      out += sep;
    }
    out += items[i];
  }
  return out;
}

TypePtr callable_function_type_for_member_pointer(const TypePtr & member_pointer_type)
{
  TypePtr base = strip_top_level_cv(member_pointer_type);
  if(!base || base->kind != Type::TK_MEMBER_POINTER || !is_function_type(base->inner)) {
    return TypePtr();
  }

  TypePtr owner_type = strip_top_level_cv(base->owner);
  if(!owner_type) {
    owner_type = base->owner;
  }
  if(base->inner->function_const || base->inner->function_volatile) {
    owner_type = make_cv(owner_type,
                         base->inner->function_const,
                         base->inner->function_volatile);
  }

  vector<TypePtr> params;
  params.push_back(make_pointer(owner_type));
  for(size_t i = 0; i < base->inner->params.size(); ++i) {
    params.push_back(base->inner->params[i]);
  }
  return make_function(base->inner->inner,
                       params,
                       base->inner->variadic,
                       base->inner->function_const,
                       base->inner->function_volatile,
                       base->inner->prototype_relaxed,
                       base->inner->function_ref_qualifier);
}

ClassInfo * canonicalize_constructor_target(SemanticContext & ctx,
                                            Scope & scope,
                                            ClassInfo & info)
{
  if(!info.source_template) {
    return &info;
  }
  const std::string & instantiation_key =
      class_instantiation_key(info);
  if(!instantiation_key.empty()) {
    auto found =
        info.source_template->instantiations.find(instantiation_key);
    if(found != info.source_template->instantiations.end() &&
       found->second &&
       found->second->source_template == info.source_template &&
       found->second->name == info.name) {
      return found->second;
    }
    found =
        info.source_template->reference_instantiations.find(
            instantiation_key);
    if(found != info.source_template->reference_instantiations.end() &&
       found->second &&
       found->second->source_template == info.source_template &&
       found->second->name == info.name) {
      return found->second;
    }
    return &info;
  }

  const vector<TemplateArgument> & arguments =
      info.has_instantiation_binding_arguments ?
          class_instantiation_binding_arguments(info) :
          info.instantiation_arguments;
  if(arguments.empty() && !info.source_template->parameters.empty()) {
    return &info;
  }
  const string key =
      template_api::template_argument_identity_key(ctx, arguments);
  auto found = info.source_template->instantiations.find(key);
  if(found != info.source_template->instantiations.end() && found->second) {
    return found->second;
  }
  found = info.source_template->reference_instantiations.find(key);
  return found != info.source_template->reference_instantiations.end() &&
         found->second ?
      found->second : &info;
}

ClassInfo * complete_constructor_target_if_ready(SemanticContext & ctx,
                                                 ClassInfo & info)
{
  if(info.complete ||
     info.dependent_instantiation ||
     info.template_instantiation_in_progress ||
     info.full_member_collection_in_progress ||
     info.reference_member_collection_in_progress ||
     !info.type) {
    return &info;
  }
  ClassInfo * completed = ctx.complete_class_type(info.type);
  return completed ? completed : &info;
}

void append_conversion_ranks(ostringstream & out, const vector<ConversionRank> & ranks)
{
  for(size_t i = 0; i < ranks.size(); ++i) {
    if(i != 0) {
      out << ",";
    }
    out << static_cast<int>(ranks[i]);
  }
}

void append_function_candidate(ostringstream & out,
                               SemanticContext & ctx,
                               FunctionBinding * binding,
                               const vector<ConversionRank> * ranks = nullptr)
{
  if(!binding) {
    out << "<null>";
    return;
  }
  out << binding->name << ":" << describe_type(binding->type);
  out << semantic_trace::previous_function_location_note(ctx, "candidate", binding);
  if(ranks) {
    out << " ranks=";
    append_conversion_ranks(out, *ranks);
  }
}

std::string candidate_primary_location(SemanticContext & ctx,
                                       FunctionBinding * binding)
{
  if(!binding) {
    return std::string();
  }
  const CppAstNode * node =
      binding->definition_node ? binding->definition_node : binding->declaration_node;
  return node ? ctx.source_location_for_node(*node) : std::string();
}

void append_binding_trace_identity(std::ostringstream & out,
                                   SemanticContext & ctx,
                                   FunctionBinding * binding)
{
  out << " binding=" << static_cast<void *>(binding);
  if(!binding) {
    return;
  }
  out << " key=" << binding->template_instantiation_key
      << " owner="
      << (binding->owner_class ? binding->owner_class->qualified_name : std::string("<none>"))
      << " source_template=" << static_cast<void *>(binding->source_template)
      << " source_owner="
      << (binding->source_template &&
                  binding->source_template->declaring_scope &&
                  binding->source_template->declaring_scope->class_info ?
              binding->source_template->declaring_scope->class_info->qualified_name :
              std::string("<none>"))
      << " source_decl_loc="
      << semantic_trace::template_decl_primary_location(ctx, binding->source_template)
      << " decl_loc="
      << (binding->declaration_node ? ctx.source_location_for_node(*binding->declaration_node) :
                                      std::string("<none>"))
      << " def_loc="
      << (binding->definition_node ? ctx.source_location_for_node(*binding->definition_node) :
                                     std::string("<none>"));
  const std::string source_decl_detail =
      semantic_trace::template_decl_location_details(ctx, binding->source_template);
  if(!source_decl_detail.empty()) {
    out << " source_decl_detail=" << source_decl_detail;
  }
}

void append_template_param_trace(std::ostringstream & out,
                                 const FunctionBinding * binding)
{
  out << " template_params={";
  if(binding && binding->source_template) {
    const std::vector<template_model::TemplateParameterInfo> & params =
        binding->source_template->parameters;
    for(std::size_t i = 0; i < params.size(); ++i) {
      if(i != 0) {
        out << ",";
      }
      const template_model::TemplateParameterInfo & param = params[i];
      if(param.kind == template_model::TemplateParameterInfo::TP_TYPE) {
        out << "type ";
      } else if(param.kind == template_model::TemplateParameterInfo::TP_NON_TYPE) {
        out << "non-type ";
      } else if(param.kind == template_model::TemplateParameterInfo::TP_TEMPLATE_TEMPLATE) {
        out << "template ";
      }
      if(param.parameter_pack) {
        out << (param.name.empty() ? std::string("<empty>") : param.name) << "...";
      } else {
        out << (param.name.empty() ? std::string("<empty>") : param.name);
      }
    }
  }
  out << "}";
}

bool same_function_candidate_entity(FunctionBinding * lhs, FunctionBinding * rhs)
{
  if(lhs == rhs) {
    return true;
  }
  if(!lhs || !rhs) {
    return false;
  }
  const bool has_template_source =
      lhs->source_template != nullptr || rhs->source_template != nullptr;
  if(!has_template_source &&
     same_inline_namespace_function_entity(*lhs, *rhs)) {
    return true;
  }
  if(!lhs->name.empty() &&
     lhs->name == rhs->name &&
     lhs->type && rhs->type &&
     type_equals(lhs->type, rhs->type)) {
    if(has_template_source) {
      if(!lhs->source_template ||
         !rhs->source_template) {
        return false;
      }
      if(!same_inline_namespace_function_template_entity(lhs->source_template,
                                                        rhs->source_template)) {
        const bool same_instantiated_owner =
            lhs->owner_class &&
            rhs->owner_class &&
            lhs->owner_class->qualified_name == rhs->owner_class->qualified_name;
        const bool same_source_location =
            !lhs->source_template->debug_decl_location.empty() &&
            lhs->source_template->debug_decl_location ==
                rhs->source_template->debug_decl_location;
        if(!same_instantiated_owner ||
           !same_source_location ||
           lhs->template_instantiation_key != rhs->template_instantiation_key) {
          return false;
        }
        return true;
      }
      return lhs->source_template == rhs->source_template ||
             lhs->template_instantiation_key == rhs->template_instantiation_key;
    }
    return true;
  }
  return false;
}

bool ref_qualifier_rejects_implicit_object(RefQualifier ref_qualifier,
                                           const TypePtr & implicit_object_parameter,
                                           semantic_conversion::ValueCategory category)
{
  if(ref_qualifier == RQ_NONE) {
    return false;
  }
  if(ref_qualifier == RQ_RVALUE) {
    return category == VC_LVALUE;
  }
  if(category == VC_LVALUE) {
    return false;
  }
  return !semantic_conversion::ref_qualifier_accepts_implicit_object(
      ref_qualifier,
      implicit_object_parameter,
      category);
}

vector<TemplateArgument> constructor_deduction_local_type_arguments(
    SemanticContext & ctx,
    const vector<ExprInfo> & args)
{
  vector<TemplateArgument> out;
  out.reserve(args.size());
  for(size_t i = 0; i < args.size(); ++i) {
    if(!args[i].type) {
      continue;
    }
    TemplateArgument argument;
    argument.kind = TemplateArgument::TA_TYPE;
    argument.type = args[i].type;
    argument.text = semantic_dependent_type::lookup_type_argument_text(ctx, args[i].type);
    if(argument.text.empty()) {
      argument.text = describe_type(args[i].type);
    }
    out.push_back(argument);
  }
  return out;
}

void collect_constructor_local_named_types_from_type(
    SemanticContext & ctx,
    const TypePtr & type,
    set<const ClassInfo *> & seen,
    set<string> & visiting_named_keys,
    vector<pair<string, TypePtr> > & out)
{
  if(!type) {
    return;
  }

  switch(type->kind) {
  case Type::TK_NAMED:
  {
    if(!type->named_key.empty() &&
       !visiting_named_keys.insert(type->named_key).second) {
      return;
    }
    ClassInfo * info = ctx.class_info_for_type(type);
    if(info) {
      if(info->enclosing_scope &&
         info->enclosing_scope->function != nullptr &&
         seen.insert(info).second) {
        if(!info->name.empty()) {
          out.push_back(make_pair(info->name, info->type));
        }
        if(!info->type->named_key.empty() &&
           info->type->named_key != info->name) {
          out.push_back(make_pair(info->type->named_key, info->type));
        }
        if(!info->qualified_name.empty() &&
           info->qualified_name.find("::") == string::npos &&
           info->qualified_name != info->name) {
          out.push_back(make_pair(info->qualified_name, info->type));
        }
      }
      for(size_t i = 0; i < info->instantiation_arguments.size(); ++i) {
        if(info->instantiation_arguments[i].kind != TemplateArgument::TA_TYPE ||
           !info->instantiation_arguments[i].type) {
          continue;
        }
        collect_constructor_local_named_types_from_type(ctx,
                                                        info->instantiation_arguments[i].type,
                                                        seen,
                                                        visiting_named_keys,
                                                        out);
      }
    }
    return;
  }

  case Type::TK_CV:
  case Type::TK_ATOMIC:
  case Type::TK_POINTER:
  case Type::TK_BLOCK_POINTER:
  case Type::TK_LVALUE_REFERENCE:
  case Type::TK_RVALUE_REFERENCE:
  case Type::TK_ARRAY:
    collect_constructor_local_named_types_from_type(
        ctx, type->inner, seen, visiting_named_keys, out);
    return;

  case Type::TK_MEMBER_POINTER:
    collect_constructor_local_named_types_from_type(
        ctx, type->owner, seen, visiting_named_keys, out);
    collect_constructor_local_named_types_from_type(
        ctx, type->inner, seen, visiting_named_keys, out);
    return;

  case Type::TK_FUNCTION:
    collect_constructor_local_named_types_from_type(
        ctx, type->inner, seen, visiting_named_keys, out);
    for(size_t i = 0; i < type->params.size(); ++i) {
      collect_constructor_local_named_types_from_type(
          ctx, type->params[i], seen, visiting_named_keys, out);
    }
    return;

  default:
    return;
  }
}

void bind_constructor_local_named_types_from_args(SemanticContext & ctx,
                                                  Scope & target,
                                                  const vector<ExprInfo> & args)
{
  set<const ClassInfo *> seen;
  set<string> visiting_named_keys;
  vector<pair<string, TypePtr> > local_named_types;
  for(size_t i = 0; i < args.size(); ++i) {
    collect_constructor_local_named_types_from_type(
        ctx, args[i].type, seen, visiting_named_keys, local_named_types);
  }
  for(size_t i = 0; i < local_named_types.size(); ++i) {
    if(local_named_types[i].first.empty()) {
      continue;
    }
    semantic_scope_mutation::ensure_template_named_type(target,
                                                        local_named_types[i].first,
                                                        local_named_types[i].second);
  }
}

set<string> constructor_template_excluded_parameter_names(
    const vector<TemplateParameterInfo> & parameters)
{
  set<string> out;
  for(size_t i = 0; i < parameters.size(); ++i) {
    if(!parameters[i].name.empty()) {
      out.insert(parameters[i].name);
    }
  }
  return out;
}

void collect_partial_order_placeholder_keys(const TypePtr & type,
                                            std::set<std::string> & out)
{
  if(!type) {
    return;
  }
  TypePtr base = strip_top_level_cv(type);
  if(!base) {
    base = type;
  }
  switch(base->kind) {
  case Type::TK_NAMED:
    if(base->named_key.find("partial-order ") == 0) {
      out.insert(base->named_key);
    }
    for(size_t i = 0; i < base->named_rare().named_dependent_alias_arguments.size(); ++i) {
      collect_partial_order_placeholder_keys(
          base->named_rare().named_dependent_alias_arguments[i].type, out);
    }
    for(size_t i = 0; i < base->named_rare().named_dependent_class_arguments.size(); ++i) {
      collect_partial_order_placeholder_keys(
          base->named_rare().named_dependent_class_arguments[i].type, out);
    }
    for(size_t i = 0;
        i < base->named_rare().named_dependent_template_template_arguments.size();
        ++i) {
      collect_partial_order_placeholder_keys(
          base->named_rare().named_dependent_template_template_arguments[i].type, out);
    }
    return;
  case Type::TK_FUNCTION:
    collect_partial_order_placeholder_keys(base->inner, out);
    for(size_t i = 0; i < base->params.size(); ++i) {
      collect_partial_order_placeholder_keys(base->params[i], out);
    }
    return;
  case Type::TK_POINTER:
  case Type::TK_BLOCK_POINTER:
  case Type::TK_LVALUE_REFERENCE:
  case Type::TK_RVALUE_REFERENCE:
  case Type::TK_ARRAY:
  case Type::TK_ATOMIC:
  case Type::TK_CV:
    collect_partial_order_placeholder_keys(base->inner, out);
    if(base->owner) {
      collect_partial_order_placeholder_keys(base->owner, out);
    }
    return;
  case Type::TK_MEMBER_POINTER:
    collect_partial_order_placeholder_keys(base->owner, out);
    collect_partial_order_placeholder_keys(base->inner, out);
    return;
  case Type::TK_FUNDAMENTAL:
    return;
  }
}

std::string normalize_template_parameter_reference_text(std::string text)
{
  text = semantic_utils::trim_space(text);
  if(text.size() >= 3 && text.compare(text.size() - 3, 3, "...") == 0) {
    text.erase(text.size() - 3);
    text = semantic_utils::trim_space(text);
  }
  const std::string dependent_prefix = "dependent type ";
  if(text.compare(0, dependent_prefix.size(), dependent_prefix) == 0) {
    text = semantic_utils::trim_space(text.substr(dependent_prefix.size()));
  }
  if(text.compare(0, 9, "typename ") == 0) {
    text = semantic_utils::trim_space(text.substr(9));
  }
  return text;
}

const TemplateParameterInfo * non_type_parameter_pack_for_reference_text(
    const vector<TemplateParameterInfo> & parameters,
    const std::string & text)
{
  std::string normalized = normalize_template_parameter_reference_text(text);
  if(normalized.compare(0, 16, "dependent value ") == 0) {
    normalized = semantic_utils::trim_space(normalized.substr(16));
  }
  if(normalized.empty()) {
    return nullptr;
  }
  const TemplateParameterInfo * parameter =
      find_template_parameter(parameters, normalized);
  if(!parameter) {
    parameter = template_model::find_template_parameter_by_name(parameters, normalized);
  }
  return parameter &&
         parameter->kind == TemplateParameterInfo::TP_NON_TYPE &&
         parameter->parameter_pack ?
             parameter :
             nullptr;
}

const TemplateParameterInfo * direct_type_parameter_pack_reference(
    const vector<TemplateParameterInfo> & parameters,
    const TypePtr & type)
{
  TypePtr base = strip_top_level_cv(type);
  if(!base || base->kind != Type::TK_NAMED) {
    return nullptr;
  }

  const TemplateParameterInfo * parameter =
      find_template_parameter(parameters, base);
  if(!parameter ||
     parameter->kind != TemplateParameterInfo::TP_TYPE ||
     !parameter->parameter_pack) {
    return nullptr;
  }
  return parameter;
}

const TemplateParameterInfo * direct_non_type_parameter_pack_reference(
    const vector<TemplateParameterInfo> & parameters,
    const TemplateArgument & argument)
{
  if(argument.kind != TemplateArgument::TA_VALUE) {
    return nullptr;
  }
  const TemplateParameterInfo * parameter = nullptr;
  if(!argument.text.empty()) {
    parameter = non_type_parameter_pack_for_reference_text(parameters, argument.text);
  }
  if(!parameter && argument.source_syntax) {
    if(!argument.source_syntax->text.empty()) {
      parameter =
          non_type_parameter_pack_for_reference_text(parameters,
                                                    argument.source_syntax->text);
    }
    if(!parameter && !argument.source_syntax->source_text.empty()) {
      parameter =
          non_type_parameter_pack_for_reference_text(parameters,
                                                    argument.source_syntax->source_text);
    }
  }
  return parameter;
}

const TemplateParameterInfo * direct_non_type_parameter_pack_reference_syntax(
    const vector<TemplateParameterInfo> & parameters,
    const TemplateArgumentSyntax & syntax)
{
  const TemplateParameterInfo * parameter = nullptr;
  if(!syntax.text.empty()) {
    parameter = non_type_parameter_pack_for_reference_text(parameters, syntax.text);
  }
  if(!parameter && !syntax.source_text.empty()) {
    parameter =
        non_type_parameter_pack_for_reference_text(parameters, syntax.source_text);
  }
  return parameter;
}

void collect_type_parameter_pack_references(const vector<TemplateParameterInfo> & parameters,
                                            const TypePtr & type,
                                            std::set<const TemplateParameterInfo *> & out)
{
  if(!type) {
    return;
  }
  TypePtr base = strip_top_level_cv(type);
  if(!base) {
    base = type;
  }
  switch(base->kind) {
  case Type::TK_NAMED:
  {
    const TemplateParameterInfo * direct =
        direct_type_parameter_pack_reference(parameters, base);
    if(direct) {
      out.insert(direct);
    }
    for(size_t i = 0; i < base->named_rare().named_dependent_alias_arguments.size(); ++i) {
      collect_type_parameter_pack_references(
          parameters, base->named_rare().named_dependent_alias_arguments[i].type, out);
    }
    for(size_t i = 0; i < base->named_rare().named_dependent_class_arguments.size(); ++i) {
      collect_type_parameter_pack_references(
          parameters, base->named_rare().named_dependent_class_arguments[i].type, out);
    }
    for(size_t i = 0;
        i < base->named_rare().named_dependent_template_template_arguments.size();
        ++i) {
      collect_type_parameter_pack_references(
          parameters, base->named_rare().named_dependent_template_template_arguments[i].type, out);
    }
    return;
  }
  case Type::TK_FUNCTION:
    collect_type_parameter_pack_references(parameters, base->inner, out);
    for(size_t i = 0; i < base->params.size(); ++i) {
      collect_type_parameter_pack_references(parameters, base->params[i], out);
    }
    return;
  case Type::TK_POINTER:
  case Type::TK_BLOCK_POINTER:
  case Type::TK_LVALUE_REFERENCE:
  case Type::TK_RVALUE_REFERENCE:
  case Type::TK_ARRAY:
  case Type::TK_ATOMIC:
  case Type::TK_CV:
    collect_type_parameter_pack_references(parameters, base->inner, out);
    if(base->owner) {
      collect_type_parameter_pack_references(parameters, base->owner, out);
    }
    return;
  case Type::TK_MEMBER_POINTER:
    collect_type_parameter_pack_references(parameters, base->owner, out);
    collect_type_parameter_pack_references(parameters, base->inner, out);
    return;
  case Type::TK_FUNDAMENTAL:
    return;
  }
}

void collect_non_type_parameter_pack_references(
    const vector<TemplateParameterInfo> & parameters,
    const TypePtr & type,
    std::set<const TemplateParameterInfo *> & out)
{
  if(!type) {
    return;
  }
  TypePtr base = strip_top_level_cv(type);
  if(!base) {
    base = type;
  }
  switch(base->kind) {
  case Type::TK_NAMED:
  {
    if(shared_ptr<const ClassTemplateSpecializationMangleInfo> info =
           named_type_class_template_specialization_mangle_info_const(base)) {
      for(size_t i = 0; i < info->arguments.size(); ++i) {
        const TemplateArgument & argument = info->arguments[i];
        if(const TemplateParameterInfo * direct =
               direct_non_type_parameter_pack_reference(parameters, argument)) {
          out.insert(direct);
        }
        if(argument.kind == TemplateArgument::TA_TYPE && argument.type) {
          collect_non_type_parameter_pack_references(parameters, argument.type, out);
        }
      }
    }
    for(size_t i = 0; i < base->named_rare().named_dependent_alias_arguments.size(); ++i) {
      collect_non_type_parameter_pack_references(
          parameters, base->named_rare().named_dependent_alias_arguments[i].type, out);
      if(const TemplateParameterInfo * direct =
             direct_non_type_parameter_pack_reference_syntax(
                 parameters, base->named_rare().named_dependent_alias_arguments[i].syntax)) {
        out.insert(direct);
      }
    }
    for(size_t i = 0; i < base->named_rare().named_dependent_class_arguments.size(); ++i) {
      collect_non_type_parameter_pack_references(
          parameters, base->named_rare().named_dependent_class_arguments[i].type, out);
      if(const TemplateParameterInfo * direct =
             direct_non_type_parameter_pack_reference_syntax(
                 parameters, base->named_rare().named_dependent_class_arguments[i].syntax)) {
        out.insert(direct);
      }
    }
    for(size_t i = 0;
        i < base->named_rare().named_dependent_template_template_arguments.size();
        ++i) {
      collect_non_type_parameter_pack_references(
          parameters, base->named_rare().named_dependent_template_template_arguments[i].type, out);
      if(const TemplateParameterInfo * direct =
             direct_non_type_parameter_pack_reference_syntax(
                 parameters, base->named_rare().named_dependent_template_template_arguments[i].syntax)) {
        out.insert(direct);
      }
    }
    return;
  }
  case Type::TK_FUNCTION:
    collect_non_type_parameter_pack_references(parameters, base->inner, out);
    for(size_t i = 0; i < base->params.size(); ++i) {
      collect_non_type_parameter_pack_references(parameters, base->params[i], out);
    }
    return;
  case Type::TK_POINTER:
  case Type::TK_BLOCK_POINTER:
  case Type::TK_LVALUE_REFERENCE:
  case Type::TK_RVALUE_REFERENCE:
  case Type::TK_ARRAY:
  case Type::TK_ATOMIC:
  case Type::TK_CV:
    collect_non_type_parameter_pack_references(parameters, base->inner, out);
    if(base->owner) {
      collect_non_type_parameter_pack_references(parameters, base->owner, out);
    }
    return;
  case Type::TK_MEMBER_POINTER:
    collect_non_type_parameter_pack_references(parameters, base->owner, out);
    collect_non_type_parameter_pack_references(parameters, base->inner, out);
    return;
  case Type::TK_FUNDAMENTAL:
    return;
  }
}

size_t count_function_template_type_parameter_pack_references(
    const FunctionTemplateDecl & decl)
{
  std::set<const TemplateParameterInfo *> references;
  for(size_t i = 0; i < decl.params_pattern.size(); ++i) {
    collect_type_parameter_pack_references(decl.parameters,
                                           decl.params_pattern[i].second,
                                           references);
  }
  return references.size();
}

size_t count_function_template_non_type_parameter_pack_references(
    const FunctionTemplateDecl & decl)
{
  std::set<const TemplateParameterInfo *> references;
  for(size_t i = 0; i < decl.params_pattern.size(); ++i) {
    collect_non_type_parameter_pack_references(decl.parameters,
                                               decl.params_pattern[i].second,
                                               references);
  }
  return references.size();
}

bool function_template_declares_type_parameter_pack(const FunctionTemplateDecl & decl)
{
  for(size_t i = 0; i < decl.parameters.size(); ++i) {
    if(decl.parameters[i].kind == TemplateParameterInfo::TP_TYPE &&
       decl.parameters[i].parameter_pack) {
      return true;
    }
  }
  return false;
}

bool function_template_declares_non_type_parameter_pack(const FunctionTemplateDecl & decl)
{
  for(size_t i = 0; i < decl.parameters.size(); ++i) {
    if(decl.parameters[i].kind == TemplateParameterInfo::TP_NON_TYPE &&
       decl.parameters[i].parameter_pack) {
      return true;
    }
  }
  return false;
}

int compare_function_template_type_pack_pattern_preference(
    const CandidateMatch & current,
    const CandidateMatch & best)
{
  if(!current.function || !best.function ||
     !current.function->source_template || !best.function->source_template) {
    return 0;
  }

  const bool current_declares_pack =
      function_template_declares_type_parameter_pack(
          *current.function->source_template);
  const bool best_declares_pack =
      function_template_declares_type_parameter_pack(
          *best.function->source_template);
  if(!current_declares_pack && !best_declares_pack) {
    return 0;
  }

  const size_t current_packs =
      current_declares_pack ?
          count_function_template_type_parameter_pack_references(
              *current.function->source_template) :
          0;
  const size_t best_packs =
      best_declares_pack ?
          count_function_template_type_parameter_pack_references(
              *best.function->source_template) :
          0;
  if(current_packs == 0 && best_packs != 0) {
    return -1;
  }
  if(best_packs == 0 && current_packs != 0) {
    return 1;
  }
  if(!current_declares_pack && best_declares_pack) {
    return -1;
  }
  if(!best_declares_pack && current_declares_pack) {
    return 1;
  }
  return 0;
}

int compare_function_template_non_type_pack_pattern_preference(
    const CandidateMatch & current,
    const CandidateMatch & best)
{
  if(!current.function || !best.function ||
     !current.function->source_template || !best.function->source_template) {
    return 0;
  }

  const bool current_declares_pack =
      function_template_declares_non_type_parameter_pack(
          *current.function->source_template);
  const bool best_declares_pack =
      function_template_declares_non_type_parameter_pack(
          *best.function->source_template);
  if(!current_declares_pack && !best_declares_pack) {
    return 0;
  }

  const size_t current_packs =
      current_declares_pack ?
          count_function_template_non_type_parameter_pack_references(
              *current.function->source_template) :
          0;
  const size_t best_packs =
      best_declares_pack ?
          count_function_template_non_type_parameter_pack_references(
              *best.function->source_template) :
          0;
  if(current_packs == 0 && best_packs != 0) {
    return -1;
  }
  if(best_packs == 0 && current_packs != 0) {
    return 1;
  }
  return 0;
}

int compare_partial_order_placeholder_specificity(const vector<TypePtr> & current_params,
                                                  const vector<TypePtr> & best_params)
{
  std::set<std::string> current_keys;
  std::set<std::string> best_keys;
  for(size_t i = 0; i < current_params.size(); ++i) {
    collect_partial_order_placeholder_keys(current_params[i], current_keys);
  }
  for(size_t i = 0; i < best_params.size(); ++i) {
    collect_partial_order_placeholder_keys(best_params[i], best_keys);
  }
  if(current_keys.size() < best_keys.size()) {
    return -1;
  }
  if(best_keys.size() < current_keys.size()) {
    return 1;
  }
  return 0;
}

int partial_order_template_structure_score(const TypePtr & type);

int partial_order_template_argument_structure_score(const TemplateArgument & argument)
{
  if(argument.partial_order_placeholder) {
    return 0;
  }
  int score = 0;
  if(argument.kind == TemplateArgument::TA_TYPE && argument.type) {
    score += partial_order_template_structure_score(argument.type);
  } else if(argument.kind == TemplateArgument::TA_CLASS_TEMPLATE ||
            argument.kind == TemplateArgument::TA_ALIAS_TEMPLATE) {
    ++score;
  } else if(argument.kind == TemplateArgument::TA_VALUE && !argument.dependent) {
    ++score;
  }
  return score;
}

int partial_order_dependent_argument_structure_score(
    const vector<DependentAliasTemplateArgumentSyntax> & arguments)
{
  int score = 0;
  for(size_t i = 0; i < arguments.size(); ++i) {
    if(arguments[i].partial_order_placeholder) {
      continue;
    }
    score += partial_order_template_structure_score(arguments[i].type);
    if(!arguments[i].type && !arguments[i].text.empty()) {
      ++score;
    }
  }
  return score;
}

int partial_order_template_structure_score(const TypePtr & type)
{
  if(!type) {
    return 0;
  }
  TypePtr base = strip_top_level_cv(type);
  if(!base) {
    base = type;
  }

  switch(base->kind) {
  case Type::TK_NAMED:
  {
    if(named_type_is_partial_order_placeholder(base)) {
      return 0;
    }

    int score = 1;
    if(shared_ptr<const ClassTemplateSpecializationMangleInfo> info =
           named_type_class_template_specialization_mangle_info_const(base)) {
      ++score;
      for(size_t i = 0; i < info->arguments.size(); ++i) {
        score += partial_order_template_argument_structure_score(
            info->arguments[i]);
      }
    }
    if(base->named_rare().named_dependent_alias_template_decl) {
      ++score;
      score += partial_order_dependent_argument_structure_score(
          base->named_rare().named_dependent_alias_arguments);
    }
    if(base->named_rare().named_dependent_class_template_decl) {
      ++score;
      score += partial_order_dependent_argument_structure_score(
          base->named_rare().named_dependent_class_arguments);
    }
    if(!base->named_rare().named_dependent_template_template_parameter_name.empty()) {
      ++score;
      score += partial_order_dependent_argument_structure_score(
          base->named_rare().named_dependent_template_template_arguments);
    }
    if(base->named_rare().named_dependent_qualified_owner) {
      score += partial_order_template_structure_score(
          base->named_rare().named_dependent_qualified_owner);
    }
    return score;
  }

  case Type::TK_FUNCTION:
  {
    int score = partial_order_template_structure_score(base->inner);
    for(size_t i = 0; i < base->params.size(); ++i) {
      score += partial_order_template_structure_score(base->params[i]);
    }
    return score;
  }

  case Type::TK_POINTER:
  case Type::TK_BLOCK_POINTER:
  case Type::TK_LVALUE_REFERENCE:
  case Type::TK_RVALUE_REFERENCE:
  case Type::TK_ARRAY:
  case Type::TK_ATOMIC:
  case Type::TK_CV:
  {
    int score = partial_order_template_structure_score(base->inner);
    if(base->owner) {
      score += partial_order_template_structure_score(base->owner);
    }
    return score;
  }

  case Type::TK_MEMBER_POINTER:
    return partial_order_template_structure_score(base->owner) +
           partial_order_template_structure_score(base->inner);

  case Type::TK_FUNDAMENTAL:
    return 1;
  }
  return 0;
}

int compare_partial_order_template_structure_specificity(
    const vector<TypePtr> & current_params,
    const vector<TypePtr> & best_params)
{
  int current_score = 0;
  for(size_t i = 0; i < current_params.size(); ++i) {
    current_score += partial_order_template_structure_score(current_params[i]);
  }

  int best_score = 0;
  for(size_t i = 0; i < best_params.size(); ++i) {
    best_score += partial_order_template_structure_score(best_params[i]);
  }

  if(current_score > best_score) {
    return -1;
  }
  if(best_score > current_score) {
    return 1;
  }
  return 0;
}

int compare_function_template_parameter_count_preference(const CandidateMatch & current,
                                                         const CandidateMatch & best)
{
  if(!current.function || !best.function ||
     !current.function->source_template || !best.function->source_template) {
    return 0;
  }

  const size_t current_count = current.function->source_template->parameters.size();
  const size_t best_count = best.function->source_template->parameters.size();
  if(current_count < best_count) {
    return -1;
  }
  if(best_count < current_count) {
    return 1;
  }
  return 0;
}

bool is_forwarding_reference_pattern(const vector<TemplateParameterInfo> & parameters,
                                     const TypePtr & pattern)
{
  TypePtr base = strip_top_level_cv(pattern);
  if(!base || base->kind != Type::TK_RVALUE_REFERENCE) {
    return false;
  }

  TypePtr inner = base->inner;
  if(!inner || inner->kind != Type::TK_NAMED) {
    return false;
  }

  const TemplateParameterInfo * parameter =
      find_template_parameter(parameters, inner);
  return parameter && parameter->kind == TemplateParameterInfo::TP_TYPE;
}

enum TemplateReferencePatternKind
{
  TRPK_OTHER,
  TRPK_LVALUE_REFERENCE,
  TRPK_FORWARDING_REFERENCE,
};

TemplateReferencePatternKind classify_template_reference_pattern(
    const FunctionTemplateDecl & decl,
    size_t index)
{
  if(index >= decl.params_pattern.size()) {
    return TRPK_OTHER;
  }

  TypePtr base = strip_top_level_cv(decl.params_pattern[index].second);
  if(!base) {
    return TRPK_OTHER;
  }
  if(base->kind == Type::TK_LVALUE_REFERENCE) {
    return TRPK_LVALUE_REFERENCE;
  }
  if(is_forwarding_reference_pattern(decl.parameters, decl.params_pattern[index].second)) {
    return TRPK_FORWARDING_REFERENCE;
  }
  return TRPK_OTHER;
}

int reference_pattern_cv_match_score(const TypePtr & pattern,
                                     const TypePtr & actual)
{
  TypePtr pattern_base = strip_top_level_cv(pattern);
  if(!pattern_base ||
     (pattern_base->kind != Type::TK_LVALUE_REFERENCE &&
      pattern_base->kind != Type::TK_RVALUE_REFERENCE)) {
    return -1;
  }

  bool pattern_const = false;
  bool pattern_volatile = false;
  TypePtr pattern_unqualified;
  if(!top_level_cv_flags(pattern_base->inner,
                         pattern_unqualified,
                         pattern_const,
                         pattern_volatile)) {
    return -1;
  }
  (void)pattern_unqualified;

  bool actual_const = false;
  bool actual_volatile = false;
  TypePtr actual_unqualified;
  if(!top_level_cv_flags(remove_reference_type(actual),
                         actual_unqualified,
                         actual_const,
                         actual_volatile)) {
    return -1;
  }
  (void)actual_unqualified;

  int score = 0;
  if(pattern_const == actual_const) {
    ++score;
  }
  if(pattern_volatile == actual_volatile) {
    ++score;
  }
  return score;
}

string named_template_base_name(const TypePtr & type)
{
  TypePtr base = strip_top_level_cv(remove_reference_type(type));
  if(!base || base->kind != Type::TK_NAMED) {
    return string();
  }

  const string display = named_type_display_text(base);
  string text = display.empty() ? base->named_key : display;
  text = semantic_utils::trim_space(semantic_utils::strip_elaborated_type_prefix(text));
  if(text.compare(0, 15, "dependent type ") == 0) {
    text = text.substr(15);
  } else if(text.compare(0, 9, "typename ") == 0) {
    text = text.substr(9);
  }

  const size_t open = text.find('<');
  if(open == string::npos) {
    return string();
  }
  return text.substr(0, open);
}

string unqualified_template_base_name(const string & name)
{
  const size_t qualifier = name.rfind("::");
  return qualifier == string::npos ? name : name.substr(qualifier + 2);
}

bool template_base_names_compatible(const string & pattern,
                                    const string & actual)
{
  return pattern == actual ||
         unqualified_template_base_name(pattern) ==
             unqualified_template_base_name(actual);
}

bool template_base_name_is_direct_template_template_parameter(
    const vector<TemplateParameterInfo> & parameters,
    const string & pattern)
{
  if(pattern.empty() || pattern.find("::") != string::npos) {
    return false;
  }
  for(size_t i = 0; i < parameters.size(); ++i) {
    if(parameters[i].kind == TemplateParameterInfo::TP_TEMPLATE_TEMPLATE &&
       parameters[i].name == pattern) {
      return true;
    }
  }
  return false;
}

bool type_has_template_base_named(SemanticContext & ctx,
                                  const TypePtr & type,
                                  const string & pattern_template,
                                  set<ClassInfo *> & visiting)
{
  TypePtr base = strip_top_level_cv(remove_reference_type(type));
  if(!base || base->kind != Type::TK_NAMED) {
    return false;
  }

  ClassInfo * info = ctx.complete_class_type(base);
  if(!info) {
    info = ctx.class_info_for_type(base);
  }
  if(!info || !visiting.insert(info).second) {
    return false;
  }

  for(size_t i = 0; i < info->bases.size(); ++i) {
    if(!info->bases[i].type) {
      continue;
    }
    TypePtr base_type = info->bases[i].type->type;
    const string base_template = named_template_base_name(base_type);
    if(!base_template.empty() &&
       template_base_names_compatible(pattern_template, base_template)) {
      return true;
    }
    if(type_has_template_base_named(ctx, base_type, pattern_template, visiting)) {
      return true;
    }
  }
  return false;
}

bool class_or_bases_have_conversion_functions(SemanticContext & ctx,
                                              ClassInfo & info,
                                              set<ClassInfo *> & visited)
{
  if(!visited.insert(&info).second) {
    return false;
  }

  for(map<string, vector<FunctionBinding *> >::const_iterator it = info.methods.begin();
      it != info.methods.end();
      ++it) {
    if(ctx.is_conversion_function_name(it->first)) {
      return true;
    }
  }

  if(info.member_scope) {
    for(map<string, vector<FunctionBinding *> >::const_iterator it =
            info.member_scope->function_sets.begin();
        it != info.member_scope->function_sets.end();
        ++it) {
      if(ctx.is_conversion_function_name(it->first) && !it->second.empty()) {
        return true;
      }
    }

    for(map<string, vector<FunctionTemplateDecl *> >::const_iterator it =
            info.member_scope->function_templates.begin();
        it != info.member_scope->function_templates.end();
        ++it) {
      if(ctx.is_conversion_function_name(it->first) && !it->second.empty()) {
        return true;
      }
    }
  }

  for(size_t i = 0; i < info.bases.size(); ++i) {
    if(info.bases[i].type &&
       class_or_bases_have_conversion_functions(ctx, *info.bases[i].type, visited)) {
      return true;
    }
  }
  return false;
}

bool class_or_bases_have_conversion_functions(SemanticContext & ctx,
                                              ClassInfo & info)
{
  set<ClassInfo *> visited;
  return class_or_bases_have_conversion_functions(ctx, info, visited);
}

void collect_conversion_function_names_for_call(SemanticContext & ctx,
                                                ClassInfo & info,
                                                set<ClassInfo *> & visited,
                                                set<ClassInfo *> & visited_virtual,
                                                set<string> & out)
{
  if(!visited.insert(&info).second) {
    return;
  }

  for(map<string, vector<FunctionBinding *> >::const_iterator it = info.methods.begin();
      it != info.methods.end();
      ++it) {
    if(ctx.is_conversion_function_name(it->first)) {
      out.insert(it->first);
    }
  }

  if(info.member_scope) {
    for(map<string, vector<FunctionBinding *> >::const_iterator it =
            info.member_scope->function_sets.begin();
        it != info.member_scope->function_sets.end();
        ++it) {
      if(ctx.is_conversion_function_name(it->first) && !it->second.empty()) {
        out.insert(it->first);
      }
    }

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
    if(!base.type) {
      continue;
    }
    if(base.is_virtual && !visited_virtual.insert(base.type).second) {
      continue;
    }
    collect_conversion_function_names_for_call(ctx,
                                               *base.type,
                                               visited,
                                               visited_virtual,
                                               out);
  }
}

vector<MemberFunctionLookupResult> collect_visible_conversion_function_sets_for_call(
    SemanticContext & ctx,
    ClassInfo & info)
{
  set<ClassInfo *> visited;
  set<ClassInfo *> visited_virtual;
  set<string> names;
  collect_conversion_function_names_for_call(ctx, info, visited, visited_virtual, names);

  vector<MemberFunctionLookupResult> out;
  for(set<string>::const_iterator it = names.begin(); it != names.end(); ++it) {
    MemberFunctionLookupResult visible = lookup_member_functions(info, *it);
    if(!visible.functions.empty()) {
      out.push_back(visible);
    }
  }
  return out;
}

TypePtr function_binding_result_type(FunctionBinding * binding)
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

TypePtr explicit_conversion_member_target_type(SemanticContext & ctx,
                                               Scope & scope,
                                               const CppAstNode & member_identifier)
{
  const QualifiedName * qualified_name =
      cppast_qualified_name_syntax(member_identifier);
  const string member_name =
      qualified_name && !qualified_name->name.empty() ?
          qualified_name->name :
          semantic_utils::unqualified_member_name(member_identifier.value);
  if(!ctx.is_conversion_function_name(member_name)) {
    return TypePtr();
  }

  if(const CppAstNode * conversion_type_id =
         cppast_conversion_type_id_syntax(member_identifier)) {
    TypePtr parsed_type;
    if(ctx.parse_type_id(scope, *conversion_type_id, parsed_type, false, false)) {
      return parsed_type;
    }
  }

  const string suffix = semantic_utils::trim_space(member_name.substr(8));
  if(suffix.empty() || suffix == "new" || suffix == "delete") {
    return TypePtr();
  }
  return ctx.lookup_type(scope, suffix);
}

bool collect_explicit_conversion_member_call_candidates(
    SemanticContext & ctx,
    ClassInfo & info,
    const TypePtr & target_type,
    MemberFunctionLookupResult & out)
{
  if(!target_type) {
    return false;
  }

  vector<MemberFunctionLookupResult> visible_sets =
      collect_visible_conversion_function_sets_for_call(ctx, info);
  for(size_t i = 0; i < visible_sets.size(); ++i) {
    MemberFunctionLookupResult & set = visible_sets[i];
    for(size_t j = 0; j < set.functions.size(); ++j) {
      FunctionBinding * function = set.functions[j];
      if(!type_equals(function_binding_result_type(function), target_type)) {
        continue;
      }
      if(out.functions.empty()) {
        out.declared_in = set.declared_in;
        out.path_access = set.path_access;
        out.path_offset = set.path_offset;
      }
      out.functions.push_back(function);
    }
  }
  return !out.functions.empty();
}

struct CachedConstructorConversionResult
{
  bool attempted = false;
  bool available = false;
  ExprInfo ctor_expr;
};

ExprInfo make_unmaterialized_constructor_conversion_expr(SemanticContext & ctx,
                                                        const TypePtr & result_type,
                                                        const ExprInfo & source_arg)
{
  ExprInfo result = source_arg;
  result.type = result_type;
  result.category = VC_PRVALUE;
  result.null_pointer_constant = false;
  ctx.set_expr_info_metadata(result, result.type, result.category);
  return result;
}

struct CachedArgumentConversionKey
{
  const Type * target_type = nullptr;
  const ExprInfo * source_identity = nullptr;
  unsigned option_bits = 0;

  bool operator==(const CachedArgumentConversionKey & other) const
  {
    return target_type == other.target_type &&
           source_identity == other.source_identity &&
           option_bits == other.option_bits;
  }
};

struct CachedArgumentConversionKeyHash
{
  std::size_t operator()(const CachedArgumentConversionKey & key) const
  {
    std::size_t seed = 0;
    hash_combine(seed, reinterpret_cast<std::size_t>(key.target_type));
    hash_combine(seed, reinterpret_cast<std::size_t>(key.source_identity));
    hash_combine(seed, static_cast<std::size_t>(key.option_bits));
    return seed;
  }
};

struct CachedArgumentConversionResult
{
  bool attempted = false;
  bool available = false;
  ExprInfo converted_expr;
  ConversionRank rank = CR_BAD;
};

unsigned argument_conversion_option_bits(const ArgumentConversionOptions & options)
{
  return (options.allow_user_defined ? 1u : 0u) |
         (options.instantiate_user_defined_bodies ? 2u : 0u) |
         (options.materialize_user_defined_output ? 4u : 0u) |
         (options.allow_explicit ? 8u : 0u) |
         (options.materialize_standard_adjustments ? 16u : 0u);
}

string cached_constructor_conversion_key(ClassInfo & target_class,
                                         const ExprInfo & source_arg)
{
  ostringstream out;
  out << target_class.qualified_name
      << '|'
      << (source_arg.type ? describe_type(source_arg.type) : string("<null-type>"))
      << '|'
      << static_cast<int>(source_arg.category);
  return out.str();
}

bool try_cached_overload_argument_conversion(
    SemanticContext & ctx,
    Scope & scope,
    const TypePtr & target,
    const ExprInfo & source_arg,
    const ExprInfo * source_identity,
    ExprInfo & out,
    ConversionRank & rank,
    const ArgumentConversionOptions & options,
    map<string, CachedConstructorConversionResult> & ctor_cache,
    unordered_map<CachedArgumentConversionKey,
                  CachedArgumentConversionResult,
                  CachedArgumentConversionKeyHash> & conversion_cache)
{
  ScopedCallSemConstructionPath construction_path("overload.cached-conversion");
  if(source_identity) {
    CachedArgumentConversionKey cache_key;
    cache_key.target_type = target.get();
    cache_key.source_identity = source_identity;
    cache_key.option_bits = argument_conversion_option_bits(options);
    unordered_map<CachedArgumentConversionKey,
                  CachedArgumentConversionResult,
                  CachedArgumentConversionKeyHash>::iterator found =
        conversion_cache.find(cache_key);
    if(found != conversion_cache.end()) {
      if(found->second.available) {
        out = found->second.converted_expr;
        rank = found->second.rank;
      }
      return found->second.available;
    }

    CachedArgumentConversionResult cached_result;
    cached_result.attempted = true;
    const bool available =
        try_cached_overload_argument_conversion(ctx,
                                               scope,
                                               target,
                                               source_arg,
                                               nullptr,
                                               out,
                                               rank,
                                               options,
                                               ctor_cache,
                                               conversion_cache);
    cached_result.available = available;
    if(available) {
      cached_result.converted_expr = out;
      cached_result.rank = rank;
    }
    conversion_cache.insert(make_pair(cache_key, cached_result));
    return available;
  }

  rank = semantic_conversion::standard_conversion_rank(target, source_arg);
  if(rank != CR_BAD) {
    semantic_conversion::apply_standard_conversion_result_metadata(ctx,
                                                                   target,
                                                                   source_arg,
                                                                   out);
    return true;
  }
  if(semantic_conversion::try_semantic_exact_reference_binding(ctx,
                                                               target,
                                                               source_arg,
                                                               out,
                                                               rank)) {
    return true;
  }

  ExprInfo inherited;
  const bool inherited_ok =
      options.materialize_standard_adjustments ?
          semantic_conversion::try_apply_inheritance_conversion(ctx,
                                                                target,
                                                                source_arg,
                                                                inherited) :
          semantic_conversion::try_apply_unmaterialized_inheritance_conversion(
              ctx,
              target,
              source_arg,
              inherited);
  if(inherited_ok) {
    out = inherited;
    rank = semantic_conversion::inheritance_conversion_rank(ctx, target, source_arg);
    return true;
  }

  if(!target || !source_arg.type) {
    return false;
  }

  if(options.allow_explicit ||
     !options.allow_user_defined ||
     options.instantiate_user_defined_bodies ||
     !options.materialize_user_defined_output) {
    return ctx.try_argument_conversion(scope, target, source_arg, out, rank, options);
  }

  TypePtr target_base = strip_top_level_cv(target);
  const bool target_is_reference =
      target_base &&
      (target_base->kind == Type::TK_LVALUE_REFERENCE ||
       target_base->kind == Type::TK_RVALUE_REFERENCE);
  TypePtr target_class_type =
      target_is_reference ? strip_top_level_cv(target_base->inner) : target_base;
  if(!target_class_type) {
    return ctx.try_argument_conversion(
        scope,
        target,
        source_arg,
        out,
        rank,
        semantic_policy::without_user_defined_body_instantiation());
  }

  // Ordinary expressions cannot convert to `std::initializer_list<T>` through
  // the constructor-probing path used for overload candidate screening. The
  // valid initializer_list cases are already built earlier from either an
  // actual initializer_list object or target-aware braced-init-list analysis.
  if(ctx.is_initializer_list_type(target_class_type, nullptr, nullptr)) {
    TypePtr source_base = strip_top_level_cv(remove_reference_type(source_arg.type));
    if(!ctx.is_initializer_list_type(source_base, nullptr, nullptr)) {
      return false;
    }
  }

  ClassInfo * target_class = ctx.class_info_for_type(target_class_type);
  if(target_class && !target_class->complete) {
    target_class = complete_class_type_for_lookup(ctx, target_class_type);
  } else if(!target_class) {
    target_class = complete_class_type_for_lookup(ctx, target_class_type);
  }
  if(!target_class) {
    return ctx.try_argument_conversion(scope, target, source_arg, out, rank, options);
  }

  TypePtr source_class_type = strip_top_level_cv(remove_reference_type(source_arg.type));
  ClassInfo * source_class = ctx.class_info_for_type(source_class_type);
  if(source_class && !source_class->complete) {
    source_class = complete_class_type_for_lookup(ctx, source_class_type);
  }
  if(source_class && class_or_bases_have_conversion_functions(ctx, *source_class)) {
    return ctx.try_argument_conversion(scope, target, source_arg, out, rank, options);
  }

  bool same_or_derived_reference_source = false;
  if(target_is_reference && source_class) {
    same_or_derived_reference_source = source_class == target_class;
    if(!same_or_derived_reference_source) {
      size_t ignored_offset = 0;
      MemberAccess ignored_access = MA_PUBLIC;
      same_or_derived_reference_source =
          find_unique_base_path(*source_class, target_class, ignored_offset, ignored_access);
    }
  }
  if(same_or_derived_reference_source) {
    if(source_class == target_class &&
       semantic_conversion::try_semantic_exact_reference_binding(ctx,
                                                                 target,
                                                                 source_arg,
                                                                 out,
                                                                 rank)) {
      return true;
    }
    return false;
  }

  const string cache_key = cached_constructor_conversion_key(*target_class, source_arg);
  CachedConstructorConversionResult & cached = ctor_cache[cache_key];
  if(!cached.attempted) {
    cached.attempted = true;
    // Overload sets often ask the same target-class construction question more
    // than once with only the final reference wrapper changed, for example
    // `const T&` and `T&&`. Reuse that normalized class-construction result.
    vector<ExprInfo> ctor_source_args(1, source_arg);
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
      if(template_witness_source_capture_enabled_for_calls(ctx)) {
        ctor_options.use_location =
            prefer_later_source_location(parser_trace::current_use_location(),
                                         callsem_node_source_location_text(source_arg.node));
      }
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
      cached.available = true;
      cached.ctor_expr =
          make_unmaterialized_constructor_conversion_expr(ctx,
                                                          target_class->type,
                                                          source_arg);
    }
  }

  if(!cached.available) {
    return false;
  }

  out = cached.ctor_expr;
  ConversionRank second = semantic_conversion::standard_conversion_rank(target,
                                                                        cached.ctor_expr);
  if(second != CR_BAD) {
    rank = CR_USER_DEFINED;
    return true;
  }

  const bool inherited_ctor_ok =
      options.materialize_standard_adjustments ?
          semantic_conversion::try_apply_inheritance_conversion(ctx,
                                                                target,
                                                                cached.ctor_expr,
                                                                inherited) :
          semantic_conversion::try_apply_unmaterialized_inheritance_conversion(
              ctx,
              target,
              cached.ctor_expr,
              inherited);
  if(inherited_ctor_ok) {
    out = inherited;
    rank = CR_USER_DEFINED;
    return true;
  }

  return false;
}

bool try_memoized_argument_conversion(
    SemanticContext & ctx,
    Scope & scope,
    const TypePtr & target,
    const ExprInfo & source_arg,
    const ExprInfo * source_identity,
    ExprInfo & out,
    ConversionRank & rank,
    const ArgumentConversionOptions & options,
    unordered_map<CachedArgumentConversionKey,
                  CachedArgumentConversionResult,
                  CachedArgumentConversionKeyHash> & conversion_cache)
{
  ScopedCallSemConstructionPath construction_path("overload.memoized-conversion");
  if(source_identity) {
    CachedArgumentConversionKey cache_key;
    cache_key.target_type = target.get();
    cache_key.source_identity = source_identity;
    cache_key.option_bits = argument_conversion_option_bits(options);
    unordered_map<CachedArgumentConversionKey,
                  CachedArgumentConversionResult,
                  CachedArgumentConversionKeyHash>::iterator found =
        conversion_cache.find(cache_key);
    if(found != conversion_cache.end()) {
      if(found->second.available) {
        out = found->second.converted_expr;
        rank = found->second.rank;
      }
      return found->second.available;
    }

    CachedArgumentConversionResult cached_result;
    cached_result.attempted = true;
    const bool available =
        ctx.try_argument_conversion(scope, target, source_arg, out, rank, options);
    cached_result.available = available;
    if(available) {
      cached_result.converted_expr = out;
      cached_result.rank = rank;
    }
    conversion_cache.insert(make_pair(cache_key, cached_result));
    return available;
  }

  return ctx.try_argument_conversion(scope, target, source_arg, out, rank, options);
}

bool declarator_has_parameter_pack_fast(const CppAstNode & declarator)
{
  if(find_child(declarator, CppAstKind::parameter_pack)) {
    return true;
  }
  const CppAstNode * nested = find_child(declarator, CppAstKind::nested_declarator);
  return nested && !nested->children.empty() &&
         declarator_has_parameter_pack_fast(nested->children[0]);
}

bool constructor_template_has_trailing_parameter_pack_fast(FunctionTemplateDecl & decl)
{
  if(decl.has_trailing_function_parameter_pack || !decl.declarator) {
    return decl.has_trailing_function_parameter_pack;
  }

  const CppAstNode * parameter_clause =
      find_child(*decl.declarator, CppAstKind::parameter_clause);
  if(!parameter_clause || parameter_clause->children.empty()) {
    return false;
  }
  const CppAstNode & last = parameter_clause->children.back();
  if(last.kind != CppAstKind::parameter_declaration) {
    return false;
  }

  const CppAstNode * declarator = find_child(last, CppAstKind::declarator);
  if(declarator && declarator_has_parameter_pack_fast(*declarator)) {
    return true;
  }

  const CppAstNode * abstract = find_child(last, CppAstKind::abstract_declarator);
  return abstract && declarator_has_parameter_pack_fast(*abstract);
}

bool function_template_has_trailing_parameter_pack_fast(FunctionTemplateDecl & decl)
{
  if(decl.has_trailing_function_parameter_pack || !decl.declarator) {
    return decl.has_trailing_function_parameter_pack;
  }

  const CppAstNode * parameter_clause =
      find_child(*decl.declarator, CppAstKind::parameter_clause);
  if(!parameter_clause || parameter_clause->children.empty()) {
    return false;
  }
  const CppAstNode & last = parameter_clause->children.back();
  if(last.kind != CppAstKind::parameter_declaration) {
    return false;
  }

  const CppAstNode * declarator = find_child(last, CppAstKind::declarator);
  if(declarator && declarator_has_parameter_pack_fast(*declarator)) {
    return true;
  }

  const CppAstNode * abstract = find_child(last, CppAstKind::abstract_declarator);
  return abstract && declarator_has_parameter_pack_fast(*abstract);
}

bool constructor_template_accepts_argument_count_fast(FunctionTemplateDecl & decl,
                                                      size_t argument_count)
{
  size_t required_count = decl.params_pattern.size();
  const bool has_trailing_pack =
      decl.has_trailing_function_parameter_pack ||
      constructor_template_has_trailing_parameter_pack_fast(decl);
  if(has_trailing_pack && required_count > 0) {
    --required_count;
  }
  while(required_count > 0 &&
        required_count - 1 < decl.default_arguments_pattern.size() &&
        decl.default_arguments_pattern[required_count - 1]) {
    --required_count;
  }

  if(argument_count < required_count) {
    return false;
  }

  TypePtr function_type = strip_top_level_cv(decl.type_pattern);
  if(function_type &&
     function_type->kind == Type::TK_FUNCTION &&
     (function_type->variadic || function_type->prototype_relaxed) &&
     argument_count >= decl.params_pattern.size()) {
    return true;
  }
  if(has_trailing_pack &&
     argument_count + 1 >= decl.params_pattern.size()) {
    return true;
  }

  return argument_count <= decl.params_pattern.size();
}

bool function_template_accepts_argument_count_fast(FunctionTemplateDecl & decl,
                                                   size_t argument_count)
{
  size_t required_count = decl.params_pattern.size();
  const bool has_trailing_pack =
      decl.has_trailing_function_parameter_pack ||
      function_template_has_trailing_parameter_pack_fast(decl);
  if(has_trailing_pack && required_count > 0) {
    --required_count;
  }
  while(required_count > 0 &&
        required_count - 1 < decl.default_arguments_pattern.size() &&
        decl.default_arguments_pattern[required_count - 1]) {
    --required_count;
  }

  if(argument_count < required_count) {
    return false;
  }

  TypePtr function_type = strip_top_level_cv(decl.type_pattern);
  if(function_type &&
     function_type->kind == Type::TK_FUNCTION &&
     (function_type->variadic || function_type->prototype_relaxed) &&
     argument_count >= decl.params_pattern.size()) {
    return true;
  }

  if(has_trailing_pack &&
     argument_count + 1 >= decl.params_pattern.size()) {
    return true;
  }

  return argument_count <= decl.params_pattern.size();
}

bool constructor_template_matches_source_args_fast(SemanticContext & ctx,
                                                   FunctionTemplateDecl & decl,
                                                   const vector<ExprInfo> & source_args)
{
  if(!constructor_template_accepts_argument_count_fast(decl, source_args.size())) {
    if(parser_trace::enabled("template.resolve")) {
      ostringstream trace;
      trace << "ctor-fast-filter name=" << decl.name
            << " reason=arg-count"
            << " params=" << decl.params_pattern.size()
            << " args=" << source_args.size();
      parser_trace::note("template.resolve", std::string(), trace.str());
    }
    return false;
  }

  if(source_args.size() != 1 || decl.params_pattern.size() != 1 || !source_args[0].type) {
    return true;
  }

  TypePtr pattern_base = strip_top_level_cv(decl.params_pattern[0].second);
  if(pattern_base &&
     pattern_base->kind == Type::TK_RVALUE_REFERENCE &&
     !is_forwarding_reference_pattern(decl.parameters, decl.params_pattern[0].second) &&
     source_args[0].category == VC_LVALUE) {
    if(parser_trace::enabled("template.resolve")) {
      ostringstream trace;
      trace << "ctor-fast-filter name=" << decl.name
            << " reason=non-forwarding-rvalue"
            << " pattern=" << describe_type(decl.params_pattern[0].second)
            << " arg=" << describe_type(source_args[0].type)
            << " category=" << static_cast<int>(source_args[0].category);
      parser_trace::note("template.resolve", std::string(), trace.str());
    }
    return false;
  }

  const string pattern_template = named_template_base_name(decl.params_pattern[0].second);
  if(pattern_template.empty()) {
    return true;
  }
  if(template_base_name_is_direct_template_template_parameter(decl.parameters,
                                                             pattern_template)) {
    return true;
  }

  const string actual_template = named_template_base_name(source_args[0].type);
  if(actual_template.empty()) {
    return true;
  }

  // Constructor fast filtering must tolerate injected-class-name spellings in
  // patterns, e.g. `box<U>` inside `N::box<T>`, while the actual argument type
  // carries the fully qualified `N::box<V>` name.
  bool compatible = template_base_names_compatible(pattern_template, actual_template);
  if(!compatible) {
    set<ClassInfo *> visiting;
    compatible = type_has_template_base_named(
        ctx, source_args[0].type, pattern_template, visiting);
  }
  if(!compatible && parser_trace::enabled("template.resolve")) {
    ostringstream trace;
    trace << "ctor-fast-filter name=" << decl.name
          << " reason=template-base-mismatch"
          << " pattern=" << describe_type(decl.params_pattern[0].second)
          << " pattern-template=" << pattern_template
          << " arg=" << describe_type(source_args[0].type)
          << " arg-template=" << actual_template;
    parser_trace::note("template.resolve", std::string(), trace.str());
  }
  return compatible;
}

int compare_template_specialization_preference(const CandidateMatch & current,
                                               const CandidateMatch & best)
{
  const bool current_is_template =
      current.function && current.function->source_template != nullptr;
  const bool best_is_template =
      best.function && best.function->source_template != nullptr;
  if(current_is_template == best_is_template) {
    return 0;
  }
  return current_is_template ? 1 : -1;
}

TypePtr pointer_conversion_pointee_type(const TypePtr & type)
{
  TypePtr base = strip_top_level_cv(remove_reference_type(type));
  if(!base || base->kind != Type::TK_POINTER) {
    return TypePtr();
  }
  return strip_top_level_cv(base->inner);
}

TypePtr pointer_conversion_source_pointee_type(const ExprInfo & arg)
{
  TypePtr source = strip_top_level_cv(value_conversion_type(arg));
  if(!source || source->kind != Type::TK_POINTER) {
    return TypePtr();
  }
  TypePtr pointee = strip_top_level_cv(source->inner);
  if(pointee && pointee->kind == Type::TK_FUNCTION) {
    return TypePtr();
  }
  return pointee;
}

bool pointer_conversion_targets_base_of_source(SemanticContext & ctx,
                                               const TypePtr & target,
                                               const ExprInfo & source_arg)
{
  TypePtr target_pointee = pointer_conversion_pointee_type(target);
  TypePtr source_pointee = pointer_conversion_source_pointee_type(source_arg);
  if(!target_pointee || !source_pointee ||
     target_pointee->kind != Type::TK_NAMED ||
     source_pointee->kind != Type::TK_NAMED ||
     type_equals(target_pointee, source_pointee)) {
    return false;
  }

  ClassInfo * target_class = ctx.class_info_for_type(target_pointee);
  if(!target_class) {
    target_class = ctx.complete_class_type(target_pointee);
  }
  ClassInfo * source_class = ctx.class_info_for_type(source_pointee);
  if(!source_class) {
    source_class = ctx.complete_class_type(source_pointee);
  }
  return target_class && source_class && is_same_or_derived(source_class, target_class);
}

int compare_pointer_base_over_void_preference(SemanticContext & ctx,
                                              const TypePtr & lhs_param,
                                              const ExprInfo & lhs_source_arg,
                                              const TypePtr & rhs_param,
                                              const ExprInfo & rhs_source_arg)
{
  TypePtr lhs_pointee = pointer_conversion_pointee_type(lhs_param);
  TypePtr rhs_pointee = pointer_conversion_pointee_type(rhs_param);
  const bool lhs_void = lhs_pointee && is_void_type(lhs_pointee);
  const bool rhs_void = rhs_pointee && is_void_type(rhs_pointee);
  if(lhs_void == rhs_void) {
    return 0;
  }

  if(!lhs_void &&
     pointer_conversion_targets_base_of_source(ctx, lhs_param, lhs_source_arg)) {
    return -1;
  }
  if(!rhs_void &&
     pointer_conversion_targets_base_of_source(ctx, rhs_param, rhs_source_arg)) {
    return 1;
  }
  return 0;
}

TypePtr reference_conversion_target_object_type(const TypePtr & param)
{
  TypePtr base = strip_top_level_cv(param);
  if(!base ||
     (base->kind != Type::TK_LVALUE_REFERENCE &&
      base->kind != Type::TK_RVALUE_REFERENCE)) {
    return TypePtr();
  }
  return strip_top_level_cv(base->inner);
}

TypePtr reference_conversion_source_object_type(const ExprInfo & arg)
{
  TypePtr source = strip_top_level_cv(arg.type);
  if(source &&
     (source->kind == Type::TK_LVALUE_REFERENCE ||
      source->kind == Type::TK_RVALUE_REFERENCE)) {
    return strip_top_level_cv(source->inner);
  }
  return strip_top_level_cv(remove_reference_type(arg.type));
}

TypePtr object_conversion_target_type(const TypePtr & param)
{
  TypePtr base = strip_top_level_cv(remove_reference_type(param));
  return base && base->kind == Type::TK_NAMED ? base : TypePtr();
}

ClassInfo * complete_class_info_for_conversion_type(SemanticContext & ctx,
                                                    const TypePtr & type)
{
  TypePtr base = strip_top_level_cv(type);
  if(!base || base->kind != Type::TK_NAMED) {
    return nullptr;
  }
  ClassInfo * info = ctx.class_info_for_type(base);
  if(!info) {
    info = ctx.complete_class_type(base);
  }
  return info;
}

int compare_class_base_conversion_target_preference(SemanticContext & ctx,
                                                    const TypePtr & lhs_param,
                                                    const ExprInfo & lhs_source_arg,
                                                    const TypePtr & rhs_param,
                                                    const ExprInfo & rhs_source_arg)
{
  TypePtr lhs_target = reference_conversion_target_object_type(lhs_param);
  TypePtr rhs_target = reference_conversion_target_object_type(rhs_param);
  TypePtr lhs_source = reference_conversion_source_object_type(lhs_source_arg);
  TypePtr rhs_source = reference_conversion_source_object_type(rhs_source_arg);
  if(!lhs_target && !rhs_target) {
    lhs_target = pointer_conversion_pointee_type(lhs_param);
    rhs_target = pointer_conversion_pointee_type(rhs_param);
    lhs_source = pointer_conversion_source_pointee_type(lhs_source_arg);
    rhs_source = pointer_conversion_source_pointee_type(rhs_source_arg);
  }
  if(!lhs_target && !rhs_target) {
    lhs_target = object_conversion_target_type(lhs_param);
    rhs_target = object_conversion_target_type(rhs_param);
    lhs_source = reference_conversion_source_object_type(lhs_source_arg);
    rhs_source = reference_conversion_source_object_type(rhs_source_arg);
  }
  if(!lhs_target ||
     !rhs_target ||
     type_equals(lhs_target, rhs_target) ||
     !type_equals(lhs_source, rhs_source)) {
    return 0;
  }

  ClassInfo * lhs_target_class =
      complete_class_info_for_conversion_type(ctx, lhs_target);
  ClassInfo * rhs_target_class =
      complete_class_info_for_conversion_type(ctx, rhs_target);
  ClassInfo * source_class =
      complete_class_info_for_conversion_type(ctx, lhs_source);
  if(!lhs_target_class ||
     !rhs_target_class ||
     !source_class ||
     lhs_target_class == rhs_target_class ||
     !is_same_or_derived(source_class, lhs_target_class) ||
     !is_same_or_derived(source_class, rhs_target_class)) {
    return 0;
  }

  if(is_same_or_derived(lhs_target_class, rhs_target_class)) {
    return -1;
  }
  if(is_same_or_derived(rhs_target_class, lhs_target_class)) {
    return 1;
  }
  return 0;
}

const ExprInfo & source_arg_for_compare(const CandidateMatch & match, size_t index)
{
  if(index < match.source_args.size()) {
    return match.source_args[index];
  }
  return match.args[index];
}

bool user_defined_conversions_share_intermediate_type(const CandidateMatch & lhs,
                                                      const CandidateMatch & rhs,
                                                      size_t index)
{
  if(index >= lhs.args.size() || index >= rhs.args.size()) {
    return false;
  }
  TypePtr lhs_type = strip_top_level_cv(value_conversion_type(lhs.args[index]));
  TypePtr rhs_type = strip_top_level_cv(value_conversion_type(rhs.args[index]));
  return lhs_type && rhs_type && type_equals(lhs_type, rhs_type);
}

TypePtr implicit_object_source_pointee_type(const CandidateMatch & match)
{
  if(match.source_args.empty()) {
    return TypePtr();
  }
  TypePtr source = strip_top_level_cv(match.source_args[0].type);
  if(source && source->kind == Type::TK_POINTER) {
    return source->inner;
  }
  return TypePtr();
}

int compare_implicit_object_cv_preference(const CandidateMatch & current,
                                          const CandidateMatch & best)
{
  if(!current.function ||
     !best.function ||
     !current.function->is_method ||
     !best.function->is_method) {
    return 0;
  }

  TypePtr current_source = implicit_object_source_pointee_type(current);
  TypePtr best_source = implicit_object_source_pointee_type(best);
  TypePtr current_source_base;
  TypePtr best_source_base;
  bool current_source_const = false;
  bool current_source_volatile = false;
  bool best_source_const = false;
  bool best_source_volatile = false;
  if(!top_level_cv_flags(current_source,
                         current_source_base,
                         current_source_const,
                         current_source_volatile) ||
     !top_level_cv_flags(best_source,
                         best_source_base,
                         best_source_const,
                         best_source_volatile) ||
     current_source_const != best_source_const ||
     current_source_volatile != best_source_volatile ||
     !type_equals(strip_top_level_cv(current_source_base),
                  strip_top_level_cv(best_source_base))) {
    return 0;
  }

  int current_added_cv = 0;
  int best_added_cv = 0;
  if(!current_source_const) {
    current_added_cv += current.function->is_const_method ? 1 : 0;
    best_added_cv += best.function->is_const_method ? 1 : 0;
  }
  if(!current_source_volatile) {
    current_added_cv += current.function->is_volatile_method ? 1 : 0;
    best_added_cv += best.function->is_volatile_method ? 1 : 0;
  }
  if(current_added_cv == best_added_cv) {
    return 0;
  }
  return current_added_cv < best_added_cv ? -1 : 1;
}

int compare_implicit_object_ref_qualifier_preference(
    const CandidateMatch & current,
    const CandidateMatch & best)
{
  if(!current.function ||
     !best.function ||
     !current.function->is_method ||
     !best.function->is_method ||
     current.function->ref_qualifier == best.function->ref_qualifier) {
    return 0;
  }
  if(current.function->ref_qualifier == RQ_RVALUE &&
     best.function->ref_qualifier == RQ_LVALUE) {
    return -1;
  }
  if(current.function->ref_qualifier == RQ_LVALUE &&
     best.function->ref_qualifier == RQ_RVALUE) {
    return 1;
  }
  return 0;
}

int compare_initializer_list_element_rank_preference(
    const CandidateMatch & current,
    const CandidateMatch & best,
    size_t index)
{
  if(index >= current.list_initialization_element_ranks.size() ||
     index >= best.list_initialization_element_ranks.size()) {
    return 0;
  }
  const vector<ConversionRank> & current_ranks =
      current.list_initialization_element_ranks[index];
  const vector<ConversionRank> & best_ranks =
      best.list_initialization_element_ranks[index];
  if(current_ranks.size() != best_ranks.size()) {
    return 0;
  }

  bool current_better = false;
  bool best_better = false;
  for(size_t i = 0; i < current_ranks.size(); ++i) {
    if(current_ranks[i] < best_ranks[i]) {
      current_better = true;
    } else if(current_ranks[i] > best_ranks[i]) {
      best_better = true;
    }
  }
  if(current_better == best_better) {
    return 0;
  }
  return current_better ? -1 : 1;
}

int compare_candidate_match_preference(SemanticContext & ctx,
                                       const CandidateMatch & current,
                                       const CandidateMatch & best)
{
  const size_t rank_count = std::min(current.ranks.size(), best.ranks.size());
  const bool compare_implicit_object_last =
      current.function && best.function &&
      current.function->is_method && best.function->is_method &&
      rank_count > 0;

  const auto compare_slots =
      [&](size_t begin, size_t end, bool & current_better, bool & best_better) -> void
  {
    for(size_t j = begin; j < end; ++j) {
      const bool compare_second_standard_conversion =
          current.ranks[j] == CR_USER_DEFINED &&
          best.ranks[j] == CR_USER_DEFINED &&
          user_defined_conversions_share_intermediate_type(current, best, j);
      const ExprInfo & current_compare_arg =
          compare_second_standard_conversion ? current.args[j] :
                                               source_arg_for_compare(current, j);
      const ExprInfo & best_compare_arg =
          compare_second_standard_conversion ? best.args[j] :
                                               source_arg_for_compare(best, j);
      if(current.ranks[j] < best.ranks[j]) {
        current_better = true;
      } else if(current.ranks[j] > best.ranks[j]) {
        best_better = true;
      } else {
        int list_pref = 0;
        if(j < current.list_initialization_args.size() &&
           j < best.list_initialization_args.size() &&
           current.list_initialization_args[j] &&
           best.list_initialization_args[j]) {
          const TypePtr current_param =
              strip_top_level_cv(remove_reference_type(current.params[j]));
          const TypePtr best_param =
              strip_top_level_cv(remove_reference_type(best.params[j]));
          const bool current_initializer_list =
              ctx.is_initializer_list_type(current_param, nullptr, nullptr);
          const bool best_initializer_list =
              ctx.is_initializer_list_type(best_param, nullptr, nullptr);
          if(current_initializer_list != best_initializer_list) {
            list_pref = current_initializer_list ? -1 : 1;
          } else if(current_initializer_list) {
            list_pref = compare_initializer_list_element_rank_preference(
                current, best, j);
          }
        }
        if(list_pref < 0) {
          current_better = true;
        } else if(list_pref > 0) {
          best_better = true;
        } else {
          int ref_pref = compare_reference_binding_preference(current.params[j],
                                                              current_compare_arg,
                                                              best.params[j],
                                                              best_compare_arg);
          if(ref_pref < 0) {
            current_better = true;
          } else if(ref_pref > 0) {
            best_better = true;
          } else {
            int qual_pref = compare_qualification_conversion_preference(current.params[j],
                                                                        current_compare_arg,
                                                                        best.params[j],
                                                                        best_compare_arg);
            if(qual_pref < 0) {
              current_better = true;
            } else if(qual_pref > 0) {
              best_better = true;
            } else {
              int std_pref = compare_standard_conversion_preference(current.params[j],
                                                                    current_compare_arg,
                                                                    best.params[j],
                                                                    best_compare_arg);
              if(std_pref == 0 &&
                 (current.ranks[j] == CR_CONVERSION ||
                  compare_second_standard_conversion)) {
                std_pref = compare_class_base_conversion_target_preference(
                    ctx,
                    current.params[j],
                    current_compare_arg,
                    best.params[j],
                    best_compare_arg);
              }
              if(std_pref == 0 && j < current.args.size() && j < best.args.size()) {
                std_pref = compare_pointer_base_over_void_preference(
                    ctx,
                    current.params[j],
                    current_compare_arg,
                    best.params[j],
                    best_compare_arg);
              }
              if(std_pref < 0) {
                current_better = true;
              } else if(std_pref > 0) {
                best_better = true;
              }
            }
          }
        }
      }
    }
  };

  bool current_better = false;
  bool best_better = false;
  compare_slots(compare_implicit_object_last ? 1 : 0,
                rank_count,
                current_better,
                best_better);

  if(current_better && !best_better) {
    return -1;
  }
  if(best_better && !current_better) {
    return 1;
  }
  if(current_better && best_better) {
    return 0;
  }

  if(compare_implicit_object_last) {
    const int object_cv_pref =
        compare_implicit_object_cv_preference(current, best);
    if(object_cv_pref != 0) {
      return object_cv_pref;
    }
    compare_slots(0, 1, current_better, best_better);
    if(!current_better && !best_better) {
      const int object_ref_pref =
          compare_implicit_object_ref_qualifier_preference(current, best);
      if(object_ref_pref != 0) {
        return object_ref_pref;
      }
    }
  }

  if(current_better && !best_better) {
    return -1;
  }
  if(best_better && !current_better) {
    return 1;
  }
  if(!current_better && !best_better) {
    return compare_template_specialization_preference(current, best);
  }
  return 0;
}

BestCandidateSelection select_best_candidate_match(SemanticContext & ctx,
                                                   const vector<CandidateMatch> & matches)
{
  BestCandidateSelection selection;
  for(size_t i = 1; i < matches.size(); ++i) {
    int comparison = compare_candidate_match_preference(ctx,
                                                        matches[i],
                                                        matches[selection.index]);
    const int rank_comparison = comparison;
    if(comparison == 0) {
      comparison = compare_function_template_partial_order_preference(ctx, matches[i],
                                                                      matches[selection.index]);
    }
    if(parser_trace::enabled("overload")) {
      ostringstream trace;
      trace << "compare current=";
      append_function_candidate(trace, ctx, matches[i].function, &matches[i].ranks);
      append_binding_trace_identity(trace, ctx, matches[i].function);
      trace << " best=";
      append_function_candidate(trace, ctx, matches[selection.index].function,
                                &matches[selection.index].ranks);
      append_binding_trace_identity(trace, ctx, matches[selection.index].function);
      trace << " rank_cmp=" << rank_comparison
            << " final_cmp=" << comparison;
      parser_trace::note("overload",
                         candidate_primary_location(ctx, matches[i].function),
                         trace.str());
    }
    if(comparison < 0) {
      selection.index = i;
      selection.ambiguous = false;
    } else if(comparison == 0) {
      selection.ambiguous = true;
    }
  }
  if(parser_trace::enabled("overload") && !matches.empty()) {
    ostringstream trace;
    trace << "selected index=" << selection.index
          << " ambiguous=" << (selection.ambiguous ? "yes" : "no")
          << " winner=";
    append_function_candidate(trace, ctx, matches[selection.index].function,
                              &matches[selection.index].ranks);
    append_binding_trace_identity(trace, ctx, matches[selection.index].function);
    parser_trace::note("overload",
                       candidate_primary_location(ctx, matches[selection.index].function),
                       trace.str());
  }
  return selection;
}

void append_candidate_match(ostringstream & out,
                            SemanticContext & ctx,
                            const CandidateMatch & match,
                            bool include_params,
                            bool include_args)
{
  append_function_candidate(out, ctx, match.function, &match.ranks);
  if(include_params) {
    out << " params={";
    for(size_t i = 0; i < match.params.size(); ++i) {
      if(i != 0) {
        out << ", ";
      }
      out << describe_type(match.params[i]);
    }
    out << "}";
  }
  if(include_args) {
    out << " args={";
    for(size_t i = 0; i < match.args.size(); ++i) {
      if(i != 0) {
        out << ", ";
      }
      out << describe_type(match.args[i].type);
    }
    out << "}";
  }
}

void append_candidate_match_list(ostringstream & out,
                                 SemanticContext & ctx,
                                 const vector<CandidateMatch> & matches,
                                 bool include_params,
                                 bool include_args)
{
  for(size_t i = 0; i < matches.size(); ++i) {
    out << (i == 0 ? " " : "; ");
    append_candidate_match(out, ctx, matches[i], include_params, include_args);
  }
}

std::string candidate_match_identity(SemanticContext & ctx,
                                     const CandidateMatch & match,
                                     bool include_params,
                                     bool include_args)
{
  std::ostringstream out;
  append_candidate_match(out, ctx, match, include_params, include_args);
  if(match.function && match.function->source_template) {
    out << " source_template="
        << static_cast<const void *>(match.function->source_template);
  }
  std::string result = out.str();
  if(semantic_metrics::AnalyzerCounters * counters = performance_counters(ctx)) {
    ++counters->candidate_identity_builds;
    counters->candidate_identity_chars += result.size();
  }
  return result;
}

CallSemNode make_resolved_callee_node(SemanticContext & ctx,
                                      FunctionBinding & chosen,
                                      const vector<ExprInfo> & call_args,
                                      bool use_virtual_dispatch)
{
  const auto peel_base_subobject_root =
      [](const CallSemNode & node) -> const CallSemNode *
      {
        const CallSemNode * current = &node;
        while(current) {
          if(current->kind == CallSemKind::unary_expression &&
             current->children.size() == 1 &&
             (callsem_has_token(*current, OP_AMP) ||
              callsem_has_token(*current, OP_STAR))) {
            current = &current->children[0];
            continue;
          }
          if(current->kind == CallSemKind::member_expression &&
             current->is_base_subobject &&
             current->children.size() == 1) {
            current = &current->children[0];
            continue;
          }
          return current;
        }
        return nullptr;
      };
  const auto find_actual_subobject_offset =
      [](const ClassInfo & object_class,
         const ClassInfo * target_class,
         size_t & out_offset) -> bool
      {
        if(!target_class) {
          return false;
        }

        bool found = false;
        for(size_t i = 0; i < object_class.complete_subobjects.size(); ++i) {
          const SubobjectInfo & subobject = object_class.complete_subobjects[i];
          if(!subobject.type) {
            continue;
          }
          if(subobject.type != target_class &&
             subobject.type->qualified_name != target_class->qualified_name) {
            continue;
          }
          if(found && out_offset != subobject.offset) {
            throw logic_error("ambiguous complete subobject offset for " +
                              target_class->qualified_name);
          }
          out_offset = subobject.offset;
          found = true;
        }
        if(found) {
          return true;
        }

        MemberAccess access = MA_PUBLIC;
        return find_unique_base_path(object_class, target_class, out_offset, access);
      };
  const auto class_for_dispatch_object_type =
      [&ctx](const TypePtr & type) -> ClassInfo *
      {
        TypePtr object_type = strip_top_level_cv(remove_reference_type(type));
        if(object_type && object_type->kind == Type::TK_POINTER) {
          object_type = strip_top_level_cv(object_type->inner);
        }
        if(!object_type) {
          return nullptr;
        }
        ClassInfo * info = ctx.class_info_for_type(object_type);
        if(!info) {
          info = complete_class_type_for_lookup(ctx, object_type);
        }
        return info;
      };

  FunctionBinding * resolved =
      semantic_output::resolve_output_function_binding(ctx, &chosen);
  FunctionBinding & emitted = resolved ? *resolved : chosen;
  ScopedCallSemConstructionPath construction_path("overload.make-resolved-callee");
  CallSemNode resolved_callee = make_dump_node(CallSemKind::callee, emitted.name);
  set_callsem_resolved_name(resolved_callee, function_output_name(emitted));
  resolved_callee.semantic_type = emitted.type;
  resolved_callee.is_c_linkage = emitted.is_c_linkage;
  resolved_callee.is_semantically_nothrow = ctx.function_binding_is_nothrow(emitted);
  ClassInfo * object_class = nullptr;
  if(emitted.owner_class) {
    if(emitted.is_method &&
       !emitted.is_constructor &&
       !emitted.is_destructor &&
       !call_args.empty()) {
      const CallSemNode * root_object = peel_base_subobject_root(call_args[0].node);
      if(root_object) {
        object_class = class_for_dispatch_object_type(root_object->semantic_type);
      }
      if(!object_class) {
        object_class = class_for_dispatch_object_type(call_args[0].type);
      }
    }

    for(size_t i = 0; i < emitted.owner_class->virtual_base_subobjects.size(); ++i) {
      const SubobjectInfo & subobject = emitted.owner_class->virtual_base_subobjects[i];
      if(!subobject.type) {
        continue;
      }

      size_t actual_offset = subobject.offset;
      if(object_class) {
        size_t actual_path_offset = 0;
        if(find_actual_subobject_offset(*object_class, subobject.type, actual_path_offset)) {
          actual_offset = actual_path_offset;
        }
      }

      mutable_callsem_virtual_base_layout(resolved_callee).push_back(
          make_pair(subobject.type->qualified_name, actual_offset));
    }
  }
  set_callsem_runtime_bridge_symbol(
      resolved_callee,
      runtime_bridge_symbol_for_bound_function(
          emitted.name,
          emitted.owner_class ? emitted.owner_class->qualified_name : "",
          emitted.type));
  set_dump_symbol(resolved_callee, emitted.symbol);
  if(use_virtual_dispatch) {
    if(!emitted.has_virtual_slot) {
      throw logic_error("missing virtual slot");
    }
    resolved_callee.is_virtual_dispatch = true;
    set_callsem_uint_value(resolved_callee, emitted.virtual_slot);
    if(!object_class) {
      throw logic_error("missing virtual dispatch object class for " +
                        function_output_name(emitted));
    }
    size_t dispatch_object_offset = 0;
    ClassInfo * dispatch_object_class =
        call_args.empty() ? nullptr : class_for_dispatch_object_type(call_args[0].type);
    if(dispatch_object_class && dispatch_object_class != object_class) {
      if(!find_actual_subobject_offset(*object_class,
                                       dispatch_object_class,
                                       dispatch_object_offset)) {
        throw logic_error("missing virtual dispatch receiver path from " +
                          object_class->qualified_name + " to " +
                          dispatch_object_class->qualified_name);
      }
    }
    bool found_dispatch_view = false;
    for(size_t i = 0; i < object_class->vtables.size(); ++i) {
      const VTableInfo & table = object_class->vtables[i];
      if(emitted.virtual_slot >= table.slots.size() ||
         table.slots[emitted.virtual_slot].function != &emitted) {
        continue;
      }
      resolved_callee.has_virtual_dispatch_view_offset = true;
      set_callsem_virtual_dispatch_view_offset(
          resolved_callee,
          static_cast<long long>(table.view_offset) -
              static_cast<long long>(dispatch_object_offset));
      resolved_callee.uses_extended_vtable_layout = table.use_extended_layout;
      found_dispatch_view = true;
      break;
    }
    if(!found_dispatch_view) {
      for(size_t i = 0; i < object_class->vtables.size(); ++i) {
        const VTableInfo & table = object_class->vtables[i];
        if(table.view_offset != dispatch_object_offset ||
           emitted.virtual_slot >= table.slots.size()) {
          continue;
        }
        resolved_callee.has_virtual_dispatch_view_offset = true;
        set_callsem_virtual_dispatch_view_offset(
            resolved_callee,
            static_cast<long long>(table.view_offset) -
                static_cast<long long>(dispatch_object_offset));
        resolved_callee.uses_extended_vtable_layout = table.use_extended_layout;
        found_dispatch_view = true;
        break;
      }
    }
    if(!found_dispatch_view) {
      throw logic_error("missing virtual dispatch vtable view for " +
                        function_output_name(emitted) + " on " +
                        object_class->qualified_name);
    }
  }
  return resolved_callee;
}

ExprInfo make_call_result(SemanticContext & ctx,
                          const TypePtr & result_type,
                          ValueCategory result_category,
                          CallSemNode callee,
                          vector<ExprInfo> call_args)
{
  if(callee.is_virtual_dispatch) {
    TypePtr function_type = strip_top_level_cv(callee.semantic_type);
    if(function_type && function_type->kind == Type::TK_POINTER) {
      function_type = strip_top_level_cv(function_type->inner);
    }
    if(function_type && function_type->kind == Type::TK_FUNCTION) {
      const size_t parameter_count =
          std::min(function_type->params.size(), call_args.size());
      for(size_t i = 0; i < parameter_count; ++i) {
        CallSemVirtualBaseLayout parameter_layout;
        if(!semantic_class_model::collect_indirect_parameter_virtual_base_layout(
               ctx,
               function_type->params[i],
               parameter_layout)) {
          continue;
        }
        CallSemVirtualBaseLayout & argument_layout =
            mutable_callsem_virtual_base_layout(call_args[i].node);
        for(size_t j = 0; j < parameter_layout.size(); ++j) {
          bool found = false;
          for(size_t k = 0; k < argument_layout.size(); ++k) {
            if(argument_layout[k].first == parameter_layout[j].first) {
              found = true;
              break;
            }
          }
          if(!found) {
            argument_layout.push_back(parameter_layout[j]);
          }
        }
      }
    }
  }

  ExprInfo result;
  TypePtr expression_type = expression_type_for_function_result(result_type);
  result.type = expression_type;
  result.category = result_category;
  ScopedCallSemConstructionPath construction_path("overload.make-call-result");
  result.node = make_dump_node(CallSemKind::call_expression);
  ctx.set_expr_info_metadata(result, result.type, result.category);
  result.node.semantic_type = result_type;
  result.node.children.reserve(call_args.size() + 1);
  result.node.children.push_back(std::move(callee));
  for(size_t i = 0; i < call_args.size(); ++i) {
    result.node.children.push_back(std::move(call_args[i].node));
  }
  return result;
}

ExprInfo make_builtin_call_result(SemanticContext & ctx,
                                  const string & name,
                                  const TypePtr & result_type,
                                  const vector<TypePtr> & params,
                                  vector<ExprInfo> args)
{
  ScopedCallSemConstructionPath construction_path("overload.make-builtin-callee");
  CallSemNode resolved_callee = make_dump_node(CallSemKind::callee, name);
  resolved_callee.semantic_type = make_function(result_type, params, false);
  return make_call_result(ctx,
                          result_type,
                          VC_PRVALUE,
                          std::move(resolved_callee),
                          std::move(args));
}

bool scope_is_std_namespace_or_inline_child(const Scope * scope)
{
  const Scope * current = scope;
  while(current && current->namespace_scope && current->inline_namespace) {
    current = current->parent;
  }
  if(!current ||
     !current->namespace_scope ||
     current->name != "std") {
    return false;
  }
  for(const Scope * parent = current->parent; parent; parent = parent->parent) {
    if(parent->namespace_scope &&
       parent->name != "<global>" &&
       parent->name != "<unnamed>") {
      return false;
    }
  }
  return true;
}

const TemplateIdSyntax * standard_char_traits_qualifier(
    SemanticContext & ctx,
    Scope & scope,
    const CppAstNode & callee_node)
{
  const QualifiedName * qualified = cppast_qualified_name_syntax(callee_node);
  if(!qualified ||
     (!qualified->rooted && qualified->qualifiers.empty()) ||
     (qualified->name != "to_char_type" && qualified->name != "to_int_type")) {
    return nullptr;
  }

  const TemplateIdSyntax * match = nullptr;
  for(size_t i = 0; i < callee_node.qualifier_template_id_syntaxes.size(); ++i) {
    const TemplateIdSyntax & syntax =
        callee_node.qualifier_template_id_syntaxes[i];
    if(syntax.name.name != "char_traits" ||
       syntax.arguments.size() != 1 ||
       syntax.argument_syntaxes.size() != 1) {
      continue;
    }

    ClassTemplateDecl * decl = ctx.lookup_class_template(scope, syntax.name);
    if(!decl ||
       decl->name != syntax.name.name ||
       !scope_is_std_namespace_or_inline_child(decl->declaring_scope)) {
      continue;
    }
    match = &syntax;
  }
  return match;
}

bool try_analyze_standard_char_traits_call(SemanticContext & ctx,
                                           Scope & scope,
                                           const CppAstNode & callee_node,
                                           const vector<const CppAstNode *> & arg_nodes,
                                           ExprInfo & out)
{
  const QualifiedName * qualified = cppast_qualified_name_syntax(callee_node);
  const TemplateIdSyntax * char_traits =
      standard_char_traits_qualifier(ctx, scope, callee_node);
  if(!qualified || !char_traits || arg_nodes.size() != 1) {
    return false;
  }

  TypePtr char_type;
  if(!template_api::type::resolve_type_argument_input(
         ctx,
         scope,
         &char_traits->argument_syntaxes[0],
         true,
         char_type) ||
     !char_type) {
    return false;
  }

  TypePtr result_type =
      qualified->name == "to_char_type" ? char_type : make_fundamental(FT_INT);
  out = ctx.analyze_expression_for_target(scope, *arg_nodes[0], result_type);
  out.type = result_type;
  out.category = VC_PRVALUE;
  ctx.set_expr_info_metadata(out, out.type, out.category);
  return true;
}

ExprInfo make_integer_literal_result(SemanticContext & ctx, long long value)
{
  ExprInfo result;
  result.type = make_fundamental(FT_INT);
  result.category = VC_PRVALUE;
  ScopedCallSemConstructionPath construction_path("overload.make-integer-literal");
  result.node = make_dump_node(CallSemKind::literal, to_string(value));
  ctx.set_expr_info_metadata(result, result.type, result.category);
  if(value >= 0) {
    set_callsem_uint_value(result.node, static_cast<unsigned long long>(value));
  }
  return result;
}

ExprInfo make_size_t_literal_result(SemanticContext & ctx,
                                    unsigned long long value)
{
  ExprInfo result;
  result.type = make_fundamental(FT_UNSIGNED_LONG_INT);
  result.category = VC_PRVALUE;
  ScopedCallSemConstructionPath construction_path("overload.make-size-t-literal");
  result.node = make_dump_node(CallSemKind::literal, to_string(value));
  ctx.set_expr_info_metadata(result, result.type, result.category);
  set_callsem_uint_value(result.node, value);
  return result;
}

ExprInfo make_resolved_call_result(SemanticContext & ctx,
                                   const TypePtr & result_type,
                                   ValueCategory result_category,
                                   FunctionBinding & chosen,
                                   vector<ExprInfo> call_args,
                                   bool use_virtual_dispatch)
{
  CallSemNode resolved_callee =
      make_resolved_callee_node(ctx, chosen, call_args, use_virtual_dispatch);
  return make_call_result(ctx,
                          result_type,
                          result_category,
                          std::move(resolved_callee),
                          std::move(call_args));
}

ExprInfo require_and_make_resolved_call_result(SemanticContext & ctx,
                                               const TypePtr & result_type,
                                               ValueCategory result_category,
                                               FunctionBinding & chosen,
                                               vector<ExprInfo> call_args,
                                               bool use_virtual_dispatch,
                                               bool mark_output_required = true)
{
  CallableEmissionDecision emission =
      ctx.decide_callable_emission(&chosen,
                                   OutputReason::DirectCall,
                                   mark_output_required);
  ctx.require_function_definition(&chosen,
                                  OutputReason::DirectCall,
                                  emission.mark_output_required_now);
  return make_resolved_call_result(ctx,
                                   result_type,
                                   result_category,
                                   chosen,
                                   std::move(call_args),
                                   use_virtual_dispatch);
}

void analyze_atomic_pointer_arg(SemanticContext & ctx,
                                Scope & scope,
                                const string & builtin_name,
                                const CppAstNode & arg,
                                ExprInfo & ptr_expr,
                                TypePtr & value_type)
{
  ptr_expr = ctx.analyze_expression(scope, arg);
  TypePtr ptr_type = strip_top_level_cv(remove_reference_type(ptr_expr.type));
  if(!ptr_type || ptr_type->kind != Type::TK_POINTER) {
    throw logic_error(builtin_name + " requires pointer argument");
  }
  value_type = strip_top_level_cv(ptr_type->inner);
  if(value_type && value_type->kind == Type::TK_ATOMIC) {
    value_type = strip_top_level_cv(value_type->inner);
  }
  if(!value_type) {
    throw logic_error(builtin_name + " requires pointed value type");
  }
}

void analyze_atomic_value_pointer_arg(SemanticContext & ctx,
                                      Scope & scope,
                                      const string & builtin_name,
                                      const CppAstNode & arg,
                                      const TypePtr & value_type,
                                      ExprInfo & ptr_expr)
{
  ptr_expr = ctx.analyze_expression(scope, arg);
  TypePtr ptr_type = strip_top_level_cv(remove_reference_type(ptr_expr.type));
  if(!ptr_type || ptr_type->kind != Type::TK_POINTER) {
    throw logic_error(builtin_name + " requires value pointer argument");
  }
  TypePtr pointed_type = strip_top_level_cv(ptr_type->inner);
  if(!pointed_type || !type_equals(pointed_type, strip_top_level_cv(value_type))) {
    throw logic_error(builtin_name + " requires compatible pointed value type");
  }
}

ExprInfo analyze_atomic_delta_arg(SemanticContext & ctx,
                                  Scope & scope,
                                  const TypePtr & value_type,
                                  const CppAstNode & arg)
{
  TypePtr value_base = strip_top_level_cv(value_type);
  if(value_base && value_base->kind == Type::TK_POINTER) {
    return ctx.analyze_expression_for_target(scope, arg,
                                             make_fundamental(FT_LONG_LONG_INT));
  }
  return ctx.analyze_expression_for_target(scope, arg, value_type);
}

ExprInfo analyze_atomic_order_arg(SemanticContext & ctx,
                                  Scope & scope,
                                  const CppAstNode & arg)
{
  return ctx.analyze_expression_for_target(scope, arg, make_fundamental(FT_INT));
}

ExprInfo analyze_atomic_query_call(SemanticContext & ctx,
                                   Scope & scope,
                                   const string & builtin_name,
                                   const vector<const CppAstNode *> & arg_nodes)
{
  const bool c11_query = builtin_name == "__c11_atomic_is_lock_free";
  if(arg_nodes.size() != (c11_query ? 1u : 2u)) {
    throw logic_error(builtin_name + " arity");
  }
  ExprInfo size_arg =
      ctx.analyze_expression_for_target(scope, *arg_nodes[0],
                                        make_fundamental(FT_UNSIGNED_LONG_INT));
  if(c11_query) {
    return make_builtin_call_result(ctx,
                                    builtin_name,
                                    make_fundamental(FT_BOOL),
                                    vector<TypePtr>(1, make_fundamental(FT_UNSIGNED_LONG_INT)),
                                    vector<ExprInfo>(1, size_arg));
  }
  ExprInfo ptr_arg = ctx.analyze_expression(scope, *arg_nodes[1]);
  return make_builtin_call_result(ctx,
                                  builtin_name,
                                  make_fundamental(FT_BOOL),
                                  vector<TypePtr>{
                                      make_fundamental(FT_UNSIGNED_LONG_INT),
                                      ptr_arg.type
                                  },
                                  vector<ExprInfo>{size_arg, ptr_arg});
}

ExprInfo analyze_builtin_clzg_call(SemanticContext & ctx,
                                   Scope & scope,
                                   const string & builtin_name,
                                   const vector<const CppAstNode *> & arg_nodes)
{
  if(arg_nodes.empty() || arg_nodes.size() > 2) {
    throw logic_error(builtin_name + " arity");
  }

  ExprInfo value_arg = ctx.analyze_expression(scope, *arg_nodes[0]);
  TypePtr value_type = strip_top_level_cv(remove_reference_type(value_arg.type));
  if(!value_type ||
     (!is_integral_type(value_type) &&
      !ctx.type_depends_on_template_parameter(value_type))) {
    throw logic_error(builtin_name + " requires integral first argument");
  }

  vector<TypePtr> params;
  vector<ExprInfo> args;
  params.push_back(value_type);
  args.push_back(value_arg);

  if(arg_nodes.size() == 2) {
    ExprInfo fallback_arg =
        ctx.analyze_expression_for_target(scope, *arg_nodes[1], make_fundamental(FT_INT));
    params.push_back(fallback_arg.type);
    args.push_back(fallback_arg);
  }

  return make_builtin_call_result(ctx,
                                  builtin_name,
                                  make_fundamental(FT_INT),
                                  params,
                                  std::move(args));
}

ExprInfo analyze_builtin_ctzg_call(SemanticContext & ctx,
                                   Scope & scope,
                                   const string & builtin_name,
                                   const vector<const CppAstNode *> & arg_nodes)
{
  return analyze_builtin_clzg_call(ctx, scope, builtin_name, arg_nodes);
}

ExprInfo analyze_builtin_popcountg_call(SemanticContext & ctx,
                                        Scope & scope,
                                        const string & builtin_name,
                                        const vector<const CppAstNode *> & arg_nodes)
{
  if(arg_nodes.size() != 1) {
    throw logic_error(builtin_name + " arity");
  }

  ExprInfo value_arg = ctx.analyze_expression(scope, *arg_nodes[0]);
  TypePtr value_type = strip_top_level_cv(remove_reference_type(value_arg.type));
  if(!value_type ||
     (!is_integral_type(value_type) &&
      !ctx.type_depends_on_template_parameter(value_type))) {
    throw logic_error(builtin_name + " requires integral argument");
  }

  return make_builtin_call_result(ctx,
                                  builtin_name,
                                  make_fundamental(FT_INT),
                                  vector<TypePtr>{value_type},
                                  vector<ExprInfo>{value_arg});
}

ExprInfo analyze_builtin_fixed_unsigned_call(SemanticContext & ctx,
                                             Scope & scope,
                                             const string & builtin_name,
                                             const vector<const CppAstNode *> & arg_nodes,
                                             const TypePtr & param_type)
{
  if(arg_nodes.size() != 1) {
    throw logic_error(builtin_name + " arity");
  }

  ExprInfo value_arg = ctx.analyze_expression_for_target(scope, *arg_nodes[0], param_type);
  return make_builtin_call_result(ctx,
                                  builtin_name,
                                  make_fundamental(FT_INT),
                                  vector<TypePtr>(1, param_type),
                                  vector<ExprInfo>(1, value_arg));
}

bool try_analyze_builtin_call_expression(SemanticContext & ctx,
                                         Scope & scope,
                                         const string & builtin_name,
                                         const vector<const CppAstNode *> & arg_nodes,
                                         const CallAnalysisOptions & options,
                                         ExprInfo & out)
{
  if(builtin_name.size() < 2 || builtin_name[0] != '_' || builtin_name[1] != '_') {
    return false;
  }

  if(builtin_name == "__builtin_convertvector") {
    if(arg_nodes.size() != 2) {
      throw logic_error("__builtin_convertvector arity");
    }
    TypePtr target_type;
    if(!ctx.try_parse_builtin_type_trait_call_arg(scope,
                                                  *arg_nodes[1],
                                                  target_type) ||
       !target_type) {
      throw logic_error("__builtin_convertvector requires target type");
    }
    out = ctx.analyze_expression_for_target(scope, *arg_nodes[0], target_type);
    return true;
  }

  if(builtin_name == "__builtin_bit_cast") {
    if(arg_nodes.size() != 2) {
      throw logic_error("__builtin_bit_cast arity");
    }
    TypePtr target_type;
    if(!ctx.try_parse_builtin_type_trait_call_arg(scope,
                                                  *arg_nodes[0],
                                                  target_type) ||
       !target_type) {
      throw logic_error("__builtin_bit_cast requires target type");
    }
    ExprInfo source = ctx.analyze_expression(scope, *arg_nodes[1]);
    TypePtr source_type = value_conversion_type(source);
    TypePtr target_base = strip_top_level_cv(remove_reference_type(target_type));
    if(!source_type ||
       !target_base ||
       !is_integral_type(source_type) ||
       !is_integral_type(target_base) ||
       type_size(source_type) != type_size(target_base)) {
      throw logic_error("unsupported __builtin_bit_cast representation");
    }
    // GNU vector attributes are scalarized to their one-lane element type.
    // Preserve the bits of that supported integral representation instead of
    // accepting an ordinary value conversion for unrelated bit-cast forms.
    out.type = target_type;
    out.category = VC_PRVALUE;
    out.node = make_dump_node(CallSemKind::cast_expression,
                              "__builtin_bit_cast");
    set_dump_token(out.node, *arg_nodes[1]);
    callsemantic_internal::set_expr_metadata(out.node,
                                             out.type,
                                             out.category);
    out.node.children.push_back(std::move(source.node));
    return true;
  }

  if(builtin_name == "__builtin_reduce_and" ||
     builtin_name == "__builtin_reduce_or") {
    if(arg_nodes.size() != 1) {
      throw logic_error(builtin_name + " arity");
    }
    out = ctx.analyze_expression_for_target(scope,
                                            *arg_nodes[0],
                                            make_fundamental(FT_BOOL));
    return true;
  }

  if(builtin_name == "__builtin_invoke") {
    if(arg_nodes.empty()) {
      throw logic_error("__builtin_invoke arity");
    }

    ExprInfo callable = ctx.analyze_expression(scope, *arg_nodes[0]);
    TypePtr callable_type = strip_top_level_cv(remove_reference_type(callable.type));
    if(callable_type &&
       callable_type->kind == Type::TK_MEMBER_POINTER) {
      if(arg_nodes.size() < 2) {
        throw logic_error("__builtin_invoke member pointer arity");
      }

      ExprInfo object = ctx.analyze_expression(scope, *arg_nodes[1]);
      TypePtr object_type = value_conversion_type(object);
      TypePtr object_base = strip_top_level_cv(object_type);
      const bool use_arrow =
          object_base &&
          object_base->kind == Type::TK_POINTER;

      TypePtr member_owner = strip_top_level_cv(callable_type->owner);
      TypePtr direct_object_type =
          strip_top_level_cv(remove_reference_type(object.type));
      bool use_direct_object =
          member_owner &&
          direct_object_type &&
          same_type_with_compatible_top_cv(member_owner, direct_object_type);
      if(!use_arrow && !use_direct_object && member_owner) {
        ExprInfo converted_object;
        use_direct_object =
            try_apply_inheritance_conversion(ctx,
                                             member_owner,
                                             object,
                                             converted_object);
      }

      CppAstNode member_object = *arg_nodes[1];
      if(!use_arrow && !use_direct_object) {
        CppAstNode dereference;
        dereference.kind = CppAstKind::unary_expression;
        dereference.value = "*";
        dereference.has_token = true;
        dereference.token_kind = RT_SIMPLE;
        dereference.simple_type = OP_STAR;
        dereference.children.push_back(*arg_nodes[1]);
        member_object = dereference;
      }

      CppAstNode member_access;
      member_access.kind = CppAstKind::binary_expression;
      member_access.value = use_arrow ? "->*" : ".*";
      member_access.has_token = true;
      member_access.token_kind = RT_SIMPLE;
      member_access.simple_type = use_arrow ? OP_ARROWSTAR : OP_DOTSTAR;
      member_access.children.push_back(member_object);
      member_access.children.push_back(*arg_nodes[0]);

      if(is_function_type(callable_type->inner)) {
        CppAstNode rewritten_call;
        rewritten_call.kind = CppAstKind::call_expression;
        rewritten_call.children.push_back(member_access);

        CppAstNode rewritten_args;
        rewritten_args.kind = CppAstKind::paren_argument_list;
        for(size_t i = 2; i < arg_nodes.size(); ++i) {
          rewritten_args.children.push_back(*arg_nodes[i]);
        }
        rewritten_call.children.push_back(rewritten_args);

        out = analyze_call_expression(ctx, scope, rewritten_call, options);
        return true;
      }

      if(arg_nodes.size() != 2) {
        throw logic_error("__builtin_invoke data member arity");
      }
      out = ctx.analyze_expression(scope, member_access);
      return true;
    }

    CppAstNode rewritten_call;
    rewritten_call.kind = CppAstKind::call_expression;
    rewritten_call.children.push_back(*arg_nodes[0]);

    CppAstNode rewritten_args;
    rewritten_args.kind = CppAstKind::paren_argument_list;
    for(size_t i = 1; i < arg_nodes.size(); ++i) {
      rewritten_args.children.push_back(*arg_nodes[i]);
    }
    rewritten_call.children.push_back(rewritten_args);

    out = analyze_call_expression(ctx, scope, rewritten_call, options);
    return true;
  }

  if(builtin_name == "__builtin_complex") {
    if(arg_nodes.size() != 2) {
      throw logic_error("__builtin_complex arity");
    }

    ExprInfo real_arg = ctx.analyze_expression(scope, *arg_nodes[0]);
    TypePtr component_type = value_conversion_type(real_arg);
    TypePtr complex_type = semantic_builtins::gnu_complex_type_for_component(component_type);
    if(!complex_type) {
      throw logic_error("__builtin_complex requires floating first argument");
    }

    ExprInfo imag_arg = ctx.analyze_expression_for_target(scope, *arg_nodes[1], component_type);
    if(!type_equals(value_conversion_type(imag_arg), component_type)) {
      throw logic_error("__builtin_complex requires matching floating arguments");
    }

    out = make_builtin_call_result(ctx,
                                   builtin_name,
                                   complex_type,
                                   vector<TypePtr>{component_type, component_type},
                                   vector<ExprInfo>{real_arg, imag_arg});
    return true;
  }

  if(builtin_name == "__builtin_constant_p") {
    if(arg_nodes.size() != 1) {
      throw logic_error("__builtin_constant_p arity");
    }
    long long ignored = 0;
    out = make_integer_literal_result(ctx,
                                      ctx.evaluate_constant_expression(scope,
                                                                       *arg_nodes[0],
                                                                       ignored)
                                          ? 1
                                          : 0);
    return true;
  }
  if(builtin_name == "__builtin_addressof") {
    if(arg_nodes.size() != 1) {
      throw logic_error("__builtin_addressof arity");
    }
    ExprInfo arg = ctx.analyze_expression(scope, *arg_nodes[0]);
    if(arg.category != VC_LVALUE) {
      throw logic_error("__builtin_addressof requires lvalue");
    }
    ScopedCallSemConstructionPath construction_path("overload.builtin-addressof");
    out = ctx.make_address_of_expr(arg);
    return true;
  }
  if(builtin_name == "__builtin_offsetof") {
    if(arg_nodes.size() != 2) {
      throw logic_error("__builtin_offsetof arity");
    }

    TypePtr object_type;
    if(!ctx.try_parse_builtin_type_trait_call_arg(scope, *arg_nodes[0], object_type) ||
       !object_type) {
      throw logic_error("__builtin_offsetof requires object type");
    }
    if(ctx.type_depends_on_template_parameter(object_type)) {
      return false;
    }

    const CppAstNode & member = *arg_nodes[1];
    if(member.kind != CppAstKind::id_expression) {
      throw logic_error("__builtin_offsetof requires direct member name");
    }

    semantic_consteval::OffsetofFieldInfo field;
    if(!ctx.lookup_offsetof_field(object_type, member.value, field) ||
       !field.found) {
      throw logic_error("__builtin_offsetof unknown field " + member.value);
    }
    if(field.is_bit_field) {
      throw logic_error("__builtin_offsetof on bit-field unsupported");
    }

    out = make_size_t_literal_result(
        ctx,
        static_cast<unsigned long long>(field.offset));
    return true;
  }
  if(builtin_name == "__builtin_va_start") {
    if(arg_nodes.size() != 2) {
      throw logic_error("__builtin_va_start arity");
    }
    ExprInfo list_arg = ctx.analyze_expression(scope, *arg_nodes[0]);
    if(list_arg.category != VC_LVALUE ||
       !semantic_builtins::is_builtin_va_list_type(list_arg.type)) {
      throw logic_error("__builtin_va_start requires __builtin_va_list lvalue");
    }
    ScopedCallSemConstructionPath construction_path("overload.builtin-va-address");
    ExprInfo list_ptr = ctx.make_address_of_expr(list_arg);
    ExprInfo last_arg = ctx.analyze_expression(scope, *arg_nodes[1]);
    out = make_builtin_call_result(ctx,
                                   builtin_name,
                                   make_fundamental(FT_VOID),
                                   vector<TypePtr>{list_ptr.type, last_arg.type},
                                   vector<ExprInfo>{list_ptr, last_arg});
    return true;
  }
  if(builtin_name == "__builtin_va_end") {
    if(arg_nodes.size() != 1) {
      throw logic_error("__builtin_va_end arity");
    }
    ExprInfo list_arg = ctx.analyze_expression(scope, *arg_nodes[0]);
    if(list_arg.category != VC_LVALUE ||
       !semantic_builtins::is_builtin_va_list_type(list_arg.type)) {
      throw logic_error("__builtin_va_end requires __builtin_va_list lvalue");
    }
    ScopedCallSemConstructionPath construction_path("overload.builtin-va-address");
    ExprInfo list_ptr = ctx.make_address_of_expr(list_arg);
    out = make_builtin_call_result(ctx,
                                   builtin_name,
                                   make_fundamental(FT_VOID),
                                   vector<TypePtr>{list_ptr.type},
                                   vector<ExprInfo>{list_ptr});
    return true;
  }
  if(builtin_name == "__builtin_va_arg") {
    if(arg_nodes.size() != 2 || arg_nodes[1]->kind != CppAstKind::type_id) {
      throw logic_error("__builtin_va_arg arity");
    }
    ExprInfo list_arg = ctx.analyze_expression(scope, *arg_nodes[0]);
    if(list_arg.category != VC_LVALUE ||
       !semantic_builtins::is_builtin_va_list_type(list_arg.type)) {
      throw logic_error("__builtin_va_arg requires __builtin_va_list lvalue");
    }
    TypePtr result_type;
    if(!ctx.parse_type_id(scope, *arg_nodes[1], result_type) || !result_type) {
      throw logic_error("__builtin_va_arg requires type-id");
    }
    ScopedCallSemConstructionPath construction_path("overload.builtin-va-address");
    ExprInfo list_ptr = ctx.make_address_of_expr(list_arg);
    out = make_builtin_call_result(ctx,
                                   builtin_name,
                                   result_type,
                                   vector<TypePtr>{list_ptr.type},
                                   vector<ExprInfo>{list_ptr});
    return true;
  }
  if(builtin_name == "__builtin_assume_aligned") {
    if(arg_nodes.size() != 2 && arg_nodes.size() != 3) {
      throw logic_error("__builtin_assume_aligned arity");
    }
    ExprInfo ptr_arg = ctx.analyze_expression(scope, *arg_nodes[0]);
    TypePtr ptr_type = value_conversion_type(ptr_arg);
    TypePtr ptr_base = strip_top_level_cv(remove_reference_type(ptr_type));
    if(!ptr_base || ptr_base->kind != Type::TK_POINTER) {
      throw logic_error("__builtin_assume_aligned requires pointer argument");
    }
    TypePtr size_type = make_fundamental(FT_UNSIGNED_LONG_INT);
    ExprInfo align_arg = ctx.analyze_expression_for_target(scope, *arg_nodes[1], size_type);
    vector<TypePtr> params;
    vector<ExprInfo> args;
    params.push_back(ptr_type);
    args.push_back(ptr_arg);
    params.push_back(align_arg.type);
    args.push_back(align_arg);
    if(arg_nodes.size() == 3) {
      ExprInfo offset_arg =
          ctx.analyze_expression_for_target(scope, *arg_nodes[2], size_type);
      params.push_back(offset_arg.type);
      args.push_back(offset_arg);
    }
    out = make_builtin_call_result(ctx,
                                   builtin_name,
                                   ptr_type,
                                   params,
                                   std::move(args));
    return true;
  }
  if(builtin_name == "__builtin_clzg") {
    out = analyze_builtin_clzg_call(ctx, scope, builtin_name, arg_nodes);
    return true;
  }
  if(builtin_name == "__builtin_ctzg") {
    out = analyze_builtin_ctzg_call(ctx, scope, builtin_name, arg_nodes);
    return true;
  }
  if(builtin_name == "__builtin_popcountg") {
    out = analyze_builtin_popcountg_call(ctx, scope, builtin_name, arg_nodes);
    return true;
  }
  if(builtin_name == "__builtin_clz") {
    out = analyze_builtin_fixed_unsigned_call(ctx,
                                              scope,
                                              builtin_name,
                                              arg_nodes,
                                              make_fundamental(FT_UNSIGNED_INT));
    return true;
  }
  if(builtin_name == "__builtin_clzl") {
    out = analyze_builtin_fixed_unsigned_call(ctx,
                                              scope,
                                              builtin_name,
                                              arg_nodes,
                                              make_fundamental(FT_UNSIGNED_LONG_INT));
    return true;
  }
  if(builtin_name == "__builtin_clzll") {
    out = analyze_builtin_fixed_unsigned_call(ctx,
                                              scope,
                                              builtin_name,
                                              arg_nodes,
                                              make_fundamental(FT_UNSIGNED_LONG_LONG_INT));
    return true;
  }
  if(builtin_name == "__builtin_ctz") {
    out = analyze_builtin_fixed_unsigned_call(ctx,
                                              scope,
                                              builtin_name,
                                              arg_nodes,
                                              make_fundamental(FT_UNSIGNED_INT));
    return true;
  }
  if(builtin_name == "__builtin_ctzl") {
    out = analyze_builtin_fixed_unsigned_call(ctx,
                                              scope,
                                              builtin_name,
                                              arg_nodes,
                                              make_fundamental(FT_UNSIGNED_LONG_INT));
    return true;
  }
  if(builtin_name == "__builtin_ctzll") {
    out = analyze_builtin_fixed_unsigned_call(ctx,
                                              scope,
                                              builtin_name,
                                              arg_nodes,
                                              make_fundamental(FT_UNSIGNED_LONG_LONG_INT));
    return true;
  }
  if(builtin_name == "__builtin_popcount") {
    out = analyze_builtin_fixed_unsigned_call(ctx,
                                              scope,
                                              builtin_name,
                                              arg_nodes,
                                              make_fundamental(FT_UNSIGNED_INT));
    return true;
  }
  if(builtin_name == "__builtin_popcountl") {
    out = analyze_builtin_fixed_unsigned_call(ctx,
                                              scope,
                                              builtin_name,
                                              arg_nodes,
                                              make_fundamental(FT_UNSIGNED_LONG_INT));
    return true;
  }
  if(builtin_name == "__builtin_popcountll") {
    out = analyze_builtin_fixed_unsigned_call(ctx,
                                              scope,
                                              builtin_name,
                                              arg_nodes,
                                              make_fundamental(FT_UNSIGNED_LONG_LONG_INT));
    return true;
  }
  if(builtin_name == "__atomic_load_n") {
    if(arg_nodes.size() != 2) {
      throw logic_error("__atomic_load_n arity");
    }
    ExprInfo ptr_arg;
    TypePtr value_type;
    analyze_atomic_pointer_arg(ctx, scope, builtin_name, *arg_nodes[0], ptr_arg, value_type);
    ExprInfo order_arg = analyze_atomic_order_arg(ctx, scope, *arg_nodes[1]);
    out = make_builtin_call_result(ctx,
                                   builtin_name,
                                   value_type,
                                   vector<TypePtr>{ptr_arg.type, order_arg.type},
                                   vector<ExprInfo>{ptr_arg, order_arg});
    return true;
  }
  if(builtin_name == "__atomic_store_n") {
    if(arg_nodes.size() != 3) {
      throw logic_error("__atomic_store_n arity");
    }
    ExprInfo ptr_arg;
    TypePtr value_type;
    analyze_atomic_pointer_arg(ctx, scope, builtin_name, *arg_nodes[0], ptr_arg, value_type);
    ExprInfo value_arg = ctx.analyze_expression_for_target(scope, *arg_nodes[1], value_type);
    ExprInfo order_arg = analyze_atomic_order_arg(ctx, scope, *arg_nodes[2]);
    out = make_builtin_call_result(ctx,
                                   builtin_name,
                                   make_fundamental(FT_VOID),
                                   vector<TypePtr>{ptr_arg.type, value_arg.type, order_arg.type},
                                   vector<ExprInfo>{ptr_arg, value_arg, order_arg});
    return true;
  }
  if(builtin_name == "__atomic_exchange_n") {
    if(arg_nodes.size() != 3) {
      throw logic_error("__atomic_exchange_n arity");
    }
    ExprInfo ptr_arg;
    TypePtr value_type;
    analyze_atomic_pointer_arg(ctx, scope, builtin_name, *arg_nodes[0], ptr_arg, value_type);
    ExprInfo value_arg = ctx.analyze_expression_for_target(scope, *arg_nodes[1], value_type);
    ExprInfo order_arg = analyze_atomic_order_arg(ctx, scope, *arg_nodes[2]);
    out = make_builtin_call_result(ctx,
                                   builtin_name,
                                   value_type,
                                   vector<TypePtr>{ptr_arg.type, value_arg.type, order_arg.type},
                                   vector<ExprInfo>{ptr_arg, value_arg, order_arg});
    return true;
  }
  if(builtin_name == "__atomic_compare_exchange_n") {
    if(arg_nodes.size() != 6) {
      throw logic_error("__atomic_compare_exchange_n arity");
    }
    ExprInfo ptr_arg;
    TypePtr value_type;
    analyze_atomic_pointer_arg(ctx, scope, builtin_name, *arg_nodes[0], ptr_arg, value_type);
    ExprInfo expected_arg;
    analyze_atomic_value_pointer_arg(ctx,
                                     scope,
                                     builtin_name,
                                     *arg_nodes[1],
                                     value_type,
                                     expected_arg);
    ExprInfo desired_arg = ctx.analyze_expression_for_target(scope, *arg_nodes[2], value_type);
    ExprInfo weak_arg =
        ctx.analyze_expression_for_target(scope, *arg_nodes[3], make_fundamental(FT_BOOL));
    ExprInfo success_arg = analyze_atomic_order_arg(ctx, scope, *arg_nodes[4]);
    ExprInfo failure_arg = analyze_atomic_order_arg(ctx, scope, *arg_nodes[5]);
    out = make_builtin_call_result(ctx,
                                   builtin_name,
                                   make_fundamental(FT_BOOL),
                                   vector<TypePtr>{ptr_arg.type,
                                                   expected_arg.type,
                                                   desired_arg.type,
                                                   weak_arg.type,
                                                   success_arg.type,
                                                   failure_arg.type},
                                   vector<ExprInfo>{ptr_arg,
                                                    expected_arg,
                                                    desired_arg,
                                                    weak_arg,
                                                    success_arg,
                                                    failure_arg});
    return true;
  }
  if(builtin_name == "__sync_synchronize") {
    if(!arg_nodes.empty()) {
      throw logic_error("__sync_synchronize arity");
    }
    out = make_builtin_call_result(ctx,
                                   builtin_name,
                                   make_fundamental(FT_VOID),
                                   vector<TypePtr>(),
                                   vector<ExprInfo>());
    return true;
  }
  if(builtin_name == "__sync_fetch_and_add") {
    if(arg_nodes.size() != 2) {
      throw logic_error("__sync_fetch_and_add arity");
    }
    ExprInfo ptr_arg;
    TypePtr value_type;
    analyze_atomic_pointer_arg(ctx, scope, builtin_name, *arg_nodes[0], ptr_arg, value_type);
    ExprInfo delta_arg = analyze_atomic_delta_arg(ctx, scope, value_type, *arg_nodes[1]);
    out = make_builtin_call_result(ctx,
                                   builtin_name,
                                   value_type,
                                   vector<TypePtr>{ptr_arg.type, delta_arg.type},
                                   vector<ExprInfo>{ptr_arg, delta_arg});
    return true;
  }
  if(builtin_name == "__sync_val_compare_and_swap") {
    if(arg_nodes.size() != 3) {
      throw logic_error("__sync_val_compare_and_swap arity");
    }
    ExprInfo ptr_arg;
    TypePtr value_type;
    analyze_atomic_pointer_arg(ctx, scope, builtin_name, *arg_nodes[0], ptr_arg, value_type);
    ExprInfo compare_arg = ctx.analyze_expression_for_target(scope, *arg_nodes[1], value_type);
    ExprInfo value_arg = ctx.analyze_expression_for_target(scope, *arg_nodes[2], value_type);
    out = make_builtin_call_result(ctx,
                                   builtin_name,
                                   value_type,
                                   vector<TypePtr>{ptr_arg.type, compare_arg.type, value_arg.type},
                                   vector<ExprInfo>{ptr_arg, compare_arg, value_arg});
    return true;
  }
  if(builtin_name == "__sync_lock_test_and_set") {
    if(arg_nodes.size() != 2) {
      throw logic_error("__sync_lock_test_and_set arity");
    }
    ExprInfo ptr_arg;
    TypePtr value_type;
    analyze_atomic_pointer_arg(ctx, scope, builtin_name, *arg_nodes[0], ptr_arg, value_type);
    ExprInfo value_arg = ctx.analyze_expression_for_target(scope, *arg_nodes[1], value_type);
    out = make_builtin_call_result(ctx,
                                   builtin_name,
                                   value_type,
                                   vector<TypePtr>{ptr_arg.type, value_arg.type},
                                   vector<ExprInfo>{ptr_arg, value_arg});
    return true;
  }
  if(builtin_name == "__sync_lock_release") {
    if(arg_nodes.size() != 1) {
      throw logic_error("__sync_lock_release arity");
    }
    ExprInfo ptr_arg;
    TypePtr value_type;
    analyze_atomic_pointer_arg(ctx, scope, builtin_name, *arg_nodes[0], ptr_arg, value_type);
    out = make_builtin_call_result(ctx,
                                   builtin_name,
                                   make_fundamental(FT_VOID),
                                   vector<TypePtr>{ptr_arg.type},
                                   vector<ExprInfo>{ptr_arg});
    return true;
  }
  if(builtin_name == "__c11_atomic_init") {
    if(arg_nodes.size() != 2) {
      throw logic_error("__c11_atomic_init arity");
    }
    ExprInfo ptr_arg;
    TypePtr value_type;
    analyze_atomic_pointer_arg(ctx, scope, builtin_name, *arg_nodes[0], ptr_arg, value_type);
    ExprInfo value_arg = ctx.analyze_expression_for_target(scope, *arg_nodes[1], value_type);
    out = make_builtin_call_result(ctx,
                                   builtin_name,
                                   make_fundamental(FT_VOID),
                                   vector<TypePtr>{ptr_arg.type, value_arg.type},
                                   vector<ExprInfo>{ptr_arg, value_arg});
    return true;
  }
  if(builtin_name == "__c11_atomic_load") {
    if(arg_nodes.size() != 2) {
      throw logic_error("__c11_atomic_load arity");
    }
    ExprInfo ptr_arg;
    TypePtr value_type;
    analyze_atomic_pointer_arg(ctx, scope, builtin_name, *arg_nodes[0], ptr_arg, value_type);
    ExprInfo order_arg = analyze_atomic_order_arg(ctx, scope, *arg_nodes[1]);
    out = make_builtin_call_result(ctx,
                                   builtin_name,
                                   value_type,
                                   vector<TypePtr>{ptr_arg.type, order_arg.type},
                                   vector<ExprInfo>{ptr_arg, order_arg});
    return true;
  }
  if(builtin_name == "__c11_atomic_store") {
    if(arg_nodes.size() != 3) {
      throw logic_error("__c11_atomic_store arity");
    }
    ExprInfo ptr_arg;
    TypePtr value_type;
    analyze_atomic_pointer_arg(ctx, scope, builtin_name, *arg_nodes[0], ptr_arg, value_type);
    ExprInfo value_arg = ctx.analyze_expression_for_target(scope, *arg_nodes[1], value_type);
    ExprInfo order_arg = analyze_atomic_order_arg(ctx, scope, *arg_nodes[2]);
    out = make_builtin_call_result(ctx,
                                   builtin_name,
                                   make_fundamental(FT_VOID),
                                   vector<TypePtr>{ptr_arg.type, value_arg.type, order_arg.type},
                                   vector<ExprInfo>{ptr_arg, value_arg, order_arg});
    return true;
  }
  if(builtin_name == "__c11_atomic_exchange") {
    if(arg_nodes.size() != 3) {
      throw logic_error("__c11_atomic_exchange arity");
    }
    ExprInfo ptr_arg;
    TypePtr value_type;
    analyze_atomic_pointer_arg(ctx, scope, builtin_name, *arg_nodes[0], ptr_arg, value_type);
    ExprInfo value_arg = ctx.analyze_expression_for_target(scope, *arg_nodes[1], value_type);
    ExprInfo order_arg = analyze_atomic_order_arg(ctx, scope, *arg_nodes[2]);
    out = make_builtin_call_result(ctx,
                                   builtin_name,
                                   value_type,
                                   vector<TypePtr>{ptr_arg.type, value_arg.type, order_arg.type},
                                   vector<ExprInfo>{ptr_arg, value_arg, order_arg});
    return true;
  }
  if(builtin_name == "__c11_atomic_compare_exchange_strong" ||
     builtin_name == "__c11_atomic_compare_exchange_weak") {
    if(arg_nodes.size() != 5) {
      throw logic_error(builtin_name + " arity");
    }
    ExprInfo ptr_arg;
    TypePtr value_type;
    analyze_atomic_pointer_arg(ctx, scope, builtin_name, *arg_nodes[0], ptr_arg, value_type);
    ExprInfo expected_arg;
    analyze_atomic_value_pointer_arg(ctx,
                                     scope,
                                     builtin_name,
                                     *arg_nodes[1],
                                     value_type,
                                     expected_arg);
    ExprInfo desired_arg = ctx.analyze_expression_for_target(scope, *arg_nodes[2], value_type);
    ExprInfo success_arg = analyze_atomic_order_arg(ctx, scope, *arg_nodes[3]);
    ExprInfo failure_arg = analyze_atomic_order_arg(ctx, scope, *arg_nodes[4]);
    out = make_builtin_call_result(ctx,
                                   builtin_name,
                                   make_fundamental(FT_BOOL),
                                   vector<TypePtr>{ptr_arg.type,
                                                   expected_arg.type,
                                                   desired_arg.type,
                                                   success_arg.type,
                                                   failure_arg.type},
                                   vector<ExprInfo>{ptr_arg,
                                                    expected_arg,
                                                    desired_arg,
                                                    success_arg,
                                                    failure_arg});
    return true;
  }
  if(builtin_name == "__atomic_load") {
    if(arg_nodes.size() != 3) {
      throw logic_error("__atomic_load arity");
    }
    ExprInfo ptr_arg;
    TypePtr value_type;
    analyze_atomic_pointer_arg(ctx, scope, builtin_name, *arg_nodes[0], ptr_arg, value_type);
    ExprInfo dst_arg;
    analyze_atomic_value_pointer_arg(ctx,
                                     scope,
                                     builtin_name,
                                     *arg_nodes[1],
                                     value_type,
                                     dst_arg);
    ExprInfo order_arg = analyze_atomic_order_arg(ctx, scope, *arg_nodes[2]);
    out = make_builtin_call_result(ctx,
                                   builtin_name,
                                   make_fundamental(FT_VOID),
                                   vector<TypePtr>{ptr_arg.type, dst_arg.type, order_arg.type},
                                   vector<ExprInfo>{ptr_arg, dst_arg, order_arg});
    return true;
  }
  if(builtin_name == "__atomic_store") {
    if(arg_nodes.size() != 3) {
      throw logic_error("__atomic_store arity");
    }
    ExprInfo ptr_arg;
    TypePtr value_type;
    analyze_atomic_pointer_arg(ctx, scope, builtin_name, *arg_nodes[0], ptr_arg, value_type);
    ExprInfo src_arg;
    analyze_atomic_value_pointer_arg(ctx,
                                     scope,
                                     builtin_name,
                                     *arg_nodes[1],
                                     value_type,
                                     src_arg);
    ExprInfo order_arg = analyze_atomic_order_arg(ctx, scope, *arg_nodes[2]);
    out = make_builtin_call_result(ctx,
                                   builtin_name,
                                   make_fundamental(FT_VOID),
                                   vector<TypePtr>{ptr_arg.type, src_arg.type, order_arg.type},
                                   vector<ExprInfo>{ptr_arg, src_arg, order_arg});
    return true;
  }
  if(builtin_name == "__atomic_test_and_set") {
    if(arg_nodes.size() != 2) {
      throw logic_error("__atomic_test_and_set arity");
    }
    ExprInfo ptr_arg;
    TypePtr value_type;
    analyze_atomic_pointer_arg(ctx, scope, builtin_name, *arg_nodes[0], ptr_arg, value_type);
    ExprInfo order_arg = analyze_atomic_order_arg(ctx, scope, *arg_nodes[1]);
    out = make_builtin_call_result(ctx,
                                   builtin_name,
                                   make_fundamental(FT_BOOL),
                                   vector<TypePtr>{ptr_arg.type, order_arg.type},
                                   vector<ExprInfo>{ptr_arg, order_arg});
    return true;
  }
  if(builtin_name == "__atomic_clear") {
    if(arg_nodes.size() != 2) {
      throw logic_error("__atomic_clear arity");
    }
    ExprInfo ptr_arg;
    TypePtr value_type;
    analyze_atomic_pointer_arg(ctx, scope, builtin_name, *arg_nodes[0], ptr_arg, value_type);
    ExprInfo order_arg = analyze_atomic_order_arg(ctx, scope, *arg_nodes[1]);
    out = make_builtin_call_result(ctx,
                                   builtin_name,
                                   make_fundamental(FT_VOID),
                                   vector<TypePtr>{ptr_arg.type, order_arg.type},
                                   vector<ExprInfo>{ptr_arg, order_arg});
    return true;
  }
  if(builtin_name == "__atomic_add_fetch") {
    if(arg_nodes.size() != 3) {
      throw logic_error("__atomic_add_fetch arity");
    }
    ExprInfo ptr_arg;
    TypePtr value_type;
    analyze_atomic_pointer_arg(ctx, scope, builtin_name, *arg_nodes[0], ptr_arg, value_type);
    ExprInfo delta_arg = analyze_atomic_delta_arg(ctx, scope, value_type, *arg_nodes[1]);
    ExprInfo order_arg = analyze_atomic_order_arg(ctx, scope, *arg_nodes[2]);
    out = make_builtin_call_result(ctx,
                                   builtin_name,
                                   value_type,
                                   vector<TypePtr>{ptr_arg.type, delta_arg.type, order_arg.type},
                                   vector<ExprInfo>{ptr_arg, delta_arg, order_arg});
    return true;
  }
  if(builtin_name == "__atomic_fetch_add" ||
     builtin_name == "__atomic_fetch_sub" ||
     builtin_name == "__c11_atomic_fetch_add" ||
     builtin_name == "__c11_atomic_fetch_sub") {
    if(arg_nodes.size() != 3) {
      throw logic_error(builtin_name + " arity");
    }
    ExprInfo ptr_arg;
    TypePtr value_type;
    analyze_atomic_pointer_arg(ctx, scope, builtin_name, *arg_nodes[0], ptr_arg, value_type);
    ExprInfo delta_arg = analyze_atomic_delta_arg(ctx, scope, value_type, *arg_nodes[1]);
    ExprInfo order_arg = analyze_atomic_order_arg(ctx, scope, *arg_nodes[2]);
    out = make_builtin_call_result(ctx,
                                   builtin_name,
                                   value_type,
                                   vector<TypePtr>{ptr_arg.type, delta_arg.type, order_arg.type},
                                   vector<ExprInfo>{ptr_arg, delta_arg, order_arg});
    return true;
  }
  if(builtin_name == "__c11_atomic_fetch_and" ||
     builtin_name == "__c11_atomic_fetch_or" ||
     builtin_name == "__c11_atomic_fetch_xor" ||
     builtin_name == "__atomic_fetch_and" ||
     builtin_name == "__atomic_fetch_or" ||
     builtin_name == "__atomic_fetch_xor") {
    if(arg_nodes.size() != 3) {
      throw logic_error(builtin_name + " arity");
    }
    ExprInfo ptr_arg;
    TypePtr value_type;
    analyze_atomic_pointer_arg(ctx, scope, builtin_name, *arg_nodes[0], ptr_arg, value_type);
    ExprInfo pattern_arg = ctx.analyze_expression_for_target(scope, *arg_nodes[1], value_type);
    ExprInfo order_arg = analyze_atomic_order_arg(ctx, scope, *arg_nodes[2]);
    out = make_builtin_call_result(ctx,
                                   builtin_name,
                                   value_type,
                                   vector<TypePtr>{ptr_arg.type, pattern_arg.type, order_arg.type},
                                   vector<ExprInfo>{ptr_arg, pattern_arg, order_arg});
    return true;
  }
  if(builtin_name == "__atomic_thread_fence" ||
     builtin_name == "__atomic_signal_fence" ||
     builtin_name == "__c11_atomic_thread_fence" ||
     builtin_name == "__c11_atomic_signal_fence") {
    if(arg_nodes.size() != 1) {
      throw logic_error(builtin_name + " arity");
    }
    ExprInfo order_arg = analyze_atomic_order_arg(ctx, scope, *arg_nodes[0]);
    out = make_builtin_call_result(ctx,
                                   builtin_name,
                                   make_fundamental(FT_VOID),
                                   vector<TypePtr>(1, order_arg.type),
                                   vector<ExprInfo>(1, order_arg));
    return true;
  }
  if(builtin_name == "__atomic_is_lock_free" ||
     builtin_name == "__atomic_always_lock_free" ||
     builtin_name == "__c11_atomic_is_lock_free") {
    out = analyze_atomic_query_call(ctx, scope, builtin_name, arg_nodes);
    return true;
  }
  return false;
}

ExprInfo finalize_functional_cast_result(SemanticContext & ctx,
                                         Scope & scope,
                                         const TypePtr & callee_type,
                                         ExprInfo result,
                                         const CppAstNode & arg_node)
{
  ExprInfo operand = result;
  if(!semantic_conversion::can_copy_initialize(ctx, callee_type, result)) {
    ExprInfo converted;
    ConversionRank rank = CR_BAD;
    if(ctx.try_argument_conversion(scope,
                                   callee_type,
                                   result,
                                   converted,
                                   rank,
                                   semantic_policy::allow_explicit_argument_conversion())) {
      result = converted;
      operand = result;
    } else if(semantic_conversion::supports_non_reference_explicit_cast(
           ctx, callee_type, result, true)) {
      result.type = callee_type;
      result.category = VC_PRVALUE;
    } else {
      ostringstream out;
      out << "invalid functional cast";
      out << " [target " << describe_type(callee_type) << "]";
      out << " [arg " << node_text(arg_node) << "]";
      out << " [arg_type " << describe_type(result.type) << "]";
      out << " [arg_category "
          << call_value_category_text(to_call_value_category(result.category)) << "]";
      throw NoViableConstructorError(out.str());
    }
  } else {
    result.type = callee_type;
    if(!semantic_conversion::result_value_category_for_function_result(callee_type,
                                                                       result.category)) {
      result.category = VC_PRVALUE;
    }
  }

  TypePtr target_base = strip_top_level_cv(remove_reference_type(callee_type));
  const bool use_explicit_lowir_cast =
      !is_void_type(callee_type) &&
      !is_reference_type(callee_type) &&
      target_base &&
      target_base->kind != Type::TK_ARRAY &&
      !is_function_type(target_base) &&
      (is_integral_or_unscoped_enum_type(target_base) ||
       is_floating_type(target_base) ||
       is_pointer_type(target_base));
  if(use_explicit_lowir_cast) {
    ExprInfo cast_result;
    cast_result.type = result.type;
    cast_result.category = result.category;
    cast_result.node = make_dump_node(CallSemKind::cast_expression);
    ctx.set_expr_info_metadata(cast_result, cast_result.type, cast_result.category);
    cast_result.node.children.push_back(operand.node);
    set_callsem_materialization_source_type(
        cast_result.node,
        callsem_materialization_source_type(operand.node)
            ? callsem_materialization_source_type(operand.node)
            : (callsem_conversion_source_type(operand.node) ?
                   callsem_conversion_source_type(operand.node) :
                   operand.type));
    set_callsem_conversion_source_type(cast_result.node, operand.type);
    return cast_result;
  }

  ctx.set_expr_info_metadata(result, result.type, result.category);
  return result;
}

vector<const CppAstNode *> expand_functional_braced_init_elements(
    SemanticContext & ctx,
    Scope & scope,
    const CppAstNode & node,
    vector<unique_ptr<CppAstNode> > & storage)
{
  vector<const CppAstNode *> out;
  if(node.kind != CppAstKind::braced_init_list) {
    return out;
  }
  for(size_t i = 0; i < node.children.size(); ++i) {
    const CppAstNode & child = node.children[i];
    if(child.kind != CppAstKind::pack_expansion_expression) {
      out.push_back(&child);
      continue;
    }
    vector<CppAstNode> expanded_nodes;
    if(!ctx.expand_pack_argument_node(scope, child, expanded_nodes)) {
      throw logic_error("unsupported array functional-cast pack-expansion element");
    }
    for(size_t j = 0; j < expanded_nodes.size(); ++j) {
      storage.emplace_back(new CppAstNode(expanded_nodes[j]));
      out.push_back(storage.back().get());
    }
  }
  return out;
}

bool node_is_designated_initializer(const CppAstNode & node)
{
  return node.kind == CppAstKind::designated_initializer;
}

ExprInfo analyze_array_functional_braced_init(SemanticContext & ctx,
                                              Scope & scope,
                                              const TypePtr & callee_type,
                                              const CppAstNode & direct_braced_init)
{
  TypePtr array_type = strip_top_level_cv(remove_reference_type(callee_type));
  if(!array_type || array_type->kind != Type::TK_ARRAY || !array_type->has_bound) {
    throw logic_error("unsupported array functional cast target");
  }

  vector<unique_ptr<CppAstNode> > expanded_storage;
  vector<const CppAstNode *> elements =
      expand_functional_braced_init_elements(ctx,
                                             scope,
                                             direct_braced_init,
                                             expanded_storage);
  if(elements.size() > array_type->bound) {
    throw logic_error("too many array functional-cast initializer elements");
  }

  ExprInfo result;
  result.type = callee_type;
  result.category = VC_PRVALUE;
  result.node = make_dump_node(CallSemKind::braced_init_list);
  ctx.set_expr_info_metadata(result, result.type, result.category);

  for(size_t i = 0; i < array_type->bound; ++i) {
    ExprInfo element;
    if(i < elements.size()) {
      if(node_is_designated_initializer(*elements[i])) {
        throw logic_error("designated array functional-cast initializer unsupported");
      }
      element = ctx.analyze_expression_for_target(scope, *elements[i], array_type->inner);
      if(!semantic_conversion::can_copy_initialize(ctx, array_type->inner, element)) {
        throw logic_error("invalid array functional-cast initializer element");
      }
    } else {
      TypePtr element_base = strip_top_level_cv(remove_reference_type(array_type->inner));
      if(element_base &&
         (element_base->kind == Type::TK_ARRAY ||
          ctx.complete_class_type(element_base))) {
        CppAstNode empty;
        empty.kind = CppAstKind::braced_init_list;
        element = ctx.analyze_expression_for_target(scope, empty, array_type->inner);
      } else {
        element = ctx.make_value_initialized_expr(array_type->inner);
      }
    }
    result.node.children.push_back(std::move(element.node));
  }
  return result;
}

ExprInfo analyze_functional_cast_impl(SemanticContext & ctx,
                                      Scope & scope,
                                      const TypePtr & callee_type,
                                      const vector<const CppAstNode *> & arg_nodes,
                                      const CppAstNode * direct_braced_init,
                                      const CallAnalysisOptions & options)
{
  TypePtr stripped_callee = strip_top_level_cv(callee_type);
  const bool reference_target =
      stripped_callee &&
      (stripped_callee->kind == Type::TK_LVALUE_REFERENCE ||
       stripped_callee->kind == Type::TK_RVALUE_REFERENCE);
  TypePtr target_base = strip_top_level_cv(remove_reference_type(callee_type));
  ClassInfo * class_info = nullptr;
  if(!reference_target) {
    class_info = ctx.class_info_for_type(target_base);
    if(!class_info) {
      class_info = complete_class_type_for_lookup(ctx, target_base);
    }
  }
  if(direct_braced_init &&
     class_info &&
     ctx.is_initializer_list_type(target_base, nullptr, nullptr)) {
    return semantic_expression::make_initializer_list_expression(ctx,
                                                                 scope,
                                                                 callee_type,
                                                                 *direct_braced_init);
  }
  if(class_info) {
    const ConstructorSelectionOptions ctor_options =
        constructor_lifecycle_service::selection_options_for(
            constructor_lifecycle_service::direct_initialization_profile(
                "functional cast"));
    constructor_lifecycle_service::ConstructorSelectionResult ctor;
    if(direct_braced_init) {
      constructor_lifecycle_service::select_constructor_for_direct_braced_init_into(
          ctx,
          scope,
          *class_info,
          *direct_braced_init,
          ctor,
          ctor_options);
    } else {
      constructor_lifecycle_service::select_constructor_into(ctx,
                                                             scope,
                                                             *class_info,
                                                             arg_nodes,
                                                             ctor,
                                                             ctor_options);
    }
    constructor_lifecycle_service::ConstructorActionResult ctor_action;
    constructor_lifecycle_service::prepare_lifecycle_call_into(
        ctx,
        ctor.ctor,
        ctor.converted_args,
        false,
        OutputReason::ConstructorUse,
        ctor_action,
        options.instantiate_bodies);
    ExprInfo result =
        make_resolved_call_result(ctx,
                                  callee_type,
                                  VC_PRVALUE,
                                  *ctor_action.ctor,
                                  std::move(ctor_action.call_args),
                                  false);
    if((direct_braced_init ? direct_braced_init->children.empty() : arg_nodes.empty()) &&
       constructor_lifecycle_service::value_initialization_requires_zero_init(
           *ctor_action.ctor)) {
      result.node.value_initializes_result = true;
    }
    return result;
  }

  if(direct_braced_init) {
    ExprInfo array_result;
    if(target_base && target_base->kind == Type::TK_ARRAY) {
      if(semantic_expression::try_analyze_array_braced_init_list_expression(
             ctx,
             scope,
             callee_type,
             *direct_braced_init,
             array_result)) {
        return array_result;
      }
      throw logic_error("invalid array braced-init-list");
    }
    vector<unique_ptr<CppAstNode> > expanded_storage;
    vector<const CppAstNode *> elements =
        expand_functional_braced_init_elements(ctx,
                                               scope,
                                               *direct_braced_init,
                                               expanded_storage);
    if(elements.empty()) {
      return ctx.make_value_initialized_expr(callee_type);
    }
    TypePtr target_array = strip_top_level_cv(remove_reference_type(callee_type));
    if(target_array &&
       target_array->kind == Type::TK_ARRAY &&
       target_array->has_bound) {
      return analyze_array_functional_braced_init(ctx,
                                                  scope,
                                                  callee_type,
                                                  *direct_braced_init);
    }
    if(elements.size() != 1) {
      throw logic_error("non-class braced-init-list requires one element");
    }
    return finalize_functional_cast_result(
        ctx,
        scope,
        callee_type,
        ctx.analyze_expression_for_target(scope, *elements[0], callee_type),
        *elements[0]);
  }
  if(arg_nodes.empty()) {
    return ctx.make_value_initialized_expr(callee_type);
  }
  if(arg_nodes.size() != 1) {
    throw logic_error("unsupported functional cast arity");
  }
  return finalize_functional_cast_result(
      ctx,
      scope,
      callee_type,
      ctx.analyze_expression_for_target(scope, *arg_nodes[0], callee_type),
      *arg_nodes[0]);
}

int compare_function_template_reference_pattern_preference(
    const CandidateMatch & current,
    const CandidateMatch & best)
{
  if(!current.function || !best.function ||
     !current.function->source_template || !best.function->source_template) {
    return 0;
  }

  FunctionTemplateDecl & current_template = *current.function->source_template;
  FunctionTemplateDecl & best_template = *best.function->source_template;
  const size_t current_arg_offset =
      current.function->is_method &&
      !current_template.is_static_member &&
      current.call_args.size() > current_template.params_pattern.size() ?
          1u :
          0u;
  const size_t best_arg_offset =
      best.function->is_method &&
      !best_template.is_static_member &&
      best.call_args.size() > best_template.params_pattern.size() ?
          1u :
          0u;
  const size_t common_arg_count =
      std::min(current_template.params_pattern.size(),
               best_template.params_pattern.size());
  if(common_arg_count == 0) {
    return 0;
  }

  bool current_better = false;
  bool best_better = false;
  for(size_t i = 0; i < common_arg_count; ++i) {
    const size_t current_arg_index = i + current_arg_offset;
    const size_t best_arg_index = i + best_arg_offset;
    if(current_arg_index >= current.call_args.size() ||
       best_arg_index >= best.call_args.size()) {
      continue;
    }
    if(current.call_args[current_arg_index].category != VC_LVALUE ||
       best.call_args[best_arg_index].category != VC_LVALUE) {
      continue;
    }

    TemplateReferencePatternKind current_kind =
        classify_template_reference_pattern(current_template, i);
    TemplateReferencePatternKind best_kind =
        classify_template_reference_pattern(best_template, i);
    if(current_kind == TRPK_LVALUE_REFERENCE &&
       best_kind == TRPK_FORWARDING_REFERENCE) {
      current_better = true;
    } else if(current_kind == TRPK_FORWARDING_REFERENCE &&
              best_kind == TRPK_LVALUE_REFERENCE) {
      best_better = true;
    }
    if(current_kind != best_kind) {
      continue;
    }

    const ExprInfo & actual =
        current_arg_index < current.source_args.size() ?
            current.source_args[current_arg_index] :
            current.call_args[current_arg_index];
    const int current_cv_score =
        reference_pattern_cv_match_score(current_template.params_pattern[i].second,
                                         actual.type);
    const int best_cv_score =
        reference_pattern_cv_match_score(best_template.params_pattern[i].second,
                                         actual.type);
    if(current_cv_score >= 0 &&
       best_cv_score >= 0 &&
       current_cv_score != best_cv_score) {
      if(current_cv_score > best_cv_score) {
        current_better = true;
      } else {
        best_better = true;
      }
    }
  }

  if(current_better && !best_better) {
    return -1;
  }
  if(best_better && !current_better) {
    return 1;
  }
  return 0;
}

bool array_element_cv_flags_for_template_ordering(const TypePtr & type,
                                                  bool & cv_const,
                                                  bool & cv_volatile)
{
  TypePtr base = strip_top_level_cv(type);
  if(base &&
     (base->kind == Type::TK_LVALUE_REFERENCE ||
      base->kind == Type::TK_RVALUE_REFERENCE)) {
    base = strip_top_level_cv(base->inner);
  }
  if(!base || base->kind != Type::TK_ARRAY || !base->inner) {
    return false;
  }

  TypePtr element_base;
  return top_level_cv_flags(base->inner, element_base, cv_const, cv_volatile);
}

int array_element_cv_match_score(const TypePtr & pattern,
                                 const TypePtr & actual)
{
  bool pattern_const = false;
  bool pattern_volatile = false;
  bool actual_const = false;
  bool actual_volatile = false;
  if(!array_element_cv_flags_for_template_ordering(pattern,
                                                   pattern_const,
                                                   pattern_volatile) ||
     !array_element_cv_flags_for_template_ordering(actual,
                                                   actual_const,
                                                   actual_volatile)) {
    return -1;
  }

  int score = 0;
  if(pattern_const == actual_const) {
    ++score;
  }
  if(pattern_volatile == actual_volatile) {
    ++score;
  }
  return score;
}

int compare_function_template_array_element_cv_preference(
    const CandidateMatch & current,
    const CandidateMatch & best)
{
  if(!current.function || !best.function ||
     !current.function->source_template || !best.function->source_template) {
    return 0;
  }

  FunctionTemplateDecl & current_template = *current.function->source_template;
  FunctionTemplateDecl & best_template = *best.function->source_template;
  if(current.call_args.size() != current_template.params_pattern.size() ||
     best.call_args.size() != best_template.params_pattern.size() ||
     current.call_args.size() != best.call_args.size()) {
    return 0;
  }

  bool current_better = false;
  bool best_better = false;
  for(size_t i = 0; i < current.call_args.size(); ++i) {
    const ExprInfo & actual =
        i < current.source_args.size() ? current.source_args[i] : current.call_args[i];
    const int current_score =
        array_element_cv_match_score(current_template.params_pattern[i].second,
                                     actual.type);
    const int best_score =
        array_element_cv_match_score(best_template.params_pattern[i].second,
                                     actual.type);
    if(current_score < 0 || best_score < 0 || current_score == best_score) {
      continue;
    }
    if(current_score > best_score) {
      current_better = true;
    } else {
      best_better = true;
    }
  }

  if(current_better && !best_better) {
    return -1;
  }
  if(best_better && !current_better) {
    return 1;
  }
  return 0;
}

bool class_template_arg_cv_match_score(const TypePtr & pattern,
                                       const TypePtr & actual,
                                       int & score)
{
  score = 0;

  TypePtr pattern_base = strip_top_level_cv(pattern);
  if(pattern_base &&
     (pattern_base->kind == Type::TK_LVALUE_REFERENCE ||
      pattern_base->kind == Type::TK_RVALUE_REFERENCE)) {
    pattern_base = strip_top_level_cv(pattern_base->inner);
  }
  TypePtr actual_base = strip_top_level_cv(actual);
  if(actual_base &&
     (actual_base->kind == Type::TK_LVALUE_REFERENCE ||
      actual_base->kind == Type::TK_RVALUE_REFERENCE)) {
    actual_base = strip_top_level_cv(actual_base->inner);
  }

  shared_ptr<const ClassTemplateSpecializationMangleInfo> pattern_info =
      named_type_class_template_specialization_mangle_info_const(pattern_base);
  shared_ptr<const ClassTemplateSpecializationMangleInfo> actual_info =
      named_type_class_template_specialization_mangle_info_const(actual_base);
  if(!pattern_info || !actual_info ||
     !pattern_info->class_template_decl ||
     pattern_info->class_template_decl != actual_info->class_template_decl ||
     pattern_info->arguments.size() != actual_info->arguments.size()) {
    return false;
  }

  bool considered_type_arg = false;
  for(size_t i = 0; i < pattern_info->arguments.size(); ++i) {
    const TemplateArgument & pattern_arg = pattern_info->arguments[i];
    const TemplateArgument & actual_arg = actual_info->arguments[i];
    if(pattern_arg.kind != TemplateArgument::TA_TYPE ||
       actual_arg.kind != TemplateArgument::TA_TYPE ||
       !pattern_arg.type ||
       !actual_arg.type) {
      continue;
    }

    TypePtr pattern_inner;
    TypePtr actual_inner;
    bool pattern_const = false;
    bool pattern_volatile = false;
    bool actual_const = false;
    bool actual_volatile = false;
    if(!top_level_cv_flags(pattern_arg.type,
                           pattern_inner,
                           pattern_const,
                           pattern_volatile) ||
       !top_level_cv_flags(actual_arg.type,
                           actual_inner,
                           actual_const,
                           actual_volatile)) {
      continue;
    }
    considered_type_arg = true;
    if(pattern_const == actual_const) {
      ++score;
    }
    if(pattern_volatile == actual_volatile) {
      ++score;
    }
  }

  return considered_type_arg;
}

int compare_function_template_class_template_arg_cv_preference(
    const CandidateMatch & current,
    const CandidateMatch & best)
{
  if(!current.function || !best.function ||
     !current.function->source_template || !best.function->source_template) {
    return 0;
  }

  FunctionTemplateDecl & current_template = *current.function->source_template;
  FunctionTemplateDecl & best_template = *best.function->source_template;
  if(current.call_args.size() != current_template.params_pattern.size() ||
     best.call_args.size() != best_template.params_pattern.size() ||
     current.call_args.size() != best.call_args.size()) {
    return 0;
  }

  bool current_better = false;
  bool best_better = false;
  for(size_t i = 0; i < current.call_args.size(); ++i) {
    const ExprInfo & actual =
        i < current.source_args.size() ? current.source_args[i] : current.call_args[i];
    int current_score = 0;
    int best_score = 0;
    if(!class_template_arg_cv_match_score(current_template.params_pattern[i].second,
                                          actual.type,
                                          current_score) ||
       !class_template_arg_cv_match_score(best_template.params_pattern[i].second,
                                          actual.type,
                                          best_score) ||
       current_score == best_score) {
      continue;
    }
    if(current_score > best_score) {
      current_better = true;
    } else {
      best_better = true;
    }
  }

  if(current_better && !best_better) {
    return -1;
  }
  if(best_better && !current_better) {
    return 1;
  }
  return 0;
}

const TemplateParameterInfo * direct_type_template_parameter_pack_reference(
    const vector<TemplateParameterInfo> & parameters,
    const TypePtr & pattern)
{
  TypePtr base = strip_top_level_cv(pattern);
  if(base &&
     (base->kind == Type::TK_LVALUE_REFERENCE ||
      base->kind == Type::TK_RVALUE_REFERENCE)) {
    base = strip_top_level_cv(base->inner);
  }
  if(!base || base->kind != Type::TK_NAMED) {
    return nullptr;
  }

  const TemplateParameterInfo * parameter =
      find_template_parameter(parameters, base);
  return parameter &&
                 parameter->kind == TemplateParameterInfo::TP_TYPE &&
                 parameter->parameter_pack ?
             parameter :
             nullptr;
}

bool function_template_has_generic_trailing_type_pack(FunctionTemplateDecl & decl)
{
  const bool has_trailing_pack =
      decl.has_trailing_function_parameter_pack ||
      function_template_has_trailing_parameter_pack_fast(decl);
  if(!has_trailing_pack || decl.params_pattern.empty()) {
    return false;
  }
  return direct_type_template_parameter_pack_reference(
             decl.parameters, decl.params_pattern.back().second) != nullptr;
}

size_t explicit_function_arg_count_for_template_match(
    const CandidateMatch & match,
    const FunctionTemplateDecl & decl)
{
  size_t count =
      match.explicit_arg_count == static_cast<size_t>(-1) ?
          match.params.size() :
          match.explicit_arg_count;
  if(match.function &&
     match.function->is_method &&
     !decl.is_static_member &&
     count > decl.params_pattern.size()) {
    --count;
  }
  return count;
}

int compare_function_template_trailing_pack_preference(const CandidateMatch & current,
                                                       const CandidateMatch & best)
{
  if(!current.function || !best.function ||
     !current.function->source_template || !best.function->source_template) {
    return 0;
  }

  FunctionTemplateDecl & current_template = *current.function->source_template;
  FunctionTemplateDecl & best_template = *best.function->source_template;
  const bool current_has_trailing_pack =
      current_template.has_trailing_function_parameter_pack ||
      function_template_has_trailing_parameter_pack_fast(current_template);
  const bool best_has_trailing_pack =
      best_template.has_trailing_function_parameter_pack ||
      function_template_has_trailing_parameter_pack_fast(best_template);
  if(!current_has_trailing_pack && !best_has_trailing_pack) {
    return 0;
  }
  const bool current_generic_pack =
      function_template_has_generic_trailing_type_pack(current_template);
  const bool best_generic_pack =
      function_template_has_generic_trailing_type_pack(best_template);
  if(!current_generic_pack && !best_generic_pack) {
    return 0;
  }

  const size_t current_fixed =
      std::min(current_template.params_pattern.size() -
                   (current_has_trailing_pack ? 1u : 0u),
               explicit_function_arg_count_for_template_match(current,
                                                              current_template));
  const size_t best_fixed =
      std::min(best_template.params_pattern.size() -
                   (best_has_trailing_pack ? 1u : 0u),
               explicit_function_arg_count_for_template_match(best,
                                                              best_template));
  if(current_fixed > best_fixed) {
    return -1;
  }
  if(best_fixed > current_fixed) {
    return 1;
  }
  return 0;
}

int compare_function_template_empty_trailing_pack_preference(
    const CandidateMatch & current,
    const CandidateMatch & best)
{
  if(!current.function || !best.function ||
     !current.function->source_template || !best.function->source_template) {
    return 0;
  }

  FunctionTemplateDecl & current_template = *current.function->source_template;
  FunctionTemplateDecl & best_template = *best.function->source_template;
  const bool current_has_trailing_pack =
      current_template.has_trailing_function_parameter_pack ||
      function_template_has_trailing_parameter_pack_fast(current_template);
  const bool best_has_trailing_pack =
      best_template.has_trailing_function_parameter_pack ||
      function_template_has_trailing_parameter_pack_fast(best_template);
  if(current_has_trailing_pack == best_has_trailing_pack) {
    return 0;
  }

  FunctionTemplateDecl & pack_template =
      current_has_trailing_pack ? current_template : best_template;
  const CandidateMatch & pack_match = current_has_trailing_pack ? current : best;
  if(!function_template_has_generic_trailing_type_pack(pack_template)) {
    return 0;
  }

  const size_t pack_fixed = pack_template.params_pattern.size() - 1;
  if(explicit_function_arg_count_for_template_match(pack_match, pack_template) !=
     pack_fixed) {
    return 0;
  }

  FunctionTemplateDecl & fixed_template =
      current_has_trailing_pack ? best_template : current_template;
  const CandidateMatch & fixed_match = current_has_trailing_pack ? best : current;
  const size_t fixed_count =
      std::min(fixed_template.params_pattern.size(),
               explicit_function_arg_count_for_template_match(fixed_match,
                                                              fixed_template));
  if(fixed_count != pack_fixed) {
    return 0;
  }
  return current_has_trailing_pack ? 1 : -1;
}

bool transformed_function_template_parameter_types_for_match(
    FunctionTemplateDecl & decl,
    const CandidateMatch & match,
    vector<TypePtr> & out)
{
  if(!semantic_template_function::transformed_function_template_parameter_types(decl, out)) {
    return false;
  }
  const size_t effective_count =
      match.explicit_arg_count == static_cast<size_t>(-1) ?
          match.params.size() :
          match.explicit_arg_count;
  if(!decl.has_trailing_function_parameter_pack) {
    if(match.explicit_arg_count != static_cast<size_t>(-1)) {
      if(effective_count > out.size()) {
        return false;
      }
      out.resize(effective_count);
    }
    return true;
  }

  const size_t fixed_count = decl.params_pattern.empty() ? 0 : decl.params_pattern.size() - 1;
  if(effective_count < fixed_count || out.size() != decl.params_pattern.size()) {
    return false;
  }

  TypePtr transformed_pack_param = out.empty() ? TypePtr() : out.back();
  out.resize(fixed_count);
  while(out.size() < effective_count) {
    out.push_back(transformed_pack_param);
  }
  return true;
}

bool scope_has_direct_callable_name(Scope & current,
                                    const std::string & name)
{
  if(!lookup_direct_functions(current, name).empty() ||
     !lookup_direct_function_templates(current, name).empty()) {
    return true;
  }

  const bool has_lexical_class =
      !current.class_info && current.function &&
      current.function->lexical_access_class;
  ClassInfo * lexical_class = current.class_info;
  if(!lexical_class && has_lexical_class) {
    lexical_class = current.function->lexical_access_class;
  }
  if(!lexical_class) {
    return false;
  }

  if(lexical_class->member_scope) {
    const vector<FunctionBinding *> * direct_functions =
        find_direct_function_set(*lexical_class->member_scope, name);
    if(direct_functions && !direct_functions->empty()) {
      return true;
    }
  }
  map<string, vector<FunctionBinding *> >::const_iterator methods =
      lexical_class->methods.find(name);
  if(methods != lexical_class->methods.end() && !methods->second.empty()) {
    return true;
  }
  return lexical_class->member_scope &&
         !lookup_direct_function_templates(*lexical_class->member_scope, name).empty();
}

bool scope_has_direct_type_name(Scope & current,
                                const std::string & name)
{
  if(current.named_types.find(name) != current.named_types.end() ||
     current.class_templates.find(name) != current.class_templates.end() ||
     current.alias_templates.find(name) != current.alias_templates.end()) {
    return true;
  }

  const bool has_lexical_class =
      !current.class_info && current.function &&
      current.function->lexical_access_class;
  ClassInfo * lexical_class = current.class_info;
  if(!lexical_class && has_lexical_class) {
    lexical_class = current.function->lexical_access_class;
  }
  if(!lexical_class) {
    return false;
  }

  return lexical_class->member_scope &&
         (lexical_class->member_scope->named_types.find(name) !=
              lexical_class->member_scope->named_types.end() ||
          lexical_class->member_scope->class_templates.find(name) !=
              lexical_class->member_scope->class_templates.end() ||
          lexical_class->member_scope->alias_templates.find(name) !=
              lexical_class->member_scope->alias_templates.end());
}

bool resolved_functional_cast_type_declares_direct_name(SemanticContext & ctx,
                                                        const std::string & name,
                                                        const TypePtr & type)
{
  TypePtr base = strip_top_level_cv(remove_reference_type(type));
  ClassInfo * info = ctx.class_info_for_type(base);
  if(!info || !info->enclosing_scope) {
    return false;
  }
  return !semantic_lookup::lookup_direct_value(*info->enclosing_scope, name) &&
         !scope_has_direct_callable_name(*info->enclosing_scope, name) &&
         scope_has_direct_type_name(*info->enclosing_scope, name);
}

bool unqualified_functional_cast_type_hides_outer_functions(
    SemanticContext & ctx,
    Scope & scope,
    const std::string & name,
    const TypePtr & deferred_functional_cast_type)
{
  if(!deferred_functional_cast_type) {
    return false;
  }

  for(Scope * current = &scope; current; current = current->parent) {
    if(semantic_lookup::lookup_direct_value(*current, name)) {
      return false;
    }
    if(scope_has_direct_callable_name(*current, name)) {
      return false;
    }
    if(scope_has_direct_type_name(*current, name)) {
      return true;
    }
  }
  return resolved_functional_cast_type_declares_direct_name(
      ctx, name, deferred_functional_cast_type);
}

bool unqualified_functional_cast_type_lookup_needed_for_value(
    Scope & scope,
    const std::string & name,
    const ValueBinding & value)
{
  for(Scope * current = &scope; current; current = current->parent) {
    const ValueBinding * direct_value =
        semantic_lookup::lookup_direct_value(*current, name);
    if(direct_value && semantic_lookup::same_value_binding_entity(direct_value, &value)) {
      return false;
    }
    if(scope_has_direct_callable_name(*current, name)) {
      return false;
    }
    if(scope_has_direct_type_name(*current, name)) {
      return true;
    }
  }
  return false;
}

int compare_function_template_partial_order_preference(SemanticContext & ctx,
                                                       const CandidateMatch & current,
                                                       const CandidateMatch & best)
{
  if(semantic_metrics::AnalyzerCounters * counters = performance_counters(ctx)) {
    ++counters->candidate_partial_order_comparisons;
  }
  if(!current.function || !best.function ||
     !current.function->source_template || !best.function->source_template) {
    return 0;
  }

  const int trailing_pack_preference =
      compare_function_template_trailing_pack_preference(current, best);
  const int empty_trailing_pack_preference =
      compare_function_template_empty_trailing_pack_preference(current, best);

  vector<TypePtr> current_transformed_params;
  vector<TypePtr> best_transformed_params;
  if(!transformed_function_template_parameter_types_for_match(
         *current.function->source_template, current, current_transformed_params) ||
     !transformed_function_template_parameter_types_for_match(
         *best.function->source_template, best, best_transformed_params)) {
    return trailing_pack_preference != 0 ? trailing_pack_preference :
                                           empty_trailing_pack_preference;
  }

  const int early_non_type_pack_pattern_preference =
      compare_function_template_non_type_pack_pattern_preference(current, best);
  if(early_non_type_pack_pattern_preference != 0) {
    return early_non_type_pack_pattern_preference;
  }
  if(trailing_pack_preference != 0) {
    return trailing_pack_preference;
  }

  if(semantic_metrics::AnalyzerCounters * counters = performance_counters(ctx)) {
    counters->candidate_partial_order_acceptance_checks += 2;
  }
  const bool current_more_specialized =
      semantic_template_function::function_template_accepts_transformed_parameter_types(
          ctx,
          *best.function->source_template,
          current_transformed_params,
          current.function->source_template->pattern_scope ?
              current.function->source_template->pattern_scope :
              current.function->source_template->declaring_scope,
          true);
  const bool best_more_specialized =
      semantic_template_function::function_template_accepts_transformed_parameter_types(
          ctx,
          *current.function->source_template,
          best_transformed_params,
          best.function->source_template->pattern_scope ?
              best.function->source_template->pattern_scope :
          best.function->source_template->declaring_scope,
          true);

  if(current_more_specialized && !best_more_specialized) {
    return -1;
  }
  if(best_more_specialized && !current_more_specialized) {
    return 1;
  }
  if(empty_trailing_pack_preference != 0) {
    return empty_trailing_pack_preference;
  }
  const int reference_pattern_preference =
      compare_function_template_reference_pattern_preference(current, best);
  if(reference_pattern_preference != 0) {
    return reference_pattern_preference;
  }
  const int array_element_cv_preference =
      compare_function_template_array_element_cv_preference(current, best);
  if(array_element_cv_preference != 0) {
    return array_element_cv_preference;
  }
  const int class_template_arg_cv_preference =
      compare_function_template_class_template_arg_cv_preference(current, best);
  if(class_template_arg_cv_preference != 0) {
    return class_template_arg_cv_preference;
  }
  const int non_type_pack_pattern_preference =
      compare_function_template_non_type_pack_pattern_preference(current, best);
  if(non_type_pack_pattern_preference != 0) {
    return non_type_pack_pattern_preference;
  }
  if(current.function->type && best.function->type &&
     type_equals(current.function->type, best.function->type)) {
    const int type_pack_pattern_preference =
        compare_function_template_type_pack_pattern_preference(current, best);
    if(type_pack_pattern_preference != 0) {
      return type_pack_pattern_preference;
    }
    const int placeholder_specificity =
        compare_partial_order_placeholder_specificity(current_transformed_params,
                                                      best_transformed_params);
    if(placeholder_specificity != 0) {
      return placeholder_specificity;
    }
  }
  const int template_structure_specificity =
      compare_partial_order_template_structure_specificity(current_transformed_params,
                                                          best_transformed_params);
  if(template_structure_specificity != 0) {
    return template_structure_specificity;
  }
  if(current.function->type && best.function->type &&
     type_equals(current.function->type, best.function->type)) {
    const int parameter_count_preference =
        compare_function_template_parameter_count_preference(current, best);
    if(parameter_count_preference != 0) {
      return parameter_count_preference;
    }
  }
  return 0;
}

TypePtr target_function_type(const TypePtr & target)
{
  if(!target) {
    return TypePtr();
  }

  TypePtr base = strip_top_level_cv(target);
  if(base->kind == Type::TK_FUNCTION) {
    return base;
  }
  if((base->kind == Type::TK_LVALUE_REFERENCE ||
      base->kind == Type::TK_RVALUE_REFERENCE) &&
     strip_top_level_cv(base->inner)->kind == Type::TK_FUNCTION) {
    return strip_top_level_cv(base->inner);
  }
  if((base->kind == Type::TK_POINTER || base->kind == Type::TK_BLOCK_POINTER) &&
     strip_top_level_cv(base->inner)->kind == Type::TK_FUNCTION) {
    return strip_top_level_cv(base->inner);
  }
  return TypePtr();
}

bool target_has_unknown_array_bound(const TypePtr & target)
{
  TypePtr base = strip_top_level_cv(remove_reference_type(target));
  return base &&
         base->kind == Type::TK_ARRAY &&
         !base->has_bound;
}

bool should_use_target_aware_argument_analysis(const CppAstNode & node,
                                               const TypePtr & target)
{
  if(!target) {
    return false;
  }
  if(node.kind == CppAstKind::initializer ||
     node.kind == CppAstKind::paren_initializer ||
     node.kind == CppAstKind::braced_init_list) {
    return true;
  }
  if(!target_function_type(target)) {
    return false;
  }
  if(node.kind == CppAstKind::id_expression) {
    return true;
  }
  return node.kind == CppAstKind::unary_expression &&
         node_has_simple_type(node, OP_AMP) &&
         node.children.size() == 1 &&
         node.children[0].kind == CppAstKind::id_expression;
}

bool should_use_template_deduction_target_aware_argument_analysis(
    SemanticContext & ctx,
    const CppAstNode & node,
    const TypePtr & target)
{
  const CppAstNode * payload = &node;
  if((node.kind == CppAstKind::initializer ||
      node.kind == CppAstKind::paren_initializer) &&
     node.children.size() == 1) {
    payload = &node.children[0];
  }
  if(payload->kind == CppAstKind::braced_init_list &&
     target) {
    TypePtr initializer_list_element;
    if(ctx.is_initializer_list_type(target, &initializer_list_element, nullptr) &&
       initializer_list_element &&
       ctx.type_depends_on_template_parameter(initializer_list_element)) {
      return false;
    }
    if(ctx.type_depends_on_template_parameter(target) ||
       target_has_unknown_array_bound(target)) {
      return false;
    }
  }
  return true;
}

ExprInfo make_target_parameter_deduction_argument(const TypePtr & parameter_type)
{
  ExprInfo arg;
  TypePtr base = strip_top_level_cv(parameter_type);
  if(base &&
     (base->kind == Type::TK_LVALUE_REFERENCE ||
      base->kind == Type::TK_RVALUE_REFERENCE)) {
    arg.type = base->inner;
    arg.category = base->kind == Type::TK_LVALUE_REFERENCE ? VC_LVALUE : VC_XVALUE;
  } else {
    arg.type = parameter_type;
    arg.category = VC_PRVALUE;
  }
  arg.node = make_dump_node(CallSemKind::id_expression, "<target-parameter>");
  return arg;
}

bool deduce_function_template_from_target_parameter_types(
    SemanticContext & ctx,
    FunctionTemplateDecl & decl,
    Scope & scope,
    const TypePtr & function_target,
    semantic_template_function::FunctionTemplateDeduction & out)
{
  TypePtr target_base = strip_top_level_cv(function_target);
  if(!target_base || target_base->kind != Type::TK_FUNCTION) {
    return false;
  }

  vector<ExprInfo> args;
  args.reserve(target_base->params.size());
  for(size_t i = 0; i < target_base->params.size(); ++i) {
    args.push_back(make_target_parameter_deduction_argument(target_base->params[i]));
  }
  return semantic_template_function::deduce_function_template_from_arguments(
      ctx,
      decl,
      args,
      &scope,
      out,
      &scope);
}

vector<const CppAstNode *> initializer_argument_nodes(const CppAstNode & node)
{
  vector<const CppAstNode *> args;
  if(node.kind == CppAstKind::initializer && node.children.size() == 1) {
    return initializer_argument_nodes(node.children[0]);
  }
  if(node.kind == CppAstKind::paren_initializer ||
     node.kind == CppAstKind::paren_argument_list ||
     node.kind == CppAstKind::braced_init_list) {
    for(size_t i = 0; i < node.children.size(); ++i) {
      args.push_back(&node.children[i]);
    }
    return args;
  }
  args.push_back(&node);
  return args;
}

bool token_after_node_is_simple(SemanticContext & ctx,
                                const CppAstNode & node,
                                ETokenType type)
{
  const template_api::TemplateWitnessContext witness_ctx =
      ctx.template_witness_context();
  if(!witness_ctx.token_sequence ||
     node.token_end >= witness_ctx.token_sequence->size()) {
    return false;
  }
  return witness_ctx.token_sequence->peek(node.token_end).is_simple(type);
}

bool call_argument_list_is_parenthesized(SemanticContext & ctx,
                                         const CppAstNode & callee_node,
                                         const CppAstNode & argument_list)
{
  return argument_list.kind == CppAstKind::paren_argument_list ||
         token_after_node_is_simple(ctx, callee_node, OP_LPAREN);
}

}  // namespace

namespace {

bool function_binding_accepts_argument_count(const FunctionBinding * binding,
                                             size_t argument_count)
{
  if(!binding) {
    return false;
  }

  size_t required_count = binding->params.size();
  while(required_count > 0 &&
        required_count - 1 < binding->default_arguments.size() &&
        binding->default_arguments[required_count - 1]) {
    --required_count;
  }
  if(argument_count < required_count) {
    return false;
  }

  TypePtr function_type = strip_top_level_cv(binding->type);
  if(function_type &&
     function_type->kind == Type::TK_FUNCTION &&
     (function_type->variadic || function_type->prototype_relaxed) &&
     argument_count >= binding->params.size()) {
    return true;
  }
  return argument_count <= binding->params.size();
}

void filter_function_bindings_by_argument_count(vector<FunctionBinding *> & functions,
                                                size_t argument_count)
{
  vector<FunctionBinding *> filtered;
  filtered.reserve(functions.size());
  for(size_t i = 0; i < functions.size(); ++i) {
    if(function_binding_accepts_argument_count(functions[i], argument_count)) {
      filtered.push_back(functions[i]);
    }
  }
  functions.swap(filtered);
}

bool nonmember_operator_function_candidate(const FunctionBinding * binding)
{
  if(!binding || binding->is_method) {
    return false;
  }
  if(binding->owner_class &&
     (!binding->source_template ||
      binding->source_template->friend_access_classes.empty())) {
    return false;
  }
  return true;
}

void filter_nonmember_operator_functions(vector<FunctionBinding *> & functions)
{
  vector<FunctionBinding *> filtered;
  filtered.reserve(functions.size());
  for(size_t i = 0; i < functions.size(); ++i) {
    if(nonmember_operator_function_candidate(functions[i])) {
      filtered.push_back(functions[i]);
    }
  }
  functions.swap(filtered);
}

void filter_function_templates_by_argument_count(vector<FunctionTemplateDecl *> & templates,
                                                 size_t argument_count)
{
  vector<FunctionTemplateDecl *> filtered;
  filtered.reserve(templates.size());
  for(size_t i = 0; i < templates.size(); ++i) {
    if(templates[i] &&
       function_template_accepts_argument_count_fast(*templates[i], argument_count)) {
      filtered.push_back(templates[i]);
    }
  }
  templates.swap(filtered);
}

bool namespace_function_template_candidate_visible_from_node(
    SemanticContext & ctx,
    const FunctionTemplateDecl * decl,
    const CppAstNode * use_node,
    const Scope * lookup_scope)
{
  if(!decl || !use_node ||
     (decl->declaring_scope && decl->declaring_scope->class_info) ||
     use_node->token_end <= use_node->token_start) {
    return true;
  }

  const CppAstNode * declaration_node =
      decl->declaration_node ? decl->declaration_node : decl->definition_node;
  if(!declaration_node ||
     declaration_node->token_end <= declaration_node->token_start) {
    return true;
  }

  for(const Scope * current = lookup_scope; current; current = current->parent) {
    const CppAstNode * source_node = nullptr;
    if(current->class_info &&
       current->class_info->source_template &&
       current->class_info->source_template->class_node) {
      source_node = current->class_info->source_template->class_node;
    } else if(current->function &&
              current->function->source_template &&
              current->function->source_template->declaration_node) {
      source_node = current->function->source_template->declaration_node;
    }
    if(source_node &&
       source_node->token_end > source_node->token_start &&
       use_node->token_start < source_node->token_start) {
      return true;
    }
    if(current->function && !current->function->source_template) {
      source_node = current->function->definition_node ?
          current->function->definition_node :
          current->function->declaration_node;
      if(source_node &&
         source_node->token_end > source_node->token_start &&
         use_node->token_start < source_node->token_start) {
        // Retained macro replacement tokens can predate the function whose
        // body contains their expansion. Namespace templates declared before
        // that definition are visible throughout the body.
        return declaration_node->token_start <= source_node->token_start;
      }
    }
  }

  if(declaration_node->token_start <= use_node->token_start) {
    return true;
  }

  const ParsedSourceLocation declaration_location =
      parse_source_location(ctx.source_location_for_node(*declaration_node));
  const ParsedSourceLocation use_location =
      parse_source_location(ctx.source_location_for_node(*use_node));
  if(declaration_location.valid &&
     use_location.valid &&
     declaration_location.file == use_location.file) {
    if(declaration_location.line < use_location.line) {
      return true;
    }
    return declaration_location.line == use_location.line &&
           declaration_location.column < use_location.column;
  }
  if(parser_trace::enabled("template.resolve")) {
    std::ostringstream trace;
    trace << "function-template-visibility-reject name=" << decl->name
          << " declaration-token=" << declaration_node->token_start
          << " use-token=" << use_node->token_start
          << " declaration-location="
          << ctx.source_location_for_node(*declaration_node)
          << " use-location=" << ctx.source_location_for_node(*use_node)
          << " current-use-location=" << parser_trace::current_use_location();
    parser_trace::note("template.resolve", std::string(), trace.str());
  }
  return false;
}

void filter_function_template_candidates_visible_from_node(
    SemanticContext & ctx,
    vector<FunctionTemplateDecl *> & templates,
    const CppAstNode * use_node,
    const Scope * lookup_scope)
{
  if(!use_node) {
    return;
  }
  vector<FunctionTemplateDecl *> filtered;
  filtered.reserve(templates.size());
  for(size_t i = 0; i < templates.size(); ++i) {
    if(namespace_function_template_candidate_visible_from_node(ctx,
                                                               templates[i],
                                                               use_node,
                                                               lookup_scope)) {
      filtered.push_back(templates[i]);
    }
  }
  templates.swap(filtered);
}

bool nonmember_operator_function_template_candidate(const FunctionTemplateDecl * decl)
{
  if(!decl) {
    return false;
  }
  if(decl->declaring_scope &&
     decl->declaring_scope->class_info &&
     decl->friend_access_classes.empty()) {
    return false;
  }
  return true;
}

void filter_nonmember_operator_function_templates(
    vector<FunctionTemplateDecl *> & templates)
{
  vector<FunctionTemplateDecl *> filtered;
  filtered.reserve(templates.size());
  for(size_t i = 0; i < templates.size(); ++i) {
    if(nonmember_operator_function_template_candidate(templates[i])) {
      filtered.push_back(templates[i]);
    }
  }
  templates.swap(filtered);
}

void collect_associated_function_candidates_for_types(
    SemanticContext & ctx,
    const std::string & name,
    const std::vector<TypePtr> & types,
    vector<Scope *> & associated_scopes,
    vector<FunctionBinding *> & associated_functions,
    vector<FunctionTemplateDecl *> & associated_templates)
{
  semantic_metrics::AnalyzerCounters * counters = performance_counters(ctx);
  const size_t scope_count_before = associated_scopes.size();
  const size_t function_count_before = associated_functions.size();
  const size_t template_count_before = associated_templates.size();
  if(counters) {
    ++counters->adl_associated_collections;
  }
  for(size_t i = 0; i < types.size(); ++i) {
    if(!types[i]) {
      continue;
    }
    if(counters) {
      ++counters->adl_associated_type_visits;
    }
    collect_associated_namespace_scopes_for_type(ctx, types[i], associated_scopes);
    lookup_associated_friend_functions_for_type(ctx, types[i], name, associated_functions);
    lookup_associated_friend_function_templates_for_type(ctx,
                                                         types[i],
                                                         name,
                                                         associated_templates);
  }
  if(counters) {
    counters->adl_associated_scope_outputs +=
        associated_scopes.size() - scope_count_before;
    counters->adl_associated_function_outputs +=
        associated_functions.size() - function_count_before;
    counters->adl_associated_template_outputs +=
        associated_templates.size() - template_count_before;
  }
}

}  // namespace

void collect_nonmember_operator_candidates(
    SemanticContext & ctx,
    Scope & scope,
    const std::string & operator_name,
    const std::vector<TypePtr> & operand_types,
    std::size_t argument_count,
    NonmemberOperatorCandidateSet & out,
    AssociatedFriendTemplateMergeOrder friend_template_order)
{
  out.functions =
      ctx.lookup_functions(scope, operator_name, semantic_policy::default_call_analysis());
  out.templates.clear();
  out.associated_scopes.clear();
  collect_function_templates(ctx, scope, operator_name, out.templates);

  vector<FunctionTemplateDecl *> associated_templates;
  vector<FunctionTemplateDecl *> & friend_template_out =
      friend_template_order == MERGE_FRIEND_TEMPLATES_AFTER_ASSOCIATED_SCOPES ?
          associated_templates :
          out.templates;
  collect_associated_function_candidates_for_types(ctx,
                                                   operator_name,
                                                   operand_types,
                                                   out.associated_scopes,
                                                   out.functions,
                                                   friend_template_out);
  lookup_adl_functions_in_scopes(out.associated_scopes, operator_name, out.functions);
  lookup_adl_function_templates_in_scopes(out.associated_scopes,
                                          operator_name,
                                          out.templates);
  if(friend_template_order == MERGE_FRIEND_TEMPLATES_AFTER_ASSOCIATED_SCOPES) {
    append_unique_function_templates(out.templates, associated_templates);
  }
  filter_nonmember_operator_functions(out.functions);
  filter_nonmember_operator_function_templates(out.templates);
  filter_function_bindings_by_argument_count(out.functions, argument_count);
  filter_function_templates_by_argument_count(out.templates, argument_count);
}

void initialize_operator_candidate_scope(
    Scope & operator_scope,
    Scope & source_scope,
    const std::string & operator_name,
    const std::vector<Scope *> & associated_scopes,
    const std::vector<FunctionBinding *> & functions,
    const std::vector<FunctionTemplateDecl *> & templates)
{
  operator_scope.class_info = source_scope.class_info;
  operator_scope.function = nullptr;
  operator_scope.name = "<adl>";
  (void)associated_scopes;
  direct_function_set_slot(operator_scope, operator_name) = functions;
  direct_function_template_slot(operator_scope, operator_name) = templates;
}

namespace {

template<typename Map>
void overlay_map_entries(Map & out, const Map & in)
{
  for(typename Map::const_iterator it = in.begin(); it != in.end(); ++it) {
    out[it->first] = it->second;
  }
}

void overlay_adl_argument_lookup_bindings(Scope & out, const Scope & scope)
{
  vector<const Scope *> path;
  for(const Scope * current = &scope; current; current = current->parent) {
    path.push_back(current);
  }
  for(vector<const Scope *>::const_reverse_iterator it = path.rbegin();
      it != path.rend();
      ++it) {
    const Scope & current = **it;
    overlay_map_entries(out.named_types, current.named_types);
    overlay_map_entries(out.named_type_access, current.named_type_access);
    overlay_map_entries(out.named_type_packs, current.named_type_packs);
    overlay_map_entries(out.named_value_packs, current.named_value_packs);
    overlay_map_entries(out.named_pack_sizes, current.named_pack_sizes);
    out.template_bound_type_names.insert(current.template_bound_type_names.begin(),
                                         current.template_bound_type_names.end());
    out.template_bound_type_pack_names.insert(
        current.template_bound_type_pack_names.begin(),
        current.template_bound_type_pack_names.end());
    out.template_bound_value_names.insert(current.template_bound_value_names.begin(),
                                          current.template_bound_value_names.end());
    out.template_bound_value_pack_names.insert(
        current.template_bound_value_pack_names.begin(),
        current.template_bound_value_pack_names.end());
    out.template_bound_template_names.insert(
        current.template_bound_template_names.begin(),
        current.template_bound_template_names.end());
    overlay_map_entries(out.template_bound_template_arguments,
                        current.template_bound_template_arguments);
    overlay_map_entries(out.values, current.values);
    overlay_map_entries(out.namespace_bindings, current.namespace_bindings);
    overlay_map_entries(out.class_templates, current.class_templates);
    overlay_map_entries(out.alias_templates, current.alias_templates);
    overlay_map_entries(out.variable_templates, current.variable_templates);
  }
}

}  // namespace

ExprInfo analyze_adl_only_call_expression(SemanticContext & ctx,
                                          Scope & scope,
                                          const std::string & name,
                                          const vector<const CppAstNode *> & arg_nodes,
                                          const CallAnalysisOptions & options)
{
  vector<ExprInfo> arg_values;
  vector<TypePtr> arg_types;
  arg_values.reserve(arg_nodes.size());
  arg_types.reserve(arg_nodes.size());
  for(size_t i = 0; i < arg_nodes.size(); ++i) {
    arg_values.push_back(ctx.analyze_expression(scope, *arg_nodes[i]));
    arg_types.push_back(arg_values.back().type);
  }

  vector<Scope *> associated_scopes;
  vector<FunctionBinding *> associated_functions;
  vector<FunctionTemplateDecl *> associated_templates;
  collect_associated_function_candidates_for_types(ctx,
                                                   name,
                                                   arg_types,
                                                   associated_scopes,
                                                   associated_functions,
                                                   associated_templates);
  lookup_adl_functions_in_scopes(
      associated_scopes,
      name,
      associated_functions,
      arg_nodes.empty() ? nullptr : arg_nodes[0]);
  lookup_adl_function_templates_in_scopes(associated_scopes,
                                          name,
                                          associated_templates);

  Scope adl_scope(nullptr, "<adl-only>", false);
  overlay_adl_argument_lookup_bindings(adl_scope, scope);
  // Range-for's synthesized begin/end calls perform ADL without ordinary
  // unqualified lookup.  Keep lexical bindings available for analyzing
  // template arguments and candidate signatures, but do not let an
  // unrelated local object with the synthesized callee name turn the call
  // into an object-call expression.
  adl_scope.values.erase(name);
  adl_scope.variable_templates.erase(name);
  if(!associated_functions.empty()) {
    direct_function_set_slot(adl_scope, name) = associated_functions;
  }
  if(!associated_templates.empty()) {
    direct_function_template_slot(adl_scope, name) = associated_templates;
  }

  CppAstNode callee;
  callee.kind = CppAstKind::id_expression;
  callee.value = name;
  CppAstNode call;
  call.kind = CppAstKind::call_expression;
  call.children.push_back(callee);
  CppAstNode args;
  args.kind = CppAstKind::paren_argument_list;
  for(size_t i = 0; i < arg_nodes.size(); ++i) {
    args.children.push_back(*arg_nodes[i]);
  }
  call.children.push_back(args);

  CallAnalysisHints adl_hints = options.hints ? *options.hints : CallAnalysisHints();
  adl_hints.adl_candidates_precollected = true;
  adl_hints.args.assign(arg_values.size(), nullptr);
  for(size_t i = 0; i < arg_values.size(); ++i) {
    adl_hints.args[i] = &arg_values[i];
  }
  return analyze_call_expression(ctx,
                                 adl_scope,
                                 call,
                                 CallAnalysisOptions(options.instantiate_bodies,
                                                     &adl_hints));
}

ExprInfo analyze_functional_cast(SemanticContext & ctx,
                                 Scope & scope,
                                 const TypePtr & callee_type,
                                 const std::vector<const CppAstNode *> & arg_nodes,
                                 const CppAstNode * direct_braced_init,
                                 const CallAnalysisOptions & options)
{
  return analyze_functional_cast_impl(
      ctx, scope, callee_type, arg_nodes, direct_braced_init, options);
}

bool resolve_function_id_for_target(SemanticContext & ctx,
                                    Scope & scope,
                                    const std::string & name,
                                    const TypePtr & target,
                                    ExprInfo & out,
                                    const QualifiedName * name_syntax,
                                    const TemplateIdSyntax * name_template_id_syntax,
                                    const CppAstNode * name_node,
                                    bool require_output_definition)
{
  if(name_node &&
     lookup_id_expression_value_binding_for_call(ctx, scope, *name_node)) {
    return false;
  }

  TypePtr function_target = target_function_type(target);
  if(!function_target) {
    return false;
  }

  const bool has_qualified_name_syntax =
      name_syntax && (name_syntax->rooted || !name_syntax->qualifiers.empty());
  const bool has_qualified_template_id =
      name_template_id_syntax &&
      (name_template_id_syntax->name.rooted ||
       !name_template_id_syntax->name.qualifiers.empty());
  const string template_lookup_name =
      name_template_id_syntax && !name_template_id_syntax->name.name.empty() ?
          name_template_id_syntax->name.name :
          name;
  vector<FunctionBinding *> overloads =
      name_template_id_syntax ?
          (name_node ?
               ctx.lookup_function_template_id_node(
                   scope,
                   *name_node,
                   *name_template_id_syntax,
                   semantic_policy::without_body_instantiation()) :
               ctx.lookup_function_template_id(
                   scope,
                   *name_template_id_syntax,
                   semantic_policy::without_body_instantiation())) :
          (name_node && has_qualified_name_syntax ?
               ctx.lookup_functions_node(
                   scope,
                   *name_node,
                   name,
                   semantic_policy::without_body_instantiation()) :
               ctx.lookup_functions(scope,
                                    name,
                                    semantic_policy::without_body_instantiation()));
  vector<FunctionTemplateDecl *> templates;
  try
  {
    if(name_node && has_qualified_name_syntax) {
      templates =
          ctx.lookup_function_templates_node(scope, *name_node, template_lookup_name);
    } else if(name_syntax && has_qualified_name_syntax) {
      collect_function_templates(ctx, scope, *name_syntax, templates);
    } else if(name_template_id_syntax && has_qualified_template_id) {
      collect_function_templates(ctx, scope, name_template_id_syntax->name, templates);
    } else {
      collect_function_templates(ctx, scope, template_lookup_name, templates);
    }
  }
  catch(const TemplateSubstitutionFailure &)
  {
    templates.clear();
  }
  for(size_t i = 0; i < templates.size(); ++i) {
    vector<TemplateArgument> explicit_arguments;
    const vector<TemplateArgument> * explicit_arguments_ptr = nullptr;
    if(name_template_id_syntax) {
      try
      {
        if(!semantic_template_function::resolve_call_explicit_function_template_arguments(
               ctx,
               *templates[i],
               scope,
               name_template_id_syntax->arguments,
               explicit_arguments,
               &name_template_id_syntax->argument_syntaxes)) {
          continue;
        }
      }
      catch(const TemplateSubstitutionFailure &)
      {
        continue;
      }
      explicit_arguments_ptr = &explicit_arguments;
    }
    semantic_template_function::FunctionTemplateDeduction result;
    bool deduced =
        semantic_template_function::deduce_function_template_from_target_type(
            ctx,
            *templates[i],
            function_target,
            &scope,
            result,
            &scope,
            explicit_arguments_ptr);
    if(!deduced && !explicit_arguments_ptr) {
      deduced =
          deduce_function_template_from_target_parameter_types(ctx,
                                                              *templates[i],
                                                              scope,
                                                              function_target,
                                                              result);
    }
    if(!deduced) {
      continue;
    }
    FunctionBinding * binding = nullptr;
    try
    {
      binding = semantic_template_function::acquire_function_template_binding(
          ctx,
          *templates[i],
          result.arguments,
          &scope,
          &result.pack_sizes,
          false);
    }
    catch(const TemplateSubstitutionFailure &)
    {
      continue;
    }
    if(binding &&
       find(overloads.begin(), overloads.end(), binding) == overloads.end()) {
      overloads.push_back(binding);
    }
  }
  if(overloads.empty()) {
    return false;
  }

  FunctionBinding * match = nullptr;
  vector<FunctionBinding *> matching_overloads;
  for(size_t i = 0; i < overloads.size(); ++i) {
    if(!type_equals(strip_top_level_cv(overloads[i]->type), function_target)) {
      continue;
    }
    matching_overloads.push_back(overloads[i]);
    if(match) {
      ostringstream out;
      out << "ambiguous overloaded function id";
      out << " [target " << describe_type(function_target) << "]";
      out << " [matches ";
      for(size_t j = 0; j < matching_overloads.size(); ++j) {
        if(j != 0) {
          out << "; ";
        }
        append_function_candidate(out, ctx, matching_overloads[j]);
      }
      out << "]";
      throw logic_error(out.str());
    }
    match = overloads[i];
  }

  if(!match) {
    return false;
  }

  match = require_output_definition ?
      semantic_template_function::acquire_function_definition_binding(ctx, match, scope) :
      semantic_template_function::acquire_required_function_definition_binding(
          ctx,
          match,
          scope);
  if(name_node &&
     match &&
     match->owner_class &&
     match->owner_class->type &&
     witness::source_capture_enabled(ctx.template_witness_context())) {
    ctx.record_declaration_type_class_use_for_resolved_type_node(
        scope,
        *name_node,
        match->owner_class->type,
        ctx.source_location_for_node(*name_node),
        true,
        !require_output_definition);
  }
  if(require_output_definition) {
    ctx.require_function_definition(match,
                                    OutputReason::FunctionIdUse,
                                    !match->is_deleted);
  }

  out.type = match->type;
  out.category = VC_LVALUE;
  out.node = make_dump_node(CallSemKind::id_expression, name);
  set_dump_symbol(out.node, match->symbol);
  ctx.set_expr_info_metadata(out, out.type, out.category);
  return true;
}

TypePtr target_member_function_pointer_type(const TypePtr & target)
{
  if(!target) {
    return TypePtr();
  }
  TypePtr base = strip_top_level_cv(remove_reference_type(target));
  if(!base || base->kind != Type::TK_MEMBER_POINTER) {
    return TypePtr();
  }
  TypePtr inner = strip_top_level_cv(base->inner);
  if(!inner || inner->kind != Type::TK_FUNCTION) {
    return TypePtr();
  }
  return base;
}

TypePtr member_pointer_function_type(FunctionBinding & binding)
{
  TypePtr member_function_type = binding.declared_type ? binding.declared_type :
                                                         binding.type;
  TypePtr stripped_function_type = strip_top_level_cv(member_function_type);
  if(stripped_function_type &&
     stripped_function_type->kind == Type::TK_FUNCTION &&
     (stripped_function_type->function_const != binding.is_const_method ||
      stripped_function_type->function_volatile != binding.is_volatile_method)) {
    member_function_type = make_function(stripped_function_type->inner,
                                         stripped_function_type->params,
                                         stripped_function_type->variadic,
                                         binding.is_const_method,
                                         binding.is_volatile_method,
                                         stripped_function_type->prototype_relaxed,
                                         stripped_function_type->function_ref_qualifier);
  }
  return member_function_type;
}

ExprInfo make_member_pointer_function_id_expr(SemanticContext & ctx,
                                              const CppAstNode & unary_node,
                                              const CppAstNode & operand_node,
                                              FunctionBinding & binding)
{
  ExprInfo out;
  out.type =
      make_member_pointer(binding.owner_class->type, member_pointer_function_type(binding));
  out.category = VC_PRVALUE;
  out.node = make_dump_node(CallSemKind::unary_expression, "&");
  set_dump_token(out.node, unary_node);
  out.node.is_c_linkage = binding.is_c_linkage;
  set_dump_symbol(out.node, binding.symbol);
  if(binding.is_virtual) {
    out.node.is_virtual_dispatch = true;
    set_callsem_uint_value(out.node, binding.virtual_slot);
    out.node.uses_extended_vtable_layout =
        binding.owner_class &&
        semantic_class_model::class_uses_extended_virtual_abi(*binding.owner_class);
  }

  ExprInfo child;
  child.type = binding.type;
  child.category = VC_LVALUE;
  child.node = make_dump_node(CallSemKind::id_expression, operand_node.value);
  child.node.is_c_linkage = binding.is_c_linkage;
  child.node.text = binding.name;
  set_dump_symbol(child.node, binding.symbol);
  ctx.set_expr_info_metadata(child, child.type, child.category);
  out.node.children.push_back(std::move(child.node));

  ctx.set_expr_info_metadata(out, out.type, out.category);
  return out;
}

bool collect_overloaded_member_pointer_argument_options(
    SemanticContext & ctx,
    Scope & scope,
    const CppAstNode & unary_node,
    vector<ExprInfo> & out,
    const TypePtr & target_member_pointer = TypePtr())
{
  if(!node_has_simple_type(unary_node, OP_AMP) ||
     unary_node.children.size() != 1) {
    return false;
  }
  const CppAstNode & operand_node = unary_node.children[0];
  const QualifiedName * qualified = cppast_qualified_name_syntax(operand_node);
  if(!qualified || (!qualified->rooted && qualified->qualifiers.empty())) {
    return false;
  }

  const TemplateIdSyntax * template_id =
      cppast_template_id_syntax(operand_node);
  vector<FunctionBinding *> functions =
      template_id ?
          ctx.lookup_function_template_id_node(
              scope,
              operand_node,
              *template_id,
              semantic_policy::without_body_instantiation()) :
          ctx.lookup_functions_node(
              scope,
              operand_node,
              operand_node.value,
              semantic_policy::without_body_instantiation());

  if(target_member_pointer) {
    TypePtr function_target = strip_top_level_cv(target_member_pointer->inner);
    vector<FunctionTemplateDecl *> templates;
    try
    {
      templates = ctx.lookup_function_templates_node(scope,
                                                     operand_node,
                                                     template_id &&
                                                             !template_id->name.name.empty() ?
                                                         template_id->name.name :
                                                         operand_node.value);
    }
    catch(const TemplateSubstitutionFailure &)
    {
      templates.clear();
    }
    for(size_t i = 0; i < templates.size(); ++i) {
      vector<TemplateArgument> explicit_arguments;
      const vector<TemplateArgument> * explicit_arguments_ptr = nullptr;
      if(template_id) {
        try
        {
          if(!semantic_template_function::resolve_call_explicit_function_template_arguments(
                 ctx,
                 *templates[i],
                 scope,
                 template_id->arguments,
                 explicit_arguments,
                 &template_id->argument_syntaxes)) {
            continue;
          }
        }
        catch(const TemplateSubstitutionFailure &)
        {
          continue;
        }
        explicit_arguments_ptr = &explicit_arguments;
      }

      semantic_template_function::FunctionTemplateDeduction result;
      bool deduced =
          semantic_template_function::deduce_function_template_from_target_type(
              ctx,
              *templates[i],
              function_target,
              &scope,
              result,
              &scope,
              explicit_arguments_ptr);
      if(!deduced && !explicit_arguments_ptr) {
        deduced = deduce_function_template_from_target_parameter_types(ctx,
                                                                      *templates[i],
                                                                      scope,
                                                                      function_target,
                                                                      result);
      }
      if(!deduced) {
        continue;
      }

      FunctionBinding * binding = nullptr;
      try
      {
        binding = semantic_template_function::acquire_function_template_binding(
            ctx,
            *templates[i],
            result.arguments,
            &scope,
            &result.pack_sizes,
            false);
      }
      catch(const TemplateSubstitutionFailure &)
      {
        continue;
      }
      if(binding &&
         find(functions.begin(), functions.end(), binding) == functions.end()) {
        functions.push_back(binding);
      }
    }
  }
  if(functions.empty()) {
    return false;
  }

  ScopedCallSemConstructionPath construction_path(
      "overload.arg.overloaded-member-pointer-id");
  out.clear();
  for(size_t i = 0; i < functions.size(); ++i) {
    FunctionBinding * binding = functions[i];
    if(!binding || !binding->is_method || !binding->owner_class ||
       binding->is_constructor || binding->is_destructor) {
      continue;
    }
    ctx.require_function_definition(binding,
                                    OutputReason::FunctionIdUse,
                                    !binding->is_deleted);
    out.push_back(make_member_pointer_function_id_expr(ctx,
                                                       unary_node,
                                                       operand_node,
                                                       *binding));
  }
  return !out.empty();
}

bool resolve_member_function_id_for_target(SemanticContext & ctx,
                                           Scope & scope,
                                           const CppAstNode & unary_node,
                                           const TypePtr & target,
                                           ExprInfo & out)
{
  TypePtr target_member_pointer = target_member_function_pointer_type(target);
  if(!target_member_pointer) {
    return false;
  }

  vector<ExprInfo> options;
  if(!collect_overloaded_member_pointer_argument_options(ctx,
                                                         scope,
                                                         unary_node,
                                                         options,
                                                         target_member_pointer)) {
    return false;
  }

  const TypePtr target_base = strip_top_level_cv(target_member_pointer);
  bool found = false;
  bool ambiguous = false;
  ExprInfo selected;
  for(size_t i = 0; i < options.size(); ++i) {
    TypePtr option_type = strip_top_level_cv(remove_reference_type(options[i].type));
    if(!type_equals(option_type, target_base)) {
      continue;
    }
    if(found) {
      ambiguous = true;
      break;
    }
    found = true;
    selected = options[i];
  }

  if(ambiguous) {
    ostringstream msg;
    msg << "ambiguous overloaded member function id";
    msg << " [target " << describe_type(target_base) << "]";
    throw logic_error(msg.str());
  }
  if(!found) {
    return false;
  }

  out = selected;
  return true;
}

ExprInfo make_function_id_expr(SemanticContext & ctx, FunctionBinding & binding)
{
  ExprInfo out;
  out.type = binding.type;
  out.category = VC_LVALUE;
  out.node = make_dump_node(CallSemKind::id_expression, binding.name);
  set_dump_symbol(out.node, binding.symbol);
  ctx.set_expr_info_metadata(out, out.type, out.category);
  return out;
}

bool collect_overloaded_function_id_argument_options(SemanticContext & ctx,
                                                     Scope & scope,
                                                     const CppAstNode & id_node,
                                                     vector<ExprInfo> & out)
{
  if(lookup_id_expression_value_binding_for_call(ctx, scope, id_node)) {
    return false;
  }

  const TemplateIdSyntax * template_id = cppast_template_id_syntax(id_node);
  const QualifiedName * qualified = cppast_qualified_name_syntax(id_node);
  const bool has_qualified_name =
      qualified && (qualified->rooted || !qualified->qualifiers.empty());
  vector<FunctionBinding *> overloads =
      template_id ?
          ctx.lookup_function_template_id_node(
              scope,
              id_node,
              *template_id,
              semantic_policy::without_body_instantiation()) :
          (has_qualified_name ?
               ctx.lookup_functions_node(
                   scope,
                   id_node,
                   id_node.value,
                   semantic_policy::without_body_instantiation()) :
               ctx.lookup_functions(
                   scope,
                   id_node.value,
                   semantic_policy::without_body_instantiation()));
  if(overloads.empty()) {
    return false;
  }

  ScopedCallSemConstructionPath construction_path("overload.arg.overloaded-function-id");
  out.clear();
  for(size_t i = 0; i < overloads.size(); ++i) {
    if(!overloads[i]) {
      continue;
    }
    out.push_back(make_function_id_expr(ctx, *overloads[i]));
  }
  return !out.empty();
}

bool id_expression_names_function_or_template_set(SemanticContext & ctx,
                                                  Scope & scope,
                                                  const CppAstNode & id_node)
{
  if(id_node.kind != CppAstKind::id_expression ||
     lookup_id_expression_value_binding_for_call(ctx, scope, id_node)) {
    return false;
  }

  const TemplateIdSyntax * template_id = cppast_template_id_syntax(id_node);
  const QualifiedName * qualified = cppast_qualified_name_syntax(id_node);
  const bool has_qualified_name =
      qualified && (qualified->rooted || !qualified->qualifiers.empty());
  try {
    vector<FunctionBinding *> functions =
        template_id ?
            ctx.lookup_function_template_id_node(
                scope,
                id_node,
                *template_id,
                semantic_policy::without_body_instantiation()) :
            (has_qualified_name ?
                 ctx.lookup_functions_node(
                     scope,
                     id_node,
                     id_node.value,
                     semantic_policy::without_body_instantiation()) :
                 ctx.lookup_functions(
                     scope,
                     id_node.value,
                     semantic_policy::without_body_instantiation()));
    if(!functions.empty()) {
      return true;
    }
  } catch(const TemplateSubstitutionFailure &) {
    // A target type may be needed before an explicit function-template-id can
    // materialize a binding.  The template set lookup below still proves that
    // this is a function-id argument.
  }

  vector<FunctionTemplateDecl *> templates;
  try {
    if(has_qualified_name) {
      templates = ctx.lookup_function_templates_node(
          scope,
          id_node,
          template_id && !template_id->name.name.empty() ?
              template_id->name.name : id_node.value);
    } else {
      collect_function_templates(
          ctx,
          scope,
          template_id ? template_id->name.name : id_node.value,
          templates);
    }
  } catch(const TemplateSubstitutionFailure &) {
    templates.clear();
  }
  return !templates.empty();
}

bool collect_dependent_overloaded_function_id_placeholder(
    SemanticContext & ctx,
    Scope & scope,
    const CppAstNode & id_node,
    const TypePtr & target,
    vector<ExprInfo> & out)
{
  if(!target ||
     !ctx.type_depends_on_template_parameter(target) ||
     !id_expression_names_function_or_template_set(ctx, scope, id_node)) {
    return false;
  }

  static const TypePtr placeholder_type =
      make_named("<unresolved overloaded function id>",
                 "__cppgm_unresolved_overloaded_function_id",
                 true);
  ExprInfo placeholder;
  placeholder.type = placeholder_type;
  placeholder.category = VC_LVALUE;
  placeholder.node = make_dump_node(CallSemKind::id_expression, id_node.value);
  set_dump_token(placeholder.node, id_node);
  out.clear();
  out.push_back(placeholder);
  return true;
}

void append_function_template_call_candidates_impl(
    SemanticContext & ctx,
    Scope & lookup_scope,
    Scope & argument_scope,
    const std::string & name,
    const std::vector<const CppAstNode *> & arg_nodes,
    std::vector<FunctionBinding *> & out,
    const CallAnalysisOptions & options,
    std::vector<template_api::TemplateWitnessSourceDrop> * witness_drops,
    const CppAstNode * name_node = nullptr,
    const QualifiedName * name_syntax = nullptr,
    const TemplateIdSyntax * name_template_id_syntax = nullptr)
{
  ScopedCallSemConstructionPath construction_path("overload.template-candidates");
  std::string trace_location;
  if(template_witness_source_capture_enabled_for_calls(ctx)) {
    const std::string raw_trace_location =
        !arg_nodes.empty() ? ctx.source_location_for_node(*arg_nodes.front()) :
                             std::string();
    trace_location =
        prefer_later_source_location(parser_trace::current_use_location(),
                                     raw_trace_location);
  }
  ScopedTemplateUseLocation use_location_guard(trace_location);
  QualifiedName explicit_name;
  vector<string> explicit_args;
  const bool has_explicit_args = name_template_id_syntax != nullptr;
  const bool use_preselected_member_templates =
      lookup_scope.name == "<member-templates>" && lookup_scope.class_info;
  if(name_template_id_syntax) {
    explicit_name = name_template_id_syntax->name;
    explicit_args = name_template_id_syntax->arguments;
  }
  string template_name = name;
  if(has_explicit_args) {
    template_name.clear();
    if(explicit_name.rooted) {
      template_name += "::";
    }
    for(size_t i = 0; i < explicit_name.qualifiers.size(); ++i) {
      template_name += explicit_name.qualifiers[i];
      template_name += "::";
    }
    template_name += explicit_name.name;
  }
  QualifiedName qualified_template_name;
  bool has_qualified_template_name = false;
  if(has_explicit_args) {
    qualified_template_name = explicit_name;
    has_qualified_template_name =
        qualified_template_name.rooted || !qualified_template_name.qualifiers.empty();
  } else if(name_syntax &&
            (name_syntax->rooted || !name_syntax->qualifiers.empty())) {
    qualified_template_name = *name_syntax;
    has_qualified_template_name = true;
    template_name.clear();
    if(qualified_template_name.rooted) {
      template_name += "::";
    }
    for(size_t i = 0; i < qualified_template_name.qualifiers.size(); ++i) {
      template_name += qualified_template_name.qualifiers[i];
      template_name += "::";
    }
    template_name += qualified_template_name.name;
  }
  if(use_preselected_member_templates && has_qualified_template_name) {
    template_name = qualified_template_name.name;
    has_qualified_template_name = false;
  }
  vector<FunctionTemplateDecl *> templates;
  const bool has_structured_qualified_name =
      name_syntax && (name_syntax->rooted || !name_syntax->qualifiers.empty());
  const bool use_structured_template_lookup =
      !use_preselected_member_templates &&
      name_node && has_qualified_template_name &&
      (has_structured_qualified_name || name_template_id_syntax);
  map<FunctionTemplateDecl *, ClassInfo *> template_active_owners;
  const auto note_template_active_owner =
      [&](const vector<FunctionTemplateDecl *> & owner_templates, const ClassInfo * owner)
      {
        if(!owner) {
          return;
        }
        for(size_t owner_index = 0; owner_index < owner_templates.size(); ++owner_index) {
          if(owner_templates[owner_index]) {
            template_active_owners[owner_templates[owner_index]] =
                const_cast<ClassInfo *>(owner);
          }
        }
      };
  const auto ensure_template_lookup_reference_members_for_class =
      [&](ClassInfo * info, const string & lookup_name) -> void
      {
        if(!info ||
           !info->member_scope ||
           info->reference_members_collected ||
           info->reference_named_members_collected.count(lookup_name) != 0 ||
           info->reference_member_collection_in_progress ||
           info->full_member_collection_in_progress) {
          return;
        }
        ctx.ensure_class_reference_named_member(*info, lookup_name);
      };
  const auto ensure_template_lookup_reference_members =
      [&](Scope * qualified_scope, const string & lookup_name) -> void
      {
        if(!qualified_scope || !qualified_scope->class_info) {
          return;
        }
        ensure_template_lookup_reference_members_for_class(
            qualified_scope->class_info,
            lookup_name);
      };
  if(use_preselected_member_templates) {
    const vector<FunctionTemplateDecl *> * found =
        find_direct_function_template_set(lookup_scope, template_name);
    if(found) {
      append_unique_function_templates(templates, *found);
      note_template_active_owner(*found, lookup_scope.class_info);
    }
  } else if(use_structured_template_lookup) {
    Scope * qualified_scope =
        ctx.resolve_qualified_scope_for_node(lookup_scope,
                                             qualified_template_name,
                                             *name_node,
                                             false);
    if(qualified_scope && qualified_scope->class_info) {
      ensure_template_lookup_reference_members(
          qualified_scope,
          qualified_template_name.name);
      MemberFunctionTemplateLookupResult result =
          lookup_visible_member_function_templates(*qualified_scope->class_info,
                                                   qualified_template_name.name);
      append_unique_function_templates(templates, result.templates);
      note_template_active_owner(result.templates, result.declared_in);
    } else {
      templates = ctx.lookup_function_templates_node(lookup_scope,
                                                     *name_node,
                                                     template_name);
    }
  } else if(has_qualified_template_name) {
    Scope * qualified_scope =
        resolve_qualified_scope_for_class_or_namespace(ctx,
                                                       lookup_scope,
                                                       qualified_template_name);
    if(qualified_scope && qualified_scope->class_info) {
      ensure_template_lookup_reference_members(
          qualified_scope,
          qualified_template_name.name);
      MemberFunctionTemplateLookupResult result =
          lookup_visible_member_function_templates(*qualified_scope->class_info,
                                                   qualified_template_name.name);
      append_unique_function_templates(templates, result.templates);
      note_template_active_owner(result.templates, result.declared_in);
    } else {
      collect_function_templates(ctx, lookup_scope, qualified_template_name, templates);
    }
  } else {
    ensure_template_lookup_reference_members_for_class(
        current_class_scope(lookup_scope),
        template_name);
    if(FunctionBinding * function = current_function_scope(lookup_scope)) {
      ensure_template_lookup_reference_members_for_class(
          function->lexical_access_class,
          template_name);
    }
    collect_function_templates(ctx, lookup_scope, template_name, templates);
  }
  if(lookup_scope.name != "<adl>") {
    filter_function_template_candidates_visible_from_node(ctx,
                                                          templates,
                                                          name_node,
                                                          &lookup_scope);
  }
  suppress_implicit_template_instantiation_lookup_candidates(ctx, out, templates);
  FunctionCandidateBucketMap seen_candidates;
  for(size_t i = 0; i < out.size(); ++i) {
    if(ctx.function_binding_is_live(out[i])) {
      note_function_candidate_bucket(seen_candidates, out[i]);
    }
  }
  if(parser_trace::enabled("template.resolve")) {
    std::ostringstream trace;
    trace << "call-candidates name=" << name
          << " template_name=" << template_name
          << " explicit=" << (has_explicit_args ? "yes" : "no")
          << " template_count=" << templates.size();
    parser_trace::note("template.resolve", trace_location, trace.str());
  }
  if(templates.empty()) {
    return;
  }

  SharedCallArgumentAnalyzer argument_analyzer(ctx,
                                               argument_scope,
                                               arg_nodes,
                                               options,
                                               true);

  for(size_t i = 0; i < templates.size(); ++i) {
    if(!templates[i]) {
      continue;
    }
    const bool uses_explicit_member_arg_prefix =
        options.hints &&
        options.hints->explicit_member_base &&
        options.hints->explicit_member_arg_prefix <= arg_nodes.size() &&
        templates[i]->declaring_scope &&
        templates[i]->declaring_scope->class_info &&
        !templates[i]->is_static_member;
    if(uses_explicit_member_arg_prefix && options.hints->explicit_member_declared_in) {
      template_active_owners[templates[i]] =
          const_cast<ClassInfo *>(options.hints->explicit_member_declared_in);
    }
    const size_t source_arg_begin =
        uses_explicit_member_arg_prefix ? options.hints->explicit_member_arg_prefix : 0;
    const size_t template_arg_count = arg_nodes.size() - source_arg_begin;
    if(!function_template_accepts_argument_count_fast(*templates[i], template_arg_count)) {
      append_template_function_candidate_drop(
          ctx,
          templates[i],
          function_template_argument_count_drop_reason(*templates[i], template_arg_count),
          witness_drops);
      continue;
    }
    if(parser_trace::enabled("template.resolve")) {
      std::ostringstream trace;
      trace << "candidate-template name=" << templates[i]->name
            << " decl="
            << (templates[i]->debug_decl_location.empty() ?
                    std::string("<none>") :
                    templates[i]->debug_decl_location)
            << " scope="
            << (templates[i]->debug_scope_name.empty() ?
                    std::string("<none>") :
                    templates[i]->debug_scope_name)
            << " snapshot={"
            << (templates[i]->debug_signature.empty() ?
                    semantic_trace::function_template_signature_for_diagnostic(*templates[i]) :
                    templates[i]->debug_signature)
            << "}"
            << " current={"
            << semantic_trace::function_template_signature_for_diagnostic(*templates[i]) << "}"
            << " source_arg_begin=" << source_arg_begin
            << " template_arg_count=" << template_arg_count;
      parser_trace::note("template.resolve", trace_location, trace.str());
    }
    vector<vector<ExprInfo> > arg_options;
    bool args_ok = true;
    string arg_error;
    for(size_t j = 0; j < template_arg_count; ++j) {
      const size_t source_arg_index = source_arg_begin + j;
      const TypePtr target =
          j < templates[i]->params_pattern.size() ? templates[i]->params_pattern[j].second :
                                                    TypePtr();
      vector<ExprInfo> overload_options;
      try {
        if(arg_nodes[source_arg_index]->kind == CppAstKind::id_expression &&
           target_function_type(target) &&
           collect_overloaded_function_id_argument_options(ctx,
                                                           argument_scope,
                                                           *arg_nodes[source_arg_index],
                                                           overload_options)) {
          arg_options.push_back(overload_options);
          continue;
        }
        if(target_member_function_pointer_type(target) &&
           collect_overloaded_member_pointer_argument_options(ctx,
                                                              argument_scope,
                                                              *arg_nodes[source_arg_index],
                                                              overload_options)) {
          arg_options.push_back(overload_options);
          continue;
        }
        ExprInfo reference_source;
        if(target && is_reference_type(target)) {
          if(argument_analyzer.analyze_reference_source(source_arg_index, reference_source)) {
            arg_options.push_back(vector<ExprInfo>(1, reference_source));
            continue;
          }
        }
        ExprInfo arg;
        {
          ScopedCallSemConstructionPath arg_path("overload.template.arg-analysis");
          arg = argument_analyzer.analyze_argument(
              source_arg_index,
              target,
              should_use_template_deduction_target_aware_argument_analysis(
                  ctx, *arg_nodes[source_arg_index], target));
        }
        arg_options.push_back(vector<ExprInfo>(1, arg));
      } catch(const logic_error & e) {
        if(arg_nodes[source_arg_index]->kind == CppAstKind::id_expression &&
           collect_dependent_overloaded_function_id_placeholder(
               ctx,
               argument_scope,
               *arg_nodes[source_arg_index],
               target,
               overload_options)) {
          arg_options.push_back(overload_options);
          continue;
        }
        args_ok = false;
        arg_error = e.what();
        break;
      }
    }
    if(!args_ok) {
      if(parser_trace::enabled("template.resolve")) {
        std::ostringstream trace;
        trace << "candidate name=" << templates[i]->name
              << " explicit=" << (has_explicit_args ? "yes" : "no")
              << " arg-analysis-failed=" << arg_error;
        parser_trace::note("template.resolve", trace_location, trace.str());
      }
      append_template_function_candidate_drop(ctx,
                                              templates[i],
                                              "substitution_failure",
                                              witness_drops);
      continue;
    }

    const Scope * instantiation_use_scope_base =
        templates[i] &&
                templates[i]->declaring_scope &&
                !scope_is_within(argument_scope, templates[i]->declaring_scope) ?
            &lookup_scope :
            &argument_scope;
    std::unique_ptr<Scope> merged_use_scope;
    Scope * instantiation_use_scope = const_cast<Scope *>(instantiation_use_scope_base);
    auto overlay_use_scope = [&](const Scope * source, bool adopt_class_owner = false)
    {
      if(!source || source == instantiation_use_scope_base) {
        if(adopt_class_owner &&
           source &&
           source->class_info &&
           instantiation_use_scope->class_info != source->class_info) {
          if(!merged_use_scope) {
            merged_use_scope.reset(
                new Scope(const_cast<Scope *>(instantiation_use_scope_base), "", false));
          }
          merged_use_scope->class_info = source->class_info;
          instantiation_use_scope = merged_use_scope.get();
        }
        return;
      }
      if(!merged_use_scope) {
        merged_use_scope.reset(
            new Scope(const_cast<Scope *>(instantiation_use_scope_base), "", false));
      }
      semantic_template_function::overlay_instantiation_use_scope_bindings(
          *merged_use_scope, *source, nullptr);
      if(adopt_class_owner && source->class_info) {
        merged_use_scope->class_info = source->class_info;
      }
      instantiation_use_scope = merged_use_scope.get();
    };
    if(has_explicit_args) {
      // Explicit function-template arguments are spelled at the call site, so
      // names like _Functor in obj._M_access<_Functor*>() must resolve against
      // the caller's bindings even when the selected member template lives in a
      // non-ancestor scope.
      overlay_use_scope(&argument_scope);
    }
    const bool has_structured_qualifier_syntax =
        name_node &&
        (!name_node->qualifier_template_id_syntaxes.empty() ||
         !name_node->qualifier_type_syntaxes.empty());
    if(has_qualified_template_name) {
      Scope * qualified_scope = has_structured_qualifier_syntax ?
          ctx.resolve_qualified_scope_for_node(argument_scope,
                                               qualified_template_name,
                                               *name_node,
                                               false) :
          resolve_qualified_scope_for_class_or_namespace(ctx,
                                                         argument_scope,
                                                         qualified_template_name);
      overlay_use_scope(qualified_scope, qualified_scope && qualified_scope->class_info);
      if(!has_structured_qualifier_syntax &&
         !qualified_template_name.rooted &&
         !qualified_template_name.qualifiers.empty()) {
        string qualifier_text;
        for(size_t i = 0; i < qualified_template_name.qualifiers.size(); ++i) {
          if(i != 0) {
            qualifier_text += "::";
          }
          qualifier_text += qualified_template_name.qualifiers[i];
        }
        TypePtr qualifier_type = ctx.lookup_type(argument_scope, qualifier_text);
        ClassInfo * qualifier_class = ctx.class_info_for_type(qualifier_type);
        if(qualifier_class) {
          overlay_use_scope(qualifier_class->member_scope.get());
        }
      }
    }
    vector<TemplateArgument> resolved_explicit_arguments;
    if(has_explicit_args) {
      const std::vector<TemplateArgumentSyntax> * explicit_arg_syntaxes =
          name_template_id_syntax ? &name_template_id_syntax->argument_syntaxes :
                                    nullptr;
      if(!semantic_template_function::resolve_call_explicit_function_template_arguments(
             ctx,
             *templates[i],
             argument_scope,
             explicit_args,
             resolved_explicit_arguments,
             explicit_arg_syntaxes)) {
        if(parser_trace::enabled("template.resolve")) {
          std::ostringstream trace;
          trace << "candidate name=" << templates[i]->name
                << " explicit=yes explicit-arg-resolution-failed";
          parser_trace::note("template.resolve", trace_location, trace.str());
        }
        continue;
      }
      if(arg_nodes.empty() &&
         template_api::template_arguments_are_dependent(
             ctx, resolved_explicit_arguments)) {
        continue;
      }
    }
    vector<ExprInfo> args;
    args.reserve(arg_options.size());
    const size_t candidate_count_before_combinations = out.size();
    std::vector<template_api::TemplateWitnessSourceDrop> combination_drops;
    ExprInfo template_implicit_object_arg;
    bool template_implicit_object_ready = false;
    struct ArgumentCombinationRunner
    {
      SemanticContext & ctx;
      FunctionTemplateDecl & candidate_template;
      bool has_explicit_args;
      bool instantiate_bodies;
      Scope * instantiation_use_scope;
      std::vector<TemplateArgument> & resolved_explicit_arguments;
      std::vector<std::vector<ExprInfo> > & arg_options;
      std::vector<ExprInfo> & args;
      FunctionCandidateBucketMap & seen_candidates;
      std::vector<FunctionBinding *> & out;
      std::vector<template_api::TemplateWitnessSourceDrop> & combination_drops;
      std::map<FunctionTemplateDecl *, ClassInfo *> & template_active_owners;
      const CallAnalysisHints * hints;
      ExprInfo & template_implicit_object_arg;
      bool & template_implicit_object_ready;
      const std::string & trace_location;

      bool binding_accepts_implicit_object(FunctionBinding * binding)
      {
        if(!binding ||
           !binding->is_method ||
           candidate_template.is_static_member ||
           !hints ||
           !hints->explicit_member_base) {
          return true;
        }

        TypePtr function_type = strip_top_level_cv(binding->type);
        if(!function_type ||
           function_type->kind != Type::TK_FUNCTION ||
           function_type->params.empty()) {
          return false;
        }

        if(ref_qualifier_rejects_implicit_object(
               binding->ref_qualifier,
               function_type->params[0],
               hints->explicit_member_base->category)) {
          return false;
        }

        if(!template_implicit_object_ready) {
          ScopedCallSemConstructionPath construction_path(
              "overload.template-candidate-implicit-object");
          template_implicit_object_arg =
              ctx.make_address_of_expr(*hints->explicit_member_base);
          template_implicit_object_ready = true;
        }

        ExprInfo adjusted_this = template_implicit_object_arg;
        ExprInfo converted_this;
        const bool converted_this_ok =
            instantiate_bodies ?
                semantic_conversion::try_apply_unmaterialized_inheritance_conversion(
                    ctx,
                    function_type->params[0],
                    template_implicit_object_arg,
                    converted_this) :
                semantic_conversion::try_apply_inheritance_conversion(
                    ctx,
                    function_type->params[0],
                    template_implicit_object_arg,
                    converted_this);
        if(converted_this_ok) {
          adjusted_this = converted_this;
        }

        return semantic_conversion::implicit_object_conversion_rank(
                   ctx,
                   function_type->params[0],
                   adjusted_this) != CR_BAD;
      }

      void run(size_t index)
      {
        if(index == arg_options.size()) {
          vector<TemplateArgument> deduced;
          map<string, size_t> pack_sizes;
          bool deduced_ok = false;
          semantic_template_function::FunctionTemplateDeduction result;
          {
            const template_api::ScopedTemplateWitnessFunctionCallSourceCapturePause
                class_source_capture_pause;
            ScopedCallSemConstructionPath deduction_path("overload.template-deduction");
            if(has_explicit_args) {
              deduced_ok =
                  semantic_template_function::deduce_function_template_from_arguments(
                      ctx,
                      candidate_template,
                      args,
                      instantiation_use_scope,
                      result,
                      instantiation_use_scope,
                      &resolved_explicit_arguments);
            } else {
              deduced_ok =
                  semantic_template_function::deduce_function_template_from_arguments(
                      ctx, candidate_template, args, instantiation_use_scope, result);
            }
          }
          if(deduced_ok) {
            deduced.swap(result.arguments);
            pack_sizes.swap(result.pack_sizes);
          }
          if(parser_trace::enabled("template.resolve")) {
            std::ostringstream trace;
            trace << "candidate name=" << candidate_template.name
                  << " explicit=" << (has_explicit_args ? "yes" : "no")
                  << " deduced_ok=" << (deduced_ok ? "yes" : "no")
                  << " deduced_count=" << deduced.size();
            parser_trace::note("template.resolve", trace_location, trace.str());
          }
          if(!deduced_ok) {
            const std::string deduction_reason =
                function_template_deduction_failure_drop_reason(ctx,
                                                                candidate_template,
                                                                args,
                                                                has_explicit_args);
            append_template_function_candidate_drop(
                ctx,
                &candidate_template,
                deduction_reason,
                &combination_drops);
            return;
          }

          FunctionBinding * binding = nullptr;
          try
          {
            const template_api::ScopedTemplateWitnessFunctionCallSourceCapturePause
                class_source_capture_pause;
            ScopedCallSemConstructionPath acquire_path("overload.template-acquire-binding");
            ClassInfo * active_owner = nullptr;
            map<FunctionTemplateDecl *, ClassInfo *>::iterator active_owner_it =
                template_active_owners.find(&candidate_template);
            if(active_owner_it != template_active_owners.end()) {
              active_owner = active_owner_it->second;
            }
            binding =
                semantic_template_function::acquire_function_template_binding(
                    ctx,
                    candidate_template,
                    deduced,
                    instantiation_use_scope,
                    &pack_sizes,
                    false,
                    active_owner);
          }
          catch(const TemplateSubstitutionFailure & e)
          {
            if(parser_trace::enabled("template.resolve")) {
              std::ostringstream trace;
              trace << "candidate-instantiation-failed name=" << candidate_template.name
                    << " explicit=" << (has_explicit_args ? "yes" : "no")
                    << " error=" << e.what();
              parser_trace::note("template.resolve", trace_location, trace.str());
            }
            append_template_function_candidate_drop(ctx,
                                                    &candidate_template,
                                                    "substitution_failure",
                                                    &combination_drops);
            return;
          }
          catch(const logic_error & e)
          {
            if(parser_trace::enabled("template.resolve")) {
              std::ostringstream trace;
              trace << "candidate-instantiation-failed name=" << candidate_template.name
                    << " explicit=" << (has_explicit_args ? "yes" : "no")
                    << " error=" << e.what();
              parser_trace::note("template.resolve", trace_location, trace.str());
            }
            append_template_function_candidate_drop(ctx,
                                                    &candidate_template,
                                                    "substitution_failure",
                                                    &combination_drops);
            return;
          }
          if(!binding) {
            return;
          }
          FunctionCandidateRefreshKey binding_refresh_key;
          if(binding->owner_class &&
             binding->owner_class->reference_members_collected &&
             !binding->owner_class->complete) {
            binding_refresh_key = function_candidate_refresh_key(*binding);
          }
          if(!binding_accepts_implicit_object(binding)) {
            append_template_function_candidate_drop(ctx,
                                                    &candidate_template,
                                                    "implicit_object_conversion_failed",
                                                    &combination_drops);
            return;
          }
          if(binding_refresh_key.owner_class) {
            if(!function_candidate_matches_refresh_key(
                   ctx, binding, binding_refresh_key)) {
              binding = refresh_invalidated_member_candidate(
                  ctx, binding_refresh_key);
              if(!function_candidate_matches_refresh_key(
                     ctx, binding, binding_refresh_key)) {
                return;
              }
              note_overload_candidate_refresh(ctx, true);
            }
          } else if(!ctx.function_binding_is_live(binding)) {
            return;
          }
          if(!contains_equivalent_function_candidate(
                 seen_candidates, binding, &ctx)) {
            out.push_back(binding);
            note_function_candidate_bucket(seen_candidates, binding);
          }
          return;
        }

        for(size_t option_index = 0; option_index < arg_options[index].size(); ++option_index) {
          args.push_back(arg_options[index][option_index]);
          run(index + 1);
          args.pop_back();
        }
      }
    };
    ArgumentCombinationRunner combination_runner = {
        ctx,
        *templates[i],
        has_explicit_args,
        options.instantiate_bodies,
        instantiation_use_scope,
        resolved_explicit_arguments,
        arg_options,
        args,
        seen_candidates,
        out,
        combination_drops,
        template_active_owners,
        options.hints,
        template_implicit_object_arg,
        template_implicit_object_ready,
        trace_location};
    combination_runner.run(0);
    if(witness_drops && out.size() == candidate_count_before_combinations) {
      for(size_t j = 0; j < combination_drops.size(); ++j) {
        witness_drops->push_back(combination_drops[j]);
      }
    }
  }
}

namespace {

bool append_ordinary_call_adl_candidates(
    SemanticContext & ctx,
    Scope & scope,
    const CppAstNode & lookup_callee_node,
    const std::vector<const CppAstNode *> & arg_nodes,
    bool has_direct_explicit_template_args,
    bool instantiate_bodies,
    const CallAnalysisHints *& effective_hints,
    CallAnalysisHints & merged_lookup_hints,
    vector<ExprInfo> & merged_lookup_arg_values,
    SharedCallArgumentAnalyzer & argument_analyzer,
    vector<FunctionBinding *> & candidates,
    std::vector<template_api::TemplateWitnessSourceDrop> * direct_function_source_drops)
{
  ScopedCallSemConstructionPath construction_path("overload.adl-candidates");
  if(arg_nodes.empty()) {
    return false;
  }

  merged_lookup_arg_values.assign(arg_nodes.size(), ExprInfo());
  merged_lookup_hints.explicit_member_base =
      effective_hints ? effective_hints->explicit_member_base : nullptr;
  merged_lookup_hints.args.assign(arg_nodes.size(), nullptr);
  if(effective_hints) {
    for(size_t i = 0;
        i < effective_hints->args.size() && i < merged_lookup_hints.args.size();
        ++i) {
      merged_lookup_hints.args[i] = effective_hints->args[i];
    }
  }

  vector<Scope *> associated_scopes;
  vector<FunctionBinding *> associated_functions;
  vector<FunctionTemplateDecl *> associated_templates;
  vector<TypePtr> associated_arg_types;
  const TemplateIdSyntax * lookup_template_id =
      cppast_template_id_syntax(lookup_callee_node);
  const string lookup_name =
      lookup_template_id ? lookup_template_id->name.name : lookup_callee_node.value;
  for(size_t i = 0; i < arg_nodes.size(); ++i) {
    ExprInfo arg_expr;
    string arg_error;
    {
      ScopedCallSemConstructionPath arg_path("overload.adl.arg-analysis");
      if(!argument_analyzer.analyze_generic_arg(i, arg_expr, arg_error)) {
        continue;
      }
    }
    if(!(effective_hints &&
         i < effective_hints->args.size() &&
         effective_hints->args[i])) {
      merged_lookup_arg_values[i] = arg_expr;
      merged_lookup_hints.args[i] = &merged_lookup_arg_values[i];
    }

    associated_arg_types.push_back(arg_expr.type);
  }
  collect_associated_function_candidates_for_types(ctx,
                                                   lookup_name,
                                                   associated_arg_types,
                                                   associated_scopes,
                                                   associated_functions,
                                                   associated_templates);

  if(associated_scopes.empty() &&
     associated_functions.empty() &&
     associated_templates.empty()) {
    return false;
  }

  lookup_adl_functions_in_scopes(associated_scopes,
                                 lookup_name,
                                 associated_functions,
                                 &lookup_callee_node);
  lookup_adl_function_templates_in_scopes(associated_scopes,
                                          lookup_name,
                                          associated_templates);
  if(associated_functions.empty() && associated_templates.empty()) {
    return false;
  }

  Scope adl_scope(nullptr, "<adl>", false);
  adl_scope.class_info = scope.class_info;
  adl_scope.function = scope.function;
  if(!associated_functions.empty()) {
    direct_function_set_slot(adl_scope, lookup_name) = associated_functions;
  }
  if(!associated_templates.empty()) {
    direct_function_template_slot(adl_scope, lookup_name) = associated_templates;
  }

  vector<FunctionBinding *> adl_candidates =
      has_direct_explicit_template_args ?
          vector<FunctionBinding *>() :
          ctx.lookup_functions(adl_scope, lookup_name,
                               semantic_policy::without_body_instantiation());
  append_function_template_call_candidates_impl(
      ctx,
      adl_scope,
      scope,
      lookup_name,
      arg_nodes,
      adl_candidates,
      semantic_policy::call_analysis(instantiate_bodies, &merged_lookup_hints),
      direct_function_source_drops,
      &lookup_callee_node,
      cppast_qualified_name_syntax(lookup_callee_node),
      cppast_template_id_syntax(lookup_callee_node));
  append_unique_functions(candidates, adl_candidates);
  effective_hints = &merged_lookup_hints;
  argument_analyzer.set_hints(effective_hints);
  return true;
}

struct ConstructorSelectionState
{
  vector<CandidateMatch> matches;
  FunctionCandidateBucketMap seen_candidates;
  set<FunctionBinding *> considered;
  vector<FunctionBinding *> built_candidates;
  vector<string> candidate_rejections;
  vector<template_api::TemplateWitnessSourceDrop> source_drops;
  unordered_map<CachedArgumentConversionKey,
                CachedArgumentConversionResult,
                CachedArgumentConversionKeyHash> conversion_cache;

  bool begin_candidate(SemanticContext & ctx,
                       FunctionBinding * candidate,
                       bool dedupe_equivalent)
  {
    if(!candidate || !considered.insert(candidate).second) {
      return false;
    }
    if(dedupe_equivalent &&
       contains_equivalent_function_candidate(seen_candidates, candidate)) {
      return false;
    }
    if(dedupe_equivalent) {
      note_function_candidate_bucket(seen_candidates, candidate);
    }
    note_overload_candidate_attempt(ctx);
    if(!candidate->is_constructor) {
      return false;
    }
    built_candidates.push_back(candidate);
    candidate_rejections.push_back(string());
    return true;
  }

  string & current_rejection()
  {
    return candidate_rejections.back();
  }
};

void reject_deleted_selected_constructor(FunctionBinding * chosen)
{
  if(!chosen || !chosen->is_deleted) {
    return;
  }
  throw logic_error("use of deleted " + chosen->name);
}

template <typename AppendCandidate>
void append_constructor_method_candidates(ClassInfo & info,
                                          AppendCandidate append_candidate)
{
  map<string, vector<FunctionBinding *> >::iterator found =
      info.methods.find(info.name);
  if(found != info.methods.end()) {
    for(size_t i = 0; i < found->second.size(); ++i) {
      if(found->second[i] && found->second[i]->source_template) {
        continue;
      }
      append_candidate(found->second[i]);
    }
    return;
  }

  for(map<string, vector<FunctionBinding *> >::iterator it = info.methods.begin();
      it != info.methods.end();
      ++it) {
    for(size_t i = 0; i < it->second.size(); ++i) {
      if(it->second[i] && it->second[i]->source_template) {
        continue;
      }
      append_candidate(it->second[i]);
    }
  }
}

vector<FunctionTemplateDecl *> collect_constructor_templates(ClassInfo & info)
{
  vector<FunctionTemplateDecl *> constructor_templates =
      lookup_direct_function_templates(*info.member_scope, info.name);
  for(map<string, vector<FunctionTemplateDecl *> >::const_iterator it =
          info.member_scope->function_templates.begin();
      it != info.member_scope->function_templates.end();
      ++it) {
    for(size_t i = 0; i < it->second.size(); ++i) {
      if(it->second[i] &&
         it->second[i]->is_constructor &&
         find(constructor_templates.begin(),
              constructor_templates.end(),
              it->second[i]) == constructor_templates.end()) {
        constructor_templates.push_back(it->second[i]);
      }
    }
  }
  return constructor_templates;
}

bool constructor_template_set_has_exact_owner(
    const vector<FunctionTemplateDecl *> & constructor_templates,
    const ClassInfo & info)
{
  return any_of(constructor_templates.begin(),
                constructor_templates.end(),
                [&info](FunctionTemplateDecl * decl)
                {
                  return decl && decl->declaring_scope &&
                         decl->declaring_scope->class_info == &info;
                });
}

bool constructor_template_set_accepts_argument_count(ClassInfo & info,
                                                     size_t argument_count)
{
  vector<FunctionTemplateDecl *> constructor_templates =
      collect_constructor_templates(info);
  const bool has_exact_constructor_template_owner =
      constructor_template_set_has_exact_owner(constructor_templates, info);
  for(size_t i = 0; i < constructor_templates.size(); ++i) {
    FunctionTemplateDecl * decl = constructor_templates[i];
    if(!decl) {
      continue;
    }
    if(has_exact_constructor_template_owner &&
       decl->declaring_scope &&
       decl->declaring_scope->class_info &&
       decl->declaring_scope->class_info != &info) {
      continue;
    }
    if(constructor_template_accepts_argument_count_fast(*decl, argument_count)) {
      return true;
    }
  }
  return false;
}

bool exact_constructor_match_may_be_beaten_by_constructor_template(
    const CandidateMatch & match)
{
  for(size_t i = 0; i < match.params.size(); ++i) {
    const ExprInfo & source_arg = source_arg_for_compare(match, i);
    TypePtr param_base = strip_top_level_cv(match.params[i]);
    if(!param_base || param_base->kind != Type::TK_LVALUE_REFERENCE) {
      continue;
    }
    if(source_arg.category != VC_LVALUE) {
      return true;
    }

    TypePtr param_unqualified;
    TypePtr source_unqualified;
    bool param_const = false;
    bool param_volatile = false;
    bool source_const = false;
    bool source_volatile = false;
    if(top_level_cv_flags(param_base->inner,
                          param_unqualified,
                          param_const,
                          param_volatile) &&
       top_level_cv_flags(remove_reference_type(source_arg.type),
                          source_unqualified,
                          source_const,
                          source_volatile) &&
       type_equals(strip_top_level_cv(param_unqualified),
                   strip_top_level_cv(source_unqualified)) &&
       ((param_const && !source_const) ||
        (param_volatile && !source_volatile))) {
      return true;
    }
  }
  return false;
}

bool constructor_template_pattern_is_initializer_list_candidate(
    SemanticContext & ctx,
    const FunctionTemplateDecl & decl)
{
  if(decl.params_pattern.empty()) {
    return false;
  }
  TypePtr first_param =
      strip_top_level_cv(remove_reference_type(decl.params_pattern[0].second));
  return ctx.is_initializer_list_type(first_param, nullptr, nullptr);
}

template <typename AppendCandidate>
void append_constructor_template_candidates(
    SemanticContext & ctx,
    Scope & scope,
    ClassInfo & info,
    const vector<ExprInfo> & source_args,
    const ConstructorSelectionOptions & options,
    bool overlay_local_named_types,
    const char * trace_label,
    vector<template_api::TemplateWitnessSourceDrop> & source_drops,
    AppendCandidate append_candidate)
{
  vector<FunctionTemplateDecl *> constructor_templates =
      collect_constructor_templates(info);
  const bool has_exact_constructor_template_owner =
      constructor_template_set_has_exact_owner(constructor_templates, info);
  for(size_t i = 0; i < constructor_templates.size(); ++i) {
    if(has_exact_constructor_template_owner &&
       constructor_templates[i] &&
       constructor_templates[i]->declaring_scope &&
       constructor_templates[i]->declaring_scope->class_info &&
       constructor_templates[i]->declaring_scope->class_info != &info) {
      continue;
    }
    if(!constructor_templates[i]) {
      continue;
    }
    if(constructor_templates[i]->is_explicit && !options.allow_explicit) {
      append_template_function_candidate_drop(ctx,
                                              constructor_templates[i],
                                              "explicit_constructor_not_allowed",
                                              &source_drops);
      continue;
    }
    if(options.initializer_list_only &&
       !constructor_template_pattern_is_initializer_list_candidate(
           ctx, *constructor_templates[i])) {
      continue;
    }
    if(!constructor_template_matches_source_args_fast(ctx,
                                                     *constructor_templates[i],
                                                     source_args)) {
      append_template_function_candidate_drop(
          ctx,
          constructor_templates[i],
          constructor_template_argument_count_drop_reason(*constructor_templates[i],
                                                         source_args.size()),
          &source_drops);
      continue;
    }

    Scope * constructor_use_scope_base =
        constructor_templates[i]->declaring_scope &&
                info.member_scope &&
                !scope_is_within(scope, constructor_templates[i]->declaring_scope) ?
            info.member_scope.get() :
            &scope;
    Scope constructor_use_scope(constructor_use_scope_base, "", false);
    constructor_use_scope.class_info = &info;
    constructor_use_scope.function = scope.function;
    if(constructor_use_scope_base != info.member_scope.get()) {
      semantic_template_function::overlay_instantiation_use_scope_bindings(
          constructor_use_scope,
          *info.member_scope,
          constructor_templates[i]->declaring_scope);
    }
    if(overlay_local_named_types) {
      const vector<TemplateArgument> local_type_arguments =
          constructor_deduction_local_type_arguments(ctx, source_args);
      const set<string> excluded_parameter_names =
          constructor_template_excluded_parameter_names(
              constructor_templates[i]->parameters);
      bind_constructor_local_named_types_from_args(ctx, constructor_use_scope, source_args);
      semantic_template_function::overlay_instantiation_local_named_types(
          ctx,
          constructor_use_scope,
          scope,
          constructor_templates[i]->declaring_scope,
          local_type_arguments,
          &excluded_parameter_names);
    }

    vector<TemplateArgument> deduced;
    map<string, size_t> pack_sizes;
    semantic_template_function::FunctionTemplateDeduction result;
    const bool deduced_ok =
        semantic_template_function::deduce_function_template_from_arguments(
            ctx, *constructor_templates[i], source_args, &constructor_use_scope, result);
    if(deduced_ok) {
      deduced.swap(result.arguments);
      pack_sizes.swap(result.pack_sizes);
    }
    if(parser_trace::enabled("template.resolve")) {
      std::ostringstream trace;
      trace << trace_label << " class=" << info.qualified_name
            << " template=" << constructor_templates[i]->name
            << " template_ptr=" << static_cast<void *>(constructor_templates[i])
            << " template_owner="
            << (constructor_templates[i]->declaring_scope &&
                        constructor_templates[i]->declaring_scope->class_info ?
                    constructor_templates[i]->declaring_scope->class_info->qualified_name :
                    std::string("<none>"))
            << " deduced_ok=" << (deduced_ok ? "yes" : "no")
            << " deduced_count=" << deduced.size();
      parser_trace::note("template.resolve", std::string(), trace.str());
    }
    if(!deduced_ok) {
      append_template_function_candidate_drop(ctx,
                                              constructor_templates[i],
                                              function_template_deduction_failure_drop_reason(
                                                  ctx,
                                                  *constructor_templates[i],
                                                  source_args,
                                                  false),
                                              &source_drops);
      continue;
    }
    FunctionBinding * binding = nullptr;
    try
    {
      const template_api::ScopedTemplateWitnessFunctionCallSourceCapturePause
          class_source_capture_pause;
      const std::string instantiation_use_location =
          constructor_witness_source_location(options, std::string(), source_args);
      binding = semantic_template_function::acquire_function_template_binding(
          ctx,
          *constructor_templates[i],
          deduced,
          &constructor_use_scope,
          &pack_sizes,
          false,
          nullptr,
          instantiation_use_location);
    }
    catch(const TemplateSubstitutionFailure &)
    {
      append_template_function_candidate_drop(ctx,
                                              constructor_templates[i],
                                              "substitution_failure",
                                              &source_drops);
      continue;
    }
    if(parser_trace::enabled("template.resolve")) {
      std::ostringstream trace;
      trace << "ctor-instantiated-binding class=" << info.qualified_name
            << " template=" << constructor_templates[i]->name;
      append_binding_trace_identity(trace, ctx, binding);
      parser_trace::note("template.resolve",
                         template_use_or_fallback_location(
                             candidate_primary_location(ctx, binding)),
                         trace.str());
    }
    if(options.initializer_list_only) {
      TypePtr binding_type = binding ? strip_top_level_cv(binding->type) : TypePtr();
      if(!binding_type || binding_type->kind != Type::TK_FUNCTION ||
         binding_type->params.size() <= 1 ||
         !ctx.is_initializer_list_type(
             strip_top_level_cv(remove_reference_type(binding_type->params[1])),
             nullptr,
             nullptr)) {
        continue;
      }
    }
    append_candidate(binding);
  }
}

template <typename AppendCandidate>
void append_constructor_template_node_candidates(
    SemanticContext & ctx,
    Scope & scope,
    ClassInfo & info,
    const vector<const CppAstNode *> & arg_nodes,
    SharedCallArgumentAnalyzer & argument_analyzer,
    const ConstructorSelectionOptions & options,
    const char * trace_label,
    vector<template_api::TemplateWitnessSourceDrop> & source_drops,
    AppendCandidate append_candidate)
{
  vector<FunctionTemplateDecl *> constructor_templates =
      collect_constructor_templates(info);
  const bool has_exact_constructor_template_owner =
      constructor_template_set_has_exact_owner(constructor_templates, info);
  for(size_t i = 0; i < constructor_templates.size(); ++i) {
    FunctionTemplateDecl * constructor_template = constructor_templates[i];
    if(has_exact_constructor_template_owner &&
       constructor_template &&
       constructor_template->declaring_scope &&
       constructor_template->declaring_scope->class_info &&
       constructor_template->declaring_scope->class_info != &info) {
      continue;
    }
    if(!constructor_template) {
      continue;
    }
    if(constructor_template->is_explicit && !options.allow_explicit) {
      append_template_function_candidate_drop(ctx,
                                              constructor_template,
                                              "explicit_constructor_not_allowed",
                                              &source_drops);
      continue;
    }
    if(options.initializer_list_only &&
       !constructor_template_pattern_is_initializer_list_candidate(
           ctx, *constructor_template)) {
      continue;
    }
    if(!constructor_template_accepts_argument_count_fast(*constructor_template,
                                                         arg_nodes.size())) {
      append_template_function_candidate_drop(
          ctx,
          constructor_template,
          constructor_template_argument_count_drop_reason(*constructor_template,
                                                         arg_nodes.size()),
          &source_drops);
      continue;
    }

    vector<ExprInfo> source_args;
    source_args.reserve(arg_nodes.size());
    bool args_ok = true;
    string arg_error;
    for(size_t j = 0; j < arg_nodes.size(); ++j) {
      const TypePtr target =
          j < constructor_template->params_pattern.size() ?
              constructor_template->params_pattern[j].second :
              TypePtr();
      try
      {
        ExprInfo reference_source;
        if(target && is_reference_type(target)) {
          if(argument_analyzer.analyze_reference_source(j, reference_source)) {
            source_args.push_back(reference_source);
            continue;
          }
        }
        ExprInfo arg;
        {
          ScopedCallSemConstructionPath arg_path(
              "overload.constructor-template.arg-analysis");
          arg = argument_analyzer.analyze_argument(
              j,
              target,
              should_use_template_deduction_target_aware_argument_analysis(
                  ctx, *arg_nodes[j], target));
        }
        source_args.push_back(arg);
      }
      catch(const logic_error & e)
      {
        args_ok = false;
        arg_error = e.what();
        break;
      }
    }
    if(!args_ok) {
      if(parser_trace::enabled("template.resolve")) {
        std::ostringstream trace;
        trace << "constructor-template name=" << constructor_template->name
              << " arg-analysis-failed=" << arg_error;
        parser_trace::note("template.resolve", std::string(), trace.str());
      }
      append_template_function_candidate_drop(ctx,
                                              constructor_template,
                                              "substitution_failure",
                                              &source_drops);
      continue;
    }

    Scope * constructor_use_scope_base =
        constructor_template->declaring_scope &&
                info.member_scope &&
                !scope_is_within(scope, constructor_template->declaring_scope) ?
            info.member_scope.get() :
            &scope;
    Scope constructor_use_scope(constructor_use_scope_base, "", false);
    constructor_use_scope.class_info = &info;
    constructor_use_scope.function = scope.function;
    if(constructor_use_scope_base != info.member_scope.get()) {
      semantic_template_function::overlay_instantiation_use_scope_bindings(
          constructor_use_scope,
          *info.member_scope,
          constructor_template->declaring_scope);
    }

    vector<TemplateArgument> deduced;
    map<string, size_t> pack_sizes;
    semantic_template_function::FunctionTemplateDeduction result;
    const bool deduced_ok =
        semantic_template_function::deduce_function_template_from_arguments(
            ctx, *constructor_template, source_args, &constructor_use_scope, result);
    if(deduced_ok) {
      deduced.swap(result.arguments);
      pack_sizes.swap(result.pack_sizes);
    }
    if(parser_trace::enabled("template.resolve")) {
      std::ostringstream trace;
      trace << trace_label << " class=" << info.qualified_name
            << " template=" << constructor_template->name
            << " template_ptr=" << static_cast<void *>(constructor_template)
            << " template_owner="
            << (constructor_template->declaring_scope &&
                        constructor_template->declaring_scope->class_info ?
                    constructor_template->declaring_scope->class_info->qualified_name :
                    std::string("<none>"))
            << " deduced_ok=" << (deduced_ok ? "yes" : "no")
            << " deduced_count=" << deduced.size();
      parser_trace::note("template.resolve", std::string(), trace.str());
    }
    if(!deduced_ok) {
      append_template_function_candidate_drop(ctx,
                                              constructor_template,
                                              function_template_deduction_failure_drop_reason(
                                                  ctx,
                                                  *constructor_template,
                                                  source_args,
                                                  false),
                                              &source_drops);
      continue;
    }

    FunctionBinding * binding = nullptr;
    try
    {
      const template_api::ScopedTemplateWitnessFunctionCallSourceCapturePause
          class_source_capture_pause;
      const std::string instantiation_use_location =
          constructor_witness_source_location(options, std::string(), source_args);
      binding = semantic_template_function::acquire_function_template_binding(
          ctx,
          *constructor_template,
          deduced,
          &constructor_use_scope,
          &pack_sizes,
          false,
          nullptr,
          instantiation_use_location);
    }
    catch(const TemplateSubstitutionFailure &)
    {
      append_template_function_candidate_drop(ctx,
                                              constructor_template,
                                              "substitution_failure",
                                              &source_drops);
      continue;
    }
    if(parser_trace::enabled("template.resolve")) {
      std::ostringstream trace;
      trace << "ctor-instantiated-binding class=" << info.qualified_name
            << " template=" << constructor_template->name;
      append_binding_trace_identity(trace, ctx, binding);
      parser_trace::note("template.resolve",
                         template_use_or_fallback_location(
                             candidate_primary_location(ctx, binding)),
                         trace.str());
    }
    if(options.initializer_list_only) {
      TypePtr binding_type = binding ? strip_top_level_cv(binding->type) : TypePtr();
      if(!binding_type || binding_type->kind != Type::TK_FUNCTION ||
         binding_type->params.size() <= 1 ||
         !ctx.is_initializer_list_type(
             strip_top_level_cv(remove_reference_type(binding_type->params[1])),
             nullptr,
             nullptr)) {
        continue;
      }
    }
    append_candidate(binding);
  }
}

bool analyze_default_argument_for_parameter(SemanticContext & ctx,
                                            Scope & decl_scope,
                                            const CppAstNode & payload,
                                            const TypePtr & param_type,
                                            ExprInfo & out)
{
  if(payload.kind != CppAstKind::braced_init_list) {
    out = ctx.analyze_expression_for_target(decl_scope, payload, param_type);
    return true;
  }

  TypePtr target_base = strip_top_level_cv(remove_reference_type(param_type));
  const bool target_is_initializer_list =
      ctx.is_initializer_list_type(target_base, nullptr, nullptr);
  const bool target_requires_typed_braces =
      target_is_initializer_list ||
      (target_base &&
       (target_base->kind == Type::TK_ARRAY ||
        ctx.complete_class_type(target_base)));
  if(target_requires_typed_braces) {
    const ConstructorSelectionOptions ctor_options =
        constructor_lifecycle_service::selection_options_for(
            constructor_lifecycle_service::copy_list_initialization_profile(
                "default argument copy-list-initialization"));
    return ctx.try_analyze_target_aware_expression(decl_scope,
                                                   payload,
                                                   param_type,
                                                   out,
                                                   &ctor_options);
  }

  if(payload.children.empty()) {
    out = ctx.make_value_initialized_expr(param_type);
    return true;
  }
  if(payload.children.size() != 1) {
    return false;
  }
  out = ctx.analyze_expression_for_target(decl_scope,
                                          payload.children[0],
                                          param_type);
  return true;
}

bool append_constructor_default_arguments(
    SemanticContext & ctx,
    Scope & conversion_scope,
    Scope & decl_scope,
    ClassInfo & info,
    FunctionBinding & candidate,
    const TypePtr & function_type,
    size_t first_param_index,
    const ConstructorSelectionOptions & options,
    CandidateMatch & match,
    string & candidate_rejection)
{
  for(size_t param_index = first_param_index;
      param_index < function_type->params.size();
      ++param_index) {
    const CppAstNode * default_arg =
        param_index < candidate.default_arguments.size() ?
            candidate.default_arguments[param_index] :
            nullptr;
    const CppAstNode * payload = default_argument_payload(default_arg);
    if(!payload) {
      candidate_rejection = candidate.name + ": bad default argument";
      return false;
    }
    if(payload->kind == CppAstKind::initializer) {
      if(payload->children.size() != 1) {
        if(parser_trace::enabled("overload")) {
          ostringstream trace;
          trace << "ctor-action-skip class=" << info.qualified_name
                << " candidate=" << candidate.name
                << " reason=bad-default-initializer-shape"
                << " index=" << param_index;
          parser_trace::note("overload", std::string(), trace.str());
        }
        candidate_rejection = candidate.name + ": bad default initializer shape";
        return false;
      }
      payload = &payload->children[0];
    }

    ExprInfo arg;
    ConversionRank rank = CR_BAD;
    try
    {
      ScopedSuppressedTemplateUseLocation suppressed_use_location;
      const template_api::ScopedTemplateWitnessFunctionCallSourceCapturePause
          suppress_default_argument_function_source_capture;
      if(!analyze_default_argument_for_parameter(ctx,
                                                 decl_scope,
                                                 *payload,
                                                 function_type->params[param_index],
                                                 arg)) {
        if(parser_trace::enabled("overload")) {
          ostringstream trace;
          trace << "ctor-action-skip class=" << info.qualified_name
                << " candidate=" << candidate.name
                << " reason=bad-default-argument"
                << " index=" << param_index;
          parser_trace::note("overload", std::string(), trace.str());
        }
        candidate_rejection = candidate.name + ": bad default argument";
        return false;
      }
    }
    catch(const logic_error &)
    {
      candidate_rejection = candidate.name + ": default analysis failed";
      if(parser_trace::enabled("overload")) {
        ostringstream trace;
        trace << "ctor-action-skip class=" << info.qualified_name
              << " candidate=" << candidate.name
              << " reason=default-analysis-failed"
              << " index=" << param_index;
        parser_trace::note("overload", std::string(), trace.str());
      }
      return false;
    }

    ExprInfo source_arg = arg;
    try
    {
      ArgumentConversionOptions conversion_options =
          semantic_policy::without_user_defined_body_instantiation(
              options.allow_explicit);
      conversion_options.materialize_standard_adjustments =
          !options.instantiate_bodies;
      if(!ctx.try_argument_conversion(conversion_scope,
                                      function_type->params[param_index],
                                      source_arg,
                                      arg,
                                      rank,
                                      conversion_options)) {
        ostringstream detail;
        detail << candidate.name << ": default arg " << param_index
               << " conversion failed param="
               << describe_type(function_type->params[param_index])
               << " arg="
               << (source_arg.type ? describe_type(source_arg.type) : string("<null>"))
               << " category=" << static_cast<int>(source_arg.category);
        candidate_rejection = detail.str();
        if(parser_trace::enabled("overload")) {
          ostringstream trace;
          trace << "ctor-action-skip class=" << info.qualified_name
                << " candidate=" << candidate.name
                << " reason=default-conversion-failed"
                << " index=" << param_index;
          parser_trace::note("overload", std::string(), trace.str());
        }
        return false;
      }
    }
    catch(const SemanticSoftFailure & e)
    {
      candidate_rejection = candidate.name + ": default arg " +
                            to_string(param_index) +
                            " conversion soft-failed: " + e.what();
      return false;
    }
    catch(const TemplateSubstitutionFailure & e)
    {
      candidate_rejection = candidate.name + ": default arg " +
                            to_string(param_index) +
                            " conversion substitution-failed: " + e.what();
      return false;
    }
    catch(const SemanticDiagnosticError & e)
    {
      candidate_rejection = candidate.name + ": default arg " +
                            to_string(param_index) +
                            " conversion diagnosed: " + e.what();
      return false;
    }

    match.ranks.push_back(rank);
    match.args.push_back(arg);
    match.call_args.push_back(arg);
    match.source_args.push_back(source_arg);
    match.params.push_back(function_type->params[param_index]);
  }
  return true;
}

}  // namespace

FunctionBinding * select_constructor_from_exprs(SemanticContext & ctx,
                                                Scope & scope,
                                                ClassInfo & info,
                                                const std::vector<ExprInfo> & source_args,
                                                std::vector<ExprInfo> & args_out,
                                                std::vector<ConversionRank> * ranks_out,
                                                const ConstructorSelectionOptions & options)
{
  ScopedCallSemConstructionPath construction_path("overload.select-constructor-from-exprs");
  DIAG_CONTEXT("select_constructor_from_exprs [" + info.qualified_name +
               ", args=" + to_string(source_args.size()) + "]");
  ClassInfo * target_info_ptr = canonicalize_constructor_target(ctx, scope, info);
  target_info_ptr = complete_constructor_target_if_ready(ctx, *target_info_ptr);
  ClassInfo & target_info = *target_info_ptr;
  if(semantic_hotspot::enabled()) {
    std::ostringstream query;
    query << target_info.qualified_name << "(";
    for(size_t i = 0; i < source_args.size(); ++i) {
      if(i != 0) {
        query << ", ";
      }
      query << (source_args[i].type ? describe_type(source_args[i].type) : std::string("<null>"))
            << "/" << static_cast<int>(source_args[i].category);
    }
    query << ")";
    semantic_hotspot::note_semantic_query("select_constructor_from_exprs", query.str());
  }
  if(source_args.size() == 1 && options.synthesize_implicit_copy_move) {
    semantic_class_model::ensure_implicit_copy_constructor(ctx, target_info);
    semantic_class_model::ensure_implicit_move_constructor(ctx, target_info);
  }

  semantic_class_model::ensure_implicit_special_members(ctx, target_info);
  const std::size_t aggregate_count =
      semantic_class_model::aggregate_element_count(target_info);
  if(source_args.size() == aggregate_count) {
    semantic_class_model::ensure_implicit_aggregate_constructor(ctx, target_info);
  } else if(options.allow_partial_aggregate && source_args.size() < aggregate_count) {
    semantic_class_model::ensure_implicit_aggregate_constructor(ctx,
                                                                target_info,
                                                                source_args.size());
  }
  note_overload_candidate_set(ctx);
  ConstructorSelectionState state;
  auto append_candidate = [&](FunctionBinding * candidate)
  {
    if(!state.begin_candidate(ctx, candidate, true)) {
      return;
    }
    string & candidate_rejection = state.current_rejection();
    if(binding_declares_explicit_function(*candidate) && !options.allow_explicit) {
      candidate_rejection = candidate->name + ": explicit constructor not allowed";
      return;
    }
    if(!member_access_allowed(&scope, current_class_scope(scope), current_function_scope(scope),
                              &target_info, candidate->access, MA_PUBLIC)) {
      candidate_rejection = candidate->name + ": member access not allowed";
      return;
    }
    if(!options.synthesize_implicit_copy_move &&
       constructor_is_class_copy_or_move_candidate(target_info, candidate)) {
      candidate_rejection = candidate->name + ": copy/move constructor not considered";
      return;
    }

    TypePtr function_type = strip_top_level_cv(candidate->type);
    if(!function_type || function_type->kind != Type::TK_FUNCTION ||
       function_type->params.empty()) {
      candidate_rejection = candidate->name + ": argument count/type shape mismatch";
      return;
    }

    const size_t explicit_param_offset = 1;
    if(!(function_type->variadic || function_type->prototype_relaxed) &&
       source_args.size() + explicit_param_offset > function_type->params.size()) {
      candidate_rejection = candidate->name + ": argument count/type shape mismatch";
      return;
    }
    size_t required_params = function_type->params.size();
    while(required_params > explicit_param_offset &&
          required_params - 1 < candidate->default_arguments.size() &&
          candidate->default_arguments[required_params - 1]) {
      --required_params;
    }
    if(source_args.size() + explicit_param_offset < required_params) {
      candidate_rejection = candidate->name + ": missing defaults";
      return;
    }

    CandidateMatch match;
    match.function = candidate;
    bool okay = true;
    for(size_t j = 0; okay && j < source_args.size(); ++j) {
      ExprInfo arg;
      ConversionRank rank = CR_BAD;
      ArgumentConversionOptions conversion_options(options.allow_user_defined,
                                                   false,
                                                   true,
                                                   options.allow_explicit);
      conversion_options.materialize_standard_adjustments =
          !options.instantiate_bodies;
      try
      {
        if(j + explicit_param_offset < function_type->params.size()) {
          if(!try_memoized_argument_conversion(
                 ctx,
                 scope,
                 function_type->params[j + explicit_param_offset],
                 source_args[j],
                 &source_args[j],
                 arg,
                 rank,
                 conversion_options,
                 state.conversion_cache)) {
            ostringstream detail;
            detail << candidate->name << ": argument " << j
                   << " conversion failed param="
                   << describe_type(function_type->params[j + explicit_param_offset])
                   << " arg="
                   << (source_args[j].type ? describe_type(source_args[j].type) :
                                             string("<null>"))
                   << " category=" << static_cast<int>(source_args[j].category);
            candidate_rejection = detail.str();
            okay = false;
            break;
          }
        } else {
          arg = source_args[j];
          rank = CR_ELLIPSIS;
        }
      }
      catch(const SemanticSoftFailure & e)
      {
        candidate_rejection = candidate->name + ": argument " + to_string(j) +
                              " conversion soft-failed: " + e.what();
        okay = false;
        break;
      }
      catch(const TemplateSubstitutionFailure & e)
      {
        candidate_rejection = candidate->name + ": argument " + to_string(j) +
                              " conversion substitution-failed: " + e.what();
        okay = false;
        break;
      }
      catch(const SemanticDiagnosticError & e)
      {
        candidate_rejection = candidate->name + ": argument " + to_string(j) +
                              " conversion diagnosed: " + e.what();
        okay = false;
        break;
      }
      match.ranks.push_back(rank);
      match.args.push_back(arg);
      match.call_args.push_back(source_args[j]);
      match.source_args.push_back(source_args[j]);
      match.params.push_back(
          j + explicit_param_offset < function_type->params.size() ?
              function_type->params[j + explicit_param_offset] :
              TypePtr());
    }
    match.explicit_arg_count = match.params.size();

    if(okay) {
      Scope & decl_scope = candidate->declaration_scope ? *candidate->declaration_scope : scope;
      okay = append_constructor_default_arguments(ctx,
                                                  scope,
                                                  decl_scope,
                                                  target_info,
                                                  *candidate,
                                                  function_type,
                                                  source_args.size() + explicit_param_offset,
                                                  options,
                                                  match,
                                                  candidate_rejection);
    }

    if(okay) {
      if(parser_trace::enabled("overload")) {
        ostringstream trace;
        trace << "ctor-action-accept class=" << target_info.qualified_name
              << " candidate=" << candidate->name
              << " arg_count=" << source_args.size()
              << " resolved_args=" << match.args.size();
        append_binding_trace_identity(trace, ctx, candidate);
        parser_trace::note("overload",
                           template_use_or_fallback_location(
                               candidate_primary_location(ctx, candidate)),
                           trace.str());
      }
      state.matches.push_back(std::move(match));
      note_overload_viable_candidate(ctx);
    }
  };

  append_constructor_method_candidates(target_info, append_candidate);

  if(!state.matches.empty()) {
    BestCandidateSelection exact_selection = select_best_candidate_match(ctx, state.matches);
    const bool selected_exact =
        !exact_selection.ambiguous &&
        candidate_match_is_all_exact(state.matches[exact_selection.index]);
    const bool constructor_template_may_beat_exact =
        selected_exact &&
        exact_constructor_match_may_be_beaten_by_constructor_template(
            state.matches[exact_selection.index]) &&
        constructor_template_set_accepts_argument_count(target_info, source_args.size());
    if(selected_exact && !constructor_template_may_beat_exact) {
      FunctionBinding * chosen = state.matches[exact_selection.index].function;
      if(options.instantiate_bodies &&
         !rematerialize_candidate_match_args(ctx,
                                             scope,
                                             state.matches[exact_selection.index],
                                             semantic_policy::rematerialization_conversion(options),
                                             false)) {
        throw logic_error("failed to rematerialize selected constructor conversions");
      }
      if(options.instantiate_bodies) {
        chosen = semantic_template_function::acquire_function_definition_binding(
            ctx, chosen, scope);
      }
      reject_deleted_selected_constructor(chosen);
      if(!constructor_selection_is_speculative_user_defined_conversion_probe(options) &&
         template_witness_source_capture_enabled_for_calls(ctx)) {
        const std::string witness_use_location =
            constructor_witness_source_location(options, std::string(), source_args);
        note_function_call_source_event(ctx,
                                        witness_use_location,
                                        target_info.name,
                                        nullptr,
                                        chosen,
                                        state.built_candidates,
                                        state.candidate_rejections,
                                        state.matches,
                                        exact_selection,
                                        state.source_drops,
                                        options.source_witness_direct_construction,
                                        0,
                                        state.built_candidates.size());
      }
      args_out = std::move(state.matches[exact_selection.index].args);
      if(ranks_out) {
        *ranks_out = std::move(state.matches[exact_selection.index].ranks);
      }
      return chosen;
    }
  }

  append_constructor_template_candidates(ctx,
                                         scope,
                                         target_info,
                                         source_args,
                                         options,
                                         true,
                                         "ctor-candidate",
                                         state.source_drops,
                                         append_candidate);

  if(state.matches.empty()) {
    if(parser_trace::enabled("overload") && !state.candidate_rejections.empty()) {
      for(size_t i = 0; i < state.candidate_rejections.size(); ++i) {
        ostringstream trace;
        trace << "ctor-action-reject class=" << target_info.qualified_name
              << " detail=" << state.candidate_rejections[i];
        parser_trace::note("overload", std::string(), trace.str());
      }
    }
    ostringstream out;
    out << "no viable constructor";
    out << " [class " << target_info.qualified_name << "]";
    out << " [arg_count " << source_args.size() << "]";
    if(!state.candidate_rejections.empty()) {
      out << " [rejections";
      for(size_t i = 0; i < state.candidate_rejections.size(); ++i) {
        out << (i == 0 ? " " : "; ") << state.candidate_rejections[i];
      }
      out << "]";
    }
    if(options.context && *options.context) {
      out << " [context " << options.context << "]";
    }
    throw NoViableConstructorError(out.str());
  }

  std::set<std::string> deduped_match_keys;
  vector<CandidateMatch> deduped_matches;
  for(size_t i = 0; i < state.matches.size(); ++i) {
    const std::string key =
        candidate_match_identity(ctx, state.matches[i], false, false);
    if(deduped_match_keys.insert(key).second) {
      deduped_matches.push_back(std::move(state.matches[i]));
    }
  }
  state.matches.swap(deduped_matches);

  BestCandidateSelection selection = select_best_candidate_match(ctx, state.matches);

  if(selection.ambiguous) {
    ostringstream out;
    out << "ambiguous constructor";
    out << " [class " << target_info.qualified_name << "]";
    out << " [matches";
    append_candidate_match_list(out, ctx, state.matches, false, false);
    out << "]";
    throw logic_error(out.str());
  }

  FunctionBinding * chosen = state.matches[selection.index].function;
  if(options.instantiate_bodies &&
     !rematerialize_candidate_match_args(ctx,
                                         scope,
                                         state.matches[selection.index],
                                         semantic_policy::rematerialization_conversion(options),
                                         false)) {
    throw logic_error("failed to rematerialize selected constructor conversions");
  }
  if(options.instantiate_bodies) {
    chosen = semantic_template_function::acquire_function_definition_binding(
        ctx, chosen, scope);
  }
  reject_deleted_selected_constructor(chosen);
  if(parser_trace::enabled("template.resolve")) {
    std::ostringstream trace;
    trace << "select-constructor-chosen class=" << target_info.qualified_name
          << " context="
          << (options.context && *options.context ? options.context : "<none>");
    append_binding_trace_identity(trace, ctx, chosen);
    parser_trace::note("template.resolve",
                       template_use_or_fallback_location(
                           candidate_primary_location(ctx, chosen)),
                       trace.str());
  }
  if(!constructor_selection_is_speculative_user_defined_conversion_probe(options) &&
     template_witness_source_capture_enabled_for_calls(ctx)) {
    const std::string witness_use_location =
        constructor_witness_source_location(options, std::string(), source_args);
    append_unmaterialized_copy_move_constructor_arity_drop(ctx,
                                                           target_info,
                                                           source_args.size(),
                                                           options,
                                                           chosen,
                                                           state.source_drops);
    note_function_call_source_event(ctx,
                                    witness_use_location,
                                    target_info.name,
                                    nullptr,
                                    chosen,
                                    state.built_candidates,
                                    state.candidate_rejections,
                                    state.matches,
                                    selection,
                                    state.source_drops,
                                    options.source_witness_direct_construction,
                                    0,
                                    state.built_candidates.size());
  }
  args_out = std::move(state.matches[selection.index].args);
  if(ranks_out) {
    *ranks_out = std::move(state.matches[selection.index].ranks);
  }
  return chosen;
}

FunctionBinding * select_constructor(SemanticContext & ctx,
                                     Scope & scope,
                                     ClassInfo & info,
                                     const std::vector<const CppAstNode *> & arg_nodes,
                                     std::vector<ExprInfo> & args_out,
                                     const ConstructorSelectionOptions & options)
{
  ScopedCallSemConstructionPath construction_path("overload.select-constructor");
  ClassInfo * target_info_ptr = canonicalize_constructor_target(ctx, scope, info);
  target_info_ptr = complete_constructor_target_if_ready(ctx, *target_info_ptr);
  ClassInfo & target_info = *target_info_ptr;
  vector<unique_ptr<CppAstNode> > expanded_arg_storage;
  vector<const CppAstNode *> expanded_arg_nodes;
  bool expanded_any_arg_nodes = false;
  for(size_t i = 0; i < arg_nodes.size(); ++i) {
    if(arg_nodes[i]->kind != CppAstKind::pack_expansion_expression) {
      expanded_arg_nodes.push_back(arg_nodes[i]);
      continue;
    }
    vector<CppAstNode> expanded_nodes;
    if(!ctx.expand_pack_argument_node(scope, *arg_nodes[i], expanded_nodes)) {
      throw logic_error("unsupported constructor pack-expansion argument");
    }
    expanded_any_arg_nodes = true;
    for(size_t j = 0; j < expanded_nodes.size(); ++j) {
      expanded_arg_storage.emplace_back(new CppAstNode(expanded_nodes[j]));
      expanded_arg_nodes.push_back(expanded_arg_storage.back().get());
    }
  }
  const vector<const CppAstNode *> & effective_arg_nodes =
      expanded_any_arg_nodes ? expanded_arg_nodes : arg_nodes;
  const std::string first_arg_location =
      !effective_arg_nodes.empty() ?
          ctx.source_location_for_node(*effective_arg_nodes.front()) :
          std::string();
  const std::string configured_constructor_use_location =
      constructor_selection_use_location(options);
  const std::string constructor_use_location =
      !configured_constructor_use_location.empty() ?
          configured_constructor_use_location :
          first_arg_location;
  ScopedTemplateUseLocation use_location_guard(constructor_use_location);

  if(effective_arg_nodes.size() == 1 &&
     !options.initializer_list_only &&
     options.synthesize_implicit_copy_move) {
    semantic_class_model::ensure_implicit_copy_constructor(ctx, target_info);
    semantic_class_model::ensure_implicit_move_constructor(ctx, target_info);
  }

  semantic_class_model::ensure_implicit_special_members(ctx, target_info);
  const std::size_t aggregate_count =
      semantic_class_model::aggregate_element_count(target_info);
  if(effective_arg_nodes.size() == aggregate_count) {
    semantic_class_model::ensure_implicit_aggregate_constructor(ctx, target_info);
  } else if(!effective_arg_nodes.empty() && effective_arg_nodes.size() < aggregate_count) {
    semantic_class_model::ensure_implicit_aggregate_constructor(ctx,
                                                                target_info,
                                                                effective_arg_nodes.size());
  }
  note_overload_candidate_set(ctx);
  ConstructorSelectionState state;
  const CallAnalysisOptions constructor_arg_options =
      semantic_policy::call_analysis(options.instantiate_bodies);
  SharedCallArgumentAnalyzer argument_analyzer(
      ctx,
      scope,
      effective_arg_nodes,
      constructor_arg_options);
  auto append_candidate = [&](FunctionBinding * candidate)
  {
    if(!state.begin_candidate(ctx, candidate, false)) {
      return;
    }
    string & candidate_rejection = state.current_rejection();
    if(binding_declares_explicit_function(*candidate) && !options.allow_explicit) {
      candidate_rejection = candidate->name + ": explicit constructor not allowed";
      if(parser_trace::enabled("overload")) {
        ostringstream trace;
        trace << "ctor-action-skip class=" << target_info.qualified_name
              << " candidate=" << candidate->name
              << " reason=explicit-constructor-not-allowed";
        parser_trace::note("overload", std::string(), trace.str());
      }
      return;
    }
    if(!member_access_allowed(&scope, current_class_scope(scope), current_function_scope(scope),
                              &target_info, candidate->access, MA_PUBLIC)) {
      candidate_rejection = candidate->name + ": inaccessible";
      if(parser_trace::enabled("overload")) {
        ostringstream trace;
        trace << "ctor-action-skip class=" << target_info.qualified_name
              << " candidate=" << candidate->name
              << " reason=inaccessible";
        parser_trace::note("overload", std::string(), trace.str());
      }
      return;
    }
    if(!options.synthesize_implicit_copy_move &&
       constructor_is_class_copy_or_move_candidate(target_info, candidate)) {
      candidate_rejection = candidate->name + ": copy/move constructor not considered";
      if(parser_trace::enabled("overload")) {
        ostringstream trace;
        trace << "ctor-action-skip class=" << target_info.qualified_name
              << " candidate=" << candidate->name
              << " reason=copy-move-not-considered";
        parser_trace::note("overload", std::string(), trace.str());
      }
      return;
    }

    TypePtr function_type = strip_top_level_cv(candidate->type);
    if(!function_type || function_type->kind != Type::TK_FUNCTION ||
       function_type->params.empty()) {
      candidate_rejection = candidate->name + ": invalid function type";
      if(parser_trace::enabled("overload")) {
        ostringstream trace;
        trace << "ctor-action-skip class=" << target_info.qualified_name
              << " candidate=" << candidate->name
              << " reason=invalid-function-type";
        parser_trace::note("overload", std::string(), trace.str());
      }
      return;
    }

    const size_t explicit_param_offset = 1;
    if(options.initializer_list_only &&
       (function_type->params.size() <= explicit_param_offset ||
        !ctx.is_initializer_list_type(
            strip_top_level_cv(remove_reference_type(
                function_type->params[explicit_param_offset])),
            nullptr,
            nullptr))) {
      candidate_rejection = candidate->name + ": not initializer_list ctor";
      if(parser_trace::enabled("overload")) {
        ostringstream trace;
        trace << "ctor-action-skip class=" << target_info.qualified_name
              << " candidate=" << candidate->name
              << " reason=not-initializer-list-ctor";
        parser_trace::note("overload", std::string(), trace.str());
      }
      return;
    }
    if(!(function_type->variadic || function_type->prototype_relaxed) &&
       effective_arg_nodes.size() + explicit_param_offset > function_type->params.size()) {
      candidate_rejection = candidate->name + ": too many args";
      if(parser_trace::enabled("overload")) {
        ostringstream trace;
        trace << "ctor-action-skip class=" << target_info.qualified_name
              << " candidate=" << candidate->name
              << " reason=too-many-args"
              << " arg_count=" << effective_arg_nodes.size()
              << " param_count=" << (function_type->params.size() - explicit_param_offset);
        parser_trace::note("overload", std::string(), trace.str());
      }
      return;
    }
    size_t required_params = function_type->params.size();
    while(required_params > explicit_param_offset &&
          required_params - 1 < candidate->default_arguments.size() &&
          candidate->default_arguments[required_params - 1]) {
      --required_params;
    }
    if(effective_arg_nodes.size() + explicit_param_offset < required_params) {
      candidate_rejection = candidate->name + ": missing defaults";
      if(parser_trace::enabled("overload")) {
        ostringstream trace;
        trace << "ctor-action-skip class=" << target_info.qualified_name
              << " candidate=" << candidate->name
              << " reason=missing-defaults"
              << " arg_count=" << effective_arg_nodes.size()
              << " required=" << (required_params - explicit_param_offset)
              << " param_count=" << (function_type->params.size() - explicit_param_offset);
        parser_trace::note("overload", std::string(), trace.str());
      }
      return;
    }

    CandidateMatch match;
    match.function = candidate;
    bool okay = true;
    string arg_error;
    for(size_t j = 0; okay && j < effective_arg_nodes.size(); ++j) {
      ExprInfo arg;
      try {
        ExprInfo source_arg;
        const ExprInfo * source_identity = nullptr;
        const bool has_fixed_param = j + explicit_param_offset < function_type->params.size();
        const TypePtr target =
            has_fixed_param ? function_type->params[j + explicit_param_offset] : TypePtr();
        const TypePtr target_base =
            target ? strip_top_level_cv(remove_reference_type(target)) : TypePtr();
        ClassInfo * target_class = target_base ? ctx.class_info_for_type(target_base) : nullptr;
        const bool needs_target_aware_analysis =
            has_fixed_param &&
            ((effective_arg_nodes[j]->kind == CppAstKind::lambda_expression &&
              target_class && target_class->is_lambda_closure) ||
             should_use_target_aware_argument_analysis(*effective_arg_nodes[j], target));
        if(needs_target_aware_analysis) {
          source_arg = argument_analyzer.analyze_argument(j, target, true);
        } else if(!argument_analyzer.analyze_generic_arg(j, source_arg, arg_error)) {
          candidate_rejection =
              candidate->name + ": arg analysis failed: " + arg_error;
          if(parser_trace::enabled("overload")) {
            ostringstream trace;
            trace << "ctor-action-skip class=" << target_info.qualified_name
                  << " candidate=" << candidate->name
                  << " reason=arg-analysis-failed"
                  << " index=" << j
                  << " detail=" << arg_error;
            parser_trace::note("overload", std::string(), trace.str());
          }
          okay = false;
          break;
        } else if(argument_analyzer.generic_arg_cache[j].state ==
                  CachedArgumentAnalysis::VALUE) {
          source_identity = &argument_analyzer.generic_arg_cache[j].value;
        }
        ConversionRank rank = CR_BAD;
        ArgumentConversionOptions conversion_options(true,
                                                    false,
                                                    true,
                                                    options.allow_explicit);
        conversion_options.materialize_standard_adjustments =
            !options.instantiate_bodies;
        try
        {
          if(has_fixed_param) {
            if(!try_memoized_argument_conversion(
                   ctx,
                   scope,
                   target,
                   source_arg,
                   source_identity,
                   arg,
                   rank,
                   conversion_options,
                   state.conversion_cache)) {
              ostringstream detail;
              detail << candidate->name << ": arg " << j
                     << " conversion failed param="
                     << describe_type(target)
                     << " arg="
                     << (source_arg.type ? describe_type(source_arg.type) : string("<null>"))
                     << " category=" << static_cast<int>(source_arg.category);
              candidate_rejection = detail.str();
              if(parser_trace::enabled("overload")) {
                ostringstream trace;
                trace << "ctor-action-skip class=" << target_info.qualified_name
                      << " candidate=" << candidate->name
                      << " reason=conversion-failed"
                      << " detail=" << candidate_rejection;
                parser_trace::note("overload", std::string(), trace.str());
              }
              okay = false;
              break;
            }
          } else {
            arg = source_arg;
            rank = CR_ELLIPSIS;
          }
        }
        catch(const SemanticSoftFailure & e)
        {
          candidate_rejection = candidate->name + ": arg " + to_string(j) +
                                " conversion soft-failed: " + e.what();
          okay = false;
          break;
        }
        catch(const TemplateSubstitutionFailure & e)
        {
          candidate_rejection = candidate->name + ": arg " + to_string(j) +
                                " conversion substitution-failed: " + e.what();
          okay = false;
          break;
        }
        catch(const SemanticDiagnosticError & e)
        {
          candidate_rejection = candidate->name + ": arg " + to_string(j) +
                                " conversion diagnosed: " + e.what();
          okay = false;
          break;
        }
        if(has_fixed_param &&
           needs_target_aware_analysis &&
           effective_arg_nodes[j]->kind == CppAstKind::braced_init_list &&
           target_base &&
           target_class &&
           !ctx.is_initializer_list_type(target_base, nullptr, nullptr) &&
           rank < CR_USER_DEFINED) {
          rank = CR_USER_DEFINED;
        }
        match.ranks.push_back(rank);
        match.call_args.push_back(source_arg);
        match.source_args.push_back(source_arg);
      } catch(const logic_error & e) {
        candidate_rejection = candidate->name + ": arg analysis failed: " + e.what();
        if(parser_trace::enabled("overload")) {
          ostringstream trace;
          trace << "ctor-action-skip class=" << target_info.qualified_name
                << " candidate=" << candidate->name
                << " reason=arg-analysis-failed"
                << " index=" << j
                << " detail=" << e.what();
          if(!arg_error.empty()) {
            trace << " detail=" << arg_error;
          }
          parser_trace::note("overload", std::string(), trace.str());
        }
        okay = false;
        break;
      }
      match.args.push_back(arg);
      match.params.push_back(
          j + explicit_param_offset < function_type->params.size() ?
              function_type->params[j + explicit_param_offset] :
              TypePtr());
    }
    match.explicit_arg_count = match.params.size();

    if(okay) {
      Scope & decl_scope = candidate->declaration_scope ? *candidate->declaration_scope : scope;
      okay = append_constructor_default_arguments(ctx,
                                                  scope,
                                                  decl_scope,
                                                  target_info,
                                                  *candidate,
                                                  function_type,
                                                  effective_arg_nodes.size() +
                                                      explicit_param_offset,
                                                  options,
                                                  match,
                                                  candidate_rejection);
    }

    if(okay) {
      if(parser_trace::enabled("overload")) {
        ostringstream trace;
        trace << "ctor-action-accept class=" << target_info.qualified_name
              << " candidate=" << candidate->name
              << " arg_count=" << effective_arg_nodes.size()
              << " resolved_args=" << match.args.size();
        append_binding_trace_identity(trace, ctx, candidate);
        parser_trace::note("overload",
                           template_use_or_fallback_location(
                               candidate_primary_location(ctx, candidate)),
                           trace.str());
      }
      state.matches.push_back(std::move(match));
      note_overload_viable_candidate(ctx);
    }
  };

  append_constructor_method_candidates(target_info, append_candidate);

  if(!state.matches.empty()) {
    BestCandidateSelection exact_selection = select_best_candidate_match(ctx, state.matches);
    const bool selected_exact =
        !exact_selection.ambiguous &&
        candidate_match_is_all_exact(state.matches[exact_selection.index]);
    const bool constructor_template_may_beat_exact =
        selected_exact &&
        exact_constructor_match_may_be_beaten_by_constructor_template(
            state.matches[exact_selection.index]) &&
        constructor_template_set_accepts_argument_count(target_info,
                                                        effective_arg_nodes.size());
    if(selected_exact && !constructor_template_may_beat_exact) {
      FunctionBinding * chosen =
          semantic_template_function::acquire_function_definition_binding(
              ctx,
              state.matches[exact_selection.index].function,
              scope);
      reject_deleted_selected_constructor(chosen);
      if(!rematerialize_candidate_match_args(ctx,
                                             scope,
                                             state.matches[exact_selection.index],
                                             semantic_policy::rematerialization_conversion(options),
                                             false)) {
        throw logic_error("failed to rematerialize selected constructor action");
      }
      if(template_witness_source_capture_enabled_for_calls(ctx)) {
        const std::string witness_use_location =
            constructor_witness_source_location(
                options,
                constructor_use_location,
                state.matches[exact_selection.index].source_args);
        append_unmaterialized_copy_move_constructor_arity_drop(ctx,
                                                               target_info,
                                                               effective_arg_nodes.size(),
                                                               options,
                                                               chosen,
                                                               state.source_drops);
        note_function_call_source_event(ctx,
                                        witness_use_location,
                                        target_info.name,
                                        nullptr,
                                        chosen,
                                        state.built_candidates,
                                        state.candidate_rejections,
                                        state.matches,
                                        exact_selection,
                                        state.source_drops,
                                        options.source_witness_direct_construction,
                                        0,
                                        state.built_candidates.size());
      }
      args_out = std::move(state.matches[exact_selection.index].args);
      return chosen;
    }
  }

  append_constructor_template_node_candidates(ctx,
                                              scope,
                                              target_info,
                                              effective_arg_nodes,
                                              argument_analyzer,
                                              options,
                                              "ctor-node-candidate",
                                              state.source_drops,
                                              append_candidate);

  if(state.matches.empty()) {
    ostringstream out;
    out << "no viable constructor";
    out << " [class " << target_info.qualified_name << "]";
    out << " [arg_count " << effective_arg_nodes.size() << "]";
    out << " [ctor-set";
    bool any = false;
    for(map<string, vector<FunctionBinding *> >::iterator it = target_info.methods.begin();
        it != target_info.methods.end();
        ++it) {
      for(size_t i = 0; i < it->second.size(); ++i) {
        FunctionBinding * candidate = it->second[i];
        if(!candidate->is_constructor) {
          continue;
        }
        out << (any ? "; " : " ");
        any = true;
        out << candidate->name << ":" << describe_type(candidate->type);
      }
    }
    if(!any) {
      out << " <none>";
    }
    out << "]";
    if(!state.candidate_rejections.empty()) {
      out << " [rejections";
      for(size_t i = 0; i < state.candidate_rejections.size(); ++i) {
        out << (i == 0 ? " " : "; ") << state.candidate_rejections[i];
      }
      out << "]";
    }
    if(options.context && *options.context) {
      out << " [context " << options.context << "]";
    }
    throw NoViableConstructorError(out.str());
  }

  std::set<std::string> deduped_match_keys;
  vector<CandidateMatch> deduped_matches;
  for(size_t i = 0; i < state.matches.size(); ++i) {
    const std::string key =
        candidate_match_identity(ctx, state.matches[i], false, false);
    if(deduped_match_keys.insert(key).second) {
      deduped_matches.push_back(std::move(state.matches[i]));
    }
  }
  state.matches.swap(deduped_matches);

  if(!options.initializer_list_only &&
     effective_arg_nodes.size() == 1 &&
     effective_arg_nodes[0]->kind == CppAstKind::braced_init_list) {
    vector<CandidateMatch> initializer_list_matches;
    for(size_t i = 0; i < state.matches.size(); ++i) {
      if(!state.matches[i].params.empty()) {
        TypePtr param = strip_top_level_cv(
            remove_reference_type(state.matches[i].params[0]));
        if(ctx.is_initializer_list_type(param, nullptr, nullptr)) {
          initializer_list_matches.push_back(std::move(state.matches[i]));
        }
      }
    }
    if(!initializer_list_matches.empty()) {
      state.matches.swap(initializer_list_matches);
    }
  }

  BestCandidateSelection selection = select_best_candidate_match(ctx, state.matches);

  if(selection.ambiguous) {
    ostringstream out;
    out << "ambiguous constructor";
    out << " [class " << target_info.qualified_name << "]";
    out << " [arg_count " << arg_nodes.size() << "]";
    out << " [matches";
    append_candidate_match_list(out, ctx, state.matches, false, false);
    out << "]";
    throw logic_error(out.str());
  }

  FunctionBinding * chosen =
      semantic_template_function::acquire_function_definition_binding(
          ctx, state.matches[selection.index].function, scope);
  reject_deleted_selected_constructor(chosen);
  if(!rematerialize_candidate_match_args(ctx,
                                         scope,
                                         state.matches[selection.index],
                                         semantic_policy::rematerialization_conversion(options),
                                         false)) {
    throw logic_error("failed to rematerialize selected constructor action");
  }
  if(template_witness_source_capture_enabled_for_calls(ctx)) {
    const std::string witness_use_location =
        constructor_witness_source_location(options,
                                            constructor_use_location,
                                            state.matches[selection.index].source_args);
    append_unmaterialized_copy_move_constructor_arity_drop(ctx,
                                                           target_info,
                                                           effective_arg_nodes.size(),
                                                           options,
                                                           chosen,
                                                           state.source_drops);
    note_function_call_source_event(ctx,
                                    witness_use_location,
                                    target_info.name,
                                    nullptr,
                                    chosen,
                                    state.built_candidates,
                                    state.candidate_rejections,
                                    state.matches,
                                    selection,
                                    state.source_drops,
                                    options.source_witness_direct_construction,
                                    0,
                                    state.built_candidates.size());
  }
  args_out = std::move(state.matches[selection.index].args);
  return chosen;
}

FunctionBinding * select_constructor_for_direct_braced_init(
    SemanticContext & ctx,
    Scope & scope,
    ClassInfo & info,
    const CppAstNode & direct_braced_init,
    std::vector<ExprInfo> & args_out,
    const ConstructorSelectionOptions & options)
{
  ScopedCallSemConstructionPath construction_path("overload.direct-braced-constructor");
  vector<const CppAstNode *> single_arg_node(1, &direct_braced_init);
  std::string single_arg_error;
  try
  {
    ConstructorSelectionOptions single_arg_options = options;
    single_arg_options.initializer_list_only = true;
    if(FunctionBinding * ctor =
           select_constructor(ctx,
                              scope,
                              info,
                              single_arg_node,
                              args_out,
                              single_arg_options)) {
      return ctor;
    }
  }
  catch(const SemanticSoftFailure & e)
  {
    single_arg_error = e.what();
  }

  vector<const CppAstNode *> expanded_arg_nodes =
      initializer_argument_nodes(direct_braced_init);
  if(expanded_arg_nodes.size() == 1 && expanded_arg_nodes[0] == &direct_braced_init) {
    if(!single_arg_error.empty()) {
      throw NoViableConstructorError(single_arg_error);
    }
    return nullptr;
  }
  try
  {
    ConstructorSelectionOptions expanded_options = options;
    expanded_options.initializer_list_only = false;
    if(expanded_arg_nodes.size() == 1 &&
       expanded_arg_nodes[0]->kind == CppAstKind::braced_init_list) {
      const vector<const CppAstNode *> nested_args =
          initializer_argument_nodes(*expanded_arg_nodes[0]);
      const bool nested_braces_may_copy_object =
          nested_args.size() == 1 &&
          nested_args[0]->kind != CppAstKind::braced_init_list;
      if(!nested_braces_may_copy_object) {
        expanded_options.synthesize_implicit_copy_move = false;
      }
    }
    if(expanded_arg_nodes.size() == 1 &&
       expanded_arg_nodes[0]->kind == CppAstKind::braced_init_list &&
       semantic_class_model::can_synthesize_aggregate_constructor(info) &&
       semantic_class_model::aggregate_element_count(info) == 1) {
      expanded_options.synthesize_implicit_copy_move = false;
    }
    return select_constructor(ctx,
                              scope,
                              info,
                              expanded_arg_nodes,
                              args_out,
                              expanded_options);
  }
  catch(const SemanticSoftFailure & e)
  {
    if(single_arg_error.empty()) {
      throw;
    }
    throw NoViableConstructorError(string(e.what()) +
                                   " [single-arg-direct-braced-init " +
                                   single_arg_error + "]");
  }
}

ExprInfo analyze_overloaded_assignment_expression(SemanticContext & ctx,
                                                  Scope & scope,
                                                  const CppAstNode & node,
                                                  const ExprInfo & lhs)
{
  ScopedCallSemConstructionPath construction_path("overload.assignment-expression");
  const bool instantiate_bodies =
      ctx.current_analysis_policy().instantiate_function_bodies;
  struct AssignmentCandidate
  {
    FunctionBinding * binding = nullptr;
    const ClassInfo * declared_in = nullptr;
    MemberAccess access = MA_PUBLIC;
    MemberAccess path_access = MA_PUBLIC;
  };

  auto invalid_assignment = [&node](const ExprInfo * lhs_expr = nullptr,
                                    const ExprInfo * rhs_expr = nullptr,
                                    const string & detail = string()) -> void
  {
    ostringstream out;
    out << "invalid assignment";
    out << " [expr " << node_text(node) << "]";
    if(node.has_token) {
      out << " [op " << node.value << "]";
    }
    if(lhs_expr) {
      out << " [lhs " << callsem_display_text(lhs_expr->node) << "]";
      out << " [lhs_type " << describe_type(lhs_expr->type) << "]";
    }
    if(rhs_expr) {
      out << " [rhs " << callsem_display_text(rhs_expr->node) << "]";
      out << " [rhs_type " << describe_type(rhs_expr->type) << "]";
    }
    if(!detail.empty()) {
      out << " [detail " << detail << "]";
    }
    throw logic_error(out.str());
  };

  if(node.children.size() != 2) {
    invalid_assignment(&lhs, nullptr, "assignment-expression arity");
  }

  TypePtr lhs_class_type = strip_top_level_cv(remove_reference_type(lhs.type));
  if(!lhs_class_type) {
    lhs_class_type = strip_top_level_cv(lhs.type);
  }
  ClassInfo * class_info = complete_class_type_for_lookup(ctx, lhs_class_type);
  if(!class_info) {
    invalid_assignment(&lhs, nullptr, "lhs is not class type");
  }

  const string operator_name = overloaded_assignment_operator_name(node);
  if(operator_name.empty()) {
    invalid_assignment(&lhs, nullptr, "unsupported overloaded assignment operator");
  }

  ExprInfo canonical_rhs;
  bool canonical_rhs_ready = false;
  try
  {
    if(node.children[1].kind == CppAstKind::lambda_expression) {
      canonical_rhs = semantic_expression::analyze_lambda_expression_as_closure(ctx,
                                                                                scope,
                                                                                node.children[1]);
    } else {
      canonical_rhs = ctx.analyze_expression(scope, node.children[1]);
    }
    canonical_rhs_ready = true;
  }
  catch(const logic_error &)
  {
  }

  semantic_class_model::ensure_implicit_special_members(ctx, *class_info);
  semantic_class_model::ensure_implicit_copy_assignment(ctx, *class_info);
  if(canonical_rhs_ready && canonical_rhs.category != VC_LVALUE) {
    semantic_class_model::ensure_implicit_move_assignment(ctx, *class_info);
  }
  MemberFunctionLookupResult candidates =
      lookup_visible_member_functions(*class_info, operator_name);
  MemberFunctionTemplateLookupResult template_candidates =
      lookup_visible_member_function_templates(*class_info, operator_name);
  if(candidates.functions.empty() && template_candidates.templates.empty()) {
    invalid_assignment(&lhs, nullptr, "no " + operator_name);
  }

  vector<AssignmentCandidate> assignment_candidates;
  assignment_candidates.reserve(candidates.functions.size() +
                                template_candidates.templates.size());
  vector<template_api::TemplateWitnessSourceDrop> assignment_source_drops;
  string template_source_args_error;
  for(size_t i = 0; i < candidates.functions.size(); ++i) {
    AssignmentCandidate candidate;
    candidate.binding = candidates.functions[i];
    candidate.declared_in = candidates.declared_in ? candidates.declared_in :
                                                    candidates.functions[i]->owner_class;
    candidate.access =
        candidate.declared_in && candidate.declared_in->member_scope ?
            effective_direct_function_access(*candidate.declared_in->member_scope,
                                             operator_name,
                                             *candidates.functions[i]) :
            candidates.functions[i]->access;
    candidate.path_access = candidates.path_access;
    assignment_candidates.push_back(candidate);
  }

  if(!template_candidates.templates.empty()) {
    vector<ExprInfo> template_source_args;
    bool template_source_args_ready = false;
    bool template_source_args_valid = true;
    const bool has_exact_template_owner =
        any_of(template_candidates.templates.begin(),
               template_candidates.templates.end(),
               [class_info](FunctionTemplateDecl * decl)
               {
                 return decl && decl->declaring_scope &&
                        decl->declaring_scope->class_info == class_info;
               });
    for(size_t i = 0; i < template_candidates.templates.size(); ++i) {
      FunctionTemplateDecl * decl = template_candidates.templates[i];
      if(!decl) {
        continue;
      }
      if(has_exact_template_owner &&
         decl->declaring_scope &&
         decl->declaring_scope->class_info &&
         decl->declaring_scope->class_info != class_info) {
        continue;
      }
      if(!template_source_args_ready) {
        template_source_args_ready = true;
        try
        {
          template_source_args.push_back(canonical_rhs_ready ?
                                             canonical_rhs :
                                             ctx.analyze_expression(scope,
                                                                    node.children[1]));
        }
        catch(const logic_error & e)
        {
          template_source_args_valid = false;
          template_source_args_error = e.what();
        }
      }
      if(!template_source_args_valid) {
        break;
      }

      Scope template_use_scope(&scope, "", false);
      template_use_scope.class_info = class_info;
      template_use_scope.function = scope.function;
      semantic_template_function::overlay_instantiation_use_scope_bindings(
          template_use_scope,
          *class_info->member_scope,
          decl->declaring_scope);
      vector<TemplateArgument> deduced;
      map<string, size_t> pack_sizes;
      semantic_template_function::FunctionTemplateDeduction result;
      bool deduced_ok = false;
      {
        const template_api::ScopedTemplateWitnessFunctionCallSourceCapturePause
            class_source_capture_pause;
        deduced_ok =
            semantic_template_function::deduce_function_template_from_arguments(
                ctx, *decl, template_source_args, &template_use_scope, result);
      }
      if(deduced_ok) {
        deduced.swap(result.arguments);
        pack_sizes.swap(result.pack_sizes);
      }
      if(!deduced_ok) {
        append_template_function_candidate_drop(ctx,
                                                decl,
                                                function_template_deduction_failure_drop_reason(
                                                    ctx, *decl, template_source_args, false),
                                                &assignment_source_drops);
        continue;
      }
      FunctionBinding * binding = nullptr;
      try
      {
        const template_api::ScopedTemplateWitnessFunctionCallSourceCapturePause
            class_source_capture_pause;
        binding = semantic_template_function::acquire_function_template_binding(
            ctx,
            *decl,
            deduced,
            &template_use_scope,
            &pack_sizes,
            false);
      }
      catch(const TemplateSubstitutionFailure &)
      {
        append_template_function_candidate_drop(ctx,
                                                decl,
                                                "substitution_failure",
                                                &assignment_source_drops);
        continue;
      }
      if(!binding) {
        continue;
      }
      AssignmentCandidate candidate;
      candidate.binding = binding;
      candidate.declared_in = template_candidates.declared_in ? template_candidates.declared_in :
                                                              binding->owner_class;
      candidate.access = binding->access;
      candidate.path_access = template_candidates.path_access;
      assignment_candidates.push_back(candidate);
    }
  }

  ExprInfo implicit_object_arg;
  {
    ScopedCallSemConstructionPath construction_path("overload.assignment-implicit-object");
    implicit_object_arg = ctx.make_address_of_expr(lhs);
  }
  note_overload_candidate_set(ctx);
  vector<CandidateMatch> matches;
  vector<string> candidate_rejections(assignment_candidates.size());
  for(size_t i = 0; i < assignment_candidates.size(); ++i) {
    FunctionBinding * candidate = assignment_candidates[i].binding;
    note_overload_candidate_attempt(ctx);
    TypePtr function_type = strip_top_level_cv(candidate->type);
    if(!candidate->is_method || !function_type || function_type->kind != Type::TK_FUNCTION ||
       function_type->params.size() != 2) {
      candidate_rejections[i] = "candidate shape mismatch";
      continue;
    }

    if(!member_access_allowed(&scope, current_class_scope(scope), current_function_scope(scope),
                              assignment_candidates[i].declared_in ?
                                  assignment_candidates[i].declared_in :
                                  candidate->owner_class,
                              assignment_candidates[i].access,
                              assignment_candidates[i].path_access)) {
      candidate_rejections[i] = "member access not allowed";
      continue;
    }

    CandidateMatch match;
    match.function = candidate;
    ExprInfo adjusted_this = implicit_object_arg;
    ExprInfo this_source_arg = implicit_object_arg;
    ExprInfo converted_this;
    const bool converted_this_ok =
        instantiate_bodies ?
            try_apply_unmaterialized_inheritance_conversion(
                ctx,
                function_type->params[0],
                implicit_object_arg,
                converted_this) :
            try_apply_inheritance_conversion(ctx,
                                             function_type->params[0],
                                             implicit_object_arg,
                                             converted_this);
    if(converted_this_ok) {
      adjusted_this = converted_this;
    }
    ConversionRank this_rank =
        implicit_object_conversion_rank(ctx, function_type->params[0], implicit_object_arg);
    if(this_rank == CR_BAD) {
      candidate_rejections[i] = "implicit object conversion failed";
      continue;
    }
    match.ranks.push_back(this_rank);
    match.args.push_back(adjusted_this);
    match.call_args.push_back(adjusted_this);
    match.source_args.push_back(this_source_arg);
    match.params.push_back(function_type->params[0]);
    match.list_initialization_args.push_back(false);
    match.list_initialization_element_ranks.push_back(
        vector<ConversionRank>());
    match.needs_rematerialization.push_back(instantiate_bodies &&
                                            converted_this_ok);

    ExprInfo rhs;
    ExprInfo source_rhs;
    ConversionRank rhs_rank = CR_BAD;
    try
    {
      if(!should_use_target_aware_argument_analysis(node.children[1],
                                                    function_type->params[1]) ||
         !ctx.try_analyze_target_aware_expression(scope,
                                                  node.children[1],
                                                  function_type->params[1],
                                                  source_rhs)) {
        source_rhs = ctx.analyze_expression(scope, node.children[1]);
      }
      ArgumentConversionOptions conversion_options =
          semantic_policy::without_user_defined_body_instantiation();
      conversion_options.materialize_standard_adjustments = false;
      if(!ctx.try_argument_conversion(scope,
                                      function_type->params[1],
                                      source_rhs,
                                      rhs,
                                      rhs_rank,
                                      conversion_options)) {
        candidate_rejections[i] = "argument conversion failed";
        continue;
      }
    }
    catch(const logic_error & e)
    {
      candidate_rejections[i] = string("argument analysis failed: ") + e.what();
      continue;
    }
    match.ranks.push_back(rhs_rank);
    match.args.push_back(rhs);
    match.call_args.push_back(rhs);
    match.source_args.push_back(source_rhs);
    match.params.push_back(function_type->params[1]);
    match.list_initialization_args.push_back(
        node.children[1].kind == CppAstKind::braced_init_list);
    vector<ConversionRank> element_ranks;
    collect_initializer_list_element_conversion_ranks(ctx,
                                                      scope,
                                                      node.children[1],
                                                      function_type->params[1],
                                                      element_ranks);
    match.list_initialization_element_ranks.push_back(
        std::move(element_ranks));
    matches.push_back(std::move(match));
    note_overload_viable_candidate(ctx);
  }

  FunctionCandidateBucketMap deduped_match_buckets;
  vector<CandidateMatch> deduped_matches;
  for(size_t i = 0; i < matches.size(); ++i) {
    if(!contains_equivalent_function_candidate(deduped_match_buckets,
                                               matches[i].function)) {
      FunctionBinding * function = matches[i].function;
      deduped_matches.push_back(std::move(matches[i]));
      note_function_candidate_bucket(deduped_match_buckets, function);
    }
  }
  matches.swap(deduped_matches);

  if(matches.empty()) {
    ostringstream detail;
    detail << "no viable operator= [candidates";
    if(assignment_candidates.empty()) {
      detail << " <none>";
    }
    for(size_t i = 0; i < assignment_candidates.size(); ++i) {
      FunctionBinding * candidate = assignment_candidates[i].binding;
      detail << (i == 0 ? " " : "; ");
      detail << candidate->name << ":" << describe_type(candidate->type);
      if(candidate->owner_class) {
        detail << " owner=" << candidate->owner_class->qualified_name;
      }
      if(!candidate_rejections[i].empty()) {
        detail << " reject=" << candidate_rejections[i];
      }
    }
    if(!template_candidates.templates.empty() &&
       assignment_candidates.size() == candidates.functions.size()) {
      detail << " [template-candidates " << template_candidates.templates.size()
             << " but none instantiated";
      if(!template_source_args_error.empty()) {
        detail << " source_error=" << template_source_args_error;
      }
      detail << "]";
    }
    detail << "]";
    invalid_assignment(&lhs, nullptr, detail.str());
  }

  if(node.children[1].kind == CppAstKind::braced_init_list) {
    vector<CandidateMatch> initializer_list_matches;
    for(size_t i = 0; i < matches.size(); ++i) {
      if(matches[i].params.size() > 1) {
        TypePtr param = strip_top_level_cv(remove_reference_type(matches[i].params[1]));
        if(ctx.is_initializer_list_type(param, nullptr, nullptr)) {
          initializer_list_matches.push_back(matches[i]);
        }
      }
    }
    if(!initializer_list_matches.empty()) {
      matches.swap(initializer_list_matches);
    }
  }

  BestCandidateSelection selection = select_best_candidate_match(ctx, matches);

  if(selection.ambiguous) {
    ostringstream out;
    out << "ambiguous overload";
    out << " [" << operator_name << " candidates";
    append_candidate_match_list(out, ctx, matches, false, false);
    out << "]";
    throw logic_error(out.str());
  }

  FunctionBinding * chosen = matches[selection.index].function;
  if(chosen->is_deleted) {
    ostringstream out;
    out << "use of deleted " << operator_name;
    out << " [selected ";
    append_function_candidate(out, ctx, chosen, &matches[selection.index].ranks);
    out << "]";
    throw logic_error(out.str());
  }
  if(chosen->source_template &&
     chosen->owner_class &&
     chosen->owner_class != class_info &&
     !matches[selection.index].needs_rematerialization.empty() &&
     matches[selection.index].needs_rematerialization[0]) {
    witness::append_source_drop(
        assignment_source_drops,
        function_binding_witness_name(ctx, chosen),
        function_binding_witness_decl_location(
            ctx,
            chosen,
            FunctionWitnessDeclLocationKind::CandidateDrop),
        "bad_conversion");
  }
  if(instantiate_bodies &&
     !rematerialize_candidate_match_args(ctx,
                                         scope,
                                         matches[selection.index],
                                         semantic_policy::default_argument_conversion(),
                                         false)) {
    throw_internal_error("failed to rematerialize selected assignment conversions",
                         std::string(),
                         "overload");
  }
  if(parser_trace::enabled("overload")) {
    const std::string assignment_location = ctx.source_location_for_node(node);
    for(size_t i = 0; i < assignment_candidates.size(); ++i) {
      FunctionBinding * candidate = assignment_candidates[i].binding;
      if(!candidate || candidate == chosen || candidate_rejections[i].empty()) {
        continue;
      }
      ostringstream drop_trace;
      drop_trace << "assignment-drop candidate=";
      append_function_candidate(drop_trace, ctx, candidate);
      drop_trace << " reason="
                 << coarse_assignment_drop_reason(candidate_rejections[i]);
      append_binding_trace_identity(drop_trace, ctx, candidate);
      parser_trace::note("overload", assignment_location, drop_trace.str());
    }
    for(size_t i = 0; i < matches.size(); ++i) {
      if(i == selection.index || !matches[i].function || matches[i].function == chosen) {
        continue;
      }
      ostringstream drop_trace;
      drop_trace << "assignment-drop candidate=";
      append_function_candidate(drop_trace, ctx, matches[i].function, &matches[i].ranks);
      drop_trace << " reason=worse_conversion";
      append_binding_trace_identity(drop_trace, ctx, matches[i].function);
      parser_trace::note("overload", assignment_location, drop_trace.str());
    }
    ostringstream trace;
    trace << "assignment-select"
          << " op=" << operator_name
          << " candidates_built=" << assignment_candidates.size()
          << " candidates_viable=" << matches.size()
          << " winner=";
    append_function_candidate(trace, ctx, chosen, &matches[selection.index].ranks);
    append_binding_trace_identity(trace, ctx, chosen);
    append_template_param_trace(trace, chosen);
    parser_trace::note("overload", assignment_location, trace.str());
  }
  TypePtr function_type = strip_top_level_cv(chosen->type);
  ValueCategory result_category = VC_PRVALUE;
  if(!result_value_category_for_function_result(function_type->inner, result_category)) {
    throw_internal_error("invalid function result", std::string(), "overload");
  }
  vector<FunctionBinding *> assignment_built_candidates;
  assignment_built_candidates.reserve(assignment_candidates.size());
  for(size_t i = 0; i < assignment_candidates.size(); ++i) {
    assignment_built_candidates.push_back(assignment_candidates[i].binding);
  }
  if(template_witness_source_capture_enabled_for_calls(ctx)) {
    note_function_call_source_event(ctx,
                                    ctx.source_location_for_node(node),
                                    operator_name,
                                    &node,
                                    chosen,
                                    assignment_built_candidates,
                                    candidate_rejections,
                                    matches,
                                    selection,
                                    assignment_source_drops,
                                    false,
                                    0,
                                    assignment_candidates.size());
  }
  return require_and_make_resolved_call_result(ctx,
                                               function_type->inner,
                                               result_category,
                                               *chosen,
                                               std::move(matches[selection.index].args),
                                               false);
}

ExprInfo analyze_call_expression(SemanticContext & ctx,
                                 Scope & scope,
                                 const CppAstNode & node,
                                 const CallAnalysisOptions & options)
{
  ScopedCallSemConstructionPath construction_path("overload.call-expression");
  const bool instantiate_bodies = options.instantiate_bodies;
  const CallAnalysisHints * hints = options.hints;
  const std::string hint_use_location =
      hints && !hints->use_location.empty() ?
          refine_fragment_use_location(ctx, node, hints->use_location) :
          std::string();
  ExprInfo declval_expr;
  std::string direct_declval_use_location =
      !hint_use_location.empty() ?
          hint_use_location :
          (hints && !hints->use_location.empty() ? hints->use_location : std::string());
  if(direct_declval_use_location.empty()) {
    direct_declval_use_location =
        refine_fragment_use_location(ctx, node, parser_trace::current_use_location());
  }
  if(try_analyze_declval_call_expression(ctx,
                                         scope,
                                         node,
                                         direct_declval_use_location,
                                         declval_expr)) {
    return declval_expr;
  }
  DIAG_CONTEXT("analyze_call_expression [" + node_text(node) + "]" +
               ctx.source_location_for_node(node));
  if(node.children.empty()) {
    throw_internal_error("unsupported callee", ctx.source_location_for_node(node), "overload");
  }

  const CppAstNode & callee_node = node.children[0];
  const CppAstNode * effective_callee_node = &callee_node;
  while(effective_callee_node->kind == CppAstKind::parenthesized_expression &&
        effective_callee_node->children.size() == 1) {
    effective_callee_node = &effective_callee_node->children[0];
  }
  const bool callee_name_was_parenthesized = effective_callee_node != &callee_node;
  const CppAstNode & lookup_callee_node = *effective_callee_node;
  std::vector<template_api::TemplateWitnessSourceDrop> direct_function_source_drops;
  QualifiedName direct_explicit_template_name;
  std::vector<std::string> explicit_template_arg_texts;
  std::size_t source_explicit_template_arg_count = 0;
  const TemplateIdSyntax * direct_template_id =
      lookup_callee_node.kind == CppAstKind::id_expression ?
          cppast_template_id_syntax(lookup_callee_node) :
          nullptr;
  const bool has_direct_explicit_template_args = direct_template_id != nullptr;
  if(direct_template_id) {
    direct_explicit_template_name = direct_template_id->name;
    explicit_template_arg_texts = direct_template_id->arguments;
    source_explicit_template_arg_count = explicit_template_arg_texts.size();
  }
  if(parser_trace::enabled("template.resolve")) {
    std::ostringstream trace;
    trace << "call-enter callee-kind=" << cppast_kind_text(lookup_callee_node.kind)
          << " callee-text=" << node_text(lookup_callee_node)
          << " scope=" << semantic_trace::scope_name_for_diagnostic(scope)
          << " bindings=" << semantic_trace::scope_bindings_for_diagnostic(scope);
    parser_trace::note("template.resolve", std::string(), trace.str());
  }
  const CppAstNode * argument_list = cpp_decl::find_child(node, CppAstKind::argument_list);
  if(!argument_list) {
    argument_list = cpp_decl::find_child(node, CppAstKind::paren_argument_list);
  }
  const CppAstNode * direct_braced_init = nullptr;
  if(argument_list &&
     argument_list->children.size() == 1 &&
     argument_list->children[0].kind == CppAstKind::braced_init_list &&
     !call_argument_list_is_parenthesized(ctx,
                                          lookup_callee_node,
                                          *argument_list)) {
    direct_braced_init = &argument_list->children[0];
  }
  vector<unique_ptr<CppAstNode> > expanded_arg_storage;
  vector<const CppAstNode *> arg_nodes;
  if(argument_list) {
    for(size_t i = 0; i < argument_list->children.size(); ++i) {
      const CppAstNode & arg_node = argument_list->children[i];
      if(arg_node.kind != CppAstKind::pack_expansion_expression) {
        arg_nodes.push_back(&arg_node);
        continue;
      }
      vector<CppAstNode> expanded_nodes;
      if(!ctx.expand_pack_argument_node(scope, arg_node, expanded_nodes)) {
        throw logic_error("unsupported pack-expansion argument");
      }
      for(size_t j = 0; j < expanded_nodes.size(); ++j) {
        expanded_arg_storage.emplace_back(new CppAstNode(expanded_nodes[j]));
        arg_nodes.push_back(expanded_arg_storage.back().get());
      }
    }
  }

  const auto note_direct_declval_source_call =
      [&](const CppAstNode & child) -> void
  {
    if(child.kind != CppAstKind::call_expression) {
      return;
    }
    if(!template_witness_source_capture_enabled_for_calls(ctx)) {
      return;
    }
    ExprInfo ignored_declval_expr;
    const std::string child_use_location =
        refine_fragment_use_location(ctx,
                                     child,
                                     !hint_use_location.empty() ?
                                         hint_use_location :
                                         parser_trace::current_use_location());
    const std::string child_declval_location =
        normalize_template_witness_location(
            source_location_for_name_in_subtree(ctx, child, "declval", false));
    std::string effective_child_use_location = child_use_location;
    if(effective_child_use_location.empty() &&
       source_location_points_at_identifier(ctx,
                                            child_declval_location,
                                            "declval")) {
      effective_child_use_location = child_declval_location;
    }
    try_analyze_declval_call_expression(ctx,
                                        scope,
                                        child,
                                        effective_child_use_location,
                                        ignored_declval_expr);
  };
  note_direct_declval_source_call(callee_node);
  for(size_t i = 0; i < arg_nodes.size(); ++i) {
    note_direct_declval_source_call(*arg_nodes[i]);
  }

  bool explicit_member_call = lookup_callee_node.kind == CppAstKind::member_expression;
  bool callable_object_call = false;
  ExprInfo implicit_object_arg;
  ValueCategory implicit_object_category = VC_LVALUE;
  vector<FunctionBinding *> candidates;
  MemberFunctionLookupResult member_candidates;
  string member_access_lookup_name;
  bool use_function_lookup = false;
  bool suppress_virtual_dispatch_for_qualified_id = false;
  TypePtr deferred_functional_cast_type;
  const auto record_functional_cast_class_use =
      [&](const TypePtr & target_type) -> void
  {
    if(target_type && lookup_callee_node.kind == CppAstKind::id_expression) {
      ctx.record_class_use_for_resolved_type_node(
          scope,
          lookup_callee_node,
          target_type,
          ctx.source_location_for_node(lookup_callee_node),
          true);
    }
  };
  const CallAnalysisHints * effective_hints = hints;
  CallAnalysisHints merged_lookup_hints;
  vector<ExprInfo> merged_lookup_arg_values;
  SharedCallArgumentAnalyzer argument_analyzer(ctx, scope, arg_nodes, options);
  ExprInfo hinted_member_implicit_object_arg;
  bool hinted_member_implicit_object_ready = false;
  const auto get_hinted_member_implicit_object_arg = [&]() -> const ExprInfo &
  {
    if(!hinted_member_implicit_object_ready) {
      if(!(hints && hints->explicit_member_base)) {
        throw logic_error("missing hinted member base");
      }
      ScopedCallSemConstructionPath construction_path(
          "overload.hinted-member-implicit-object");
      hinted_member_implicit_object_arg =
          ctx.make_address_of_expr(*hints->explicit_member_base);
      hinted_member_implicit_object_ready = true;
    }
    return hinted_member_implicit_object_arg;
  };
  ExprInfo implicit_this_arg;
  bool implicit_this_ready = false;
  bool implicit_this_available_checked = false;
  bool implicit_this_available = false;
  const auto has_implicit_this_arg = [&]() -> bool
  {
    if(!implicit_this_available_checked) {
      implicit_this_available_checked = true;
      for(Scope * current = &scope; current; current = current->parent) {
        if(!current->function) {
          continue;
        }
        implicit_this_available = current->values.find("this") != current->values.end();
        break;
      }
    }
    return implicit_this_available;
  };
  const auto get_implicit_this_arg = [&]() -> const ExprInfo &
  {
    if(!implicit_this_ready) {
      CppAstNode this_node;
      this_node.kind = CppAstKind::keyword_literal;
      this_node.has_token = true;
      this_node.token_kind = RT_SIMPLE;
      this_node.simple_type = KW_THIS;
      this_node.value = "this";
      ScopedCallSemConstructionPath construction_path("overload.implicit-this");
      implicit_this_arg = ctx.analyze_this_expression(scope, this_node);
      implicit_this_ready = true;
    }
    return implicit_this_arg;
  };
  ExprInfo qualified_id_implicit_object_arg;
  bool qualified_id_implicit_object_checked = false;
  bool qualified_id_implicit_object_available = false;
  const auto get_qualified_id_implicit_object_arg =
      [&](ExprInfo & out) -> bool
  {
    if(!qualified_id_implicit_object_checked) {
      qualified_id_implicit_object_checked = true;
      const QualifiedName * qualified =
          lookup_callee_node.kind == CppAstKind::id_expression ?
              cppast_qualified_name_syntax(lookup_callee_node) :
              nullptr;
      if(qualified && (qualified->rooted || !qualified->qualifiers.empty())) {
        Scope * target_scope =
            ctx.resolve_qualified_scope_for_node(scope,
                                                 *qualified,
                                                 lookup_callee_node,
                                                 false);
        ClassInfo * target_class =
            target_scope && target_scope->class_info ? target_scope->class_info : nullptr;
        const ExprInfo & source_this = get_implicit_this_arg();
        TypePtr source_pointer = strip_top_level_cv(source_this.type);
        ClassInfo * source_class = nullptr;
        if(source_pointer && source_pointer->kind == Type::TK_POINTER) {
          source_class =
              ctx.class_info_for_type(strip_top_level_cv(source_pointer->inner));
        }
        if(target_class && source_class) {
          if(target_class == source_class) {
            qualified_id_implicit_object_arg = source_this;
            qualified_id_implicit_object_available = true;
          } else {
            size_t offset = 0;
            MemberAccess access = MA_PUBLIC;
            try {
              if(find_unique_base_path(*source_class, target_class, offset, access)) {
                TypePtr target_object_type = target_class->type;
                TypePtr source_inner;
                bool cv_const = false;
                bool cv_volatile = false;
                if(source_pointer->kind == Type::TK_POINTER &&
                   top_level_cv_flags(source_pointer->inner,
                                      source_inner,
                                      cv_const,
                                      cv_volatile)) {
                  target_object_type =
                      apply_cv(target_object_type, cv_const, cv_volatile);
                }
                qualified_id_implicit_object_arg =
                    ctx.apply_base_subobject_adjustment(
                        source_this,
                        make_pointer(target_object_type),
                        *target_class,
                        offset);
                qualified_id_implicit_object_available = true;
              }
            } catch(const logic_error &) {
              qualified_id_implicit_object_available = false;
            }
          }
        }
      }
    }
    if(!qualified_id_implicit_object_available) {
      return false;
    }
    out = qualified_id_implicit_object_arg;
    return true;
  };
  if(const CppAstNode * conversion_type_id = cppast_conversion_type_id_syntax(node)) {
    TypePtr functional_cast_type;
    if(ctx.parse_type_id(scope, *conversion_type_id, functional_cast_type)) {
      record_functional_cast_class_use(functional_cast_type);
      return analyze_functional_cast(ctx,
                                     scope,
                                     functional_cast_type,
                                     arg_nodes,
                                     direct_braced_init,
                                     CallAnalysisOptions(instantiate_bodies));
    }
  }
  const auto required_parameter_count =
      [](const FunctionBinding & candidate, size_t arg_offset, size_t total_params) -> size_t
      {
        size_t required = total_params;
        while(required > arg_offset &&
              required - 1 < candidate.default_arguments.size() &&
              candidate.default_arguments[required - 1]) {
          --required;
        }
        return required;
      };
  const auto append_default_call_arguments =
      [&](CandidateMatch & match,
          const FunctionBinding & candidate,
          const TypePtr & function_type,
          size_t first_missing_param,
          string * failure_reason) -> bool
      {
        Scope & decl_scope =
            candidate.declaration_scope ? *candidate.declaration_scope : scope;
        for(size_t param_index = first_missing_param;
            param_index < function_type->params.size();
            ++param_index) {
          const CppAstNode * default_arg =
              param_index < candidate.default_arguments.size() ?
                  candidate.default_arguments[param_index] :
                  nullptr;
          if(!default_arg || default_arg->children.size() != 1) {
            if(failure_reason) {
              *failure_reason = string("missing default argument for parameter ") +
                                to_string(param_index);
            }
            return false;
          }
          const CppAstNode * payload = &default_arg->children[0];
          if(payload->kind == CppAstKind::initializer) {
            if(payload->children.size() != 1) {
              if(failure_reason) {
                *failure_reason = string("unsupported default initializer shape for parameter ") +
                                  to_string(param_index);
              }
              return false;
            }
            payload = &payload->children[0];
          }
          ExprInfo arg;
          ConversionRank rank = CR_BAD;
          try
          {
            ScopedSuppressedTemplateUseLocation suppressed_use_location;
            const template_api::ScopedTemplateWitnessFunctionCallSourceCapturePause
                suppress_default_argument_function_source_capture;
            if(!analyze_default_argument_for_parameter(ctx,
                                                       decl_scope,
                                                       *payload,
                                                       function_type->params[param_index],
                                                       arg)) {
              if(failure_reason) {
                *failure_reason = string("unsupported default argument for parameter ") +
                                  to_string(param_index);
              }
              return false;
            }
          }
          catch(const logic_error & e)
          {
            if(failure_reason) {
              *failure_reason = string("default argument analysis failed for parameter ") +
                                to_string(param_index) + ": " + e.what();
            }
            return false;
          }
          ExprInfo source_arg = arg;
          if(!ctx.try_argument_conversion(scope,
                                          function_type->params[param_index],
                                          source_arg,
                                          arg,
                                          rank,
                                          semantic_policy::without_user_defined_body_instantiation())) {
            if(failure_reason) {
              *failure_reason = string("default argument conversion failed for parameter ") +
                                to_string(param_index);
            }
            return false;
          }
          match.call_args.push_back(arg);
          match.source_args.push_back(source_arg);
        }
        return true;
      };
  const auto resolve_callee_expr_call =
      [&](const ExprInfo & callee_expr, ExprInfo & direct_result) -> bool
      {
        TypePtr function_type;
        if(!resolve_callable_function_type(callee_expr.type, function_type)) {
          TypePtr callable_object_type = remove_reference_type(callee_expr.type);
          if(!callable_object_type) {
            callable_object_type = callee_expr.type;
          }
          ClassInfo * callable_class =
              complete_class_type_for_lookup(ctx, callable_object_type);
          if(!callable_class) {
            ostringstream out;
            out << "callee expression is not callable";
            out << " [callee " << node_text(callee_node) << "]";
            out << " [type " << describe_type(callee_expr.type) << "]";
            out << " [category "
                << call_value_category_text(to_call_value_category(callee_expr.category)) << "]";
            throw logic_error(out.str());
          }
          MemberCallableLookupResult callable_candidates =
              lookup_visible_member_callables(*callable_class, "operator()");
          member_candidates.functions = callable_candidates.functions;
          member_candidates.declared_in = callable_candidates.declared_in;
          member_candidates.path_access = callable_candidates.path_access;
          member_candidates.path_offset = callable_candidates.path_offset;
          candidates = callable_candidates.functions;
          if(callable_candidates.declared_in && !callable_candidates.templates.empty()) {
            Scope member_template_scope(nullptr, "<member-templates>", false);
            member_template_scope.class_info =
                const_cast<ClassInfo *>(callable_candidates.declared_in);
            member_template_scope.function = scope.function;
            direct_function_template_slot(member_template_scope, "operator()") =
                callable_candidates.templates;
            CallAnalysisHints callable_template_hints =
                hints ? *hints : CallAnalysisHints();
            callable_template_hints.explicit_member_base = &callee_expr;
            callable_template_hints.explicit_member_arg_prefix = 0;
            callable_template_hints.explicit_member_declared_in =
                callable_candidates.declared_in;
            callable_template_hints.explicit_member_path_access =
                callable_candidates.path_access;
            append_function_template_call_candidates_impl(
                ctx,
                member_template_scope,
                scope,
                "operator()",
                arg_nodes,
                candidates,
                CallAnalysisOptions(instantiate_bodies, &callable_template_hints),
                &direct_function_source_drops);
          }
          if(candidates.empty()) {
            const auto try_resolve_conversion_function_pointer_call =
                [&]() -> bool
            {
              vector<CandidateMatch> surrogate_matches;
              vector<TypePtr> surrogate_callee_types;
              vector<TypePtr> surrogate_function_types;
              ExprInfo conversion_implicit_object_arg;
              bool conversion_implicit_object_ready = false;
              const auto get_conversion_implicit_object_arg =
                  [&]() -> const ExprInfo &
              {
                if(!conversion_implicit_object_ready) {
                  ScopedCallSemConstructionPath construction_path(
                      "overload.surrogate-call-implicit-object");
                  conversion_implicit_object_arg = ctx.make_address_of_expr(callee_expr);
                  conversion_implicit_object_ready = true;
                }
                return conversion_implicit_object_arg;
              };

              vector<MemberFunctionLookupResult> conversion_sets =
                  collect_visible_conversion_function_sets_for_call(ctx, *callable_class);
              for(size_t set_index = 0; set_index < conversion_sets.size(); ++set_index) {
                const MemberFunctionLookupResult & visible = conversion_sets[set_index];
                const ClassInfo * declared_in =
                    visible.declared_in ? visible.declared_in : callable_class;
                for(size_t i = 0; i < visible.functions.size(); ++i) {
                  FunctionBinding * candidate = visible.functions[i];
                  if(!candidate || !candidate->is_method ||
                     !ctx.is_conversion_function_name(candidate->name)) {
                    continue;
                  }

                  TypePtr conversion_function_type = strip_top_level_cv(candidate->type);
                  if(!conversion_function_type ||
                     conversion_function_type->kind != Type::TK_FUNCTION ||
                     conversion_function_type->params.size() != 1) {
                    continue;
                  }

                  TypePtr surrogate_function_type;
                  if(!resolve_callable_function_type(conversion_function_type->inner,
                                                     surrogate_function_type)) {
                    continue;
                  }
                  if(!surrogate_function_type ||
                     surrogate_function_type->kind != Type::TK_FUNCTION) {
                    continue;
                  }
                  if((!(surrogate_function_type->variadic ||
                        surrogate_function_type->prototype_relaxed) &&
                      arg_nodes.size() != surrogate_function_type->params.size()) ||
                     ((surrogate_function_type->variadic ||
                       surrogate_function_type->prototype_relaxed) &&
                      arg_nodes.size() < surrogate_function_type->params.size())) {
                    continue;
                  }

                  const ExprInfo & implicit_object_arg =
                      get_conversion_implicit_object_arg();
                  MemberAccess member_access =
                      declared_in && declared_in->member_scope ?
                          effective_direct_function_access(*declared_in->member_scope,
                                                           candidate->name,
                                                           *candidate) :
                          candidate->access;
                  const ClassInfo * access_root_class = nullptr;
                  TypePtr implicit_object_type = strip_top_level_cv(implicit_object_arg.type);
                  if(implicit_object_type &&
                     implicit_object_type->kind == Type::TK_POINTER) {
                    access_root_class = ctx.class_info_for_type(
                        strip_top_level_cv(implicit_object_type->inner));
                  }
                  if(!member_access_allowed_through_object(&scope,
                                                          current_class_scope(scope),
                                                          current_function_scope(scope),
                                                          access_root_class,
                                                          declared_in,
                                                          member_access,
                                                          visible.path_access)) {
                    continue;
                  }
                  if(ref_qualifier_rejects_implicit_object(
                         candidate->ref_qualifier,
                         conversion_function_type->params[0],
                         callee_expr.category)) {
                    continue;
                  }

                  ExprInfo adjusted_this = implicit_object_arg;
                  ExprInfo converted_this;
                  const bool converted_this_ok =
                      instantiate_bodies ?
                          semantic_conversion::
                              try_apply_unmaterialized_inheritance_conversion(
                                  ctx,
                                  conversion_function_type->params[0],
                                  implicit_object_arg,
                                  converted_this) :
                          semantic_conversion::try_apply_inheritance_conversion(
                              ctx,
                              conversion_function_type->params[0],
                              implicit_object_arg,
                              converted_this);
                  if(converted_this_ok) {
                    adjusted_this = converted_this;
                  } else if(visible.path_offset != 0 && declared_in) {
                    adjusted_this =
                        ctx.apply_base_subobject_adjustment(
                            implicit_object_arg,
                            conversion_function_type->params[0],
                            *declared_in,
                            visible.path_offset);
                  }

                  ConversionRank this_rank =
                      semantic_conversion::implicit_object_conversion_rank(
                          ctx,
                          conversion_function_type->params[0],
                          adjusted_this);
                  if(this_rank == CR_BAD) {
                    continue;
                  }

                  CandidateMatch match;
                  match.function = candidate;
                  match.ranks.push_back(this_rank);
                  match.args.push_back(adjusted_this);
                  match.source_args.push_back(implicit_object_arg);
                  match.source_arg_locations.push_back(std::string());
                  match.params.push_back(conversion_function_type->params[0]);

                  bool okay = true;
                  for(size_t arg_index = 0; arg_index < arg_nodes.size(); ++arg_index) {
                    ExprInfo arg;
                    ExprInfo source_arg;
                    ConversionRank rank = CR_EXACT;
                    try
                    {
                      if(arg_index < surrogate_function_type->params.size()) {
                        source_arg =
                            argument_analyzer.analyze_argument(
                                arg_index,
                                surrogate_function_type->params[arg_index],
                                true);
                        if(!ctx.try_argument_conversion(
                               scope,
                               surrogate_function_type->params[arg_index],
                               source_arg,
                               arg,
                               rank,
                               semantic_policy::without_user_defined_body_instantiation())) {
                          okay = false;
                          break;
                        }
                      } else {
                        source_arg =
                            argument_analyzer.analyze_argument(arg_index,
                                                               TypePtr(),
                                                               false);
                        arg = source_arg;
                        rank = CR_ELLIPSIS;
                      }
                    }
                    catch(const logic_error &)
                    {
                      okay = false;
                      break;
                    }

                    match.ranks.push_back(rank);
                    match.args.push_back(arg);
                    match.call_args.push_back(arg);
                    match.source_args.push_back(source_arg);
                    match.source_arg_locations.push_back(
                        ctx.source_location_for_node(*arg_nodes[arg_index]));
                    match.params.push_back(
                        arg_index < surrogate_function_type->params.size() ?
                            surrogate_function_type->params[arg_index] :
                            TypePtr());
                  }
                  if(!okay) {
                    continue;
                  }

                  surrogate_callee_types.push_back(conversion_function_type->inner);
                  surrogate_function_types.push_back(surrogate_function_type);
                  surrogate_matches.push_back(std::move(match));
                }
              }

              if(surrogate_matches.empty()) {
                return false;
              }

              BestCandidateSelection selection =
                  select_best_candidate_match(ctx, surrogate_matches);
              if(selection.ambiguous) {
                ostringstream out;
                out << "ambiguous conversion-function-pointer call";
                out << " [callee " << node_text(callee_node) << "]";
                out << " [matches";
                append_candidate_match_list(out, ctx, surrogate_matches, true, true);
                out << "]";
                throw logic_error(out.str());
              }

              const size_t selected_index = selection.index;
              CandidateMatch selected_match = std::move(surrogate_matches[selected_index]);
              TypePtr selected_callee_type = surrogate_callee_types[selected_index];
              TypePtr selected_function_type = surrogate_function_types[selected_index];
              ExprInfo converted_callee;
              ConversionRank callee_rank = CR_BAD;
              if(!ctx.try_argument_conversion(
                     scope,
                     selected_callee_type,
                     callee_expr,
                     converted_callee,
                     callee_rank,
                     semantic_policy::rematerialization_conversion(options))) {
                throw logic_error("failed to materialize conversion-function-pointer callee");
              }

              ValueCategory result_category = VC_PRVALUE;
              if(!result_value_category_for_function_result(selected_function_type->inner,
                                                            result_category)) {
                throw logic_error("invalid conversion-function-pointer call result");
              }
              direct_result = make_call_result(ctx,
                                               selected_function_type->inner,
                                               result_category,
                                               std::move(converted_callee.node),
                                               std::move(selected_match.call_args));
              return true;
            };
            if(try_resolve_conversion_function_pointer_call()) {
              return true;
            }
            ostringstream out;
            out << "callee expression is not callable";
            out << " [callee " << node_text(callee_node) << "]";
            out << " [type " << describe_type(callee_expr.type) << "]";
            out << " [category "
                << call_value_category_text(to_call_value_category(callee_expr.category)) << "]";
            out << " [callable_class " << callable_class->qualified_name << "]";
            out << " [methods";
            if(member_candidates.functions.empty()) {
              out << " <none>";
            } else {
              for(size_t i = 0; i < member_candidates.functions.size(); ++i) {
                out << (i == 0 ? " " : ",")
                    << describe_type(member_candidates.functions[i]->type);
              }
            }
            out << "]";
            out << " [member_templates";
            vector<FunctionTemplateDecl *> template_candidates =
                callable_class && callable_class->member_scope ?
                    lookup_direct_function_templates(*callable_class->member_scope,
                                                     "operator()") :
                    vector<FunctionTemplateDecl *>();
            if(template_candidates.empty()) {
              out << " <none>";
            } else {
              for(size_t i = 0; i < template_candidates.size(); ++i) {
                out << (i == 0 ? " " : ",")
                    << template_candidates[i]->name;
              }
            }
            out << "]";
            out << " [template_deduction";
            if(template_candidates.empty()) {
              out << " <none>";
            } else {
              vector<ExprInfo> debug_args;
              bool debug_args_ok = true;
              for(size_t i = 0; i < arg_nodes.size(); ++i) {
                try {
                  debug_args.push_back(ctx.analyze_expression(scope, *arg_nodes[i]));
                } catch(const logic_error & e) {
                  debug_args_ok = false;
                  out << " arg-error=" << e.what();
                  break;
                }
              }
              if(debug_args_ok) {
                for(size_t i = 0; i < template_candidates.size(); ++i) {
                  out << (i == 0 ? " " : "; ");
                  out << template_candidates[i]->name << ":";
                  vector<TemplateArgument> debug_deduced;
                  semantic_template_function::FunctionTemplateDeduction result;
                  if(!semantic_template_function::deduce_function_template_from_arguments(
                         ctx, *template_candidates[i], debug_args, &scope, result)) {
                    out << "deduce-fail";
                    continue;
                  }
                  debug_deduced.swap(result.arguments);
                  out << "deduced";
                  if(debug_deduced.empty()) {
                    out << " <empty>";
                  } else {
                    out << " ";
                    for(size_t j = 0; j < debug_deduced.size(); ++j) {
                      if(j != 0) {
                        out << ",";
                      }
                      out << debug_deduced[j].text;
                    }
                  }
                  try {
                    FunctionBinding * debug_binding =
                        semantic_template_function::acquire_function_template_binding(
                            ctx,
                            *template_candidates[i],
                            debug_deduced,
                            &scope,
                            &result.pack_sizes,
                            instantiate_bodies);
                    out << " -> "
                        << (debug_binding ? describe_type(debug_binding->type) :
                                            string("<null>"));
                  } catch(const logic_error & e) {
                    out << " instantiate=" << e.what();
                  }
                }
              }
            }
            out << "]";
            throw logic_error(out.str());
          }
          {
            ScopedCallSemConstructionPath construction_path(
                "overload.callable-object-implicit-object");
            implicit_object_arg = ctx.make_address_of_expr(callee_expr);
          }
          implicit_object_category = callee_expr.category;
          callable_object_call = true;
          return false;
        }
        FunctionBinding * direct_function_binding = nullptr;
        TypePtr direct_function_entity_type =
            strip_top_level_cv(remove_reference_type(callee_expr.type));
        const bool names_direct_function_entity =
            direct_function_entity_type &&
            direct_function_entity_type->kind == Type::TK_FUNCTION &&
            callee_expr.node.kind == CallSemKind::id_expression;
        if(names_direct_function_entity &&
           (callee_node.kind != CppAstKind::id_expression ||
            callee_expr.node.text.find("::") != string::npos)) {
          vector<FunctionBinding *> expr_bindings =
              ctx.lookup_functions(scope,
                                   callee_expr.node.text,
                                   semantic_policy::call_analysis(instantiate_bodies));
          for(size_t i = 0; i < expr_bindings.size(); ++i) {
            TypePtr candidate_type = strip_top_level_cv(expr_bindings[i]->type);
            if(candidate_type && type_equals(candidate_type, function_type)) {
              candidates.push_back(expr_bindings[i]);
              if(!direct_function_binding) {
                direct_function_binding = expr_bindings[i];
              }
            }
          }
          if(!candidates.empty()) {
            return false;
          }
        }
        if((!(function_type->variadic || function_type->prototype_relaxed) &&
            function_type->params.size() != arg_nodes.size()) ||
           ((function_type->variadic || function_type->prototype_relaxed) &&
            arg_nodes.size() < function_type->params.size())) {
          throw_user_error("no viable overload", ctx.source_location_for_node(node), "overload");
        }
        vector<ExprInfo> args;
        for(size_t i = 0; i < arg_nodes.size(); ++i) {
          ExprInfo arg;
          if(i < function_type->params.size()) {
            ExprInfo source_arg =
                argument_analyzer.analyze_argument(i, function_type->params[i], true);
            ConversionRank rank = CR_EXACT;
            if(!ctx.try_argument_conversion(scope,
                                            function_type->params[i],
                                            source_arg,
                                            arg,
                                            rank,
                                            semantic_policy::without_user_defined_body_instantiation())) {
              throw_user_error("no viable overload",
                               ctx.source_location_for_node(node),
                               "overload");
            }
          } else {
            arg = argument_analyzer.analyze_argument(i, TypePtr(), false);
          }
          args.push_back(arg);
        }

        ValueCategory result_category = VC_PRVALUE;
        if(!result_value_category_for_function_result(function_type->inner,
                                                      result_category)) {
          throw logic_error("invalid function result");
        }
        if(direct_function_binding) {
          if(direct_function_binding->is_deleted) {
            ostringstream out;
            out << "use of deleted " << direct_function_binding->name;
            throw logic_error(out.str());
          }
          direct_result = require_and_make_resolved_call_result(ctx,
                                                                function_type->inner,
                                                                result_category,
                                                                *direct_function_binding,
                                                                std::move(args),
                                                                false);
        } else {
          direct_result = make_call_result(ctx,
                                           function_type->inner,
                                           result_category,
                                           std::move(callee_expr.node),
                                           std::move(args));
        }
        return true;
      };
  const auto ordinary_lookup_suppresses_adl =
      [&](const string & name) -> bool
      {
        for(Scope * current = &scope; current; current = current->parent) {
          vector<FunctionBinding *> direct = lookup_direct_functions(*current, name);
          if(!direct.empty()) {
            if(current->class_info &&
               current->class_info->member_scope.get() == current) {
              return true;
            }
            if(current->parent != nullptr && !current->namespace_scope) {
              for(size_t i = 0; i < direct.size(); ++i) {
                if(direct[i] && direct[i]->declaration_scope == current) {
                  return true;
                }
              }
            }
            return false;
          }

          if(current->class_info) {
            if(!lookup_class_scoped_functions(*current->class_info, name).functions.empty()) {
              return true;
            }
            if(!lookup_member_functions(*current->class_info, name).functions.empty()) {
              return true;
            }
          }

          vector<FunctionBinding *> imported;
          set<const Scope *> visited;
          lookup_functions_from_using_directives(*current, name, visited, imported);
          if(!imported.empty()) {
            return false;
          }
        }

        return false;
      };
  const auto id_call_prefers_function_lookup =
      [&](const string & name) -> bool
      {
        for(Scope * current = &scope; current; current = current->parent) {
          if(semantic_lookup::lookup_direct_value(*current, name)) {
            return false;
          }

          const bool has_lexical_class =
              !current->class_info && current->function &&
              current->function->lexical_access_class;
          const bool lexical_only =
              has_lexical_class &&
              (!current->function->is_method ||
               current->function->owner_class != current->function->lexical_access_class);
          ClassInfo * lexical_class = current->class_info;
          if(!lexical_class && has_lexical_class) {
            lexical_class = current->function->lexical_access_class;
          }
          if(lexical_class) {
            if(lexical_class->member_scope) {
              map<string, ValueBinding>::const_iterator direct_value =
                  lexical_class->member_scope->values.find(name);
              if(direct_value != lexical_class->member_scope->values.end() &&
                 !(lexical_only && direct_value->second.kind == ValueBinding::VK_FIELD)) {
                return false;
              }
              const vector<FunctionBinding *> * direct_functions =
                  find_direct_function_set(*lexical_class->member_scope, name);
              if(direct_functions && !direct_functions->empty()) {
                return true;
              }
              const vector<FunctionTemplateDecl *> * direct_templates =
                  find_direct_function_template_set(*lexical_class->member_scope, name);
              if(direct_templates && !direct_templates->empty()) {
                return true;
              }
            }
            MemberValueLookupResult member = lookup_member_value(*lexical_class, name);
            if(lexical_only && member.binding && member.binding->kind == ValueBinding::VK_FIELD) {
              member.binding = nullptr;
            }
            if(member.binding) {
              return false;
            }
            if(!lookup_class_scoped_functions(*lexical_class, name).functions.empty() ||
               !lookup_member_functions(*lexical_class, name).functions.empty()) {
              return true;
            }
            if(lexical_class->member_scope &&
               !lookup_direct_function_templates(*lexical_class->member_scope, name).empty()) {
              return true;
            }
          }

          if(!lookup_direct_functions(*current, name).empty() ||
             !lookup_direct_function_templates(*current, name).empty()) {
            return true;
          }
        }
        return false;
      };
  const ValueBinding * value_callee = nullptr;
  bool resolved_non_id_callee_expr = false;
  const CppAstNode * bound_member_pointer_callee = effective_callee_node;
  if(bound_member_pointer_callee->kind == CppAstKind::binary_expression &&
     (node_has_simple_type(*bound_member_pointer_callee, OP_DOTSTAR) ||
      node_has_simple_type(*bound_member_pointer_callee, OP_ARROWSTAR))) {
    if(bound_member_pointer_callee->children.size() != 2) {
      throw logic_error("pointer-to-member call arity");
    }

    ExprInfo object_expr = ctx.analyze_expression(scope, bound_member_pointer_callee->children[0]);
    ExprInfo member_pointer_expr =
        ctx.analyze_expression(scope, bound_member_pointer_callee->children[1]);
    try {
      ExprInfo callee_expr = argument_analyzer.analyze_subexpression(callee_node);
      ExprInfo direct_result;
      if(resolve_callee_expr_call(callee_expr, direct_result)) {
        return direct_result;
      }
      resolved_non_id_callee_expr = callable_object_call || !candidates.empty();
    } catch(const logic_error &) {
      // A built-in pointer-to-member function access is only callable after this
      // special path supplies the implicit object argument below.
    }
    if(!resolved_non_id_callee_expr) {
    TypePtr member_pointer_type =
        strip_top_level_cv(remove_reference_type(member_pointer_expr.type));
    if(!member_pointer_type ||
       member_pointer_type->kind != Type::TK_MEMBER_POINTER ||
       !is_function_type(member_pointer_type->inner)) {
      throw logic_error("pointer-to-member call requires member function pointer");
    }

    TypePtr callable_function_type =
        callable_function_type_for_member_pointer(member_pointer_type);
    if(!callable_function_type || callable_function_type->kind != Type::TK_FUNCTION ||
       callable_function_type->params.empty()) {
      throw logic_error("invalid member function pointer call type");
    }

    const ValueCategory implicit_object_category =
        node_has_simple_type(*bound_member_pointer_callee, OP_DOTSTAR) ?
            object_expr.category :
            VC_LVALUE;
    if(semantic_conversion::member_pointer_ref_qualifier_rejects_object(
           callable_function_type->function_ref_qualifier,
           implicit_object_category)) {
      throw logic_error("pointer-to-member call object ref-qualifier mismatch");
    }

    ExprInfo implicit_object_expr;
    if(node_has_simple_type(*bound_member_pointer_callee, OP_DOTSTAR)) {
      {
        ScopedCallSemConstructionPath construction_path(
            "overload.member-pointer-implicit-object");
        implicit_object_expr = ctx.make_address_of_expr(object_expr);
      }
    } else {
      TypePtr object_type = strip_top_level_cv(remove_reference_type(object_expr.type));
      if(!object_type || object_type->kind != Type::TK_POINTER) {
        throw logic_error("pointer-to-member call requires object pointer");
      }
      implicit_object_expr = object_expr;
    }

    vector<ExprInfo> call_args;
    ConversionRank object_rank = CR_BAD;
    ExprInfo converted_object;
    if(!ctx.try_argument_conversion(scope,
                                    callable_function_type->params[0],
                                    implicit_object_expr,
                                    converted_object,
                                    object_rank,
                                    semantic_policy::rematerialization_conversion(options))) {
      throw logic_error("invalid pointer-to-member call object");
    }
    call_args.push_back(converted_object);

    if(!(callable_function_type->variadic || callable_function_type->prototype_relaxed) &&
       arg_nodes.size() + 1 != callable_function_type->params.size()) {
      throw logic_error("pointer-to-member call argument count mismatch");
    }
    if(arg_nodes.size() + 1 < callable_function_type->params.size()) {
      throw logic_error("pointer-to-member call missing arguments");
    }

    for(size_t i = 0; i < arg_nodes.size(); ++i) {
      const size_t param_index = i + 1;
      ExprInfo source_arg = argument_analyzer.analyze_argument(
          i,
          param_index < callable_function_type->params.size() ?
              callable_function_type->params[param_index] :
              TypePtr(),
          param_index < callable_function_type->params.size());
      if(param_index >= callable_function_type->params.size()) {
        call_args.push_back(source_arg);
        continue;
      }

      ExprInfo converted_arg;
      ConversionRank rank = CR_BAD;
      if(!ctx.try_argument_conversion(scope,
                                      callable_function_type->params[param_index],
                                      source_arg,
                                      converted_arg,
                                      rank,
                                      semantic_policy::rematerialization_conversion(options))) {
        throw logic_error("invalid pointer-to-member call argument");
      }
      call_args.push_back(converted_arg);
    }

    ExprInfo callee_expr = member_pointer_expr;
    callee_expr.type = make_pointer(callable_function_type);
    callee_expr.category = VC_PRVALUE;
    ctx.set_expr_info_metadata(callee_expr, callee_expr.type, callee_expr.category);
    set_callsem_materialization_source_type(callee_expr.node, member_pointer_expr.type);

    ValueCategory result_category = VC_PRVALUE;
    if(!result_value_category_for_function_result(callable_function_type->inner, result_category)) {
      throw logic_error("pointer-to-member call result category");
    }
    return make_call_result(ctx,
                            callable_function_type->inner,
                            result_category,
                            std::move(callee_expr.node),
                            std::move(call_args));
    }
  }
  if(explicit_member_call) {
    const CppAstNode & member_callee_node = lookup_callee_node;
    if(member_callee_node.children.size() != 2 ||
       member_callee_node.children[1].kind != CppAstKind::identifier) {
      throw logic_error("unsupported member call");
    }

    const bool destructor_member_call =
        is_scalar_pseudo_destructor_name(member_callee_node.children[1].value);
    const template_api::ScopedTemplateWitnessDeclvalCallSourceCapturePause
        declval_witness_pause(destructor_member_call);
    ExprInfo base = hints && hints->explicit_member_base ?
        *hints->explicit_member_base :
        ctx.analyze_expression(scope, member_callee_node.children[0]);
    TypePtr base_type = strip_top_level_cv(remove_reference_type(base.type));
    ClassInfo * class_info = nullptr;
    if(node_has_simple_type(member_callee_node, OP_DOT)) {
      if(base.category != VC_LVALUE &&
         base.category != VC_XVALUE &&
         base.category != VC_PRVALUE) {
        throw logic_error("dot requires class object");
      }
      class_info = complete_class_type_for_lookup(ctx, base_type);
      if(!class_info) {
        class_info = ctx.class_info_for_type(base_type);
      }
      {
        ScopedCallSemConstructionPath construction_path(
            "overload.member-call-implicit-object");
        implicit_object_arg = ctx.make_address_of_expr(base);
      }
      implicit_object_category = base.category;
    } else if(node_has_simple_type(member_callee_node, OP_ARROW)) {
      if(base_type && base_type->kind == Type::TK_POINTER) {
        class_info = complete_class_type_for_lookup(ctx, base_type->inner);
        if(!class_info) {
          class_info = ctx.class_info_for_type(base_type->inner);
        }
        implicit_object_arg = base;
        implicit_object_category = VC_LVALUE;
      } else {
        ClassInfo * base_class =
            base_type ? complete_class_type_for_lookup(ctx, base_type) : nullptr;
        if(!base_class && base_type) {
          base_class = ctx.class_info_for_type(base_type);
        }
        const bool has_member_arrow =
            base_class &&
            (!lookup_visible_member_functions(*base_class, "operator->").functions.empty() ||
             (base_class->member_scope &&
              !lookup_direct_function_templates(*base_class->member_scope, "operator->").empty()));
        if(has_member_arrow) {
          CppAstNode operator_call;
          operator_call.kind = CppAstKind::call_expression;
          operator_call.children.push_back(
              make_dot_member_operator_callee(member_callee_node.children[0], "operator->"));

          CppAstNode empty_args;
          empty_args.kind = CppAstKind::paren_argument_list;
          operator_call.children.push_back(empty_args);

          CppAstNode rewritten_callee = member_callee_node;
          rewritten_callee.children[0] = operator_call;

          CppAstNode rewritten_call = node;
          rewritten_call.children[0] = rewritten_callee;
          return analyze_call_expression(ctx,
                                         scope,
                                         rewritten_call,
                                         CallAnalysisOptions(instantiate_bodies));
        }
        throw logic_error("arrow requires pointer");
      }
    } else {
      throw logic_error("unsupported member access operator");
    }

    if(!class_info) {
      ExprInfo pseudo_destructor;
      if(try_analyze_scalar_pseudo_destructor_call(ctx,
                                                   scope,
                                                   node,
                                                   member_callee_node,
                                                   base,
                                                   pseudo_destructor)) {
        return pseudo_destructor;
      }
    }

    if(!class_info) {
      throw logic_error("member call requires class type");
    }

    const QualifiedName * member_name =
        cppast_qualified_name_syntax(member_callee_node.children[1]);
    if(!member_name) {
      throw logic_error("member-call target missing structured name");
    }

    QualifiedMemberTarget target;
    if(!resolve_qualified_member_target(ctx,
                                        scope,
                                        *class_info,
                                        *member_name,
                                        target,
                                        true,
                                        &member_callee_node.children[1])) {
      throw logic_error("unsupported qualified member call");
    }

    const TemplateIdSyntax * member_template_id =
        cppast_template_id_syntax(member_callee_node.children[1]);
    const TemplateIdSyntax * function_member_template_id =
        destructor_member_call ? nullptr : member_template_id;
    if(function_member_template_id) {
      source_explicit_template_arg_count =
          function_member_template_id->arguments.size();
    }
    const std::string member_lookup_name =
        function_member_template_id ?
            function_member_template_id->name.name : target.lookup_name;
    if(target.target_class && member_lookup_name == "operator=") {
      semantic_class_model::ensure_implicit_special_members(ctx,
                                                            *target.target_class);
      semantic_class_model::ensure_implicit_copy_assignment(ctx,
                                                            *target.target_class);
      semantic_class_model::ensure_implicit_move_assignment(ctx,
                                                            *target.target_class);
    }
    member_candidates = target.qualified ?
        lookup_visible_member_functions(*target.target_class, member_lookup_name) :
        lookup_visible_member_functions(*class_info, member_lookup_name);
    suppress_virtual_dispatch_for_qualified_id = target.qualified;
    member_access_lookup_name = member_lookup_name;
    member_candidates.path_access =
        combine_member_access(target.path_access, member_candidates.path_access);
    member_candidates.path_offset += target.path_offset;
    candidates = function_member_template_id ? vector<FunctionBinding *>() :
                                               member_candidates.functions;
    MemberCallableLookupResult member_callable_candidates;
    if(!target.qualified) {
      member_callable_candidates =
          lookup_visible_member_callables(*class_info, member_lookup_name);
      member_candidates.functions = member_callable_candidates.functions;
      member_candidates.declared_in = member_callable_candidates.declared_in;
      member_candidates.path_access =
          combine_member_access(target.path_access,
                                member_callable_candidates.path_access);
      member_candidates.path_offset = target.path_offset + member_callable_candidates.path_offset;
      candidates = function_member_template_id ? vector<FunctionBinding *>() :
                                                 member_callable_candidates.functions;
    } else {
      member_callable_candidates.functions = candidates;
      MemberFunctionTemplateLookupResult member_template_candidates =
          lookup_visible_member_function_templates(*target.target_class, member_lookup_name);
      member_callable_candidates.templates = member_template_candidates.templates;
      member_callable_candidates.declared_in = member_template_candidates.declared_in;
      member_callable_candidates.path_access = member_template_candidates.path_access;
      member_callable_candidates.path_offset = member_template_candidates.path_offset;
    }
    if(!member_callable_candidates.templates.empty()) {
      Scope member_template_scope(nullptr, "<member-templates>", false);
      member_template_scope.class_info =
          const_cast<ClassInfo *>(member_callable_candidates.declared_in ?
                                      member_callable_candidates.declared_in :
                                      target.target_class);
      member_template_scope.function = scope.function;
      direct_function_template_slot(member_template_scope, member_lookup_name) =
          member_callable_candidates.templates;
      CallAnalysisHints member_template_hints =
          hints ? *hints : CallAnalysisHints();
      const CallAnalysisHints * member_template_hints_ptr = hints;
      if(node_has_simple_type(member_callee_node, OP_DOT)) {
        member_template_hints.explicit_member_base = &base;
        member_template_hints.explicit_member_arg_prefix = 0;
        member_template_hints.explicit_member_declared_in =
            member_callable_candidates.declared_in ?
                member_callable_candidates.declared_in :
                target.target_class;
        member_template_hints.explicit_member_path_access =
            member_callable_candidates.path_access;
        member_template_hints_ptr = &member_template_hints;
      }
      append_function_template_call_candidates_impl(
          ctx,
          member_template_scope,
          scope,
          member_lookup_name,
          arg_nodes,
          candidates,
          CallAnalysisOptions(instantiate_bodies, member_template_hints_ptr),
          &direct_function_source_drops,
          &member_callee_node.children[1],
          member_name,
          function_member_template_id);
    }
    if(candidates.empty() && !function_member_template_id) {
      if(TypePtr conversion_target =
             explicit_conversion_member_target_type(ctx,
                                                    scope,
                                                    member_callee_node.children[1])) {
        ClassInfo * conversion_lookup_class =
            target.qualified ? target.target_class : class_info;
        MemberFunctionLookupResult conversion_candidates;
        if(conversion_lookup_class &&
           collect_explicit_conversion_member_call_candidates(ctx,
                                                             *conversion_lookup_class,
                                                             conversion_target,
                                                             conversion_candidates)) {
          conversion_candidates.path_access =
              combine_member_access(target.path_access,
                                    conversion_candidates.path_access);
          conversion_candidates.path_offset += target.path_offset;
          member_candidates = conversion_candidates;
          candidates = conversion_candidates.functions;
          bool same_lookup_name = !candidates.empty();
          for(size_t i = 1; i < candidates.size(); ++i) {
            if(candidates[i]->name != candidates[0]->name) {
              same_lookup_name = false;
              break;
            }
          }
          if(same_lookup_name) {
            member_access_lookup_name = candidates[0]->name;
          }
        }
      }
    }
    if(!target.qualified && candidates.size() > 1) {
      remove_hidden_using_base_member_function_candidates(candidates, *class_info);
    }
    if(candidates.empty() &&
       !looks_like_operator_function_name(target.lookup_name)) {
      try {
        ExprInfo field_callee = ctx.analyze_expression(scope, member_callee_node);
        ExprInfo direct_result;
        if(resolve_callee_expr_call(field_callee, direct_result)) {
          return direct_result;
        }
        explicit_member_call = false;
      }
      catch(const NotDataMemberExpressionError &) {}
    }
    if(candidates.empty()) {
      vector<string> available_methods;
      ClassInfo * available_class = target.target_class ? target.target_class : class_info;
      for(map<string, vector<FunctionBinding *> >::const_iterator it =
              available_class->methods.begin();
          it != available_class->methods.end();
          ++it) {
        if(!it->second.empty()) {
          available_methods.push_back(it->first + ":" + to_string(it->second.size()));
        }
      }
      throw logic_error(string("unknown member function [class ") +
                        available_class->qualified_name +
                        "] [member " + member_lookup_name +
                        "] [available " + join_string_list(available_methods, ",") + "]");
    }
  } else if(lookup_callee_node.kind == CppAstKind::id_expression) {
    const bool operator_function_call =
        looks_like_operator_function_name(
            semantic_utils::unqualified_member_name(lookup_callee_node.value));
    ExprInfo standard_char_traits_result;
    if(!operator_function_call &&
       try_analyze_standard_char_traits_call(ctx,
                                             scope,
                                             lookup_callee_node,
                                             arg_nodes,
                                             standard_char_traits_result)) {
      return standard_char_traits_result;
    }
    ExprInfo builtin_result;
    if(try_analyze_builtin_call_expression(ctx, scope, lookup_callee_node.value, arg_nodes,
                                           options,
                                           builtin_result)) {
      return builtin_result;
    }
    value_callee = operator_function_call ?
                       nullptr :
                       lookup_id_expression_value_binding_for_call(ctx,
                                                                   scope,
                                                                   lookup_callee_node);
    const QualifiedName * functional_cast_type_name =
        cppast_qualified_name_syntax(lookup_callee_node);
    const TemplateIdSyntax * functional_cast_template_id =
        cppast_template_id_syntax(lookup_callee_node);
    const QualifiedName * functional_cast_lookup_name =
        functional_cast_template_id ? &functional_cast_template_id->name :
                                      functional_cast_type_name;
    const bool unqualified_functional_cast_type_lookup_needed =
        value_callee &&
        functional_cast_lookup_name &&
        !functional_cast_lookup_name->rooted &&
        functional_cast_lookup_name->qualifiers.empty() &&
        unqualified_functional_cast_type_lookup_needed_for_value(
            scope, functional_cast_lookup_name->name, *value_callee);
    if((!value_callee || unqualified_functional_cast_type_lookup_needed) &&
       !operator_function_call) {
      if(parser_trace::enabled("template.resolve")) {
        std::ostringstream trace;
        trace << "call-id-type-lookup callee=" << lookup_callee_node.value
              << " scope="
              << semantic_trace::scope_name_for_diagnostic(scope)
              << " bindings="
              << semantic_trace::scope_bindings_for_diagnostic(scope);
        parser_trace::note("template.resolve", std::string(), trace.str());
      }
      deferred_functional_cast_type = lookup_callee_node.semantic_type;
      if(!deferred_functional_cast_type) {
        try {
          deferred_functional_cast_type =
              functional_cast_type_name || functional_cast_template_id ?
                  ctx.lookup_type_node(scope,
                                       lookup_callee_node,
                                       functional_cast_type_name ?
                                           qualified_name_lookup_text(*functional_cast_type_name) :
                                           lookup_callee_node.value) :
                  ctx.lookup_type(scope, lookup_callee_node.value);
        } catch(const TemplateSubstitutionFailure &) {
          deferred_functional_cast_type.reset();
        } catch(const SemanticSoftFailure &) {
          deferred_functional_cast_type.reset();
        } catch(const SemanticDiagnosticError &) {
          deferred_functional_cast_type.reset();
        }
      }
      if(deferred_functional_cast_type &&
         functional_cast_lookup_name &&
         !functional_cast_lookup_name->rooted &&
         functional_cast_lookup_name->qualifiers.empty() &&
         unqualified_functional_cast_type_hides_outer_functions(
             ctx,
             scope,
             functional_cast_lookup_name->name,
             deferred_functional_cast_type)) {
        record_functional_cast_class_use(deferred_functional_cast_type);
        return analyze_functional_cast(ctx,
                                       scope,
                                       deferred_functional_cast_type,
                                       arg_nodes,
                                       direct_braced_init,
                                       CallAnalysisOptions(instantiate_bodies));
      }
    }
    use_function_lookup =
        !value_callee ||
        operator_function_call ||
        id_call_prefers_function_lookup(lookup_callee_node.value);
    member_access_lookup_name = lookup_callee_node.value;
  }

  if(use_function_lookup) {
    const TemplateIdSyntax * callee_template_id =
        cppast_template_id_syntax(lookup_callee_node);
    candidates =
        callee_template_id ?
            ctx.lookup_function_template_id_node(
                scope,
                lookup_callee_node,
                *callee_template_id,
                semantic_policy::without_body_instantiation()) :
            ctx.lookup_functions_node(
                scope,
                lookup_callee_node,
                lookup_callee_node.value,
                semantic_policy::without_body_instantiation());
    append_function_template_call_candidates_impl(
        ctx,
        scope,
        scope,
        lookup_callee_node.value,
        arg_nodes,
        candidates,
        semantic_policy::call_analysis(instantiate_bodies, effective_hints),
        &direct_function_source_drops,
        &lookup_callee_node,
        cppast_qualified_name_syntax(lookup_callee_node),
        cppast_template_id_syntax(lookup_callee_node));
    const QualifiedName * qualified_name = cppast_qualified_name_syntax(lookup_callee_node);
    const bool is_qualified_lookup_name =
        qualified_name != nullptr &&
        (qualified_name->rooted || !qualified_name->qualifiers.empty());
    suppress_virtual_dispatch_for_qualified_id = is_qualified_lookup_name;
    const bool suppress_adl =
        (effective_hints && effective_hints->adl_candidates_precollected) ||
        callee_name_was_parenthesized ||
        (!is_qualified_lookup_name &&
         ordinary_lookup_suppresses_adl(lookup_callee_node.value));
    if(!is_qualified_lookup_name && !suppress_adl && !arg_nodes.empty()) {
      append_ordinary_call_adl_candidates(ctx,
                                          scope,
                                          lookup_callee_node,
                                          arg_nodes,
                                          has_direct_explicit_template_args,
                                          instantiate_bodies,
                                          effective_hints,
                                          merged_lookup_hints,
                                          merged_lookup_arg_values,
                                          argument_analyzer,
                                          candidates,
                                          &direct_function_source_drops);
    }
    if(parser_trace::enabled("template.resolve")) {
      std::ostringstream trace;
      trace << "call-post-lookup callee=" << lookup_callee_node.value
            << " candidate_count=" << candidates.size()
            << " deferred_cast=" << (deferred_functional_cast_type ? "yes" : "no");
      for(size_t i = 0; i < candidates.size(); ++i) {
        trace << " [" << i
              << " name=" << (candidates[i] ? candidates[i]->name : std::string("<null>"))
              << " method=" << ((candidates[i] && candidates[i]->is_method) ? "yes" : "no")
              << " ctor=" << ((candidates[i] && candidates[i]->is_constructor) ? "yes" : "no")
              << " type=" << ((candidates[i] && candidates[i]->type) ?
                                  describe_type(candidates[i]->type) :
                                  std::string("<none>"))
              << " internal="
              << ((candidates[i] && !candidates[i]->symbol.internal_symbol.empty()) ?
                      candidates[i]->symbol.internal_symbol :
                      std::string("<none>"))
              << " object="
              << ((candidates[i] && !candidates[i]->symbol.object_symbol.empty()) ?
                      candidates[i]->symbol.object_symbol :
                      std::string("<none>"))
              << "]";
      }
      parser_trace::note("template.resolve", std::string(), trace.str());
    }
    if(constructor_candidates_match_deferred_functional_cast(ctx,
                                                             deferred_functional_cast_type,
                                                             candidates)) {
      record_functional_cast_class_use(deferred_functional_cast_type);
      return analyze_functional_cast(ctx,
                                     scope,
                                     deferred_functional_cast_type,
                                     arg_nodes,
                                     direct_braced_init,
                                     CallAnalysisOptions(instantiate_bodies));
    }
    if(candidates.empty()) {
        if(deferred_functional_cast_type) {
          record_functional_cast_class_use(deferred_functional_cast_type);
          return analyze_functional_cast(ctx,
                                         scope,
                                         deferred_functional_cast_type,
                                         arg_nodes,
                                         direct_braced_init,
                                         CallAnalysisOptions(instantiate_bodies));
        }
        ostringstream out;
        out << "unknown function " << lookup_callee_node.value;
        if(ClassInfo * current_class = current_class_scope(scope)) {
          out << " [current_class " << current_class->qualified_name << "]";
          out << " [bases";
          if(current_class->bases.empty()) {
            out << " <none>";
          } else {
            for(size_t i = 0; i < current_class->bases.size(); ++i) {
              out << (i == 0 ? " " : ",")
                  << current_class->bases[i].type->qualified_name;
            }
          }
          out << "]";
          out << " [base_dependent ";
          out << (current_class->bases.empty() ? "<n/a>" :
                  (current_class->bases[0].type->dependent_instantiation ? "yes" : "no"));
          out << "]";
          out << " [base_placeholders ";
          out << (current_class->bases.empty() ? "<n/a>" :
                  (ctx.scope_has_template_placeholders(*current_class->bases[0].type->member_scope) ?
                       "yes" : "no"));
          out << "]";
          out << " [base_dependent_locals";
          if(current_class->bases.empty()) {
            out << " <n/a>";
          } else {
            Scope & base_scope = *current_class->bases[0].type->member_scope;
            bool any_local = false;
            for(const auto & named : base_scope.named_types) {
              if(ctx.type_depends_on_template_parameter(named.second)) {
                out << (any_local ? "," : " ") << "type=" << named.first;
                any_local = true;
              }
            }
            for(const auto & value : base_scope.values) {
              if(value.second.dependent_template_value ||
                 ctx.type_depends_on_template_parameter(value.second.type)) {
                out << (any_local ? "," : " ") << "value=" << value.first;
                any_local = true;
              }
            }
            if(!any_local) {
              out << " <none>";
            }
          }
          out << "]";
          out << " [base_functions";
          if(current_class->bases.empty() ||
             current_class->bases[0].type->member_scope->function_sets.empty()) {
            out << " <none>";
          } else {
            bool first = true;
            for(const auto & entry : current_class->bases[0].type->member_scope->function_sets) {
              out << (first ? " " : ",") << entry.first;
              first = false;
            }
          }
          out << "]";
          out << " [base_methods";
          if(current_class->bases.empty() || current_class->bases[0].type->methods.empty()) {
            out << " <none>";
          } else {
            bool first = true;
            for(const auto & entry : current_class->bases[0].type->methods) {
              out << (first ? " " : ",") << entry.first;
              first = false;
            }
          }
          out << "]";
          MemberFunctionLookupResult scoped =
              lookup_class_scoped_functions(*current_class, lookup_callee_node.value);
          out << " [class_scoped";
          if(scoped.functions.empty()) {
            out << " <none>";
          } else {
            for(size_t i = 0; i < scoped.functions.size(); ++i) {
              out << (i == 0 ? " " : ",") << scoped.functions[i]->name;
            }
          }
          out << "]";
          MemberFunctionLookupResult member =
              lookup_member_functions(*current_class, lookup_callee_node.value);
          out << " [member";
          if(member.functions.empty()) {
            out << " <none>";
          } else {
            for(size_t i = 0; i < member.functions.size(); ++i) {
              out << (i == 0 ? " " : ",") << member.functions[i]->name;
            }
          }
          out << "]";
          out << " [member_templates";
          if(!current_class->member_scope ||
             current_class->member_scope->function_templates.empty()) {
            out << " <none>";
          } else {
            bool first = true;
            for(const auto & entry : current_class->member_scope->function_templates) {
              if(entry.second.empty()) {
                continue;
              }
              out << (first ? " " : ",") << entry.first;
              first = false;
            }
            if(first) {
              out << " <none>";
            }
          }
          out << "]";
        }
        out << " [visible";
        bool any = false;
        for(Scope * current = &scope; current; current = current->parent) {
          if(current->function_sets.empty()) {
            continue;
          }
          out << (any ? "; " : " ");
          any = true;
          out << (current->name.empty() ? string("<anon>") : current->name) << ":";
          bool first = true;
          for(const auto & entry : current->function_sets) {
            out << (first ? "" : ",") << entry.first;
            first = false;
          }
        }
        if(!any) {
          out << " <none>";
        }
        out << "]";
        const QualifiedName * qualified_name = cppast_qualified_name_syntax(lookup_callee_node);
        if(qualified_name != nullptr &&
           (qualified_name->rooted || !qualified_name->qualifiers.empty())) {
          Scope * qualified_target =
              ctx.resolve_qualified_scope_for_node(scope,
                                                   *qualified_name,
                                                   lookup_callee_node,
                                                   false);
          out << " [qualified_target ";
          if(!qualified_target) {
            out << "<none>";
          } else {
            out << scope_qualified_name(*qualified_target, qualified_target->name);
          }
          out << "]";
          out << " [qualified_target_function_templates";
          if(!qualified_target || qualified_target->function_templates.empty()) {
            out << " <none>";
          } else {
            bool first = true;
            for(const auto & entry : qualified_target->function_templates) {
              out << (first ? " " : ",") << entry.first;
              first = false;
            }
          }
          out << "]";
          out << " [qualified_target_functions";
          if(!qualified_target || qualified_target->function_sets.empty()) {
            out << " <none>";
          } else {
            bool first = true;
            for(const auto & entry : qualified_target->function_sets) {
              out << (first ? " " : ",") << entry.first;
              first = false;
            }
          }
          out << "]";
        }
        throw UnknownFunctionError(out.str());
      }
  } else if(!explicit_member_call && !resolved_non_id_callee_expr) {
    ExprInfo callee_expr = argument_analyzer.analyze_subexpression(callee_node);
    ExprInfo direct_result;
    if(resolve_callee_expr_call(callee_expr, direct_result)) {
      return direct_result;
    }
  }

    note_overload_candidate_set(ctx);
    vector<CandidateMatch> matches;
    vector<size_t> match_candidate_indices;
    vector<string> candidate_rejections(candidates.size());
    vector<FunctionCandidateRefreshKey> candidate_refresh_keys(candidates.size());
    for(size_t i = 0; i < candidates.size(); ++i) {
      FunctionBinding * candidate = candidates[i];
      if(!ctx.function_binding_is_live(candidate)) {
        candidates[i] = nullptr;
        continue;
      }
      if(candidate->owner_class &&
         candidate->owner_class->reference_members_collected &&
         !candidate->owner_class->complete) {
        candidate_refresh_keys[i] = function_candidate_refresh_key(*candidate);
      }
    }
    const auto refresh_candidate_at = [&](size_t index) -> FunctionBinding *
    {
      FunctionBinding * candidate = candidates[index];
      const FunctionCandidateRefreshKey & key = candidate_refresh_keys[index];
      if(key.owner_class) {
        if(!function_candidate_matches_refresh_key(ctx, candidate, key)) {
          candidate = refresh_invalidated_member_candidate(ctx, key);
          if(!function_candidate_matches_refresh_key(ctx, candidate, key)) {
            candidate = nullptr;
          }
          candidates[index] = candidate;
          note_overload_candidate_refresh(ctx, candidate != nullptr);
          if(parser_trace::enabled("overload.refresh")) {
            std::ostringstream trace;
            trace << "candidate-refresh index=" << index
                  << " owner=" << key.owner_class->qualified_name
                  << " name=" << key.name
                  << " template=" << (key.has_source_template ? "yes" : "no")
                  << " result=" << (candidate ? "reacquired" : "missing");
            parser_trace::note("overload.refresh", std::string(), trace.str());
          }
        }
        return candidate;
      }
      return ctx.function_binding_is_live(candidate) ? candidate : nullptr;
    };
    map<string, CachedConstructorConversionResult> ctor_conversion_cache;
    unordered_map<CachedArgumentConversionKey,
                  CachedArgumentConversionResult,
                  CachedArgumentConversionKeyHash> conversion_cache;
    for(size_t i = 0; i < candidates.size(); ++i) {
      FunctionBinding * candidate = refresh_candidate_at(i);
      note_overload_candidate_attempt(ctx);
      if(!candidate || !candidate->type) {
        candidate_rejections[i] = "candidate missing type";
        continue;
      }
      TypePtr function_type = strip_top_level_cv(candidate->type);
      if(function_type->kind != Type::TK_FUNCTION) {
        candidate_rejections[i] = "candidate type is not function";
        continue;
      }
      if(function_template_specialization_retained_dependent_parameter(
             ctx,
             *candidate,
             function_type)) {
        candidate_rejections[i] = "substitution failure";
        continue;
      }

      CandidateMatch match;
      match.function = candidate;
      bool okay = true;
      size_t arg_offset = 0;
      size_t source_arg_begin = 0;
      if(candidate->is_method) {
        const bool has_hinted_member_base =
            hints &&
            hints->explicit_member_base &&
            hints->explicit_member_arg_prefix <= arg_nodes.size();
        if(has_hinted_member_base) {
          source_arg_begin = hints->explicit_member_arg_prefix;
        }

        const size_t explicit_arg_count = arg_nodes.size() - source_arg_begin;
        const size_t required_params =
            required_parameter_count(*candidate, 1, function_type->params.size());
        if((!(function_type->variadic || function_type->prototype_relaxed) &&
            (explicit_arg_count + 1 < required_params ||
             explicit_arg_count + 1 > function_type->params.size())) ||
           ((function_type->variadic || function_type->prototype_relaxed) &&
            explicit_arg_count + 1 < required_params)) {
          candidate_rejections[i] = "member argument count mismatch";
          continue;
        }

        if(explicit_member_call || callable_object_call) {
          // implicit_object_arg already prepared
        } else if(has_hinted_member_base) {
          implicit_object_arg = get_hinted_member_implicit_object_arg();
          implicit_object_category = hints->explicit_member_base->category;
        } else if(use_function_lookup) {
          if(!current_class_scope(scope) || !has_implicit_this_arg()) {
            okay = false;
          } else {
            implicit_object_arg = get_implicit_this_arg();
            ExprInfo qualified_object_arg;
            if(get_qualified_id_implicit_object_arg(qualified_object_arg)) {
              implicit_object_arg = qualified_object_arg;
            }
            implicit_object_category = VC_LVALUE;
          }
        } else {
          okay = false;
        }

        if(!okay) {
          candidate_rejections[i] = "member call requires implicit object";
          continue;
        }

        MemberAccess path_access = MA_PUBLIC;
        const ClassInfo * declared_in = candidate->owner_class;
        if(explicit_member_call) {
          path_access = member_candidates.path_access;
          declared_in = member_candidates.declared_in ? member_candidates.declared_in :
                                                        candidate->owner_class;
        } else if(hints &&
                  hints->explicit_member_base &&
                  hints->explicit_member_arg_prefix <= arg_nodes.size()) {
          path_access = hints->explicit_member_path_access;
          declared_in = hints->explicit_member_declared_in ? hints->explicit_member_declared_in :
                                                             candidate->owner_class;
        } else if(use_function_lookup && current_class_scope(scope)) {
          MemberFunctionLookupResult implicit_lookup =
              lookup_visible_member_functions(*current_class_scope(scope),
                                              lookup_callee_node.value);
          path_access = implicit_lookup.path_access;
          declared_in = implicit_lookup.declared_in ? implicit_lookup.declared_in :
                                                      candidate->owner_class;
        }
        MemberAccess member_access =
            declared_in && declared_in->member_scope && !member_access_lookup_name.empty() ?
                effective_direct_function_access(*declared_in->member_scope,
                                                 member_access_lookup_name,
                                                 *candidate) :
                candidate->access;
        const ClassInfo * access_root_class = nullptr;
        TypePtr implicit_object_type = strip_top_level_cv(implicit_object_arg.type);
        if(implicit_object_type && implicit_object_type->kind == Type::TK_POINTER) {
          access_root_class =
              ctx.class_info_for_type(strip_top_level_cv(implicit_object_type->inner));
        }
        if(!member_access_allowed_through_object(&scope,
                                                 current_class_scope(scope),
                                                 current_function_scope(scope),
                                                 access_root_class,
                                                 declared_in,
                                                 member_access,
                                                 path_access)) {
          candidate_rejections[i] = "member access not allowed";
          continue;
        }
        if(ref_qualifier_rejects_implicit_object(candidate->ref_qualifier,
                                                 function_type->params[0],
                                                 implicit_object_category)) {
          candidate_rejections[i] = "implicit object ref-qualifier mismatch";
          continue;
        }

        ExprInfo adjusted_this = implicit_object_arg;
        ExprInfo this_source_arg = implicit_object_arg;
        ExprInfo converted_this;
        const bool converted_this_ok =
            instantiate_bodies ?
                semantic_conversion::try_apply_unmaterialized_inheritance_conversion(
                    ctx,
                    function_type->params[0],
                    implicit_object_arg,
                    converted_this) :
                semantic_conversion::try_apply_inheritance_conversion(ctx,
                                                                      function_type->params[0],
                                                                      implicit_object_arg,
                                                                      converted_this);
        if(converted_this_ok) {
          adjusted_this = converted_this;
        }
        ConversionRank this_rank =
            semantic_conversion::implicit_object_conversion_rank(ctx,
                                                                 function_type->params[0],
                                                                 implicit_object_arg);
        if(this_rank == CR_BAD &&
           candidate->is_destructor &&
           destructor_implicit_object_cv_compatible(function_type->params[0],
                                                    implicit_object_arg)) {
          this_rank = CR_EXACT;
          ctx.set_expr_info_metadata(adjusted_this,
                                     function_type->params[0],
                                     adjusted_this.category);
        }
        if(this_rank == CR_BAD) {
          okay = false;
          candidate_rejections[i] = "implicit object conversion failed";
        } else {
          match.ranks.push_back(this_rank);
          match.args.push_back(adjusted_this);
          match.call_args.push_back(adjusted_this);
          match.source_args.push_back(this_source_arg);
          match.source_arg_locations.push_back(std::string());
          match.params.push_back(function_type->params[0]);
          match.needs_rematerialization.push_back(instantiate_bodies &&
                                                    converted_this_ok);
          match.list_initialization_args.push_back(false);
          match.list_initialization_element_ranks.push_back(
              vector<ConversionRank>());
          arg_offset = 1;
        }
      } else {
        const size_t required_params =
            required_parameter_count(*candidate, 0, function_type->params.size());
        if((!(function_type->variadic || function_type->prototype_relaxed) &&
            (arg_nodes.size() < required_params ||
             arg_nodes.size() > function_type->params.size())) ||
           ((function_type->variadic || function_type->prototype_relaxed) &&
            arg_nodes.size() < required_params)) {
          candidate_rejections[i] = "argument count mismatch";
          continue;
        }
      }

      for(size_t j = source_arg_begin; okay && j < arg_nodes.size(); ++j) {
        const size_t explicit_index = j - source_arg_begin;
        ExprInfo arg;
        ExprInfo source_arg;
        const ExprInfo * source_identity = nullptr;
        ConversionRank rank = CR_EXACT;
        try {
          if(explicit_index + arg_offset < function_type->params.size()) {
            const TypePtr target = function_type->params[explicit_index + arg_offset];
            TypePtr target_base = strip_top_level_cv(remove_reference_type(target));
            ClassInfo * target_class = target_base ? ctx.class_info_for_type(target_base) :
                                                     nullptr;
            ArgumentConversionOptions conversion_options(true, false);
            conversion_options.materialize_standard_adjustments = !instantiate_bodies;
            if(hints && hints->suppress_user_defined_output_materialization) {
              conversion_options.materialize_user_defined_output = false;
            }
            bool converted_from_member_pointer_option = false;
            vector<ExprInfo> member_pointer_options;
            if(target_member_function_pointer_type(target) &&
               collect_overloaded_member_pointer_argument_options(ctx,
                                                                  scope,
                                                                  *arg_nodes[j],
                                                                  member_pointer_options)) {
              bool found_member_pointer_option = false;
              bool ambiguous_member_pointer_option = false;
              ExprInfo best_arg;
              ExprInfo best_source_arg;
              ConversionRank best_rank = CR_BAD;
              for(size_t k = 0; k < member_pointer_options.size(); ++k) {
                ExprInfo option_arg;
                ConversionRank option_rank = CR_BAD;
                if(!try_cached_overload_argument_conversion(ctx,
                                                            scope,
                                                            target,
                                                            member_pointer_options[k],
                                                            nullptr,
                                                            option_arg,
                                                            option_rank,
                                                            conversion_options,
                                                            ctor_conversion_cache,
                                                            conversion_cache)) {
                  continue;
                }
                if(!found_member_pointer_option || option_rank < best_rank) {
                  found_member_pointer_option = true;
                  ambiguous_member_pointer_option = false;
                  best_rank = option_rank;
                  best_arg = option_arg;
                  best_source_arg = member_pointer_options[k];
                } else if(option_rank == best_rank) {
                  ambiguous_member_pointer_option = true;
                }
              }
              if(!found_member_pointer_option) {
                okay = false;
                candidate_rejections[i] = "argument conversion failed";
                break;
              }
              if(ambiguous_member_pointer_option) {
                okay = false;
                candidate_rejections[i] = "ambiguous overloaded member pointer argument";
                break;
              }
              arg = best_arg;
              source_arg = best_source_arg;
              rank = best_rank;
              converted_from_member_pointer_option = true;
            }
            if(!converted_from_member_pointer_option) {
              const bool needs_target_aware_analysis =
                  (arg_nodes[j]->kind == CppAstKind::lambda_expression &&
                   target_class && target_class->is_lambda_closure) ||
                  should_use_target_aware_argument_analysis(*arg_nodes[j], target);
              string arg_error;
              if(needs_target_aware_analysis) {
                source_arg = argument_analyzer.analyze_argument(j, target, true);
              } else if(!argument_analyzer.analyze_generic_arg(j, source_arg, arg_error)) {
                okay = false;
                candidate_rejections[i] =
                    string("argument analysis failed: ") + arg_error;
                break;
              } else if(argument_analyzer.generic_arg_cache[j].state ==
                        CachedArgumentAnalysis::VALUE) {
                source_identity = &argument_analyzer.generic_arg_cache[j].value;
              }
              if(!try_cached_overload_argument_conversion(
                     ctx,
                     scope,
                     target,
                     source_arg,
                     source_identity,
                     arg,
                     rank,
                     conversion_options,
                     ctor_conversion_cache,
                     conversion_cache)) {
                okay = false;
                candidate_rejections[i] = "argument conversion failed";
                break;
              }
              if(needs_target_aware_analysis &&
                 arg_nodes[j]->kind == CppAstKind::braced_init_list &&
                 target_base &&
                 target_class &&
                 !ctx.is_initializer_list_type(target_base, nullptr, nullptr) &&
                 rank < CR_USER_DEFINED) {
                rank = CR_USER_DEFINED;
              }
            }
          } else {
            string arg_error;
            if(!argument_analyzer.analyze_generic_arg(j, arg, arg_error)) {
              okay = false;
              candidate_rejections[i] =
                  string("argument analysis failed: ") + arg_error;
              break;
            }
            rank = CR_ELLIPSIS;
            source_arg = arg;
          }
        } catch(const logic_error & e) {
          okay = false;
          candidate_rejections[i] = string("argument analysis failed: ") + e.what();
          break;
        }

        match.ranks.push_back(rank);
        match.args.push_back(arg);
        match.call_args.push_back(arg);
        match.source_args.push_back(source_arg);
        match.source_arg_locations.push_back(
            ctx.source_location_for_node(*arg_nodes[j]));
        match.list_initialization_args.push_back(
            arg_nodes[j]->kind == CppAstKind::braced_init_list);
        if(explicit_index + arg_offset < function_type->params.size()) {
          const TypePtr target =
              function_type->params[explicit_index + arg_offset];
          match.params.push_back(target);
          vector<ConversionRank> element_ranks;
          collect_initializer_list_element_conversion_ranks(ctx,
                                                            scope,
                                                            *arg_nodes[j],
                                                            target,
                                                            element_ranks);
          match.list_initialization_element_ranks.push_back(
              std::move(element_ranks));
        } else {
          match.params.push_back(TypePtr());
          match.list_initialization_element_ranks.push_back(
              vector<ConversionRank>());
        }
      }

      const auto refresh_candidate_if_needed = [&]() -> bool
      {
        candidate = refresh_candidate_at(i);
        if(!candidate) {
          candidate_rejections[i] =
              "candidate invalidated during class completion";
          return false;
        }
        match.function = candidate;
        return true;
      };

      if(okay && !refresh_candidate_if_needed()) {
        okay = false;
      }

      if(okay &&
         arg_nodes.size() - source_arg_begin + arg_offset < function_type->params.size() &&
         !append_default_call_arguments(match, *candidate, function_type,
                                        arg_nodes.size() - source_arg_begin + arg_offset,
                                        &candidate_rejections[i])) {
        okay = false;
      }

      if(okay && !refresh_candidate_if_needed()) {
        okay = false;
      }

      if(okay) {
        if(parser_trace::enabled("template.resolve")) {
          std::ostringstream trace;
          trace << "call-match-accept callee=" << lookup_callee_node.value
                << " candidate=" << candidate->name
                << " type=" << describe_type(candidate->type);
          parser_trace::note("template.resolve", std::string(), trace.str());
        }
        matches.push_back(std::move(match));
        match_candidate_indices.push_back(i);
        note_overload_viable_candidate(ctx);
      } else if(candidate_rejections[i].empty()) {
        candidate_rejections[i] = "rejected";
        if(parser_trace::enabled("template.resolve")) {
          std::ostringstream trace;
          trace << "call-match-reject callee=" << lookup_callee_node.value
                << " candidate=" << (candidate ? candidate->name : std::string("<null>"))
                << " reason=" << candidate_rejections[i];
          parser_trace::note("template.resolve", std::string(), trace.str());
        }
      } else if(parser_trace::enabled("template.resolve")) {
        std::ostringstream trace;
        trace << "call-match-reject callee=" << lookup_callee_node.value
              << " candidate=" << (candidate ? candidate->name : std::string("<null>"))
              << " reason=" << candidate_rejections[i];
        parser_trace::note("template.resolve", std::string(), trace.str());
      }
    }

    vector<CandidateMatch> refreshed_matches;
    vector<size_t> refreshed_match_candidate_indices;
    refreshed_matches.reserve(matches.size());
    refreshed_match_candidate_indices.reserve(matches.size());
    for(size_t i = 0; i < matches.size(); ++i) {
      const size_t candidate_index = match_candidate_indices[i];
      FunctionBinding * candidate = refresh_candidate_at(candidate_index);
      if(!candidate) {
        candidate_rejections[candidate_index] =
            "candidate invalidated during class completion";
        continue;
      }
      matches[i].function = candidate;
      refreshed_matches.push_back(std::move(matches[i]));
      refreshed_match_candidate_indices.push_back(candidate_index);
    }
    matches.swap(refreshed_matches);
    match_candidate_indices.swap(refreshed_match_candidate_indices);

    FunctionCandidateBucketMap deduped_match_buckets;
    vector<CandidateMatch> deduped_matches;
    vector<size_t> deduped_match_candidate_indices;
    for(size_t i = 0; i < matches.size(); ++i) {
      if(!contains_equivalent_function_candidate(deduped_match_buckets,
                                                 matches[i].function)) {
        FunctionBinding * function = matches[i].function;
        deduped_matches.push_back(std::move(matches[i]));
        deduped_match_candidate_indices.push_back(match_candidate_indices[i]);
        note_function_candidate_bucket(deduped_match_buckets, function);
      }
    }
    matches.swap(deduped_matches);
    match_candidate_indices.swap(deduped_match_candidate_indices);
    if(parser_trace::enabled("template.resolve")) {
      std::ostringstream trace;
      trace << "call-match-summary callee=" << lookup_callee_node.value
            << " match_count=" << matches.size();
      parser_trace::note("template.resolve", std::string(), trace.str());
    }

    if(matches.empty()) {
      ostringstream outmsg;
      outmsg << "no viable overload";
      if(explicit_member_call) {
        outmsg << " for member call " << callee_node.children[1].value;
      } else if(lookup_callee_node.kind == CppAstKind::id_expression) {
        outmsg << " for call " << lookup_callee_node.value;
      } else {
        outmsg << " for callee kind " << cppast_kind_text(callee_node.kind);
      }
      outmsg << " with " << arg_nodes.size() << " argument(s)";
      outmsg << " [candidates";
      bool any_candidate = false;
      for(size_t i = 0; i < candidates.size(); ++i) {
        FunctionBinding * candidate = refresh_candidate_at(i);
        if(!candidate || !candidate->type) {
          continue;
        }
        outmsg << (any_candidate ? "; " : " ");
        any_candidate = true;
        outmsg << candidate->name << ":" << describe_type(candidate->type);
        if(candidate->owner_class) {
          outmsg << " owner=" << candidate->owner_class->qualified_name;
        }
        outmsg << " method=" << (candidate->is_method ? "yes" : "no");
        outmsg << " params={";
        for(size_t j = 0; j < candidate->params.size(); ++j) {
          if(j != 0) {
            outmsg << ", ";
          }
          outmsg << candidate->params[j].first << ":" <<
                    describe_type(candidate->params[j].second);
        }
        outmsg << "}";
        outmsg << " defaults={";
        for(size_t j = 0; j < candidate->default_arguments.size(); ++j) {
          if(j != 0) {
            outmsg << ", ";
          }
          outmsg << (candidate->default_arguments[j] ?
                        node_text(*candidate->default_arguments[j]) :
                        string("<none>"));
        }
        outmsg << "}";
        if(i < candidate_rejections.size() && !candidate_rejections[i].empty()) {
          outmsg << " reject=" << candidate_rejections[i];
        }
      }
      if(!any_candidate) {
        outmsg << " <none>";
      }
      outmsg << "]";
      outmsg << " [args";
      for(size_t i = 0; i < arg_nodes.size(); ++i) {
        outmsg << (i == 0 ? " " : "; ");
        outmsg << node_text(*arg_nodes[i]);
      }
      if(arg_nodes.empty()) {
        outmsg << " <none>";
      }
      outmsg << "]";
      throw NoViableOverloadError(outmsg.str());
    }

    BestCandidateSelection selection = select_best_candidate_match(ctx, matches);

    if(selection.ambiguous) {
      ostringstream outmsg;
      outmsg << "ambiguous overload";
      if(explicit_member_call) {
        outmsg << " for member call " << callee_node.children[1].value;
      } else if(lookup_callee_node.kind == CppAstKind::id_expression) {
        outmsg << " for call " << lookup_callee_node.value;
      } else {
        outmsg << " for callee kind " << cppast_kind_text(callee_node.kind);
      }
      outmsg << " [matches";
      append_candidate_match_list(outmsg, ctx, matches, true, true);
      outmsg << "]";
      throw logic_error(outmsg.str());
    }

    CandidateMatch & selected_match = matches[selection.index];
    const size_t selected_candidate_index =
        match_candidate_indices[selection.index];
    if(hints && hints->selected_ranks_out) {
      *hints->selected_ranks_out = selected_match.ranks;
    }

    FunctionBinding * chosen = refresh_candidate_at(selected_candidate_index);
    if(!chosen) {
      throw logic_error("selected candidate invalidated during class completion");
    }
    selected_match.function = chosen;
    if(chosen->is_deleted) {
      ostringstream outmsg;
      outmsg << "use of deleted " << chosen->name;
      if(lookup_callee_node.kind == CppAstKind::id_expression) {
        outmsg << " for call " << lookup_callee_node.value;
      }
      outmsg << " [selected ";
      append_function_candidate(outmsg, ctx, chosen, &selected_match.ranks);
      outmsg << "]";
      throw logic_error(outmsg.str());
    }
    if(instantiate_bodies &&
       !rematerialize_candidate_match_args(ctx,
                                           scope,
                                           selected_match,
                                           semantic_policy::rematerialization_conversion(options),
                                           true)) {
      throw logic_error("failed to rematerialize selected call conversions");
    }
    if(instantiate_bodies) {
      chosen = refresh_candidate_at(selected_candidate_index);
      if(!chosen) {
        throw logic_error("selected candidate invalidated during call rematerialization");
      }
      selected_match.function = chosen;
      chosen = semantic_template_function::acquire_function_definition_binding(
          ctx, chosen, scope);
    }
    CallableEmissionDecision emission =
        ctx.decide_callable_emission(chosen,
                                     OutputReason::DirectCall,
                                     instantiate_bodies);
    ctx.require_function_definition(chosen,
                                    OutputReason::DirectCall,
                                    emission.mark_output_required_now);
    TypePtr function_type = strip_top_level_cv(chosen->type);
    const bool use_virtual_dispatch =
        !suppress_virtual_dispatch_for_qualified_id &&
        chosen->is_method && !chosen->is_destructor &&
        chosen->is_virtual && chosen->has_virtual_slot &&
        ((explicit_member_call || callable_object_call) ||
         (use_function_lookup && current_class_scope(scope)));

    ValueCategory result_category = VC_PRVALUE;
    if(!result_value_category_for_function_result(function_type->inner, result_category)) {
      throw_internal_error("invalid function result", std::string(), "overload");
    }
    if(template_witness_source_capture_enabled_for_calls(ctx)) {
      std::string witness_use_location =
          !hint_use_location.empty() ? hint_use_location :
          (hints && !hints->use_location.empty() ? hints->use_location : std::string());
      if(witness_use_location.empty()) {
        witness_use_location = ctx.source_location_for_node(node);
      }
      if(witness_use_location.empty()) {
        witness_use_location = parser_trace::current_use_location();
      }
      if(witness_use_location.empty()) {
        witness_use_location = ctx.source_location_for_node(lookup_callee_node);
      }
      note_function_call_source_event(ctx,
                                      witness_use_location,
                                      lookup_callee_node.value,
                                      &lookup_callee_node,
                                      chosen,
                                      candidates,
                                      candidate_rejections,
                                      matches,
                                      selection,
                                      direct_function_source_drops,
                                      false,
                                      source_explicit_template_arg_count,
                                      candidates.size());
    }
    return make_resolved_call_result(ctx,
                                     function_type->inner,
                                     result_category,
                                     *chosen,
                                     std::move(matches[selection.index].call_args),
                                     use_virtual_dispatch);
  }

}  // namespace semantic_overload
