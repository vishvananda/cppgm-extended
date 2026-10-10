#pragma once

#include "syntax/ast.h"

// What semantic analysis established about statements beyond their
// expressions' facts (see sema/expression_facts.h).
namespace sema {

// 6.5.4p1: a range-based for statement is its equivalent
//   { auto&& __range = range-init;
//     for (auto __begin = begin-expr, __end = end-expr; __begin != __end;
//          ++__begin) { for-range-declaration = *__begin; statement } }
// whose implicit declarations and expressions the analysis made once,
// typed as written ones (their variables' names are not found by lookup).
// An array range takes the course's form: an index `__idx` from 0 while
// less than the bound, the element `__range[__idx]`. A range that is an
// id-expression naming a variable has no __range: each use names the
// variable again (the course's form; the name has no effect to repeat).
struct RangeForFacts
{
	// `auto&& __range = range-init;` (a braced-init-list's hidden array
	// `const E __range[] = {...};`), or null.
	const syntax::InitDeclarator* range;
	// `auto __begin = begin-expr;` or `int __idx = 0;`, and `auto __end =
	// end-expr;` (null for an array).
	const syntax::InitDeclarator* begin;
	const syntax::InitDeclarator* end;
	// `__begin != __end` converted to bool, or `__idx < bound`; and
	// `++__begin` or `++__idx`.
	const syntax::Expr* condition;
	const syntax::Expr* increment;
	// The for-range-declaration initialized by `*__begin` (or the element).
	syntax::InitDeclarator declaration;
};

} // namespace sema
