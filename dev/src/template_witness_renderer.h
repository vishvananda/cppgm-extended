#pragma once

#include <map>
#include <set>
#include <string>

#include "template_witness.h"

namespace template_api {

struct RenderedTemplateSourceWitness
{
  std::string text;
  std::map<std::string, std::string> defaulted_aliases;
  std::set<std::string> owner_entities;
  std::set<std::string> explicit_owner_entities;
  std::set<std::string> argument_value_entities;
  std::set<std::string> argument_value_decl_locations;
};

std::string render_template_source_witness_text(
    const TemplateWitnessSession & session,
    const std::string & source_path);

std::string render_template_source_witness_debug_text(
    const TemplateWitnessSession & session,
    const std::string & source_path);

RenderedTemplateSourceWitness analyze_template_source_witness(
    const TemplateWitnessSession & session,
    const std::string & source_path,
    bool debug);

}  // namespace template_api
