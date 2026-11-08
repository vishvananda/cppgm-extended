#include "asm/module_expand.h"

#include "asm/module_expression.h"

namespace assembly {
namespace {
std::string trim(std::string text)
{
	const size_t first = text.find_first_not_of(" \t\r"), last = text.find_last_not_of(" \t\r");
	if (first == std::string::npos)
		return {};
	text = text.substr(first, last - first + 1);
	if (text.size() >= 2 && (text.front() == '\'' || text.front() == '"') && text.front() == text.back())
		text = text.substr(1, text.size() - 2);
	return text;
}
bool strings(const std::string& kind, ModuleCursor& cursor)
{
	std::string a, b;
	if (kind == ".ifeqs" || kind == ".ifnes")
	{
		a = cursor.quoted();
		if (!cursor.take(','))
			cursor.fail("string conditional requires a comma");
		b = cursor.quoted();
		cursor.finish();
	}
	else
	{
		const std::string text = cursor.rest();
		size_t comma = std::string::npos;
		char quote = 0;
		bool escape = false;
		for (size_t i = 0; i < text.size(); ++i)
		{
			const char c = text[i];
			if (quote)
			{
				if (c == quote && !escape)
					quote = 0;
				escape = c == '\\' && !escape;
			}
			else if (c == '\'' || c == '"')
				quote = c;
			else if (c == ',')
			{
				comma = i;
				break;
			}
		}
		if (comma == std::string::npos)
			cursor.fail("string conditional requires two arguments");
		a = trim(text.substr(0, comma));
		b = trim(text.substr(comma + 1));
	}
	return (a == b) == (kind == ".ifc" || kind == ".ifeqs");
}
}
bool module_condition(const std::string& kind, ModuleCursor& cursor, const ModuleLine& line)
{
	if (kind == ".ifb" || kind == ".ifnb")
	{
		const bool blank = cursor.end();
		cursor.rest();
		return blank == (kind == ".ifb");
	}
	if (kind == ".ifc" || kind == ".ifnc" || kind == ".ifeqs" || kind == ".ifnes")
	{
		if ((!line.symbols.empty() || !line.labels.empty()))
			cursor.fail("typed symbol is not an absolute string expression");
		return strings(kind, cursor);
	}
	if (kind != ".if" && kind != ".ifne" && kind != ".ifeq" && kind != ".ifge" && kind != ".ifgt" && kind != ".ifle" &&
		kind != ".iflt")
		cursor.fail("module conditional is not implemented: " + kind);
	const int64_t value = static_cast<int64_t>(module_expression(line, cursor.position()));
	return kind == ".ifeq" ? value == 0
		: kind == ".ifge"  ? value >= 0
		: kind == ".ifgt"  ? value > 0
		: kind == ".ifle"  ? value <= 0
		: kind == ".iflt"  ? value < 0
						   : value != 0;
}
} // namespace assembly
