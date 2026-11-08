#pragma once

#include "asm/module_expand.h"
#include "lowir/program.h"

namespace native {

// Snapshots refer to a prefix of ordered module invocations. Typed symbol
// spans retain program identities until object placement, never rendered names.
class AssemblyEnvironment
{
public:
	explicit AssemblyEnvironment(const lowir::Program& program);
	assembly::ModuleExpander snapshot(uint32_t prefix);
	const std::vector<std::vector<assembly::ModuleLine>>& globals();

private:
	void ensure(uint32_t prefix);
	const lowir::Program& program_;
	assembly::ModuleExpander current_;
	std::vector<assembly::ModuleExpander> snapshots_;
	std::vector<std::vector<assembly::ModuleLine>> sources_;
};

} // namespace native
