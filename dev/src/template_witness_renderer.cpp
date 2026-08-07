#include "template_witness_renderer.h"

#include "witness_text.h"

#include <algorithm>
#include <cctype>
#include <map>
#include <regex>
#include <set>
#include <sstream>
#include <stdexcept>
#include <string>
#include <tuple>
#include <unordered_map>
#include <utility>
#include <vector>

namespace {

using std::map;
using std::pair;
using std::set;
using std::string;
using std::tuple;
using std::vector;

string trim_space(const string & text)
{
  string::size_type begin = 0;
  while(begin < text.size() &&
        std::isspace(static_cast<unsigned char>(text[begin]))) {
    ++begin;
  }
  string::size_type end = text.size();
  while(end > begin &&
        std::isspace(static_cast<unsigned char>(text[end - 1]))) {
    --end;
  }
  return text.substr(begin, end - begin);
}

string strip_simple_elaborated_prefix(const string & text)
{
  const string prefixes[] = {"typename ", "class ", "struct ", "union "};
  string out = trim_space(text);
  bool changed = true;
  while(changed) {
    changed = false;
    for(size_t i = 0; i < sizeof(prefixes) / sizeof(prefixes[0]); ++i) {
      if(out.compare(0, prefixes[i].size(), prefixes[i]) == 0) {
        out = trim_space(out.substr(prefixes[i].size()));
        changed = true;
        break;
      }
    }
  }
  return out;
}

void replace_all(string & text,
                 const string & needle,
                 const string & replacement)
{
  if(needle.empty()) {
    return;
  }
  string::size_type pos = 0;
  while((pos = text.find(needle, pos)) != string::npos) {
    text.replace(pos, needle.size(), replacement);
    pos += replacement.size();
  }
}

string normalize_compact_type_layout(const string & text)
{
  static const std::regex pointer_suffix_regex("([A-Za-z_0-9>\\)])\\*");
  static const std::regex rvalue_ref_suffix_regex("([A-Za-z_0-9>\\)])&&");
  static const std::regex lvalue_ref_suffix_regex("([A-Za-z_0-9>\\)])&");
  static const std::regex call_paren_regex("([A-Za-z_0-9>])\\(");
  static const std::regex pointer_const_regex("\\*\\s+const\\b");
  static const std::regex pointer_volatile_regex("\\*\\s+volatile\\b");
  static const std::regex pointer_rvalue_ref_regex("\\*\\s+&&");
  static const std::regex pointer_lvalue_ref_regex("\\*\\s+&");
  string out = text;
  out = std::regex_replace(out, pointer_suffix_regex, "$1 *");
  out = std::regex_replace(out, rvalue_ref_suffix_regex, "$1 &&");
  out = std::regex_replace(out, lvalue_ref_suffix_regex, "$1 &");
  out = std::regex_replace(out, call_paren_regex, "$1 (");
  replace_all(out, "operator (", "operator(");
  out = std::regex_replace(out, pointer_const_regex, "*const");
  out = std::regex_replace(out, pointer_volatile_regex, "*volatile");
  out = std::regex_replace(out, pointer_rvalue_ref_regex, "*&&");
  out = std::regex_replace(out, pointer_lvalue_ref_regex, "*&");
  return out;
}

string normalize_template_argument_separator_layout(const string & text)
{
  string out;
  out.reserve(text.size());
  int angle_depth = 0;
  bool in_string = false;
  bool in_char = false;
  bool escaped = false;
  for(size_t i = 0; i < text.size(); ++i) {
    const char ch = text[i];
    if(in_string || in_char) {
      out += ch;
      if(escaped) {
        escaped = false;
      } else if(ch == '\\') {
        escaped = true;
      } else if(in_string && ch == '"') {
        in_string = false;
      } else if(in_char && ch == '\'') {
        in_char = false;
      }
      continue;
    }
    if(ch == '"') {
      in_string = true;
      out += ch;
      continue;
    }
    if(ch == '\'') {
      in_char = true;
      out += ch;
      continue;
    }
    if(ch == '<') {
      ++angle_depth;
      out += ch;
      continue;
    }
    if(ch == '>' && angle_depth > 0) {
      --angle_depth;
      out += ch;
      continue;
    }
    if(ch == ',' && angle_depth > 0) {
      out += ", ";
      while(i + 1 < text.size() &&
            std::isspace(static_cast<unsigned char>(text[i + 1]))) {
        ++i;
      }
      continue;
    }
    out += ch;
  }
  return out;
}

string normalize_source_event_type_spellings(const string & text)
{
  static const std::regex signed_long_long_int_regex("\\bsigned long long int\\b");
  static const std::regex signed_long_int_regex("\\bsigned long int\\b");
  static const std::regex signed_short_int_regex("\\bsigned short int\\b");
  static const std::regex signed_long_long_regex("\\bsigned long long\\b");
  static const std::regex signed_long_regex("\\bsigned long\\b");
  static const std::regex signed_short_regex("\\bsigned short\\b");
  static const std::regex signed_int_regex("\\bsigned int\\b");
  static const std::regex signed_regex("\\bsigned\\b(?!\\s+char\\b)");
  static const std::regex integer_suffix_regex("\\b([0-9]+)[uUlL]+\\b");
  static const std::regex double_pointer_space_regex("\\*\\s+\\*");
  static const std::regex member_function_scope_regex(
      "::([A-Za-z_~][A-Za-z0-9_~]*) \\(");
  string out = text;
  replace_all(out, "unsigned long long int", "unsigned long long");
  replace_all(out, "unsigned long int", "unsigned long");
  replace_all(out, "long long int", "long long");
  replace_all(out, "long int", "long");
  replace_all(out, "short int", "short");
  replace_all(out, "unsigned int", "unsigned");
  out = std::regex_replace(out, signed_long_long_int_regex, "long long");
  out = std::regex_replace(out, signed_long_int_regex, "long");
  out = std::regex_replace(out, signed_short_int_regex, "short");
  out = std::regex_replace(out, signed_long_long_regex, "long long");
  out = std::regex_replace(out, signed_long_regex, "long");
  out = std::regex_replace(out, signed_short_regex, "short");
  out = std::regex_replace(out, signed_int_regex, "signed");
  out = std::regex_replace(out, signed_regex, "int");
  out = std::regex_replace(out, integer_suffix_regex, "$1");
  out = normalize_template_argument_separator_layout(out);
  while(true) {
    const string collapsed =
        std::regex_replace(out, double_pointer_space_regex, "**");
    if(collapsed == out) {
      break;
    }
    out = collapsed;
  }
  out = normalize_compact_type_layout(out);
  return std::regex_replace(out, member_function_scope_regex, "::$1(");
}

string normalize_string_literal_array_witness_text(const string & text)
{
  static const std::regex const_char_array_regex(
      "\\bconst\\s+(char|wchar_t|char16_t|char32_t)\\s*\\[");
  return std::regex_replace(text, const_char_array_regex, "$1[");
}

string collapse_duplicate_owner_prefix(const string & entity)
{
  const string::size_type member_pos = entity.rfind("::");
  if(member_pos == string::npos) {
    return entity;
  }

  const string owner = entity.substr(0, member_pos);
  const string member = entity.substr(member_pos + 2);
  for(string::size_type split = owner.find("::");
      split != string::npos;
      split = owner.find("::", split + 2)) {
    const string owner_prefix = owner.substr(0, split);
    const string owner_suffix = owner.substr(split + 2);
    if(owner_prefix == owner_suffix) {
      return owner_suffix + "::" + member;
    }
  }
  return entity;
}

string normalize_source_event_entity_text(const string & entity)
{
  return witness_text::normalize_anonymous_namespace_segments(
      normalize_string_literal_array_witness_text(
          normalize_source_event_type_spellings(
              collapse_duplicate_owner_prefix(entity))));
}

vector<string> split_top_level(const string & text, char separator)
{
  vector<string> out;
  string current;
  int depth_angle = 0;
  int depth_brace = 0;
  int depth_paren = 0;
  for(size_t i = 0; i < text.size(); ++i) {
    const char ch = text[i];
    if(ch == '<') {
      ++depth_angle;
    } else if(ch == '>') {
      depth_angle = std::max(depth_angle - 1, 0);
    } else if(ch == '{') {
      ++depth_brace;
    } else if(ch == '}') {
      depth_brace = std::max(depth_brace - 1, 0);
    } else if(ch == '(') {
      ++depth_paren;
    } else if(ch == ')') {
      depth_paren = std::max(depth_paren - 1, 0);
    }
    if(ch == separator && depth_angle == 0 && depth_brace == 0 &&
       depth_paren == 0) {
      out.push_back(trim_space(current));
      current.clear();
      continue;
    }
    current.push_back(ch);
  }
  out.push_back(trim_space(current));
  return out;
}

int find_matching_paren(const string & text, size_t open_pos)
{
  int depth = 0;
  for(size_t i = open_pos; i < text.size(); ++i) {
    if(text[i] == '(') {
      ++depth;
    } else if(text[i] == ')') {
      --depth;
      if(depth == 0) {
        return static_cast<int>(i);
      }
    }
  }
  return -1;
}

string normalize_witness_path(const string & path)
{
  static map<string, string> cache;
  map<string, string>::const_iterator cached = cache.find(path);
  if(cached != cache.end()) {
    return cached->second;
  }
  if(path.empty()) {
    cache[path] = string();
    return string();
  }
  string value = path;
  std::replace(value.begin(), value.end(), '\\', '/');
  const string libcxx_marker = "/include/c++/v1/";
  const string::size_type libcxx_pos = value.find(libcxx_marker);
  if(libcxx_pos != string::npos) {
    cache[path] = "libc++/" + value.substr(libcxx_pos + libcxx_marker.size());
    return cache[path];
  }
  static const std::regex pa_regex("(^|/)pa[0-9]+/((tests|course)/.*)$");
  std::smatch match;
  if(std::regex_search(value, match, pa_regex)) {
    cache[path] = match[2].str();
    return cache[path];
  }
  cache[path] = value;
  return value;
}

string move_trailing_cv_to_prefix(const string & text)
{
  string value = trim_space(text);
  bool changed = true;
  while(changed) {
    changed = false;
    static const std::regex const_paren_suffix("^(.*) const (\\([^)]*\\).*)$");
    static const std::regex const_ref_suffix("^(.*) const (&&|&|\\*)$");
    static const std::regex volatile_paren_suffix("^(.*) volatile (\\([^)]*\\).*)$");
    static const std::regex volatile_ref_suffix("^(.*) volatile (&&|&|\\*)$");
    std::smatch match;
    if(std::regex_match(value, match, const_paren_suffix) ||
       std::regex_match(value, match, const_ref_suffix)) {
      value = "const " + trim_space(match[1].str()) + " " +
          trim_space(match[2].str());
      changed = true;
    }
    const string::size_type const_array_pos = value.find(" const [");
    if(const_array_pos != string::npos) {
      value = "const " + trim_space(value.substr(0, const_array_pos)) + " " +
          trim_space(value.substr(const_array_pos + 7));
      changed = true;
    }
    if(std::regex_match(value, match, volatile_paren_suffix) ||
       std::regex_match(value, match, volatile_ref_suffix)) {
      value = "volatile " + trim_space(match[1].str()) + " " +
          trim_space(match[2].str());
      changed = true;
    }
    const string::size_type volatile_array_pos = value.find(" volatile [");
    if(volatile_array_pos != string::npos) {
      value = "volatile " + trim_space(value.substr(0, volatile_array_pos)) + " " +
          trim_space(value.substr(volatile_array_pos + 10));
      changed = true;
    }
    if(value.size() >= 6 &&
       value.compare(value.size() - 6, 6, " const") == 0 &&
       value.compare(0, 6, "const ") != 0) {
      value = "const " + trim_space(value.substr(0, value.size() - 6));
      changed = true;
    }
    if(value.size() >= 9 &&
       value.compare(value.size() - 9, 9, " volatile") == 0 &&
       value.compare(0, 9, "volatile ") != 0) {
      value = "volatile " + trim_space(value.substr(0, value.size() - 9));
      changed = true;
    }
  }
  return trim_space(value);
}

string attach_type_suffix(const string & base, const string & suffix)
{
  const string normalized_base = trim_space(base);
  const string::size_type array_split = normalized_base.find(" [");
  if(array_split != string::npos) {
    return normalized_base.substr(0, array_split) + " (" + suffix + ")" +
        normalized_base.substr(array_split + 1);
  }
  const string::size_type function_split = normalized_base.find(" (");
  if(function_split != string::npos && !normalized_base.empty() &&
     normalized_base[normalized_base.size() - 1] == ')') {
    return normalized_base.substr(0, function_split) + " (" + suffix + ")" +
        normalized_base.substr(function_split + 1);
  }
  return normalized_base + " " + suffix;
}

string normalize_student_text(const string & text);

string normalize_array_text(const string & text)
{
  const string prefix = "array of ";
  const size_t count_begin = prefix.size();
  const size_t count_end = text.find(' ', count_begin);
  if(count_end == string::npos) {
    return trim_space(text);
  }
  const string count = trim_space(text.substr(count_begin, count_end - count_begin));
  const string element = trim_space(text.substr(count_end + 1));
  if(count.empty() || element.empty()) {
    return trim_space(text);
  }
  return normalize_student_text(element) + " [" + count + "]";
}

string normalize_function_text(const string & text)
{
  const string prefix = "function of (";
  if(text.compare(0, prefix.size(), prefix) != 0) {
    return trim_space(text);
  }
  const size_t open_pos = text.find('(');
  if(open_pos == string::npos) {
    return trim_space(text);
  }
  const int close_pos = find_matching_paren(text, open_pos);
  if(close_pos < 0) {
    return trim_space(text);
  }
  const size_t returning_pos = text.find(" returning ", static_cast<size_t>(close_pos) + 1);
  if(returning_pos == string::npos) {
    return trim_space(text);
  }
  const string params_body = text.substr(open_pos + 1,
                                         returning_pos - open_pos - 10);
  const string return_body = trim_space(text.substr(returning_pos + 11));
  vector<string> params;
  const vector<string> raw_params = split_top_level(params_body, ',');
  for(size_t i = 0; i < raw_params.size(); ++i) {
    if(!trim_space(raw_params[i]).empty()) {
      params.push_back(normalize_student_text(raw_params[i]));
    }
  }
  string out = move_trailing_cv_to_prefix(normalize_student_text(return_body)) +
      " (";
  for(size_t i = 0; i < params.size(); ++i) {
    if(i) {
      out += ", ";
    }
    out += params[i];
  }
  out += ")";
  return out;
}

string normalize_student_text(const string & text)
{
  string value = trim_space(text);
  if(value.compare(0, 20, "lvalue-reference to ") == 0) {
    return attach_type_suffix(
        normalize_student_text(value.substr(20)),
        "&");
  }
  if(value.compare(0, 20, "rvalue-reference to ") == 0) {
    return attach_type_suffix(
        normalize_student_text(value.substr(20)),
        "&&");
  }
  if(value.compare(0, 11, "pointer to ") == 0) {
    return attach_type_suffix(
        normalize_student_text(value.substr(11)),
        "*");
  }
  if(value.compare(0, 18, "const function of (") == 0) {
    return normalize_function_text(value.substr(6));
  }
  if(value.compare(0, 21, "volatile function of (") == 0) {
    return normalize_function_text(value.substr(9));
  }
  if(value.compare(0, 9, "array of ") == 0) {
    return normalize_array_text(value);
  }
  if(value.compare(0, 13, "function of (") == 0) {
    return normalize_function_text(value);
  }
  static const std::regex classish_regex("\\b(struct|class|typename)\\s+");
  static const std::regex local_regex("__local_\\d+");
  static const std::regex top_owner_regex(
      "^[A-Z][A-Za-z0-9_]*::([A-Z][A-Za-z0-9_]*(<.*>)?)$");
  static const std::regex path_regex("((/[A-Za-z0-9_+.\\-]+)+:\\d+:\\d+)");
  static const std::regex libcxx_namespace_regex("std::__1::");
  value = std::regex_replace(value, libcxx_namespace_regex, "std::");
  value = std::regex_replace(value, classish_regex, "");
  value = std::regex_replace(value, local_regex, "");
  value = std::regex_replace(value, top_owner_regex, "$1");
  {
    string out;
    std::sregex_iterator it(value.begin(), value.end(), path_regex);
    std::sregex_iterator end;
    string::const_iterator cursor = value.begin();
    for(; it != end; ++it) {
      out.append(cursor, it->prefix().second);
      out += normalize_witness_path((*it)[1].str());
      cursor = it->suffix().first;
    }
    out.append(cursor, value.cend());
    value.swap(out);
  }
  return move_trailing_cv_to_prefix(value);
}

string strip_top_level_template_suffix(const string & text)
{
  const string value = trim_space(text);
  if(value.empty() || value[value.size() - 1] != '>') {
    return value;
  }
  int depth = 0;
  for(int i = static_cast<int>(value.size()) - 1; i >= 0; --i) {
    if(value[static_cast<size_t>(i)] == '>') {
      ++depth;
    } else if(value[static_cast<size_t>(i)] == '<') {
      --depth;
      if(depth == 0) {
        return trim_space(value.substr(0, static_cast<size_t>(i)));
      }
    }
  }
  return value;
}

using WitnessBinding = semantic_source_use::SourceBinding;
using WitnessDrop = semantic_source_use::SourceDrop;
using WitnessEventKind = semantic_source_use::SourceUseKind;
using SourceUseOwnership = semantic_source_use::SourceUseOwnership;
using SourceSelectionKind = semantic_source_use::SourceSelectionKind;
using SourceAnchorKind = semantic_source_use::SourceAnchorKind;

struct ParsedLocation
{
  int line = 0;
  int column = 0;
};

ParsedLocation parse_line_col(const string & location)
{
  ParsedLocation parsed;
  const string::size_type last_colon = location.rfind(':');
  if(last_colon == string::npos ||
     last_colon + 1 >= location.size()) {
    return parsed;
  }
  const string::size_type second_colon =
      last_colon == 0 ? string::npos : location.rfind(':', last_colon - 1);
  if(second_colon == string::npos ||
     second_colon + 1 >= last_colon) {
    return parsed;
  }
  for(string::size_type i = second_colon + 1; i < last_colon; ++i) {
    if(!std::isdigit(static_cast<unsigned char>(location[i]))) {
      return ParsedLocation();
    }
  }
  for(string::size_type i = last_colon + 1; i < location.size(); ++i) {
    if(!std::isdigit(static_cast<unsigned char>(location[i]))) {
      return ParsedLocation();
    }
  }
  parsed.line = std::atoi(location.c_str() + second_colon + 1);
  parsed.column = std::atoi(location.c_str() + last_colon + 1);
  return parsed;
}

int witness_drop_reason_priority(const string & reason)
{
  if(reason == "bad_conversion" ||
     reason == "too_few_arguments" ||
     reason == "too_many_arguments" ||
     reason == "inconsistent") {
    return 0;
  }
  if(reason == "worse_conversion") {
    return 1;
  }
  if(reason == "better_candidate_selected") {
    return 2;
  }
  if(reason == "non_deduced_mismatch") {
    return 3;
  }
  if(reason == "substitution_failure") {
    return 4;
  }
  return 100;
}

bool drop_reason_pair_uses_clang_location_order(const string & lhs_reason,
                                                const string & rhs_reason)
{
  return (lhs_reason == "better_candidate_selected" &&
          rhs_reason == "worse_conversion") ||
         (lhs_reason == "worse_conversion" &&
          rhs_reason == "better_candidate_selected");
}

const char * witness_event_kind_text(WitnessEventKind kind)
{
  switch(kind) {
  case WitnessEventKind::FunctionCall:
    return "function_call";
  case WitnessEventKind::ClassUse:
    return "class_use";
  case WitnessEventKind::AliasUse:
    return "alias_use";
  case WitnessEventKind::VariableUse:
    return "variable_use";
  }
  return "";
}

struct WitnessEvent
{
  WitnessEventKind kind = WitnessEventKind::FunctionCall;
  SourceUseOwnership ownership = SourceUseOwnership::Direct;
  semantic_source_use::SourceUseRole source_role =
      semantic_source_use::SourceUseRole::Unknown;
  string location;
  string raw_location;
  SourceAnchorKind use_anchor_kind = SourceAnchorKind::None;
  semantic_source_use::SourceTemplateIdOccurrence template_id_occurrence;
  string template_name;
  string selected;
  SourceSelectionKind selection = SourceSelectionKind::None;
  string selected_decl_location;
  string raw_selected_decl_location;
  SourceAnchorKind selected_decl_anchor_kind = SourceAnchorKind::None;
  string resolved;
  string spelling;
  string pattern;
  string expanded_to;
  string value;
  string guide;
  string guide_decl_location;
  string selected_type;
  int candidates_built = -1;
  int candidates_viable = -1;
  int candidate_count = -1;
  vector<WitnessBinding> bindings;
  vector<WitnessBinding> specialization_bindings;
  vector<WitnessDrop> drops;
  bool drop_if_needed = false;
};

#if defined(CPPGM_ENABLE_WITNESS_PROVENANCE)

struct RendererTraceContext
{
  const template_api::TemplateWitnessSession * session = nullptr;
  const string * source_path = nullptr;
  const char * pass = nullptr;
  vector<witness_provenance::RendererEventLineage> * lineages = nullptr;
};

RendererTraceContext *& current_renderer_trace()
{
  static thread_local RendererTraceContext * value = nullptr;
  return value;
}

void merge_renderer_lineage(
    witness_provenance::RendererEventLineage & retained,
    const witness_provenance::RendererEventLineage & removed)
{
  set<witness_provenance::WitnessProducerSite> producers(
      retained.producers.begin(), retained.producers.end());
  producers.insert(removed.producers.begin(), removed.producers.end());
  retained.producers.assign(producers.begin(), producers.end());
  set<witness_provenance::WitnessUpstreamRoute> upstream_routes(
      retained.upstream_routes.begin(), retained.upstream_routes.end());
  upstream_routes.insert(removed.upstream_routes.begin(),
                         removed.upstream_routes.end());
  retained.upstream_routes.assign(upstream_routes.begin(),
                                  upstream_routes.end());
}

bool witness_events_equal(const WitnessEvent & lhs, const WitnessEvent & rhs)
{
  return lhs.kind == rhs.kind &&
      lhs.ownership == rhs.ownership &&
      lhs.source_role == rhs.source_role &&
      lhs.location == rhs.location &&
      lhs.raw_location == rhs.raw_location &&
      lhs.use_anchor_kind == rhs.use_anchor_kind &&
      lhs.template_id_occurrence == rhs.template_id_occurrence &&
      lhs.template_name == rhs.template_name &&
      lhs.selected == rhs.selected &&
      lhs.selection == rhs.selection &&
      lhs.selected_decl_location == rhs.selected_decl_location &&
      lhs.raw_selected_decl_location == rhs.raw_selected_decl_location &&
      lhs.selected_decl_anchor_kind == rhs.selected_decl_anchor_kind &&
      lhs.resolved == rhs.resolved &&
      lhs.spelling == rhs.spelling &&
      lhs.pattern == rhs.pattern &&
      lhs.expanded_to == rhs.expanded_to &&
      lhs.value == rhs.value &&
      lhs.guide == rhs.guide &&
      lhs.guide_decl_location == rhs.guide_decl_location &&
      lhs.selected_type == rhs.selected_type &&
      lhs.candidates_built == rhs.candidates_built &&
      lhs.candidates_viable == rhs.candidates_viable &&
      lhs.candidate_count == rhs.candidate_count &&
      lhs.bindings == rhs.bindings &&
      lhs.specialization_bindings == rhs.specialization_bindings &&
      lhs.drops == rhs.drops &&
      lhs.drop_if_needed == rhs.drop_if_needed;
}

string renderer_changed_fields(const WitnessEvent & before,
                               const WitnessEvent & after)
{
  vector<string> fields;
  if(before.kind != after.kind) fields.push_back("kind");
  if(before.ownership != after.ownership) fields.push_back("ownership");
  if(before.source_role != after.source_role) fields.push_back("role");
  if(before.location != after.location ||
     before.raw_location != after.raw_location)
    fields.push_back("location");
  if(before.use_anchor_kind != after.use_anchor_kind)
    fields.push_back("anchor");
  if(!(before.template_id_occurrence == after.template_id_occurrence))
    fields.push_back("occurrence");
  if(before.template_name != after.template_name ||
     before.selected != after.selected)
    fields.push_back("entity");
  if(before.selection != after.selection) fields.push_back("selection");
  if(before.selected_decl_location != after.selected_decl_location ||
     before.raw_selected_decl_location != after.raw_selected_decl_location ||
     before.selected_decl_anchor_kind != after.selected_decl_anchor_kind)
    fields.push_back("selected_decl");
  if(before.bindings != after.bindings) fields.push_back("bindings");
  if(before.specialization_bindings != after.specialization_bindings)
    fields.push_back("specialization_bindings");
  if(before.drops != after.drops) fields.push_back("drops");
  if(before.expanded_to != after.expanded_to ||
     before.resolved != after.resolved ||
     before.spelling != after.spelling ||
     before.pattern != after.pattern ||
     before.value != after.value ||
     before.guide != after.guide ||
     before.guide_decl_location != after.guide_decl_location ||
     before.selected_type != after.selected_type)
    fields.push_back("payload");
  string out;
  for(size_t i = 0; i < fields.size(); ++i) {
    if(i != 0) out += ',';
    out += fields[i];
  }
  return out;
}

void note_renderer_trace_action(const WitnessEvent & event,
                                const string & action,
                                const witness_provenance::RendererEventLineage & lineage,
                                const string & changed_fields = string())
{
  RendererTraceContext * trace = current_renderer_trace();
  if(!trace || !trace->session || !trace->source_path || !trace->pass) return;
  witness_provenance::note_renderer_action(
      *trace->session,
      *trace->source_path,
      trace->pass,
      action,
      lineage,
      witness_event_kind_text(event.kind),
      event.location,
      !event.template_name.empty() ? event.template_name : event.selected,
      changed_fields);
}

#endif

void normalize_drop_order(vector<WitnessEvent> & events)
{
  for(size_t i = 0; i < events.size(); ++i) {
    std::stable_sort(
        events[i].drops.begin(),
        events[i].drops.end(),
        [](const WitnessDrop & lhs, const WitnessDrop & rhs)
        {
          if(lhs.candidate == rhs.candidate &&
             drop_reason_pair_uses_clang_location_order(lhs.reason, rhs.reason)) {
            const ParsedLocation left_location = parse_line_col(lhs.location);
            const ParsedLocation right_location = parse_line_col(rhs.location);
            if(left_location.line > 0 &&
               right_location.line > 0 &&
               std::make_pair(left_location.line, left_location.column) !=
                   std::make_pair(right_location.line, right_location.column)) {
              return std::make_pair(left_location.line, left_location.column) <
                     std::make_pair(right_location.line, right_location.column);
            }
          }
          const int left_priority = witness_drop_reason_priority(lhs.reason);
          const int right_priority = witness_drop_reason_priority(rhs.reason);
          if(left_priority != right_priority) {
            return left_priority < right_priority;
          }
          return std::make_pair(lhs.candidate, lhs.location) <
                 std::make_pair(rhs.candidate, rhs.location);
        });
  }
}

string witness_selection_text(SourceSelectionKind selection,
                              WitnessEventKind kind)
{
  switch(selection) {
  case SourceSelectionKind::None:
    return "";
  case SourceSelectionKind::Primary:
    return "primary";
  case SourceSelectionKind::PartialSpecialization:
    return "partial";
  case SourceSelectionKind::ExplicitSpecialization:
    return kind == WitnessEventKind::FunctionCall ?
        "explicit_specialization" :
        "explicit";
  case SourceSelectionKind::Instantiation:
    return "instantiation";
  }
  return "";
}

string witness_selection_text(const WitnessEvent & event)
{
  return witness_selection_text(event.selection, event.kind);
}

void compact_events(vector<WitnessEvent> & events, const vector<char> & drop)
{
#if defined(CPPGM_ENABLE_WITNESS_PROVENANCE)
  RendererTraceContext * trace = current_renderer_trace();
  const bool trace_active = trace && trace->lineages &&
      trace->lineages->size() == events.size();
  if(trace_active) {
    for(size_t i = 0; i < events.size(); ++i) {
      if(!drop[i]) continue;
      for(size_t j = 0; j < events.size(); ++j) {
        if(!drop[j] && witness_events_equal(events[i], events[j])) {
          merge_renderer_lineage((*trace->lineages)[j],
                                 (*trace->lineages)[i]);
          break;
        }
      }
      note_renderer_trace_action(events[i],
                                 "removed",
                                 (*trace->lineages)[i]);
    }
  }
#endif
  vector<WitnessEvent> kept;
  kept.reserve(events.size());
#if defined(CPPGM_ENABLE_WITNESS_PROVENANCE)
  vector<witness_provenance::RendererEventLineage> kept_lineages;
  if(trace_active) kept_lineages.reserve(events.size());
#endif
  for(size_t i = 0; i < events.size(); ++i) {
    if(!drop[i]) {
      kept.push_back(events[i]);
#if defined(CPPGM_ENABLE_WITNESS_PROVENANCE)
      if(trace_active) kept_lineages.push_back((*trace->lineages)[i]);
#endif
    }
  }
  events.swap(kept);
#if defined(CPPGM_ENABLE_WITNESS_PROVENANCE)
  if(trace_active) trace->lineages->swap(kept_lineages);
#endif
}

SourceUseOwnership rendered_ownership_sort_key(SourceUseOwnership ownership)
{
  switch(ownership) {
  case SourceUseOwnership::NestedDerived:
    return SourceUseOwnership::NestedDerived;
  case SourceUseOwnership::Direct:
  case SourceUseOwnership::SourceOwned:
    return SourceUseOwnership::Direct;
  }
  return SourceUseOwnership::Direct;
}

WitnessEvent witness_event_from_source_use(
    const semantic_source_use::SemanticSourceUse & use)
{
  WitnessEvent event;
  event.kind = use.kind;
  event.ownership = use.ownership;
  event.source_role = use.role;
  event.location = !use.spelling_anchor.location.empty() ?
      use.spelling_anchor.location :
      use.location;
  event.raw_location = use.location;
  event.use_anchor_kind = use.spelling_anchor.kind;
  event.template_id_occurrence = use.template_id_occurrence;
  event.template_name = normalize_source_event_entity_text(use.template_name);
  event.selected = normalize_source_event_entity_text(use.selected);
  event.selection = use.selection;
  event.selected_decl_location = !use.selected_decl_anchor.location.empty() ?
      use.selected_decl_anchor.location :
      use.selected_entity.decl_location;
  event.raw_selected_decl_location = use.selected_entity.decl_location;
  event.selected_decl_anchor_kind = use.selected_decl_anchor.kind;
  event.expanded_to = normalize_source_event_entity_text(use.expanded_to);
  event.candidate_count = use.candidate_count;
  event.candidates_built = use.candidates_built;
  event.candidates_viable = use.candidates_viable;
  event.bindings = use.bindings;
  event.specialization_bindings = use.specialization_bindings;
  for(size_t i = 0; i < use.drops.size(); ++i) {
    WitnessDrop drop;
    drop.candidate = normalize_source_event_entity_text(use.drops[i].candidate);
    drop.location = use.drops[i].location;
    drop.reason = use.drops[i].reason;
    event.drops.push_back(drop);
  }
  return event;
}

bool string_ends_with(const string & text, const string & suffix)
{
  return text.size() >= suffix.size() &&
      text.compare(text.size() - suffix.size(), suffix.size(), suffix) == 0;
}

string location_source_path(const string & location)
{
  const string::size_type last_colon = location.rfind(':');
  if(last_colon == string::npos || last_colon == 0) {
    return string();
  }
  const string::size_type second_colon = location.rfind(':', last_colon - 1);
  if(second_colon == string::npos) {
    return string();
  }
  return normalize_witness_path(location.substr(0, second_colon));
}

bool location_is_for_input_path(const string & location,
                                const string & normalized_input)
{
  const string path = location_source_path(location);
  if(path.empty() || normalized_input.empty()) {
    return false;
  }
  return path == normalized_input ||
      string_ends_with(path, "/" + normalized_input) ||
      string_ends_with(normalized_input, "/" + path);
}

bool source_file_is_for_input_path(const string & file,
                                   const string & normalized_input)
{
  static std::unordered_map<string, bool> cache;
  const string key = file + "\n" + normalized_input;
  std::unordered_map<string, bool>::const_iterator cached = cache.find(key);
  if(cached != cache.end()) {
    return cached->second;
  }
  const string path = normalize_witness_path(file);
  if(path.empty() || normalized_input.empty()) {
    cache[key] = false;
    return false;
  }
  const bool result =
      path == normalized_input ||
      string_ends_with(path, "/" + normalized_input) ||
      string_ends_with(normalized_input, "/" + path);
  cache[key] = result;
  return result;
}

int binding_source_sort_rank(const WitnessEvent & event)
{
  int explicit_count = 0;
  int defaulted_count = 0;
  int deduced_count = 0;
  for(size_t i = 0; i < event.bindings.size(); ++i) {
    if(event.bindings[i].source == "explicit") {
      ++explicit_count;
    } else if(event.bindings[i].source == "defaulted") {
      ++defaulted_count;
    } else if(event.bindings[i].source == "deduced") {
      ++deduced_count;
    }
  }
  for(size_t i = 0; i < event.specialization_bindings.size(); ++i) {
    if(event.specialization_bindings[i].source == "explicit") {
      ++explicit_count;
    } else if(event.specialization_bindings[i].source == "defaulted") {
      ++defaulted_count;
    } else if(event.specialization_bindings[i].source == "deduced") {
      ++deduced_count;
    }
  }
  return -(explicit_count * 100 + defaulted_count * 10 - deduced_count);
}

int witness_event_kind_sort_rank(WitnessEventKind kind)
{
  switch(kind) {
  case WitnessEventKind::ClassUse:
    return 1;
  case WitnessEventKind::FunctionCall:
    return 2;
  case WitnessEventKind::AliasUse:
    return 3;
  case WitnessEventKind::VariableUse:
    return 4;
  }
  return 0;
}

void sort_events(vector<WitnessEvent> & events)
{
#if !defined(CPPGM_ENABLE_WITNESS_PROVENANCE)
  std::stable_sort(events.begin(),
                   events.end(),
                   [](const WitnessEvent & lhs, const WitnessEvent & rhs)
                   {
                     const ParsedLocation left_loc = parse_line_col(lhs.location);
                     const ParsedLocation right_loc = parse_line_col(rhs.location);
                     const string::size_type left_split = lhs.location.rfind(':');
                     const string::size_type right_split = rhs.location.rfind(':');
                     const string::size_type left_path_split =
                         left_split == string::npos ? string::npos :
                         lhs.location.rfind(':', left_split - 1);
                     const string::size_type right_path_split =
                         right_split == string::npos ? string::npos :
                         rhs.location.rfind(':', right_split - 1);
                     const string left_path =
                         left_path_split == string::npos ?
                             lhs.location :
                             lhs.location.substr(0, left_path_split);
                     const string right_path =
                         right_path_split == string::npos ?
                             rhs.location :
                             rhs.location.substr(0, right_path_split);
                     return std::make_tuple(left_path,
                                            left_loc.line,
                                            left_loc.column,
                                            witness_event_kind_sort_rank(lhs.kind),
                                            binding_source_sort_rank(lhs),
                                            rendered_ownership_sort_key(lhs.ownership),
                                            !lhs.selected.empty() ?
                                                lhs.selected :
                                                lhs.template_name) <
                         std::make_tuple(right_path,
                                         right_loc.line,
                                         right_loc.column,
                                         witness_event_kind_sort_rank(rhs.kind),
                                         binding_source_sort_rank(rhs),
                                         rendered_ownership_sort_key(rhs.ownership),
                                         !rhs.selected.empty() ?
                                             rhs.selected :
                                             rhs.template_name);
                   });
#else
  const auto less = [](const WitnessEvent & lhs, const WitnessEvent & rhs)
  {
                     const ParsedLocation left_loc = parse_line_col(lhs.location);
                     const ParsedLocation right_loc = parse_line_col(rhs.location);
                     const string::size_type left_split = lhs.location.rfind(':');
                     const string::size_type right_split = rhs.location.rfind(':');
                     const string::size_type left_path_split =
                         left_split == string::npos ? string::npos :
                         lhs.location.rfind(':', left_split - 1);
                     const string::size_type right_path_split =
                         right_split == string::npos ? string::npos :
                         rhs.location.rfind(':', right_split - 1);
                     const string left_path =
                         left_path_split == string::npos ?
                             lhs.location :
                             lhs.location.substr(0, left_path_split);
                     const string right_path =
                         right_path_split == string::npos ?
                             rhs.location :
                             rhs.location.substr(0, right_path_split);
                     return std::make_tuple(left_path,
                                            left_loc.line,
                                            left_loc.column,
                                            witness_event_kind_sort_rank(lhs.kind),
                                            binding_source_sort_rank(lhs),
                                            rendered_ownership_sort_key(lhs.ownership),
                                            !lhs.selected.empty() ?
                                                lhs.selected :
                                                lhs.template_name) <
                         std::make_tuple(right_path,
                                         right_loc.line,
                                         right_loc.column,
                                         witness_event_kind_sort_rank(rhs.kind),
                                         binding_source_sort_rank(rhs),
                                         rendered_ownership_sort_key(rhs.ownership),
                                         !rhs.selected.empty() ?
                                             rhs.selected :
                                             rhs.template_name);
  };
  RendererTraceContext * trace = current_renderer_trace();
  if(!trace || !trace->lineages || trace->lineages->size() != events.size()) {
    std::stable_sort(events.begin(), events.end(), less);
    return;
  }
  vector<size_t> order(events.size());
  for(size_t i = 0; i < order.size(); ++i) order[i] = i;
  std::stable_sort(order.begin(), order.end(),
                   [&](size_t lhs, size_t rhs)
                   {
                     return less(events[lhs], events[rhs]);
                   });
  vector<WitnessEvent> sorted_events;
  vector<witness_provenance::RendererEventLineage> sorted_lineages;
  sorted_events.reserve(events.size());
  sorted_lineages.reserve(events.size());
  for(size_t i = 0; i < order.size(); ++i) {
    sorted_events.push_back(events[order[i]]);
    sorted_lineages.push_back((*trace->lineages)[order[i]]);
  }
  events.swap(sorted_events);
  trace->lineages->swap(sorted_lineages);
#endif
}

bool cv_type_atom_char(char ch)
{
  return std::isalnum(static_cast<unsigned char>(ch)) ||
         ch == '_' ||
         ch == ':';
}

size_t type_atom_begin_before(const string & text, size_t end)
{
  if(end == 0) {
    return string::npos;
  }
  if(text[end - 1] == '>') {
    int depth = 0;
    size_t begin = end;
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
    return string::npos;
  }
  if(!cv_type_atom_char(text[end - 1])) {
    return string::npos;
  }
  size_t begin = end;
  while(begin > 0 && cv_type_atom_char(text[begin - 1])) {
    --begin;
  }
  return begin;
}

string move_postfix_cv_before_type_atoms(string value, const string & cv)
{
  const string needle = " " + cv;
  size_t search = 0;
  while((search = value.find(needle, search)) != string::npos) {
    size_t atom_end = search;
    while(atom_end > 0 &&
          std::isspace(static_cast<unsigned char>(value[atom_end - 1]))) {
      --atom_end;
    }
    const size_t atom_begin = type_atom_begin_before(value, atom_end);
    if(atom_begin == string::npos || atom_begin >= atom_end) {
      search += needle.size();
      continue;
    }
    const size_t cv_end = search + needle.size();
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

string normalize_const_order(const string & text)
{
  static map<string, string> cache;
  map<string, string>::const_iterator cached = cache.find(text);
  if(cached != cache.end()) {
    return cached->second;
  }
  static const std::regex const_volatile_suffix_regex(
      "\\b(?!const\\b)(?!volatile\\b)([A-Za-z_][A-Za-z0-9_:]*)\\s+const\\s+volatile\\b");
  static const std::regex volatile_const_suffix_regex(
      "\\b(?!const\\b)(?!volatile\\b)([A-Za-z_][A-Za-z0-9_:]*)\\s+volatile\\s+const\\b");
  static const std::regex parenthesized_indirection_volatile_const_regex(
      "(\\([^)]*[*^]\\s*)volatile\\s+const(\\))");
  static const std::regex const_before_indirection_regex(
      "\\b(?!const\\b)(?!volatile\\b)([A-Za-z_][A-Za-z0-9_:]*)\\s+const(\\s*[*&])");
  static const std::regex const_suffix_regex(
      "\\b(?!const\\b)(?!volatile\\b)([A-Za-z_][A-Za-z0-9_:]*)\\s+const\\b");
  static const std::regex volatile_before_indirection_regex(
      "\\b(?!const\\b)(?!volatile\\b)([A-Za-z_][A-Za-z0-9_:]*)\\s+volatile(\\s*[*&])");
  static const std::regex volatile_suffix_regex(
      "\\b(?!const\\b)(?!volatile\\b)([A-Za-z_][A-Za-z0-9_:]*)\\s+volatile\\b");
  static const std::regex compact_const_before_indirection_regex(
      "([A-Za-z_][A-Za-z0-9_:]*)const([*&])");
  static const std::regex compact_const_suffix_regex(
      "([A-Za-z_][A-Za-z0-9_:]*)const\\b");
  string value = text;
  value = move_postfix_cv_before_type_atoms(value, "const");
  value = move_postfix_cv_before_type_atoms(value, "volatile");
  value = std::regex_replace(value, const_volatile_suffix_regex,
                             "const volatile $1");
  value = std::regex_replace(value, volatile_const_suffix_regex,
                             "const volatile $1");
  value = std::regex_replace(value,
                             parenthesized_indirection_volatile_const_regex,
                             "$1const volatile$2");
  value = std::regex_replace(value, const_before_indirection_regex,
                             "const $1$2");
  value = std::regex_replace(value, const_suffix_regex, "const $1");
  value = std::regex_replace(value, volatile_before_indirection_regex,
                             "volatile $1$2");
  value = std::regex_replace(value, volatile_suffix_regex, "volatile $1");
  value = std::regex_replace(value, compact_const_before_indirection_regex,
                             "const$1$2");
  value = std::regex_replace(value, compact_const_suffix_regex, "const$1");
  cache[text] = value;
  return value;
}

string strip_namespace_qualifiers(const string & text)
{
  static const std::regex namespace_qualifier_regex(
      "\\b[A-Za-z_][A-Za-z0-9_]*::");
  return std::regex_replace(text, namespace_qualifier_regex, "");
}

string canonical_template_text(const string & text)
{
  string value = normalize_const_order(strip_namespace_qualifiers(text));
  value.erase(std::remove(value.begin(), value.end(), ' '), value.end());
  string replacements[][2] = {
      {"longint", "long"},
      {"unsignedint", "unsigned"},
      {"signedint", "signed"},
      {"longlongint", "longlong"}};
  for(size_t i = 0; i < 4; ++i) {
    string::size_type pos = 0;
    while((pos = value.find(replacements[i][0], pos)) != string::npos) {
      value.replace(pos, replacements[i][0].size(), replacements[i][1]);
      pos += replacements[i][1].size();
    }
  }
  return value;
}

struct SourceOccurrence
{
  string location;
  vector<string> args;
  vector<char> preserve_semantic_args;
};

string line_col_location_from_source_location(const string & location)
{
  const ParsedLocation parsed = parse_line_col(location);
  if(parsed.line <= 0 || parsed.column <= 0) {
    return string();
  }
  std::ostringstream out;
  out << parsed.line << ":" << parsed.column;
  return out.str();
}

bool source_occurrence_from_typed_template_id(const WitnessEvent & event,
                                              SourceOccurrence & out)
{
  const semantic_source_use::SourceTemplateIdOccurrence & occurrence =
      event.template_id_occurrence;
  if(!occurrence.present ||
     !occurrence.source_spelled ||
     !occurrence.argument_list_spelled) {
    return false;
  }
  const string location =
      !occurrence.name_anchor.location.empty() ?
          occurrence.name_anchor.location :
          (!event.raw_location.empty() ? event.raw_location : event.location);
  out.location = line_col_location_from_source_location(location);
  if(out.location.empty()) {
    return false;
  }
  out.args.clear();
  out.preserve_semantic_args.clear();
  for(size_t i = 0; i < occurrence.arguments.size(); ++i) {
    if(occurrence.arguments[i].source_spelled) {
      out.args.push_back(occurrence.arguments[i].text);
      out.preserve_semantic_args.push_back(
          occurrence.arguments[i].dependent ||
          occurrence.arguments[i].current_specialization ||
          occurrence.arguments[i].preserve_qualified_member);
    }
  }
  return true;
}

string normalize_binding_arg_for_event(const string & arg);
string normalize_binding_arg_for_event(const string & arg,
                                       bool preserve_qualified_member,
                                       bool preserve_const_char_array = false);
bool is_simple_identifier_text(const string & text);

bool template_id_occurrence_is_result_type_use(
    const semantic_source_use::SourceTemplateIdOccurrence & occurrence)
{
  return occurrence.conversion_result_type_use ||
         occurrence.function_result_type_use;
}

bool binding_is_pack_binding(const WitnessBinding & binding)
{
  return binding.pack_binding;
}

vector<string> binding_pack_argument_texts(const WitnessBinding & binding)
{
  if(binding.pack_binding) {
    return binding.pack_arguments;
  }
  return vector<string>(1, trim_space(binding.arg));
}

vector<string> binding_pack_argument_match_texts(const WitnessBinding & binding)
{
  vector<string> parts = binding_pack_argument_texts(binding);
  if(parts.empty() && binding_is_pack_binding(binding)) {
    parts.push_back(string());
  }
  return parts;
}

bool typed_class_occurrence_arguments_match_bindings_exactly(
    const WitnessEvent & event)
{
  if(event.kind != WitnessEventKind::ClassUse ||
     !event.template_id_occurrence.present ||
     !event.template_id_occurrence.source_spelled ||
     !event.template_id_occurrence.argument_list_spelled) {
    return false;
  }
  vector<string> source_args;
  for(size_t i = 0; i < event.template_id_occurrence.arguments.size(); ++i) {
    if(event.template_id_occurrence.arguments[i].source_spelled) {
      source_args.push_back(event.template_id_occurrence.arguments[i].text);
    }
  }
  vector<string> expected_args;
  vector<string> expected_sources;
  for(size_t i = 0; i < event.bindings.size(); ++i) {
    if(binding_is_pack_binding(event.bindings[i])) {
      const vector<string> pack_args =
          binding_pack_argument_texts(event.bindings[i]);
      for(size_t j = 0; j < pack_args.size(); ++j) {
        expected_args.push_back(pack_args[j]);
        expected_sources.push_back(event.bindings[i].source);
      }
      continue;
    }
    expected_args.push_back(event.bindings[i].arg);
    expected_sources.push_back(event.bindings[i].source);
  }
  if(source_args.size() > expected_args.size()) {
    return false;
  }
  for(size_t i = 0; i < source_args.size(); ++i) {
    const string actual = canonical_template_text(
        normalize_binding_arg_for_event(source_args[i]));
    const string wanted = canonical_template_text(
        normalize_binding_arg_for_event(expected_args[i]));
    if(actual != wanted) {
      return false;
    }
  }
  for(size_t i = source_args.size(); i < expected_sources.size(); ++i) {
    if(expected_sources[i] != "defaulted" &&
       expected_sources[i] != "deduced") {
      return false;
    }
  }
  return true;
}

string unqualified_template_name_text(const string & text)
{
  const string stripped = strip_top_level_template_suffix(trim_space(text));
  const string::size_type split = stripped.rfind("::");
  return split == string::npos ? stripped : stripped.substr(split + 2);
}

bool is_simple_identifier_text(const string & text)
{
  const string value = strip_simple_elaborated_prefix(text);
  if(value.empty()) {
    return false;
  }
  for(size_t i = 0; i < value.size(); ++i) {
    const char ch = value[i];
    if(i == 0) {
      if(!(std::isalpha(static_cast<unsigned char>(ch)) || ch == '_')) {
        return false;
      }
    } else if(!(std::isalnum(static_cast<unsigned char>(ch)) || ch == '_')) {
      return false;
    }
  }
  return true;
}

void canonicalize_function_pointer_binding_args(vector<WitnessEvent> & events)
{
  for(size_t i = 0; i < events.size(); ++i) {
    if(events[i].kind != WitnessEventKind::ClassUse &&
       events[i].kind != WitnessEventKind::AliasUse &&
       events[i].kind != WitnessEventKind::VariableUse) {
      continue;
    }
    for(size_t j = 0; j < events[i].bindings.size(); ++j) {
      string arg = strip_simple_elaborated_prefix(trim_space(events[i].bindings[j].arg));
      if(arg.empty() || arg[0] == '&' || !is_simple_identifier_text(arg)) {
        continue;
      }
      if(events[i].bindings[j].function_pointer_parameter) {
        events[i].bindings[j].arg = "&" + arg;
      }
    }
  }
}

string normalize_binding_arg_for_event(const string & arg);
string normalize_binding_arg_for_event(const string & arg,
                                       bool preserve_qualified_member,
                                       bool preserve_const_char_array);
string normalize_function_template_argument_spacing(const string & text);

string semantic_or_source_argument_text(
    const semantic_source_use::SourceTemplateArgumentOccurrence & argument)
{
  return !argument.semantic_text.empty() ? argument.semantic_text :
      argument.text;
}

void clear_binding_pack_shape(WitnessBinding & binding)
{
  binding.pack_binding = false;
  binding.pack_aggregate = false;
  binding.pack_arguments.clear();
}

void apply_alias_source_spelled_binding_args(WitnessEvent & event)
{
  if(event.kind != WitnessEventKind::AliasUse || event.bindings.empty()) {
    return;
  }
  const semantic_source_use::SourceTemplateIdOccurrence & occurrence =
      event.template_id_occurrence;
  if(!occurrence.present ||
     !occurrence.source_spelled ||
     !occurrence.argument_list_spelled ||
     occurrence.arguments.empty()) {
    return;
  }
  const size_t limit = std::min(event.bindings.size(),
                                occurrence.arguments.size());
  for(size_t i = 0; i < limit; ++i) {
    if(event.bindings[i].source != "explicit" ||
       !occurrence.arguments[i].source_spelled) {
      continue;
    }
    const semantic_source_use::SourceTemplateArgumentOccurrence & argument =
        occurrence.arguments[i];
    if(event.bindings[i].pack_aggregate &&
       (argument.current_specialization ||
        argument.preserve_qualified_member)) {
      string replacement = argument.semantic_text;
      if(replacement.empty() &&
         event.bindings[i].pack_binding &&
         !event.bindings[i].pack_arguments.empty()) {
        replacement = event.bindings[i].pack_arguments[0];
      }
      if(!replacement.empty()) {
        event.bindings[i].arg =
            normalize_binding_arg_for_event(
                replacement,
                argument.preserve_qualified_member);
        clear_binding_pack_shape(event.bindings[i]);
      }
      continue;
    }
    if(argument.current_specialization) {
      if(!argument.semantic_text.empty()) {
        event.bindings[i].arg =
            normalize_binding_arg_for_event(
                argument.semantic_text,
                argument.preserve_qualified_member);
      }
      continue;
    }
    event.bindings[i].arg =
        normalize_binding_arg_for_event(
            semantic_or_source_argument_text(argument),
            argument.preserve_qualified_member);
    if(argument.preserve_qualified_member) {
      event.bindings[i].preserve_qualified_member = true;
    }
  }
}

void apply_alias_source_spelled_binding_args(vector<WitnessEvent> & events)
{
  for(size_t i = 0; i < events.size(); ++i) {
    apply_alias_source_spelled_binding_args(events[i]);
  }
}

void canonicalize_is_same_partial_bindings(vector<WitnessEvent> & events)
{
  for(size_t i = 0; i < events.size(); ++i) {
    if(events[i].kind != WitnessEventKind::ClassUse ||
       events[i].selection != SourceSelectionKind::PartialSpecialization ||
       unqualified_template_name_text(events[i].template_name) != "is_same" ||
       events[i].bindings.size() != 2 ||
       events[i].specialization_bindings.size() != 1 ||
       events[i].specialization_bindings[0].arg.empty()) {
      continue;
    }
    const string canonical = events[i].specialization_bindings[0].arg;
    for(size_t j = 0; j < events[i].bindings.size(); ++j) {
      if(events[i].bindings[j].source == "explicit") {
        events[i].bindings[j].arg = canonical;
      }
    }
  }
}

bool is_simple_qualified_identifier_text(const string & text)
{
  const string trimmed = trim_space(text);
  if(trimmed.find("::") == string::npos) {
    return false;
  }
  size_t start = 0;
  size_t part_count = 0;
  while(start <= trimmed.size()) {
    const size_t split = trimmed.find("::", start);
    const string part = split == string::npos ?
        trimmed.substr(start) : trimmed.substr(start, split - start);
    if(!is_simple_identifier_text(part)) {
      return false;
    }
    ++part_count;
    if(split == string::npos) {
      break;
    }
    start = split + 2;
  }
  return part_count >= 2;
}

string unqualified_identifier_tail(const string & text)
{
  const string trimmed = trim_space(text);
  const string::size_type split = trimmed.rfind("::");
  return split == string::npos ? trimmed : trimmed.substr(split + 2);
}

void collect_qualified_binding_alias(
    const WitnessEvent & event,
    const WitnessBinding & binding,
    map<tuple<string, string, string>, string> & aliases,
    set<tuple<string, string, string> > & ambiguous)
{
  if(!is_simple_qualified_identifier_text(binding.arg)) {
    return;
  }
  const string tail = unqualified_identifier_tail(binding.arg);
  if(tail.empty()) {
    return;
  }
  const tuple<string, string, string> key =
      std::make_tuple(event.template_name, binding.param, tail);
  if(ambiguous.count(key) != 0) {
    return;
  }
  map<tuple<string, string, string>, string>::iterator found =
      aliases.find(key);
  if(found == aliases.end()) {
    aliases[key] = binding.arg;
    return;
  }
  if(found->second != binding.arg) {
    aliases.erase(found);
    ambiguous.insert(key);
  }
}

void apply_qualified_binding_alias(
    const WitnessEvent & event,
    WitnessBinding & binding,
    const map<tuple<string, string, string>, string> & aliases)
{
  const string arg = trim_space(binding.arg);
  if(!is_simple_identifier_text(arg)) {
    return;
  }
  const tuple<string, string, string> key =
      std::make_tuple(event.template_name, binding.param, arg);
  map<tuple<string, string, string>, string>::const_iterator found =
      aliases.find(key);
  if(found != aliases.end()) {
    binding.arg = found->second;
  }
}

void canonicalize_qualified_binding_arguments(vector<WitnessEvent> & events)
{
  map<tuple<string, string, string>, string> aliases;
  set<tuple<string, string, string> > ambiguous;
  for(size_t i = 0; i < events.size(); ++i) {
    for(size_t j = 0; j < events[i].bindings.size(); ++j) {
      collect_qualified_binding_alias(events[i],
                                      events[i].bindings[j],
                                      aliases,
                                      ambiguous);
    }
    for(size_t j = 0; j < events[i].specialization_bindings.size(); ++j) {
      collect_qualified_binding_alias(events[i],
                                      events[i].specialization_bindings[j],
                                      aliases,
                                      ambiguous);
    }
  }
  if(aliases.empty()) {
    return;
  }
  for(size_t i = 0; i < events.size(); ++i) {
    for(size_t j = 0; j < events[i].bindings.size(); ++j) {
      apply_qualified_binding_alias(events[i], events[i].bindings[j], aliases);
    }
    for(size_t j = 0; j < events[i].specialization_bindings.size(); ++j) {
      apply_qualified_binding_alias(events[i],
                                    events[i].specialization_bindings[j],
                                    aliases);
    }
  }
}

using TemplateBodyRange = template_api::TemplateWitnessSourceRange;

bool is_identifier_token_char(char ch)
{
  return std::isalnum(static_cast<unsigned char>(ch)) || ch == '_';
}

bool text_mentions_any_identifier(const string & text, const set<string> & names)
{
  for(size_t i = 0; i < text.size();) {
    if(!(std::isalpha(static_cast<unsigned char>(text[i])) || text[i] == '_')) {
      ++i;
      continue;
    }
    const size_t begin = i;
    ++i;
    while(i < text.size() && is_identifier_token_char(text[i])) {
      ++i;
    }
    if(names.count(text.substr(begin, i - begin)) != 0) {
      return true;
    }
  }
  return false;
}

bool event_bindings_mention_any_identifier(const WitnessEvent & event,
                                           const set<string> & names)
{
  for(size_t i = 0; i < event.bindings.size(); ++i) {
    if(text_mentions_any_identifier(event.bindings[i].arg, names)) {
      return true;
    }
  }
  for(size_t i = 0; i < event.specialization_bindings.size(); ++i) {
    if(text_mentions_any_identifier(event.specialization_bindings[i].arg,
                                    names)) {
      return true;
    }
  }
  return false;
}

bool binding_arg_has_template_placeholder(const string & arg)
{
  return arg.find("type-parameter-") != string::npos ||
         arg.find("value-parameter-") != string::npos ||
         arg.find("template-parameter-") != string::npos;
}

bool event_bindings_have_template_placeholder(const WitnessEvent & event)
{
  for(size_t i = 0; i < event.bindings.size(); ++i) {
    if(binding_arg_has_template_placeholder(event.bindings[i].arg)) {
      return true;
    }
  }
  for(size_t i = 0; i < event.specialization_bindings.size(); ++i) {
    if(binding_arg_has_template_placeholder(
           event.specialization_bindings[i].arg)) {
      return true;
    }
  }
  return false;
}

bool event_source_arguments_mention_any_identifier(const WitnessEvent & event,
                                                   const set<string> & names)
{
  const semantic_source_use::SourceTemplateIdOccurrence & occurrence =
      event.template_id_occurrence;
  if(!occurrence.present || !occurrence.source_spelled) {
    return false;
  }
  for(size_t i = 0; i < occurrence.arguments.size(); ++i) {
    if(!occurrence.arguments[i].source_spelled) {
      continue;
    }
    if(text_mentions_any_identifier(occurrence.arguments[i].text, names) ||
       text_mentions_any_identifier(occurrence.arguments[i].semantic_text,
                                    names)) {
      return true;
    }
  }
  return false;
}

bool occurrence_argument_references_value(
    const semantic_source_use::SourceTemplateArgumentOccurrence & argument)
{
  return !argument.referenced_value_entities.empty() ||
         !argument.referenced_value_decl_locations.empty();
}

bool occurrence_references_value(
    const semantic_source_use::SourceTemplateIdOccurrence & occurrence)
{
  for(size_t i = 0; i < occurrence.arguments.size(); ++i) {
    if(occurrence_argument_references_value(occurrence.arguments[i])) {
      return true;
    }
  }
  return false;
}

bool event_source_occurrence_references_value(const WitnessEvent & event)
{
  return occurrence_references_value(event.template_id_occurrence);
}

bool event_replays_source_template_parameter_with_concrete_binding(
    const WitnessEvent & event,
    const set<string> & parameter_names)
{
  if(event.kind != WitnessEventKind::ClassUse ||
     parameter_names.empty() ||
     !event_source_arguments_mention_any_identifier(event, parameter_names) ||
     event_bindings_mention_any_identifier(event, parameter_names) ||
     event_bindings_have_template_placeholder(event)) {
    return false;
  }
  for(size_t i = 0; i < event.bindings.size(); ++i) {
    if(event.bindings[i].source != "explicit") {
      return false;
    }
  }
  return true;
}

bool event_replays_different_source_arguments_with_concrete_bindings(
    const WitnessEvent & event)
{
  if(event.kind != WitnessEventKind::ClassUse ||
     event_bindings_have_template_placeholder(event)) {
    return false;
  }
  const semantic_source_use::SourceTemplateIdOccurrence & occurrence =
      event.template_id_occurrence;
  if(!occurrence.present ||
     !occurrence.source_spelled ||
     occurrence.arguments.empty()) {
    return false;
  }
  if(template_id_occurrence_is_result_type_use(occurrence)) {
    return false;
  }
  vector<string> expected_args;
  vector<string> expected_sources;
  for(size_t i = 0; i < event.bindings.size(); ++i) {
    if(binding_is_pack_binding(event.bindings[i])) {
      const vector<string> pack_args =
          binding_pack_argument_match_texts(event.bindings[i]);
      for(size_t j = 0; j < pack_args.size(); ++j) {
        expected_args.push_back(pack_args[j]);
        expected_sources.push_back(event.bindings[i].source);
      }
      continue;
    }
    expected_args.push_back(event.bindings[i].arg);
    expected_sources.push_back(event.bindings[i].source);
  }
  const size_t count = std::min(occurrence.arguments.size(),
                                expected_args.size());
  bool any_different = false;
  for(size_t i = 0; i < count; ++i) {
    if(expected_sources[i] != "explicit" ||
       !occurrence.arguments[i].source_spelled) {
      return false;
    }
    if(trim_space(occurrence.arguments[i].text) !=
       trim_space(expected_args[i])) {
      if(occurrence_argument_references_value(occurrence.arguments[i])) {
        if(occurrence.current_specialization_use) {
          any_different = true;
        }
      } else if(!occurrence.arguments[i].current_specialization) {
        any_different = true;
      }
    }
  }
  return any_different;
}

using TemplateHeaderContext =
    template_api::TemplateWitnessTemplateHeaderContext;

bool template_header_context_contains_location(
    const TemplateHeaderContext & context,
    int line,
    int column,
    const string & normalized_input)
{
  if(line <= 0 ||
     !source_file_is_for_input_path(context.file, normalized_input)) {
    return false;
  }
  if(line < context.begin_line || line > context.end_line) {
    return false;
  }
  if(line == context.begin_line &&
     column > 0 &&
     column < context.begin_column) {
    return false;
  }
  if(line == context.end_line &&
     context.end_column > 0 &&
     column > context.end_column) {
    return false;
  }
  return true;
}

bool template_header_context_for_location(
    const vector<TemplateHeaderContext> & contexts,
    const string & normalized_input,
    int line,
    int column,
    set<string> & parameter_names,
    bool & class_template_context)
{
  class_template_context = false;
  for(size_t i = 0; i < contexts.size(); ++i) {
    if(!template_header_context_contains_location(contexts[i],
                                                  line,
                                                  column,
                                                  normalized_input)) {
      continue;
    }
    for(size_t j = 0; j < contexts[i].parameter_names.size(); ++j) {
      parameter_names.insert(contexts[i].parameter_names[j]);
    }
    class_template_context =
        class_template_context || contexts[i].class_template;
  }
  return !parameter_names.empty();
}

bool event_selected_decl_is_after_use(const WitnessEvent & event)
{
  const ParsedLocation use = parse_line_col(
      !event.location.empty() ? event.location : event.raw_location);
  const ParsedLocation decl = parse_line_col(event.selected_decl_location);
  return use.line > 0 &&
         decl.line > 0 &&
         std::make_pair(decl.line, decl.column) >
             std::make_pair(use.line, use.column);
}

string compact_source_match_text(const string & text);

string class_event_source_template_id_text(const WitnessEvent & event)
{
  const semantic_source_use::SourceTemplateIdOccurrence & occurrence =
      event.template_id_occurrence;
  if(event.kind != WitnessEventKind::ClassUse ||
     !occurrence.present ||
     !occurrence.source_spelled ||
     !occurrence.argument_list_spelled) {
    return string();
  }
  string out = unqualified_template_name_text(event.template_name);
  if(out.empty()) {
    return string();
  }
  out += "<";
  for(size_t i = 0; i < occurrence.arguments.size(); ++i) {
    if(!occurrence.arguments[i].source_spelled) {
      return string();
    }
    if(i != 0) {
      out += ", ";
    }
    out += occurrence.arguments[i].text;
  }
  out += ">";
  return out;
}

bool same_line_alias_binding_mentions_class_source(
    const vector<WitnessEvent> & events,
    size_t index)
{
  const string source_template_id =
      compact_source_match_text(class_event_source_template_id_text(events[index]));
  if(source_template_id.empty()) {
    return false;
  }
  const ParsedLocation target = parse_line_col(
      !events[index].location.empty() ? events[index].location :
                                        events[index].raw_location);
  if(target.line <= 0) {
    return false;
  }
  for(size_t i = 0; i < events.size(); ++i) {
    if(i == index || events[i].kind != WitnessEventKind::AliasUse) {
      continue;
    }
    const ParsedLocation alias_location = parse_line_col(
        !events[i].location.empty() ? events[i].location :
                                      events[i].raw_location);
    if(alias_location.line != target.line ||
       (alias_location.column > 0 &&
        target.column > 0 &&
        alias_location.column > target.column)) {
      continue;
    }
    for(size_t j = 0; j < events[i].bindings.size(); ++j) {
      const string binding_text =
          compact_source_match_text(events[i].bindings[j].arg);
      if(binding_text.find(source_template_id) != string::npos) {
        return true;
      }
      for(size_t k = 0; k < events[i].bindings[j].pack_arguments.size(); ++k) {
        const string pack_text =
            compact_source_match_text(events[i].bindings[j].pack_arguments[k]);
        if(pack_text.find(source_template_id) != string::npos) {
          return true;
        }
      }
    }
  }
  return false;
}

void drop_template_header_pattern_events(vector<WitnessEvent> & events,
                                         const vector<TemplateHeaderContext> & contexts,
                                         const string & input_path)
{
  if(contexts.empty()) {
    return;
  }
  const string normalized_input = normalize_witness_path(input_path);
  vector<char> drop(events.size(), 0);
  const auto has_same_line_qualifier_class_use =
      [&](size_t index) -> bool
  {
    const ParsedLocation target = parse_line_col(
        !events[index].location.empty() ? events[index].location :
                                          events[index].raw_location);
    if(target.line <= 0) {
      return false;
    }
    for(size_t j = 0; j < events.size(); ++j) {
      if(j == index ||
         events[j].kind != WitnessEventKind::ClassUse ||
         events[j].source_role != semantic_source_use::SourceUseRole::QualifierUse) {
        continue;
      }
      const ParsedLocation candidate = parse_line_col(
          !events[j].location.empty() ? events[j].location :
                                        events[j].raw_location);
      if(candidate.line == target.line) {
        return true;
      }
    }
    return false;
  };
  for(size_t i = 0; i < events.size(); ++i) {
    if(events[i].kind != WitnessEventKind::ClassUse &&
       events[i].kind != WitnessEventKind::FunctionCall) {
      continue;
    }
    const ParsedLocation parsed = parse_line_col(
        !events[i].location.empty() ? events[i].location : events[i].raw_location);
    set<string> parameter_names;
    bool class_template_context = false;
    if(!template_header_context_for_location(contexts,
                                             normalized_input,
                                             parsed.line,
                                             parsed.column,
                                             parameter_names,
                                             class_template_context)) {
      continue;
    }
    if(events[i].kind == WitnessEventKind::ClassUse &&
        (event_replays_source_template_parameter_with_concrete_binding(
            events[i],
            parameter_names) &&
        (class_template_context ||
         event_selected_decl_is_after_use(events[i]) ||
         same_line_alias_binding_mentions_class_source(events, i)))) {
      drop[i] = 1;
      continue;
    }
    if(class_template_context &&
       events[i].kind == WitnessEventKind::ClassUse &&
       event_replays_different_source_arguments_with_concrete_bindings(
           events[i])) {
      drop[i] = 1;
      continue;
    }
    if(events[i].kind == WitnessEventKind::ClassUse &&
       events[i].source_role == semantic_source_use::SourceUseRole::ValueUse &&
       events[i].selection == SourceSelectionKind::Primary &&
       !has_same_line_qualifier_class_use(i)) {
      drop[i] = 1;
      continue;
    }
    if(event_bindings_mention_any_identifier(events[i], parameter_names)) {
      drop[i] = 1;
    }
  }
  compact_events(events, drop);
}

string compact_source_match_text(const string & text)
{
  string normalized = witness_text::normalize_source_spelling_text(text);
  string out;
  out.reserve(normalized.size());
  for(size_t i = 0; i < normalized.size(); ++i) {
    if(!std::isspace(static_cast<unsigned char>(normalized[i]))) {
      out += normalized[i];
    }
  }
  return out;
}

bool alias_event_has_source_spelled_explicit_arguments(
    const WitnessEvent & event)
{
  const semantic_source_use::SourceTemplateIdOccurrence & occurrence =
      event.template_id_occurrence;
  if(event.kind != WitnessEventKind::AliasUse ||
     !occurrence.present ||
     !occurrence.source_spelled ||
     !occurrence.argument_list_spelled ||
     occurrence.synthesized) {
    return false;
  }
  size_t explicit_bindings = 0;
  for(size_t i = 0; i < event.bindings.size(); ++i) {
    if(event.bindings[i].source == "explicit") {
      ++explicit_bindings;
    }
  }
  if(explicit_bindings == 0) {
    return false;
  }
  size_t source_spelled_args = 0;
  for(size_t i = 0; i < occurrence.arguments.size(); ++i) {
    if(occurrence.arguments[i].source_spelled) {
      ++source_spelled_args;
    }
  }
  return source_spelled_args >= explicit_bindings;
}

bool location_in_any_template_body_range(
    int line_no,
    int column,
    const string & normalized_input,
    const vector<TemplateBodyRange> & ranges,
    set<string> * parameter_names = nullptr)
{
  bool matched = false;
  for(size_t i = 0; i < ranges.size(); ++i) {
    if(!source_file_is_for_input_path(ranges[i].file, normalized_input)) {
      continue;
    }
    if(ranges[i].begin_line <= line_no && line_no <= ranges[i].end_line) {
      if(line_no == ranges[i].begin_line &&
         ranges[i].first_body_column > 1 &&
         column > 0 &&
         column < ranges[i].first_body_column) {
        continue;
      }
      matched = true;
      if(parameter_names) {
        for(size_t j = 0; j < ranges[i].parameter_names.size(); ++j) {
          parameter_names->insert(ranges[i].parameter_names[j]);
        }
      }
    }
  }
  return matched;
}

bool class_event_has_concrete_template_body_source_occurrence(
    const WitnessEvent & event)
{
  const semantic_source_use::SourceTemplateIdOccurrence & occurrence =
      event.template_id_occurrence;
  if(event.selection != SourceSelectionKind::Primary ||
     !event.specialization_bindings.empty() ||
     event.bindings.empty() ||
     !occurrence.present ||
     !occurrence.source_spelled ||
     !occurrence.in_template_body ||
     occurrence.synthesized ||
     occurrence.arguments.empty()) {
    return false;
  }
  const bool source_argument_value_reference =
      !occurrence.current_specialization_use &&
      occurrence_references_value(occurrence);
  if((occurrence.has_dependent_argument ||
     occurrence.has_current_specialization_argument) &&
     !source_argument_value_reference &&
     !template_id_occurrence_is_result_type_use(occurrence)) {
    return false;
  }
  return true;
}

bool class_event_has_selected_specialization_template_body_source_occurrence(
    const WitnessEvent & event)
{
  const semantic_source_use::SourceTemplateIdOccurrence & occurrence =
      event.template_id_occurrence;
  return event.selection != SourceSelectionKind::Primary &&
         event.use_anchor_kind == semantic_source_use::SourceAnchorKind::Spelling &&
         !event.bindings.empty() &&
         occurrence.present &&
         occurrence.source_spelled &&
         occurrence.argument_list_spelled &&
         (occurrence.in_template_body ||
          typed_class_occurrence_arguments_match_bindings_exactly(event)) &&
         !occurrence.synthesized;
}

string binding_signature_key(const vector<WitnessBinding> & bindings)
{
  vector<string> parts;
  for(size_t i = 0; i < bindings.size(); ++i) {
    parts.push_back(bindings[i].param + "\x1f" + bindings[i].arg + "\x1f" +
                    bindings[i].source);
  }
  std::sort(parts.begin(), parts.end());
  string out;
  for(size_t i = 0; i < parts.size(); ++i) {
    if(i) {
      out += "\x1e";
    }
    out += parts[i];
  }
  return out;
}

string compact_binding_arg_key(const string & arg)
{
  string out;
  const string normalized = normalize_binding_arg_for_event(arg);
  out.reserve(normalized.size());
  for(size_t i = 0; i < normalized.size(); ++i) {
    if(std::isspace(static_cast<unsigned char>(normalized[i]))) {
      continue;
    }
    out.push_back(normalized[i]);
  }
  return out;
}

string normalized_binding_signature_key(const vector<WitnessBinding> & bindings)
{
  vector<string> parts;
  for(size_t i = 0; i < bindings.size(); ++i) {
    parts.push_back(bindings[i].param + "\x1f" +
                    compact_binding_arg_key(bindings[i].arg) + "\x1f" +
                    bindings[i].source);
  }
  std::sort(parts.begin(), parts.end());
  string out;
  for(size_t i = 0; i < parts.size(); ++i) {
    if(i) {
      out += "\x1e";
    }
    out += parts[i];
  }
  return out;
}

string binding_arg_signature_key(const vector<WitnessBinding> & bindings)
{
  vector<string> parts;
  for(size_t i = 0; i < bindings.size(); ++i) {
    parts.push_back(bindings[i].param + "\x1f" + bindings[i].arg);
  }
  std::sort(parts.begin(), parts.end());
  string out;
  for(size_t i = 0; i < parts.size(); ++i) {
    if(i) {
      out += "\x1e";
    }
    out += parts[i];
  }
  return out;
}

size_t defaulted_binding_count(const vector<WitnessBinding> & bindings)
{
  size_t count = 0;
  for(size_t i = 0; i < bindings.size(); ++i) {
    if(bindings[i].source == "defaulted") {
      ++count;
    }
  }
  return count;
}

size_t explicit_binding_count(const vector<WitnessBinding> & bindings)
{
  size_t count = 0;
  for(size_t i = 0; i < bindings.size(); ++i) {
    if(bindings[i].source == "explicit") {
      ++count;
    }
  }
  return count;
}

size_t deduced_binding_count(const vector<WitnessBinding> & bindings)
{
  size_t count = 0;
  for(size_t i = 0; i < bindings.size(); ++i) {
    if(bindings[i].source == "deduced") {
      ++count;
    }
  }
  return count;
}

int binding_source_preference_score(const WitnessEvent & event)
{
  const size_t defaulted =
      defaulted_binding_count(event.bindings) +
      defaulted_binding_count(event.specialization_bindings);
  const size_t explicit_count =
      explicit_binding_count(event.bindings) +
      explicit_binding_count(event.specialization_bindings);
  const size_t deduced =
      deduced_binding_count(event.bindings) +
      deduced_binding_count(event.specialization_bindings);
  return static_cast<int>(defaulted * 100 + explicit_count * 10) -
      static_cast<int>(deduced);
}

#if defined(CPPGM_ENABLE_WITNESS_PROVENANCE)

struct WitnessBuilder
{
  typedef tuple<string, string, string, string, string> EventKey;

  WitnessBuilder(const string & source_path_in,
                 const template_api::TemplateWitnessSession * session_in,
                 bool trace_in)
    : source_path(source_path_in), session(session_in), trace(trace_in)
  {}

  vector<WitnessEvent> function_calls;
  vector<witness_provenance::RendererEventLineage> function_call_lineages;
  vector<WitnessEvent> class_events;
  vector<witness_provenance::RendererEventLineage> class_lineages;
  vector<WitnessEvent> alias_events;
  vector<witness_provenance::RendererEventLineage> alias_lineages;
  map<EventKey, WitnessEvent> variable_events;
  map<EventKey, witness_provenance::RendererEventLineage> variable_lineages;
  string source_path;
  const template_api::TemplateWitnessSession * session = nullptr;
  bool trace = false;

  void note_build_action(
      const WitnessEvent & event,
      const string & action,
      const witness_provenance::RendererEventLineage & lineage)
  {
    if(!trace || !session) return;
    witness_provenance::note_renderer_action(
        *session,
        source_path,
        "build_events",
        action,
        lineage,
        witness_event_kind_text(event.kind),
        event.location,
        !event.template_name.empty() ? event.template_name : event.selected);
  }

  void consume_direct_event(
      const WitnessEvent & event,
      const witness_provenance::RendererEventLineage * lineage = nullptr)
  {
    if(event.kind == WitnessEventKind::ClassUse) {
      class_events.push_back(event);
      if(trace && lineage) class_lineages.push_back(*lineage);
      return;
    }
    if(event.kind == WitnessEventKind::AliasUse) {
      alias_events.push_back(event);
      if(trace && lineage) alias_lineages.push_back(*lineage);
      return;
    }
    if(event.kind == WitnessEventKind::VariableUse) {
      const EventKey key = event_key(event);
      map<EventKey, WitnessEvent>::iterator existing = variable_events.find(key);
      if(trace && lineage) {
        witness_provenance::RendererEventLineage retained = *lineage;
        if(existing != variable_events.end()) {
          retained = variable_lineages[key];
          merge_renderer_lineage(retained, *lineage);
          note_build_action(existing->second,
                            "replaced",
                            variable_lineages[key]);
        }
        variable_lineages[key] = retained;
      }
      variable_events[key] = event;
      return;
    }
    if(event.kind == WitnessEventKind::FunctionCall) {
      function_calls.push_back(event);
      if(trace && lineage) function_call_lineages.push_back(*lineage);
    }
  }

  vector<WitnessEvent> finish(
      vector<witness_provenance::RendererEventLineage> * out_lineages = nullptr)
  {
    vector<WitnessEvent> events;
    events.insert(events.end(), class_events.begin(), class_events.end());
    vector<witness_provenance::RendererEventLineage> lineages;
    if(trace) {
      lineages.insert(lineages.end(), class_lineages.begin(), class_lineages.end());
    }
    events.insert(events.end(), alias_events.begin(), alias_events.end());
    if(trace) {
      lineages.insert(lineages.end(),
                      alias_lineages.begin(),
                      alias_lineages.end());
    }
    for(map<EventKey, WitnessEvent>::const_iterator it = variable_events.begin();
        it != variable_events.end();
        ++it) {
      events.push_back(it->second);
      if(trace) lineages.push_back(variable_lineages[it->first]);
    }
    events.insert(events.end(), function_calls.begin(), function_calls.end());
    if(trace) {
      lineages.insert(lineages.end(),
                      function_call_lineages.begin(),
                      function_call_lineages.end());
      vector<size_t> order(events.size());
      for(size_t i = 0; i < order.size(); ++i) order[i] = i;
      std::stable_sort(order.begin(), order.end(),
                       [&](size_t lhs, size_t rhs)
                       {
                         return sort_key_less(events[lhs], events[rhs]);
                       });
      vector<WitnessEvent> sorted_events;
      vector<witness_provenance::RendererEventLineage> sorted_lineages;
      sorted_events.reserve(events.size());
      sorted_lineages.reserve(events.size());
      for(size_t i = 0; i < order.size(); ++i) {
        sorted_events.push_back(events[order[i]]);
        sorted_lineages.push_back(lineages[order[i]]);
      }
      events.swap(sorted_events);
      lineages.swap(sorted_lineages);
      if(out_lineages) out_lineages->swap(lineages);
    } else {
      std::stable_sort(events.begin(), events.end(), sort_key_less);
    }
    return events;
  }

  static bool sort_key_less(const WitnessEvent & lhs, const WitnessEvent & rhs)
  {
    return std::make_tuple(lhs.location,
                           lhs.kind,
                           !lhs.selected.empty() ? lhs.selected : lhs.template_name) <
        std::make_tuple(rhs.location,
                        rhs.kind,
                        !rhs.selected.empty() ? rhs.selected : rhs.template_name);
  }

  static EventKey event_key(const WitnessEvent & event)
  {
    const string payload =
        event.pattern + "|" +
        event.spelling + "|" +
        event.value + "|" +
        event.expanded_to + "|" +
        event.resolved + "|" +
        event.selected_decl_location + "|" +
        binding_signature_key(event.bindings) + "|" +
        binding_signature_key(event.specialization_bindings);
    return std::make_tuple(string(witness_event_kind_text(event.kind)),
                           event.location,
                           !event.selected.empty() ? event.selected : event.template_name,
                           event.selection != SourceSelectionKind::None ?
                               witness_selection_text(event) :
                               event.expanded_to,
                           payload);
  }
};

#else

struct WitnessBuilder
{
  explicit WitnessBuilder(const string &) {}

  vector<WitnessEvent> function_calls;
  vector<WitnessEvent> class_events;
  vector<WitnessEvent> alias_events;
  map<tuple<string, string, string, string, string>, WitnessEvent> variable_events;

  void consume_direct_event(const WitnessEvent & event)
  {
    if(event.kind == WitnessEventKind::ClassUse) {
      class_events.push_back(event);
      return;
    }
    if(event.kind == WitnessEventKind::AliasUse) {
      alias_events.push_back(event);
      return;
    }
    if(event.kind == WitnessEventKind::VariableUse) {
      variable_events[event_key(event)] = event;
      return;
    }
    if(event.kind == WitnessEventKind::FunctionCall) {
      function_calls.push_back(event);
    }
  }

  vector<WitnessEvent> finish()
  {
    vector<WitnessEvent> events;
    events.insert(events.end(), class_events.begin(), class_events.end());
    events.insert(events.end(), alias_events.begin(), alias_events.end());
    for(map<tuple<string, string, string, string, string>, WitnessEvent>::const_iterator
            it = variable_events.begin();
        it != variable_events.end();
        ++it) {
      events.push_back(it->second);
    }
    events.insert(events.end(), function_calls.begin(), function_calls.end());
    std::stable_sort(events.begin(), events.end(), sort_key_less);
    return events;
  }

  static bool sort_key_less(const WitnessEvent & lhs, const WitnessEvent & rhs)
  {
    return std::make_tuple(lhs.location,
                           lhs.kind,
                           !lhs.selected.empty() ? lhs.selected : lhs.template_name) <
        std::make_tuple(rhs.location,
                        rhs.kind,
                        !rhs.selected.empty() ? rhs.selected : rhs.template_name);
  }

  static tuple<string, string, string, string, string> event_key(
      const WitnessEvent & event)
  {
    const string payload =
        event.pattern + "|" +
        event.spelling + "|" +
        event.value + "|" +
        event.expanded_to + "|" +
        event.resolved + "|" +
        event.selected_decl_location + "|" +
        binding_signature_key(event.bindings) + "|" +
        binding_signature_key(event.specialization_bindings);
    return std::make_tuple(string(witness_event_kind_text(event.kind)),
                           event.location,
                           !event.selected.empty() ? event.selected : event.template_name,
                           event.selection != SourceSelectionKind::None ?
                               witness_selection_text(event) :
                               event.expanded_to,
                           payload);
  }
};

#endif

string header_from_kind(WitnessEventKind kind)
{
  if(kind == WitnessEventKind::ClassUse) {
    return "class-use";
  }
  if(kind == WitnessEventKind::AliasUse) {
    return "alias-use";
  }
  if(kind == WitnessEventKind::VariableUse) {
    return "variable-use";
  }
  return "function-call";
}

void normalize_event_names(vector<WitnessEvent> & events,
                           const vector<string> & names)
{
  for(size_t i = 0; i < events.size(); ++i) {
    events[i].template_name =
        witness_text::normalize_anonymous_namespace_segments(
            witness_text::strip_inline_namespace_segments(events[i].template_name,
                                                          names));
    events[i].selected =
        witness_text::normalize_anonymous_namespace_segments(
            witness_text::strip_inline_namespace_segments(events[i].selected,
                                                          names));
    events[i].resolved =
        witness_text::normalize_anonymous_namespace_segments(
            witness_text::strip_inline_namespace_segments(events[i].resolved,
                                                          names));
    events[i].expanded_to =
        witness_text::normalize_anonymous_namespace_segments(
            witness_text::strip_inline_namespace_segments(events[i].expanded_to,
                                                          names));
    for(size_t j = 0; j < events[i].bindings.size(); ++j) {
      events[i].bindings[j].arg =
          witness_text::normalize_anonymous_namespace_segments(
              witness_text::strip_inline_namespace_segments(events[i].bindings[j].arg,
                                                            names));
    }
    for(size_t j = 0; j < events[i].specialization_bindings.size(); ++j) {
      events[i].specialization_bindings[j].arg =
          witness_text::normalize_anonymous_namespace_segments(
              witness_text::strip_inline_namespace_segments(
                  events[i].specialization_bindings[j].arg,
                  names));
    }
    for(size_t j = 0; j < events[i].drops.size(); ++j) {
      events[i].drops[j].candidate =
          witness_text::normalize_anonymous_namespace_segments(
              witness_text::strip_inline_namespace_segments(
                  events[i].drops[j].candidate,
                  names));
    }
  }
}

string normalize_binding_arg_for_event(const string & arg,
                                       bool preserve_qualified_member,
                                       bool preserve_const_char_array)
{
  typedef tuple<string, bool, bool> CacheKey;
  static map<CacheKey, string> cache;
  const CacheKey key(arg, preserve_qualified_member, preserve_const_char_array);
  map<CacheKey, string>::const_iterator cached = cache.find(key);
  if(cached != cache.end()) {
    return cached->second;
  }
  static const std::regex libcxx_namespace_regex("std::__1::");
  static const std::regex qualified_local_regex(
      "\\b([A-Za-z_][A-Za-z0-9_]*::)+([A-Za-z_][A-Za-z0-9_]*__local_\\d+)");
  static const std::regex local_regex("__local_\\d+");
  static const std::regex member_typedef_regex(
      "\\b([A-Za-z_][A-Za-z0-9_]*)::(value_type|iterator|const_iterator|reference|const_reference)\\b");
  static const std::regex separated_angle_regex(">\\s+>");
  string value = arg;
  value = std::regex_replace(value, libcxx_namespace_regex, "std::");
  replace_all(value, "(*(&))", "(*&)");
  replace_all(value, "(*(&&))", "(*&&)");
  replace_all(value, "(^(&))", "(^&)");
  replace_all(value, "(^(&&))", "(^&&)");
  value = std::regex_replace(value, qualified_local_regex, "$2");
  value = std::regex_replace(value, local_regex, "");
  value = normalize_const_order(value);
  value = normalize_source_event_type_spellings(value);
  if(!preserve_qualified_member) {
    value = std::regex_replace(value, member_typedef_regex, "$2");
  }
  while(true) {
    const string collapsed =
        std::regex_replace(value, separated_angle_regex, ">>");
    if(collapsed == value) {
      break;
    }
    value = collapsed;
  }
  value = witness_text::normalize_source_spelling_text(value);
  const string trimmed = trim_space(value);
  if((trimmed.size() >= 17 && trimmed.compare(0, 16, "decltype(delete ") == 0) ||
     (trimmed.size() >= 19 && trimmed.compare(0, 18, "__decltype(delete ") == 0) ||
     (trimmed.size() >= 21 && trimmed.compare(0, 20, "__decltype__(delete ") == 0)) {
    cache[key] = "void";
    return "void";
  }
  const string normalized = normalize_function_template_argument_spacing(value);
  const string anonymous_normalized =
      witness_text::normalize_anonymous_namespace_segments(
          preserve_const_char_array ?
              normalized :
              normalize_string_literal_array_witness_text(normalized));
  cache[key] = anonymous_normalized;
  return anonymous_normalized;
}

bool function_template_name_before_angle(const string & text)
{
  size_t end = text.size();
  while(end > 0 && std::isspace(static_cast<unsigned char>(text[end - 1]))) {
    --end;
  }
  size_t begin = end;
  while(begin > 0) {
    const char ch = text[begin - 1];
    if(std::isalnum(static_cast<unsigned char>(ch)) ||
       ch == '_' ||
       ch == ':') {
      --begin;
      continue;
    }
    break;
  }
  const string name = text.substr(begin, end - begin);
  return name == "function" ||
         (name.size() > 10 &&
          name.compare(name.size() - 10, 10, "::function") == 0);
}

string normalize_function_template_argument_spacing(const string & text)
{
  string out;
  out.reserve(text.size());
  vector<char> angle_stack;
  for(size_t i = 0; i < text.size(); ++i) {
    const char ch = text[i];
    if(ch == '<') {
      angle_stack.push_back(function_template_name_before_angle(out));
      out.push_back(ch);
      continue;
    }
    if(ch == '>') {
      if(!angle_stack.empty()) {
        angle_stack.pop_back();
      }
      out.push_back(ch);
      continue;
    }
    if(ch == '(' &&
       !angle_stack.empty() &&
       angle_stack.back() &&
       !out.empty() &&
       !std::isspace(static_cast<unsigned char>(out[out.size() - 1]))) {
      out.push_back(' ');
    }
    out.push_back(ch);
  }
  return out;
}

string normalize_binding_arg_for_event(const string & arg)
{
  return normalize_binding_arg_for_event(arg, false);
}

string normalize_type_like_function_declarator_spacing(const string & text)
{
  string out = trim_space(text);
  int angle_depth = 0;
  for(size_t i = 0; i < out.size(); ++i) {
    const char ch = out[i];
    if(ch == '<') {
      ++angle_depth;
      continue;
    }
    if(ch == '>' && angle_depth > 0) {
      --angle_depth;
      continue;
    }
    if(ch == '(' &&
       angle_depth == 0 &&
       i > 0 &&
       out[i - 1] == '>') {
      out.insert(i, " ");
      break;
    }
  }
  return out;
}

string normalize_template_closing_angle_spacing(const string & text)
{
  string out;
  out.reserve(text.size());
  for(size_t i = 0; i < text.size(); ++i) {
    if(text[i] == '>') {
      while(!out.empty() &&
            std::isspace(static_cast<unsigned char>(out[out.size() - 1]))) {
        out.resize(out.size() - 1);
      }
    }
    out.push_back(text[i]);
  }
  return trim_space(out);
}

string normalize_entity_name_for_event(const string & entity)
{
  return normalize_template_closing_angle_spacing(
      normalize_binding_arg_for_event(entity));
}

string normalize_binding_arg_for_event(const WitnessBinding & binding)
{
  const string normalized =
      normalize_binding_arg_for_event(binding.arg,
                                      binding.preserve_qualified_member,
                                      binding.source == "explicit");
  const string normalized_only =
      normalized == "unsigned" ? "unsigned int" : normalized;
  if(binding.type_like) {
    return normalize_template_closing_angle_spacing(
        normalize_type_like_function_declarator_spacing(normalized_only));
  }
  return normalized_only;
}

string make_bound_template_text(const string & template_name,
                                const vector<WitnessBinding> & bindings,
                                size_t count)
{
  string text = template_name;
  text += "<";
  for(size_t i = 0; i < count; ++i) {
    if(i != 0) {
      text += ", ";
    }
    text += bindings[i].arg;
  }
  text += ">";
  return text;
}

string top_level_owner_entity_text(const string & entity)
{
  const string value = trim_space(entity);
  int angle_depth = 0;
  for(int i = static_cast<int>(value.size()) - 1; i > 0; --i) {
    const char ch = value[static_cast<size_t>(i)];
    if(ch == '>') {
      ++angle_depth;
      continue;
    }
    if(ch == '<' && angle_depth > 0) {
      --angle_depth;
      continue;
    }
    if(ch == ':' &&
       value[static_cast<size_t>(i - 1)] == ':' &&
       angle_depth == 0) {
      return trim_space(value.substr(0, static_cast<size_t>(i - 1)));
    }
  }
  return string();
}

string bound_class_use_entity_text(const WitnessEvent & event)
{
  if(event.bindings.empty()) {
    return normalize_source_event_entity_text(
        strip_top_level_template_suffix(event.template_name));
  }
  return normalize_source_event_entity_text(
      make_bound_template_text(event.template_name,
                               event.bindings,
                               event.bindings.size()));
}

bool same_line_event_binding_mentions_bound_class_use(
    const vector<WitnessEvent> & events,
    size_t index)
{
  if(index >= events.size() || events[index].kind != WitnessEventKind::ClassUse) {
    return false;
  }
  const ParsedLocation target = parse_line_col(
      !events[index].location.empty() ? events[index].location :
                                        events[index].raw_location);
  if(target.line <= 0) {
    return false;
  }
  const string bound = bound_class_use_entity_text(events[index]);
  if(bound.empty()) {
    return false;
  }
  const string target_template =
      normalize_source_event_entity_text(
          strip_top_level_template_suffix(events[index].template_name));
  for(size_t i = 0; i < events.size(); ++i) {
    if(i == index) {
      continue;
    }
    const ParsedLocation candidate = parse_line_col(
        !events[i].location.empty() ? events[i].location :
                                      events[i].raw_location);
    if(candidate.line != target.line) {
      continue;
    }
    const string candidate_template =
        normalize_source_event_entity_text(
            strip_top_level_template_suffix(events[i].template_name));
    if(!target_template.empty() && candidate_template == target_template) {
      continue;
    }
    for(size_t j = 0; j < events[i].bindings.size(); ++j) {
      if(normalize_source_event_entity_text(events[i].bindings[j].arg) ==
         bound) {
        return true;
      }
    }
    for(size_t j = 0; j < events[i].specialization_bindings.size(); ++j) {
      if(normalize_source_event_entity_text(
             events[i].specialization_bindings[j].arg) == bound) {
        return true;
      }
    }
  }
  return false;
}

string strip_anonymous_namespace_qualifiers(const string & text)
{
  string out = text;
  replace_all(out, "::(anonymous namespace)::", "::");
  if(out.compare(0, 23, "(anonymous namespace)::") == 0) {
    out = out.substr(23);
  }
  return out;
}

string visible_event_signature(const WitnessEvent & event)
{
  std::ostringstream out;
  out << witness_event_kind_text(event.kind) << "\x1f"
      << event.location << "\x1f"
      << event.template_name << "\x1f"
      << event.selected << "\x1f"
      << witness_selection_text(event);
  for(size_t i = 0; i < event.bindings.size(); ++i) {
    out << "\x1e" << "B" << i << "=" << event.bindings[i].arg
        << "[" << event.bindings[i].source << "]";
  }
  for(size_t i = 0; i < event.specialization_bindings.size(); ++i) {
    out << "\x1e" << "S" << i << "="
        << event.specialization_bindings[i].arg << "["
        << event.specialization_bindings[i].source << "]";
  }
  for(size_t i = 0; i < event.drops.size(); ++i) {
    out << "\x1e" << "D" << event.drops[i].candidate << "="
        << event.drops[i].reason;
  }
  return out.str();
}

string visible_event_signature_without_binding_sources(const WitnessEvent & event)
{
  std::ostringstream out;
  out << witness_event_kind_text(event.kind) << "\x1f"
      << event.location << "\x1f"
      << event.template_name << "\x1f"
      << event.selected << "\x1f"
      << witness_selection_text(event) << "\x1f"
      << event.selected_decl_location << "\x1f"
      << event.resolved << "\x1f"
      << event.expanded_to << "\x1f"
      << event.value;
  for(size_t i = 0; i < event.bindings.size(); ++i) {
    out << "\x1e" << "B" << i << "=" << event.bindings[i].arg;
  }
  for(size_t i = 0; i < event.specialization_bindings.size(); ++i) {
    out << "\x1e" << "S" << i << "=" << event.specialization_bindings[i].arg;
  }
  for(size_t i = 0; i < event.drops.size(); ++i) {
    out << "\x1e" << "D" << event.drops[i].candidate << "="
        << event.drops[i].reason;
  }
  return out.str();
}

string rendered_event_signature_without_binding_sources(const WitnessEvent & event)
{
  const string rendered_name =
      event.kind == WitnessEventKind::FunctionCall ?
          (!event.selected.empty() ? event.selected : event.template_name) :
          event.template_name;
  const ParsedLocation rendered_location = parse_line_col(event.location);
  const string rendered_path = location_source_path(event.location);
  std::ostringstream location_key;
  if(!rendered_path.empty() && rendered_location.line > 0) {
    location_key << rendered_path << ":" << rendered_location.line << ":"
                 << rendered_location.column;
  } else {
    location_key << event.location;
  }
  std::ostringstream out;
  out << witness_event_kind_text(event.kind) << "\x1f"
      << location_key.str() << "\x1f"
      << rendered_name << "\x1f"
      << witness_selection_text(event) << "\x1f"
      << event.resolved << "\x1f"
      << event.expanded_to << "\x1f"
      << event.value;
  for(size_t i = 0; i < event.bindings.size(); ++i) {
    out << "\x1e" << "B" << i << "=" << event.bindings[i].param
        << "=" << event.bindings[i].arg;
  }
  for(size_t i = 0; i < event.specialization_bindings.size(); ++i) {
    out << "\x1e" << "S" << i << "="
        << event.specialization_bindings[i].param << "="
        << event.specialization_bindings[i].arg;
  }
  for(size_t i = 0; i < event.drops.size(); ++i) {
    out << "\x1e" << "D" << event.drops[i].candidate << "="
        << event.drops[i].reason;
  }
  return out.str();
}

string event_signature_without_location(const WitnessEvent & event)
{
  std::ostringstream out;
  out << witness_event_kind_text(event.kind) << "\x1f"
      << event.template_name << "\x1f"
      << event.selected << "\x1f"
      << witness_selection_text(event);
  for(size_t i = 0; i < event.bindings.size(); ++i) {
    out << "\x1e" << "B" << i << "=" << event.bindings[i].arg
        << "[" << event.bindings[i].source << "]";
  }
  for(size_t i = 0; i < event.specialization_bindings.size(); ++i) {
    out << "\x1e" << "S" << i << "="
        << event.specialization_bindings[i].arg << "["
        << event.specialization_bindings[i].source << "]";
  }
  for(size_t i = 0; i < event.drops.size(); ++i) {
    out << "\x1e" << "D" << event.drops[i].candidate << "="
        << event.drops[i].reason;
  }
  return out.str();
}

bool all_primary_bindings_deduced(const WitnessEvent & event)
{
  if(event.bindings.empty()) {
    return false;
  }
  for(size_t i = 0; i < event.bindings.size(); ++i) {
    if(event.bindings[i].source != "deduced") {
      return false;
    }
  }
  return true;
}

string source_line_location_key(const string & location)
{
  const ParsedLocation parsed = parse_line_col(location);
  if(parsed.line <= 0) {
    return string();
  }
  return location_source_path(location) + ":" + std::to_string(parsed.line);
}

bool event_has_same_line_template_id_occurrence(const WitnessEvent & event)
{
  const ParsedLocation raw = parse_line_col(
      !event.raw_location.empty() ? event.raw_location : event.location);
  if(raw.line <= 0) {
    return false;
  }
  SourceOccurrence occurrence;
  if(!source_occurrence_from_typed_template_id(event, occurrence)) {
    return false;
  }
  return parse_line_col(occurrence.location).line == raw.line;
}

bool function_call_contains_deduced_binding_sequence(
    const WitnessEvent & function_event,
    const vector<WitnessBinding> & class_bindings)
{
  if(function_event.kind != WitnessEventKind::FunctionCall ||
     class_bindings.empty() ||
     function_event.bindings.size() < class_bindings.size()) {
    return false;
  }
  vector<string> class_args;
  for(size_t i = 0; i < class_bindings.size(); ++i) {
    class_args.push_back(compact_binding_arg_key(class_bindings[i].arg));
  }
  for(size_t start = 0;
      start + class_args.size() <= function_event.bindings.size();
      ++start) {
    bool match = true;
    for(size_t i = 0; i < class_args.size(); ++i) {
      const WitnessBinding & binding = function_event.bindings[start + i];
      if(binding.source != "deduced" ||
         compact_binding_arg_key(binding.arg) != class_args[i]) {
        match = false;
        break;
      }
    }
    if(match) {
      return true;
    }
  }
  return false;
}

bool same_template_binding_arguments(const WitnessEvent & lhs,
                                     const WitnessEvent & rhs)
{
  if(lhs.template_name != rhs.template_name ||
     lhs.bindings.size() != rhs.bindings.size()) {
    return false;
  }
  for(size_t i = 0; i < lhs.bindings.size(); ++i) {
    if(compact_binding_arg_key(lhs.bindings[i].arg) !=
       compact_binding_arg_key(rhs.bindings[i].arg)) {
      return false;
    }
  }
  return true;
}

bool has_non_deduced_class_use_peer(const vector<WitnessEvent> & events,
                                    size_t index)
{
  for(size_t i = 0; i < events.size(); ++i) {
    if(i == index ||
       events[i].kind != WitnessEventKind::ClassUse ||
       events[i].selection != SourceSelectionKind::Primary ||
       !events[i].specialization_bindings.empty() ||
       all_primary_bindings_deduced(events[i]) ||
       !same_template_binding_arguments(events[i], events[index])) {
      continue;
    }
    return true;
  }
  return false;
}

void drop_class_uses_redundant_with_function_deduction(
    vector<WitnessEvent> & events)
{
  vector<char> drop(events.size(), 0);
  for(size_t i = 0; i < events.size(); ++i) {
    const bool materialized_declaration_type_spelling =
        events[i].ownership == SourceUseOwnership::SourceOwned &&
        events[i].source_role ==
            semantic_source_use::SourceUseRole::MaterializedTypeUse &&
        events[i].use_anchor_kind == semantic_source_use::SourceAnchorKind::Spelling &&
        !events[i].template_id_occurrence.present;
    if(events[i].kind != WitnessEventKind::ClassUse ||
       events[i].selection != SourceSelectionKind::Primary ||
       !events[i].specialization_bindings.empty() ||
       !all_primary_bindings_deduced(events[i]) ||
       !has_non_deduced_class_use_peer(events, i) ||
       event_has_same_line_template_id_occurrence(events[i]) ||
       materialized_declaration_type_spelling) {
      continue;
    }
    for(size_t j = 0; j < events.size(); ++j) {
      if(function_call_contains_deduced_binding_sequence(events[j],
                                                         events[i].bindings)) {
        drop[i] = 1;
        break;
      }
    }
  }
  compact_events(events, drop);
}

void drop_redundant_nested_events(vector<WitnessEvent> & events)
{
  set<string> direct_signatures;
  for(size_t i = 0; i < events.size(); ++i) {
    if(events[i].kind != WitnessEventKind::ClassUse &&
       events[i].kind != WitnessEventKind::AliasUse &&
       events[i].ownership != SourceUseOwnership::NestedDerived) {
      direct_signatures.insert(event_signature_without_location(events[i]));
    }
  }

  vector<char> drop(events.size(), 0);
  for(size_t i = 0; i < events.size(); ++i) {
    if(events[i].kind == WitnessEventKind::ClassUse ||
       events[i].kind == WitnessEventKind::AliasUse ||
       events[i].ownership != SourceUseOwnership::NestedDerived) {
      continue;
    }
    if(direct_signatures.count(event_signature_without_location(events[i])) != 0) {
      drop[i] = 1;
    }
  }
  compact_events(events, drop);
}

string apply_text_aliases(const string & text,
                          const map<string, string> & aliases)
{
  string out = text;
  for(map<string, string>::const_iterator it = aliases.begin();
      it != aliases.end();
      ++it) {
    replace_all(out, it->first, it->second);
  }
  return out;
}

map<string, string> build_defaulted_class_aliases(
    const vector<WitnessEvent> & events,
    bool include_concrete_explicit_specializations = true)
{
  map<string, string> aliases;
  for(size_t i = 0; i < events.size(); ++i) {
    if(events[i].kind != WitnessEventKind::ClassUse ||
       events[i].bindings.empty()) {
      continue;
    }
    if(!include_concrete_explicit_specializations &&
       events[i].selection == SourceSelectionKind::ExplicitSpecialization) {
      const ParsedLocation use = parse_line_col(events[i].location);
      const ParsedLocation decl = parse_line_col(events[i].selected_decl_location);
      const bool source_omitted_trailing_arguments =
          events[i].template_id_occurrence.present &&
          events[i].template_id_occurrence.arguments.size() <
              events[i].bindings.size();
      if((use.line <= 0 || decl.line <= 0 || decl.line <= use.line) &&
         !source_omitted_trailing_arguments) {
        continue;
      }
    }
    size_t keep_count = events[i].bindings.size();
    while(keep_count > 0 &&
          events[i].bindings[keep_count - 1].source == "defaulted") {
      --keep_count;
    }
    if(keep_count == events[i].bindings.size()) {
      continue;
    }
    const string full =
        make_bound_template_text(events[i].template_name,
                                 events[i].bindings,
                                 events[i].bindings.size());
    const string shortened =
        make_bound_template_text(events[i].template_name,
                                 events[i].bindings,
                                 keep_count);
    aliases[full] = shortened;
  }
  return aliases;
}

map<string, string> build_predecl_all_defaulted_class_aliases(
    const vector<WitnessEvent> & events)
{
  map<string, string> aliases;
  for(size_t i = 0; i < events.size(); ++i) {
    if(events[i].kind != WitnessEventKind::ClassUse || events[i].bindings.empty()) {
      continue;
    }
    bool all_defaulted = true;
    for(size_t j = 0; j < events[i].bindings.size(); ++j) {
      if(events[i].bindings[j].source != "defaulted") {
        all_defaulted = false;
        break;
      }
    }
    if(!all_defaulted) {
      continue;
    }
    const ParsedLocation raw =
        parse_line_col(events[i].location);
    const ParsedLocation decl =
        parse_line_col(events[i].selected_decl_location);
    if(raw.line <= 0 || decl.line <= 0 || raw.line >= decl.line) {
      continue;
    }
    const string full =
        make_bound_template_text(events[i].template_name,
                                 events[i].bindings,
                                 events[i].bindings.size());
    const string shortened =
        make_bound_template_text(events[i].template_name,
                                 events[i].bindings,
                                 0);
    aliases[full] = shortened;
  }
  return aliases;
}

bool source_has_empty_template_id_at_event_location(const WitnessEvent & event)
{
  const semantic_source_use::SourceTemplateIdOccurrence & occurrence =
      event.template_id_occurrence;
  return occurrence.present &&
         occurrence.source_spelled &&
         occurrence.argument_list_spelled &&
         occurrence.empty_argument_list;
}

map<string, string> build_omitted_all_defaulted_class_aliases(
    const vector<WitnessEvent> & events)
{
  map<string, string> aliases;
  for(size_t i = 0; i < events.size(); ++i) {
    if(events[i].kind != WitnessEventKind::ClassUse || events[i].bindings.empty()) {
      continue;
    }
    bool all_defaulted = true;
    for(size_t j = 0; j < events[i].bindings.size(); ++j) {
      if(events[i].bindings[j].source != "defaulted") {
        all_defaulted = false;
        break;
      }
    }
    if(!all_defaulted ||
       !source_has_empty_template_id_at_event_location(events[i])) {
      continue;
    }
    const string full =
        make_bound_template_text(events[i].template_name,
                                 events[i].bindings,
                                 events[i].bindings.size());
    const string shortened =
        make_bound_template_text(events[i].template_name,
                                 events[i].bindings,
                                 0);
    aliases[full] = shortened;
  }
  return aliases;
}

map<string, string> build_function_pointer_class_aliases(
    const vector<WitnessEvent> & events)
{
  map<string, string> aliases;
  for(size_t i = 0; i < events.size(); ++i) {
    if(events[i].kind != WitnessEventKind::ClassUse || events[i].bindings.empty()) {
      continue;
    }
    vector<WitnessBinding> unqualified_bindings = events[i].bindings;
    bool changed = false;
    for(size_t j = 0; j < unqualified_bindings.size(); ++j) {
      const string arg = trim_space(unqualified_bindings[j].arg);
      if(arg.size() <= 1 || arg[0] != '&') {
        continue;
      }
      const string without_amp = trim_space(arg.substr(1));
      if(!is_simple_identifier_text(without_amp)) {
        continue;
      }
      unqualified_bindings[j].arg = without_amp;
      changed = true;
    }
    if(!changed) {
      continue;
    }
    const string canonical =
        make_bound_template_text(events[i].template_name,
                                 events[i].bindings,
                                 events[i].bindings.size());
    const string shorthand =
        make_bound_template_text(events[i].template_name,
                                 unqualified_bindings,
                                 unqualified_bindings.size());
    aliases[shorthand] = canonical;
  }
  return aliases;
}

void apply_binding_aliases(vector<WitnessEvent> & events,
                          const map<string, string> & aliases)
{
  if(aliases.empty()) {
    return;
  }
  for(size_t i = 0; i < events.size(); ++i) {
    for(size_t j = 0; j < events[i].bindings.size(); ++j) {
      events[i].bindings[j].arg =
          apply_text_aliases(events[i].bindings[j].arg, aliases);
      for(size_t k = 0; k < events[i].bindings[j].pack_arguments.size(); ++k) {
        events[i].bindings[j].pack_arguments[k] =
            apply_text_aliases(events[i].bindings[j].pack_arguments[k],
                               aliases);
      }
    }
    for(size_t j = 0; j < events[i].specialization_bindings.size(); ++j) {
      events[i].specialization_bindings[j].arg =
          apply_text_aliases(events[i].specialization_bindings[j].arg, aliases);
      for(size_t k = 0;
          k < events[i].specialization_bindings[j].pack_arguments.size();
          ++k) {
        events[i].specialization_bindings[j].pack_arguments[k] =
            apply_text_aliases(
                events[i].specialization_bindings[j].pack_arguments[k],
                aliases);
      }
    }
    events[i].template_name =
        apply_text_aliases(events[i].template_name, aliases);
    events[i].resolved =
        apply_text_aliases(events[i].resolved, aliases);
    if(!events[i].expanded_to.empty()) {
      events[i].expanded_to =
          apply_text_aliases(events[i].expanded_to, aliases);
    }
    if(!events[i].value.empty()) {
      events[i].value =
          apply_text_aliases(events[i].value, aliases);
    }
    if(!events[i].selected.empty()) {
      events[i].selected =
          apply_text_aliases(events[i].selected, aliases);
    }
    for(size_t j = 0; j < events[i].drops.size(); ++j) {
      events[i].drops[j].candidate =
          apply_text_aliases(events[i].drops[j].candidate, aliases);
    }
  }
}

void apply_event_name_aliases(vector<WitnessEvent> & events,
                              const map<string, string> & aliases)
{
  if(aliases.empty()) {
    return;
  }
  for(size_t i = 0; i < events.size(); ++i) {
    events[i].template_name =
        apply_text_aliases(events[i].template_name, aliases);
    events[i].resolved =
        apply_text_aliases(events[i].resolved, aliases);
    if(!events[i].expanded_to.empty()) {
      events[i].expanded_to =
          apply_text_aliases(events[i].expanded_to, aliases);
    }
    if(!events[i].selected.empty()) {
      events[i].selected =
          apply_text_aliases(events[i].selected, aliases);
    }
    for(size_t j = 0; j < events[i].drops.size(); ++j) {
      events[i].drops[j].candidate =
          apply_text_aliases(events[i].drops[j].candidate, aliases);
    }
  }
}

set<string> same_line_explicit_default_alias_uses(
    const vector<WitnessEvent> & events,
    const map<string, string> & aliases)
{
  set<string> out;
  if(aliases.empty()) {
    return out;
  }
  for(size_t i = 0; i < events.size(); ++i) {
    if(events[i].kind != WitnessEventKind::ClassUse ||
       !events[i].template_id_occurrence.present ||
       !events[i].template_id_occurrence.source_spelled) {
      continue;
    }
    bool has_defaulted_binding = false;
    for(size_t j = 0; j < events[i].bindings.size(); ++j) {
      if(events[i].bindings[j].source == "defaulted") {
        has_defaulted_binding = true;
        break;
      }
    }
    if(has_defaulted_binding) {
      continue;
    }
    const string line_key = source_line_location_key(events[i].location);
    if(line_key.empty()) {
      continue;
    }
    const string full =
        make_bound_template_text(events[i].template_name,
                                 events[i].bindings,
                                 events[i].bindings.size());
    if(aliases.find(full) != aliases.end()) {
      out.insert(line_key + "\x1f" + full);
    }
  }
  return out;
}

map<string, int> explicit_specialization_default_alias_pattern_lines(
    const vector<WitnessEvent> & events,
    const map<string, string> & aliases)
{
  map<string, int> out;
  if(aliases.empty()) {
    return out;
  }
  for(size_t i = 0; i < events.size(); ++i) {
    if(events[i].kind != WitnessEventKind::ClassUse ||
       events[i].selection != SourceSelectionKind::ExplicitSpecialization) {
      continue;
    }
    const string full =
        make_bound_template_text(events[i].template_name,
                                 events[i].bindings,
                                 events[i].bindings.size());
    if(aliases.find(full) == aliases.end()) {
      continue;
    }
    const ParsedLocation use = parse_line_col(events[i].location);
    const ParsedLocation decl = parse_line_col(events[i].selected_decl_location);
    if(use.line <= 0 || decl.line <= 0 || decl.line >= use.line) {
      continue;
    }
    map<string, int>::iterator found = out.find(full);
    if(found == out.end() || use.line < found->second) {
      out[full] = use.line;
    }
  }
  return out;
}

string apply_defaulted_aliases_to_binding_arg(
    const string & arg,
    const string & source,
    const string & line_key,
    int line_number,
    const map<string, string> & aliases,
    const set<string> & protected_explicit_uses,
    const map<string, int> & explicit_specialization_pattern_lines)
{
  string out = arg;
  for(map<string, string>::const_iterator it = aliases.begin();
      it != aliases.end();
      ++it) {
    map<string, int>::const_iterator pattern_line =
        explicit_specialization_pattern_lines.find(it->first);
    if(pattern_line != explicit_specialization_pattern_lines.end() &&
       line_number > pattern_line->second) {
      continue;
    }
    if(source == "explicit" &&
       !line_key.empty() &&
       protected_explicit_uses.count(line_key + "\x1f" + it->first) != 0) {
      continue;
    }
    replace_all(out, it->first, it->second);
  }
  return out;
}

void apply_defaulted_binding_aliases(vector<WitnessEvent> & events,
                                     const map<string, string> & aliases)
{
  if(aliases.empty()) {
    return;
  }
  const set<string> protected_explicit_uses =
      same_line_explicit_default_alias_uses(events, aliases);
  const map<string, int> explicit_specialization_pattern_lines =
      explicit_specialization_default_alias_pattern_lines(events, aliases);
  for(size_t i = 0; i < events.size(); ++i) {
    const string line_key = source_line_location_key(events[i].location);
    const int line_number = parse_line_col(events[i].location).line;
    for(size_t j = 0; j < events[i].bindings.size(); ++j) {
      events[i].bindings[j].arg =
          apply_defaulted_aliases_to_binding_arg(
              events[i].bindings[j].arg,
              events[i].bindings[j].source,
              line_key,
              line_number,
              aliases,
              protected_explicit_uses,
              explicit_specialization_pattern_lines);
      for(size_t k = 0; k < events[i].bindings[j].pack_arguments.size(); ++k) {
        events[i].bindings[j].pack_arguments[k] =
            apply_defaulted_aliases_to_binding_arg(
                events[i].bindings[j].pack_arguments[k],
                events[i].bindings[j].source,
                line_key,
                line_number,
                aliases,
                protected_explicit_uses,
                explicit_specialization_pattern_lines);
      }
    }
    for(size_t j = 0; j < events[i].specialization_bindings.size(); ++j) {
      events[i].specialization_bindings[j].arg =
          apply_defaulted_aliases_to_binding_arg(
              events[i].specialization_bindings[j].arg,
              events[i].specialization_bindings[j].source,
              line_key,
              line_number,
              aliases,
              protected_explicit_uses,
              explicit_specialization_pattern_lines);
      for(size_t k = 0;
          k < events[i].specialization_bindings[j].pack_arguments.size();
          ++k) {
        events[i].specialization_bindings[j].pack_arguments[k] =
            apply_defaulted_aliases_to_binding_arg(
                events[i].specialization_bindings[j].pack_arguments[k],
                events[i].specialization_bindings[j].source,
                line_key,
                line_number,
                aliases,
                protected_explicit_uses,
                explicit_specialization_pattern_lines);
      }
    }
  }
}

void normalize_event_bindings(vector<WitnessEvent> & events,
                              const string & input_path)
{
  for(size_t i = 0; i < events.size(); ++i) {
    for(size_t j = 0; j < events[i].bindings.size(); ++j) {
      events[i].bindings[j].arg =
          normalize_binding_arg_for_event(events[i].bindings[j]);
    }
    for(size_t j = 0; j < events[i].specialization_bindings.size(); ++j) {
      events[i].specialization_bindings[j].arg =
          normalize_binding_arg_for_event(events[i].specialization_bindings[j]);
    }
    events[i].template_name =
        normalize_entity_name_for_event(events[i].template_name);
    events[i].resolved =
        normalize_entity_name_for_event(events[i].resolved);
    if(!events[i].expanded_to.empty()) {
      events[i].expanded_to =
          normalize_entity_name_for_event(events[i].expanded_to);
    }
    if(!events[i].value.empty()) {
      events[i].value =
          normalize_binding_arg_for_event(events[i].value);
    }
    if(!events[i].selected.empty()) {
      events[i].selected =
          normalize_entity_name_for_event(events[i].selected);
    }
    for(size_t j = 0; j < events[i].drops.size(); ++j) {
      events[i].drops[j].candidate =
          normalize_entity_name_for_event(events[i].drops[j].candidate);
    }
  }
  canonicalize_function_pointer_binding_args(events);
  apply_alias_source_spelled_binding_args(events);
  canonicalize_is_same_partial_bindings(events);
  canonicalize_qualified_binding_arguments(events);
  drop_class_uses_redundant_with_function_deduction(events);
  for(size_t i = 0; i < events.size(); ++i) {
    if(events[i].kind != WitnessEventKind::AliasUse) {
      continue;
    }
    vector<WitnessBinding> source_written_bindings;
    for(size_t j = 0; j < events[i].bindings.size(); ++j) {
      if(events[i].bindings[j].source != "defaulted") {
        source_written_bindings.push_back(events[i].bindings[j]);
      }
    }
    events[i].bindings.swap(source_written_bindings);
  }
  const map<string, string> aliases = build_defaulted_class_aliases(events);
  apply_event_name_aliases(events, aliases);
  apply_defaulted_binding_aliases(events, aliases);
  apply_binding_aliases(events, build_function_pointer_class_aliases(events));
  apply_binding_aliases(events, build_predecl_all_defaulted_class_aliases(events));
  for(size_t i = 0; i < events.size(); ++i) {
    if(events[i].kind != WitnessEventKind::ClassUse ||
       events[i].selection != SourceSelectionKind::Primary ||
       !events[i].specialization_bindings.empty() ||
       events[i].bindings.empty()) {
      continue;
    }
    const string full =
        make_bound_template_text(events[i].template_name,
                                 events[i].bindings,
                                 events[i].bindings.size());
    map<string, string>::const_iterator found = aliases.find(full);
    if(found == aliases.end()) {
      continue;
    }
    size_t keep_count = events[i].bindings.size();
    while(keep_count > 0) {
      if(make_bound_template_text(events[i].template_name,
                                  events[i].bindings,
                                  keep_count) == found->second) {
        break;
      }
      --keep_count;
    }
    if(keep_count == 0 || keep_count == events[i].bindings.size()) {
      continue;
    }
    for(size_t j = keep_count; j < events[i].bindings.size(); ++j) {
      events[i].bindings[j].source = "defaulted";
    }
  }

  {
    vector<char> drop(events.size(), 0);
    map<string, size_t> preferred;
    for(size_t i = 0; i < events.size(); ++i) {
      if(events[i].kind != WitnessEventKind::ClassUse ||
         events[i].selection != SourceSelectionKind::Primary ||
         !events[i].specialization_bindings.empty() ||
         events[i].bindings.empty()) {
        continue;
      }
      const string full =
          make_bound_template_text(events[i].template_name,
                                   events[i].bindings,
                                   events[i].bindings.size());
      const string canonical = apply_text_aliases(full, aliases);
      if(canonical == full) {
        continue;
      }
      const string key = events[i].location + "\x1f" +
          events[i].template_name + "\x1f" +
          events[i].selected_decl_location + "\x1f" +
          witness_selection_text(events[i]) + "\x1f" + canonical;
      size_t defaulted = 0;
      for(size_t j = 0; j < events[i].bindings.size(); ++j) {
        if(events[i].bindings[j].source == "defaulted") {
          ++defaulted;
        }
      }
      map<string, size_t>::iterator found = preferred.find(key);
      if(found == preferred.end()) {
        preferred[key] = i;
        continue;
      }
      size_t best = found->second;
      size_t best_defaulted = 0;
      for(size_t j = 0; j < events[best].bindings.size(); ++j) {
        if(events[best].bindings[j].source == "defaulted") {
          ++best_defaulted;
        }
      }
      if(defaulted > best_defaulted) {
        drop[best] = 1;
        found->second = i;
      } else {
        drop[i] = 1;
      }
    }
    compact_events(events, drop);
  }
}

void canonicalize_event_locations_and_dedupe(vector<WitnessEvent> & events,
                                             const string & input_path)
{
  const string normalized_input = normalize_witness_path(input_path);
  for(size_t i = 0; i < events.size(); ++i) {
    if(events[i].kind != WitnessEventKind::VariableUse) {
      continue;
    }
    if(events[i].selection == SourceSelectionKind::Primary) {
      continue;
    }
    const bool has_structured_decl_anchor =
        (events[i].selected_decl_anchor_kind ==
             semantic_source_use::SourceAnchorKind::DeclarationName ||
         events[i].selected_decl_anchor_kind ==
             semantic_source_use::SourceAnchorKind::ApproximateDeclaration) &&
        location_is_for_input_path(events[i].selected_decl_location,
                                   normalized_input);
    if(has_structured_decl_anchor) {
      events[i].raw_selected_decl_location = events[i].selected_decl_location;
      continue;
    }
  }

  {
    vector<char> drop(events.size(), 0);
    map<string, vector<size_t> > grouped_function_calls;
    for(size_t i = 0; i < events.size(); ++i) {
      if(events[i].kind != WitnessEventKind::FunctionCall) {
        continue;
      }
      const string key = events[i].selected + "\x1f" + events[i].template_name +
          "\x1f" + events[i].selected_decl_location + "\x1f" +
          events[i].location;
      grouped_function_calls[key].push_back(i);
    }
    for(map<string, vector<size_t> >::iterator it = grouped_function_calls.begin();
        it != grouped_function_calls.end();
        ++it) {
      if(it->second.size() < 2) {
        continue;
      }
      std::sort(it->second.begin(), it->second.end(),
                [&](size_t lhs, size_t rhs)
                {
                  const ParsedLocation left = parse_line_col(events[lhs].location);
                  const ParsedLocation right = parse_line_col(events[rhs].location);
                  return std::make_pair(left.line, left.column) <
                      std::make_pair(right.line, right.column);
                });
      for(size_t j = 0; j + 1 < it->second.size(); ++j) {
        drop[it->second[j]] = 1;
      }
    }
    compact_events(events, drop);
  }

  {
    vector<char> drop(events.size(), 0);
    set<string> seen_variable_signatures;
    for(size_t i = 0; i < events.size(); ++i) {
      if(events[i].kind != WitnessEventKind::VariableUse) {
        continue;
      }
      const string key = events[i].location + "\x1f" + events[i].template_name +
          "\x1f" + witness_selection_text(events[i]) + "\x1f" +
          events[i].selected_decl_location +
          "\x1f" + binding_signature_key(events[i].bindings) + "\x1f" +
          binding_signature_key(events[i].specialization_bindings);
      if(!seen_variable_signatures.insert(key).second) {
        drop[i] = 1;
      }
    }
    compact_events(events, drop);
  }

  {
    vector<char> drop(events.size(), 0);
    map<string, vector<size_t> > grouped_variable_uses;
    for(size_t i = 0; i < events.size(); ++i) {
      if(events[i].kind != WitnessEventKind::VariableUse) {
        continue;
      }
      const ParsedLocation parsed = parse_line_col(events[i].location);
      const string key = std::to_string(parsed.line) + "\x1f" +
          event_signature_without_location(events[i]);
      grouped_variable_uses[key].push_back(i);
    }
    for(map<string, vector<size_t> >::const_iterator it = grouped_variable_uses.begin();
        it != grouped_variable_uses.end();
        ++it) {
      bool has_exact_anchor = false;
      for(size_t j = 0; j < it->second.size(); ++j) {
        if(events[it->second[j]].use_anchor_kind ==
           semantic_source_use::SourceAnchorKind::Spelling) {
          has_exact_anchor = true;
          break;
        }
      }
      if(!has_exact_anchor) {
        continue;
      }
      for(size_t j = 0; j < it->second.size(); ++j) {
        if(events[it->second[j]].use_anchor_kind !=
           semantic_source_use::SourceAnchorKind::Spelling) {
          drop[it->second[j]] = 1;
        }
      }
    }
    compact_events(events, drop);
  }

  {
    vector<char> drop(events.size(), 0);
    set<string> seen_signatures;
    for(size_t i = 0; i < events.size(); ++i) {
      if(events[i].kind == WitnessEventKind::ClassUse ||
         events[i].kind == WitnessEventKind::AliasUse) {
        continue;
      }
      const string key = visible_event_signature(events[i]);
      if(!seen_signatures.insert(key).second) {
        drop[i] = 1;
      }
    }
    compact_events(events, drop);
  }
}

void normalize_source_defined_template_calls(vector<WitnessEvent> & events,
                                             const vector<TemplateBodyRange> & template_body_lines,
                                             const string & input_path)
{
  const string normalized_input = normalize_witness_path(input_path);
  vector<char> drop(events.size(), 0);
  for(size_t i = 0; i < events.size(); ++i) {
    if(events[i].kind != WitnessEventKind::FunctionCall &&
       events[i].kind != WitnessEventKind::ClassUse &&
       events[i].kind != WitnessEventKind::AliasUse &&
       events[i].kind != WitnessEventKind::VariableUse) {
      continue;
    }
    const string location = !events[i].location.empty() ?
        events[i].location : events[i].raw_location;
    const ParsedLocation parsed = parse_line_col(location);
    set<string> body_parameter_names;
    if(parsed.line > 0 &&
       location_in_any_template_body_range(parsed.line,
                                           parsed.column,
                                           normalized_input,
                                           template_body_lines,
                                           &body_parameter_names)) {
      if(events[i].kind == WitnessEventKind::VariableUse) {
        continue;
      }
      if(events[i].kind == WitnessEventKind::ClassUse &&
         (event_replays_source_template_parameter_with_concrete_binding(
              events[i],
              body_parameter_names) ||
          event_replays_different_source_arguments_with_concrete_bindings(
              events[i])) &&
         !same_line_event_binding_mentions_bound_class_use(events, i)) {
        drop[i] = 1;
        continue;
      }
      if(events[i].kind == WitnessEventKind::ClassUse &&
         events[i].source_role == semantic_source_use::SourceUseRole::ValueUse) {
        drop[i] = 1;
        continue;
      }
      if(events[i].kind == WitnessEventKind::ClassUse &&
         events[i].ownership == SourceUseOwnership::NestedDerived) {
        drop[i] = 1;
        continue;
      }
      if(events[i].kind == WitnessEventKind::ClassUse &&
         events[i].selection == SourceSelectionKind::Primary &&
         events[i].specialization_bindings.empty() &&
         (events[i].template_id_occurrence.has_dependent_argument ||
          events[i].template_id_occurrence.has_current_specialization_argument) &&
         !event_source_occurrence_references_value(events[i]) &&
         !template_id_occurrence_is_result_type_use(
             events[i].template_id_occurrence) &&
         !same_line_event_binding_mentions_bound_class_use(events, i)) {
        drop[i] = 1;
        continue;
      }
      if(events[i].kind == WitnessEventKind::ClassUse) {
        bool all_bindings_are_explicit = true;
        for(size_t j = 0; j < events[i].bindings.size(); ++j) {
          if(events[i].bindings[j].source != "explicit") {
            all_bindings_are_explicit = false;
            break;
          }
        }
        if(!all_bindings_are_explicit) {
          drop[i] = 1;
          continue;
        }
      }
      if(events[i].kind == WitnessEventKind::ClassUse &&
         events[i].selection == SourceSelectionKind::Primary &&
         events[i].specialization_bindings.empty()) {
        const string::size_type member_split = events[i].template_name.rfind("::");
        const string owner =
            member_split == string::npos ? string() :
                                           events[i].template_name.substr(0, member_split);
        if(owner.find("<") == string::npos) {
          continue;
        }
        drop[i] = 1;
        continue;
      }
      if(events[i].kind == WitnessEventKind::ClassUse &&
         (class_event_has_concrete_template_body_source_occurrence(events[i]) ||
          class_event_has_selected_specialization_template_body_source_occurrence(
              events[i]))) {
        continue;
      }
      if(events[i].kind == WitnessEventKind::AliasUse &&
         alias_event_has_source_spelled_explicit_arguments(events[i])) {
        continue;
      }
      drop[i] = 1;
    }
  }
  bool changed = true;
  while(changed) {
    changed = false;
    for(size_t i = 0; i < events.size(); ++i) {
      if(!drop[i] || events[i].kind != WitnessEventKind::ClassUse) {
        continue;
      }
      const ParsedLocation parent_location = parse_line_col(events[i].location);
      if(parent_location.line <= 0) {
        continue;
      }
      for(size_t j = 0; j < events.size(); ++j) {
        if(drop[j] || events[j].kind != WitnessEventKind::ClassUse) {
          continue;
        }
        const ParsedLocation child_location = parse_line_col(events[j].location);
        if(child_location.line != parent_location.line) {
          continue;
        }
        const string child_entity = bound_class_use_entity_text(events[j]);
        if(child_entity.empty()) {
          continue;
        }
        bool parent_mentions_child = false;
        for(size_t k = 0; k < events[i].bindings.size(); ++k) {
          if(normalize_source_event_entity_text(events[i].bindings[k].arg) ==
             child_entity) {
            parent_mentions_child = true;
            break;
          }
        }
        if(!parent_mentions_child) {
          for(size_t k = 0; k < events[i].specialization_bindings.size(); ++k) {
            if(normalize_source_event_entity_text(
                   events[i].specialization_bindings[k].arg) == child_entity) {
              parent_mentions_child = true;
              break;
            }
          }
        }
        if(parent_mentions_child) {
          drop[j] = 1;
          changed = true;
        }
      }
    }
  }
#if defined(CPPGM_ENABLE_WITNESS_PROVENANCE)
  compact_events(events, drop);
#else
  vector<WitnessEvent> kept;
  kept.reserve(events.size());
  for(size_t i = 0; i < events.size(); ++i) {
    if(!drop[i]) {
      kept.push_back(events[i]);
    }
  }
  events.swap(kept);
#endif
}

std::string function_call_target_name(const WitnessEvent & event)
{
  return !event.selected.empty() ? event.selected : event.template_name;
}

bool binding_prefix_matches(const vector<WitnessBinding> & prefix,
                            const vector<WitnessBinding> & full)
{
  if(prefix.size() >= full.size()) {
    return false;
  }
  for(size_t i = 0; i < prefix.size(); ++i) {
    if(prefix[i].param != full[i].param ||
       prefix[i].source != full[i].source ||
       compact_binding_arg_key(prefix[i].arg) !=
           compact_binding_arg_key(full[i].arg)) {
      return false;
    }
  }
  for(size_t i = prefix.size(); i < full.size(); ++i) {
    if(full[i].source != "deduced") {
      return false;
    }
  }
  return true;
}

void drop_function_call_events_with_deduced_trailing_bindings(
    vector<WitnessEvent> & events)
{
  vector<char> drop(events.size(), 0);
  for(size_t i = 0; i < events.size(); ++i) {
    if(events[i].kind != WitnessEventKind::FunctionCall) {
      continue;
    }
    for(size_t j = 0; j < events.size(); ++j) {
      if(i == j ||
         drop[i] ||
         events[j].kind != WitnessEventKind::FunctionCall ||
         events[i].location != events[j].location ||
         function_call_target_name(events[i]) !=
             function_call_target_name(events[j]) ||
         events[i].selected_decl_location.empty() ||
         events[j].selected_decl_location.empty() ||
         events[i].selected_decl_location == events[j].selected_decl_location) {
        continue;
      }
      if(binding_prefix_matches(events[j].bindings, events[i].bindings)) {
        drop[i] = 1;
        break;
      }
    }
  }
  compact_events(events, drop);
}

void dedupe_visible_events(vector<WitnessEvent> & events)
{
  drop_function_call_events_with_deduced_trailing_bindings(events);

  {
    vector<char> drop(events.size(), 0);
    map<string, size_t> preferred;
    for(size_t i = 0; i < events.size(); ++i) {
      if(events[i].kind == WitnessEventKind::AliasUse ||
         events[i].kind == WitnessEventKind::ClassUse) {
        continue;
      }
      const string key =
          string(witness_event_kind_text(events[i].kind)) + "\x1f" +
          events[i].location + "\x1f" +
          events[i].template_name + "\x1f" +
          events[i].selected + "\x1f" +
          witness_selection_text(events[i]) + "\x1f" +
          events[i].selected_decl_location + "\x1f" +
          events[i].resolved + "\x1f" +
          events[i].expanded_to + "\x1f" +
          events[i].value + "\x1f" +
          binding_arg_signature_key(events[i].bindings) + "\x1f" +
          binding_arg_signature_key(events[i].specialization_bindings);
      map<string, size_t>::iterator found = preferred.find(key);
      if(found == preferred.end()) {
        preferred[key] = i;
        continue;
      }
      const size_t existing = found->second;
      if(binding_source_preference_score(events[i]) >
         binding_source_preference_score(events[existing])) {
        drop[existing] = 1;
        found->second = i;
      } else {
        drop[i] = 1;
      }
    }
    compact_events(events, drop);
  }

  {
    vector<char> drop(events.size(), 0);
    map<string, size_t> preferred;
    for(size_t i = 0; i < events.size(); ++i) {
      if(events[i].kind == WitnessEventKind::AliasUse ||
         events[i].kind == WitnessEventKind::ClassUse) {
        continue;
      }
      const string key = rendered_event_signature_without_binding_sources(events[i]);
      map<string, size_t>::iterator found = preferred.find(key);
      if(found == preferred.end()) {
        preferred[key] = i;
        continue;
      }
      const size_t existing = found->second;
      if(binding_source_preference_score(events[i]) >
         binding_source_preference_score(events[existing])) {
        drop[existing] = 1;
        found->second = i;
      } else {
        drop[i] = 1;
      }
    }
    compact_events(events, drop);
  }

  {
    vector<char> drop(events.size(), 0);
    map<string, size_t> preferred;
    for(size_t i = 0; i < events.size(); ++i) {
      if(events[i].kind == WitnessEventKind::AliasUse ||
         events[i].kind == WitnessEventKind::ClassUse) {
        continue;
      }
      const string key = visible_event_signature_without_binding_sources(events[i]);
      map<string, size_t>::iterator found = preferred.find(key);
      if(found == preferred.end()) {
        preferred[key] = i;
        continue;
      }
      const size_t existing = found->second;
      if(binding_source_preference_score(events[i]) >
         binding_source_preference_score(events[existing])) {
        drop[existing] = 1;
        found->second = i;
      } else {
        drop[i] = 1;
      }
    }
    compact_events(events, drop);
  }

  vector<char> drop(events.size(), 0);
  set<string> seen_signatures;
  for(size_t i = 0; i < events.size(); ++i) {
    if(events[i].kind == WitnessEventKind::AliasUse ||
       events[i].kind == WitnessEventKind::ClassUse) {
      continue;
    }
    const string key = visible_event_signature(events[i]);
    if(!seen_signatures.insert(key).second) {
      drop[i] = 1;
    }
  }
  compact_events(events, drop);
}

string source_location_compare_key(const string & location)
{
  const ParsedLocation parsed = parse_line_col(location);
  if(parsed.line <= 0 || parsed.column <= 0) {
    return normalize_witness_path(location);
  }
  return location_source_path(location) + ":" +
      std::to_string(parsed.line) + ":" + std::to_string(parsed.column);
}

set<string> variable_instantiation_owner_entities(
    const template_api::TemplateWitnessSession & session)
{
  set<string> owners;
  const auto insert_owner = [&owners](const string & entity)
  {
    const string owner = top_level_owner_entity_text(entity);
    if(owner.empty()) {
      return;
    }
    owners.insert(owner);
    owners.insert(strip_anonymous_namespace_qualifiers(owner));
  };
  for(size_t i = 0; i < session.lifecycle_events.size(); ++i) {
    const template_api::TemplateLifecycleEvent & event =
        session.lifecycle_events[i];
    if(event.kind != template_api::TemplateLifecycleEventKind::VariableInstantiation) {
      continue;
    }
    const string entity =
        normalize_source_event_entity_text(
            !event.normalized_entity.empty() ? event.normalized_entity :
                                               event.entity);
    insert_owner(entity);
    insert_owner(normalize_entity_name_for_event(entity));
  }
  return owners;
}

void drop_uninstantiated_static_member_definition_owner_uses(
    vector<WitnessEvent> & events,
    const template_api::TemplateWitnessSession & session)
{
  const set<string> variable_owners =
      variable_instantiation_owner_entities(session);
  vector<char> drop(events.size(), 0);
  for(size_t i = 0; i < events.size(); ++i) {
    if(events[i].kind != WitnessEventKind::ClassUse ||
       events[i].source_role !=
           semantic_source_use::SourceUseRole::StaticMemberDefinitionOwner) {
      continue;
    }
    const string owner = bound_class_use_entity_text(events[i]);
    if(variable_owners.count(owner) == 0 &&
       variable_owners.count(strip_anonymous_namespace_qualifiers(owner)) == 0) {
      drop[i] = 1;
    } else {
      events[i].source_role = semantic_source_use::SourceUseRole::QualifierUse;
    }
  }
  compact_events(events, drop);
}

bool text_has_anonymous_namespace(const string & text)
{
  return text.find("(anonymous namespace)") != string::npos ||
         text.find("_GLOBAL__N_") != string::npos;
}

void prefer_anonymous_namespace_class_use_names(vector<WitnessEvent> & events)
{
  map<string, string> anonymous_name_by_decl;
  for(size_t i = 0; i < events.size(); ++i) {
    if(events[i].kind != WitnessEventKind::ClassUse ||
       events[i].selected_decl_location.empty() ||
       !text_has_anonymous_namespace(events[i].template_name)) {
      continue;
    }
    const string key =
        events[i].selected_decl_location + "\x1f" +
        unqualified_template_name_text(events[i].template_name) + "\x1f" +
        witness_selection_text(events[i]) + "\x1f" +
        normalized_binding_signature_key(events[i].bindings) + "\x1f" +
        normalized_binding_signature_key(events[i].specialization_bindings);
    string & preferred = anonymous_name_by_decl[key];
    if(preferred.empty() || events[i].template_name.size() > preferred.size()) {
      preferred = events[i].template_name;
    }
  }
  if(anonymous_name_by_decl.empty()) {
    return;
  }
  for(size_t i = 0; i < events.size(); ++i) {
    if(events[i].kind != WitnessEventKind::ClassUse ||
       events[i].selected_decl_location.empty()) {
      continue;
    }
    const string key =
        events[i].selected_decl_location + "\x1f" +
        unqualified_template_name_text(events[i].template_name) + "\x1f" +
        witness_selection_text(events[i]) + "\x1f" +
        normalized_binding_signature_key(events[i].bindings) + "\x1f" +
        normalized_binding_signature_key(events[i].specialization_bindings);
    map<string, string>::const_iterator found = anonymous_name_by_decl.find(key);
    if(found == anonymous_name_by_decl.end()) {
      continue;
    }
    events[i].template_name = found->second;
    if(!events[i].selected.empty()) {
      events[i].selected = found->second;
    }
  }
}

string render_events_text(const vector<WitnessEvent> & events,
                          bool debug)
{
  vector<WitnessEvent> ordered = events;
  for(size_t i = 0; i < ordered.size(); ++i) {
    ordered[i].location = source_location_compare_key(ordered[i].location);
    ordered[i].selected_decl_location =
        source_location_compare_key(ordered[i].selected_decl_location);
    ordered[i].guide_decl_location =
        source_location_compare_key(ordered[i].guide_decl_location);
    for(size_t j = 0; j < ordered[i].drops.size(); ++j) {
      ordered[i].drops[j].location =
          source_location_compare_key(ordered[i].drops[j].location);
    }
  }
  sort_events(ordered);
  std::ostringstream out;
  out << "translation-unit\n";
  for(size_t i = 0; i < ordered.size(); ++i) {
    const WitnessEvent & event = ordered[i];
    out << "  " << header_from_kind(event.kind) << " at " << event.location << "\n";
    if(event.kind == WitnessEventKind::FunctionCall) {
      out << "    callee "
          << (!event.selected.empty() ? event.selected : event.template_name)
          << "\n";
    } else {
      out << "    template " << event.template_name << "\n";
    }
    if(!event.resolved.empty()) {
      out << "    resolved " << event.resolved << "\n";
    }
    if(event.selection != SourceSelectionKind::None) {
      out << "    selected " << witness_selection_text(event) << "\n";
    }
    if(debug && !event.selected_decl_location.empty()) {
      out << "    decl " << event.selected_decl_location << "\n";
    }
    if(debug && event.candidates_built >= 0) {
      out << "    candidates_built " << event.candidates_built << "\n";
    }
    if(debug && event.candidates_viable >= 0) {
      out << "    candidates_viable " << event.candidates_viable << "\n";
    }
    for(size_t j = 0; j < event.bindings.size(); ++j) {
      out << "    bind " << (debug ? event.bindings[j].param :
                             ("#" + std::to_string(j + 1))) << " = "
          << event.bindings[j].arg << " source=" << event.bindings[j].source
          << "\n";
    }
    for(size_t j = 0; j < event.specialization_bindings.size(); ++j) {
      out << "    specialize "
          << (debug ? event.specialization_bindings[j].param :
              ("#" + std::to_string(j + 1))) << " = "
          << event.specialization_bindings[j].arg << " source="
          << event.specialization_bindings[j].source << "\n";
    }
    if(!event.value.empty()) {
      out << "    value " << event.value << "\n";
    }
    if(!event.guide.empty()) {
      out << "    guide " << event.guide << "\n";
    }
    if(debug && !event.guide_decl_location.empty()) {
      out << "    guide_decl " << event.guide_decl_location << "\n";
    }
    for(size_t j = 0; j < event.drops.size(); ++j) {
      out << "    drop " << event.drops[j].candidate;
      if(debug) {
        out << " at " << event.drops[j].location;
      }
      out << " reason=" << event.drops[j].reason << "\n";
    }
  }
  return out.str();
}

string sort_rendered_source_event_blocks(const string & text)
{
  const string prefix = "translation-unit\n";
  if(text.compare(0, prefix.size(), prefix) != 0) {
    return text;
  }

  vector<string> blocks;
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
      [](const string & block)
      {
        const string::size_type at = block.find(" at ");
        const string::size_type nl = block.find('\n');
        const string location =
            at == string::npos ? string() :
            block.substr(at + 4,
                         (nl == string::npos ? block.size() : nl) - (at + 4));
        const ParsedLocation parsed = parse_line_col(location);
        const string::size_type split = location.rfind(':');
        const string::size_type path_split =
            split == string::npos ? string::npos : location.rfind(':', split - 1);
        const string path =
            path_split == string::npos ? location : location.substr(0, path_split);
        return std::make_tuple(path, parsed.line, parsed.column);
      };
  std::stable_sort(blocks.begin(),
                   blocks.end(),
                   [&](const string & lhs, const string & rhs)
                   {
                     return block_key(lhs) < block_key(rhs);
                   });
  {
    vector<string> deduped;
    set<string> seen_blocks;
    for(size_t i = 0; i < blocks.size(); ++i) {
      if(!seen_blocks.insert(blocks[i]).second) {
        continue;
      }
      deduped.push_back(blocks[i]);
    }
    blocks.swap(deduped);
  }

  std::ostringstream out;
  out << prefix;
  for(size_t i = 0; i < blocks.size(); ++i) {
    out << blocks[i];
  }
  return out.str();
}

#if defined(CPPGM_ENABLE_WITNESS_PROVENANCE)

template <typename Function>
void run_renderer_pass(
    const char * pass,
    const template_api::TemplateWitnessSession & session,
    const string & source_path,
    vector<WitnessEvent> & events,
    vector<witness_provenance::RendererEventLineage> & lineages,
    const Function & function)
{
  if(lineages.size() != events.size()) {
    function();
    return;
  }
  const vector<WitnessEvent> before_events = events;
  const vector<witness_provenance::RendererEventLineage> before_lineages =
      lineages;
  RendererTraceContext trace;
  trace.session = &session;
  trace.source_path = &source_path;
  trace.pass = pass;
  trace.lineages = &lineages;
  RendererTraceContext * previous = current_renderer_trace();
  current_renderer_trace() = &trace;
  function();
  current_renderer_trace() = previous;

  for(size_t i = 0; i < events.size() && i < lineages.size(); ++i) {
    for(size_t j = 0; j < before_events.size() && j < before_lineages.size(); ++j) {
      if(lineages[i].event_id != before_lineages[j].event_id) continue;
      if(!witness_events_equal(before_events[j], events[i])) {
        const string fields = renderer_changed_fields(before_events[j], events[i]);
        const bool replaced =
            fields.find("kind") != string::npos ||
            fields.find("location") != string::npos ||
            fields.find("entity") != string::npos ||
            fields.find("selection") != string::npos;
        witness_provenance::note_renderer_action(
            session,
            source_path,
            pass,
            replaced ? "replaced" : "rewritten",
            lineages[i],
            witness_event_kind_text(events[i].kind),
            events[i].location,
            !events[i].template_name.empty() ?
                events[i].template_name : events[i].selected,
            fields);
      }
      break;
    }
  }
}

#endif

void collect_rendered_source_events(const template_api::TemplateWitnessSession & session,
                                    const string & source_path,
                                    vector<WitnessEvent> & events
#if defined(CPPGM_ENABLE_WITNESS_PROVENANCE)
                                    ,
                                    bool trace_renderer = false)
#else
                                    )
#endif
{
#if defined(CPPGM_ENABLE_WITNESS_PROVENANCE)
  vector<witness_provenance::RendererEventLineage> lineages;
  if(trace_renderer) {
    lineages = witness_provenance::renderer_table_lineages(session);
  }
  const bool trace_active =
      trace_renderer && witness_provenance::enabled() &&
      lineages.size() == session.source_use_table.uses.size();
  WitnessBuilder source_builder(source_path, &session, trace_active);
  for(size_t i = 0; i < session.source_use_table.uses.size(); ++i) {
    source_builder.consume_direct_event(
        witness_event_from_source_use(session.source_use_table.uses[i]),
        trace_active ? &lineages[i] : nullptr);
  }
  vector<witness_provenance::RendererEventLineage> built_lineages;
  events = source_builder.finish(trace_active ? &built_lineages : nullptr);
  lineages.swap(built_lineages);

#define CPPGM_RENDER_PASS(name, expression) \
  run_renderer_pass(name, session, source_path, events, lineages, [&]() { expression; })
  CPPGM_RENDER_PASS("canonicalize_locations_and_dedupe",
                    canonicalize_event_locations_and_dedupe(events, source_path));
  CPPGM_RENDER_PASS("normalize_names",
                    normalize_event_names(events, session.inline_namespace_names));
  CPPGM_RENDER_PASS("normalize_bindings",
                    normalize_event_bindings(events, source_path));
  CPPGM_RENDER_PASS("prefer_anonymous_namespace_class_names",
                    prefer_anonymous_namespace_class_use_names(events));
  CPPGM_RENDER_PASS("drop_uninstantiated_static_member_owners",
                    drop_uninstantiated_static_member_definition_owner_uses(
                        events, session));
  CPPGM_RENDER_PASS("normalize_drop_order", normalize_drop_order(events));
  CPPGM_RENDER_PASS("normalize_source_defined_calls",
                    normalize_source_defined_template_calls(
                        events, session.template_body_ranges, source_path));
  CPPGM_RENDER_PASS("drop_template_header_patterns",
                    drop_template_header_pattern_events(
                        events, session.template_header_contexts, source_path));
  CPPGM_RENDER_PASS("drop_redundant_nested_events",
                    drop_redundant_nested_events(events));
  CPPGM_RENDER_PASS("dedupe_visible_events", dedupe_visible_events(events));
  CPPGM_RENDER_PASS("sort_visible_events", sort_events(events));
#undef CPPGM_RENDER_PASS

  if(trace_active && lineages.size() == events.size()) {
    for(size_t i = 0; i < events.size(); ++i) {
      witness_provenance::note_renderer_final_visible(
          session,
          source_path,
          lineages[i],
          witness_event_kind_text(events[i].kind),
          events[i].location,
          !events[i].template_name.empty() ?
              events[i].template_name : events[i].selected);
    }
  }
#else
  WitnessBuilder source_builder(source_path);
  for(size_t i = 0; i < session.source_use_table.uses.size(); ++i) {
    source_builder.consume_direct_event(
        witness_event_from_source_use(session.source_use_table.uses[i]));
  }
  events = source_builder.finish();
  canonicalize_event_locations_and_dedupe(events, source_path);
  normalize_event_names(events, session.inline_namespace_names);
  normalize_event_bindings(events, source_path);
  prefer_anonymous_namespace_class_use_names(events);
  drop_uninstantiated_static_member_definition_owner_uses(events, session);
  normalize_drop_order(events);
  normalize_source_defined_template_calls(events,
                                          session.template_body_ranges,
                                          source_path);
  drop_template_header_pattern_events(events,
                                      session.template_header_contexts,
                                      source_path);
  drop_redundant_nested_events(events);
  dedupe_visible_events(events);
  sort_events(events);
#endif
}

void record_explicit_source_owner_entity(set<string> & out,
                                         const string & owner_text)
{
  const string owner =
      template_api::template_witness_detail::normalize_template_log_entity(
          owner_text);
  if(owner.empty()) {
    return;
  }
  out.insert(owner);
  const string stripped = strip_top_level_template_suffix(owner);
  if(!stripped.empty()) {
    out.insert(stripped);
  }
}

}  // namespace

namespace template_api {

std::string render_template_source_witness_text(
    const TemplateWitnessSession & session,
    const std::string & source_path)
{
  vector<WitnessEvent> events;
#if defined(CPPGM_ENABLE_WITNESS_PROVENANCE)
  collect_rendered_source_events(session, source_path, events, true);
  const string rendered =
      sort_rendered_source_event_blocks(render_events_text(events, false));
  witness_provenance::finish_session(session, source_path);
  return rendered;
#else
  collect_rendered_source_events(session, source_path, events);
  return sort_rendered_source_event_blocks(render_events_text(events, false));
#endif
}

std::string render_template_source_witness_debug_text(
    const TemplateWitnessSession & session,
    const std::string & source_path)
{
  vector<WitnessEvent> events;
#if defined(CPPGM_ENABLE_WITNESS_PROVENANCE)
  collect_rendered_source_events(session, source_path, events, true);
  const string rendered =
      sort_rendered_source_event_blocks(render_events_text(events, true));
  witness_provenance::finish_session(session, source_path);
  return rendered;
#else
  collect_rendered_source_events(session, source_path, events);
  return sort_rendered_source_event_blocks(render_events_text(events, true));
#endif
}

std::map<std::string, std::string> template_source_defaulted_aliases(
    const TemplateWitnessSession & session,
    const std::string & source_path)
{
  vector<WitnessEvent> events;
  collect_rendered_source_events(session, source_path, events);
  map<string, string> aliases = build_defaulted_class_aliases(events, false);
  const map<string, string> predecl_all_defaulted =
      build_predecl_all_defaulted_class_aliases(events);
  aliases.insert(predecl_all_defaulted.begin(), predecl_all_defaulted.end());
  const map<string, string> omitted_all_defaulted =
      build_omitted_all_defaulted_class_aliases(events);
  aliases.insert(omitted_all_defaulted.begin(), omitted_all_defaulted.end());
  const map<string, string> function_pointer_aliases =
      build_function_pointer_class_aliases(events);
  aliases.insert(function_pointer_aliases.begin(), function_pointer_aliases.end());
  return aliases;
}

std::set<std::string> template_source_owner_entities(
    const TemplateWitnessSession & session,
    const std::string & source_path)
{
  vector<WitnessEvent> events;
  collect_rendered_source_events(session, source_path, events);
  set<string> out;
  for(size_t i = 0; i < events.size(); ++i) {
    if(events[i].kind == WitnessEventKind::FunctionCall) {
      const string owner =
          !events[i].selected.empty() ? events[i].selected : events[i].template_name;
      if(!owner.empty()) {
        out.insert(
            template_api::template_witness_detail::normalize_template_log_entity(
                owner));
      }
      continue;
    }
    if(events[i].kind != WitnessEventKind::ClassUse &&
       events[i].kind != WitnessEventKind::AliasUse &&
       events[i].kind != WitnessEventKind::VariableUse) {
      continue;
    }
    if(events[i].template_name.empty()) {
      continue;
    }
    if(events[i].bindings.empty()) {
      out.insert(template_api::template_witness_detail::normalize_template_log_entity(
          events[i].template_name));
      continue;
    }
    const string full_owner =
        make_bound_template_text(events[i].template_name,
                                 events[i].bindings,
                                 events[i].bindings.size());
    out.insert(template_api::template_witness_detail::normalize_template_log_entity(
        full_owner));
    size_t visible_count = events[i].bindings.size();
    while(visible_count > 0 &&
          events[i].bindings[visible_count - 1].source == "defaulted") {
      --visible_count;
    }
    if(visible_count != events[i].bindings.size()) {
      out.insert(template_api::template_witness_detail::normalize_template_log_entity(
          make_bound_template_text(events[i].template_name,
                                   events[i].bindings,
                                   visible_count)));
    }
  }
  return out;
}

std::set<std::string> template_source_explicit_owner_entities(
    const TemplateWitnessSession & session,
    const std::string & source_path)
{
  vector<WitnessEvent> events;
  collect_rendered_source_events(session, source_path, events);
  set<string> out;
  for(size_t i = 0; i < events.size(); ++i) {
    if(events[i].selection != SourceSelectionKind::ExplicitSpecialization) {
      continue;
    }
    if(events[i].kind == WitnessEventKind::FunctionCall) {
      record_explicit_source_owner_entity(
          out,
          !events[i].selected.empty() ? events[i].selected : events[i].template_name);
      continue;
    }
    if(events[i].kind == WitnessEventKind::ClassUse) {
      // A source class-use owns the class selection witness, not member
      // function materialization triggered by using that class.
      continue;
    }
    if(events[i].kind != WitnessEventKind::AliasUse &&
       events[i].kind != WitnessEventKind::VariableUse) {
      continue;
    }
    if(events[i].template_name.empty()) {
      continue;
    }
    if(events[i].bindings.empty()) {
      record_explicit_source_owner_entity(out, events[i].template_name);
      continue;
    }
    record_explicit_source_owner_entity(
        out,
        make_bound_template_text(events[i].template_name,
                                 events[i].bindings,
                                 events[i].bindings.size()));
  }
  return out;
}

std::set<std::string> template_source_argument_value_entities(
    const TemplateWitnessSession & session,
    const std::string & source_path)
{
  vector<WitnessEvent> events;
  collect_rendered_source_events(session, source_path, events);
  set<string> out;
  for(size_t i = 0; i < events.size(); ++i) {
    if(events[i].kind != WitnessEventKind::ClassUse &&
       events[i].kind != WitnessEventKind::AliasUse) {
      continue;
    }
    const semantic_source_use::SourceTemplateIdOccurrence & occurrence =
        events[i].template_id_occurrence;
    if(!occurrence.present) {
      continue;
    }
    for(size_t j = 0; j < occurrence.arguments.size(); ++j) {
      const vector<string> & entities =
          occurrence.arguments[j].referenced_value_entities;
      for(size_t k = 0; k < entities.size(); ++k) {
        const string normalized =
            template_witness_detail::normalize_template_log_entity(entities[k]);
        if(!normalized.empty()) {
          out.insert(normalized);
        }
      }
    }
  }
  return out;
}

std::set<std::string> template_source_argument_value_decl_locations(
    const TemplateWitnessSession & session,
    const std::string & source_path)
{
  vector<WitnessEvent> events;
  collect_rendered_source_events(session, source_path, events);
  set<string> out;
  for(size_t i = 0; i < events.size(); ++i) {
    if(events[i].kind != WitnessEventKind::ClassUse &&
       events[i].kind != WitnessEventKind::AliasUse) {
      continue;
    }
    const semantic_source_use::SourceTemplateIdOccurrence & occurrence =
        events[i].template_id_occurrence;
    if(!occurrence.present) {
      continue;
    }
    for(size_t j = 0; j < occurrence.arguments.size(); ++j) {
      const vector<string> & locations =
          occurrence.arguments[j].referenced_value_decl_locations;
      for(size_t k = 0; k < locations.size(); ++k) {
        const string normalized =
            normalize_template_witness_source_location(locations[k]);
        if(!normalized.empty()) {
          out.insert(normalized);
        }
      }
    }
  }
  return out;
}

}  // namespace template_api
