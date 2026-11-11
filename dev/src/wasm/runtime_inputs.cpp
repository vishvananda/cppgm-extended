#include "wasm/object.h"

namespace wasm {

void add_runtime_inputs(std::vector<Object>& objects, const backend::Options& options)
{
	if (options.target == target_model::Target::Wasi && !options.no_startfiles)
	{
		if (!options.sysroot.empty())
			read_link_inputs(options.sysroot + "/lib/wasm32-wasip1/crt1-command.o", objects);
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
		read_link_inputs(options.sysroot + "/lib/wasm32-wasip1/libc.a", objects);
	if (!options.resource_directory.empty())
		read_link_inputs(options.resource_directory + "/lib/wasm32-unknown-wasip1/libclang_rt.builtins.a", objects);
}

} // namespace wasm
