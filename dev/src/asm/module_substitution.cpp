#include "asm/module_expand.h"

#include <cctype>

namespace assembly {
namespace {
bool identifier(char c)
{
	return std::isalnum(static_cast<unsigned char>(c)) || c == '_' || c == '.' || c == '$';
}
void append(ModuleLine& to, const ModuleLine& from)
{
	const uint32_t first = static_cast<uint32_t>(to.text.size());
	if (to.text.size() + from.text.size() > 64u * 1024u * 1024u)
		throw AssemblyError{ "macro substitution exceeds byte limit", to.line, 1 };
	to.text += from.text;
	for (SymbolUse use : from.symbols)
	{
		use.first += first;
		to.symbols.push_back(use);
	}
}
}
ModuleLine module_slice(const ModuleLine& line, uint32_t first, uint32_t count)
{
	ModuleLine result;
	result.line = line.line;
	result.text = line.text.substr(first, count);
	for (const SymbolUse& use : line.symbols)
		if (use.first < uint64_t(first) + count && uint64_t(use.first) + use.count > first &&
			!(use.first >= first && uint64_t(use.first) + use.count <= uint64_t(first) + count))
			throw AssemblyError{ "macro argument splits a typed symbol", line.line, first + 1 };
	for (SymbolUse use : line.symbols)
		if (use.first >= first && uint64_t(use.first) + use.count <= uint64_t(first) + count)
		{
			use.first -= first;
			result.symbols.push_back(use);
		}
	return result;
}
ModuleLine substitute_module(
	const ModuleLine& line, const std::map<std::string, ModuleLine>& bindings, uint64_t unique, uint64_t iteration)
{
	ModuleLine result;
	result.line = line.line;
	size_t use = 0;
	for (uint32_t i = 0; i < line.text.size();)
	{
		if (use < line.symbols.size() && line.symbols[use].first == i)
		{
			const SymbolUse& symbol = line.symbols[use++];
			append(result, module_slice(line, i, symbol.count));
			i += symbol.count;
			continue;
		}
		if (line.text[i] != '\\' || i + 1 == line.text.size())
		{
			result.text += line.text[i++];
			continue;
		}
		const char next = line.text[i + 1];
		if (next == '@' || next == '+')
		{
			result.text += std::to_string(next == '@' ? unique : iteration);
			i += 2;
		}
		else if (next == '(' && i + 2 < line.text.size() && line.text[i + 2] == ')')
			i += 3;
		else
		{
			uint32_t end = i + 1;
			while (end < line.text.size() && identifier(line.text[end]))
				++end;
			const auto found = bindings.find(line.text.substr(i + 1, end - i - 1));
			if (found == bindings.end())
				result.text += line.text[i++];
			else
			{
				append(result, found->second);
				i = end;
			}
		}
	}
	return result;
}
} // namespace assembly
