#pragma once

#include "syntax/model/arena.h"

#include <cstddef>
#include <vector>

namespace cppgm
{
namespace syntax
{

// A GNU attribute is consumed as raw tokens, so an argument that is not a
// literal cannot be given meaning here.  `aligned` is the one attribute whose
// argument is a constant expression the language already knows how to
// evaluate, so its token range is handed back for the parser to re-parse with
// the ordinary expression grammar.
struct GnuAttributeExpressionArgument
{
	syntax::NodeId attribute;
	std::size_t begin;
	std::size_t end;
};

bool ConsumeLeadingGnuObjectAttribute(
	const std::vector<syntax::SyntaxToken>& tokens,
	const syntax::StringTable& strings,
	syntax::SyntaxArena& arena, std::size_t* position,
	std::vector<syntax::NodeId>* attributes,
	std::vector<GnuAttributeExpressionArgument>* expression_arguments);
bool ConsumeLeadingStandardObjectAttribute(
	const std::vector<syntax::SyntaxToken>& tokens,
	const syntax::StringTable& strings,
	syntax::SyntaxArena& arena, std::size_t* position,
	std::vector<syntax::NodeId>* attributes);

}
}
