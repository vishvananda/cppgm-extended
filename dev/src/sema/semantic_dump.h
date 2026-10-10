#pragma once

#include <ostream>
#include <string>

#include "sema/analyzer.h"

namespace sema {

// Appends the course spelling of a type (scopes-and-types.md): `const
// pointer to int`, `array of 0 T` for an unknown bound, `function of (P1,
// ...) const & returning R`, `member-pointer of C to T`, `struct C`,
// `enum class E`, `typename T`.
void AppendTypeSpelling(const Type* type, std::string& out);

// Writes the PA6 `--emit-types` view of one translation unit's graph,
// rooted at `translation-unit`: each scope's bindings in declaration order,
// then its child scopes in the order they were opened, indented two spaces
// per level. The dump is a view; nothing reads it back.
void PrintTypesDump(const SemanticGraph& graph, std::ostream& out);

} // namespace sema
