#pragma once

// Work counters for ABI mangling (spec section 9). They observe work the
// reader and encoder already do; nothing is computed for the report alone.

#include <cstddef>

#include "support/telemetry.h"

namespace abi_mangle {

struct AbiMangleStats
{
	telemetry::Counter source_files = 0;
	telemetry::Counter source_bytes = 0;
	telemetry::Counter cases = 0;
	telemetry::Counter records = 0;
	telemetry::Counter definitions = 0;
	telemetry::Counter targets = 0;
	telemetry::Counter peak_graph_nodes = 0;
	telemetry::Counter peak_graph_bytes = 0;
	telemetry::Counter substitution_lookups = 0;
	telemetry::Counter substitution_hits = 0;
	telemetry::Counter substitution_entries = 0;
	telemetry::Counter entity_references = 0;
	telemetry::Counter output_bytes = 0;
};

} // namespace abi_mangle
