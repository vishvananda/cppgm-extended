#pragma once

// The syntax tree of one translation unit, built by syntax/parser.h and
// consumed directly by later semantic stages (the PA5 dump is a view of it,
// syntax/ast_printer.h). Nodes are typed structs allocated in the
// translation unit's syntax arena and never destroyed individually; variable
// children are arena slices; identifiers are interned (Identifier*), and
// literal spellings view the preprocessor's stable token spellings. Every
// node keeps its translation-unit location (the first token's begin).
//
// The tree is source-faithful: it keeps the syntactic distinctions later
// stages and the dump need (parentheses, the written form of casts and
// initializers, where a pack ellipsis appeared), and leaves unresolved what
// the parser's name categories cannot decide (a call may be a functional
// cast of a class type; a template argument may be a type or a value).

#include "syntax/ast_declarations.h"
