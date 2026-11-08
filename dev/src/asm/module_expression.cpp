#include "asm/module_expression.h"

#include <climits>
#include <cstring>

namespace assembly {
namespace {
class Expression
{
public:
	explicit Expression(const ModuleLine& line, uint32_t first) : line_(line), cursor_(line) { cursor_.advance(first); }
	uint64_t run()
	{
		const uint64_t result = binary(1, 0);
		cursor_.finish();
		return result;
	}

private:
	struct Operator
	{
		const char* text;
		unsigned precedence;
	};
	Operator operation();
	uint64_t primary(unsigned depth);
	uint64_t binary(unsigned minimum, unsigned depth);
	uint64_t apply(const std::string& op, uint64_t a, uint64_t b);
	const ModuleLine& line_;
	ModuleCursor cursor_;
};
Expression::Operator Expression::operation()
{
	cursor_.space();
	static const Operator operators[] = { { "||", 1 }, { "&&", 2 }, { "==", 3 }, { "!=", 3 }, { "<>", 3 }, { "<=", 3 },
		{ ">=", 3 }, { "<<", 6 }, { ">>", 6 }, { "+", 4 }, { "-", 4 }, { "<", 3 }, { ">", 3 }, { "|", 5 }, { "&", 5 },
		{ "^", 5 }, { "!", 5 }, { "*", 6 }, { "/", 6 }, { "%", 6 } };
	for (const Operator& op : operators)
		if (line_.text.compare(cursor_.position(), std::strlen(op.text), op.text) == 0)
			return op;
	return { "", 0 };
}
uint64_t Expression::primary(unsigned depth)
{
	if (depth > 64)
		cursor_.fail("module expression nesting exceeds limit");
	if (cursor_.take('-'))
		return uint64_t(0) - primary(depth + 1);
	if (cursor_.take('~'))
		return ~primary(depth + 1);
	if (cursor_.take('+'))
		return primary(depth + 1);
	if (cursor_.take('('))
	{
		const uint64_t value = binary(1, depth + 1);
		if (!cursor_.take(')'))
			cursor_.fail("unclosed module expression");
		return value;
	}
	cursor_.space();
	if (cursor_.symbol_use())
		cursor_.fail("absolute expression cannot use a relocatable symbol");
	return cursor_.number();
}
uint64_t Expression::binary(unsigned minimum, unsigned depth)
{
	uint64_t result = primary(depth);
	for (;;)
	{
		const Operator op = operation();
		if (op.precedence < minimum)
			return result;
		cursor_.advance(static_cast<uint32_t>(std::strlen(op.text)));
		result = apply(op.text, result, binary(op.precedence + 1, depth + 1));
	}
}
uint64_t Expression::apply(const std::string& op, uint64_t a, uint64_t b)
{
	if (op == "+")
		return a + b;
	if (op == "-")
		return a - b;
	if (op == "*")
		return a * b;
	if (op == "&")
		return a & b;
	if (op == "|")
		return a | b;
	if (op == "^")
		return a ^ b;
	if (op == "!")
		return a | ~b;
	if (op == "&&")
		return a != 0 && b != 0;
	if (op == "||")
		return a != 0 || b != 0;
	if (op == "<<" || op == ">>")
	{
		if (b >= 64)
			cursor_.fail("module expression shift exceeds 63 bits");
		return op == "<<" ? a << b : a >> b;
	}
	if (op == "/" || op == "%")
	{
		if (b == 0)
			cursor_.fail("division by zero in module expression");
		if (a == (uint64_t(1) << 63) && b == UINT64_MAX)
			return op == "/" ? a : 0;
		return static_cast<uint64_t>(op == "/" ? static_cast<int64_t>(a) / static_cast<int64_t>(b)
											   : static_cast<int64_t>(a) % static_cast<int64_t>(b));
	}
	const int64_t x = static_cast<int64_t>(a), y = static_cast<int64_t>(b);
	const bool truth = op == "=="  ? x == y
		: op == "!=" || op == "<>" ? x != y
		: op == "<"                ? x < y
		: op == ">"                ? x > y
		: op == "<="               ? x <= y
								   : x >= y;
	return truth ? UINT64_MAX : 0;
}
}
uint64_t module_expression(const ModuleLine& line, uint32_t first)
{
	return Expression(line, first).run();
}
} // namespace assembly
