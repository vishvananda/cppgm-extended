#include "witness_text.h"

#include <cctype>
#include <fstream>
#include <regex>
#include <set>
#include <string>
#include <vector>

namespace witness_text {
namespace {

bool is_identifier_char(char ch)
{
  return std::isalnum(static_cast<unsigned char>(ch)) || ch == '_';
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

bool out_ends_with_word(const std::string & text, const std::string & word)
{
  if(text.size() < word.size() ||
     text.compare(text.size() - word.size(), word.size(), word) != 0) {
    return false;
  }
  const std::size_t before = text.size() - word.size();
  return before == 0 || !is_identifier_char(text[before - 1]);
}

bool out_ends_with_spaced_operator(const std::string & text)
{
  return (text.size() >= 2 &&
          (text.compare(text.size() - 2, 2, " >") == 0 ||
           text.compare(text.size() - 2, 2, " <") == 0)) ||
         (text.size() >= 3 &&
          (text.compare(text.size() - 3, 3, " >>") == 0 ||
           text.compare(text.size() - 3, 3, " <<") == 0));
}

std::string trailing_identifier_word(const std::string & text)
{
  std::size_t end = text.size();
  while(end > 0 &&
        std::isspace(static_cast<unsigned char>(text[end - 1]))) {
    --end;
  }
  std::size_t begin = end;
  while(begin > 0 && is_identifier_char(text[begin - 1])) {
    --begin;
  }
  return text.substr(begin, end - begin);
}

int find_matching_paren(const std::string & text, std::size_t open_pos);

bool is_builtin_type_word(const std::string & word)
{
  return word == "void" ||
         word == "bool" ||
         word == "char" ||
         word == "char16_t" ||
         word == "char32_t" ||
         word == "wchar_t" ||
         word == "short" ||
         word == "int" ||
         word == "long" ||
         word == "signed" ||
         word == "unsigned" ||
         word == "float" ||
         word == "double";
}

bool has_simple_expression_cast_argument(const std::string & text,
                                         std::size_t open_pos)
{
  std::size_t pos = open_pos + 1;
  while(pos < text.size() &&
        std::isspace(static_cast<unsigned char>(text[pos]))) {
    ++pos;
  }
  if(pos >= text.size() || !is_identifier_char(text[pos])) {
    return false;
  }

  std::size_t depth = 1;
  bool saw_scope_or_member = false;
  for(; pos < text.size(); ++pos) {
    const char ch = text[pos];
    if(ch == '(') {
      ++depth;
      continue;
    }
    if(ch == ')') {
      --depth;
      if(depth == 0) {
        return saw_scope_or_member;
      }
      continue;
    }
    if(depth != 1) {
      continue;
    }
    if(ch == ',' || ch == '*' || ch == '&') {
      return false;
    }
    if(ch == ':' && pos + 1 < text.size() && text[pos + 1] == ':') {
      saw_scope_or_member = true;
    }
    if(ch == '.') {
      saw_scope_or_member = true;
    }
  }
  return false;
}

bool remove_space_before_open_paren(const std::string & out,
                                    const std::string & text,
                                    std::size_t open_pos)
{
  const char prev_ch = out.empty() ? '\0' : out[out.size() - 1];
  if(!is_identifier_char(prev_ch) && prev_ch != '>') {
    return false;
  }
  if(out_ends_with_spaced_operator(out)) {
    return false;
  }

  std::size_t inside = open_pos + 1;
  while(inside < text.size() &&
        std::isspace(static_cast<unsigned char>(text[inside]))) {
    ++inside;
  }
  if(inside < text.size() &&
     (text[inside] == '*' || text[inside] == '&')) {
    return false;
  }
  const int close = find_matching_paren(text, open_pos);
  if(close > static_cast<int>(open_pos)) {
    const std::string body =
        text.substr(open_pos + 1, static_cast<std::size_t>(close) - open_pos - 1);
    if(body.find("::*") != std::string::npos) {
      return false;
    }
  }

  const std::string word = trailing_identifier_word(out);
  if(is_builtin_type_word(word)) {
    return has_simple_expression_cast_argument(text, open_pos);
  }
  if(!word.empty() &&
     std::isdigit(static_cast<unsigned char>(word[word.size() - 1]))) {
    return false;
  }
  return true;
}

std::size_t previous_nonspace_position(const std::string & text,
                                       std::size_t pos)
{
  while(pos > 0) {
    --pos;
    if(!std::isspace(static_cast<unsigned char>(text[pos]))) {
      return pos;
    }
  }
  return std::string::npos;
}

std::size_t next_nonspace_position(const std::string & text,
                                   std::size_t pos)
{
  while(pos < text.size()) {
    if(!std::isspace(static_cast<unsigned char>(text[pos]))) {
      return pos;
    }
    ++pos;
  }
  return std::string::npos;
}

bool decltype_operator_rhs_starts_expression(const std::string & text,
                                             std::size_t pos)
{
  pos = next_nonspace_position(text, pos);
  if(pos == std::string::npos) {
    return false;
  }
  if(text[pos] == '(') {
    return true;
  }
  if(!(std::isalpha(static_cast<unsigned char>(text[pos])) || text[pos] == '_')) {
    return false;
  }
  ++pos;
  while(pos < text.size() && is_identifier_char(text[pos])) {
    ++pos;
  }
  pos = next_nonspace_position(text, pos);
  if(pos == std::string::npos) {
    return false;
  }
  if(text[pos] == '<') {
    int depth = 0;
    for(; pos < text.size(); ++pos) {
      if(text[pos] == '<') {
        ++depth;
      } else if(text[pos] == '>') {
        --depth;
        if(depth == 0) {
          ++pos;
          break;
        }
      }
    }
    if(depth != 0) {
      return false;
    }
    pos = next_nonspace_position(text, pos);
    if(pos == std::string::npos) {
      return false;
    }
  }
  return text[pos] == '(';
}

bool parenthesized_decltype_binary_operator_at(const std::string & text,
                                               std::size_t pos,
                                               const std::string & op)
{
  if(text.compare(pos, op.size(), op) != 0) {
    return false;
  }
  const std::size_t prev = previous_nonspace_position(text, pos);
  return prev != std::string::npos &&
         text[prev] == ')' &&
         decltype_operator_rhs_starts_expression(text, pos + op.size());
}

void trim_trailing_spaces(std::string & text)
{
  while(!text.empty() &&
        std::isspace(static_cast<unsigned char>(text[text.size() - 1]))) {
    text.erase(text.size() - 1);
  }
}

void append_spaced_operator(std::string & out, const std::string & op)
{
  if(out_ends_with_word(out, "operator")) {
    out += op;
    return;
  }
  trim_trailing_spaces(out);
  if(!out.empty()) {
    out += ' ';
  }
  out += op;
  out += ' ';
}

int find_matching_paren(const std::string & text, std::size_t open_pos)
{
  int depth = 0;
  bool in_string = false;
  bool in_char = false;
  bool escaped = false;
  for(std::size_t i = open_pos; i < text.size(); ++i) {
    const char ch = text[i];
    if(in_string || in_char) {
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
      continue;
    }
    if(ch == '\'') {
      in_char = true;
      continue;
    }
    if(ch == '(') {
      ++depth;
    } else if(ch == ')') {
      --depth;
      if(depth == 0) {
        return static_cast<int>(i);
      }
    }
  }
  return -1;
}

bool decltype_like_token_at(const std::string & text,
                            std::size_t pos,
                            std::string & token)
{
  static const char * const tokens[] = {
      "decltype", "__decltype", "__decltype__", "__typeof", "__typeof__"};
  for(std::size_t i = 0; i < sizeof(tokens) / sizeof(tokens[0]); ++i) {
    const std::string candidate = tokens[i];
    if(text.compare(pos, candidate.size(), candidate) != 0) {
      continue;
    }
    const std::size_t after = pos + candidate.size();
    if(pos > 0 && is_identifier_char(text[pos - 1])) {
      continue;
    }
    if(after < text.size() && is_identifier_char(text[after])) {
      continue;
    }
    token = candidate;
    return true;
  }
  return false;
}

std::string normalize_decltype_call_body_spacing(const std::string & text)
{
  std::string out;
  out.reserve(text.size());
  for(std::size_t i = 0; i < text.size();) {
    const std::string two = i + 1 < text.size() ? text.substr(i, 2) :
        std::string();
    if((two == ">>" || two == "<<") &&
       parenthesized_decltype_binary_operator_at(text, i, two)) {
      append_spaced_operator(out, two);
      i += 2;
      while(i < text.size() &&
            std::isspace(static_cast<unsigned char>(text[i]))) {
        ++i;
      }
      continue;
    }
    if((text[i] == '>' || text[i] == '<') &&
       parenthesized_decltype_binary_operator_at(
           text, i, std::string(1, text[i]))) {
      append_spaced_operator(out, std::string(1, text[i]));
      ++i;
      while(i < text.size() &&
            std::isspace(static_cast<unsigned char>(text[i]))) {
        ++i;
      }
      continue;
    }
    if(!std::isspace(static_cast<unsigned char>(text[i]))) {
      out += text[i++];
      continue;
    }
    std::size_t next = i;
    while(next < text.size() &&
          std::isspace(static_cast<unsigned char>(text[next]))) {
      ++next;
    }
    const char prev_ch = out.empty() ? '\0' : out[out.size() - 1];
    const char next_ch = next < text.size() ? text[next] : '\0';
    if(next_ch == '(' && (is_identifier_char(prev_ch) || prev_ch == '>')) {
      i = next;
      continue;
    }
    out += ' ';
    i = next;
  }
  return out;
}

std::string normalize_decltype_expression_layout(const std::string & text)
{
  std::string out;
  out.reserve(text.size());
  for(std::size_t i = 0; i < text.size();) {
    std::string token;
    if(!decltype_like_token_at(text, i, token)) {
      out += text[i++];
      continue;
    }
    std::size_t open = i + token.size();
    while(open < text.size() &&
          std::isspace(static_cast<unsigned char>(text[open]))) {
      ++open;
    }
    if(open >= text.size() || text[open] != '(') {
      out += text[i++];
      continue;
    }
    const int close = find_matching_paren(text, open);
    if(close < 0) {
      out += text[i++];
      continue;
    }
    const std::string body =
        text.substr(open + 1, static_cast<std::size_t>(close) - open - 1);
    out += token;
    out += '(';
    out += normalize_decltype_call_body_spacing(
        normalize_decltype_expression_layout(body));
    out += ')';
    i = static_cast<std::size_t>(close) + 1;
  }
  return out;
}

std::string normalize_source_token_spacing(const std::string & text)
{
  std::string out;
  out.reserve(text.size());
  bool in_string = false;
  bool in_char = false;
  bool escaped = false;
  for(std::size_t i = 0; i < text.size(); ++i) {
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

    if(std::isspace(static_cast<unsigned char>(ch))) {
      std::size_t next = i + 1;
      while(next < text.size() &&
            std::isspace(static_cast<unsigned char>(text[next]))) {
        ++next;
      }
      const char prev_ch = out.empty() ? '\0' : out[out.size() - 1];
      const char next_ch = next < text.size() ? text[next] : '\0';
      const bool skip =
          next_ch == '\0' ||
          next_ch == ',' ||
          next_ch == ')' ||
          next_ch == ']' ||
          (next_ch == '(' &&
           remove_space_before_open_paren(out, text, next)) ||
          prev_ch == '(' ||
          prev_ch == '[';
      if(!skip && !out.empty() && out[out.size() - 1] != ' ') {
        out += ' ';
      }
      i = next == 0 ? next : next - 1;
      continue;
    }

    if(ch == ',') {
      trim_trailing_spaces(out);
      out += ", ";
      while(i + 1 < text.size() &&
            std::isspace(static_cast<unsigned char>(text[i + 1]))) {
        ++i;
      }
      continue;
    }

    if(i + 1 < text.size()) {
      const std::string two = text.substr(i, 2);
      if(two == "&&" ||
         two == "||" ||
         two == "==" ||
         two == "!=" ||
         two == "<=" ||
         two == ">=") {
        append_spaced_operator(out, two);
        ++i;
        while(i + 1 < text.size() &&
              std::isspace(static_cast<unsigned char>(text[i + 1]))) {
          ++i;
        }
        continue;
      }
    }

    if(ch == ')' || ch == ']') {
      trim_trailing_spaces(out);
    }
    out += ch;
  }
  trim_trailing_spaces(out);
  return out;
}

}  // namespace

std::vector<std::string> inline_namespace_names(
    const std::vector<std::string> & lines)
{
  std::vector<std::string> out;
  std::set<std::string> seen;
  static const std::regex pattern("\\binline\\s+namespace\\s+([A-Za-z_][A-Za-z0-9_]*)\\b");
  for(std::size_t i = 0; i < lines.size(); ++i) {
    std::smatch match;
    if(std::regex_search(lines[i], match, pattern)) {
      const std::string name = match[1].str();
      if(seen.insert(name).second) {
        out.push_back(name);
      }
    }
  }
  return out;
}

std::vector<std::string> inline_namespace_names_from_source(
    const std::string & path)
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
  return inline_namespace_names(lines);
}

std::string strip_inline_namespace_segments(
    const std::string & text,
    const std::vector<std::string> & inline_names)
{
  std::string out = text;
  for(std::size_t i = 0; i < inline_names.size(); ++i) {
    replace_all(out, "::" + inline_names[i] + "::", "::");
  }
  return out;
}

std::string normalize_anonymous_namespace_segments(const std::string & text)
{
  static const std::regex anonymous_namespace_regex("_GLOBAL__N_[0-9]+");
  return std::regex_replace(text,
                            anonymous_namespace_regex,
                            "(anonymous namespace)");
}

std::string normalize_source_spelling_text(const std::string & text)
{
  return normalize_source_token_spacing(
      normalize_decltype_expression_layout(text));
}

}  // namespace witness_text
