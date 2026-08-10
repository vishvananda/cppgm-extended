#include "callsemantic/source_location_utils.h"

#include "callsemantic_internal.h"
#include "semantic_utils.h"
#include "template_api.h"

#include <cctype>
#include <cstdlib>

namespace callsemantic {

using semantic_utils::strip_trailing_top_level_template_arguments;
using callsemantic_internal::is_identifier_text;
using namespace std;

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

ParsedSourceLocation parse_physical_source_location(const std::string & text)
{
  ParsedSourceLocation parsed = parse_source_location(text);
  const std::string prefix = " at ";
  if(parsed.file.compare(0, prefix.size(), prefix) == 0) {
    parsed.file = parsed.file.substr(prefix.size());
  }
  return parsed;
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

bool source_location_is_later(const std::string & first,
                              const std::string & second)
{
  const ParsedSourceLocation parsed_first = parse_source_location(first);
  const ParsedSourceLocation parsed_second = parse_source_location(second);
  if(!parsed_first.valid || !parsed_second.valid ||
     parsed_first.file != parsed_second.file) {
    return false;
  }
  if(parsed_second.line > parsed_first.line) {
    return true;
  }
  return parsed_second.line == parsed_first.line &&
         parsed_second.column > parsed_first.column;
}

std::string prefer_earlier_source_location(const std::string & first,
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
  if(parsed_second.line < parsed_first.line) {
    return second;
  }
  if(parsed_second.line == parsed_first.line &&
     parsed_second.column < parsed_first.column) {
    return second;
  }
  return first;
}

SourceLocationTokenView::SourceLocationTokenView(
    const SourceLocationTable * source_locations,
    IRecogTokenSequence * token_sequence)
  : source_locations_(source_locations),
    token_sequence_(token_sequence)
{
}

string SourceLocationTokenView::source_location_for_node(const CppAstNode & node) const
{
  if(source_locations_ == nullptr) {
    return string();
  }
  if(node.source_location_id != 0) {
    const string location = source_locations_->describe(node.source_location_id);
    if(!location.empty() && location != "<unknown>") {
      return string(" at ") + location;
    }
  }
  if(token_sequence_ == nullptr) {
    return string();
  }

  const RecogToken & token = token_sequence_->peek(node.token_start);
  if(token.location_id == 0) {
    return string();
  }

  const string location = source_locations_->describe(token.location_id);
  if(location.empty() || location == "<unknown>") {
    return string();
  }

  return string(" at ") + location;
}

string SourceLocationTokenView::source_location_for_node_syntax_start(
    const CppAstNode & node) const
{
  if(token_sequence_ != nullptr && node.token_end > node.token_start) {
    const string location = source_location_for_token_index(node.token_start);
    if(!location.empty()) {
      return location;
    }
  }
  return source_location_for_node(node);
}

string SourceLocationTokenView::source_location_for_identifier_before_node_syntax(
    const CppAstNode & node) const
{
  if(token_sequence_ != nullptr && node.token_start > 0) {
    const RecogToken & previous = token_sequence_->peek(node.token_start - 1);
    if(previous.is_identifier()) {
      const string location = source_location_for_token_index(node.token_start - 1);
      if(!location.empty()) {
        return location;
      }
    }
  }
  return string();
}

string SourceLocationTokenView::source_location_for_node_token(
    const CppAstNode & node) const
{
  if(token_sequence_ != nullptr && node.has_token &&
     node.token_end > node.token_start) {
    size_t begin = node.token_start;
    size_t end = node.token_end;
    if(node.children.size() == 2 &&
       node.children[0].token_end <= node.children[1].token_start) {
      begin = node.children[0].token_end;
      end = node.children[1].token_start;
    }
    for(size_t i = begin; i < end; ++i) {
      const RecogToken & token = token_sequence_->peek(i);
      if(token.kind != node.token_kind ||
         (node.token_kind == RT_SIMPLE && token.simple_type != node.simple_type)) {
        continue;
      }
      const string location = source_location_for_token_index(i);
      if(!location.empty()) {
        return location;
      }
    }
  }
  return source_location_for_node(node);
}

string SourceLocationTokenView::source_location_for_name_in_node(const CppAstNode & node,
                                                const string & name,
                                                bool prefer_last) const
{
  if(source_locations_ == nullptr || name.empty() ||
     node.token_end <= node.token_start) {
    return string();
  }

  const string search_name =
      name.compare(0, 8, "operator") == 0 ? string("operator") : name;
  if(node.source_location_id != 0 &&
     (node.value == search_name || node.value == name)) {
    return source_location_for_node(node);
  }
  if(token_sequence_ == nullptr) {
    return string();
  }
  size_t selected = static_cast<size_t>(-1);
  for(size_t i = node.token_start; i < node.token_end; ++i) {
    const RecogToken & token = token_sequence_->peek(i);
    if(token.source != search_name) {
      continue;
    }
    selected = i;
    if(!prefer_last) {
      break;
    }
  }

  if(selected == static_cast<size_t>(-1)) {
    return string();
  }

  return source_location_for_token_index(selected);
}

string SourceLocationTokenView::source_location_for_name_in_subtree(const CppAstNode & node,
                                           const string & name,
                                           bool prefer_last) const
{
  std::string location =
      source_location_for_name_in_node(node, name, prefer_last);
  if(!location.empty()) {
    return location;
  }

  if(!prefer_last) {
    for(std::size_t i = 0; i < node.children.size(); ++i) {
      location = source_location_for_name_in_subtree(
          node.children[i], name, prefer_last);
      if(!location.empty()) {
        return location;
      }
    }
    return std::string();
  }

  for(std::size_t i = node.children.size(); i > 0; --i) {
    location = source_location_for_name_in_subtree(
        node.children[i - 1], name, prefer_last);
    if(!location.empty()) {
      return location;
    }
  }
  return std::string();
}

bool SourceLocationTokenView::source_location_identifier_followed_by_indirection_type_suffix(
    const std::string & location,
    const std::string & identifier) const
{
  if(identifier.empty() || !(token_sequence_ && source_locations_)) {
    return false;
  }
  const ParsedSourceLocation target = parse_source_location(
      template_api::normalize_template_witness_source_location(location));
  if(!target.valid) {
    return false;
  }
  const std::size_t token_count = token_sequence_->size();
  for(std::size_t i = 0; i < token_count; ++i) {
    const RecogToken & token = token_sequence_->peek(i);
    if(token.is_eof()) {
      break;
    }
    if(token.location_id == 0 || !token.is_identifier() ||
       token.source != identifier) {
      continue;
    }
    const ParsedSourceLocation parsed = parse_source_location(
        template_api::normalize_template_witness_source_location(
            source_locations_->describe(token.location_id)));
    if(!(parsed.valid &&
         parsed.file == target.file &&
         parsed.line == target.line &&
         parsed.column == target.column)) {
      continue;
    }
    std::size_t next_index = i + 1;
    while(next_index < token_count) {
      const RecogToken & next = token_sequence_->peek(next_index);
      if(next.is_eof()) {
        return false;
      }
      if(next.source == "const" || next.source == "volatile") {
        ++next_index;
        continue;
      }
      return next.source == "*" ||
          next.source == "&" ||
          next.source == "&&";
    }
    return false;
  }
  return false;
}

std::string SourceLocationTokenView::source_location_for_qualified_member_start_on_line(
    const std::string & location,
    const std::string & member_name) const
{
  if(token_sequence_ == nullptr) {
    return std::string();
  }
  return source_location_for_qualified_member_start_on_line_in_range(
      location, member_name, 0, token_sequence_->size());
}

std::string
SourceLocationTokenView::source_location_for_qualified_member_start_on_line_in_range(
    const std::string & location,
    const std::string & member_name,
    std::size_t begin,
    std::size_t end) const
{
  if(member_name.empty() || !(token_sequence_ && source_locations_)) {
    return std::string();
  }
  const ParsedSourceLocation base = parse_source_location(
      template_api::normalize_template_witness_source_location(location));
  if(!base.valid) {
    return std::string();
  }
  const std::size_t token_count = token_sequence_->size();
  if(begin > token_count) {
    begin = token_count;
  }
  if(end > token_count) {
    end = token_count;
  }
  if(end <= begin) {
    return std::string();
  }
  for(std::size_t i = begin; i + 1 < end; ++i) {
    const RecogToken & scope_token = token_sequence_->peek(i);
    if(scope_token.is_eof()) {
      break;
    }
    if(scope_token.location_id == 0 || scope_token.source != "::") {
      continue;
    }
    const RecogToken & member_token = token_sequence_->peek(i + 1);
    if(member_token.is_eof() ||
       member_token.location_id == 0 ||
       member_token.source != member_name) {
      continue;
    }
    const ParsedSourceLocation scope_location = parse_source_location(
        template_api::normalize_template_witness_source_location(
            source_locations_->describe(scope_token.location_id)));
    const ParsedSourceLocation member_location = parse_source_location(
        template_api::normalize_template_witness_source_location(
            source_locations_->describe(member_token.location_id)));
    if(!(scope_location.valid &&
         member_location.valid &&
         scope_location.file == base.file &&
         member_location.file == base.file &&
         scope_location.line == base.line &&
         member_location.line == base.line &&
         member_location.column >= base.column &&
         member_location.column == scope_location.column + 2)) {
      continue;
    }

    std::size_t start = i;
    int angle_depth = 0;
    while(start > begin) {
      const RecogToken & prev = token_sequence_->peek(start - 1);
      if(prev.is_eof() || prev.location_id == 0) {
        break;
      }
      const ParsedSourceLocation prev_location = parse_source_location(
          template_api::normalize_template_witness_source_location(
              source_locations_->describe(prev.location_id)));
      if(!(prev_location.valid &&
           prev_location.file == base.file &&
           prev_location.line == base.line)) {
        break;
      }
      if(angle_depth == 0) {
        if(prev.source == ">") {
          ++angle_depth;
          --start;
          continue;
        }
        if(prev.is_identifier() || prev.source == "::" ||
           prev.source == "template") {
          --start;
          continue;
        }
        break;
      }
      if(prev.source == ">") {
        ++angle_depth;
      } else if(prev.source == "<") {
        --angle_depth;
      }
      --start;
    }
    return template_api::normalize_template_witness_source_location(
        source_location_for_token_index(start));
  }
  return std::string();
}

string SourceLocationTokenView::source_location_for_token_index(std::size_t index) const
{
  if(source_locations_ == nullptr || token_sequence_ == nullptr) {
    return string();
  }

  const RecogToken & token = token_sequence_->peek(index);
  if(token.location_id == 0) {
    return string();
  }
  const string location = source_locations_->describe(token.location_id);
  if(location.empty() || location == "<unknown>") {
    return string();
  }
  return string(" at ") + location;
}

bool SourceLocationTokenView::parsed_source_location_for_token_index(std::size_t index,
                                            ParsedSourceLocation & out) const
{
  if(source_locations_ == nullptr || token_sequence_ == nullptr) {
    return false;
  }
  const RecogToken & token = token_sequence_->peek(index);
  if(token.is_eof() || token.location_id == 0) {
    return false;
  }
  out = parse_physical_source_location(source_locations_->describe(token.location_id));
  return out.valid;
}

bool SourceLocationTokenView::token_index_for_source_location(const std::string & location,
                                     const std::string & token_source,
                                     std::size_t & out_index) const
{
  if(token_source.empty() || token_sequence_ == nullptr) {
    return false;
  }
  const ParsedSourceLocation target = parse_physical_source_location(location);
  if(!target.valid) {
    return false;
  }
  const std::size_t token_count = token_sequence_->size();
  for(std::size_t i = 0; i < token_count; ++i) {
    const RecogToken & token = token_sequence_->peek(i);
    if(token.is_eof()) {
      break;
    }
    if(token.source != token_source) {
      continue;
    }
    ParsedSourceLocation parsed;
    if(parsed_source_location_for_token_index(i, parsed) &&
       parsed.file == target.file &&
       parsed.line == target.line &&
       parsed.column == target.column) {
      out_index = i;
      return true;
    }
  }
  return false;
}

bool SourceLocationTokenView::token_index_at_source_location(const std::string & location,
                                    std::size_t & out_index) const
{
  if(token_sequence_ == nullptr) {
    return false;
  }
  const ParsedSourceLocation target = parse_physical_source_location(location);
  if(!target.valid) {
    return false;
  }
  const std::size_t token_count = token_sequence_->size();
  for(std::size_t i = 0; i < token_count; ++i) {
    const RecogToken & token = token_sequence_->peek(i);
    if(token.is_eof()) {
      break;
    }
    ParsedSourceLocation parsed;
    if(parsed_source_location_for_token_index(i, parsed) &&
       parsed.file == target.file &&
       parsed.line == target.line &&
       parsed.column == target.column) {
      out_index = i;
      return true;
    }
  }
  return false;
}

bool SourceLocationTokenView::find_next_token_source_on_same_line(std::size_t start_index,
                                         const std::string & token_source,
                                         std::size_t & out_index) const
{
  ParsedSourceLocation start_location;
  if(token_source.empty() ||
     !parsed_source_location_for_token_index(start_index, start_location)) {
    return false;
  }
  const std::size_t token_count = token_sequence_->size();
  for(std::size_t i = start_index + 1; i < token_count; ++i) {
    const RecogToken & token = token_sequence_->peek(i);
    if(token.is_eof()) {
      break;
    }
    ParsedSourceLocation parsed;
    if(!parsed_source_location_for_token_index(i, parsed) ||
       parsed.file != start_location.file) {
      continue;
    }
    if(parsed.line > start_location.line) {
      break;
    }
    if(parsed.line == start_location.line && token.source == token_source) {
      out_index = i;
      return true;
    }
  }
  return false;
}

bool SourceLocationTokenView::token_source_is_name_part(const std::string & source)
{
  return source == "::" || is_identifier_text(source);
}

bool SourceLocationTokenView::tokens_are_adjacent(std::size_t left_index,
                         std::size_t right_index) const
{
  ParsedSourceLocation left;
  ParsedSourceLocation right;
  if(!parsed_source_location_for_token_index(left_index, left) ||
     !parsed_source_location_for_token_index(right_index, right) ||
     left.file != right.file ||
     left.line != right.line) {
    return false;
  }
  const std::string & left_source = token_sequence_->peek(left_index).source;
  return right.column ==
      left.column + static_cast<int>(left_source.size());
}

bool SourceLocationTokenView::token_text_needs_separator(const std::string & left,
                                       const std::string & right)
{
  if(left.empty() || right.empty()) {
    return false;
  }
  const char left_ch = left[left.size() - 1];
  const char right_ch = right[0];
  return (std::isalnum(static_cast<unsigned char>(left_ch)) ||
          left_ch == '_') &&
         (std::isalnum(static_cast<unsigned char>(right_ch)) ||
          right_ch == '_');
}

std::string SourceLocationTokenView::compact_token_range_text(std::size_t begin,
                                     std::size_t end) const
{
  std::string out;
  std::string previous;
  for(std::size_t i = begin; i < end; ++i) {
    const RecogToken & token = token_sequence_->peek(i);
    if(token.is_eof()) {
      break;
    }
    if(token_text_needs_separator(previous, token.source)) {
      out.push_back(' ');
    }
    out += token.source;
    previous = token.source;
  }
  return out;
}

std::string SourceLocationTokenView::compact_template_id_token_text(std::size_t begin,
                                           std::size_t close_index,
                                           int close_chars) const
{
  std::string out = compact_token_range_text(begin, close_index);
  const RecogToken & close_token = token_sequence_->peek(close_index);
  std::string close_text = close_token.source;
  if(close_text == ">>" && close_chars == 1) {
    close_text = ">";
  }
  if(token_text_needs_separator(
         close_index > begin ? token_sequence_->peek(close_index - 1).source :
                               std::string(),
         close_text)) {
    out.push_back(' ');
  }
  out += close_text;
  return out;
}

bool SourceLocationTokenView::template_name_before_open(std::size_t open_index,
                               std::size_t & name_begin,
                               std::size_t & anchor_index) const
{
  if(open_index == 0) {
    return false;
  }
  std::size_t begin = open_index - 1;
  if(!is_identifier_text(token_sequence_->peek(begin).source)) {
    return false;
  }
  anchor_index = begin;
  while(begin > 0) {
    const std::size_t previous = begin - 1;
    if(!token_source_is_name_part(token_sequence_->peek(previous).source) ||
       !tokens_are_adjacent(previous, begin)) {
      break;
    }
    begin = previous;
    if(is_identifier_text(token_sequence_->peek(begin).source)) {
      anchor_index = begin;
    }
  }
  if(!is_identifier_text(token_sequence_->peek(begin).source)) {
    return false;
  }
  for(std::size_t i = begin; i < open_index; ++i) {
    if(token_sequence_->peek(i).source == "::" && i + 1 < open_index &&
       is_identifier_text(token_sequence_->peek(i + 1).source)) {
      anchor_index = i + 1;
    }
  }
  name_begin = begin;
  return true;
}

bool SourceLocationTokenView::template_id_at_location_is_nested(
    const std::string & location) const
{
  if(token_sequence_ == nullptr) {
    return false;
  }
  std::size_t name_index = 0;
  ParsedSourceLocation name_location;
  if(!token_index_at_source_location(location, name_index) ||
     !parsed_source_location_for_token_index(name_index, name_location)) {
    return false;
  }

  for(std::size_t i = 0; i < name_index; ++i) {
    const RecogToken & token = token_sequence_->peek(i);
    if(token.is_eof()) {
      break;
    }
    if(token.source != "<") {
      continue;
    }
    ParsedSourceLocation open_location;
    if(!parsed_source_location_for_token_index(i, open_location) ||
       open_location.file != name_location.file ||
       open_location.line > name_location.line) {
      continue;
    }
    std::size_t outer_name_begin = 0;
    std::size_t outer_anchor = 0;
    if(!template_name_before_open(i, outer_name_begin, outer_anchor)) {
      continue;
    }
    std::size_t close_index = 0;
    int close_chars = 0;
    if(matching_template_close_token(i,
                                     close_index,
                                     close_chars,
                                     false) &&
       i < name_index &&
       name_index < close_index) {
      return true;
    }
  }
  return false;
}

bool SourceLocationTokenView::template_id_at_location_is_qualified_member_owner(
    const std::string & location) const
{
  if(token_sequence_ == nullptr) {
    return false;
  }
  std::size_t name_index = 0;
  if(!token_index_at_source_location(location, name_index)) {
    return false;
  }
  std::size_t open_index = 0;
  if(!find_next_token_source_on_same_line(name_index, "<", open_index)) {
    return false;
  }
  std::size_t name_begin = 0;
  std::size_t anchor_index = 0;
  if(!template_name_before_open(open_index, name_begin, anchor_index) ||
     anchor_index != name_index) {
    return false;
  }
  std::size_t close_index = 0;
  int close_chars = 0;
  if(!matching_template_close_token(open_index,
                                    close_index,
                                    close_chars,
                                    false)) {
    return false;
  }
  if(token_sequence_->peek(close_index).source == ">>" && close_chars == 1) {
    return false;
  }
  const std::size_t next_index = close_index + 1;
  if(next_index >= token_sequence_->size()) {
    return false;
  }
  const RecogToken & next = token_sequence_->peek(next_index);
  return !next.is_eof() && next.source == "::";
}

bool SourceLocationTokenView::template_id_at_location_is_conversion_operator_result(
    const std::string & location) const
{
  if(token_sequence_ == nullptr) {
    return false;
  }
  std::size_t name_index = 0;
  ParsedSourceLocation name_location;
  if(!token_index_at_source_location(location, name_index) ||
     !parsed_source_location_for_token_index(name_index, name_location)) {
    return false;
  }
  std::size_t open_index = 0;
  if(!find_next_token_source_on_same_line(name_index, "<", open_index)) {
    return false;
  }
  std::size_t name_begin = 0;
  std::size_t anchor_index = 0;
  if(!template_name_before_open(open_index, name_begin, anchor_index) ||
     anchor_index != name_index) {
    return false;
  }
  for(std::size_t i = name_index; i > 0;) {
    --i;
    ParsedSourceLocation token_location;
    if(!parsed_source_location_for_token_index(i, token_location) ||
       token_location.file != name_location.file ||
       token_location.line != name_location.line) {
      continue;
    }
    const std::string & source = token_sequence_->peek(i).source;
    if(source == "operator") {
      return true;
    }
    if(source == ";" || source == "{" || source == "}" || source == "(") {
      return false;
    }
  }
  return false;
}

bool SourceLocationTokenView::matching_template_close_token(std::size_t open_index,
                                   std::size_t & close_index,
                                   int & close_chars,
                                   bool same_line_only) const
{
  ParsedSourceLocation open_location;
  if(token_sequence_ == nullptr ||
     token_sequence_->peek(open_index).source != "<" ||
     !parsed_source_location_for_token_index(open_index, open_location)) {
    return false;
  }
  const std::size_t token_count = token_sequence_->size();
  int angle_depth = 0;
  int paren_depth = 0;
  int bracket_depth = 0;
  int brace_depth = 0;
  for(std::size_t i = open_index + 1; i < token_count; ++i) {
    const RecogToken & token = token_sequence_->peek(i);
    if(token.is_eof()) {
      break;
    }
    ParsedSourceLocation parsed;
    if(!parsed_source_location_for_token_index(i, parsed) ||
       parsed.file != open_location.file) {
      continue;
    }
    if(same_line_only && parsed.line > open_location.line) {
      break;
    }
    if(same_line_only && parsed.line != open_location.line) {
      continue;
    }
    const std::string & source = token.source;
    if(source == "(") {
      ++paren_depth;
      continue;
    }
    if(source == ")") {
      if(paren_depth > 0) {
        --paren_depth;
      }
      continue;
    }
    if(source == "[") {
      ++bracket_depth;
      continue;
    }
    if(source == "]") {
      if(bracket_depth > 0) {
        --bracket_depth;
      }
      continue;
    }
    if(source == "{") {
      ++brace_depth;
      continue;
    }
    if(source == "}") {
      if(brace_depth > 0) {
        --brace_depth;
      }
      continue;
    }
    if(paren_depth != 0 || bracket_depth != 0 || brace_depth != 0) {
      continue;
    }
    if(source == "<") {
      ++angle_depth;
      continue;
    }
    const int count = source == ">" ? 1 : (source == ">>" ? 2 : 0);
    for(int close = 0; close < count; ++close) {
      if(angle_depth == 0) {
        close_index = i;
        close_chars = close + 1;
        return true;
      }
      --angle_depth;
    }
  }
  return false;
}

bool SourceLocationTokenView::template_argument_token_ranges_from_open(
    std::size_t open_index,
    std::vector<std::pair<std::size_t, std::size_t> > & ranges) const
{
  if(token_sequence_ == nullptr ||
     token_sequence_->peek(open_index).source != "<") {
    return false;
  }
  const std::size_t token_count = token_sequence_->size();
  std::size_t arg_begin = open_index + 1;
  int angle_depth = 0;
  int paren_depth = 0;
  int bracket_depth = 0;
  int brace_depth = 0;
  for(std::size_t i = open_index + 1; i < token_count; ++i) {
    const RecogToken & token = token_sequence_->peek(i);
    if(token.is_eof()) {
      break;
    }
    const std::string & source = token.source;
    if(source == "(") {
      ++paren_depth;
      continue;
    }
    if(source == ")") {
      if(paren_depth > 0) {
        --paren_depth;
      }
      continue;
    }
    if(source == "[") {
      ++bracket_depth;
      continue;
    }
    if(source == "]") {
      if(bracket_depth > 0) {
        --bracket_depth;
      }
      continue;
    }
    if(source == "{") {
      ++brace_depth;
      continue;
    }
    if(source == "}") {
      if(brace_depth > 0) {
        --brace_depth;
      }
      continue;
    }
    if(paren_depth != 0 || bracket_depth != 0 || brace_depth != 0) {
      continue;
    }
    if(source == "<") {
      ++angle_depth;
      continue;
    }
    if(source == "," && angle_depth == 0) {
      ranges.push_back(std::make_pair(arg_begin, i));
      arg_begin = i + 1;
      continue;
    }
    const int close_count = source == ">" ? 1 : (source == ">>" ? 2 : 0);
    for(int close = 0; close < close_count; ++close) {
      if(angle_depth == 0) {
        ranges.push_back(std::make_pair(arg_begin, i));
        return true;
      }
      --angle_depth;
    }
  }
  return false;
}

void collect_qualified_use_occurrences(
    const CppAstNode & node,
    const SourceLocationTokenView & locations,
    const template_api::TemplateWitnessContext & witness_context,
    std::vector<QualifiedUseOccurrence> & out)
{
  const cpp_decl::QualifiedName * qualified = cppast_qualified_name_syntax(node);
  if(qualified != nullptr) {
    std::string location;
    std::string anchor_name;
    if(!qualified->qualifiers.empty()) {
      anchor_name = qualified->qualifiers.front();
    } else {
      anchor_name = qualified->name;
    }
    anchor_name = strip_trailing_top_level_template_arguments(anchor_name);
    const bool witness_capture_enabled =
        witness::source_capture_enabled(witness_context);
    if(!anchor_name.empty() && witness_capture_enabled) {
      location =
          template_api::normalize_template_witness_source_location(
              locations.source_location_for_name_in_subtree(node, anchor_name));
    }
    if(!anchor_name.empty() &&
       witness_capture_enabled &&
       !template_api::template_witness_detail::
           source_location_points_at_identifier_token(witness_context,
                                                      location,
                                                      anchor_name)) {
      location.clear();
    }
    if(location.empty() && !anchor_name.empty()) {
      const std::string node_location =
          template_api::normalize_template_witness_source_location(
              locations.source_location_for_node(node));
      if(!witness_capture_enabled ||
         template_api::template_witness_detail::
             source_location_points_at_identifier_token(witness_context,
                                                        node_location,
                                                        anchor_name)) {
        location = node_location;
      }
    }
    if(!location.empty()) {
      out.push_back(QualifiedUseOccurrence{node.value, location});
    }
  }
  for(std::size_t i = 0; i < node.children.size(); ++i) {
    collect_qualified_use_occurrences(node.children[i],
                                      locations,
                                      witness_context,
                                      out);
  }
}

std::string earliest_qualified_use_location_for_prefix(
    const std::vector<QualifiedUseOccurrence> & occurrences,
    const std::string & prefix)
{
  std::string earliest;
  const std::string qualified_prefix = prefix + "::";
  for(std::size_t i = 0; i < occurrences.size(); ++i) {
    const QualifiedUseOccurrence & occurrence = occurrences[i];
    if(occurrence.value != prefix &&
       occurrence.value.compare(0, qualified_prefix.size(), qualified_prefix) != 0) {
      continue;
    }
    earliest = prefer_earlier_source_location(earliest, occurrence.location);
  }
  return earliest;
}

std::string earliest_qualified_use_location_for_value(
    const std::vector<QualifiedUseOccurrence> & occurrences,
    const std::string & value)
{
  std::string earliest;
  for(std::size_t i = 0; i < occurrences.size(); ++i) {
    const QualifiedUseOccurrence & occurrence = occurrences[i];
    if(occurrence.value != value) {
      continue;
    }
    earliest = prefer_earlier_source_location(earliest, occurrence.location);
  }
  return earliest;
}

}  // namespace callsemantic
