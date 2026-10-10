#pragma once

#include <ostream>

#include "syntax/ast.h"

namespace syntax {

// Writes the PA5 dump of one translation unit's tree, rooted at
// `translation-unit`: one node per line, indented two spaces per level,
// with the checked-in fixtures' labels and leaf-token formats. Output is
// buffered in bounded chunks (deep nesting makes the indentation alone
// quadratic). The dump is a view; nothing reads it back.
void PrintTranslationUnit(const TranslationUnit& unit, std::ostream& out);

} // namespace syntax
