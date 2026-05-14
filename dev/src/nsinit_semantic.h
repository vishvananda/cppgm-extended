#pragma once

#include <ctime>
#include <memory>
#include <string>
#include <vector>

#include "posttokenizer.h"
#include "preprocessor.h"
#include "recog_token_buffer.h"

struct NSTranslationUnitInput
{
  NSTranslationUnitInput();
  NSTranslationUnitInput(const std::string & source_path, std::time_t now);
  NSTranslationUnitInput(NSTranslationUnitInput &&) = delete;
  NSTranslationUnitInput & operator=(NSTranslationUnitInput &&) = delete;
  NSTranslationUnitInput(const NSTranslationUnitInput &) = delete;
  NSTranslationUnitInput & operator=(const NSTranslationUnitInput &) = delete;

  IRecogTokenSequence & token_sequence() const;

  std::string source_path;
  std::unique_ptr<SourceLocationTable> source_locations;
  std::unique_ptr<Preprocessor> preprocessor;
  std::unique_ptr<PostTokenizer> posttokenizer;
  std::unique_ptr<RecogTokenizer> tokenizer;
  std::unique_ptr<RecogTokenBuffer> token_buffer;
};

std::vector<char> build_nsinit_program_image(
    const std::vector<std::unique_ptr<NSTranslationUnitInput>> & translation_units);
