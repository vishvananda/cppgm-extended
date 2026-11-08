#include "asm/module_text.h"

#include <algorithm>
#include <cctype>
#include <climits>

namespace assembly {
namespace {
bool identifier(const std::string& text)
{
	bool seen = false, trailing = false;
	for (char c : text)
	{
		if (c == ' ' || c == '\t' || c == '\r')
		{
			trailing = trailing || seen;
		}
		else if (std::isalnum(static_cast<unsigned char>(c)) || c == '_' || c == '.' || c == '$')
		{
			if (trailing)
				return false;
			seen = true;
		}
		else
			return false;
	}
	return seen;
}
unsigned digit(char c)
{
	return c >= '0' && c <= '9' ? static_cast<unsigned>(c - '0')
		: c >= 'a' && c <= 'f'  ? static_cast<unsigned>(c - 'a' + 10)
		: c >= 'A' && c <= 'F'  ? static_cast<unsigned>(c - 'A' + 10)
								: 16;
}
}

std::vector<ModuleLine> module_lines(const std::string& text, const std::vector<SymbolUse>& symbols)
{
	std::vector<ModuleLine> result;
	size_t first = 0;
	uint32_t line = 1, first_line = 1;
	bool quote = false, escape = false;
	auto append = [&](size_t end) {
		ModuleLine entry;
		entry.text = text.substr(first, end - first);
		entry.line = first_line;
		const auto begin = std::lower_bound(symbols.begin(), symbols.end(), first,
			[](const SymbolUse& use, size_t position) { return use.first < position; });
		for (auto use = begin; use != symbols.end() && use->first < end; ++use)
		{
			if (uint64_t(use->first) + use->count > end)
				throw AssemblyError{ "module statement splits a typed symbol", first_line, 1 };
			SymbolUse shifted = *use;
			shifted.first -= static_cast<uint32_t>(first);
			entry.symbols.push_back(shifted);
		}
		if (entry.text.find_first_not_of(" \t\r") != std::string::npos)
			result.push_back(std::move(entry));
	};
	for (size_t i = 0; i < text.size(); ++i)
	{
		const char c = text[i];
		if (c == '\0')
			throw AssemblyError{ "NUL in module assembly", line, 1 };
		if (quote)
		{
			if (!escape && c == '"')
				quote = false;
			escape = !escape && c == '\\';
		}
		else if (c == '"')
			quote = true;
		else if (c == '#')
		{
			append(i);
			while (i < text.size() && text[i] != '\n')
				++i;
			if (i == text.size())
			{
				first = i;
				break;
			}
			first = i + 1;
			++line;
			first_line = line;
		}
		else if (c == '\n' || c == ';' || (c == ':' && identifier(text.substr(first, i - first))))
		{
			append(i + (c == ':'));
			first = i + 1;
			first_line = line + (c == '\n');
		}
		if (c == '\n')
			++line;
	}
	if (quote)
		throw AssemblyError{ "unterminated module string", line, 1 };
	append(text.size());
	return result;
}

void ModuleCursor::space()
{
	while (position_ < line_.text.size() && std::isspace(static_cast<unsigned char>(line_.text[position_])))
		++position_;
}
bool ModuleCursor::take(char c)
{
	space();
	if (position_ == line_.text.size() || line_.text[position_] != c)
		return false;
	++position_;
	return true;
}
bool ModuleCursor::end()
{
	space();
	return position_ == line_.text.size();
}
bool ModuleCursor::numeric()
{
	space();
	return position_ < line_.text.size() &&
		(std::isdigit(static_cast<unsigned char>(line_.text[position_])) || line_.text[position_] == '-' ||
			line_.text[position_] == '+');
}
bool ModuleCursor::quoted_start()
{
	space();
	return position_ < line_.text.size() && line_.text[position_] == '"';
}
std::string ModuleCursor::word()
{
	space();
	const uint32_t first = position_;
	while (position_ < line_.text.size())
	{
		const char c = line_.text[position_];
		if (!std::isalnum(static_cast<unsigned char>(c)) && c != '_' && c != '.' && c != '$')
			break;
		++position_;
	}
	if (first == position_)
		fail("expected module assembly identifier");
	return line_.text.substr(first, position_ - first);
}
uint64_t ModuleCursor::number()
{
	space();
	const bool negative = take('-');
	if (!negative)
		take('+');
	unsigned base = 10;
	if (position_ + 1 < line_.text.size() && line_.text[position_] == '0')
	{
		const char c = line_.text[position_ + 1];
		base = c == 'x' || c == 'X' ? 16 : 8;
		if (base == 16)
			position_ += 2;
	}
	uint64_t value = 0;
	bool seen = false;
	while (position_ < line_.text.size() && digit(line_.text[position_]) < base)
	{
		const unsigned part = digit(line_.text[position_++]);
		if (value > (UINT64_MAX - part) / base)
			fail("module integer exceeds 64 bits");
		value = value * base + part;
		seen = true;
	}
	if (!seen)
		fail("expected module integer");
	return negative ? uint64_t(0) - value : value;
}
std::string ModuleCursor::quoted()
{
	if (!take('"'))
		fail("expected module string");
	std::string result;
	while (position_ < line_.text.size())
	{
		char c = line_.text[position_++];
		if (c == '"')
			return result;
		if (c == '\\')
		{
			if (position_ == line_.text.size())
				fail("unterminated module escape");
			c = line_.text[position_++];
			if (c >= '0' && c <= '7')
			{
				unsigned value = c - '0';
				for (unsigned i = 1; i < 3 && position_ < line_.text.size() && line_.text[position_] >= '0' &&
					line_.text[position_] <= '7';
					++i)
					value = value * 8 + line_.text[position_++] - '0';
				c = static_cast<char>(value);
			}
			else if (c == 'n')
				c = '\n';
			else if (c == 'r')
				c = '\r';
			else if (c == 't')
				c = '\t';
			else if (c != '\\' && c != '"')
				fail("unsupported module string escape");
		}
		result += c;
	}
	fail("unterminated module string");
}
const SymbolUse* ModuleCursor::symbol_use() const
{
	for (const SymbolUse& use : line_.symbols)
		if (use.first == position_)
			return &use;
	return nullptr;
}
std::string ModuleCursor::rest()
{
	space();
	const std::string value = line_.text.substr(position_);
	position_ = static_cast<uint32_t>(line_.text.size());
	return value;
}
void ModuleCursor::finish()
{
	if (!end())
		fail("unexpected token in module directive");
}
void ModuleCursor::fail(const std::string& message) const
{
	throw AssemblyError{ message, line_.line, position_ + 1 };
}
} // namespace assembly
