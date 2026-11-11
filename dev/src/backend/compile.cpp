#include "backend/compile.h"

#include "native/backend.h"
#include "wasm/backend.h"

namespace backend {

bool compile_program(lowir::Program& program, const Options& options, std::string& error, std::string* stats_line)
{
	if (program.explicit_target && program.target != options.target)
	{
		error = "input and output target profiles differ";
		return false;
	}
	if (target_model::is_wasm(options.target))
		return wasm::compile_program(program, options, error);
	if (!options.link_inputs.empty())
	{
		error = "additional object inputs require a Wasm target";
		return false;
	}
	if (!options.exports.empty())
	{
		error = "explicit function exports require a Wasm target";
		return false;
	}
	return native::compile_program(program, options, error, stats_line);
}

} // namespace backend
