#include "template_text_output.h"

#include "cpp_toolchain.h"
#include "template_witness_renderer.h"
#include "witness_text.h"

#include <algorithm>
#include <cctype>
#include <cstdlib>
#include <functional>
#include <fstream>
#include <map>
#include <regex>
#include <set>
#include <sstream>
#include <stdexcept>
#include <string>
#include <tuple>
#include <utility>
#include <vector>

namespace {

std::string resolved_path(const std::string & path)
{
  if(path.empty()) {
    return std::string();
  }
  char * resolved = realpath(path.c_str(), nullptr);
  if(resolved == nullptr) {
    return std::string();
  }
  const std::string out = resolved;
  std::free(resolved);
  return out;
}

std::string path_dirname(const std::string & path)
{
  const std::string::size_type pos = path.find_last_of("/\\");
  if(pos == std::string::npos) {
    return std::string();
  }
  if(pos == 0) {
    return "/";
  }
  return path.substr(0, pos);
}

std::vector<std::string> read_source_lines(const std::string & path)
{
  std::ifstream in(path.c_str());
  if(!in) {
    return std::vector<std::string>();
  }
  std::vector<std::string> lines;
  std::string line;
  while(std::getline(in, line)) {
    lines.push_back(line);
  }
  return lines;
}

std::string repo_root_from_program_path()
{
  std::string resolved_program = resolved_path(cpp_tool_program_path());
  if(resolved_program.empty()) {
    resolved_program = cpp_tool_program_path();
  }
  if(resolved_program.empty()) {
    return std::string();
  }
  return path_dirname(path_dirname(resolved_program));
}

std::string normalize_template_log_location(const std::string & location)
{
  if(location.empty()) {
    return std::string();
  }

  std::string value = location;
  const std::string libcxx_marker = "/include/c++/v1/";
  const std::string::size_type libcxx_pos = value.find(libcxx_marker);
  if(libcxx_pos != std::string::npos) {
    return "libc++/" + value.substr(libcxx_pos + libcxx_marker.size());
  }

  const std::string repo_root = repo_root_from_program_path();
  if(!repo_root.empty()) {
    const std::string repo_prefix = repo_root + "/";
    if(value.compare(0, repo_prefix.size(), repo_prefix) == 0) {
      return value.substr(repo_prefix.size());
    }
  }

  return value;
}

std::string normalize_template_log_location_with_source(
    const std::string & location,
    const std::string & source_path)
{
  const std::string normalized = normalize_template_log_location(location);
  if(normalized.compare(0, 6, "tests/") != 0 &&
     normalized.compare(0, 7, "course/") != 0) {
    return normalized;
  }

  std::string resolved_source = resolved_path(source_path);
  if(resolved_source.empty()) {
    resolved_source = source_path;
  }
  const std::string normalized_source =
      normalize_template_log_location(resolved_source);

  std::string::size_type marker = normalized_source.rfind("/tests/");
  if(marker == std::string::npos) {
    marker = normalized_source.rfind("/course/");
  }
  if(marker == std::string::npos) {
    return normalized;
  }

  return normalized_source.substr(0, marker + 1) + normalized;
}

void replace_all(std::string & text,
                 const std::string & needle,
                 const std::string & replacement)
{
  if(needle.empty()) {
    return;
  }
  std::string::size_type pos = 0;
  while((pos = text.find(needle, pos)) != std::string::npos) {
    text.replace(pos, needle.size(), replacement);
    pos += replacement.size();
  }
}

bool cv_type_atom_char(char ch)
{
  return std::isalnum(static_cast<unsigned char>(ch)) ||
         ch == '_' ||
         ch == ':';
}

std::string::size_type type_atom_begin_before(const std::string & text,
                                              std::string::size_type end)
{
  if(end == 0) {
    return std::string::npos;
  }
  if(text[end - 1] == '>') {
    int depth = 0;
    std::string::size_type begin = end;
    while(begin > 0) {
      --begin;
      if(text[begin] == '>') {
        ++depth;
      } else if(text[begin] == '<') {
        --depth;
        if(depth == 0) {
          while(begin > 0 && cv_type_atom_char(text[begin - 1])) {
            --begin;
          }
          return begin;
        }
      }
    }
    return std::string::npos;
  }
  if(!cv_type_atom_char(text[end - 1])) {
    return std::string::npos;
  }
  std::string::size_type begin = end;
  while(begin > 0 && cv_type_atom_char(text[begin - 1])) {
    --begin;
  }
  return begin;
}

std::string move_postfix_cv_before_type_atoms(std::string value,
                                              const std::string & cv)
{
  const std::string needle = " " + cv;
  std::string::size_type search = 0;
  while((search = value.find(needle, search)) != std::string::npos) {
    std::string::size_type atom_end = search;
    while(atom_end > 0 &&
          std::isspace(static_cast<unsigned char>(value[atom_end - 1]))) {
      --atom_end;
    }
    const std::string::size_type atom_begin =
        type_atom_begin_before(value, atom_end);
    if(atom_begin == std::string::npos || atom_begin >= atom_end) {
      search += needle.size();
      continue;
    }
    const std::string::size_type cv_end = search + needle.size();
    if(cv_end < value.size() && cv_type_atom_char(value[cv_end])) {
      search = cv_end;
      continue;
    }
    value = value.substr(0, atom_begin) + cv + " " +
        value.substr(atom_begin, atom_end - atom_begin) +
        value.substr(cv_end);
    search = atom_begin + cv.size() + 1 + (atom_end - atom_begin);
  }
  return value;
}

std::string normalize_public_closure_const_order(const std::string & text)
{
  static const std::regex volatile_const_regex("\\bvolatile\\s+const\\b");
  static const std::regex compact_const_before_indirection_regex(
      "([A-Za-z_][A-Za-z0-9_:]*)const([*&])");
  static const std::regex compact_const_suffix_regex(
      "([A-Za-z_][A-Za-z0-9_:]*)const\\b");
  std::string out = move_postfix_cv_before_type_atoms(text, "const");
  out = move_postfix_cv_before_type_atoms(out, "volatile");
  out = std::regex_replace(out, volatile_const_regex, "const volatile");
  out = std::regex_replace(out,
                           compact_const_before_indirection_regex,
                           "const$1$2");
  return std::regex_replace(out, compact_const_suffix_regex, "const$1");
}

bool public_operator_punctuator_start(char ch)
{
  const std::string punctuators = "~!%^&*-=+|<>/,[]()";
  return punctuators.find(ch) != std::string::npos;
}

std::string normalize_public_operator_entity_text(std::string out)
{
  const std::string marker = "operator ";
  std::string::size_type pos = 0;
  while((pos = out.find(marker, pos)) != std::string::npos) {
    const std::string::size_type space_begin = pos + 8;
    std::string::size_type token_begin = space_begin;
    while(token_begin < out.size() &&
          std::isspace(static_cast<unsigned char>(out[token_begin]))) {
      ++token_begin;
    }
    if(token_begin < out.size() &&
       public_operator_punctuator_start(out[token_begin])) {
      out.erase(space_begin, token_begin - space_begin);
      pos = space_begin + 1;
      continue;
    }
    pos = token_begin;
  }
  return out;
}

std::string normalize_template_log_entity(const std::string & entity);

std::string normalize_public_template_entity_text(const std::string & text)
{
  static const std::regex char_array_spacing_regex(
      "\\b((?:const\\s+)?(?:char|wchar_t|char16_t|char32_t))\\s+\\[");
  return witness_text::normalize_anonymous_namespace_segments(
      std::regex_replace(
          normalize_public_operator_entity_text(
              normalize_public_closure_const_order(text)),
          char_array_spacing_regex,
          "$1["));
}

struct ParsedRenderedLocation
{
  std::string file;
  int line = 0;
  int column = 0;
};

ParsedRenderedLocation parse_rendered_location(const std::string & location)
{
  ParsedRenderedLocation out;
  static const std::regex pattern("^(.*:)?(\\d+):(\\d+)$");
  std::smatch match;
  if(!std::regex_match(location, match, pattern)) {
    return out;
  }
  out.file = match[1].matched ? match[1].str() : std::string();
  if(!out.file.empty() && out.file[out.file.size() - 1] == ':') {
    out.file.erase(out.file.size() - 1);
  }
  out.line = std::atoi(match[2].str().c_str());
  out.column = std::atoi(match[3].str().c_str());
  return out;
}

std::string sort_rendered_source_blocks(const std::string & text)
{
  const std::string prefix = "translation-unit\n";
  if(text.compare(0, prefix.size(), prefix) != 0) {
    return text;
  }

  std::vector<std::string> blocks;
  std::istringstream in(text.substr(prefix.size()));
  std::string line;
  std::string current;
  while(std::getline(in, line)) {
    if(line.compare(0, 2, "  ") == 0 && line.compare(0, 4, "    ") != 0) {
      if(!current.empty()) {
        blocks.push_back(current);
      }
      current.clear();
    }
    current += line;
    current += "\n";
  }
  if(!current.empty()) {
    blocks.push_back(current);
  }

  const auto block_key =
      [](const std::string & block)
      {
        const std::string::size_type at = block.find(" at ");
        const std::string::size_type nl = block.find('\n');
        const std::string location =
            at == std::string::npos ? std::string() :
            block.substr(at + 4,
                         (nl == std::string::npos ? block.size() : nl) - (at + 4));
        const ParsedRenderedLocation parsed = parse_rendered_location(location);
        return std::make_tuple(parsed.file, parsed.line, parsed.column);
      };
  std::stable_sort(blocks.begin(),
                   blocks.end(),
                   [&](const std::string & lhs, const std::string & rhs)
                   {
                     return block_key(lhs) < block_key(rhs);
                   });
  {
    std::vector<std::string> deduped;
    std::set<std::string> seen_blocks;
    for(std::size_t i = 0; i < blocks.size(); ++i) {
      if(!seen_blocks.insert(blocks[i]).second) {
        continue;
      }
      deduped.push_back(blocks[i]);
    }
    blocks.swap(deduped);
  }

  std::ostringstream out;
  out << prefix;
  for(std::size_t i = 0; i < blocks.size(); ++i) {
    out << blocks[i];
  }
  return out.str();
}

std::map<std::string, std::string> build_defaulted_source_aliases(
    const template_api::TemplateWitnessSession & session,
    const std::string & source_path)
{
  return template_api::template_source_defaulted_aliases(session, source_path);
}

std::string strip_trailing_template_id(const std::string & text);

void add_anonymous_namespace_source_aliases(
    std::map<std::string, std::string> & aliases,
    const std::set<std::string> & source_owner_entities)
{
  const std::string anonymous_segment = "::(anonymous namespace)::";
  for(std::set<std::string>::const_iterator it = source_owner_entities.begin();
      it != source_owner_entities.end();
      ++it) {
    if(it->find(anonymous_segment) == std::string::npos) {
      continue;
    }
    std::string stripped = *it;
    replace_all(stripped, anonymous_segment, "::");
    if(stripped != *it) {
      aliases[stripped] = *it;
    }
    const std::string stripped_head = strip_trailing_template_id(stripped);
    const std::string anonymous_head = strip_trailing_template_id(*it);
    if(!stripped_head.empty() &&
       !anonymous_head.empty() &&
       stripped_head != anonymous_head) {
      aliases[stripped_head] = anonymous_head;
    }
  }
}

std::string apply_template_log_aliases(
    const std::string & text,
    const std::map<std::string, std::string> & aliases)
{
  std::string out = text;
  for(std::map<std::string, std::string>::const_iterator it = aliases.begin();
      it != aliases.end();
      ++it) {
    const bool default_elision_alias =
        it->first.find('<') != std::string::npos &&
        it->second.find('<') != std::string::npos &&
        it->second.size() < it->first.size();
    if(!default_elision_alias) {
      replace_all(out, it->first, it->second);
      continue;
    }
    std::string::size_type pos = 0;
    while((pos = out.find(it->first, pos)) != std::string::npos) {
      const std::string::size_type after = pos + it->first.size();
      const bool whole_text = pos == 0 && after == out.size();
      const bool owner_prefix =
          after + 1 < out.size() &&
          out[after] == ':' &&
          out[after + 1] == ':';
      const bool argument_before =
          pos == 0 ||
          out[pos - 1] == '<' ||
          out[pos - 1] == ',' ||
          std::isspace(static_cast<unsigned char>(out[pos - 1]));
      const bool argument_after =
          after == out.size() ||
          out[after] == '>' ||
          out[after] == ',' ||
          std::isspace(static_cast<unsigned char>(out[after]));
      if(whole_text || owner_prefix || (argument_before && argument_after)) {
        out.replace(pos, it->first.size(), it->second);
        pos += it->second.size();
      } else {
        pos = after;
      }
    }
  }
  return out;
}

std::string normalize_template_log_entity(const std::string & entity)
{
  return normalize_public_template_entity_text(
      template_api::template_witness_detail::normalize_template_log_entity(entity));
}

std::string strip_trailing_template_id(const std::string & text)
{
  const std::string value = normalize_template_log_entity(text);
  if(value.empty() || value[value.size() - 1] != '>') {
    return value;
  }
  int depth = 0;
  for(int i = static_cast<int>(value.size()) - 1; i >= 0; --i) {
    if(value[static_cast<std::size_t>(i)] == '>') {
      ++depth;
    } else if(value[static_cast<std::size_t>(i)] == '<') {
      --depth;
      if(depth == 0) {
        return value.substr(0, static_cast<std::size_t>(i));
      }
    }
  }
  return value;
}

std::string strip_simple_numeric_casts(const std::string & text)
{
  static const std::regex numeric_cast_regex("\\([^()<>]+\\)\\s*(-?[0-9]+)");
  return std::regex_replace(text, numeric_cast_regex, "$1");
}

std::string top_level_owner_entity(const std::string & text)
{
  int angle_depth = 0;
  for(std::size_t i = text.size(); i > 1; --i) {
    const char ch = text[i - 1];
    if(ch == '>') {
      ++angle_depth;
      continue;
    }
    if(ch == '<') {
      --angle_depth;
      continue;
    }
    if(angle_depth == 0 && ch == ':' && text[i - 2] == ':') {
      return text.substr(0, i - 2);
    }
  }
  return std::string();
}

std::string::size_type top_level_scope_operator_pos(const std::string & text)
{
  int angle_depth = 0;
  for(std::size_t i = text.size(); i > 1; --i) {
    const char ch = text[i - 1];
    if(ch == '>') {
      ++angle_depth;
      continue;
    }
    if(ch == '<') {
      --angle_depth;
      continue;
    }
    if(angle_depth == 0 && ch == ':' && text[i - 2] == ':') {
      return i - 2;
    }
  }
  return std::string::npos;
}

std::string unqualified_top_level_owner_entity(const std::string & entity)
{
  const std::string owner = top_level_owner_entity(entity);
  const std::string::size_type split = top_level_scope_operator_pos(owner);
  if(owner.empty() || split == std::string::npos) {
    return entity;
  }
  return owner.substr(split + 2) + entity.substr(owner.size());
}

bool top_level_owner_is_qualified(const std::string & entity)
{
  const std::string owner = top_level_owner_entity(entity);
  return top_level_scope_operator_pos(owner) != std::string::npos;
}

bool is_function_lifecycle_event(
    const template_api::TemplateLifecycleEvent & event)
{
  using template_api::TemplateLifecycleEventKind;
  return event.kind == TemplateLifecycleEventKind::RequireDefinition ||
         event.kind == TemplateLifecycleEventKind::EnsureDefinition ||
         event.kind == TemplateLifecycleEventKind::FunctionInstantiation;
}

std::string unqualified_entity_name(const std::string & entity)
{
  const std::string::size_type split = entity.rfind("::");
  return split == std::string::npos ? entity : entity.substr(split + 2);
}

bool member_function_closure_can_be_owned_by_class_use(
    const template_api::TemplateLifecycleEvent & event)
{
  if(!is_function_lifecycle_event(event) || event.owner_entity.empty()) {
    return true;
  }
  const std::string owner_name =
      unqualified_entity_name(strip_trailing_template_id(event.owner_entity));
  const std::string member_name = unqualified_entity_name(event.normalized_entity);
  return member_name == owner_name ||
         member_name == "~" + owner_name ||
         member_name.compare(0, 8, "operator") == 0;
}

bool lifecycle_event_is_constructor(
    const template_api::TemplateLifecycleEvent & event)
{
  if(!is_function_lifecycle_event(event) || event.owner_entity.empty()) {
    return false;
  }
  const std::string owner_name =
      unqualified_entity_name(strip_trailing_template_id(event.owner_entity));
  const std::string member_name = unqualified_entity_name(event.normalized_entity);
  return !owner_name.empty() && member_name == owner_name;
}

bool lifecycle_event_is_operator_punctuator_function(
    const template_api::TemplateLifecycleEvent & event)
{
  if(!is_function_lifecycle_event(event)) {
    return false;
  }
  const std::string member_name = unqualified_entity_name(event.normalized_entity);
  if(member_name.compare(0, 8, "operator") != 0) {
    return false;
  }
  std::string::size_type pos = 8;
  while(pos < member_name.size() &&
        std::isspace(static_cast<unsigned char>(member_name[pos]))) {
    ++pos;
  }
  return pos < member_name.size() &&
      public_operator_punctuator_start(member_name[pos]);
}

bool source_entities_contain_event(
    const std::set<std::string> & source_owner_entities,
    const std::vector<std::string> & inline_names,
    const template_api::TemplateLifecycleEvent & event)
{
  if(source_owner_entities.find(event.normalized_entity) !=
     source_owner_entities.end()) {
    return true;
  }
  const std::string inline_stripped =
      witness_text::strip_inline_namespace_segments(event.normalized_entity,
                                                    inline_names);
  return !inline_stripped.empty() &&
         source_owner_entities.find(inline_stripped) !=
             source_owner_entities.end();
}

bool source_entities_contain_name(
    const std::set<std::string> & source_owner_entities,
    const std::vector<std::string> & inline_names,
    const std::string & entity)
{
  if(entity.empty()) {
    return false;
  }
  if(source_owner_entities.find(entity) != source_owner_entities.end()) {
    return true;
  }
  const std::string castless_entity = strip_simple_numeric_casts(entity);
  if(castless_entity != entity &&
     source_owner_entities.find(castless_entity) != source_owner_entities.end()) {
    return true;
  }
  for(std::set<std::string>::const_iterator it = source_owner_entities.begin();
      it != source_owner_entities.end();
      ++it) {
    if(strip_simple_numeric_casts(*it) == castless_entity) {
      return true;
    }
  }
  const std::string inline_stripped =
      witness_text::strip_inline_namespace_segments(entity, inline_names);
  if(inline_stripped.empty()) {
    return false;
  }
  if(source_owner_entities.find(inline_stripped) != source_owner_entities.end()) {
    return true;
  }
  const std::string castless_inline = strip_simple_numeric_casts(inline_stripped);
  if(castless_inline != inline_stripped &&
     source_owner_entities.find(castless_inline) != source_owner_entities.end()) {
    return true;
  }
  for(std::set<std::string>::const_iterator it = source_owner_entities.begin();
      it != source_owner_entities.end();
      ++it) {
    if(strip_simple_numeric_casts(*it) == castless_inline) {
      return true;
    }
  }
  return false;
}

bool identifier_char(char ch)
{
  return std::isalnum(static_cast<unsigned char>(ch)) || ch == '_';
}

bool line_mentions_identifier(const std::string & line,
                              const std::string & identifier)
{
  if(identifier.empty()) {
    return false;
  }
  std::string::size_type pos = 0;
  while((pos = line.find(identifier, pos)) != std::string::npos) {
    const bool before_ok = pos == 0 || !identifier_char(line[pos - 1]);
    const std::string::size_type after = pos + identifier.size();
    const bool after_ok =
        after >= line.size() || !identifier_char(line[after]);
    if(before_ok && after_ok) {
      return true;
    }
    ++pos;
  }
  return false;
}

bool source_mentions_member_outside_declaration(
    const std::vector<std::string> & source_lines,
    const template_api::TemplateLifecycleEvent & event)
{
  const std::string member_name = unqualified_entity_name(event.normalized_entity);
  if(member_name.empty() ||
     member_name[0] == '~' ||
     member_name.compare(0, 8, "operator") == 0) {
    return false;
  }
  const ParsedRenderedLocation decl = parse_rendered_location(event.decl_location);
  for(std::size_t i = 0; i < source_lines.size(); ++i) {
    if(decl.line > 0 && static_cast<int>(i + 1) == decl.line) {
      continue;
    }
    if(line_mentions_identifier(source_lines[i], member_name)) {
      return true;
    }
  }
  return false;
}

std::set<std::string> explicit_source_owner_entities(
    const template_api::TemplateWitnessSession & session,
    const std::string & source_path)
{
  return template_api::template_source_explicit_owner_entities(session, source_path);
}

std::set<std::string> source_owner_entities(
    const template_api::TemplateWitnessSession & session,
    const std::string & source_path)
{
  return template_api::template_source_owner_entities(session, source_path);
}

void add_inline_stripped_entities(std::set<std::string> & entities,
                                  const std::vector<std::string> & inline_names)
{
  if(inline_names.empty()) {
    return;
  }
  std::vector<std::string> stripped;
  for(std::set<std::string>::const_iterator it = entities.begin();
      it != entities.end();
      ++it) {
    const std::string value =
        witness_text::strip_inline_namespace_segments(*it, inline_names);
    if(!value.empty() && value != *it) {
      stripped.push_back(value);
    }
  }
  entities.insert(stripped.begin(), stripped.end());
}

bool public_closure_event_is_owned_by_explicit_source_event(
    const std::set<std::string> & explicit_owner_entities,
    const template_api::TemplateLifecycleEvent & event)
{
  const std::string & normalized_entity = event.normalized_entity;
  const auto template_arg_scope_alias =
      [](const std::string & text) -> std::string
      {
        const std::string::size_type first_angle = text.find('<');
        if(first_angle == std::string::npos) {
          return text;
        }
        static const std::regex qualified_arg_regex(
            "([<, ]+)([A-Za-z_][A-Za-z0-9_]*::)+([A-Za-z_][A-Za-z0-9_]*)");
        std::string out = text;
        while(true) {
          const std::string collapsed =
              std::regex_replace(out, qualified_arg_regex, "$1$3");
          if(collapsed == out) {
            return out;
          }
          out = collapsed;
        }
      };
  const std::string normalized_alias = template_arg_scope_alias(normalized_entity);
  for(std::set<std::string>::const_iterator it = explicit_owner_entities.begin();
      it != explicit_owner_entities.end();
      ++it) {
    if(it->empty()) {
      continue;
    }
    const std::string owner_alias = template_arg_scope_alias(*it);
    if(normalized_entity == *it ||
       (!normalized_alias.empty() && normalized_alias == owner_alias)) {
      return true;
    }
    if(normalized_entity.size() > it->size() + 2 &&
       normalized_entity.compare(0, it->size(), *it) == 0 &&
       normalized_entity.compare(it->size(), 2, "::") == 0) {
      return true;
    }
    if(normalized_entity.size() > it->size() + 1 &&
       normalized_entity.compare(0, it->size(), *it) == 0 &&
       normalized_entity[it->size()] == '<') {
      return true;
    }
  }
  return false;
}

void record_explicit_instantiation_owner_entity(
    std::set<std::string> & out,
    const template_api::TemplateLifecycleEvent & event)
{
  const std::string & entity = event.normalized_entity;
  if(entity.empty()) {
    return;
  }
  out.insert(entity);
  const std::string stripped = strip_trailing_template_id(entity);
  if(!stripped.empty()) {
    out.insert(stripped);
  }
  if(!event.owner_entity.empty()) {
    out.insert(event.owner_entity);
  }
}

struct ClosureLifecycleEventIndex
{
  std::set<std::string> function_instantiation_normalized_entities;
  std::set<std::string> function_instantiation_entities;
  std::set<std::string> newly_created_function_instantiation_entities;
  std::set<std::string> materialized_function_instantiation_entities;
};

ClosureLifecycleEventIndex build_closure_lifecycle_event_index(
    const std::vector<const template_api::TemplateLifecycleEvent *> & lifecycle_events)
{
  using template_api::TemplateLifecycleEventKind;
  ClosureLifecycleEventIndex out;
  for(std::size_t i = 0; i < lifecycle_events.size(); ++i) {
    const template_api::TemplateLifecycleEvent & event = *lifecycle_events[i];
    if(event.kind != TemplateLifecycleEventKind::FunctionInstantiation) {
      continue;
    }
    out.function_instantiation_normalized_entities.insert(event.normalized_entity);
    out.function_instantiation_entities.insert(event.entity);
    if(event.detail.find("created-new=yes") != std::string::npos) {
      out.newly_created_function_instantiation_entities.insert(event.entity);
    }
    if(event.detail.find("definition-materialized=yes") != std::string::npos) {
      out.materialized_function_instantiation_entities.insert(event.entity);
    }
  }
  return out;
}

std::set<std::string> explicit_instantiation_owner_entities(
    const std::vector<const template_api::TemplateLifecycleEvent *> & lifecycle_events)
{
  std::set<std::string> out;
  for(std::size_t i = 0; i < lifecycle_events.size(); ++i) {
    const template_api::TemplateLifecycleEvent & event = *lifecycle_events[i];
    if(event.cause != template_api::TemplateLifecycleCause::ExplicitInstantiationDeclaration &&
       event.cause != template_api::TemplateLifecycleCause::ExplicitInstantiationDefinition) {
      continue;
    }
    record_explicit_instantiation_owner_entity(out, event);
  }
  return out;
}

std::set<std::string> explicit_class_instantiation_owner_entities(
    const std::vector<const template_api::TemplateLifecycleEvent *> & lifecycle_events)
{
  std::set<std::string> out;
  for(std::size_t i = 0; i < lifecycle_events.size(); ++i) {
    const template_api::TemplateLifecycleEvent & event = *lifecycle_events[i];
    if(event.kind != template_api::TemplateLifecycleEventKind::ClassFinalization) {
      continue;
    }
    if(event.cause != template_api::TemplateLifecycleCause::ExplicitInstantiationDeclaration &&
       event.cause != template_api::TemplateLifecycleCause::ExplicitInstantiationDefinition) {
      continue;
    }
    record_explicit_instantiation_owner_entity(out, event);
  }
  return out;
}

std::set<std::string> non_materialized_function_closure_entities(
    const std::vector<const template_api::TemplateLifecycleEvent *> & lifecycle_events)
{
  using template_api::TemplateLifecycleEventKind;
  std::set<std::string> saw_non_materialized;
  std::set<std::string> saw_materialized;
  for(std::size_t i = 0; i < lifecycle_events.size(); ++i) {
    const template_api::TemplateLifecycleEvent & event = *lifecycle_events[i];
    if(event.kind != TemplateLifecycleEventKind::FunctionInstantiation) {
      continue;
    }
    if(event.detail.find("definition-materialized=yes") != std::string::npos) {
      saw_materialized.insert(event.normalized_entity);
    } else if(event.detail.find("definition-materialized=no") != std::string::npos) {
      saw_non_materialized.insert(event.normalized_entity);
    }
  }

  std::set<std::string> out;
  for(std::set<std::string>::const_iterator it = saw_non_materialized.begin();
      it != saw_non_materialized.end();
      ++it) {
    if(saw_materialized.count(*it) == 0) {
      out.insert(*it);
    }
  }
  return out;
}

bool render_public_closure_event(
    const ClosureLifecycleEventIndex & lifecycle_index,
    const std::map<std::string, std::string> & aliases,
    const std::set<std::string> & source_owner_entities,
    const std::set<std::string> & explicit_owner_entities,
    const std::set<std::string> & source_argument_value_entities,
    const std::set<std::string> & source_argument_value_decl_locations,
    const std::set<std::string> & explicit_instantiation_entities,
    const std::set<std::string> & explicit_class_instantiation_entities,
    const std::vector<std::string> & inline_names,
    const std::vector<std::string> & source_lines,
    const template_api::TemplateLifecycleEvent & event)
{
  if(event.decl_location.empty() ||
     !event.template_related) {
    return false;
  }
  if(event.cause == template_api::TemplateLifecycleCause::ExplicitSpecialization) {
    return false;
  }
  if(is_function_lifecycle_event(event) && event.public_source_required) {
    if(event.kind == template_api::TemplateLifecycleEventKind::RequireDefinition &&
       public_closure_event_is_owned_by_explicit_source_event(
           explicit_instantiation_entities,
           event)) {
      return false;
    }
    if(event.kind == template_api::TemplateLifecycleEventKind::EnsureDefinition &&
       public_closure_event_is_owned_by_explicit_source_event(
           explicit_class_instantiation_entities,
           event)) {
      return false;
    }
    return true;
  }
  if(event.kind != template_api::TemplateLifecycleEventKind::VariableInstantiation &&
     public_closure_event_is_owned_by_explicit_source_event(explicit_owner_entities,
                                                            event)) {
    return false;
  }
  if(event.kind == template_api::TemplateLifecycleEventKind::VariableInstantiation &&
     !event.public_source_required &&
     (source_argument_value_entities.count(event.normalized_entity) != 0 ||
      source_argument_value_decl_locations.count(
          template_api::normalize_template_witness_source_location(
              event.decl_location)) != 0)) {
    const std::string owner = top_level_owner_entity(event.normalized_entity);
    const bool direct_source_owned_value =
        event.cause == template_api::TemplateLifecycleCause::TrackInstantiation &&
        event.entry_context.trigger_entity == "value" &&
        event.entry_context.trigger_decl_location == event.decl_location &&
        source_entities_contain_name(source_owner_entities, inline_names, owner);
    if(!direct_source_owned_value) {
      return false;
    }
  }
  using template_api::TemplateLifecycleEventKind;
  if(is_function_lifecycle_event(event) &&
     event.directly_owned &&
     !source_entities_contain_event(source_owner_entities, inline_names, event) &&
     public_closure_event_is_owned_by_explicit_source_event(source_owner_entities,
                                                            event) &&
     !member_function_closure_can_be_owned_by_class_use(event) &&
     !source_mentions_member_outside_declaration(source_lines, event)) {
    return false;
  }

  if(event.kind == TemplateLifecycleEventKind::RequireDefinition &&
     public_closure_event_is_owned_by_explicit_source_event(
         explicit_instantiation_entities,
         event)) {
    return false;
  }
  if(event.kind == TemplateLifecycleEventKind::RequireDefinition &&
     is_function_lifecycle_event(event) &&
     !event.public_source_required &&
     !source_entities_contain_event(source_owner_entities, inline_names, event) &&
     lifecycle_index.function_instantiation_normalized_entities.count(
         event.normalized_entity) == 0) {
    return false;
  }
  if(event.kind == TemplateLifecycleEventKind::EnsureDefinition &&
     event.entity_is_defaulted_copy_or_move_constructor &&
     event.directly_owned &&
     !event.entity_is_constexpr_function &&
     !event.public_source_required) {
    return false;
  }
  if(event.kind == TemplateLifecycleEventKind::EnsureDefinition &&
     lifecycle_event_is_constructor(event) &&
     !event.public_source_required &&
     !event.entry_context.trigger_decl_location.empty() &&
     !event.normalized_trigger_entity.empty() &&
     event.normalized_trigger_entity != event.normalized_entity) {
    return false;
  }
  if(event.kind == TemplateLifecycleEventKind::EnsureDefinition &&
     public_closure_event_is_owned_by_explicit_source_event(
         explicit_class_instantiation_entities,
         event)) {
    return false;
  }
  if(event.kind == TemplateLifecycleEventKind::EnsureDefinition &&
     !event.normalized_trigger_entity.empty() &&
     event.normalized_trigger_entity != event.normalized_entity &&
     !(event.entity_is_constexpr_function &&
       lifecycle_index.function_instantiation_normalized_entities.count(
           event.normalized_entity) != 0) &&
     !source_entities_contain_name(source_owner_entities,
                                   inline_names,
                                   event.normalized_trigger_entity) &&
     !source_entities_contain_name(source_owner_entities,
                                   inline_names,
                                   event.trigger_owner_entity)) {
    return false;
  }
  if(event.kind == TemplateLifecycleEventKind::EnsureDefinition &&
     !event.directly_owned) {
    const bool has_matching_instantiation =
        lifecycle_index.materialized_function_instantiation_entities.count(
            event.entity) != 0;
    if(has_matching_instantiation &&
       event.cross_owner_dependency &&
       lifecycle_event_is_operator_punctuator_function(event)) {
      return false;
    }
    if(has_matching_instantiation &&
       event.cross_owner_dependency &&
       lifecycle_index.newly_created_function_instantiation_entities.count(
           event.entity) != 0 &&
       !lifecycle_event_is_constructor(event) &&
       !lifecycle_event_is_operator_punctuator_function(event)) {
      return false;
    }
    const bool trigger_owns_event =
        !event.owner_entity.empty() &&
        !event.trigger_owner_entity.empty() &&
        event.owner_entity.size() > event.trigger_owner_entity.size() + 2 &&
        event.owner_entity.compare(0,
                                   event.trigger_owner_entity.size(),
                                   event.trigger_owner_entity) == 0 &&
        event.owner_entity.compare(event.trigger_owner_entity.size(), 2, "::") == 0;
    const bool event_owner_is_source_visible =
        source_entities_contain_name(source_owner_entities,
                                     inline_names,
                                     event.owner_entity);
    const bool trigger_owner_is_source_visible =
        source_entities_contain_name(source_owner_entities,
                                     inline_names,
                                     event.trigger_owner_entity);
    if(has_matching_instantiation &&
       event.cross_owner_dependency &&
       !trigger_owns_event &&
       event_owner_is_source_visible &&
       (trigger_owner_is_source_visible ||
        event.entity_is_constexpr_function ||
        member_function_closure_can_be_owned_by_class_use(event))) {
      return true;
    }
    const bool visible_source_dependency =
        !event.entry_context.trigger_decl_location.empty() &&
        event.owner_entity.empty() &&
        event.trigger_owner_entity.empty() &&
        public_closure_event_is_owned_by_explicit_source_event(
            source_owner_entities,
            event);
    if(!event.cross_owner_dependency && !visible_source_dependency) {
      return false;
    }
    if(!event.entry_context.trigger_decl_location.empty() &&
       !visible_source_dependency) {
      return false;
    }
  }
  if(event.kind == TemplateLifecycleEventKind::ClassInstantiation) {
    if(event.normalized_entity.find("typename ") != std::string::npos) {
      return false;
    }
    const std::string inline_stripped_entity =
        witness_text::strip_inline_namespace_segments(event.normalized_entity,
                                                      inline_names);
    const std::string aliased_entity =
        apply_template_log_aliases(event.normalized_entity, aliases);
    const std::string aliased_inline_stripped_entity =
        witness_text::strip_inline_namespace_segments(aliased_entity,
                                                      inline_names);
    if(source_owner_entities.find(event.normalized_entity) != source_owner_entities.end() ||
       source_owner_entities.find(inline_stripped_entity) != source_owner_entities.end() ||
       source_owner_entities.find(aliased_entity) != source_owner_entities.end() ||
       source_owner_entities.find(aliased_inline_stripped_entity) !=
           source_owner_entities.end()) {
      return false;
    }
    if(event.entity_is_unnamed_class) {
      return true;
    }
    if(event.cause == template_api::TemplateLifecycleCause::TrackInstantiation &&
       event.normalized_trigger_entity == event.normalized_entity &&
       top_level_owner_entity(event.normalized_entity).find("<") == std::string::npos &&
       public_closure_event_is_owned_by_explicit_source_event(source_owner_entities,
                                                              event)) {
      return false;
    }
    if(event.owner_entity.empty()) {
      return false;
    }
    if(event.owner_entity.find('<') == std::string::npos) {
      return false;
    }
    if(event.normalized_entity.find('<') == std::string::npos &&
       event.normalized_trigger_entity.find('<') == std::string::npos &&
       !event.entity_has_template_identity &&
       !event.entry_context.trigger_has_template_identity) {
      return false;
    }
  }
  if(event.kind == TemplateLifecycleEventKind::ClassFinalization &&
     event.cause !=
         template_api::TemplateLifecycleCause::ExplicitInstantiationDeclaration &&
     event.cause !=
         template_api::TemplateLifecycleCause::ExplicitInstantiationDefinition &&
     !event.entity_is_unnamed_class) {
    return false;
  }
  if(event.kind == TemplateLifecycleEventKind::RequireDefinition &&
     event.normalized_entity.find("<>") != std::string::npos &&
     event.directly_owned) {
    if(lifecycle_index.function_instantiation_entities.count(event.entity) == 0) {
      return false;
    }
  }
  return true;
}

std::string normalize_template_log_text_paths(const std::string & text)
{
  static const std::regex kPathRegex(
      "((/[A-Za-z0-9_+.\\-]+)+:\\d+:\\d+)");
  std::string out;
  out.reserve(text.size());

  std::sregex_iterator it(text.begin(), text.end(), kPathRegex);
  const std::sregex_iterator end;
  std::string::const_iterator cursor = text.begin();
  for(; it != end; ++it) {
    out.append(cursor, it->prefix().second);
    out += normalize_template_log_location((*it)[1].str());
    cursor = it->suffix().first;
  }
  out.append(cursor, text.end());
  return out;
}

const char * template_lifecycle_event_kind_text(
    template_api::TemplateLifecycleEventKind kind)
{
  using template_api::TemplateLifecycleEventKind;
  switch(kind) {
  case TemplateLifecycleEventKind::RequireDefinition:
    return "require-definition";
  case TemplateLifecycleEventKind::EnsureDefinition:
    return "ensure-definition";
  case TemplateLifecycleEventKind::FunctionInstantiation:
    return "function-instantiation";
  case TemplateLifecycleEventKind::ClassInstantiation:
    return "class-instantiation";
  case TemplateLifecycleEventKind::VariableInstantiation:
    return "variable-instantiation";
  case TemplateLifecycleEventKind::ClassFinalization:
    return "class-finalization";
  }
  return "unknown";
}

const char * template_lifecycle_cause_text(
    template_api::TemplateLifecycleCause cause)
{
  using template_api::TemplateLifecycleCause;
  switch(cause) {
  case TemplateLifecycleCause::None:
    return "none";
  case TemplateLifecycleCause::TrackInstantiation:
    return "track-instantiation";
  case TemplateLifecycleCause::RequireDefinition:
    return "require-definition";
  case TemplateLifecycleCause::EnsureDefinition:
    return "ensure-definition";
  case TemplateLifecycleCause::FinalizeClass:
    return "finalize-class";
  case TemplateLifecycleCause::ExplicitInstantiationDeclaration:
    return "explicit-instantiation-declaration";
  case TemplateLifecycleCause::ExplicitInstantiationDefinition:
    return "explicit-instantiation-definition";
  case TemplateLifecycleCause::ExplicitSpecialization:
    return "explicit-specialization";
  case TemplateLifecycleCause::ImplicitUse:
    return "implicit-use";
  }
  return "unknown";
}

const char * template_closure_reason_text(template_api::TemplateClosureReason reason)
{
  using template_api::TemplateClosureReason;
  switch(reason) {
  case TemplateClosureReason::None:
    return "none";
  case TemplateClosureReason::TrackInstantiation:
    return "track-instantiation";
  case TemplateClosureReason::RequireDefinition:
    return "require-definition";
  case TemplateClosureReason::EnsureDefinition:
    return "ensure-definition";
  case TemplateClosureReason::FinalizeClass:
    return "finalize-class";
  }
  return "unknown";
}

bool session_has_closure_lifecycle_events(
    const template_api::TemplateWitnessSession & session)
{
  for(std::size_t i = 0; i < session.lifecycle_events.size(); ++i) {
    if(session.lifecycle_events[i].entry_context.origin ==
       template_api::TemplateWitnessOrigin::Closure) {
      return true;
    }
  }
  return false;
}

std::string render_template_closure_events(
    const template_api::TemplateWitnessSession & session,
    const std::string & source_path,
    bool debug)
{
  std::map<std::string, std::string> aliases =
      build_defaulted_source_aliases(session, source_path);
  const std::vector<std::string> source_lines = read_source_lines(source_path);
  const std::vector<std::string> inline_names =
      witness_text::inline_namespace_names_from_source(source_path);
  std::set<std::string> source_owner_entities =
      ::source_owner_entities(session, source_path);
  add_inline_stripped_entities(source_owner_entities, inline_names);
  add_anonymous_namespace_source_aliases(aliases, source_owner_entities);
  const std::set<std::string> explicit_owner_entities =
      explicit_source_owner_entities(session, source_path);
  const std::set<std::string> source_argument_value_entities =
      template_api::template_source_argument_value_entities(session, source_path);
  const std::set<std::string> source_argument_value_decl_locations =
      template_api::template_source_argument_value_decl_locations(session,
                                                                  source_path);
  const std::vector<const template_api::TemplateLifecycleEvent *> lifecycle_events =
      template_api::template_witness_lifecycle_events_by_origin(
          session,
          template_api::TemplateWitnessOrigin::Closure);
  const ClosureLifecycleEventIndex lifecycle_index =
      build_closure_lifecycle_event_index(lifecycle_events);
  const std::set<std::string> explicit_instantiation_entities =
      explicit_instantiation_owner_entities(lifecycle_events);
  const std::set<std::string> explicit_class_instantiation_entities =
      explicit_class_instantiation_owner_entities(lifecycle_events);
  std::vector<std::reference_wrapper<const template_api::TemplateLifecycleEvent> >
      closure_events;
  const std::set<std::string> non_materialized_function_entities =
      non_materialized_function_closure_entities(lifecycle_events);
  for(std::size_t i = 0; i < lifecycle_events.size(); ++i) {
    const template_api::TemplateLifecycleEvent & event = *lifecycle_events[i];
    if(debug) {
      if(event.decl_location.empty() ||
         !event.template_related) {
        continue;
      }
    } else if(non_materialized_function_entities.count(event.normalized_entity) != 0 &&
              !event.owner_entity.empty() &&
              source_owner_entities.count(event.normalized_entity) == 0 &&
              (event.kind == template_api::TemplateLifecycleEventKind::RequireDefinition ||
               event.kind == template_api::TemplateLifecycleEventKind::EnsureDefinition ||
               event.kind == template_api::TemplateLifecycleEventKind::FunctionInstantiation)) {
      continue;
    } else if(!render_public_closure_event(lifecycle_index,
                                           aliases,
                                           source_owner_entities,
                                           explicit_owner_entities,
                                           source_argument_value_entities,
                                           source_argument_value_decl_locations,
                                           explicit_instantiation_entities,
                                           explicit_class_instantiation_entities,
                                           inline_names,
                                           source_lines,
                                           event)) {
      continue;
    }
    closure_events.push_back(std::cref(event));
	  }
	  std::ostringstream out;
	  bool wrote_header = false;
	  const auto public_entity_text =
	      [&](const template_api::TemplateLifecycleEvent & event) -> std::string
	  {
	    return normalize_public_template_entity_text(
	        witness_text::strip_inline_namespace_segments(
	            apply_template_log_aliases(event.normalized_entity,
	                                       aliases),
	            inline_names));
	  };
	  std::set<std::string> qualified_variable_unqualified_entities;
	  if(!debug) {
	    for(std::size_t i = 0; i < closure_events.size(); ++i) {
	      const template_api::TemplateLifecycleEvent & event =
	          closure_events[i].get();
	      if(event.kind !=
	         template_api::TemplateLifecycleEventKind::VariableInstantiation) {
	        continue;
	      }
	      const std::string entity_text = public_entity_text(event);
	      if(top_level_owner_is_qualified(entity_text)) {
	        qualified_variable_unqualified_entities.insert(
	            unqualified_top_level_owner_entity(entity_text));
	      }
	    }
	  }
	  if(!debug) {
	    std::sort(
	        closure_events.begin(),
        closure_events.end(),
        [](const std::reference_wrapper<const template_api::TemplateLifecycleEvent> & lhs,
           const std::reference_wrapper<const template_api::TemplateLifecycleEvent> & rhs)
        {
          const template_api::TemplateLifecycleEvent & left = lhs.get();
          const template_api::TemplateLifecycleEvent & right = rhs.get();
          return std::make_tuple(
              std::string(template_lifecycle_event_kind_text(left.kind)),
              left.normalized_entity) <
              std::make_tuple(
                  std::string(template_lifecycle_event_kind_text(right.kind)),
                  right.normalized_entity);
        });
  }
  std::set<std::pair<std::string, std::string> > public_seen;
	  for(std::size_t i = 0; i < closure_events.size(); ++i) {
	    const template_api::TemplateLifecycleEvent & event = closure_events[i].get();
	    const std::string kind_text = template_lifecycle_event_kind_text(event.kind);
	    const std::string entity_text = public_entity_text(event);
	    if(!debug &&
	       event.kind ==
	           template_api::TemplateLifecycleEventKind::VariableInstantiation &&
	       !top_level_owner_is_qualified(entity_text) &&
	       qualified_variable_unqualified_entities.count(entity_text) != 0) {
	      continue;
	    }
	    if(!debug) {
      const std::pair<std::string, std::string> public_key(kind_text, entity_text);
      if(!public_seen.insert(public_key).second) {
        continue;
      }
    }
    if(!wrote_header) {
      out << "template-closure-events\n";
      wrote_header = true;
    }
    out << "  " << kind_text;
    if(debug) {
      out << " at "
          << normalize_template_log_location_with_source(event.decl_location,
                                                       source_path);
    }
    out << "\n";
    if(!event.entity.empty()) {
      out << "    entity " << entity_text << "\n";
    }
    if(debug) {
      out << "    decl "
          << normalize_template_log_location_with_source(event.decl_location,
                                                       source_path) << "\n";
      out << "    reason "
          << template_closure_reason_text(event.entry_context.closure_reason) << "\n";
    }
    if(debug && !event.entry_context.trigger_entity.empty()) {
      out << "    trigger "
          << normalize_public_template_entity_text(
                 witness_text::strip_inline_namespace_segments(
                     apply_template_log_aliases(
                         event.normalized_trigger_entity,
                         aliases),
                     inline_names))
          << "\n";
    }
    if(debug && !event.entry_context.trigger_decl_location.empty()) {
      out << "    trigger_decl "
          << normalize_template_log_location_with_source(
                 event.entry_context.trigger_decl_location,
                 source_path) << "\n";
    }
    if(debug) {
      out << "    cause " << template_lifecycle_cause_text(event.cause) << "\n";
    }
    if(debug && !event.detail.empty()) {
      out << "    detail " << event.detail << "\n";
    }
  }
  return out.str();
}

std::string inject_template_closure_events(const std::string & source_text,
                                           const std::string & closure_text)
{
  if(closure_text.empty()) {
    return source_text;
  }

  const std::string metrics_marker = "template-metrics\n";
  const std::string end_marker = "end translation unit\n";
  const std::string::size_type metrics_pos = source_text.rfind(metrics_marker);
  if(metrics_pos != std::string::npos) {
    return source_text.substr(0, metrics_pos) + closure_text +
        source_text.substr(metrics_pos);
  }
  const std::string::size_type end_pos = source_text.rfind(end_marker);
  if(end_pos != std::string::npos) {
    return source_text.substr(0, end_pos) + closure_text +
        source_text.substr(end_pos);
  }
  return source_text + closure_text;
}

}  // namespace

namespace template_api {

TemplateWitnessSession create_template_witness_session()
{
  return TemplateWitnessSession();
}

std::string dump_template_witness_text(const TemplateWitnessSession & session,
                                       const std::string & source_path)
{
  return render_template_source_witness_text(session, source_path);
}

std::string dump_witness_text(const TemplateWitnessSession & session,
                              const std::string & source_path)
{
  const std::string source_text =
      sort_rendered_source_blocks(
          normalize_template_log_text_paths(
              dump_template_witness_text(session, source_path)));
  if(!session_has_closure_lifecycle_events(session)) {
    return source_text;
  }
  return inject_template_closure_events(source_text,
                                        render_template_closure_events(session,
                                                                       source_path,
                                                                       false));
}

std::string dump_witness_debug_text(const TemplateWitnessSession & session,
                                    const std::string & source_path)
{
  const std::string source_text =
      sort_rendered_source_blocks(
          normalize_template_log_text_paths(
              render_template_source_witness_debug_text(session, source_path)));
  if(!session_has_closure_lifecycle_events(session)) {
    return source_text;
  }
  return inject_template_closure_events(source_text,
                                        render_template_closure_events(session,
                                                                       source_path,
                                                                       true));
}

}  // namespace template_api

std::string describe_template_translation_unit(IRecogTokenSequence & tokens,
                                               const std::string & source_path)
{
  (void)tokens;
  (void)source_path;
  return std::string();
}

std::string canonicalize_template_translation_unit_text(const std::string & text,
                                                        const std::string & source_path)
{
  (void)text;
  return template_api::dump_template_witness_text(
      template_api::create_template_witness_session(),
      source_path);
}

std::string render_witness_sessions(
    const std::vector<std::string> & source_paths,
    const std::vector<template_api::TemplateWitnessSession> & sessions)
{
  if(source_paths.size() != sessions.size()) {
    throw std::logic_error("witness session count mismatch");
  }
  std::ostringstream out;
  for(std::size_t i = 0; i < source_paths.size(); ++i) {
    out << template_api::dump_witness_text(sessions[i], source_paths[i]);
  }
  return out.str();
}

std::string render_witness_debug_sessions(
    const std::vector<std::string> & source_paths,
    const std::vector<template_api::TemplateWitnessSession> & sessions)
{
  if(source_paths.size() != sessions.size()) {
    throw std::logic_error("witness session count mismatch");
  }
  std::ostringstream out;
  for(std::size_t i = 0; i < source_paths.size(); ++i) {
    out << template_api::dump_witness_debug_text(sessions[i], source_paths[i]);
  }
  return out.str();
}
