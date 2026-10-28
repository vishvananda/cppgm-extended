#include "preprocess/macro/macro_definition.h"

#include <cstring>

#include "preprocess/preprocess_error.h"

namespace {

bool IsPunctuatorToken(const PPToken& token, TokenKind kind)
{
	return token.kind == PPTokenKind::PreprocessingOpOrPunc && IsPunctuator(token.punctuator, kind);
}

bool SameSpelling(const PPToken& a, const PPToken& b)
{
	return a.kind == b.kind && a.length == b.length && std::memcmp(a.spelling, b.spelling, a.length) == 0;
}

// Clears the parameter slots that parsing set, also when it throws.
class ParameterSlotsGuard
{
public:
	explicit ParameterSlotsGuard(std::vector<Identifier*>& parameters) : parameters_(parameters) {}
	~ParameterSlotsGuard()
	{
		for (Identifier* parameter : parameters_)
			parameter->parameter_slot = 0;
	}

private:
	std::vector<Identifier*>& parameters_;
};

} // namespace

bool SameDefinition(const MacroDefinition& a, const MacroDefinition& b)
{
	if (a.builtin != b.builtin || a.function_like != b.function_like || a.variadic != b.variadic ||
		a.parameter_count != b.parameter_count || a.body_size != b.body_size)
	{
		return false;
	}
	for (uint16_t i = 0; i < a.parameter_count; ++i)
	{
		if (a.parameters[i] != b.parameters[i])
			return false;
	}
	for (uint32_t i = 0; i < a.body_size; ++i)
	{
		const PPToken& x = a.body[i].token;
		const PPToken& y = b.body[i].token;
		if (!SameSpelling(x, y))
			return false;
		// The white space before the first token only separates it from the
		// macro name or parameters.
		if (i != 0 && (x.flags & kLeadingSpace) != (y.flags & kLeadingSpace))
			return false;
	}
	return true;
}

MacroDefinition* MakeBuiltinMacro(Arena& arena, const Identifier* name, BuiltinMacro builtin)
{
	MacroDefinition* macro = arena.allocate_array<MacroDefinition>(1);
	macro->name = name;
	macro->location = 0;
	macro->body = nullptr;
	macro->body_size = 0;
	macro->parameters = nullptr;
	macro->parameter_uses = nullptr;
	macro->parameter_count = 0;
	macro->function_like = false;
	macro->variadic = false;
	macro->builtin = builtin;
	return macro;
}

MacroDefinition* MacroDefinitionParser::parse(DefinitionTokens& tokens, const PPToken& name)
{
	body_.clear();
	parameters_.clear();
	variadic_ = false;
	ParameterSlotsGuard guard(parameters_);

	// `identifier(` with no white space in between starts a parameter list;
	// an object-like macro needs white space before its replacement list
	// (16.3p3, the course grammar).
	PPToken token = tokens.next();
	const bool function_like = IsPunctuatorToken(token, TokenKind::OP_LPAREN) && (token.flags & kLeadingSpace) == 0;
	if (function_like)
	{
		parse_parameters(tokens, token);
		token = tokens.next();
	}
	else if (token.kind != PPTokenKind::NewLine && (token.flags & kLeadingSpace) == 0)
	{
		throw PreprocessError(token.begin, "missing white space after the macro name");
	}

	read_replacement_list(tokens, token);
	tokens.allow_va_args(false);
	if (function_like)
		assign_function_like_roles();
	assign_paste_roles(function_like);
	return finish(name, function_like);
}

void MacroDefinitionParser::parse_parameters(DefinitionTokens& tokens, const PPToken& open)
{
	// `( )`, `( identifier-list )`, `( ... )`, `( identifier-list , ... )`
	// (16.3p1) or, in hosted mode, GNU `( identifier-list ... )`.
	PPToken token = tokens.next();
	if (IsPunctuatorToken(token, TokenKind::OP_RPAREN))
		return;
	for (;;)
	{
		if (IsPunctuatorToken(token, TokenKind::OP_DOTS))
		{
			// __VA_ARGS__ names the variable arguments, the last parameter,
			// and may now appear.
			variadic_ = true;
			parameters_.push_back(va_args_);
			va_args_->parameter_slot = static_cast<uint16_t>(parameters_.size());
			tokens.allow_va_args(true);
			token = tokens.next();
			if (!IsPunctuatorToken(token, TokenKind::OP_RPAREN))
				throw PreprocessError(token.begin, "`...` must end the macro parameters");
			return;
		}
		if (token.kind != PPTokenKind::Identifier)
			throw PreprocessError(
				token.kind == PPTokenKind::NewLine ? open.begin : token.begin, "expected a macro parameter name");
		if (token.identifier->parameter_slot != 0)
			throw PreprocessError(token.begin, "duplicate macro parameter name");
		if (parameters_.size() >= 0xFFFF)
			throw PreprocessError(token.begin, "too many macro parameters");
		parameters_.push_back(token.identifier);
		token.identifier->parameter_slot = static_cast<uint16_t>(parameters_.size());

		token = tokens.next();
		if (IsPunctuatorToken(token, TokenKind::OP_RPAREN))
			return;
		// GNU `identifier ...` (hosted mode; C++11's grammar has no such
		// form, so course mode rejects it): the named parameter takes the
		// variable arguments, as __VA_ARGS__ would.
		if (hosted_ && IsPunctuatorToken(token, TokenKind::OP_DOTS))
		{
			variadic_ = true;
			token = tokens.next();
			if (!IsPunctuatorToken(token, TokenKind::OP_RPAREN))
				throw PreprocessError(token.begin, "`...` must end the macro parameters");
			return;
		}
		if (!IsPunctuatorToken(token, TokenKind::OP_COMMA))
			throw PreprocessError(token.kind == PPTokenKind::NewLine ? open.begin : token.begin,
				"expected `,` or `)` in the macro parameters");
		token = tokens.next();
	}
}

void MacroDefinitionParser::read_replacement_list(DefinitionTokens& tokens, PPToken token)
{
	while (token.kind != PPTokenKind::NewLine)
	{
		ReplacementToken replacement;
		replacement.token = token;
		replacement.token.flags &= kLeadingSpace;
		replacement.role = ReplacementRole::Token;
		replacement.parameter = 0;
		body_.push_back(replacement);
		token = tokens.next();
	}
}

void MacroDefinitionParser::assign_function_like_roles()
{
	for (std::size_t i = 0; i < body_.size(); ++i)
	{
		ReplacementToken& replacement = body_[i];
		const PPToken& token = replacement.token;
		if (token.kind == PPTokenKind::Identifier && token.identifier->parameter_slot != 0)
		{
			replacement.role = ReplacementRole::Parameter;
			replacement.parameter = static_cast<uint16_t>(token.identifier->parameter_slot - 1);
		}
		else if (token.kind == PPTokenKind::PreprocessingOpOrPunc && token.punctuator == kHashPunctuator)
		{
			// Each # shall be followed by a parameter (16.3.2p1).
			if (i + 1 == body_.size() || body_[i + 1].token.kind != PPTokenKind::Identifier ||
				body_[i + 1].token.identifier->parameter_slot == 0)
			{
				throw PreprocessError(token.begin, "`#` is not followed by a macro parameter");
			}
			replacement.role = ReplacementRole::Stringize;
			replacement.parameter = static_cast<uint16_t>(body_[i + 1].token.identifier->parameter_slot - 1);
			body_[i + 1].role = ReplacementRole::StringizedParameter;
			body_[i + 1].parameter = replacement.parameter;
			++i;
		}
	}
}

void MacroDefinitionParser::assign_paste_roles(bool function_like)
{
	for (std::size_t i = 0; i < body_.size(); ++i)
	{
		const PPToken& token = body_[i].token;
		if (token.kind != PPTokenKind::PreprocessingOpOrPunc || token.punctuator != kHashHashPunctuator)
			continue;
		// A ## shall not begin or end a replacement list (16.3.3p1).
		if (i == 0 || i + 1 == body_.size())
			throw PreprocessError(token.begin, "`##` cannot begin or end a replacement list");
		// A ## right after a ## operator is that operator's right operand
		// (16.3.3p3), pasted as a token.
		if (body_[i - 1].role == ReplacementRole::Paste || body_[i - 1].role == ReplacementRole::CommaPaste)
			continue;
		body_[i].role = ReplacementRole::Paste;
		if (!function_like)
			continue;
		ReplacementToken& left = body_[i - 1];
		ReplacementToken& right = body_[i + 1];
		if (left.role == ReplacementRole::Parameter)
			left.role = ReplacementRole::RawParameter;
		if (right.role == ReplacementRole::Parameter)
			right.role = ReplacementRole::RawParameter;
		// In `, ## __VA_ARGS__` the ## pastes nothing: the variable arguments
		// as written follow the comma, or remove it when they are empty.
		if (variadic_ && right.role == ReplacementRole::RawParameter && right.parameter + 1u == parameters_.size() &&
			left.role == ReplacementRole::Token && left.token.kind == PPTokenKind::PreprocessingOpOrPunc &&
			IsPunctuator(left.token.punctuator, TokenKind::OP_COMMA))
		{
			body_[i].role = ReplacementRole::CommaPaste;
		}
	}
}

MacroDefinition* MacroDefinitionParser::finish(const PPToken& name, bool function_like)
{
	MacroDefinition* macro = arena_.allocate_array<MacroDefinition>(1);
	macro->name = name.identifier;
	macro->location = name.begin;
	macro->body = arena_.copy_array(body_.data(), body_.size());
	macro->body_size = static_cast<uint32_t>(body_.size());
	macro->parameters = arena_.copy_array(parameters_.data(), parameters_.size());
	macro->parameter_count = static_cast<uint16_t>(parameters_.size());
	uint8_t* uses = arena_.allocate_array<uint8_t>(parameters_.size());
	for (std::size_t i = 0; i < parameters_.size(); ++i)
		uses[i] = 0;
	for (const ReplacementToken& replacement : body_)
	{
		if (replacement.role == ReplacementRole::Parameter)
			uses[replacement.parameter] |= kParameterExpanded;
		else if (replacement.role == ReplacementRole::RawParameter || replacement.role == ReplacementRole::Stringize)
			uses[replacement.parameter] |= kParameterRaw;
	}
	macro->parameter_uses = uses;
	macro->function_like = function_like;
	macro->variadic = variadic_;
	macro->builtin = BuiltinMacro::None;
	return macro;
}
