#pragma once

#include <string>
#include <vector>

namespace witness_text {

// Normalize preserved source spellings only. Semantic witness values should be
// produced from typed serializers before they reach this layer.
std::string normalize_source_spelling_text(const std::string & text);

std::string strip_inline_namespace_segments(
    const std::string & text,
    const std::vector<std::string> & inline_names);

std::string normalize_anonymous_namespace_segments(const std::string & text);

}  // namespace witness_text
