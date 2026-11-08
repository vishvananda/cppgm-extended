#include "asm/module_expand.h"

#include <cctype>

namespace assembly {
namespace {
ModuleLine argument(const ModuleLine& line, uint32_t first, uint32_t end)
{
	while (first < end && std::isspace(static_cast<unsigned char>(line.text[first])))
		++first;
	while (end > first && std::isspace(static_cast<unsigned char>(line.text[end - 1])))
		--end;
	ModuleLine input = module_slice(line, first, end - first), result;
	result.line = line.line;
	bool quoted = false;
	size_t use = 0, label = 0;
	for (uint32_t i = 0; i < input.text.size(); ++i)
	{
		if (use < input.symbols.size() && input.symbols[use].first == i)
		{
			SymbolUse span = input.symbols[use++];
			const uint32_t count = span.count;
			span.first = static_cast<uint32_t>(result.text.size());
			result.symbols.push_back(span);
			result.text.append(input.text, i, count);
			i += count - 1;
		}
		else if (label < input.labels.size() && input.labels[label].first == i)
		{
			const uint32_t count = input.labels[label++].count;
			append_module_line(result, module_slice(input, i, count));
			i += count - 1;
		}
		else if (input.text[i] == '"')
		{
			if (quoted && i + 1 < input.text.size() && input.text[i + 1] == '"')
			{
				result.text += '"';
				++i;
			}
			else
				quoted = !quoted;
		}
		else if (quoted && input.text[i] == '\\' && i + 1 < input.text.size() &&
			(input.text[i + 1] == '"' || input.text[i + 1] == '\\'))
			result.text += input.text[++i];
		else
			result.text += input.text[i];
	}
	return result;
}
}
std::vector<ModuleLine> module_arguments(const ModuleLine& line, uint32_t first, std::vector<uint32_t>* starts)
{
	std::vector<ModuleLine> result;
	auto push = [&](uint32_t begin, uint32_t end) {
		while (begin < end && std::isspace(static_cast<unsigned char>(line.text[begin])))
			++begin;
		if (starts)
			starts->push_back(begin);
		result.push_back(argument(line, begin, end));
	};
	uint32_t begin = first;
	std::vector<char> closing;
	bool quote = false, escape = false, emitted = false, trailing = false;
	for (uint32_t i = first; i < line.text.size(); ++i)
	{
		const char c = line.text[i];
		if (quote)
		{
			if (!escape && c == '"')
				quote = false;
			escape = !escape && c == '\\';
		}
		else if (c == '"')
			quote = true;
		else if (c == '(' || c == '[')
		{
			if (closing.size() == 64)
				throw AssemblyError{ "macro argument nesting exceeds limit", line.line, i + 1 };
			closing.push_back(c == '(' ? ')' : ']');
		}
		else if (c == ')' || c == ']')
		{
			if (closing.empty() || closing.back() != c)
				throw AssemblyError{ "unbalanced macro argument", line.line, i + 1 };
			closing.pop_back();
		}
		else if (closing.empty() && (c == ',' || std::isspace(static_cast<unsigned char>(c))))
		{
			if (c != ',')
			{
				uint32_t next = i + 1, previous = i;
				while (next < line.text.size() && std::isspace(static_cast<unsigned char>(line.text[next])))
					++next;
				while (previous > begin && std::isspace(static_cast<unsigned char>(line.text[previous - 1])))
					--previous;
				if ((next < line.text.size() && (line.text[next] == '=' || line.text[next] == ':')) ||
					(previous > begin && (line.text[previous - 1] == '=' || line.text[previous - 1] == ':')))
					continue;
			}
			const bool present = line.text.substr(begin, i - begin).find_first_not_of(" \t\r") != std::string::npos;
			if (present || (c == ',' && !emitted))
			{
				push(begin, i);
				emitted = true;
			}
			if (c == ',')
			{
				emitted = false;
				trailing = true;
			}
			else if (present)
				trailing = false;
			begin = i + 1;
		}
	}
	if (quote || !closing.empty())
		throw AssemblyError{ "unbalanced macro argument", line.line, first + 1 };
	if (line.text.substr(begin).find_first_not_of(" \t\r") != std::string::npos || trailing)
		push(begin, static_cast<uint32_t>(line.text.size()));
	return result;
}
} // namespace assembly
