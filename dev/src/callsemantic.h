#pragma once

#include <string>
#include "callsem_output.h"
#include "recog_token_buffer.h"
#include "witness_api.h"

CallSemNode analyze_calls_translation_unit(
    IRecogTokenSequence & tokens,
    bool expand_output_closure = false,
    bool emit_all_source_function_definitions = false,
    witness::TemplateWitnessSession * witness_session = nullptr);
CallSemNode analyze_calls_translation_unit(
    const std::vector<RecogToken> & tokens,
    bool expand_output_closure = false,
    bool emit_all_source_function_definitions = false,
    witness::TemplateWitnessSession * witness_session = nullptr);
std::string describe_calls_translation_unit(
    IRecogTokenSequence & tokens);
std::string describe_calls_translation_unit(
    const std::vector<RecogToken> & tokens);
