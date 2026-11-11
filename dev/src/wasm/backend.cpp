#include "wasm/backend.h"

#include <cstdio>

#include "opt/optimizer.h"
#include "wasm/module.h"

namespace wasm {

namespace {

bool write_file(const std::string& path, const void* data, size_t size, std::string& error)
{
	std::FILE* file = std::fopen(path.c_str(), "wb");
	if (file == nullptr)
	{
		error = "cannot write " + path;
		return false;
	}
	const bool written = size == 0 || std::fwrite(data, 1, size, file) == size;
	const bool closed = std::fclose(file) == 0;
	if (!written || !closed)
	{
		std::remove(path.c_str());
		error = "cannot write " + path;
		return false;
	}
	return true;
}

} // namespace

bool compile_program(lowir::Program& program, const backend::Options& options, std::string& error)
{
	if (!program.explicit_target || program.target != options.target)
	{
		error = "Wasm compilation requires a matching explicit LowIR target profile";
		return false;
	}
	if (program.assembly)
	{
		error = "assembly is not supported by this Wasm profile";
		return false;
	}
	try
	{
		opt::OptimizerOptions optimization;
		if (options.optimizer != nullptr)
			optimization = *options.optimizer;
		optimization.level = options.optimization_level;
		optimization.has_gnu_sincos = false;
		opt::OptimizerStats stats;
		opt::optimize_program(program, optimization, &stats);
		Bytes bytes;
		std::string dump = "machine_ir wasm32 1\n";
		encode_module(program, options, bytes, dump);
		if (!options.machine_ir.empty() && !write_file(options.machine_ir, dump.data(), dump.size(), error))
			return false;
		return options.output.empty() || write_file(options.output, bytes.data.data(), bytes.data.size(), error);
	}
	catch (const WasmError& failure)
	{
		error = failure.what();
		return false;
	}
}

} // namespace wasm
