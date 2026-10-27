#include "preprocess/expr/controlling_expression.h"

#include <cstring>

#include "lex/number_literal.h"
#include "lex/text_literal.h"
#include "lex/token_kind.h"

namespace {

// The course rule for integral literals in controlling expressions: these
// types act as uintmax_t, the other integral types as intmax_t.
bool IsUnsignedLiteralType(FundamentalType type)
{
	switch (type)
	{
	case FundamentalType::UnsignedChar:
	case FundamentalType::UnsignedShortInt:
	case FundamentalType::UnsignedInt:
	case FundamentalType::UnsignedLongInt:
	case FundamentalType::UnsignedLongLongInt:
	case FundamentalType::Char16T:
	case FundamentalType::Char32T:
		return true;
	default:
		return false;
	}
}

bool SpellingIs(const PPToken& pp, const char* text)
{
	const size_t length = std::strlen(text);
	return pp.length == length && std::memcmp(pp.spelling, text, length) == 0;
}

bool IsPunctuator(const PPToken& pp, char c)
{
	return pp.kind == PPTokenKind::PreprocessingOpOrPunc && pp.length == 1 && pp.spelling[0] == c;
}

} // namespace

ControllingExpressionEvaluator::ControllingExpressionEvaluator(
	const MacroDefinitionQuery& macros, NumberDialect literals)
	: macros_(macros), literals_(literals), tokens_(nullptr), token_(), at_line_end_(false), end_of_file_(false),
	  failed_(false), unevaluated_(0)
{}

ControllingExpressionResult ControllingExpressionEvaluator::evaluate(PPTokenCursor& tokens)
{
	tokens_ = &tokens;
	at_line_end_ = false;
	failed_ = false;
	unevaluated_ = 0;
	values_.clear();
	pending_.clear();

	ControllingExpressionResult result = ControllingExpressionResult();
	read_token();
	if (token_.kind == TokenClass::End)
	{
		result.status = ControllingExpressionStatus::Empty;
	}
	else
	{
		parse();
		while (!at_line_end_)
			pull();
		result.status = failed_ ? ControllingExpressionStatus::Invalid : ControllingExpressionStatus::Valid;
		if (!failed_)
			result.value = values_.back();
	}
	result.end_of_file = end_of_file_;
	return result;
}

// Token conversion

const PPToken& ControllingExpressionEvaluator::pull()
{
	for (;;)
	{
		const PPToken& pp = tokens_->next();
		if (pp.kind == PPTokenKind::WhitespaceSequence)
			continue;
		if (pp.kind == PPTokenKind::NewLine || pp.kind == PPTokenKind::EndOfFile)
		{
			at_line_end_ = true;
			end_of_file_ = pp.kind == PPTokenKind::EndOfFile;
		}
		return pp;
	}
}

void ControllingExpressionEvaluator::read_token()
{
	const PPToken& pp = pull();
	token_.kind = TokenClass::Invalid;
	switch (pp.kind)
	{
	case PPTokenKind::NewLine:
	case PPTokenKind::EndOfFile:
		token_.kind = TokenClass::End;
		return;
	case PPTokenKind::Identifier:
		// Keywords are identifiers here; `true` and `false` are the course's
		// 1 and 0, and every other identifier is replaced by 0 (16.1p4).
		if (SpellingIs(pp, "defined"))
		{
			token_.kind = TokenClass::Defined;
			return;
		}
		token_.kind = TokenClass::Operand;
		token_.value = PPValue{ SpellingIs(pp, "true") ? 1u : 0u, false };
		return;
	case PPTokenKind::PreprocessingOpOrPunc:
		classify_operator(pp);
		return;
	case PPTokenKind::PPNumber:
		classify_number(pp);
		return;
	case PPTokenKind::CharacterLiteral:
		classify_character(pp);
		return;
	default:
		// Strings, user-defined literals, header-names and other characters.
		return;
	}
}

void ControllingExpressionEvaluator::classify_operator(const PPToken& pp)
{
	// The lexer identifies each op-or-punc; `#` and `##` have no phase-7 kind.
	TokenKind kind;
	if (pp.punctuator == kHashPunctuator || pp.punctuator == kHashHashPunctuator)
		return;
	if (pp.punctuator != kNoPunctuator)
		kind = static_cast<TokenKind>(pp.punctuator - 1);
	else if (!FindSimpleToken(pp.spelling, pp.length, kind))
		return;
	token_.kind = TokenClass::Operator;
	switch (kind)
	{
	case TokenKind::OP_PLUS:
		token_.op = PPOperator::Add;
		return;
	case TokenKind::OP_MINUS:
		token_.op = PPOperator::Subtract;
		return;
	case TokenKind::OP_LNOT:
		token_.op = PPOperator::LogicalNot;
		return;
	case TokenKind::OP_COMPL:
		token_.op = PPOperator::Complement;
		return;
	case TokenKind::OP_STAR:
		token_.op = PPOperator::Multiply;
		return;
	case TokenKind::OP_DIV:
		token_.op = PPOperator::Divide;
		return;
	case TokenKind::OP_MOD:
		token_.op = PPOperator::Modulo;
		return;
	case TokenKind::OP_LSHIFT:
		token_.op = PPOperator::ShiftLeft;
		return;
	case TokenKind::OP_RSHIFT:
		token_.op = PPOperator::ShiftRight;
		return;
	case TokenKind::OP_LT:
		token_.op = PPOperator::Less;
		return;
	case TokenKind::OP_GT:
		token_.op = PPOperator::Greater;
		return;
	case TokenKind::OP_LE:
		token_.op = PPOperator::LessEqual;
		return;
	case TokenKind::OP_GE:
		token_.op = PPOperator::GreaterEqual;
		return;
	case TokenKind::OP_EQ:
		token_.op = PPOperator::Equal;
		return;
	case TokenKind::OP_NE:
		token_.op = PPOperator::NotEqual;
		return;
	case TokenKind::OP_AMP:
		token_.op = PPOperator::BitAnd;
		return;
	case TokenKind::OP_XOR:
		token_.op = PPOperator::BitXor;
		return;
	case TokenKind::OP_BOR:
		token_.op = PPOperator::BitOr;
		return;
	case TokenKind::OP_LAND:
		token_.op = PPOperator::LogicalAnd;
		return;
	case TokenKind::OP_LOR:
		token_.op = PPOperator::LogicalOr;
		return;
	case TokenKind::OP_QMARK:
		token_.kind = TokenClass::QuestionMark;
		return;
	case TokenKind::OP_COLON:
		token_.kind = TokenClass::Colon;
		return;
	case TokenKind::OP_LPAREN:
		token_.kind = TokenClass::LeftParen;
		return;
	case TokenKind::OP_RPAREN:
		token_.kind = TokenClass::RightParen;
		return;
	default:
		token_.kind = TokenClass::Invalid;
		return;
	}
}

void ControllingExpressionEvaluator::classify_number(const PPToken& pp)
{
	// Only an integer literal without ud-suffix has integral type.
	const NumberLiteral number = ClassifyNumber(pp.spelling, pp.length, literals_);
	if (number.kind != NumberLiteralKind::Integer)
		return;
	token_.kind = TokenClass::Operand;
	token_.value = PPValue{ number.integer, IsUnsignedLiteralType(number.type) };
}

void ControllingExpressionEvaluator::classify_character(const PPToken& pp)
{
	// Every valid character literal has an integral type whose values
	// include its code point.
	// Multicharacter literals stay outside the course's controlling
	// expressions.
	const CharacterLiteralValue character = EvaluateCharacterLiteral(
		pp.spelling, pp.length, literals_ == NumberDialect::Cxx11 ? CharacterDialect::Course : CharacterDialect::Gnu);
	if (!character.valid || character.multicharacter || character.suffix_begin != pp.length)
		return;
	token_.kind = TokenClass::Operand;
	uint64_t bits = character.code_point;
	// GNU numeric byte escapes have the target ABI's signed char value,
	// promoted to intmax_t in a controlling expression.
	if (character.type == FundamentalType::Char && bits >= 0x80)
		bits |= ~static_cast<uint64_t>(0xFF);
	token_.value = PPValue{ bits, IsUnsignedLiteralType(character.type) };
}

// `defined identifier` or `defined ( identifier )` (16.1p1); the identifier
// is any pp identifier, keywords included.
void ControllingExpressionEvaluator::read_defined_operand()
{
	const PPToken* name = &pull();
	const bool parenthesized = IsPunctuator(*name, '(');
	if (parenthesized)
		name = &pull();
	if (name->kind != PPTokenKind::Identifier)
	{
		failed_ = true;
		return;
	}
	const bool defined = macros_.is_defined(name->spelling, name->length);
	if (parenthesized && !IsPunctuator(pull(), ')'))
	{
		failed_ = true;
		return;
	}
	values_.push_back(PPValue{ defined ? 1u : 0u, false });
}

// Parsing and evaluation

void ControllingExpressionEvaluator::parse()
{
	bool want_operand = true;
	for (;;)
	{
		if (want_operand)
		{
			want_operand = shift_in_operand_position();
		}
		else if (token_.kind == TokenClass::End)
		{
			finish();
			return;
		}
		else
		{
			want_operand = shift_in_operator_position();
		}
		if (failed_)
			return;
		read_token();
	}
}

// An operand, a prefix operator or `(`. Returns whether an operand is still
// expected.
bool ControllingExpressionEvaluator::shift_in_operand_position()
{
	switch (token_.kind)
	{
	case TokenClass::Operand:
		values_.push_back(token_.value);
		return false;
	case TokenClass::Defined:
		read_defined_operand();
		return false;
	case TokenClass::Operator:
		if (token_.op == PPOperator::Add)
			pending_.push_back(Pending{ PendingKind::Operator, PPOperator::Plus });
		else if (token_.op == PPOperator::Subtract)
			pending_.push_back(Pending{ PendingKind::Operator, PPOperator::Minus });
		else if (IsPPUnaryOperator(token_.op))
			pending_.push_back(Pending{ PendingKind::Operator, token_.op });
		else
			failed_ = true;
		return true;
	case TokenClass::LeftParen:
		pending_.push_back(Pending{ PendingKind::Group, PPOperator::Plus });
		return true;
	default:
		failed_ = true;
		return true;
	}
}

// A binary operator, `?`, `:` or `)` after an operand. Returns whether an
// operand is expected next.
bool ControllingExpressionEvaluator::shift_in_operator_position()
{
	switch (token_.kind)
	{
	case TokenClass::Operator:
		if (IsPPUnaryOperator(token_.op))
			failed_ = true;
		else
			shift_binary(token_.op);
		return true;
	case TokenClass::QuestionMark:
		shift_condition();
		return true;
	case TokenClass::Colon:
		shift_alternative();
		return true;
	case TokenClass::RightParen:
		close_group();
		return false;
	default:
		failed_ = true;
		return false;
	}
}

void ControllingExpressionEvaluator::shift_binary(PPOperator op)
{
	// Operators of equal precedence are left-associative.
	const unsigned precedence = PPOperatorPrecedence(op);
	while (!pending_.empty() && pending_.back().kind == PendingKind::Operator &&
		PPOperatorPrecedence(pending_.back().op) >= precedence)
	{
		reduce_top();
	}
	// The right operand of `&&` or `||` is not evaluated when the left one
	// decides the result ([expr.log.and] 5.14p1, [expr.log.or] 5.15p1).
	const uint64_t left = values_.back().bits;
	if ((op == PPOperator::LogicalAnd && left == 0) || (op == PPOperator::LogicalOr && left != 0))
		++unevaluated_;
	pending_.push_back(Pending{ PendingKind::Operator, op });
}

void ControllingExpressionEvaluator::shift_condition()
{
	// The condition is a logical-or-expression. `?:` groups right to left,
	// so a pending conditional stays open.
	while (!pending_.empty() && pending_.back().kind == PendingKind::Operator)
		reduce_top();
	// Only one of the second and third operands is evaluated
	// ([expr.cond] 5.16p1).
	if (values_.back().bits == 0)
		++unevaluated_;
	pending_.push_back(Pending{ PendingKind::Condition, PPOperator::Plus });
}

void ControllingExpressionEvaluator::shift_alternative()
{
	// The second operand is a whole controlling-expression.
	reduce_operators();
	if (pending_.empty() || pending_.back().kind != PendingKind::Condition)
	{
		failed_ = true;
		return;
	}
	pending_.back().kind = PendingKind::Alternative;
	// The second operand was evaluated exactly if the third is not.
	if (values_[values_.size() - 2].bits == 0)
		--unevaluated_;
	else
		++unevaluated_;
}

void ControllingExpressionEvaluator::close_group()
{
	reduce_operators();
	if (pending_.empty() || pending_.back().kind != PendingKind::Group)
	{
		failed_ = true;
		return;
	}
	pending_.pop_back();
}

void ControllingExpressionEvaluator::finish()
{
	reduce_operators();
	// An unclosed `(` or a `?` without its `:`.
	if (!pending_.empty())
		failed_ = true;
}

// Completes every operation back to the innermost `(` or `?`.
void ControllingExpressionEvaluator::reduce_operators()
{
	while (!pending_.empty() &&
		(pending_.back().kind == PendingKind::Operator || pending_.back().kind == PendingKind::Alternative))
	{
		reduce_top();
	}
}

void ControllingExpressionEvaluator::reduce_top()
{
	const Pending top = pending_.back();
	pending_.pop_back();
	if (top.kind == PendingKind::Alternative)
	{
		reduce_conditional();
		return;
	}
	PPValue result;
	if (IsPPUnaryOperator(top.op))
	{
		if (!ApplyPPUnary(top.op, values_.back(), result))
			fail_unless_unevaluated();
		values_.back() = result;
		return;
	}
	const PPValue right = values_.back();
	values_.pop_back();
	const PPValue left = values_.back();
	if (!ApplyPPBinary(top.op, left, right, result))
		fail_unless_unevaluated();
	values_.back() = result;
	if ((top.op == PPOperator::LogicalAnd && left.bits == 0) || (top.op == PPOperator::LogicalOr && left.bits != 0))
		--unevaluated_;
}

void ControllingExpressionEvaluator::reduce_conditional()
{
	const PPValue if_false = values_.back();
	values_.pop_back();
	const PPValue if_true = values_.back();
	values_.pop_back();
	PPValue& condition = values_.back();
	if (condition.bits != 0)
		--unevaluated_;
	// Both operands take the usual arithmetic conversions (5.16p6), so the
	// type does not depend on which one is selected.
	const bool is_unsigned = if_true.is_unsigned || if_false.is_unsigned;
	condition = condition.bits != 0 ? if_true : if_false;
	condition.is_unsigned = is_unsigned;
}

void ControllingExpressionEvaluator::fail_unless_unevaluated()
{
	if (unevaluated_ == 0)
		failed_ = true;
}
