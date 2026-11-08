#pragma once

namespace lowir {

struct AssemblyModule;

// The shared assembly packet's structural contract. Invocation value types,
// block edges and target capabilities are checked by their owning consumers.
bool validate_assembly_module(const AssemblyModule& module, const char*& error);

} // namespace lowir
