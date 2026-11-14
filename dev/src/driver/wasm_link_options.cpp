#include "driver/wasm_link_options.h"

#include "support/tool_errors.h"

namespace driver {

bool parse_wasm_link_option(const std::vector<std::string>& args, size_t& index, backend::WasmLinkOptions& options)
{
	const std::string& arg = args[index];
	if (arg == "--export-table")
		options.export_table = true;
	else if (arg == "--export-ctors")
		options.export_constructors = true;
	else if (arg == "--import-function" || arg.compare(0, 18, "--import-function=") == 0)
	{
		if (arg == "--import-function" && index + 1 >= args.size())
			throw InvocationError("missing name after --import-function");
		const std::string name = arg == "--import-function" ? args[++index] : arg.substr(18);
		if (name.empty())
			throw InvocationError("empty import function name");
		options.imports.insert(name);
	}
	else
		return false;
	return true;
}

} // namespace driver
