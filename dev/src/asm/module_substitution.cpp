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
	if (to.text.size() + from.text.size() > 64u * 1024u * 1024u)
		throw AssemblyError{ "macro substitution exceeds byte limit", to.line, 1 };
	append_module_line(to, from);
}
}
ModuleLine substitute_module(
	const ModuleLine& line, const std::map<std::string, ModuleLine>& bindings, uint64_t unique, uint64_t iteration)
{
	ModuleLine result;
	result.line = line.line;
	size_t use = 0, label = 0;
	for (uint32_t i = 0; i < line.text.size();)
	{
		if (use < line.symbols.size() && line.symbols[use].first == i)
		{
			const SymbolUse& symbol = line.symbols[use++];
			append(result, module_slice(line, i, symbol.count));
			i += symbol.count;
			continue;
		}
		if (label < line.labels.size() && line.labels[label].first == i)
		{
			const uint32_t count = line.labels[label++].count;
			append(result, module_slice(line, i, count));
			i += count;
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
