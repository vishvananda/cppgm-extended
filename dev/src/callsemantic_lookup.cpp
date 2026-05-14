#include "callsemantic_internal.h"

#include <cctype>

#include "semantic_utils.h"

using namespace std;

namespace callsemantic_internal {

namespace {

bool is_identifier_char(char ch)
{
  return isalnum(static_cast<unsigned char>(ch)) || ch == '_';
}

enum ReferenceSuffixKind
{
  RSK_NONE,
  RSK_LVALUE,
  RSK_RVALUE
};

ReferenceSuffixKind trailing_reference_suffix_kind(const string & text,
                                                   string & base_text)
{
  string trimmed = semantic_utils::trim_space(text);
  if(trimmed.size() >= 2 &&
     trimmed.compare(trimmed.size() - 2, 2, "&&") == 0) {
    base_text = semantic_utils::trim_space(trimmed.substr(0, trimmed.size() - 2));
    return RSK_RVALUE;
  }
  if(!trimmed.empty() && trimmed[trimmed.size() - 1] == '&') {
    base_text = semantic_utils::trim_space(trimmed.substr(0, trimmed.size() - 1));
    return RSK_LVALUE;
  }
  base_text = trimmed;
  return RSK_NONE;
}

bool find_existing_declarator_group(const string & text,
                                    size_t & open_pos,
                                    size_t & close_pos)
{
  int angle_depth = 0;
  for(size_t i = 0; i < text.size(); ++i) {
    const char ch = text[i];
    if(ch == '<') {
      ++angle_depth;
      continue;
    }
    if(ch == '>' && angle_depth > 0) {
      --angle_depth;
      continue;
    }
    if(angle_depth != 0 || ch != '(') {
      continue;
    }

    int paren_depth = 0;
    int nested_angle_depth = 0;
    for(size_t j = i; j < text.size(); ++j) {
      const char inner = text[j];
      if(inner == '<') {
        ++nested_angle_depth;
        continue;
      }
      if(inner == '>' && nested_angle_depth > 0) {
        --nested_angle_depth;
        continue;
      }
      if(nested_angle_depth != 0) {
        continue;
      }
      if(inner == '(') {
        ++paren_depth;
      } else if(inner == ')') {
        --paren_depth;
        if(paren_depth == 0) {
          if(!semantic_utils::trim_space(text.substr(j + 1)).empty()) {
            open_pos = i;
            close_pos = j;
            return true;
          }
          break;
        }
      }
    }
  }
  return false;
}

size_t find_top_level_declarator_suffix_start(const string & text)
{
  int angle_depth = 0;
  int paren_depth = 0;
  for(size_t i = 0; i < text.size(); ++i) {
    const char ch = text[i];
    if(ch == '<') {
      ++angle_depth;
      continue;
    }
    if(ch == '>' && angle_depth > 0) {
      --angle_depth;
      continue;
    }
    if(ch == '(') {
      if(angle_depth == 0 && paren_depth == 0) {
        return i;
      }
      ++paren_depth;
      continue;
    }
    if(ch == ')' && paren_depth > 0) {
      --paren_depth;
      continue;
    }
    if(angle_depth == 0 && paren_depth == 0 && ch == '[') {
      return i;
    }
  }
  return string::npos;
}

string append_or_collapse_reference_in_declarator(const string & declarator,
                                                  const string & suffix)
{
  string base_text;
  const ReferenceSuffixKind existing =
      trailing_reference_suffix_kind(declarator, base_text);
  const ReferenceSuffixKind added = suffix == "&&" ? RSK_RVALUE : RSK_LVALUE;
  ReferenceSuffixKind combined = added;
  if(existing != RSK_NONE) {
    combined = (existing == RSK_LVALUE || added == RSK_LVALUE) ?
        RSK_LVALUE :
        RSK_RVALUE;
  }

  string out = base_text;
  if(combined != RSK_NONE) {
    const bool need_space =
        !out.empty() &&
        (isalnum(static_cast<unsigned char>(out[out.size() - 1])) ||
         out[out.size() - 1] == '>' ||
         out[out.size() - 1] == ']' ||
         out[out.size() - 1] == ')');
    if(need_space) {
      out += ' ';
    }
    out += combined == RSK_LVALUE ? "&" : "&&";
  }
  return out;
}

string collapse_reference_suffix_text(const string & replacement,
                                      const string & suffix)
{
  const string trimmed = semantic_utils::trim_space(replacement);

  size_t open_pos = string::npos;
  size_t close_pos = string::npos;
  if(find_existing_declarator_group(trimmed, open_pos, close_pos)) {
    const string before = semantic_utils::trim_space(trimmed.substr(0, open_pos));
    const string inner = semantic_utils::trim_space(
        trimmed.substr(open_pos + 1, close_pos - open_pos - 1));
    const string after = trimmed.substr(close_pos + 1);
    string out = before;
    if(!out.empty()) {
      out += " ";
    }
    out += "(" + append_or_collapse_reference_in_declarator(inner, suffix) + ")";
    out += after;
    return out;
  }

  const size_t suffix_start = find_top_level_declarator_suffix_start(trimmed);
  if(suffix_start != string::npos) {
    const string before = semantic_utils::trim_space(trimmed.substr(0, suffix_start));
    const string after = trimmed.substr(suffix_start);
    string out = before;
    if(!out.empty()) {
      out += " ";
    }
    out += "(" + append_or_collapse_reference_in_declarator(string(), suffix) + ")";
    out += after;
    return out;
  }

  return append_or_collapse_reference_in_declarator(trimmed, suffix);
}

bool extract_leading_cv_qualifier_prefix(const string & text,
                                         size_t match_start,
                                         size_t & prefix_start,
                                         bool & add_const,
                                         bool & add_volatile)
{
  prefix_start = match_start;
  add_const = false;
  add_volatile = false;

  size_t pos = match_start;
  while(pos != 0) {
    size_t token_end = pos;
    while(token_end != 0 &&
          isspace(static_cast<unsigned char>(text[token_end - 1]))) {
      --token_end;
    }
    if(token_end == 0) {
      break;
    }

    size_t token_start = token_end;
    while(token_start != 0 && is_identifier_char(text[token_start - 1])) {
      --token_start;
    }
    if(token_start == token_end) {
      break;
    }

    const string token = text.substr(token_start, token_end - token_start);
    if(token == "const") {
      add_const = true;
    } else if(token == "volatile") {
      add_volatile = true;
    } else {
      break;
    }
    prefix_start = token_start;
    pos = token_start;
  }

  return prefix_start != match_start;
}

bool replacement_text_has_top_level_paren_or_bracket(const string & text)
{
  int angle_depth = 0;
  int paren_depth = 0;
  for(size_t i = 0; i < text.size(); ++i) {
    const char ch = text[i];
    if(ch == '<') {
      ++angle_depth;
      continue;
    }
    if(ch == '>' && angle_depth > 0) {
      --angle_depth;
      continue;
    }
    if(angle_depth != 0) {
      continue;
    }
    if(ch == '(') {
      ++paren_depth;
    } else if(ch == ')' && paren_depth > 0) {
      --paren_depth;
    } else if(paren_depth == 0 && (ch == '(' || ch == '[' || ch == ']')) {
      return true;
    }
  }
  return false;
}

bool text_has_top_level_cv_suffix(const string & text, const string & qualifier)
{
  const string trimmed = semantic_utils::trim_space(text);
  if(trimmed.size() <= qualifier.size()) {
    return false;
  }
  const string needle = " " + qualifier;
  return trimmed.compare(trimmed.size() - needle.size(), needle.size(), needle) == 0;
}

string apply_leading_cv_qualifiers_to_replacement_text(const string & replacement,
                                                       bool add_const,
                                                       bool add_volatile)
{
  if(!add_const && !add_volatile) {
    return replacement;
  }

  string base_text;
  if(trailing_reference_suffix_kind(replacement, base_text) != RSK_NONE) {
    return replacement;
  }

  string out = semantic_utils::trim_space(replacement);
  if(out.empty() || replacement_text_has_top_level_paren_or_bracket(out)) {
    return replacement;
  }

  if(add_const && !text_has_top_level_cv_suffix(out, "const")) {
    out += " const";
  }
  if(add_volatile && !text_has_top_level_cv_suffix(out, "volatile")) {
    out += " volatile";
  }
  return out;
}

bool replacement_text_is_explicit_template_id(const string & replacement)
{
  return replacement.find('<') != string::npos &&
         replacement.find('>') != string::npos;
}

}  // namespace

string replace_identifier_token_text(const string & text,
                                     const string & name,
                                     const string & replacement,
                                     bool & changed)
{
  if(text.find(name) == string::npos) {
    return text;
  }
  string out;
  size_t i = 0;
  while(i < text.size()) {
    if(text.compare(i, name.size(), name) == 0 &&
       !identifier_is_qualified_component(text, i) &&
       (i == 0 || !is_identifier_char(text[i - 1])) &&
       (i + name.size() == text.size() || !is_identifier_char(text[i + name.size()]))) {
      size_t next = i + name.size();
      size_t suffix_pos = next;
      while(suffix_pos < text.size() &&
            isspace(static_cast<unsigned char>(text[suffix_pos]))) {
        ++suffix_pos;
      }

      if(suffix_pos < text.size() &&
         text[suffix_pos] == '<' &&
         replacement_text_is_explicit_template_id(replacement)) {
        out.append(text, i, next - i);
        i = next;
        continue;
      }

      size_t prefix_start = i;
      bool add_const = false;
      bool add_volatile = false;
      string adjusted_replacement = replacement;
      if(extract_leading_cv_qualifier_prefix(text, i, prefix_start, add_const, add_volatile)) {
        adjusted_replacement =
            apply_leading_cv_qualifiers_to_replacement_text(replacement,
                                                            add_const,
                                                            add_volatile);
        out.erase(out.size() - (i - prefix_start));
      }

      if(suffix_pos + 1 < text.size() &&
         text.compare(suffix_pos, 2, "::") == 0 &&
         has_invalid_top_level_qualified_owner_syntax(adjusted_replacement + "::probe")) {
        out += text[i++];
        continue;
      }

      if(suffix_pos + 1 < text.size() &&
         text.compare(suffix_pos, 2, "&&") == 0) {
        out += collapse_reference_suffix_text(adjusted_replacement, "&&");
        i = suffix_pos + 2;
      } else if(suffix_pos < text.size() && text[suffix_pos] == '&') {
        out += collapse_reference_suffix_text(adjusted_replacement, "&");
        i = suffix_pos + 1;
      } else {
        out += adjusted_replacement;
        i = next;
      }
      changed = true;
      continue;
    }
    out += text[i++];
  }
  return out;
}

string replace_elaborated_identifier_token_text(const string & text,
                                                const string & name,
                                                const string & replacement,
                                                bool & changed)
{
  if(text.find(name) == string::npos) {
    return text;
  }
  static const char * prefixes[] = {"typename", "class", "struct", "union", "enum"};

  string out;
  size_t i = 0;
  while(i < text.size()) {
    bool matched = false;
    for(size_t prefix_index = 0;
        prefix_index < sizeof(prefixes) / sizeof(prefixes[0]);
        ++prefix_index) {
      const string prefix(prefixes[prefix_index]);
      if(text.compare(i, prefix.size(), prefix) != 0 ||
         (i != 0 && is_identifier_char(text[i - 1]))) {
        continue;
      }
      size_t after_prefix = i + prefix.size();
      if(after_prefix < text.size() &&
         is_identifier_char(text[after_prefix])) {
        continue;
      }
      size_t name_pos = after_prefix;
      while(name_pos < text.size() &&
            isspace(static_cast<unsigned char>(text[name_pos]))) {
        ++name_pos;
      }
      if(text.compare(name_pos, name.size(), name) != 0 ||
         (name_pos != 0 && is_identifier_char(text[name_pos - 1])) ||
         (name_pos + name.size() < text.size() &&
          is_identifier_char(text[name_pos + name.size()]))) {
        continue;
      }

      size_t prefix_start = i;
      bool add_const = false;
      bool add_volatile = false;
      string adjusted_replacement = replacement;
      if(extract_leading_cv_qualifier_prefix(text, i, prefix_start, add_const, add_volatile)) {
        adjusted_replacement =
            apply_leading_cv_qualifiers_to_replacement_text(replacement,
                                                            add_const,
                                                            add_volatile);
        out.erase(out.size() - (i - prefix_start));
      }
      size_t next = name_pos + name.size();
      size_t suffix_pos = next;
      while(suffix_pos < text.size() &&
            isspace(static_cast<unsigned char>(text[suffix_pos]))) {
        ++suffix_pos;
      }

      if(suffix_pos + 1 < text.size() &&
         text.compare(suffix_pos, 2, "::") == 0 &&
         has_invalid_top_level_qualified_owner_syntax(adjusted_replacement + "::probe")) {
        continue;
      }
      if(suffix_pos < text.size() &&
         text[suffix_pos] == '<' &&
         replacement_text_is_explicit_template_id(adjusted_replacement)) {
        continue;
      }

      if(suffix_pos + 1 < text.size() &&
         text.compare(suffix_pos, 2, "&&") == 0) {
        out += collapse_reference_suffix_text(adjusted_replacement, "&&");
        i = suffix_pos + 2;
      } else if(suffix_pos < text.size() && text[suffix_pos] == '&') {
        out += collapse_reference_suffix_text(adjusted_replacement, "&");
        i = suffix_pos + 1;
      } else {
        out += adjusted_replacement;
        i = next;
      }
      changed = true;
      matched = true;
      break;
    }
    if(!matched) {
      out += text[i++];
    }
  }
  return out;
}

}  // namespace callsemantic_internal
