#pragma once

#include <map>
#include <set>
#include <string>

#include "template_witness.h"

namespace template_api {

std::string render_template_source_witness_text(
    const TemplateWitnessSession & session,
    const std::string & source_path);

std::string render_template_source_witness_debug_text(
    const TemplateWitnessSession & session,
    const std::string & source_path);

std::map<std::string, std::string> template_source_defaulted_aliases(
    const TemplateWitnessSession & session,
    const std::string & source_path);

std::set<std::string> template_source_owner_entities(
    const TemplateWitnessSession & session,
    const std::string & source_path);

std::set<std::string> template_source_explicit_owner_entities(
    const TemplateWitnessSession & session,
    const std::string & source_path);

std::set<std::string> template_source_argument_value_entities(
    const TemplateWitnessSession & session,
    const std::string & source_path);

std::set<std::string> template_source_argument_value_decl_locations(
    const TemplateWitnessSession & session,
    const std::string & source_path);

}  // namespace template_api
