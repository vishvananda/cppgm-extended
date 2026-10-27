#pragma once

#include <cstddef>
#include <cstdint>
#include <vector>

#include "sema/constant_values.h"
#include "support/memory/arena.h"
#include "support/pointer_map.h"

namespace syntax {
struct Expr;
}

// The state of constant evaluations (sema/constant_interpreter.h): the
// objects, names, calls and arguments of one evaluation, and the stacks
// the analyzer keeps for every evaluation of its unit.
namespace sema {

struct Conversion;
struct DataMember;
struct Entity;
struct ExprFacts;
struct FunctionEntity;

// An object of an evaluation (its value's storage is stable).
struct ConstObject
{
	ConstValue* value;
	const Type* type;
	uint32_t generation;
};

// A name bound in a frame: a variable or parameter designating its
// object, or a reference designating its referent.
struct ConstLocal
{
	const Entity* entity;
	ConstPlace place;
};

// What a call yields: a class result initializes the object at `target`;
// a reference result designates `place`; a scalar is `value`.
struct ConstResult
{
	const ConstPlace* target;
	ConstPlace place;
	ConstValue value;
	// A complex or vector result's parts or lanes, made in the caller's
	// storage before the callee's region begins (the callee's ends before
	// the caller reads `value`, see invoke and returned).
	ConstValue* held;
};

// A constexpr call being evaluated.
struct ConstFrame
{
	const FunctionEntity* function;
	bool has_this;
	ConstPlace self;
	size_t first_local;
	ConstResult* result;
	// Highest logical call depth below this frame, including memo hits.
	size_t peak_depth;
};

// An argument of a call: an expression initializing its parameter as its
// recorded conversion says, an object bound to a reference parameter (or
// copied into a parameter object), or a value.
struct ConstArgument
{
	const syntax::Expr* expr;
	const Conversion* conversion;
	bool bound;
	ConstPlace place;
	bool valued;
	ConstValue value;
};

// A class operand's node showing its own facts while its conversion
// function's object is located (`converted` the node's facts otherwise).
struct ConstSwap
{
	const syntax::Expr* node;
	const ExprFacts* converted;
	const ExprFacts* original;
};

// A non-static data member's bytes in its class.
struct MemberSpan
{
	int64_t start;
	int64_t size;
	const DataMember* member;
};

struct SpanTable
{
	const MemberSpan* spans;
	uint32_t count;
	// The empty potentially-overlapping members (no bytes of their own),
	// sorted by offset and type, and their largest class size.
	const MemberSpan* empties;
	uint32_t empty_count;
	int64_t empty_extent;
};

// Where a statement, loop iteration or call begins: the objects and names
// it creates end with it, and so does the scratch they took.
struct ConstRegion
{
	size_t objects;
	size_t locals;
	Arena::Mark mark;
};

// The evaluations' objects, names and calls, owned by the analyzer and
// shared by nested evaluations, each popping what it pushed: an
// evaluation allocates nothing once they have grown.
struct ConstantStacks
{
	std::vector<ConstObject> objects;
	std::vector<ConstLocal> locals;
	std::vector<ConstFrame> frames;
	// The swaps in effect; those from `active` are applied (a call or a
	// nested evaluation sees every node with its own facts).
	std::vector<ConstSwap> swaps;
	size_t active = 0;
	// Object generations, never reused within the unit.
	uint32_t generation = 0;
	// The member spans of classes with many members, by class, sorted by
	// offset (a member found in logarithmic time).
	PointerMap span_index;
	std::vector<SpanTable> span_tables;
};

} // namespace sema
