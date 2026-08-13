#include "template_angle_parser.h"

#include <sstream>

#include "cpp_syntax.h"
#include "parser_trace.h"

namespace template_angle {

namespace {

enum ParseHeuristicCacheState : unsigned char
{
  PHCS_IN_PROGRESS = 1,
  PHCS_FALSE = 2,
  PHCS_TRUE = 3
};

inline bool lookup_cached_heuristic(
    const std::vector<unsigned char> * cache,
    std::size_t key,
    bool & value)
{
  if(cache == nullptr) {
    return false;
  }
  if(key >= cache->size()) {
    return false;
  }
  const unsigned char state = (*cache)[key];
  if(state == 0) {
    return false;
  }
  if(state == PHCS_TRUE) {
    value = true;
    return true;
  }
  if(state == PHCS_FALSE || state == PHCS_IN_PROGRESS) {
    value = false;
    return true;
  }
  return false;
}

inline void store_cached_heuristic(
    std::vector<unsigned char> * cache,
    std::size_t key,
    bool value)
{
  if(cache == nullptr) {
    return;
  }
  if(key >= cache->size()) {
    cache->resize(key + 1, 0);
  }
  (*cache)[key] = value ? PHCS_TRUE : PHCS_FALSE;
}

inline void mark_cached_heuristic_in_progress(std::vector<unsigned char> * cache,
                                              std::size_t key)
{
  if(cache == nullptr) {
    return;
  }
  if(key >= cache->size()) {
    cache->resize(key + 1, 0);
  }
  (*cache)[key] = PHCS_IN_PROGRESS;
}

bool template_id_candidate_crosses_logical_operator(
    const IRecogTokenSequence & tokens,
    std::size_t start)
{
  int angle_depth = 1;
  int paren_depth = 0;
  int bracket_depth = 0;
  int brace_depth = 0;

  for(std::size_t pos = start; !tokens.peek(pos).is_eof(); ++pos) {
    const RecogToken & token = tokens.peek(pos);
    const bool track_angles =
        paren_depth == 0 && bracket_depth == 0 && brace_depth == 0;

    if(track_angles && token.is_simple(OP_LT)) {
      ++angle_depth;
      continue;
    }
    if(track_angles && token.is_close_angle_bracket()) {
      if(angle_depth == 1) {
        return false;
      }
      if(angle_depth > 1) {
        --angle_depth;
      }
      continue;
    }

    if(token.is_simple(OP_LPAREN)) {
      ++paren_depth;
      continue;
    }
    if(token.is_simple(OP_RPAREN)) {
      if(paren_depth == 0) {
        return false;
      }
      --paren_depth;
      continue;
    }
    if(token.is_simple(OP_LSQUARE)) {
      ++bracket_depth;
      continue;
    }
    if(token.is_simple(OP_RSQUARE)) {
      if(bracket_depth == 0) {
        return false;
      }
      --bracket_depth;
      continue;
    }
    if(token.is_simple(OP_LBRACE)) {
      ++brace_depth;
      continue;
    }
    if(token.is_simple(OP_RBRACE)) {
      if(brace_depth == 0) {
        return false;
      }
      --brace_depth;
      continue;
    }

    const bool top_level = angle_depth == 1 &&
                           paren_depth == 0 &&
                           bracket_depth == 0 &&
                           brace_depth == 0;
    if(top_level && token.is_simple(OP_LOR)) {
      return true;
    }
    if(top_level && token.is_simple(OP_LAND)) {
      const RecogToken & next = tokens.peek(pos + 1);
      if(next.is_close_angle_bracket() ||
         next.is_simple(OP_COMMA) ||
         next.is_simple(OP_DOTS)) {
        continue;
      }
      return true;
    }
  }

  return false;
}

bool template_id_candidate_is_nested_name_qualifier(
    const IRecogTokenSequence & tokens,
    std::size_t boundary,
    const NameLookup & lookup,
    ParseHeuristicCache * cache)
{
  int angle_depth = 1;
  int paren_depth = 0;
  int bracket_depth = 0;
  int brace_depth = 0;

  for(std::size_t pos = boundary + 1; !tokens.peek(pos).is_eof(); ++pos) {
    const RecogToken & token = tokens.peek(pos);
    const bool track_angles =
        paren_depth == 0 && bracket_depth == 0 && brace_depth == 0;

    if(track_angles && token.is_simple(OP_LT) &&
       pos > boundary + 1 &&
       can_open_nested_template_angle_at(tokens, pos, lookup, cache)) {
      ++angle_depth;
      continue;
    }
    if(track_angles && token.is_close_angle_bracket()) {
      if(angle_depth == 1) {
        return tokens.peek(pos + 1).is_simple(OP_COLON2);
      }
      if(angle_depth > 1) {
        --angle_depth;
      }
      continue;
    }

    if(token.is_simple(OP_LPAREN)) {
      ++paren_depth;
      continue;
    }
    if(token.is_simple(OP_RPAREN)) {
      if(paren_depth == 0) {
        return false;
      }
      --paren_depth;
      continue;
    }
    if(token.is_simple(OP_LSQUARE)) {
      ++bracket_depth;
      continue;
    }
    if(token.is_simple(OP_RSQUARE)) {
      if(bracket_depth == 0) {
        return false;
      }
      --bracket_depth;
      continue;
    }
    if(token.is_simple(OP_LBRACE)) {
      ++brace_depth;
      continue;
    }
    if(token.is_simple(OP_RBRACE)) {
      if(brace_depth == 0) {
        return false;
      }
      --brace_depth;
      continue;
    }

    if(track_angles && token.is_simple(OP_SEMICOLON)) {
      return false;
    }
  }

  return false;
}

inline void append_token_text_for_span(std::string & out, const RecogToken & token)
{
  if(token.is_rshift_piece()) {
    out += '>';
    return;
  }
  out += token.source;
}

inline std::string single_token_text_for_span(const RecogToken & token)
{
  if(token.is_rshift_piece()) {
    return ">";
  }
  return token.source;
}

bool token_text_needs_separator(const RecogToken & lhs,
                                const RecogToken & rhs)
{
  const auto is_word_like = [](const RecogToken & token) -> bool
  {
    if(token.kind == RT_IDENTIFIER || token.kind == RT_LITERAL) {
      return true;
    }

    if(token.kind != RT_SIMPLE || token.source.empty()) {
      return false;
    }

    const char c = token.source[0];
    return (c >= 'a' && c <= 'z') ||
           (c >= 'A' && c <= 'Z') ||
           (c >= '0' && c <= '9') ||
           c == '_';
  };

  return is_word_like(lhs) && is_word_like(rhs);
}

bool token_can_precede_nested_template_angle(const RecogToken & token)
{
  if(token.is_identifier() ||
     token.is_close_angle_bracket() ||
     token.is_simple(OP_COLON2) ||
     token.is_simple(KW_TEMPLATE) ||
     token.is_simple(KW_OPERATOR) ||
     token.is_simple(KW_STATIC_CAST) ||
     token.is_simple(KW_CONST_CAST) ||
     token.is_simple(KW_DYNAMIC_CAST) ||
     token.is_simple(KW_REINTERPET_CAST)) {
    return true;
  }

  if(token.kind != RT_SIMPLE) {
    return false;
  }

  switch(token.simple_type) {
  case OP_PLUS:      case OP_MINUS:     case OP_STAR:       case OP_DIV:
  case OP_MOD:       case OP_XOR:       case OP_AMP:        case OP_BOR:
  case OP_COMPL:     case OP_LNOT:      case OP_ASS:        case OP_LT:
  case OP_GT:        case OP_PLUSASS:   case OP_MINUSASS:   case OP_STARASS:
  case OP_DIVASS:    case OP_MODASS:    case OP_XORASS:     case OP_BANDASS:
  case OP_BORASS:    case OP_LSHIFT:    case OP_RSHIFTASS:  case OP_LSHIFTASS:
  case OP_EQ:        case OP_NE:        case OP_LE:         case OP_GE:
  case OP_LAND:      case OP_LOR:       case OP_INC:        case OP_DEC:
  case OP_COMMA:     case OP_ARROWSTAR: case OP_ARROW:      case OP_DOTSTAR:
    return true;
  default:
    return false;
  }
}

bool implicit_template_id_has_dependent_type_qualifier(
    const IRecogTokenSequence & tokens,
    std::size_t boundary,
    const NameLookup & lookup,
    ParseHeuristicCache * cache,
    const RecogToken ** owner_head)
{
  if(owner_head) {
    *owner_head = nullptr;
  }
  if(boundary < 3 || !tokens.peek(boundary - 2).is_simple(OP_COLON2)) {
    return false;
  }

  const RecogToken & qualifier = tokens.peek(boundary - 3);
  if(qualifier.is_identifier()) {
    const bool dependent =
        lookup.is_template_type_parameter_identifier(qualifier);
    if(dependent && owner_head) {
      *owner_head = &qualifier;
    }
    return dependent;
  }
  if(!qualifier.is_close_angle_bracket()) {
    return false;
  }

  const std::size_t qualifier_end = boundary - 2;
  for(std::size_t cursor = boundary - 3; cursor > 0; --cursor) {
    const RecogToken & token = tokens.peek(cursor);
    if(token.is_simple(OP_SEMICOLON) ||
       token.is_simple(OP_LBRACE) ||
       token.is_simple(OP_RBRACE)) {
      return false;
    }
    if(!token.is_simple(OP_LT)) {
      continue;
    }

    const RecogToken & head = tokens.peek(cursor - 1);
    if(!head.is_identifier() ||
       (!lookup.is_known_template_name_identifier(head) &&
        !lookup.is_template_type_parameter_identifier(head))) {
      continue;
    }

    std::size_t parsed_end = cursor;
    std::vector<std::pair<std::size_t, std::size_t> > argument_ranges;
    if(!parse_template_id_suffix_ranges(tokens,
                                        cursor,
                                        lookup,
                                        parsed_end,
                                        argument_ranges,
                                        cache) ||
       parsed_end != qualifier_end) {
      continue;
    }
    if(lookup.is_template_type_parameter_identifier(head)) {
      if(owner_head) {
        *owner_head = &head;
      }
      return true;
    }
    for(std::size_t range_index = 0;
        range_index < argument_ranges.size();
        ++range_index) {
      for(std::size_t argument_pos = argument_ranges[range_index].first;
          argument_pos < argument_ranges[range_index].second;
          ++argument_pos) {
        const RecogToken & argument_token = tokens.peek(argument_pos);
        if(argument_token.is_identifier() &&
          (lookup.is_template_type_parameter_identifier(argument_token) ||
            lookup.is_known_value_template_parameter_identifier(argument_token))) {
          if(owner_head) {
            *owner_head = &head;
          }
          return true;
        }
      }
    }
    return false;
  }
  return false;
}

bool token_can_follow_unknown_nested_template_id(const RecogToken & token)
{
  if(token.is_eof() || token.is_close_angle_bracket()) {
    return true;
  }

  if(is_cv_qualifier(token)) {
    return true;
  }

  if(token.kind != RT_SIMPLE) {
    return false;
  }

  switch(token.simple_type) {
  case OP_COLON2:
  case OP_COMMA:
  case OP_STAR:
  case OP_AMP:
  case OP_LAND:
  case OP_DOTS:
  case OP_LSQUARE:
  case OP_LPAREN:
    return true;
  default:
    return false;
  }
}

template<class BuildFn>
void note_trace_if_enabled(const char * category,
                           const IRecogTokenSequence & tokens,
                           std::size_t pos,
                           BuildFn build)
{
  if(!parser_trace::enabled(category)) {
    return;
  }
  std::ostringstream trace;
  build(trace);
  parser_trace::note(category, tokens, pos, trace.str());
}

void note_trace_message_if_enabled(const char * category,
                                   const IRecogTokenSequence & tokens,
                                   std::size_t pos,
                                   const char * message)
{
  if(!parser_trace::enabled(category)) {
    return;
  }
  parser_trace::note(category, tokens, pos, message);
}

}  // namespace

std::string token_span_text_spaced(const IRecogTokenSequence & tokens,
                                   std::size_t start,
                                   std::size_t end)
{
  if(end <= start) {
    return std::string();
  }

  if(end == start + 1) {
    return single_token_text_for_span(tokens.peek(start));
  }

  if(end == start + 2) {
    const RecogToken & first = tokens.peek(start);
    const RecogToken & second = tokens.peek(start + 1);
    std::string out;
    out.reserve(first.source.size() + second.source.size() + 1);
    append_token_text_for_span(out, first);
    if(token_text_needs_separator(first, second)) {
      out += ' ';
    }
    append_token_text_for_span(out, second);
    return out;
  }

  std::string out;
  out.reserve((end - start) * 8);
  const RecogToken * prev = &tokens.peek(start);
  append_token_text_for_span(out, *prev);
  for(std::size_t i = start + 1; i < end; ++i) {
    const RecogToken & current = tokens.peek(i);
    if(token_text_needs_separator(*prev, current)) {
      out += ' ';
    }
    append_token_text_for_span(out, current);
    prev = &current;
  }
  return out;
}

bool looks_like_unknown_nested_template_id_at_impl(
    const IRecogTokenSequence & tokens,
    std::size_t boundary,
    const NameLookup & lookup,
    ParseHeuristicCache * cache);

bool can_open_nested_template_angle_at(const IRecogTokenSequence & tokens,
                                       std::size_t boundary,
                                       const NameLookup & lookup,
                                       ParseHeuristicCache * cache)
{
  if(cache == nullptr) {
    ParseHeuristicCache local_cache;
    return can_open_nested_template_angle_at(tokens, boundary, lookup, &local_cache);
  }

  bool cached_value = false;
  if(cache != nullptr &&
     lookup_cached_heuristic(&cache->can_open_nested_template_angle,
                             boundary,
                             cached_value)) {
    return cached_value;
  }
  if(cache != nullptr) {
    mark_cached_heuristic_in_progress(&cache->can_open_nested_template_angle, boundary);
  }

  bool result = false;
  if(boundary == 0) {
    result = false;
    store_cached_heuristic(cache ? &cache->can_open_nested_template_angle : nullptr,
                           boundary,
                           result);
    return result;
  }

  const RecogToken & prev = tokens.peek(boundary - 1);
  if(prev.is_identifier()) {
    const bool known_type = lookup.is_known_type_name_identifier(prev);
    const bool known_template = lookup.is_known_template_name_identifier(prev);
    const bool known_value_template =
        lookup.is_known_value_template_parameter_identifier(prev);
    const bool known_value = lookup.is_known_value_name_identifier(prev);
    const bool explicit_template_prefix =
        boundary >= 2 && tokens.peek(boundary - 2).is_simple(KW_TEMPLATE);
    const bool member_access_or_qualified_prefix =
        boundary >= 2 &&
        (tokens.peek(boundary - 2).is_simple(OP_COLON2) ||
         tokens.peek(boundary - 2).is_simple(OP_DOT) ||
         tokens.peek(boundary - 2).is_simple(OP_ARROW));
    const RecogToken * dependent_owner_head = nullptr;
    const bool dependent_type_qualified_prefix =
        implicit_template_id_has_dependent_type_qualifier(tokens,
                                                         boundary,
                                                         lookup,
                                                         cache,
                                                         &dependent_owner_head);
    const bool known_dependent_member_template =
        dependent_type_qualified_prefix && dependent_owner_head &&
        lookup.is_known_member_template_identifier(*dependent_owner_head, prev);
    const bool value_name_preferred =
        !member_access_or_qualified_prefix &&
        lookup.unqualified_identifier_prefers_value_name(prev);
    const bool candidate_is_nested_name_qualifier =
        (known_type || known_template) &&
        (known_value_template || known_value) &&
        template_id_candidate_is_nested_name_qualifier(tokens,
                                                       boundary,
                                                       lookup,
                                                       cache);
    bool unknown_nested = false;

    if(explicit_template_prefix) {
      result = true;
    } else if(candidate_is_nested_name_qualifier) {
      result = true;
    } else if(known_dependent_member_template) {
      result = true;
    } else if(dependent_type_qualified_prefix) {
      result = false;
    } else if(value_name_preferred) {
      if(known_value) {
        result = false;
      } else if(known_value_template) {
        result = !template_id_candidate_crosses_logical_operator(tokens, boundary + 1);
      } else {
        unknown_nested =
            looks_like_unknown_nested_template_id_at_impl(tokens, boundary, lookup, cache);
        result = unknown_nested &&
                 !template_id_candidate_crosses_logical_operator(tokens, boundary + 1);
      }
    } else if((known_value_template || known_value) &&
              member_access_or_qualified_prefix &&
              !known_type &&
              !known_template) {
      if(lookup.prefer_template_id_for_unknown_identifiers()) {
        result = !template_id_candidate_crosses_logical_operator(tokens,
                                                                 boundary + 1);
      } else {
        unknown_nested =
            looks_like_unknown_nested_template_id_at_impl(tokens, boundary, lookup, cache);
        result = unknown_nested;
      }
    } else if(known_value_template) {
      result = !template_id_candidate_crosses_logical_operator(tokens, boundary + 1);
    } else if(known_type || known_template) {
      result = true;
    } else if(known_value) {
      result = false;
    } else if(lookup.prefer_template_id_for_unknown_identifiers()) {
      result = true;
    } else {
      unknown_nested =
          looks_like_unknown_nested_template_id_at_impl(tokens, boundary, lookup, cache);
      if(unknown_nested) {
        result = true;
      } else {
        result = boundary >= 2 &&
                 tokens.peek(boundary - 2).is_simple(KW_TEMPLATE);
      }
    }

    note_trace_if_enabled("parser.angle", tokens, boundary,
                          [&](std::ostringstream & trace)
                          {
                            trace << "open-angle prev=" << prev.source
                                  << " known_type=" << (known_type ? "yes" : "no")
                                  << " known_template=" << (known_template ? "yes" : "no")
                                  << " known_value_template="
                                  << (known_value_template ? "yes" : "no")
                                  << " known_value=" << (known_value ? "yes" : "no")
                                  << " value_preferred="
                                  << (value_name_preferred ? "yes" : "no")
                                  << " qualifier_suffix="
                                  << (candidate_is_nested_name_qualifier ? "yes" : "no")
                                  << " dependent_qualifier="
                                  << (dependent_type_qualified_prefix ? "yes" : "no")
                                  << " known_dependent_member_template="
                                  << (known_dependent_member_template ? "yes" : "no")
                                  << " prefer_unknown="
                                  << (lookup.prefer_template_id_for_unknown_identifiers()
                                          ? "yes" :
                                          "no")
                                  << " unknown_nested="
                                  << (unknown_nested ? "yes" : "no")
                                  << " result=" << (result ? "template" : "comparison");
                          });
    store_cached_heuristic(cache ? &cache->can_open_nested_template_angle : nullptr,
                           boundary,
                           result);
    return result;
  }

  result = token_can_precede_nested_template_angle(prev);
  note_trace_if_enabled("parser.angle", tokens, boundary,
                        [&](std::ostringstream & trace)
                        {
                          trace << "open-angle prev=" << prev.source
                                << " identifier=no"
                                << " result=" << (result ? "template" : "comparison");
                        });
  store_cached_heuristic(cache ? &cache->can_open_nested_template_angle : nullptr,
                         boundary,
                         result);
  return result;
}

bool looks_like_unknown_nested_template_id_at_impl(const IRecogTokenSequence & tokens,
                                                   std::size_t boundary,
                                                   const NameLookup & lookup,
                                                   ParseHeuristicCache * cache)
{
  bool cached_value = false;
  if(cache != nullptr &&
     lookup_cached_heuristic(&cache->looks_like_unknown_nested_template_id,
                             boundary,
                             cached_value)) {
    return cached_value;
  }
  if(cache != nullptr) {
    mark_cached_heuristic_in_progress(&cache->looks_like_unknown_nested_template_id,
                                      boundary);
  }

  if(boundary == 0 || !tokens.peek(boundary).is_simple(OP_LT)) {
    store_cached_heuristic(cache ? &cache->looks_like_unknown_nested_template_id : nullptr,
                           boundary,
                           false);
    return false;
  }

  int angle_depth = 1;
  int paren_depth = 0;
  int bracket_depth = 0;
  int brace_depth = 0;
  bool saw_top_level_comma = false;

  for(std::size_t i = boundary + 1; !tokens.peek(i).is_eof(); ++i) {
    const RecogToken & token = tokens.peek(i);
    const bool track_angles =
        paren_depth == 0 && bracket_depth == 0 && brace_depth == 0;

    if(track_angles && token.is_simple(OP_LT) &&
       can_open_nested_template_angle_at(tokens, i, lookup, cache)) {
      ++angle_depth;
      continue;
    }

    if(track_angles && token.is_close_angle_bracket()) {
      --angle_depth;
      if(angle_depth == 0) {
        const RecogToken & after = tokens.peek(i + 1);
        const bool result =
            saw_top_level_comma ||
            token_can_follow_unknown_nested_template_id(after);
        note_trace_if_enabled("parser.angle", tokens, boundary,
                              [&](std::ostringstream & trace)
                              {
                                trace << "unknown-nested boundary=" << boundary
                                      << " result=" << (result ? "yes" : "no");
                              });
        store_cached_heuristic(cache ? &cache->looks_like_unknown_nested_template_id :
                                       nullptr,
                               boundary,
                               result);
        return result;
      }
      continue;
    }

    if(token.is_simple(OP_LPAREN)) {
      ++paren_depth;
      continue;
    }
    if(token.is_simple(OP_RPAREN)) {
      if(paren_depth == 0) {
        note_trace_message_if_enabled("parser.angle",
                                      tokens,
                                      boundary,
                                      "unknown-nested boundary failed on unmatched ')'");
        store_cached_heuristic(cache ? &cache->looks_like_unknown_nested_template_id :
                                       nullptr,
                               boundary,
                               false);
        return false;
      }
      --paren_depth;
      continue;
    }
    if(token.is_simple(OP_LSQUARE)) {
      ++bracket_depth;
      continue;
    }
    if(token.is_simple(OP_RSQUARE)) {
      if(bracket_depth == 0) {
        note_trace_message_if_enabled("parser.angle",
                                      tokens,
                                      boundary,
                                      "unknown-nested boundary failed on unmatched ']'");
        store_cached_heuristic(cache ? &cache->looks_like_unknown_nested_template_id :
                                       nullptr,
                               boundary,
                               false);
        return false;
      }
      --bracket_depth;
      continue;
    }
    if(token.is_simple(OP_LBRACE)) {
      ++brace_depth;
      continue;
    }
    if(token.is_simple(OP_RBRACE) || token.is_simple(OP_SEMICOLON)) {
      note_trace_message_if_enabled("parser.angle",
                                    tokens,
                                    boundary,
                                    "unknown-nested boundary failed on '}' or ';'");
      store_cached_heuristic(cache ? &cache->looks_like_unknown_nested_template_id :
                                     nullptr,
                             boundary,
                             false);
      return false;
    }
    if(token.is_simple(OP_COMMA) && track_angles && angle_depth == 1) {
      saw_top_level_comma = true;
    }
  }

  note_trace_message_if_enabled("parser.angle",
                                tokens,
                                boundary,
                                "unknown-nested boundary reached eof without match");
  store_cached_heuristic(cache ? &cache->looks_like_unknown_nested_template_id : nullptr,
                         boundary,
                         false);
  return false;
}

bool collect_template_argument_delimiters(const IRecogTokenSequence & tokens,
                                          std::size_t start,
                                          const NameLookup & lookup,
                                          std::vector<Delimiter> & out,
                                          ParseHeuristicCache * cache)
{
  if(cache == nullptr) {
    ParseHeuristicCache local_cache;
    return collect_template_argument_delimiters(tokens, start, lookup, out, &local_cache);
  }

  out.clear();

  int angle_depth = 1;
  int paren_depth = 0;
  int bracket_depth = 0;
  int brace_depth = 0;

  for(std::size_t pos = start; !tokens.peek(pos).is_eof(); ++pos) {
    const RecogToken & token = tokens.peek(pos);
    const bool track_angles =
        paren_depth == 0 && bracket_depth == 0 && brace_depth == 0;

    if(track_angles && token.is_simple(OP_LT) &&
       pos > start &&
       can_open_nested_template_angle_at(tokens, pos, lookup, cache)) {
      ++angle_depth;
    } else if(track_angles && token.is_close_angle_bracket()) {
      if(angle_depth == 1) {
        Delimiter delimiter;
        delimiter.pos = pos;
        delimiter.kind = DK_CLOSE_ANGLE;
        out.push_back(delimiter);
        return true;
      }
      if(angle_depth > 1) {
        --angle_depth;
      }
    } else if(token.is_simple(OP_LPAREN)) {
      ++paren_depth;
    } else if(token.is_simple(OP_RPAREN)) {
      if(paren_depth == 0) {
        return false;
      }
      --paren_depth;
    } else if(token.is_simple(OP_LSQUARE)) {
      ++bracket_depth;
    } else if(token.is_simple(OP_RSQUARE)) {
      if(bracket_depth == 0) {
        return false;
      }
      --bracket_depth;
    } else if(token.is_simple(OP_LBRACE)) {
      ++brace_depth;
    } else if(token.is_simple(OP_RBRACE)) {
      if(brace_depth == 0) {
        return false;
      }
      --brace_depth;
    } else if(token.is_simple(OP_SEMICOLON) && track_angles) {
      return false;
    }

    const bool top_level = angle_depth == 1 &&
                           paren_depth == 0 &&
                           bracket_depth == 0 &&
                           brace_depth == 0;
    if(top_level && token.is_simple(OP_COMMA)) {
      Delimiter delimiter;
      delimiter.pos = pos;
      delimiter.kind = DK_COMMA;
      out.push_back(delimiter);
    }
  }

  return false;
}

bool parse_template_id_suffix_ranges(
    const IRecogTokenSequence & tokens,
    std::size_t start,
    const NameLookup & lookup,
    std::size_t & end,
    std::vector<std::pair<std::size_t, std::size_t> > & arg_ranges,
    ParseHeuristicCache * cache)
{
  if(cache == nullptr) {
    ParseHeuristicCache local_cache;
    return parse_template_id_suffix_ranges(tokens,
                                           start,
                                           lookup,
                                           end,
                                           arg_ranges,
                                           &local_cache);
  }

  end = start;
  arg_ranges.clear();

  if(!tokens.peek(start).is_simple(OP_LT)) {
    return false;
  }
  if(!can_open_nested_template_angle_at(tokens, start, lookup, cache)) {
    return false;
  }

  std::size_t close = start + 1;
  if(tokens.peek(close).is_close_angle_bracket()) {
    ++close;
    end = close;
    return true;
  }

  std::vector<Delimiter> delimiters;
  delimiters.reserve(8);
  if(!collect_template_argument_delimiters(tokens,
                                          start + 1,
                                          lookup,
                                          delimiters,
                                          cache)) {
    note_trace_message_if_enabled("parser.fragment",
                                  tokens,
                                  start,
                                  "template-id suffix scan failed");
    return false;
  }
  if(delimiters.empty() || delimiters.back().kind != DK_CLOSE_ANGLE) {
    return false;
  }

  std::size_t arg_start = start + 1;
  for(std::size_t i = 0; i < delimiters.size(); ++i) {
    const Delimiter & delimiter = delimiters[i];
    if(delimiter.pos <= arg_start) {
      return false;
    }
    arg_ranges.push_back(std::make_pair(arg_start, delimiter.pos));
    arg_start = delimiter.pos + 1;
  }

  close = delimiters.back().pos + 1;
  if(!is_template_id_follower(tokens.peek(close))) {
    note_trace_if_enabled("parser.fragment", tokens, start,
                          [&](std::ostringstream & trace)
                          {
                            trace << "template-id suffix rejected by follower "
                                  << describe_recog_token(tokens.peek(close))
                                  << " text={"
                                  << token_span_text_spaced(tokens, start, close + 1)
                                  << "}";
                          });
    return false;
  }

  end = close;
  note_trace_if_enabled("parser.fragment", tokens, start,
                        [&](std::ostringstream & trace)
                        {
                          trace << "template-id suffix arg-count=" << arg_ranges.size()
                                << " text={"
                                << token_span_text_spaced(tokens, start, end)
                                << "}";
                        });
  return true;
}

}  // namespace template_angle
