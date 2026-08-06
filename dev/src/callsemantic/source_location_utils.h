#pragma once

#include <cstddef>
#include <string>
#include <utility>
#include <vector>

#include "cppast_ast.h"
#include "recog_token_buffer.h"

namespace template_api {
struct TemplateWitnessContext;
}

namespace callsemantic {

struct ParsedSourceLocation
{
  bool valid = false;
  std::string file;
  int line = 0;
  int column = 0;
};

struct QualifiedUseOccurrence
{
  std::string value;
  std::string location;
};

ParsedSourceLocation parse_source_location(const std::string & text);
ParsedSourceLocation parse_physical_source_location(const std::string & text);
std::string prefer_later_source_location(const std::string & first,
                                         const std::string & second);
bool source_location_is_later(const std::string & first,
                              const std::string & second);
std::string prefer_earlier_source_location(const std::string & first,
                                           const std::string & second);

class SourceLocationTokenView
{
public:
  SourceLocationTokenView(const SourceLocationTable * source_locations,
                          IRecogTokenSequence * token_sequence);

  std::string source_location_for_node(const CppAstNode & node) const;
  std::string source_location_for_name_in_node(const CppAstNode & node,
                                               const std::string & name,
                                               bool prefer_last = false) const;
  std::string source_location_for_name_in_subtree(const CppAstNode & node,
                                                  const std::string & name,
                                                  bool prefer_last = false) const;
  bool source_location_identifier_followed_by_indirection_type_suffix(
      const std::string & location,
      const std::string & identifier) const;
  std::string source_location_for_qualified_member_start_on_line(
      const std::string & location,
      const std::string & member_name) const;
  std::string source_location_for_token_index(std::size_t index) const;
  bool parsed_source_location_for_token_index(std::size_t index,
                                              ParsedSourceLocation & out) const;
  bool token_index_for_source_location(const std::string & location,
                                       const std::string & token_source,
                                       std::size_t & out_index) const;
  bool token_index_at_source_location(const std::string & location,
                                      std::size_t & out_index) const;
  bool find_next_token_source_on_same_line(std::size_t start_index,
                                           const std::string & token_source,
                                           std::size_t & out_index) const;
  bool tokens_are_adjacent(std::size_t left_index,
                           std::size_t right_index) const;
  std::string compact_token_range_text(std::size_t begin,
                                       std::size_t end) const;
  std::string compact_template_id_token_text(std::size_t begin,
                                             std::size_t close_index,
                                             int close_chars) const;
  bool template_name_before_open(std::size_t open_index,
                                 std::size_t & name_begin,
                                 std::size_t & anchor_index) const;
  bool template_id_at_location_is_nested(const std::string & location) const;
  bool template_id_at_location_is_qualified_member_owner(
      const std::string & location) const;
  bool template_id_at_location_is_conversion_operator_result(
      const std::string & location) const;
  bool matching_template_close_token(std::size_t open_index,
                                     std::size_t & close_index,
                                     int & close_chars,
                                     bool same_line_only = true) const;
  bool template_argument_token_ranges_from_open(
      std::size_t open_index,
      std::vector<std::pair<std::size_t, std::size_t> > & ranges) const;

private:
  static bool token_source_is_name_part(const std::string & source);
  static bool token_text_needs_separator(const std::string & left,
                                         const std::string & right);
  std::string source_location_for_qualified_member_start_on_line_in_range(
      const std::string & location,
      const std::string & member_name,
      std::size_t begin,
      std::size_t end) const;
  const SourceLocationTable * source_locations_;
  IRecogTokenSequence * token_sequence_;
};

void collect_qualified_use_occurrences(
    const CppAstNode & node,
    const SourceLocationTokenView & locations,
    const template_api::TemplateWitnessContext & witness_context,
    std::vector<QualifiedUseOccurrence> & out);

std::string earliest_qualified_use_location_for_prefix(
    const std::vector<QualifiedUseOccurrence> & occurrences,
    const std::string & prefix);

std::string earliest_qualified_use_location_for_value(
    const std::vector<QualifiedUseOccurrence> & occurrences,
    const std::string & value);

}  // namespace callsemantic
