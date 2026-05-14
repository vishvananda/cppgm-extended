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

std::string trim_space(const std::string & text)
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

  return text.substr(start, end - start);
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

bool split_qualified_name_text(const std::string & text, cpp_decl::QualifiedName & out)
{
  std::string remaining = trim_space(text);
  if(remaining.empty()) {
    return false;
  }

  out = cpp_decl::QualifiedName();
  if(remaining.compare(0, 2, "::") == 0) {
    out.rooted = true;
    remaining = trim_space(remaining.substr(2));
  }

  std::vector<std::string> reversed_components;
  while(!remaining.empty()) {
    const std::size_t split = top_level_scope_split(remaining);
    if(split == std::string::npos) {
      reversed_components.push_back(trim_space(remaining));
      break;
    }
    reversed_components.push_back(trim_space(remaining.substr(split + 2)));
    remaining = trim_space(remaining.substr(0, split));
  }
  if(reversed_components.empty()) {
    return false;
  }
  std::reverse(reversed_components.begin(), reversed_components.end());
  for(std::size_t i = 0; i < reversed_components.size(); ++i) {
    if(reversed_components[i].empty()) {
      return false;
    }
  }

  out.name = reversed_components.back();
  out.qualifiers.assign(reversed_components.begin(), reversed_components.end() - 1);
  return !out.name.empty();
}

bool split_top_level_template_id_text(const std::string & text,
                                      cpp_decl::QualifiedName & out,
                                      std::vector<std::string> & arg_texts)
{
  out = cpp_decl::QualifiedName();
  arg_texts.clear();
  const std::string lookup_text = strip_elaborated_type_prefix(trim_space(text));
  if(lookup_text.size() < 3 || lookup_text.back() != '>') {
    return false;
  }

  std::size_t open = std::string::npos;
  int angle_depth = 0;
  int paren_depth = 0;
  int bracket_depth = 0;
  int brace_depth = 0;
  for(std::size_t i = 0; i < lookup_text.size(); ++i) {
    const char ch = lookup_text[i];
    if(ch == '<' &&
       !is_relational_less_operator_at(lookup_text, i) &&
       angle_depth == 0 &&
       paren_depth == 0 &&
       bracket_depth == 0 &&
       brace_depth == 0) {
      open = i;
    }
    switch(ch) {
    case '<':
      if(is_relational_less_operator_at(lookup_text, i)) {
        break;
      }
      if(paren_depth == 0 && bracket_depth == 0 && brace_depth == 0) {
        ++angle_depth;
      }
      break;
    case '>':
      if(is_greater_equal_operator_at(lookup_text, i)) {
        break;
      }
      if(paren_depth == 0 && bracket_depth == 0 && brace_depth == 0) {
        --angle_depth;
        if(angle_depth < 0) {
          return false;
        }
      }
      break;
    case '(':
      if(angle_depth > 0) {
        ++paren_depth;
      }
      break;
    case ')':
      if(angle_depth > 0 && --paren_depth < 0) {
        return false;
      }
      break;
    case '[':
      if(angle_depth > 0) {
        ++bracket_depth;
      }
      break;
    case ']':
      if(angle_depth > 0 && --bracket_depth < 0) {
        return false;
      }
      break;
    case '{':
      if(angle_depth > 0) {
        ++brace_depth;
      }
      break;
    case '}':
      if(angle_depth > 0 && --brace_depth < 0) {
        return false;
      }
      break;
    default:
      break;
    }
  }
  if(open == std::string::npos ||
     angle_depth != 0 ||
     paren_depth != 0 ||
     bracket_depth != 0 ||
     brace_depth != 0) {
    return false;
  }

  if(!split_qualified_name_text(trim_space(lookup_text.substr(0, open)), out)) {
    return false;
  }

  const std::string body =
      lookup_text.substr(open + 1, lookup_text.size() - open - 2);
  if(trim_space(body).empty()) {
    return true;
  }

  std::size_t start = 0;
  angle_depth = 0;
  paren_depth = 0;
  bracket_depth = 0;
  brace_depth = 0;
  for(std::size_t i = 0; i <= body.size(); ++i) {
    const char ch = i < body.size() ? body[i] : ',';
    const bool at_separator =
        (i == body.size() || ch == ',') &&
        angle_depth == 0 &&
        paren_depth == 0 &&
        bracket_depth == 0 &&
        brace_depth == 0;
    if(at_separator) {
      arg_texts.push_back(trim_space(body.substr(start, i - start)));
      start = i + 1;
      continue;
    }
    switch(ch) {
    case '<':
      if(is_relational_less_operator_at(body, i)) {
        break;
      }
      if(paren_depth == 0 && bracket_depth == 0 && brace_depth == 0) {
        ++angle_depth;
      }
      break;
    case '>':
      if(is_greater_equal_operator_at(body, i)) {
        break;
      }
      if(paren_depth == 0 && bracket_depth == 0 && brace_depth == 0) {
        if(--angle_depth < 0) {
          return false;
        }
      }
      break;
    case '(': ++paren_depth; break;
    case ')':
      if(--paren_depth < 0) {
        return false;
      }
      break;
    case '[': ++bracket_depth; break;
    case ']':
      if(--bracket_depth < 0) {
        return false;
      }
      break;
    case '{': ++brace_depth; break;
    case '}':
      if(--brace_depth < 0) {
        return false;
      }
      break;
    default:
      break;
    }
  }

  return true;
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

bool has_top_level_comma(const std::string & text)
{
  int angle_depth = 0;
  int paren_depth = 0;
  int bracket_depth = 0;
  int brace_depth = 0;
  for(std::size_t i = 0; i < text.size(); ++i) {
    switch(text[i]) {
    case '<':
      if(is_relational_less_operator_at(text, i)) {
        break;
      }
      if(paren_depth == 0 && bracket_depth == 0 && brace_depth == 0) {
        ++angle_depth;
      }
      break;
    case '>':
      if(is_greater_equal_operator_at(text, i)) {
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
    case ',':
      if(angle_depth == 0 && paren_depth == 0 && bracket_depth == 0 &&
         brace_depth == 0) {
        return true;
      }
      break;
    default:
      break;
    }
  }

  return false;
}

}  // namespace semantic_utils
