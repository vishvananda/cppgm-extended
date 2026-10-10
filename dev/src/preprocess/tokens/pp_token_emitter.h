#pragma once

#include "preprocess/source/source_buffer.h"
#include "preprocess/tokens/IPPTokenStream.h"

// Lexes `source` and reports every preprocessing token to `output`, ending
// with emit_eof(). Spelling and new-line events are preceded by
// set_source_location(). Throws SourceError for ill-formed input; tokens
// before the error have already been reported.
void EmitPPTokens(const SourceBuffer& source, IPPTokenStream& output);
