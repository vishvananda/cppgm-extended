#include "preprocess/tokens/pp_token_emitter.h"

#include <string>

#include "preprocess/lexer/pp_lexer.h"

namespace {

void EmitSpelledToken(IPPTokenStream& output, PPTokenKind kind, const std::string& text)
{
	switch (kind)
	{
	case PPTokenKind::HeaderName:
		output.emit_header_name(text);
		return;
	case PPTokenKind::Identifier:
		output.emit_identifier(text);
		return;
	case PPTokenKind::PPNumber:
		output.emit_pp_number(text);
		return;
	case PPTokenKind::CharacterLiteral:
		output.emit_character_literal(text);
		return;
	case PPTokenKind::UserDefinedCharacterLiteral:
		output.emit_user_defined_character_literal(text);
		return;
	case PPTokenKind::StringLiteral:
		output.emit_string_literal(text);
		return;
	case PPTokenKind::UserDefinedStringLiteral:
		output.emit_user_defined_string_literal(text);
		return;
	case PPTokenKind::PreprocessingOpOrPunc:
		output.emit_preprocessing_op_or_punc(text);
		return;
	case PPTokenKind::NonWhitespaceCharacter:
		output.emit_non_whitespace_char(text);
		return;
	case PPTokenKind::WhitespaceSequence:
	case PPTokenKind::NewLine:
	case PPTokenKind::EndOfFile:
	case PPTokenKind::Placemarker:
		break;
	}
}

} // namespace

void EmitPPTokens(const SourceBuffer& source, IPPTokenStream& output)
{
	PPLexer lexer(source);
	SourceLineCursor lines(source);
	std::string text;
	for (;;)
	{
		const PPToken& token = lexer.next();
		if (token.kind == PPTokenKind::EndOfFile)
		{
			output.emit_eof();
			return;
		}
		if (token.kind == PPTokenKind::WhitespaceSequence)
		{
			output.emit_whitespace_sequence();
			continue;
		}

		const SourcePosition position = lines.advance_to(token.begin);
		output.set_source_location(position.line, position.column);
		if (token.kind == PPTokenKind::NewLine)
		{
			output.emit_new_line();
			continue;
		}
		text.assign(token.spelling, token.length);
		EmitSpelledToken(output, token.kind, text);
	}
}
