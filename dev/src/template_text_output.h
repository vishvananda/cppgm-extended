#pragma once

#include <string>
#include <vector>

#include "recog_token_buffer.h"
#include "witness_api.h"

std::string render_witness_sessions(
    const std::vector<std::string> & source_paths,
    const std::vector<witness::TemplateWitnessSession> & sessions);

std::string render_witness_debug_sessions(
    const std::vector<std::string> & source_paths,
    const std::vector<witness::TemplateWitnessSession> & sessions);
