#include "wasm/object.h"

namespace wasm {

void add_runtime_inputs(LinkInputs& inputs, const backend::Options& options)
{
	const auto& objects = inputs.objects;
	if (options.target == target_model::Target::Wasi && !options.no_startfiles)
	{
		if (!options.sysroot.empty())
			read_link_inputs(options.sysroot + "/lib/wasm32-wasip1/crt1-command.o", inputs, true);
		else
		{
			bool entry = false;
			for (const Object& object : objects)
				if (!object.archive_member)
					for (const ObjectSymbol& symbol : object.symbols)
						entry = entry || (symbol.kind == 0 && !symbol.undefined() && symbol.name == "_start");
			if (!entry)
				throw WasmError("WASI command startup requires --sysroot or an explicit _start definition; "
								"use -nostartfiles to link a reactor");
		}
	}
	if (options.no_standard_libraries)
		return;
	if (!options.sysroot.empty())
	{
		const std::string root = options.sysroot + "/lib/wasm32-wasip1/";
		if (options.cxx_runtime)
		{
			const std::string cxx = root + (options.exceptions ? "eh/" : "noeh/");
			read_link_inputs(cxx + "libc++.a", inputs, true);
			read_link_inputs(cxx + "libc++abi.a", inputs, true);
			if (options.exceptions)
				read_link_inputs(cxx + "libunwind.a", inputs, true);
		}
		read_link_inputs(root + "libc.a", inputs, true);
	}
	if (!options.resource_directory.empty())
		read_link_inputs(
			options.resource_directory + "/lib/wasm32-unknown-wasip1/libclang_rt.builtins.a", inputs, true);
}

} // namespace wasm
