#pragma once

#include <cstddef>
#include <cstdint>

#include "sema/declaration_records.h"
#include "sema/semantic_unit.h"
#include "sema/template_records.h"
#include "support/small_list.h"
#include "syntax/ast.h"

// The analysis's working records (sema/analyzer.h): the frames of lambda
// bodies and function bodies being analyzed, pending gotos and access
// checks, overload candidates, implicit copy frames, and the deferral of a
// hosted wrapper's missing builtin.

namespace sema {

// A lambda-expression whose body is being analyzed (analyzer_lambdas.cpp):
// its closure and the closure's facts, and what it has captured so far
// (`this` apart: its member comes first).
struct LambdaFrame
{
	ClassEntity* closure;
	ClosureFacts* facts;
	const syntax::LambdaExpr* expr;
	bool this_captured;
	// A templated lambda's call operator specialization's body: its
	// captures were fixed with its closure (no more are added).
	bool sealed = false;
	SmallList<const Entity*, 8> captures;
};

// The function body (or namespace-scope initializer) being analyzed in
// semantics mode.
struct BodyContext
{
	FunctionEntity* function;
	// The declared return type; the type of `this` in a non-static member
	// function (9.3.2p1).
	const Type* result;
	const Type* this_type;
	// Enclosing iteration statements, and the innermost switch statement;
	// the try blocks, handlers (15p2) and statement expressions being
	// analyzed, which no switch label enters.
	uint32_t loops;
	uint32_t handled;
	SwitchContext* switch_statement;
	BodyContext* enclosing;
	// Where the body's labels and goto statements start on the analysis's
	// stacks of them (6.6.4: a goto names a label of its function).
	size_t first_label;
	size_t first_goto;
	// A template definition's body: its statement rules that need no types
	// (labels, jumps, bypassed initializations) are checked; the rest wait
	// for its instantiations.
	bool pattern;
	// A lambda's body whose return statements deduce its result (5.1.2p4,
	// as CWG 975 extends it to any body), or a function's whose declared
	// return type holds auto (the course's extension, C++14's 7.1.6.4p9):
	// until the first return statement, then each later one's deduction is
	// checked against the result (`deduced`).
	// `undeduced`: a return statement (or the rest of the body) a hosted
	// wrapper's deferral left untyped (see Analyzer::deferrable_statement):
	// the result is not deducible, whatever the typed ones deduced.
	bool deduce_result;
	bool deduced;
	bool undeduced;
	// The function's declared return type holding auto (null for a lambda).
	const Type* placeholder;
	// A lambda's function call operator's body: the lambda's frame.
	LambdaFrame* lambda;
	// The scope a try-block being analyzed is in (its handlers' too), for
	// 12.8p31's throw operand; null outside one.
	const Scope* try_parent;
	// A constructor's function-try-block's handlers are being analyzed: no
	// return statement appears in them (15.3p14).
	bool constructor_handlers;
	// A default member initializer's member, or a default argument's
	// function and parameter: the context of the closures typed in it
	// (Itanium 5.1.8; see Analyzer::number_closure).
	const Entity* closure_context;
	uint32_t default_argument;
};

// A goto statement waiting for its function's labels, and a label: where
// each stands (its block and the block's bypassable declarations so far,
// 6.7p3).
struct PendingGoto
{
	const Identifier* label;
	uint32_t location;
	const Scope* scope;
	uint32_t initialized;
};
typedef PendingGoto PendingLabel;

// The outcome of an overload resolution (13.3.3).
enum class Resolution : uint8_t
{
	Selected,
	NoViable,
	Ambiguous
};

// The types partial ordering compares (14.8.2.4p3): a call's parameters
// that have arguments, a non-static member's object parameter inserted
// when only one template is such a member (14.5.6.2p3) or not (a call
// with an object argument), or the function types (an address).
enum class Ordering : uint8_t
{
	Call,
	CallInserting,
	FunctionType
};

// One candidate function of an overload resolution (13.3): its implicit
// conversion sequences start at `first` in the resolution's scratch.
struct Candidate
{
	FunctionEntity* function;
	uint32_t first;
	// A function template specialization (13.3.3p1 prefers non-templates).
	bool from_template;
};

// A failed access check of a namespace-scope declaration waiting for the
// entity it declares (11p6-p7: a friend function's declaration is in the
// function): check_access's operands.
struct PendingAccess
{
	const Binding* binding;
	const ClassEntity* naming;
	const ClassEntity* object;
	const Scope* scope;
	uint32_t location;
};

// A class whose implicit copy and move members wait for those of its
// subobjects' classes (see implicit_copy_members): the next base and
// member to visit.
struct CopyFrame
{
	ClassEntity* entity;
	const BaseClass* base;
	const DataMember* member;
};

// A callee name a hosted wrapper's deferred call may use: one unqualified
// `__builtin_` identifier (whose lookup failed).
bool DeferredBuiltinName(const syntax::Name& name);

} // namespace sema
