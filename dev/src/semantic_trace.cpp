#include "semantic_trace.h"

#include <cstdlib>
#include <exception>
#include <fstream>
#include <iostream>
#include <sstream>
#include <unistd.h>
#include <utility>

#include "semantic_context.h"
#include "semantic_lookup.h"
#include "semantic_model.h"
#include "semantic_utils.h"

thread_local std::vector<DiagnosticContext::Frame> DiagnosticContext::stack_;
thread_local std::vector<std::string> DiagnosticContext::captured_stack_;

namespace {

bool live_trace_enabled()
{
  static const bool enabled = []()
  {
    const char * value = std::getenv("CPPGM_DIAG_TRACE");
    return value && *value && std::string(value) != "0";
  }();
  return enabled;
}

void log_trace_line(const char * prefix, const std::vector<DiagnosticContext::Frame> & frames)
{
  if(!live_trace_enabled()) {
    return;
  }
  const std::size_t depth = frames.empty() ? 0 : (frames.size() - 1);
  std::cerr << prefix << ' ' << std::string(depth * 2, ' ');
  if(frames.empty()) {
    std::cerr << "<empty>";
  } else {
    std::cerr << DiagnosticContext::realize_frame(frames.back());
  }
  std::cerr << '\n';
}

std::size_t read_env_size_t(const char * name, std::size_t default_value)
{
  const char * value = std::getenv(name);
  if(value == nullptr || *value == '\0') {
    return default_value;
  }
  char * end = nullptr;
  const unsigned long parsed = std::strtoul(value, &end, 10);
  if(end == value || *end != '\0' || parsed == 0) {
    return default_value;
  }
  return static_cast<std::size_t>(parsed);
}

const std::string & diagnostic_mode_value()
{
  static const std::string mode = []()
  {
    const char * value = std::getenv("CPPGM_DIAG_MODE");
    return value && *value ? std::string(value) : std::string("full");
  }();
  return mode;
}

std::string compact_line(const std::string & line, std::size_t max_line_chars)
{
  if(max_line_chars == 0 || line.size() <= max_line_chars) {
    return line;
  }
  if(max_line_chars <= 32) {
    return line.substr(0, max_line_chars);
  }

  const std::size_t reserved = 24;
  const std::size_t prefix_len = (max_line_chars - reserved) * 3 / 4;
  const std::size_t suffix_len = max_line_chars - prefix_len - reserved;

  std::ostringstream out;
  out << line.substr(0, prefix_len)
      << " ... [" << (line.size() - prefix_len - suffix_len) << " chars omitted] ... "
      << line.substr(line.size() - suffix_len);
  return out.str();
}

}  // namespace

void DiagnosticContext::Guard::log_push()
{
  log_push_current();
}

void DiagnosticContext::update_top_lazy_guard(const void * old_guard,
                                              const void * new_guard)
{
  if(!stack_.empty() && stack_.back().lazy_guard == old_guard) {
    stack_.back().lazy_guard = new_guard;
  }
}

void DiagnosticContext::log_push_current()
{
  log_trace_line(">>", stack_);
}

void DiagnosticContext::log_pop_current()
{
  log_trace_line("<<", stack_);
}

DiagnosticContext::Guard::~Guard()
{
  if(!stack_.empty() &&
     std::uncaught_exception() &&
     !unwinding_on_entry_ &&
     stack_.size() >= captured_stack_.size()) {
    captured_stack_ = realize_stack(stack_);
  }
  log_pop_current();
  if(!stack_.empty()) {
    stack_.pop_back();
  }
}

void DiagnosticContext::clear()
{
  stack_.clear();
  captured_stack_.clear();
}

std::string DiagnosticContext::current_frame()
{
  return stack_.empty() ? std::string() : realize_frame(stack_.back());
}

std::string DiagnosticContext::format_stack()
{
  const std::vector<std::string> frames =
      !captured_stack_.empty() ? captured_stack_ : realize_stack(stack_);
  std::ostringstream out;
  for(std::size_t i = 0; i < frames.size(); ++i) {
    if(i != 0) {
      out << '\n';
    }
    out << std::string(i * 2, ' ') << frames[i];
  }
  return out.str();
}

std::string DiagnosticContext::format_stack_compact(std::size_t max_frames,
                                                    std::size_t max_line_chars)
{
  const std::vector<std::string> frames =
      !captured_stack_.empty() ? captured_stack_ : realize_stack(stack_);
  if(frames.empty()) {
    return std::string();
  }

  const std::size_t frame_limit = max_frames == 0 ? frames.size() : max_frames;
  const std::size_t start = frames.size() > frame_limit ? (frames.size() - frame_limit) : 0;

  std::ostringstream out;
  if(start != 0) {
    out << "[... " << start << " earlier frame(s) omitted ...]";
  }
  for(std::size_t i = start; i < frames.size(); ++i) {
    if(i != start || start != 0) {
      out << '\n';
    }
    out << "[#" << i << "] " << compact_line(frames[i], max_line_chars);
  }
  return out.str();
}

std::string DiagnosticContext::realize_frame(const Frame & frame)
{
  if(!frame.realized) {
    frame.entry =
        frame.lazy_realize ? frame.lazy_realize(frame.lazy_guard) : std::string();
    frame.realized = true;
  }
  return frame.entry;
}

std::vector<std::string> DiagnosticContext::realize_stack(const std::vector<Frame> & frames)
{
  std::vector<std::string> out;
  out.reserve(frames.size());
  for(std::size_t i = 0; i < frames.size(); ++i) {
    out.push_back(realize_frame(frames[i]));
  }
  return out;
}

namespace semantic_trace {

namespace {

std::string node_location_or_empty(const SemanticContext & ctx, const CppAstNode * node)
{
  return node ? ctx.source_location_for_node(*node) : std::string();
}

struct ParsedPhysicalSourceLocation
{
  std::string file;
  int line = 0;
  int column = 0;
  bool valid = false;
};

ParsedPhysicalSourceLocation parse_physical_source_location_text(
    const std::string & text)
{
  ParsedPhysicalSourceLocation parsed;
  if(text.empty()) {
    return parsed;
  }
  const std::size_t last_colon = text.rfind(':');
  if(last_colon == std::string::npos) {
    return parsed;
  }
  const std::size_t second_colon = text.rfind(':', last_colon - 1);
  if(second_colon == std::string::npos) {
    return parsed;
  }
  parsed.file = text.substr(0, second_colon);
  const std::string prefix = " at ";
  if(parsed.file.compare(0, prefix.size(), prefix) == 0) {
    parsed.file = parsed.file.substr(prefix.size());
  }
  parsed.line = std::atoi(text.substr(second_colon + 1,
                                      last_colon - second_colon - 1).c_str());
  parsed.column = std::atoi(text.substr(last_colon + 1).c_str());
  parsed.valid = !parsed.file.empty();
  return parsed;
}

const std::vector<std::string> & source_lines_for_identifier_lookup(
    const std::string & path)
{
  static std::map<std::string, std::vector<std::string> > cache;
  std::map<std::string, std::vector<std::string> >::iterator found =
      cache.find(path);
  if(found != cache.end()) {
    return found->second;
  }
  std::vector<std::string> lines;
  std::ifstream in(path.c_str());
  std::string line;
  while(std::getline(in, line)) {
    lines.push_back(line);
  }
  return cache.insert(std::make_pair(path, lines)).first->second;
}

std::string unqualified_template_decl_name(const std::string & name)
{
  const std::size_t split = name.rfind("::");
  return split == std::string::npos ? name : name.substr(split + 2);
}

const CppAstNode * find_last_descendant_with_value(const CppAstNode * node,
                                                   const std::string & value)
{
  if(node == nullptr || value.empty()) {
    return nullptr;
  }
  const CppAstNode * last = nullptr;
  for(std::size_t i = 0; i < node->children.size(); ++i) {
    if(const CppAstNode * found =
           find_last_descendant_with_value(&node->children[i], value)) {
      last = found;
    }
  }
  if(node->value == value) {
    last = node;
  }
  return last;
}

const CppAstNode * find_declarator_name_node(const CppAstNode * node,
                                             const std::string & unqualified_name)
{
  if(node == nullptr) {
    return nullptr;
  }

  if(node->kind == CppAstKind::nested_declarator) {
    return node->children.size() == 1 ?
               find_declarator_name_node(&node->children[0], unqualified_name) :
               nullptr;
  }

  if(node->kind != CppAstKind::declarator &&
     node->kind != CppAstKind::abstract_declarator) {
    return nullptr;
  }

  const CppAstNode * identifier = nullptr;
  for(std::size_t i = 0; i < node->children.size(); ++i) {
    const CppAstNode & child = node->children[i];
    if(child.kind == CppAstKind::identifier) {
      if(identifier == nullptr ||
         (!unqualified_name.empty() && child.value == unqualified_name)) {
        identifier = &child;
        if(unqualified_name.empty() || child.value == unqualified_name) {
          return identifier;
        }
      }
      continue;
    }

    if(child.kind == CppAstKind::nested_declarator) {
      if(const CppAstNode * nested =
             find_declarator_name_node(&child, unqualified_name)) {
        return nested;
      }
    }
  }

  return identifier;
}

std::string function_template_name_location_impl(
    const SemanticContext & ctx,
    const semantic_model::FunctionTemplateDecl * decl)
{
  if(decl == nullptr) {
    return std::string();
  }

  const std::string unqualified_name = unqualified_template_decl_name(decl->name);
  if(unqualified_name.empty()) {
    return std::string();
  }

  std::string location;
  if(decl->definition_declarator != nullptr) {
    location = ctx.source_location_for_name_in_node(*decl->definition_declarator,
                                                    unqualified_name,
                                                    true);
  }
  if(location.empty() && decl->definition_node != nullptr) {
    location = ctx.source_location_for_name_in_node(*decl->definition_node,
                                                    unqualified_name,
                                                    true);
  }
  if(location.empty() && decl->declarator != nullptr) {
    location = ctx.source_location_for_name_in_node(*decl->declarator,
                                                    unqualified_name,
                                                    true);
  }
  if(location.empty() && decl->declaration_node != nullptr) {
    location = ctx.source_location_for_name_in_node(*decl->declaration_node,
                                                    unqualified_name,
                                                    true);
  }
  if(!location.empty()) {
    return location;
  }

  const CppAstNode * found =
      find_last_descendant_with_value(decl->definition_node, unqualified_name);
  if(found == nullptr) {
    found = find_declarator_name_node(decl->definition_declarator, unqualified_name);
  }
  if(found == nullptr) {
    found = find_last_descendant_with_value(decl->definition_inner, unqualified_name);
  }
  if(found == nullptr) {
    found = find_declarator_name_node(decl->declaration_node, unqualified_name);
  }
  if(found == nullptr) {
    found = find_declarator_name_node(decl->declarator, unqualified_name);
  }
  if(found == nullptr) {
    found = find_last_descendant_with_value(decl->declaration_node, unqualified_name);
  }
  if(found == nullptr) {
    found = find_declarator_name_node(decl->inner, unqualified_name);
  }
  return node_location_or_empty(ctx, found);
}

std::string node_name_location(const SemanticContext & ctx,
                               const CppAstNode * node,
                               const std::string & unqualified_name,
                               bool prefer_last_name)
{
  if(node == nullptr || unqualified_name.empty()) {
    return std::string();
  }

  std::string location =
      ctx.source_location_for_name_in_node(*node, unqualified_name, prefer_last_name);
  if(!location.empty()) {
    return location;
  }

  const CppAstNode * found = find_declarator_name_node(node, unqualified_name);
  if(found == nullptr) {
    found = find_last_descendant_with_value(node, unqualified_name);
  }
  return node_location_or_empty(ctx, found);
}

void append_unique_template_location(std::vector<std::pair<std::string, std::string> > & out,
                                     const char * label,
                                     const std::string & location)
{
  if(location.empty()) {
    return;
  }
  for(std::size_t i = 0; i < out.size(); ++i) {
    if(out[i].second == location) {
      return;
    }
  }
  out.push_back(std::make_pair(std::string(label), location));
}

std::vector<std::pair<std::string, std::string> > template_decl_locations(
    const SemanticContext & ctx,
    const semantic_model::FunctionTemplateDecl * decl)
{
  std::vector<std::pair<std::string, std::string> > locations;
  if(decl == nullptr) {
    return locations;
  }
  append_unique_template_location(locations, "name",
                                  function_template_name_location_impl(ctx, decl));
  append_unique_template_location(locations, "definition",
                                  node_location_or_empty(ctx, decl->definition_node));
  append_unique_template_location(locations, "declaration",
                                  node_location_or_empty(ctx, decl->declaration_node));
  append_unique_template_location(locations, "definition-inner",
                                  node_location_or_empty(ctx, decl->definition_inner));
  append_unique_template_location(locations, "inner",
                                  node_location_or_empty(ctx, decl->inner));
  append_unique_template_location(locations, "definition-specifiers",
                                  node_location_or_empty(ctx, decl->definition_specifiers));
  append_unique_template_location(locations, "specifiers",
                                  node_location_or_empty(ctx, decl->specifiers));
  append_unique_template_location(locations, "definition-declarator",
                                  node_location_or_empty(ctx, decl->definition_declarator));
  append_unique_template_location(locations, "declarator",
                                  node_location_or_empty(ctx, decl->declarator));
  append_unique_template_location(locations, "body",
                                  node_location_or_empty(ctx, decl->body));
  return locations;
}

const semantic_model::SourceDeclAnchorCache & empty_source_decl_anchor_cache()
{
  static const semantic_model::SourceDeclAnchorCache empty;
  return empty;
}

}  // namespace

std::string scope_name_for_diagnostic(const semantic_model::Scope & scope)
{
  return semantic_lookup::scope_qualified_name(scope, "<here>");
}

std::string scope_bindings_for_diagnostic(const semantic_model::Scope & scope)
{
  return semantic_model::describe_scope_bindings(scope);
}

std::string node_location_note(const SemanticContext & ctx,
                               const char * label,
                               const CppAstNode * node)
{
  if(node == nullptr) {
    return std::string();
  }
  const std::string suffix = ctx.source_location_for_node(*node);
  if(suffix.empty()) {
    return std::string();
  }
  std::string out = " [";
  out += label;
  out += suffix;
  out += "]";
  return out;
}

std::string current_location_note(const SemanticContext & ctx,
                                  const CppAstNode * node)
{
  if(node == nullptr) {
    return std::string();
  }
  return ctx.source_location_for_node(*node);
}

std::string previous_function_location_note(const SemanticContext & ctx,
                                            const char * label,
                                            const semantic_model::FunctionBinding * binding)
{
  if(binding == nullptr) {
    return std::string();
  }
  const CppAstNode * node =
      binding->definition_node ? binding->definition_node : binding->declaration_node;
  return node_location_note(ctx, label, node);
}

bool source_location_points_at_identifier(const std::string & location,
                                          const std::string & identifier)
{
  const ParsedPhysicalSourceLocation parsed =
      parse_physical_source_location_text(location);
  if(!parsed.valid || parsed.line <= 0 || parsed.column <= 0 ||
     identifier.empty()) {
    return false;
  }
  const std::vector<std::string> & lines =
      source_lines_for_identifier_lookup(parsed.file);
  if(parsed.line > static_cast<int>(lines.size())) {
    return false;
  }
  const std::string & line = lines[static_cast<std::size_t>(parsed.line - 1)];
  const std::size_t offset = static_cast<std::size_t>(parsed.column - 1);
  if(offset + identifier.size() > line.size()) {
    return false;
  }
  return line.compare(offset, identifier.size(), identifier) == 0;
}

const semantic_model::SourceDeclAnchorCache & function_template_decl_anchor(
    const SemanticContext & ctx,
    const semantic_model::FunctionTemplateDecl * decl)
{
  if(decl == nullptr) {
    return empty_source_decl_anchor_cache();
  }
  semantic_model::SourceDeclAnchorCache & cache = decl->declaration_anchor;
  if(cache.cached) {
    return cache;
  }
  cache.cached = true;
  cache.name_location = function_template_name_location_impl(ctx, decl);
  const std::vector<std::pair<std::string, std::string> > locations =
      template_decl_locations(ctx, decl);
  for(std::size_t i = 0; i < locations.size(); ++i) {
    if(locations[i].first != "name") {
      cache.approximate_location = locations[i].second;
      break;
    }
  }
  return cache;
}

const semantic_model::SourceDeclAnchorCache & function_binding_decl_anchor(
    const SemanticContext & ctx,
    const semantic_model::FunctionBinding * binding)
{
  if(binding == nullptr) {
    return empty_source_decl_anchor_cache();
  }
  semantic_model::SourceDeclAnchorCache & cache = binding->declaration_anchor;
  if(cache.cached) {
    return cache;
  }
  cache.cached = true;
  const std::string unqualified_name =
      semantic_utils::unqualified_member_name(
          semantic_lookup::canonical_function_lookup_name(binding->name));
  const bool prefer_last_name = !binding->is_constructor;
  if(binding->source_template) {
    const semantic_model::SourceDeclAnchorCache & template_cache =
        function_template_decl_anchor(ctx, binding->source_template);
    if(!template_cache.name_location.empty()) {
      cache.name_location = template_cache.name_location;
      cache.approximate_location = template_cache.approximate_location;
      return cache;
    }
    if(!template_cache.approximate_location.empty()) {
      cache.approximate_location = template_cache.approximate_location;
      return cache;
    }
  }
  if(!binding->is_constructor &&
     !binding->is_destructor &&
     binding->owner_class &&
     binding->owner_class->source_template &&
     binding->owner_class->source_template->class_node &&
     !unqualified_name.empty()) {
    cache.name_location = node_name_location(ctx,
                                             binding->owner_class->source_template->class_node,
                                             unqualified_name,
                                             prefer_last_name);
    if(!cache.name_location.empty()) {
      return cache;
    }
  }
  if(binding->declaration_node && !unqualified_name.empty()) {
    cache.name_location = node_name_location(ctx,
                                             binding->declaration_node,
                                             unqualified_name,
                                             prefer_last_name);
    if(!cache.name_location.empty()) {
      cache.approximate_location =
          node_location_or_empty(ctx, binding->declaration_node);
      return cache;
    }
  }
  if(binding->parameter_syntax_node && !unqualified_name.empty()) {
    cache.name_location = node_name_location(ctx,
                                             binding->parameter_syntax_node,
                                             unqualified_name,
                                             prefer_last_name);
    if(!cache.name_location.empty()) {
      return cache;
    }
  }
  if(binding->definition_node && !unqualified_name.empty()) {
    cache.name_location = node_name_location(ctx,
                                             binding->definition_node,
                                             unqualified_name,
                                             prefer_last_name);
    if(!cache.name_location.empty()) {
      cache.approximate_location =
          node_location_or_empty(ctx, binding->definition_node);
      return cache;
    }
  }
  cache.approximate_location = binding->definition_node ?
      node_location_or_empty(ctx, binding->definition_node) :
      node_location_or_empty(ctx, binding->declaration_node);
  return cache;
}

const semantic_model::SourceDeclAnchorCache & class_decl_anchor(
    const SemanticContext & ctx,
    const semantic_model::ClassInfo * info)
{
  if(info == nullptr) {
    return empty_source_decl_anchor_cache();
  }
  semantic_model::SourceDeclAnchorCache & cache = info->declaration_anchor;
  if(cache.cached) {
    return cache;
  }
  cache.cached = true;
  const CppAstNode * primary_node = info->class_node;
  if(primary_node == nullptr && info->source_template != nullptr) {
    primary_node = info->source_template->class_node;
  }
  if(primary_node != nullptr && !info->name.empty()) {
    cache.name_location = node_name_location(ctx, primary_node, info->name, false);
  }
  if(cache.name_location.empty() && info->template_output_node != nullptr && !info->name.empty()) {
    cache.name_location =
        node_name_location(ctx, info->template_output_node, info->name, false);
  }
  cache.approximate_location = primary_node ? node_location_or_empty(ctx, primary_node) :
                                              std::string();
  if(cache.approximate_location.empty() && info->template_output_node != nullptr) {
    cache.approximate_location = node_location_or_empty(ctx, info->template_output_node);
  }
  return cache;
}

const semantic_model::SourceDeclAnchorCache & class_template_decl_anchor(
    const SemanticContext & ctx,
    const semantic_model::ClassTemplateDecl * decl)
{
  if(decl == nullptr) {
    return empty_source_decl_anchor_cache();
  }
  semantic_model::SourceDeclAnchorCache & cache = decl->declaration_anchor;
  if(cache.cached) {
    return cache;
  }
  cache.cached = true;
  if(decl->class_node != nullptr && !decl->name.empty()) {
    cache.name_location = node_name_location(ctx, decl->class_node, decl->name, false);
    cache.approximate_location = node_location_or_empty(ctx, decl->class_node);
  }
  return cache;
}

const semantic_model::SourceDeclAnchorCache & value_decl_anchor(
    const SemanticContext & ctx,
    const semantic_model::ValueBinding * binding)
{
  if(binding == nullptr) {
    return empty_source_decl_anchor_cache();
  }
  semantic_model::SourceDeclAnchorCache & cache = binding->declaration_anchor;
  if(cache.cached) {
    return cache;
  }
  cache.cached = true;
  if(binding->declaration_node != nullptr && !binding->name.empty()) {
    cache.name_location = node_name_location(ctx,
                                             binding->declaration_node,
                                             binding->name,
                                             false);
    cache.approximate_location = node_location_or_empty(ctx, binding->declaration_node);
    if(!cache.name_location.empty()) {
      return cache;
    }
  }
  if(binding->definition_node != nullptr && !binding->name.empty()) {
    cache.name_location = node_name_location(ctx,
                                             binding->definition_node,
                                             binding->name,
                                             false);
    cache.approximate_location = node_location_or_empty(ctx, binding->definition_node);
    if(!cache.name_location.empty()) {
      return cache;
    }
  }
  if(cache.approximate_location.empty()) {
    cache.approximate_location = binding->definition_node ?
        node_location_or_empty(ctx, binding->definition_node) :
        node_location_or_empty(ctx, binding->declaration_node);
  }
  return cache;
}

const semantic_model::SourceDeclAnchorCache & alias_template_decl_anchor(
    const SemanticContext & ctx,
    const semantic_model::AliasTemplateDecl * decl)
{
  if(decl == nullptr) {
    return empty_source_decl_anchor_cache();
  }
  semantic_model::SourceDeclAnchorCache & cache = decl->declaration_anchor;
  if(cache.cached) {
    return cache;
  }
  cache.cached = true;
  if(decl->type_id != nullptr) {
    cache.approximate_location = node_location_or_empty(ctx, decl->type_id);
  }
  return cache;
}

const semantic_model::SourceDeclAnchorCache & variable_template_decl_anchor(
    const SemanticContext & ctx,
    const semantic_model::VariableTemplateDecl * decl)
{
  if(decl == nullptr) {
    return empty_source_decl_anchor_cache();
  }
  semantic_model::SourceDeclAnchorCache & cache = decl->declaration_anchor;
  if(cache.cached) {
    return cache;
  }
  cache.cached = true;
  if(decl->declarator != nullptr && !decl->name.empty()) {
    cache.name_location = node_name_location(ctx, decl->declarator, decl->name, true);
  }
  cache.approximate_location = decl->declarator ?
      node_location_or_empty(ctx, decl->declarator) :
      node_location_or_empty(ctx, decl->specifiers);
  return cache;
}

std::string template_decl_primary_location(const SemanticContext & ctx,
                                           const semantic_model::FunctionTemplateDecl * decl)
{
  const semantic_model::SourceDeclAnchorCache & cache =
      function_template_decl_anchor(ctx, decl);
  const std::string location = semantic_model::source_decl_anchor_location(cache);
  return location.empty() ? std::string("<none>") : location;
}

std::string template_decl_location_details(const SemanticContext & ctx,
                                           const semantic_model::FunctionTemplateDecl * decl)
{
  const std::vector<std::pair<std::string, std::string> > locations =
      template_decl_locations(ctx, decl);
  if(locations.size() <= 1) {
    return std::string();
  }
  std::ostringstream out;
  for(std::size_t i = 0; i < locations.size(); ++i) {
    if(i != 0) {
      out << "; ";
    }
    out << locations[i].first << locations[i].second;
  }
  return out.str();
}

std::string function_template_signature_for_diagnostic(
    const semantic_model::FunctionTemplateDecl & decl)
{
  std::ostringstream out;
  out << decl.name << " type="
      << (decl.type_pattern ? describe_type(decl.type_pattern) : std::string("<null>"))
      << " params={";
  for(std::size_t i = 0; i < decl.params_pattern.size(); ++i) {
    if(i != 0) {
      out << ",";
    }
    out << decl.params_pattern[i].first << ":"
        << (decl.params_pattern[i].second ? describe_type(decl.params_pattern[i].second) :
                                            std::string("<null>"));
  }
  out << "} template_params={";
  for(std::size_t i = 0; i < decl.parameters.size(); ++i) {
    if(i != 0) {
      out << ",";
    }
    out << decl.parameters[i].name;
  }
  out << "}";
  return out.str();
}

std::string previous_value_location_note(const SemanticContext & ctx,
                                         const char * label,
                                         const semantic_model::ValueBinding * binding)
{
  if(binding == nullptr) {
    return std::string();
  }
  const CppAstNode * node =
      binding->definition_node ? binding->definition_node : binding->declaration_node;
  return node_location_note(ctx, label, node);
}

std::string previous_class_location_note(const SemanticContext & ctx,
                                         const char * label,
                                         const semantic_model::ClassInfo * info)
{
  if(info == nullptr) {
    return std::string();
  }
  return node_location_note(ctx, label, info->class_node);
}

std::string compact_diagnostic_message(const std::string & text,
                                       std::size_t max_line_chars)
{
  std::ostringstream out;
  std::size_t start = 0;
  while(start <= text.size()) {
    const std::size_t end = text.find('\n', start);
    const std::string line =
        end == std::string::npos ? text.substr(start) : text.substr(start, end - start);
    out << compact_line(line, max_line_chars);
    if(end == std::string::npos) {
      break;
    }
    out << '\n';
    start = end + 1;
  }
  return out.str();
}

std::string write_diagnostic_sidecar(const std::string & full_message,
                                     const std::string & full_context)
{
  char path[] = "/tmp/cppgm-diag-XXXXXX";
  const int fd = mkstemp(path);
  if(fd < 0) {
    return std::string();
  }
  close(fd);

  std::ofstream out(path);
  if(!out) {
    unlink(path);
    return std::string();
  }
  out << full_message;
  if(!full_context.empty()) {
    if(full_message.find("\nDiagnostic context:\n") == std::string::npos) {
      out << "\nDiagnostic context:\n" << full_context;
    } else {
      out << "\n\n[raw diagnostic context duplicate]\n" << full_context;
    }
  }
  out.close();
  if(!out) {
    unlink(path);
    return std::string();
  }
  return std::string(path);
}

bool diagnostic_mode_compact()
{
  const std::string & mode = diagnostic_mode_value();
  return mode == "compact" || mode == "compact-sidecar";
}

bool diagnostic_mode_sidecar()
{
  return diagnostic_mode_value() == "compact-sidecar";
}

std::size_t diagnostic_max_stack_frames()
{
  static const std::size_t value = read_env_size_t("CPPGM_DIAG_MAX_STACK", 8);
  return value;
}

std::size_t diagnostic_max_line_chars()
{
  static const std::size_t value = read_env_size_t("CPPGM_DIAG_MAX_LINE", 220);
  return value;
}

}  // namespace semantic_trace
