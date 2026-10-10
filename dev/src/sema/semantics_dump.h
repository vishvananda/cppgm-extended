#pragma once

#include <ostream>

#include "sema/analyzer.h"

namespace sema {

// Writes the PA7 `--emit-semantics` view of one translation unit analyzed in
// semantics mode, rooted at `translation-unit`: its declarations in source
// order with the resolved statements and expressions of function bodies
// (the facts analysis attached to the tree), then the implicitly defined
// and inline member functions the unit odr-uses, in demand order. The dump
// is a view; nothing reads it back.
void PrintSemanticsDump(const syntax::TranslationUnit& unit, const SemanticGraph& graph, std::ostream& out);

} // namespace sema
