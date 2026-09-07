#pragma once

#include "syntax/model/arena.h"

#include <cstdint>
#include <vector>

namespace cppgm
{
namespace syntax
{

std::vector<std::uint32_t> BuildDelimiterMatches(
	const std::vector<SyntaxToken>& tokens);

}
}
