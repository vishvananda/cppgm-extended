#include "semantic_utils.h"

#include "cpp_decl_model.h"

#include <algorithm>
#include <cctype>
#include <unordered_map>
#include <vector>

namespace semantic_utils {

namespace {

inline bool is_ascii_space(unsigned char ch)
{
  return ch == ' ' || ch == '\t' || ch == '\n' ||
         ch == '\r' || ch == '\f' || ch == '\v';
}

bool is_less_equal_operator_at(const std::string & text, std::size_t pos)
{
  return pos + 1 < text.size() && text[pos] == '<' && text[pos + 1] == '=';
}

bool is_greater_equal_operator_at(const std::string & text, std::size_t pos)
{
  return pos + 1 < text.size() && text[pos] == '>' && text[pos + 1] == '=';
}

std::size_t previous_non_space(const std::string & text, std::size_t pos)
{
  while(pos > 0) {
    --pos;
    if(!is_ascii_space(static_cast<unsigned char>(text[pos]))) {
      return pos;
    }
  }
  return std::string::npos;
}

bool is_relational_less_operator_at(const std::string & text, std::size_t pos)
{
  if(is_less_equal_operator_at(text, pos)) {
    return true;
  }
  const std::size_t prev = previous_non_space(text, pos);
  return prev != std::string::npos &&
         (text[prev] == ')' || text[prev] == ']');
}

}  // namespace

std::string trim_space(std::string text)
{
  std::size_t start = 0;
  while(start < text.size() &&
        is_ascii_space(static_cast<unsigned char>(text[start]))) {
    ++start;
  }

  std::size_t end = text.size();
  while(end > start &&
        is_ascii_space(static_cast<unsigned char>(text[end - 1]))) {
    --end;
  }

  if(end < text.size()) {
    text.resize(end);
  }
  if(start > 0) {
    text.erase(0, start);
  }
  return text;
}

std::string strip_elaborated_type_prefix(const std::string & text)
{
  struct Prefix
  {
    const char * text;
    std::size_t size;
  };
  static const Prefix prefixes[] = {
      {"enum class ", 11},
      {"enum struct ", 12},
      {"class ", 6},
      {"struct ", 7},
      {"union ", 6},
      {"enum ", 5}};
  for(std::size_t i = 0; i < sizeof(prefixes) / sizeof(prefixes[0]); ++i) {
    if(text.compare(0, prefixes[i].size, prefixes[i].text) == 0) {
      return text.substr(prefixes[i].size);
    }
  }
  return text;
}

std::string strip_trailing_top_level_template_arguments(const std::string & text)
{
  const std::string trimmed = trim_space(text);
  if(trimmed.empty() || trimmed[trimmed.size() - 1] != '>') {
    return trimmed;
  }

  int depth = 0;
  int paren_depth = 0;
  int bracket_depth = 0;
  int brace_depth = 0;
  for(std::size_t i = trimmed.size(); i > 0; --i) {
    const char ch = trimmed[i - 1];
    if(ch == ')') {
      ++paren_depth;
    } else if(ch == '(') {
      if(paren_depth > 0) {
        --paren_depth;
      }
    } else if(ch == ']') {
      ++bracket_depth;
    } else if(ch == '[') {
      if(bracket_depth > 0) {
        --bracket_depth;
      }
    } else if(ch == '}') {
      ++brace_depth;
    } else if(ch == '{') {
      if(brace_depth > 0) {
        --brace_depth;
      }
    } else if(ch == '>' &&
              !(i < trimmed.size() && trimmed[i] == '=') &&
              paren_depth == 0 &&
              bracket_depth == 0 &&
              brace_depth == 0) {
      ++depth;
    } else if(ch == '<' &&
              !is_relational_less_operator_at(trimmed, i - 1) &&
              paren_depth == 0 &&
              bracket_depth == 0 &&
              brace_depth == 0) {
      --depth;
      if(depth == 0) {
        return trim_space(trimmed.substr(0, i - 1));
      }
    }
  }
  return trimmed;
}

std::size_t top_level_scope_split(const std::string & name)
{
  static std::unordered_map<std::string, std::size_t> cache;
  std::unordered_map<std::string, std::size_t>::const_iterator found =
      cache.find(name);
  if(found != cache.end()) {
    return found->second;
  }

  int angle_depth = 0;
  int paren_depth = 0;
  int bracket_depth = 0;
  int brace_depth = 0;
  std::size_t split = std::string::npos;
  for(std::size_t i = 0; i < name.size(); ++i) {
    const char ch = name[i];
    switch(ch) {
    case '<':
      if(is_relational_less_operator_at(name, i)) {
        break;
      }
      if(paren_depth == 0 && bracket_depth == 0 && brace_depth == 0) {
        ++angle_depth;
      }
      break;
    case '>':
      if(is_greater_equal_operator_at(name, i)) {
        break;
      }
      if(paren_depth == 0 && bracket_depth == 0 &&
         brace_depth == 0 && angle_depth > 0) {
        --angle_depth;
      }
      break;
    case '(':
      ++paren_depth;
      break;
    case ')':
      if(paren_depth > 0) {
        --paren_depth;
      }
      break;
    case '[':
      ++bracket_depth;
      break;
    case ']':
      if(bracket_depth > 0) {
        --bracket_depth;
      }
      break;
    case '{':
      ++brace_depth;
      break;
    case '}':
      if(brace_depth > 0) {
        --brace_depth;
      }
      break;
    case ':':
      if(angle_depth == 0 && paren_depth == 0 && bracket_depth == 0 &&
         brace_depth == 0 && i + 1 < name.size() && name[i + 1] == ':') {
        split = i;
        ++i;
      }
      break;
    default:
      break;
    }
  }
  cache[name] = split;
  return split;
}

std::string unqualified_member_name(const std::string & name)
{
  const std::size_t pos = top_level_scope_split(name);
  return pos == std::string::npos ? name : name.substr(pos + 2);
}

bool is_wrapped_in_balanced_parens(const std::string & text)
{
  if(text.size() < 2 || text.front() != '(' || text.back() != ')') {
    return false;
  }

  int depth = 0;
  for(std::size_t i = 0; i < text.size(); ++i) {
    if(text[i] == '(') {
      ++depth;
    } else if(text[i] == ')') {
      --depth;
      if(depth < 0) {
        return false;
      }
      if(depth == 0 && i + 1 != text.size()) {
        return false;
      }
    }
  }

  return depth == 0;
}

}  // namespace semantic_utils
