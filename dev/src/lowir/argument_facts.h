#pragma once

#include "lowir/program.h"

namespace lowir {

bool valid_aggregate_shape(Type type, ir_model::AggregateShape shape);
bool valid_this_return(const Program& program, const Signature& signature);

// Returns a diagnostic for malformed per-argument metadata, or null.
const char* validate_argument_facts(const Program& program, const FunctionBody& body, InstId& instruction);

} // namespace lowir
