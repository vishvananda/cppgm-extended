#pragma once

#include <cstddef>
#include <string>
#include <vector>

#include "semantic_model.h"
#include "template_service_interfaces.h"

namespace template_metadata {

// template-boundary-audit: begin canonical_key_metadata
inline const std::vector<std::string> * argument_texts(
    const template_api::TemplateNamedTypeMetadata & metadata)
{
  return metadata.instantiation_arg_texts.empty() ?
             nullptr :
             &metadata.instantiation_arg_texts;
}

inline const std::vector<std::string> * argument_texts(
    const semantic_model::ClassInfo & info)
{
  return info.instantiation_arg_texts.empty() ?
             nullptr :
             &info.instantiation_arg_texts;
}

inline bool argument_texts_contain_pack_expansion(
    const std::vector<std::string> & texts)
{
  for(std::size_t i = 0; i < texts.size(); ++i) {
    const std::string & text = texts[i];
    const std::size_t end = text.find_last_not_of(" \t\n\r\f\v");
    if(end != std::string::npos &&
       end + 1 >= 3 &&
       text.compare(end + 1 - 3, 3, "...") == 0) {
      return true;
    }
  }
  return false;
}

inline bool should_prefer_argument_texts(
    const template_api::TemplateNamedTypeMetadata & metadata,
    bool fully_bound,
    std::size_t argument_count)
{
  return !metadata.instantiation_arg_texts.empty() &&
         (!fully_bound ||
          metadata.instantiation_arg_texts.size() != argument_count ||
          argument_texts_contain_pack_expansion(metadata.instantiation_arg_texts));
}

inline bool has_argument_texts(
    const template_api::TemplateNamedTypeMetadata & metadata)
{
  return !metadata.instantiation_arg_texts.empty();
}

inline const std::vector<std::string> * selected_argument_texts(
    const template_api::TemplateNamedTypeMetadata & metadata,
    bool fully_bound,
    std::size_t argument_count)
{
  return should_prefer_argument_texts(
      metadata,
      fully_bound,
      argument_count) ?
          &metadata.instantiation_arg_texts :
          nullptr;
}
// template-boundary-audit: end canonical_key_metadata

}  // namespace template_metadata
