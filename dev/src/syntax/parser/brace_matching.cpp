#include "syntax/parser/brace_matching.h"

#include "support/exceptions.h"

#include <limits>

namespace cppgm
{
namespace syntax
{

std::vector<std::uint32_t> BuildDelimiterMatches(
	const std::vector<SyntaxToken>& tokens)
{
	const std::uint32_t no_match = std::numeric_limits<std::uint32_t>::max();
	if (tokens.size() >= no_match - 1)
		ThrowSyntaxResourceLimit("too many syntax tokens");
	std::vector<std::uint32_t> matches(tokens.size(), no_match);
	// The same immutable index serves class-body skipping and declaration
	// lookahead. Matching all delimiters lets lookahead skip nested arguments.
	std::vector<std::uint32_t> open_delimiters;
	for (std::size_t i = 0; i < tokens.size(); ++i)
	{
		const std::uint16_t kind = tokens[i].Kind();
		if (kind == static_cast<std::uint16_t>(OP_LBRACE) ||
			kind == static_cast<std::uint16_t>(OP_LPAREN) ||
			kind == static_cast<std::uint16_t>(OP_LSQUARE))
			open_delimiters.push_back(static_cast<std::uint32_t>(i));
		else if ((kind == static_cast<std::uint16_t>(OP_RBRACE) ||
			kind == static_cast<std::uint16_t>(OP_RPAREN) ||
			kind == static_cast<std::uint16_t>(OP_RSQUARE)) &&
			!open_delimiters.empty())
		{
			const std::uint16_t open = tokens[open_delimiters.back()].Kind();
			if ((open == OP_LBRACE && kind == OP_RBRACE) ||
				(open == OP_LPAREN && kind == OP_RPAREN) ||
				(open == OP_LSQUARE && kind == OP_RSQUARE))
				matches[open_delimiters.back()] = static_cast<std::uint32_t>(i);
			open_delimiters.pop_back();
		}
	}
	return matches;
}

}
}
