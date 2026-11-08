#include "asm/module_expand.h"

#include "asm/module_expression.h"

namespace assembly {
namespace {
bool loop(const std::string& name)
{
	return name == ".rept" || name == ".irp" || name == ".irpc";
}
std::string head(const ModuleLine& line)
{
	ModuleCursor cursor(line);
	cursor.space();
	if (cursor.end() || line.text[cursor.position()] == '\\')
		return {};
	return cursor.word();
}
}
std::string module_macro_name(std::string name)
{
	for (char& c : name)
		if (c >= 'A' && c <= 'Z')
			c = static_cast<char>(c + ('a' - 'A'));
	return name;
}
size_t module_block_end(const std::vector<ModuleLine>& input, size_t first, size_t last)
{
	std::vector<std::string> closing;
	for (size_t i = first; i < last; ++i)
	{
		const std::string name = head(input[i]);
		if (name == ".macro")
			closing.push_back(".endm");
		else if (loop(name))
			closing.push_back(".endr");
		else if (name == ".endm" || name == ".endr")
		{
			if (closing.empty() || closing.back() != name)
				throw AssemblyError{ "mismatched module expansion block", input[i].line, 1 };
			closing.pop_back();
			if (closing.empty())
				return i;
		}
		if (closing.size() > 64)
			throw AssemblyError{ "module block nesting exceeds limit", input[i].line, 1 };
	}
	throw AssemblyError{ "unterminated module expansion block", input[first].line, 1 };
}
std::vector<ModuleLine> ModuleExpander::expand(const std::vector<ModuleLine>& input)
{
	std::vector<ModuleLine> result;
	for (const ModuleLine& line : input)
		account(line.text.size() + 1, line);
	if (range(input, 0, input.size(), result, 0))
		throw AssemblyError{ ".exitm outside macro expansion", 1, 1 };
	return result;
}
void ModuleExpander::account(uint64_t bytes, const ModuleLine& line)
{
	const uint64_t limit = 64u * 1024u * 1024u;
	if (bytes > limit || bytes_ > limit - bytes)
		throw AssemblyError{ "module expansion exceeds byte budget", line.line, 1 };
	bytes_ += bytes;
}
void ModuleExpander::emit(const ModuleLine& line, std::vector<ModuleLine>& output)
{
	account(line.text.size() + 1, line);
	output.push_back(line);
}
bool ModuleExpander::range(
	const std::vector<ModuleLine>& input, size_t first, size_t last, std::vector<ModuleLine>& output, unsigned depth)
{
	if (depth > 64)
		throw AssemblyError{ "module expansion nesting exceeds limit", 1, 1 };
	for (size_t i = first; i < last; ++i)
	{
		const ModuleLine& line = input[i];
		if (++work_ > 1000000)
			throw AssemblyError{ "module expansion work exceeds limit", line.line, 1 };
		ModuleCursor cursor(line);
		const std::string name = cursor.word();
		if (name == ".macro" || loop(name))
		{
			const size_t end = module_block_end(input, i, last);
			if (name == ".macro")
				define(line, input, i + 1, end);
			else
				repeat(
					name, line, std::vector<ModuleLine>(input.begin() + i + 1, input.begin() + end), output, depth + 1);
			i = end;
		}
		else if (name.compare(0, 3, ".if") == 0)
		{
			bool taken = false, after_else = false;
			bool enabled = module_condition(name, cursor, line);
			size_t begin = i + 1;
			unsigned nested = 0;
			bool closed = false;
			for (size_t j = begin; j < last; ++j)
			{
				ModuleCursor branch(input[j]);
				const std::string part = head(input[j]);
				if (part.empty())
					continue;
				branch.word();
				if (part == ".macro" || loop(part))
				{
					j = module_block_end(input, j, last);
					continue;
				}
				if (part.compare(0, 3, ".if") == 0)
				{
					if (++nested > 64)
						branch.fail("module conditional nesting exceeds limit");
				}
				else if (part == ".endif" && nested != 0)
					--nested;
				else if (nested == 0 && (part == ".else" || part == ".elseif" || part == ".endif"))
				{
					if (enabled && !taken)
					{
						if (range(input, begin, j, output, depth + 1))
							return true;
						taken = true;
					}
					if (part == ".endif")
					{
						branch.finish();
						i = j;
						closed = true;
						break;
					}
					if (after_else)
						branch.fail("conditional branch after .else");
					after_else = part == ".else";
					if (after_else)
					{
						branch.finish();
						enabled = true;
					}
					else
						enabled = !taken && module_condition(".if", branch, input[j]);
					begin = j + 1;
				}
			}
			if (!closed)
				cursor.fail("unterminated module conditional");
		}
		else if (name == ".endm" || name == ".endr" || name == ".else" || name == ".elseif" || name == ".endif")
			cursor.fail("unmatched module expansion directive");
		else if (name == ".exitm")
		{
			cursor.finish();
			return true;
		}
		else if (name == ".purgem")
		{
			if (macros_.erase(module_macro_name(cursor.word())) == 0)
				cursor.fail("cannot purge an undefined macro");
			cursor.finish();
		}
		else
		{
			const auto found = macros_.find(module_macro_name(name));
			if (found != macros_.end() && !cursor.take(':'))
				invoke(line, cursor.position(), found->second, output, depth + 1);
			else
				emit(line, output);
		}
	}
	return false;
}
} // namespace assembly
