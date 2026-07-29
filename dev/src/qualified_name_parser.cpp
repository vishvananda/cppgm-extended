#include "qualified_name_parser.h"

#include <stdexcept>

#include "cpp_syntax.h"
#include "recog_token_cursor.h"
#include "template_angle_lookup.h"

using namespace std;

namespace qualified_name_parser {

namespace {

bool is_typeof_specifier_token(const RecogToken & token)
{
  return token.is_identifier() &&
         (token.source == "__typeof" || token.source == "__typeof__");
}

bool is_decltype_specifier_token(const RecogToken & token)
{
  return token.is_simple(KW_DECLTYPE) ||
         (token.is_identifier() &&
          (token.source == "__decltype" || token.source == "__decltype__"));
}

bool ambiguous_value_template_suffix_crosses_logical_operator(
    const IRecogTokenSequence & tokens,
    std::size_t start,
    const NameLookup & lookup)
{
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
       template_angle::can_open_nested_template_angle_at(tokens, pos, lookup)) {
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

struct QualifiedFinalComponentLookup : NameLookup
{
  // Qualified lookup may override the component head, but not names in its
  // arguments.
  QualifiedFinalComponentLookup(const NameLookup & inner,
                                const RecogToken * component_head) :
    inner(inner),
    component_head(component_head)
  {}

  bool is_component_head(const RecogToken & token) const
  {
    return component_head == &token;
  }

  virtual bool is_known_template_name_identifier(const RecogToken & token) const
  {
    return inner.is_known_template_name_identifier(token);
  }

  virtual bool is_known_type_name_identifier(const RecogToken & token) const
  {
    return inner.is_known_type_name_identifier(token);
  }

  virtual bool is_known_value_template_parameter_identifier(
      const RecogToken & token) const
  {
    return !is_component_head(token) &&
           inner.is_known_value_template_parameter_identifier(token);
  }

  virtual bool is_known_value_name_identifier(const RecogToken & token) const
  {
    return !is_component_head(token) &&
           inner.is_known_value_name_identifier(token);
  }

  virtual bool is_template_type_parameter_identifier(
      const RecogToken & token) const
  {
    return inner.is_template_type_parameter_identifier(token);
  }

  virtual bool is_known_member_template_identifier(
      const RecogToken & owner,
      const RecogToken & member) const
  {
    return inner.is_known_member_template_identifier(owner, member);
  }

  virtual bool unqualified_identifier_prefers_value_name(
      const RecogToken & token) const
  {
    return !is_component_head(token) &&
           inner.unqualified_identifier_prefers_value_name(token);
  }

  virtual bool prefer_template_id_for_unknown_identifiers() const
  {
    return inner.prefer_template_id_for_unknown_identifiers();
  }

  const NameLookup & inner;
  const RecogToken * component_head;
};

struct QualifiedNameCursor : RecogTokenCursor
{
  QualifiedNameCursor(IRecogTokenSequence & tokens,
                      const NameLookup & lookup,
                      size_t start = 0)
    : RecogTokenCursor(tokens),
      lookup(lookup)
  {
    pos = start;
  }

  bool parse_name_component(
      NameComponentParseResult & out,
      bool suppress_unforced_template_id_crossing_logical_operator = false,
      bool allow_value_template_id = false,
      bool require_value_template_id_qualifier = false)
  {
    const size_t start = pos;
    const bool forced_template = consume_simple(KW_TEMPLATE);

    const size_t identifier_start = pos;
    if(!consume_identifier()) {
      pos = start;
      return false;
    }
    const bool explicit_template =
        forced_template ||
        (identifier_start > 0 &&
         tokens.peek(identifier_start - 1).is_simple(KW_TEMPLATE));
    const bool qualified_component =
        (identifier_start > 0 &&
         tokens.peek(identifier_start - 1).is_simple(OP_COLON2)) ||
        (identifier_start >= 2 &&
         tokens.peek(identifier_start - 1).is_simple(KW_TEMPLATE) &&
         tokens.peek(identifier_start - 2).is_simple(OP_COLON2));

    out = NameComponentParseResult();
    out.name_component = make_pair(identifier_start, pos);
    out.end = pos;
    if(peek().is_simple(OP_LT)) {
      size_t suffix_end = pos;
      std::vector<std::pair<size_t, size_t> > arg_ranges;
      bool parsed_suffix = false;
      const RecogToken & prev = tokens.peek(pos - 1);
      QualifiedFinalComponentLookup value_template_lookup(lookup, &prev);
      const bool allow_qualified_value_template_id =
          allow_value_template_id || qualified_component;
      const NameLookup & template_suffix_lookup =
          allow_qualified_value_template_id ?
              static_cast<const NameLookup &>(value_template_lookup) :
              lookup;
      if(prev.is_identifier()) {
        const bool known_template = lookup.is_known_template_name_identifier(prev);
        const bool known_type = lookup.is_known_type_name_identifier(prev);
        const bool known_value_template =
            lookup.is_known_value_template_parameter_identifier(prev);
        const bool known_value = lookup.is_known_value_name_identifier(prev);
        if((known_value_template || known_value) &&
           !known_template && !known_type && !explicit_template) {
          if(!allow_qualified_value_template_id) {
            return true;
          }
          parsed_suffix =
              template_angle::parse_template_id_suffix_ranges(tokens,
                                                              pos,
                                                              template_suffix_lookup,
                                                              suffix_end,
                                                              arg_ranges);
          if(!parsed_suffix ||
             (require_value_template_id_qualifier &&
              !tokens.peek(suffix_end).is_simple(OP_COLON2))) {
            return true;
          }
        }
        const bool crosses_logical_operator =
            !explicit_template &&
            ambiguous_value_template_suffix_crosses_logical_operator(tokens,
                                                                     pos + 1,
                                                                     lookup);
        if(crosses_logical_operator &&
           (suppress_unforced_template_id_crossing_logical_operator ||
            known_value_template || known_value)) {
          return true;
        }
      }

      if(!parsed_suffix) {
        parsed_suffix =
            template_angle::parse_template_id_suffix_ranges(tokens,
                                                            pos,
                                                            template_suffix_lookup,
                                                            suffix_end,
                                                            arg_ranges);
      }
      if(parsed_suffix &&
         qualified_component &&
         !explicit_template &&
         !allow_value_template_id) {
        const RecogToken & after_suffix = tokens.peek(suffix_end);
        const bool known_template_suffix =
            prev.is_identifier() &&
            (lookup.is_known_template_name_identifier(prev) ||
             lookup.is_known_type_name_identifier(prev) ||
             lookup.is_known_value_template_parameter_identifier(prev));
        if(!known_template_suffix &&
           !after_suffix.is_simple(OP_COLON2) &&
           !after_suffix.is_simple(OP_LPAREN) &&
           !after_suffix.is_simple(OP_LSQUARE)) {
          parsed_suffix = false;
        }
      }
      if(parsed_suffix) {
        pos = suffix_end;
        out.end = pos;
        out.has_template_suffix = true;
        out.template_arg_ranges = arg_ranges;
      }
    }

    return true;
  }

  bool parse_decltype_specifier(size_t & end)
  {
    const size_t start = pos;
    if(!is_decltype_specifier_token(peek())) {
      if(!is_typeof_specifier_token(peek())) {
        pos = start;
        return false;
      }
      ++pos;
    } else {
      ++pos;
    }
    if(!parse_balanced_clause(OP_LPAREN, OP_RPAREN)) {
      pos = start;
      return false;
    }

    end = pos;
    return true;
  }

  bool parse_conversion_operator_type_name(size_t & end,
                                           bool allow_without_call)
  {
    const size_t start = pos;
    size_t cursor = pos;
    bool saw_tokens = false;
    int angle_depth = 0;
    int paren_depth = 0;
    int bracket_depth = 0;
    int brace_depth = 0;

    const auto restore_and_fail = [&]() -> bool
    {
      pos = start;
      return false;
    };

    const auto parse_decltype_or_typeof_at = [&](size_t at, size_t & specifier_end) -> bool
    {
      if(!is_decltype_specifier_token(tokens.peek(at)) &&
         !is_typeof_specifier_token(tokens.peek(at))) {
        return false;
      }
      const size_t saved = pos;
      pos = at;
      const bool parsed = parse_decltype_specifier(specifier_end);
      pos = saved;
      return parsed;
    };

    while(!tokens.peek(cursor).is_eof()) {
      const RecogToken & token = tokens.peek(cursor);

      size_t specifier_end = cursor;
      if(parse_decltype_or_typeof_at(cursor, specifier_end)) {
        cursor = specifier_end;
        saw_tokens = true;
        continue;
      }

      const bool top_level =
          angle_depth == 0 && paren_depth == 0 &&
          bracket_depth == 0 && brace_depth == 0;

      if(top_level && token.is_simple(OP_LPAREN)) {
        if(!saw_tokens) {
          return restore_and_fail();
        }
        end = cursor;
        return true;
      }

      if(top_level && token.kind == RT_SIMPLE) {
        switch(token.simple_type) {
        case OP_SEMICOLON:
          if(allow_without_call && saw_tokens) {
            end = cursor;
            return true;
          }
          return restore_and_fail();
        case OP_ASS:
        case OP_COMMA:
        case OP_COLON:
        case OP_LBRACE:
          return restore_and_fail();
        default:
          break;
        }
      }

      if(paren_depth == 0 && bracket_depth == 0 && brace_depth == 0 &&
         token.is_simple(OP_LT)) {
        ++angle_depth;
        ++cursor;
        saw_tokens = true;
        continue;
      }

      if(paren_depth == 0 && bracket_depth == 0 && brace_depth == 0 &&
         token.is_close_angle_bracket()) {
        if(angle_depth == 0) {
          return restore_and_fail();
        }
        --angle_depth;
        ++cursor;
        saw_tokens = true;
        continue;
      }

      if(token.is_simple(OP_LPAREN)) {
        ++paren_depth;
      }
      else if(token.is_simple(OP_RPAREN)) {
        if(paren_depth == 0) {
          return restore_and_fail();
        }
        --paren_depth;
      }
      else if(token.is_simple(OP_LSQUARE)) {
        ++bracket_depth;
      }
      else if(token.is_simple(OP_RSQUARE)) {
        if(bracket_depth == 0) {
          return restore_and_fail();
        }
        --bracket_depth;
      }
      else if(token.is_simple(OP_LBRACE)) {
        ++brace_depth;
      }
      else if(token.is_simple(OP_RBRACE)) {
        if(brace_depth == 0) {
          return restore_and_fail();
        }
        --brace_depth;
      }

      ++cursor;
      saw_tokens = true;
    }

    return restore_and_fail();
  }

  bool parse_operator_name(size_t & end,
                           bool & is_conversion,
                           bool allow_conversion_operator_type_without_call)
  {
    const size_t start = pos;
    if(!consume_simple(KW_OPERATOR)) {
      pos = start;
      return false;
    }
    is_conversion = false;

    if(peek().is_empty_string()) {
      ++pos;
      if(!consume_identifier()) {
        pos = start;
        return false;
      }
      end = pos;
      return true;
    }

    if(consume_simple(KW_NEW) || consume_simple(KW_DELETE)) {
      if(consume_simple(OP_LSQUARE) && !consume_simple(OP_RSQUARE)) {
        pos = start;
        return false;
      }
      end = pos;
      return true;
    }

    if(consume_simple(OP_LPAREN)) {
      if(!consume_simple(OP_RPAREN)) {
        pos = start;
        return false;
      }
      end = pos;
      return true;
    }

    if(consume_simple(OP_LSQUARE)) {
      if(!consume_simple(OP_RSQUARE)) {
        pos = start;
        return false;
      }
      end = pos;
      return true;
    }

    if(peek().is_rshift_piece()) {
      ++pos;
      if(!peek().is_rshift_piece()) {
        pos = start;
        return false;
      }
      ++pos;
      end = pos;
      return true;
    }

    switch(peek().kind == RT_SIMPLE ? peek().simple_type : (ETokenType)-1) {
    case OP_PLUS:      case OP_MINUS:     case OP_STAR:       case OP_DIV:
    case OP_MOD:       case OP_XOR:       case OP_AMP:        case OP_BOR:
    case OP_COMPL:     case OP_LNOT:      case OP_ASS:        case OP_LT:
    case OP_GT:        case OP_PLUSASS:   case OP_MINUSASS:   case OP_STARASS:
    case OP_DIVASS:    case OP_MODASS:    case OP_XORASS:     case OP_BANDASS:
    case OP_BORASS:    case OP_LSHIFT:    case OP_RSHIFTASS:  case OP_LSHIFTASS:
    case OP_EQ:        case OP_NE:        case OP_LE:         case OP_GE:
    case OP_LAND:      case OP_LOR:       case OP_INC:        case OP_DEC:
    case OP_COMMA:     case OP_ARROWSTAR: case OP_ARROW:      case OP_DOTSTAR:
      ++pos;
      end = pos;
      return true;
    default:
      break;
    }

    size_t conversion_end = pos;
    if(!parse_conversion_operator_type_name(
           conversion_end,
           allow_conversion_operator_type_without_call)) {
      pos = start;
      return false;
    }

    pos = conversion_end;
    end = pos;
    is_conversion = true;
    return true;
  }

  bool parse_unqualified_name(const UnqualifiedNameOptions & options,
                              UnqualifiedNameParseResult & out)
  {
    const size_t start = pos;

    if(options.allow_destructor && consume_simple(OP_COMPL)) {
      NameComponentParseResult component;
      if(!parse_name_component(component)) {
        pos = start;
        return false;
      }
      out = UnqualifiedNameParseResult();
      out.end = pos;
      out.kind = UNQ_DESTRUCTOR;
      out.name_component = component.name_component;
      out.has_template_suffix = component.has_template_suffix;
      out.template_arg_ranges = component.template_arg_ranges;
      return true;
    }

    if(options.allow_operator) {
      size_t operator_end = pos;
      bool operator_is_conversion = false;
      const size_t operator_start = pos;
      if(parse_operator_name(
             operator_end,
             operator_is_conversion,
             options.allow_conversion_operator_type_without_call)) {
        out = UnqualifiedNameParseResult();
        out.end = operator_end;
        out.name_component = make_pair(operator_start, operator_end);
        out.kind = UNQ_OPERATOR;
        out.operator_is_conversion = operator_is_conversion;
        if(options.allow_template_id && peek().is_simple(OP_LT)) {
          size_t suffix_end = pos;
          std::vector<std::pair<size_t, size_t> > arg_ranges;
          if(template_angle::parse_template_id_suffix_ranges(tokens,
                                                             pos,
                                                             lookup,
                                                             suffix_end,
                                                             arg_ranges)) {
            pos = suffix_end;
            out.end = suffix_end;
            out.has_template_suffix = true;
            out.template_arg_ranges = arg_ranges;
          }
        }
        return true;
      }
    }

    NameComponentParseResult component;
    if(!parse_name_component(
           component,
           options.suppress_unforced_template_id_crossing_logical_operator,
           options.allow_value_template_id_final_component)) {
      pos = start;
      return false;
    }

    if(!options.allow_template_id && component.has_template_suffix) {
      pos = start;
      return false;
    }

    out = UnqualifiedNameParseResult();
    out.end = component.end;
    out.name_component = component.name_component;
    out.kind = UNQ_COMPONENT;
    out.has_template_suffix = component.has_template_suffix;
    out.template_arg_ranges = component.template_arg_ranges;
    return true;
  }

  bool parse_qualified_name(const UnqualifiedNameOptions & options,
                            QualifiedNameParseResult & out)
  {
    const size_t start = pos;
    out = QualifiedNameParseResult();
    out.rooted = consume_simple(OP_COLON2);

    while(true) {
      const size_t component_start = pos;
      NameComponentParseResult component;
      size_t decltype_end = component_start;
      bool parsed_component =
          parse_name_component(component,
                               options.suppress_unforced_template_id_crossing_logical_operator,
                               true,
                               true);
      if(!parsed_component) {
        parsed_component = parse_decltype_specifier(decltype_end);
      }
      if(!parsed_component || !consume_simple(OP_COLON2)) {
        pos = component_start;
        break;
      }

      const size_t component_end = component.end != 0 ? component.end : decltype_end;
      out.qualifiers.push_back(make_pair(component_start, component_end));
      out.qualifier_components.push_back(component);
    }

    UnqualifiedNameParseResult final_component;
    const size_t name_start = pos;
    const bool final_component_is_qualified =
        out.rooted || !out.qualifiers.empty();
    if(final_component_is_qualified) {
      size_t component_head_pos = pos;
      if(tokens.peek(component_head_pos).is_simple(KW_TEMPLATE) ||
         tokens.peek(component_head_pos).is_simple(OP_COMPL)) {
        ++component_head_pos;
      }
      const RecogToken * component_head =
          tokens.peek(component_head_pos).is_identifier() ?
              &tokens.peek(component_head_pos) : nullptr;
      QualifiedFinalComponentLookup final_lookup(lookup, component_head);
      QualifiedNameCursor final_parser(tokens, final_lookup, pos);
      if(!final_parser.parse_unqualified_name(options, final_component)) {
        pos = start;
        return false;
      }
      pos = final_parser.pos;
    } else {
      if(!parse_unqualified_name(options, final_component)) {
        pos = start;
        return false;
      }
    }

    out.name_component = make_pair(name_start, final_component.end);
    out.name_template_head_component = final_component.name_component;
    out.name_kind = final_component.kind;
    out.name_has_template_suffix = final_component.has_template_suffix;
    out.operator_is_conversion = final_component.operator_is_conversion;
    out.name_template_arg_ranges = final_component.template_arg_ranges;
    out.end = final_component.end;
    return true;
  }

  const NameLookup & lookup;
};

}  // namespace

bool parse_name_component(IRecogTokenSequence & tokens,
                          size_t start,
                          const NameLookup & lookup,
                          NameComponentParseResult & out)
{
  QualifiedNameCursor parser(tokens, lookup, start);
  if(!parser.parse_name_component(out)) {
    return false;
  }
  return true;
}

bool parse_decltype_specifier(IRecogTokenSequence & tokens,
                              size_t start,
                              size_t & end)
{
  template_angle_lookup::NameSetLookup lookup;
  QualifiedNameCursor parser(tokens, lookup, start);
  return parser.parse_decltype_specifier(end);
}

bool parse_unqualified_name(IRecogTokenSequence & tokens,
                            size_t start,
                            const NameLookup & lookup,
                            const UnqualifiedNameOptions & options,
                            UnqualifiedNameParseResult & out)
{
  QualifiedNameCursor parser(tokens, lookup, start);
  return parser.parse_unqualified_name(options, out);
}

bool parse_qualified_name(IRecogTokenSequence & tokens,
                          size_t start,
                          const NameLookup & lookup,
                          const UnqualifiedNameOptions & options,
                          QualifiedNameParseResult & out)
{
  QualifiedNameCursor parser(tokens, lookup, start);
  return parser.parse_qualified_name(options, out);
}

}  // namespace qualified_name_parser
