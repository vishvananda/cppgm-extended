#include "lower/function_lowering.h"

namespace lower {

FunctionLowering::LexicalScope::LexicalScope(FunctionLowering& lowering, bool enter)
	: lowering_(lowering), saved_(lowering.debug_scope_),
	  active_(enter && lowering.debug_ && lowering.program_.debug && lowering.debug_function_ != lowir::kNoId)
{
	if (!active_)
		return;
	lowir::DebugInfo& info = *lowering.program_.debug;
	const uint32_t id = static_cast<uint32_t>(info.scopes.size());
	info.scopes.push_back(
		lowir::DebugLexicalScope{ lowering.program_.symbols[lowering.debug_function_].name, saved_, 0 });
	if (lowering.debug_function_ >= info.functions.size())
		info.functions.resize(lowering.debug_function_ + 1);
	lowir::DebugFunction& function = info.functions[lowering.debug_function_];
	if (function.last_scope != 0)
		info.scopes[function.last_scope].next = id;
	else
		function.first_scope = id;
	function.last_scope = id;
	lowering.debug_scope_ = id;
	lowir::DebugLocation location = lowering.builder_.current_location();
	location.scope = id;
	lowering.builder_.set_current_location(location);
}

FunctionLowering::LexicalScope::~LexicalScope()
{
	if (!active_)
		return;
	lowering_.debug_scope_ = saved_;
	lowir::DebugLocation location = lowering_.builder_.current_location();
	location.scope = saved_;
	lowering_.builder_.set_current_location(location);
}
} // namespace lower
