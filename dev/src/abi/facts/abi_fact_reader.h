#pragma once

// The PA9 `abimangle` fact-file adapter. Fact files are an explicit tool
// input: this reader turns their records into typed ABI graph nodes and
// targets for the shared encoder. Compiler stages build the same targets
// directly and never pass through fact text.

#include "abi/itanium/abi_encoder.h"

#include <cstdio>
#include <string>
#include <vector>

namespace abi_facts {

// Reads every case of the files in order and writes one mangled name per
// case, each followed by a newline, to `out` as the cases are encoded.
// Throws FactError for unreadable files or malformed facts and
// abi_mangle::EncodingError for unencodable targets; names already written
// stay written.
void mangle_fact_files(const std::vector<std::string>& paths, std::FILE* out, abi_mangle::AbiMangleStats* stats);

} // namespace abi_facts
