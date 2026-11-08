#include "asm/module_expand.h"

#include <algorithm>
#include "asm/module_expression.h"

namespace assembly {
void ModuleExpander::define(
	const ModuleLine& declaration, const std::vector<ModuleLine>& input, size_t first, size_t last)
{
	ModuleCursor cursor(declaration);
	cursor.word();
	const std::string name = module_macro_name(cursor.word());
	if (macros_.count(name))
		cursor.fail("macro is already defined");
	std::shared_ptr<Macro> macro(new Macro);
	for (const ModuleLine& part : module_arguments(declaration, cursor.position()))
	{
		ModuleCursor formal(part);
		Formal entry;
		entry.name = formal.word();
		for (const Formal& prior : macro->formals)
			if (prior.name == entry.name || prior.variable)
				formal.fail("duplicate formal or non-final vararg");
		if (formal.take(':'))
		{
			const std::string kind = formal.word();
			entry.required = kind == "req";
			entry.variable = kind == "vararg";
			if (!entry.required && !entry.variable)
				formal.fail("unknown macro parameter qualifier");
		}
		if (formal.take('='))
		{
			entry.fallback = module_slice(
				part, formal.position(), static_cast<uint32_t>(part.text.size() - formal.position()));
			formal.rest();
			if (entry.required || entry.variable)
				formal.fail("qualified macro parameter cannot have a default");
		}
		formal.finish();
		macro->formals.push_back(std::move(entry));
	}
	for (size_t i = first; i < last; ++i)
		account(input[i].text.size() + 1, input[i]);
	macro->body.assign(input.begin() + first, input.begin() + last);
	macros_.emplace(name, std::move(macro));
}
void ModuleExpander::invoke(const ModuleLine& invocation, uint32_t first, std::shared_ptr<Macro> macro,
	std::vector<ModuleLine>& output, unsigned depth)
{
	std::vector<uint32_t> starts;
	const std::vector<ModuleLine> arguments = module_arguments(invocation, first, &starts);
	Bindings bindings;
	size_t next = 0;
	bool keywords = false;
	for (size_t i = 0; i < arguments.size(); ++i)
	{
		ModuleLine value = arguments[i];
		size_t index = next;
		const size_t equal = value.text.find('=');
		if (equal != std::string::npos)
		{
			std::string name = value.text.substr(0, equal);
			while (!name.empty() && (name.back() == ' ' || name.back() == '\t'))
				name.pop_back();
			const auto found = std::find_if(macro->formals.begin(), macro->formals.end(),
				[&](const Formal& formal) { return formal.name == name; });
			if (found != macro->formals.end())
			{
				keywords = true;
				index = static_cast<size_t>(found - macro->formals.begin());
				value = module_slice(
					value, static_cast<uint32_t>(equal + 1), static_cast<uint32_t>(value.text.size() - equal - 1));
			}
			else if (value.text.find_first_not_of("abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789_") ==
				equal)
				throw AssemblyError{ "unknown macro keyword", invocation.line, first + 1 };
		}
		else if (keywords)
			throw AssemblyError{ "positional macro argument after keyword", invocation.line, first + 1 };
		if (index >= macro->formals.size())
			throw AssemblyError{ "too many macro arguments", invocation.line, first + 1 };
		const Formal& formal = macro->formals[index];
		if (formal.variable)
		{
			uint32_t begin = starts[i];
			if (keywords)
			{
				ModuleCursor raw(invocation);
				raw.advance(begin);
				raw.word();
				if (!raw.take('='))
					raw.fail("expected macro keyword assignment");
				begin = raw.position();
			}
			value = module_slice(invocation, begin, static_cast<uint32_t>(invocation.text.size() - begin));
			i = arguments.size() - 1;
		}
		if (!bindings.emplace(formal.name, std::move(value)).second)
			throw AssemblyError{ "duplicate macro argument", invocation.line, first + 1 };
		next = index + 1;
	}
	for (const Formal& formal : macro->formals)
	{
		auto bound = bindings.find(formal.name);
		if (bound == bindings.end())
			bound = bindings.emplace(formal.name, formal.fallback).first;
		if (bound->second.text.empty() && formal.required)
			throw AssemblyError{ "missing required macro argument", invocation.line, first + 1 };
		if (bound->second.text.empty())
			bound->second = formal.fallback;
	}
	substituted(macro->body, bindings, unique_++, macro->executions++, output, depth);
}
bool ModuleExpander::substituted(const std::vector<ModuleLine>& body, const Bindings& bindings, uint64_t unique,
	uint64_t iteration, std::vector<ModuleLine>& output, unsigned depth)
{
	std::vector<ModuleLine> expanded;
	for (const ModuleLine& line : body)
	{
		const ModuleLine result = substitute_module(line, bindings, unique, iteration);
		account(result.text.size() + 1, line);
		for (ModuleLine item : module_lines(result.text, result.symbols))
		{
			item.line = line.line;
			expanded.push_back(std::move(item));
		}
	}
	return range(expanded, 0, expanded.size(), output, depth);
}
void ModuleExpander::repeat(const std::string& kind, const ModuleLine& declaration, const std::vector<ModuleLine>& body,
	std::vector<ModuleLine>& output, unsigned depth)
{
	ModuleCursor cursor(declaration);
	cursor.word();
	uint64_t count = 0;
	std::string name;
	std::vector<ModuleLine> values;
	if (kind == ".rept")
	{
		count = module_expression(declaration, cursor.position());
		if (static_cast<int64_t>(count) < 0)
			count = 0;
	}
	else
	{
		name = cursor.word();
		if (!cursor.take(','))
			cursor.fail("iteration requires a parameter and values");
		values = module_arguments(declaration, cursor.position());
		if (kind == ".irpc")
		{
			if (values.size() > 1)
				cursor.fail("character iteration requires one string");
			const ModuleLine text = values.empty() ? ModuleLine{} : values.front();
			values.clear();
			for (uint32_t i = 0; i < text.text.size(); ++i)
				values.push_back(module_slice(text, i, 1));
		}
		if (values.empty())
			values.push_back(ModuleLine{});
		count = values.size();
	}
	if (count > 1000000 || count > 1000000 - work_)
		cursor.fail("module repetition exceeds work limit");
	for (uint64_t i = 0; i < count; ++i)
	{
		++work_;
		if (kind == ".rept")
		{
			if (range(body, 0, body.size(), output, depth))
				return;
		}
		else
		{
			Bindings bindings;
			bindings.emplace(name, values[i]);
			if (substituted(body, bindings, unique_, i, output, depth))
				return;
		}
	}
	return;
}
} // namespace assembly
