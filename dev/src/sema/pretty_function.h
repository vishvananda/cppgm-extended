#pragma once

#include <string>

namespace sema {

struct FunctionEntity;
struct SemanticGraph;

// GCC's __PRETTY_FUNCTION__ in Clang's form: the function's
// declaration with its qualified name -- `static `/`virtual `, the result
// type (none for a constructor or destructor; `auto` for a placeholder
// result and a lambda's function call operator), the parameter types as
// the function type has them (8.3.5p5: an array or function parameter is a
// pointer) and the member's cv- and ref-qualifiers -- then the template
// parameters of its enclosing class specializations and its own, outermost
// first, with their arguments: `int *ns::Box<int, 3>::get(char) const [T =
// int, N = 3, U = char]`. An explicit specialization (14.7.3) binds no
// parameters (its definition is its own, not the template's). Choices that
// differ from Clang's: types print canonically (no typedef sugar), a class
// template specialization's default arguments are written out (Clang omits
// trailing ones equal to their defaults), and the signature is the
// specialization's, not the template's (`f(int) [T = int]`, Clang's and
// GCC's `f(T) [T = int]`).
std::string PrettyFunctionName(const FunctionEntity* function, const SemanticGraph& graph);
// The function's name as __func__ holds it (8.4.1p8): an operator function's
// and a conversion function's with GCC's and Clang's spacing (`operator
// new[]`, `operator const char *`).
std::string PredefinedFunctionName(const FunctionEntity* function);

} // namespace sema
