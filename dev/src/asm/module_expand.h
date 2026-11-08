#pragma once

#include <map>
#include <memory>
#include "asm/module_text.h"

namespace assembly {

// Source-only expansion; typed symbol spans survive every argument substitution.
// Instructions are still parsed exactly once by the downstream assembler.
class ModuleExpander
{
public:
	std::vector<ModuleLine> expand(const std::vector<ModuleLine>& input);

private:
	struct Formal
	{
		std::string name;
		ModuleLine fallback;
		bool required = false;
		bool variable = false;
	};
	struct Macro
	{
		std::vector<Formal> formals;
		std::vector<ModuleLine> body;
		uint64_t executions = 0;
	};
	using Bindings = std::map<std::string, ModuleLine>;
	bool range(const std::vector<ModuleLine>& input, size_t first, size_t last, std::vector<ModuleLine>& output,
		unsigned depth);
	void define(const ModuleLine& declaration, const std::vector<ModuleLine>& input, size_t first, size_t last);
	void invoke(const ModuleLine& invocation, uint32_t first, std::shared_ptr<Macro> macro,
		std::vector<ModuleLine>& output, unsigned depth);
	bool substituted(const std::vector<ModuleLine>& body, const Bindings& bindings, uint64_t unique, uint64_t iteration,
		std::vector<ModuleLine>& output, unsigned depth);
	void repeat(const std::string& kind, const ModuleLine& declaration, const std::vector<ModuleLine>& body,
		std::vector<ModuleLine>& output, unsigned depth);
	void account(uint64_t bytes, const ModuleLine& line);
	void emit(const ModuleLine& line, std::vector<ModuleLine>& output);
	std::map<std::string, std::shared_ptr<Macro>> macros_;
	uint64_t unique_ = 0;
	uint64_t work_ = 0;
	uint64_t bytes_ = 0;
};

std::string module_macro_name(std::string name);
size_t module_block_end(const std::vector<ModuleLine>& input, size_t first, size_t last);
bool module_condition(const std::string& kind, ModuleCursor& cursor, const ModuleLine& line);

ModuleLine module_slice(const ModuleLine& line, uint32_t first, uint32_t count);
std::vector<ModuleLine> module_arguments(
	const ModuleLine& line, uint32_t first, std::vector<uint32_t>* starts = nullptr);
ModuleLine substitute_module(
	const ModuleLine& line, const std::map<std::string, ModuleLine>& bindings, uint64_t unique, uint64_t iteration);

} // namespace assembly
