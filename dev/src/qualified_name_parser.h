#pragma once

#include <cstddef>
#include <utility>
#include <vector>

#include "template_angle_parser.h"

struct IRecogTokenSequence;

namespace qualified_name_parser {

using NameLookup = template_angle::NameLookup;

struct NameComponentParseResult
{
  std::size_t end = 0;
  std::pair<std::size_t, std::size_t> name_component;
  bool has_template_suffix = false;
  std::vector<std::pair<std::size_t, std::size_t> > template_arg_ranges;
};

enum UnqualifiedNameKind
{
  UNQ_COMPONENT,
  UNQ_DESTRUCTOR,
  UNQ_OPERATOR
};

struct UnqualifiedNameOptions
{
  bool allow_template_id = true;
  bool allow_destructor = true;
  bool allow_operator = true;
  bool allow_conversion_operator_type_without_call = false;
  bool suppress_unforced_template_id_crossing_logical_operator = false;
  bool allow_value_template_id_final_component = false;
};

struct UnqualifiedNameParseResult
{
  std::size_t end = 0;
  std::pair<std::size_t, std::size_t> name_component;
  UnqualifiedNameKind kind = UNQ_COMPONENT;
  bool has_template_suffix = false;
  bool operator_is_conversion = false;
  std::vector<std::pair<std::size_t, std::size_t> > template_arg_ranges;
};

struct QualifiedNameParseResult
{
  bool rooted = false;
  std::vector<std::pair<std::size_t, std::size_t> > qualifiers;
  std::vector<NameComponentParseResult> qualifier_components;
  std::pair<std::size_t, std::size_t> name_component;
  std::pair<std::size_t, std::size_t> name_template_head_component;
  UnqualifiedNameKind name_kind = UNQ_COMPONENT;
  bool name_has_template_suffix = false;
  bool operator_is_conversion = false;
  std::vector<std::pair<std::size_t, std::size_t> > name_template_arg_ranges;
  std::size_t end = 0;
};

bool parse_name_component(IRecogTokenSequence & tokens,
                          std::size_t start,
                          const NameLookup & lookup,
                          NameComponentParseResult & out);

bool parse_decltype_specifier(IRecogTokenSequence & tokens,
                              std::size_t start,
                              std::size_t & end);

bool parse_unqualified_name(IRecogTokenSequence & tokens,
                            std::size_t start,
                            const NameLookup & lookup,
                            const UnqualifiedNameOptions & options,
                            UnqualifiedNameParseResult & out);

bool parse_qualified_name(IRecogTokenSequence & tokens,
                          std::size_t start,
                          const NameLookup & lookup,
                          const UnqualifiedNameOptions & options,
                          QualifiedNameParseResult & out);

}  // namespace qualified_name_parser
