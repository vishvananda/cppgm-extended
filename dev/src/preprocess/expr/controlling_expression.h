#pragma once

#include <cstdint>
#include <vector>

#include "lex/number_literal.h"
#include "preprocess/expr/pp_arithmetic.h"
#include "preprocess/tokens/pp_token_cursor.h"

// Answers `defined identifier` ([cpp.cond] 16.1p1) for the evaluator: PA3's
// mock, then the macro table of the preprocessor. `spelling` is borrowed for
// the duration of the call.
class MacroDefinitionQuery
{
public:
	virtual bool is_defined(const char* spelling, uint32_t length) const = 0;

protected:
	~MacroDefinitionQuery() {}
};

enum class ControllingExpressionStatus : uint8_t
{
	// Nothing but whitespace before the new-line.
	Empty,
	Valid,
	// A token that is not part of a controlling expression, a mismatch with
	// the grammar, or an evaluated operation without a value.
	Invalid
};

struct ControllingExpressionResult
{
	ControllingExpressionStatus status;
	// Valid: the value.
	PPValue value;
	// The expression was ended by the end of the file, not a new-line.
	bool end_of_file;
};

// Evaluates controlling expressions ([cpp.cond] 16.1) in one pass over the
// pp-tokens, which are converted as they arrive: integer and character
// literals of integral type become intmax_t or uintmax_t values (course
// rule: the signedness of the phase-7 type decides), `true` and `false` are
// 1 and 0, `defined` asks the MacroDefinitionQuery, every other identifier
// is 0, and operators and punctuators take their phase-7 meaning.
//
// Parsing is operator precedence with explicit stacks of pending values and
// operators, so nesting depth costs only stack entries; the stacks are
// reused between expressions. An operand that is not evaluated (the right
// operand of `&&` or `||` decided by its left, the unselected operand of
// `?:`) is still converted and typed, but an operation there without a
// value is not an error. The first error ends the expression, and the rest
// of its line is skipped.
class ControllingExpressionEvaluator
{
public:
	// `literals`: the integer literal forms read (see ClassifyNumber).
	explicit ControllingExpressionEvaluator(
		const MacroDefinitionQuery& macros, NumberDialect literals = NumberDialect::Cxx11);

	// Evaluates the pp-tokens from `tokens` up to and including the next
	// new-line or end-of-file. Propagates the cursor's exceptions.
	ControllingExpressionResult evaluate(PPTokenCursor& tokens);

private:
	enum class TokenClass : uint8_t
	{
		Operand,
		Defined,
		Operator,
		QuestionMark,
		Colon,
		LeftParen,
		RightParen,
		End,
		Invalid
	};

	// The current token: Operand has a value, Operator an operator (`+` and
	// `-` as Add and Subtract, whichever role they play).
	struct Token
	{
		TokenClass kind;
		PPOperator op;
		PPValue value;
	};

	enum class PendingKind : uint8_t
	{
		Operator,
		// `?` waiting for its `:`; the condition is the top value.
		Condition,
		// `? :` waiting for the third operand; the condition and second
		// operand are the top two values.
		Alternative,
		Group
	};

	struct Pending
	{
		PendingKind kind;
		PPOperator op;
	};

	const PPToken& pull();
	void read_token();
	void classify_operator(const PPToken& pp);
	void classify_number(const PPToken& pp);
	void classify_character(const PPToken& pp);
	void read_defined_operand();

	void parse();
	bool shift_in_operand_position();
	bool shift_in_operator_position();
	void shift_binary(PPOperator op);
	void shift_condition();
	void shift_alternative();
	void close_group();
	void finish();
	void reduce_operators();
	void reduce_top();
	void reduce_conditional();
	void fail_unless_unevaluated();

	const MacroDefinitionQuery& macros_;
	NumberDialect literals_;
	PPTokenCursor* tokens_;
	Token token_;
	// The new-line or end-of-file ending the expression has been read.
	bool at_line_end_;
	bool end_of_file_;
	bool failed_;
	// How many enclosing operands are not evaluated.
	uint32_t unevaluated_;
	std::vector<PPValue> values_;
	std::vector<Pending> pending_;
};
