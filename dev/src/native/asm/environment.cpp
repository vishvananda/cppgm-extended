#include "native/asm/environment.h"

#include "lowir/assembly.h"
#include "native/asm/binding.h"
#include "native/backend_error.h"

namespace native {

AssemblyEnvironment::AssemblyEnvironment(const lowir::Program& program) : program_(program)
{
	snapshots_.push_back(current_);
}
void AssemblyEnvironment::ensure(uint32_t prefix)
{
	if ((!program_.assembly && prefix != 0) || (program_.assembly && prefix > program_.assembly->globals.size()))
		throw BackendError("assembly environment prefix is out of range");
	while (sources_.size() < prefix)
	{
		const lowir::AssemblyGlobal& item = program_.assembly->globals[sources_.size()];
		lowir::FunctionBody constants;
		constants.literals = item.literals;
		std::string source;
		std::vector<assembly::SymbolUse> symbols;
		std::vector<assembly::LabelUse> labels;
		prepare_assembly_source(
			*program_.assembly, item.definition, constants, item.arguments, source, symbols, labels);
		sources_.push_back(current_.expand(assembly::module_lines(source, symbols, labels)));
		snapshots_.push_back(current_);
	}
}
assembly::ModuleExpander AssemblyEnvironment::fork(uint32_t prefix)
{
	ensure(prefix);
	return snapshots_[prefix];
}
const std::vector<std::vector<assembly::ModuleLine>>& AssemblyEnvironment::globals()
{
	ensure(program_.assembly ? static_cast<uint32_t>(program_.assembly->globals.size()) : 0);
	return sources_;
}

} // namespace native
