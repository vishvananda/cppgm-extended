#include "wasm/backend.h"

#include <cstdio>

#include "opt/optimizer.h"
#include "wasm/object_reader.h"

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

bool compile_object(
	lowir::Program& program, const backend::Options& options, Object& object, std::string& dump, std::string& error)
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
		legalize_wide_helpers(program);
		make_object(program, options, object, dump);
		return options.machine_ir.empty() || write_file(options.machine_ir, dump.data(), dump.size(), error);
	}
	catch (const WasmError& failure)
	{
		error = failure.what();
		return false;
	}
}

bool output_objects(std::vector<Object>& objects, const backend::Options& options, bool relocatable, std::string& error)
{
	try
	{
		if (!relocatable)
			add_runtime_inputs(objects, options);
		for (const Object& object : objects)
			validate_object(object);
		Bytes bytes;
		if (relocatable)
		{
			if (objects.size() != 1)
				throw WasmError("one source input is required for a Wasm object output");
			write_object(objects.front(), bytes);
		}
		else
			link_objects(objects, options, bytes);
		return options.output.empty() || write_file(options.output, bytes.data.data(), bytes.data.size(), error);
	}
	catch (const WasmError& failure)
	{
		error = failure.what();
		return false;
	}
}

bool compile_program(lowir::Program& program, const backend::Options& options, std::string& error)
{
	Object object;
	std::string dump = "machine_ir wasm32 1\n";
	if (!compile_object(program, options, object, dump, error))
		return false;
	std::vector<Object> objects;
	objects.push_back(std::move(object));
	try
	{
		for (const std::string& path : options.link_inputs)
			read_link_inputs(path, objects);
	}
	catch (const WasmError& failure)
	{
		error = failure.what();
		return false;
	}
	return output_objects(objects, options, false, error);
}

} // namespace wasm
