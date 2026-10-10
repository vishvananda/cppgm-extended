#pragma once

#include "lex/gnu_builtins.h"
#include "sema/entity_records.h"

// Function entities (sema/entity_records.h's Entity kinds): builtins, a
// constructor's plan of subobject initializations, and the literal
// operators' names.
namespace sema {

struct TemplateHead;
struct FunctionEntity;

// A function's template facts, allocated only for the functions
// that have some: a function template's head, a specialization's
// arguments (in its template's head order; its mangled name encodes
// them), an instance member's pattern (the pattern class's member whose
// definition it instantiates).
struct FunctionTemplateFacts
{
	TemplateHead* head;
	const Type* const* arguments;
	FunctionEntity* pattern;
	// The depth of the instantiation that made the function (its
	// definition's instantiation runs one deeper).
	uint32_t depth;
	// A function template declared with a trailing return type: its
	// deduction substitutes the parameters before it (14.8.2p7).
	bool trailing_result;
};

// A function's call effects (GNU pure and const, pa28), ordered from the
// conservative default: LowIR's effects=readonly and effects=readnone.
enum class FunctionEffect : uint8_t
{
	Default,
	ReadOnly,
	ReadNone
};

// The copy and move special member functions (12.8p2-p3, p17, p19): a
// constructor whose first parameter is a reference to its class (the
// others with default arguments), an operator= of one parameter of its
// class or a reference to it.
enum class CopyKind : uint8_t
{
	None,
	CopyConstructor,
	MoveConstructor,
	CopyAssignment,
	MoveAssignment
};

// The replaceable global allocation and deallocation functions (3.7.4p2,
// 18.6.1.1-18.6.1.2): the runtime's unless the program defines one.
enum class AllocationRole : uint8_t
{
	None,
	New,
	NewArray,
	Delete,
	DeleteArray
};

// A compiler builtin a predeclared function stands for.
enum class BuiltinFunction : uint8_t
{
	None,
	Abort,
	// void* (void*, const void*, size_t): non-overlapping and overlapping
	// byte copies.
	Memcpy,
	Memmove,
	// size_t (const char*): the length of a null-terminated string (reads
	// memory only).
	Strlen,
	// void (): control does not reach the call (lowered to `unreachable`).
	Unreachable,
	// long (long value, long expected): the value (a branch hint).
	Expect,
	// void (): GCC's __builtin_trap, abnormal termination (lowered to
	// `unreachable`, which the native backend expands to a trap
	// instruction).
	Trap,
	// GCC's builtin form of a C library function (__builtin_memcmp ...):
	// a call of the library function its name without `__builtin_` names
	// (hosted mode).
	Library,
	// A builtin the call analysis types itself (GnuBuiltin's intrinsic and
	// query forms), never called as a function.
	Intrinsic
};

// A builtin the analysis types and lowering expands in place (GCC's
// floating classification and comparisons, and its NaN and infinity
// constants of the call's type): the kFolded-like ExprFacts::value of a
// kIntrinsic call, its low byte (the __sync builtins' implied memory order
// + 1 above it).
enum class Intrinsic : uint8_t
{
	IsFinite,
	IsInf,
	IsNan,
	IsNormal,
	FpClassify,
	// __builtin_signbit (x): the sign bit of a floating x (a NaN's too).
	SignBit,
	// __builtin_isinf_sign (x): -1 for a negative infinity, 1 for a
	// positive one, else 0.
	IsInfSign,
	// __builtin_issignaling (x): x is a signaling NaN (its quiet bit clear).
	IsSignaling,
	// __builtin_isgreater ... isunordered (x, y): the quiet comparisons of
	// two operands of their common floating type.
	IsGreater,
	IsGreaterEqual,
	IsLess,
	IsLessEqual,
	IsLessGreater,
	IsUnordered,
	Nan,
	NanSignaling,
	Inf,
	// __builtin_addressof(lvalue): its address, never an overloaded
	// operator& (std::addressof).
	AddressOf,
	// __builtin_bswap16/32/64 (x): x's bytes reversed, of its unsigned type.
	ByteSwap,
	// __builtin_clz/ctz/popcount[l|ll] (x): an int, of x's unsigned type.
	CountLeadingZeros,
	CountTrailingZeros,
	PopCount,
	// __builtin_clzg/ctzg/popcountg (x[, zero]): an int, of x's own
	// unsigned type; clzg and ctzg give `zero` for 0 when it is given.
	GenericCountLeadingZeros,
	GenericCountTrailingZeros,
	GenericPopCount,
	// __builtin_alloca(size): stack storage of the function's frame.
	Alloca,
	// __builtin_va_start(list, last), __builtin_va_end(list) and
	// __builtin_va_copy(to, from) of a variadic function's arguments.
	VaStart,
	VaEnd,
	VaCopy,
	// __builtin_prefetch(address[, rw[, locality]]): the address operand's
	// effects only.
	Prefetch,
	// __builtin_assume_aligned(pointer, alignment[, offset]): the pointer
	// (as void*).
	AssumeAligned,
	// __builtin_flt_rounds(): FLT_ROUNDS (18.3.2.3) of the current
	// rounding mode.
	FltRounds,
	// __builtin_object_size and __builtin_dynamic_object_size(pointer,
	// type): the size_t GCC documents for an object it does not determine,
	// all ones, or 0 when bit 8 of the facts' value is set (type 2 or 3);
	// the pointer is not evaluated, and the call is no constant expression
	// (as in both host compilers).
	ObjectSize,
	// __builtin_complex(re, im): the complex value of two parts of one
	// floating type.
	MakeComplex,
	// __builtin_reduce_and/or/xor/add/mul/max/min(v): the lanes of a vector
	// combined, a value of their type.
	ReduceAnd,
	ReduceOr,
	ReduceXor,
	ReduceAdd,
	ReduceMul,
	ReduceMax,
	ReduceMin,
	// GCC's __builtin_ia32_vec_init/ext/set (kGnuVectorLaneOperations):
	// the vector of the operands as its lanes; the lane the constant
	// selector (bits 8-15 of the facts' value) designates, and the vector
	// with that lane replaced.
	VectorInit,
	VectorExtract,
	VectorInsert,
	// __builtin_add/sub/mul_overflow(a, b, result): the infinitely precise
	// a op b stored through `result` modulo its type, and whether that
	// changed it.
	AddOverflow,
	SubOverflow,
	MulOverflow,
	// __builtin_is_constant_evaluated(): true only in a constant
	// evaluation (false when lowered).
	IsConstantEvaluated,
	// GCC's __builtin_FILE(), __builtin_LINE(), __builtin_FUNCTION() and
	// Clang's __builtin_COLUMN() (registry order): the presumed file name
	// (a const char* to its characters), line, the name `__func__` holds
	// in the function evaluating it, and the column (none is kept: 0). The
	// position is the call's own, or in a default argument (and the
	// brace-or-equal-initializer of a member a constructor initializes)
	// that of the outermost such use being evaluated: the call that used
	// the default argument, the constructor (GCC's documentation; both host
	// compilers). Decided where evaluated, never by the analysis.
	SourceFile,
	SourceLine,
	SourceFunction,
	SourceColumn,
	// GCC's __atomic builtins on the object the first operand points to
	// (an integral or pointer type of 1, 2, 4 or 8 bytes; the generic forms
	// pass values through pointers); the memory orders are the last
	// operands. `__atomic_<op>_fetch` gives the new value, `__atomic_fetch_
	// <op>` the old; test_and_set and clear act on a byte.
	AtomicLoadN,
	AtomicLoad,
	AtomicStoreN,
	AtomicStore,
	AtomicExchangeN,
	AtomicExchange,
	AtomicCompareExchangeN,
	AtomicCompareExchange,
	AtomicAddFetch,
	AtomicSubFetch,
	AtomicAndFetch,
	AtomicOrFetch,
	AtomicXorFetch,
	AtomicNandFetch,
	AtomicFetchAdd,
	AtomicFetchSub,
	AtomicFetchAnd,
	AtomicFetchOr,
	AtomicFetchXor,
	AtomicFetchNand,
	AtomicTestAndSet,
	AtomicClear,
	AtomicThreadFence,
	AtomicSignalFence,
	// __sync_bool_compare_and_swap and __sync_val_compare_and_swap (p,
	// old, new): whether *p held old (and now holds new), or what it held.
	SyncBoolCompareSwap,
	SyncValCompareSwap
};

// One base or non-static data member initialization of a constructor, in
// initialization order (12.6.2p10): by its mem-initializer, else by the
// member's brace-or-equal-initializer, else default-initialization
// (nothing for a non-class member). A class subobject runs `constructor`
// (null when an aggregate initialization or a class prvalue initializes
// it, or its default constructor is trivial).
struct SubobjectInit
{
	// The direct base, or null for the member.
	const BaseClass* base;
	const DataMember* member;
	const syntax::MemInitializer* mem_initializer;
	const syntax::Initializer* default_initializer;
	FunctionEntity* constructor;
	// An inheriting constructor's base (12.9p8): its parameters are the
	// base constructor's arguments.
	bool forwards;
	// A mem-initializer names the member, or a member of the anonymous
	// member object within it (see Analyzer::plan_designated).
	bool designated;
	// An anonymous union's or struct's member object: the plan of its own
	// members, which are the class's members for initialization (9.5p5,
	// 12.6.2p2), one per member in declaration order.
	uint32_t inner_count;
	SubobjectInit* inner;
	// A defaulted assignment operator's subobject whose selected operator=
	// takes its class by value: the constructor that copy-initializes that
	// parameter from the source's subobject (12.8p28, 8.5p15, 5.2.2p4).
	FunctionEntity* parameter_copy;
};

struct FunctionEntity : Entity
{
	// The printed name (a constructor's class name, `~C`, `operator+`).
	const char* label;
	bool defined;
	// A function template: distinct from functions of the same signature
	// (14.5.6.1); PA6 does not merge template redeclarations.
	bool templated;
	bool member;
	bool is_virtual;
	bool constructor;
	bool destructor;
	// Defined as deleted (8.4.3): any reference but a declaration is
	// ill-formed.
	bool deleted;
	// A special member function that is trivial (12.1p5, 12.4p5): it runs
	// no code of its own and is not emitted.
	bool trivial;
	// The canonical function type with a void return type (for a conversion
	// function, whose name includes it, with its conversion type; for a
	// function template, whose signature includes it, 1.3.18, with its
	// return type), as declarations compare types (see
	// Analyzer::declared_type): functions of one name in one scope with
	// equal signatures are one function (13.1), and function templates with
	// equal signatures and equivalent template heads one template
	// (14.5.6.1p4), so redeclaration matching is a lookup by this identity.
	const Type* signature;
	// The function scope of the definition.
	Scope* scope;
	// A static member function (no implicit object parameter, 9.4.1).
	bool is_static;
	bool is_constexpr;
	// Implicitly declared (a special member function, 12p1).
	bool implicit;
	// Defined in its class's member-specification, or constexpr: a
	// definition a unit emits only when odr-used.
	bool inline_member;
	// The function set lookup last united it into (sema/lookup.cpp).
	uint32_t lookup_mark;
	// Semantics mode: the definition's body, the default arguments of the
	// parameters (null entries for none; the array is null when no
	// declaration gave one), and the functions the definition odr-uses.
	const syntax::FunctionBody* body;
	const syntax::Expr** default_arguments;
	DemandList* demands;
	DemandList* last_demand;
	// The demand list that last recorded this function (by epoch), and its
	// node there; the overload resolution that last took it as a candidate.
	uint32_t demand_mark;
	uint32_t candidate_mark;
	DemandList* demand_node;
	// Semantics mode: the parameter clause of the latest declaration (its
	// parameter names); a function template's parameter scope; a
	// specialization's template.
	const syntax::ParameterClause* parameters;
	Scope* template_parameters;
	FunctionEntity* primary;
	// The classes whose friend declarations name it: through them an
	// otherwise invisible friend is found by argument-dependent lookup
	// (3.4.2p4).
	ClassList* befriending;
	// Linkage facts of the first declaration (3.5p3-p4, 7.5); declared
	// inline (7.1.2), constexpr (7.1.5p2) or defined in its class (9.3p2);
	// a non-throwing exception specification (15.4p12: `throw()`,
	// `noexcept`, `noexcept(true)`).
	bool internal_linkage;
	bool c_linkage;
	bool is_inline;
	bool nothrow;
	BuiltinFunction builtin;
	// The registry entry a builtin was declared from (lex/gnu_builtins.h).
	GnuBuiltin gnu_builtin;
	uint8_t builtin_variant;
	// An operator function (13.5): its operator and array form. A literal
	// operator (13.5.8) is no operator function: its operator_token is
	// KW_OPERATOR (see IsLiteralOperator).
	bool is_operator;
	TokenKind operator_token;
	bool operator_array;
	// PA29's rare facts, a bit each: GNU always_inline, whose calls in this
	// unit run this unit's body (see ProgramLowering::inline_copy); a hosted
	// GNU inline wrapper of a reserved builtin this compiler lacks
	// (FunctionBody::reserved_call), whose statements holding such a call
	// are only name-checked and whose emission is an error (see
	// Analyzer::define_function); Clang's
	// exclude_from_explicit_instantiation: explicit instantiations of its
	// class do not cover it, each unit using it instantiates and emits it
	// (see Analyzer::InstantiationSuppressed); GNU gnu_inline, whose
	// definition is never emitted on its own (see
	// ProgramLowering::demand_entry).
	bool always_inline : 1;
	bool unanalyzed_body : 1;
	bool excluded_instantiation : 1;
	bool gnu_inline : 1;
	// GNU used: an object-emission root.
	bool used : 1;
	// [[noreturn]] or GNU noreturn on any declaration: its calls do not
	// return (LowIR's return=noreturn).
	bool noreturn : 1;
	// A C-linkage function whose first declaration, non-throwing, is in a
	// system header (see Analyzer::check_redeclared_exceptions).
	bool system_nothrow : 1;
	// GNU cppgm_stable_prefix: a query whose normally returning
	// call at an index keeps the results at lower indexes for the same
	// earlier arguments.
	bool stable_prefix;
	// A noexcept-specification's constant expression waits for its first
	// need (Analyzer::function_nothrow, which holds it; `nothrow` is false
	// until then).
	bool noexcept_pending;
	// Odr-used (3.2p2): its definition is instantiated by the unit's end
	// whether or not a unit emits it (14.7.1p3, 14.6.4.1p8; queued once,
	// see Analyzer::odr_use).
	bool definition_demanded;
	// It overrides a function of a base outside the primary chain (whose
	// table it would share).
	bool overrides_other_base;
	// A block-scope declaration: the function of the enclosing namespace
	// an earlier declaration there declared, which it denotes (3.5p6).
	FunctionEntity* denotes;
	// Semantics mode, a constructor: the number of its subobjects'
	// initializations (`inits`).
	uint32_t init_count;
	// A constructor declared explicit (12.3.1), and a special member
	// function defaulted on its first declaration (8.4.2p4: not
	// user-provided).
	bool is_explicit;
	bool defaulted;
	// GNU noinline: calls are never expanded inline.
	bool no_inline;
	// Declared with an exception-specification (a destructor without one
	// is as non-throwing as its subobjects' destructors, 12.4p3).
	bool exception_spec;
	// An inheriting constructor (12.9): the base constructor it inherits.
	FunctionEntity* inherited;
	// Semantics mode, a constructor: its subobjects' initializations in
	// initialization order, once its definition (or the implicit one's
	// first odr-use) is analyzed. A defaulted copy or move constructor or
	// assignment operator: each base and member in declaration order, its
	// `constructor` the function selected to copy, move or assign it
	// (null for a scalar or reference, copied as its value; an anonymous
	// union's object's is trivial, its storage copied).
	SubobjectInit* inits;
	// Which copy or move special member it is, once its class completes.
	CopyKind copy_kind;
	// Explicitly defaulted (8.4.2): its definition is the implicit one's,
	// synthesized where it is defined (out of its class: not inline).
	bool synthesized;
	// A conversion function (12.3.2): its result type is its conversion
	// type.
	bool conversion;
	AllocationRole allocation;
	// Declared with a placeholder return type (the course's extension,
	// C++14's 7.1.6.4): every declaration uses it (p13), even once its
	// definition's return statements deduced the type.
	bool placeholder_result;
	// Its body (a lambda's or a local class member's) names a block-scope
	// static of an enclosing function, or holds one that does: that
	// function's lowering makes the static's global (see
	// ProgramLowering::demand_entry).
	bool names_enclosing_statics;
	// GNU pure and const (pa28): the strongest call effect its declarations
	// state (a function template's specializations take the template's).
	FunctionEffect effects;
	// An inheriting constructor that is the full form of the constructor
	// it inherits through its bases' inheriting constructors (no trailing
	// default argument omitted: the one the hosts declare, P0136, which
	// Itanium 5.1.4.3 names CI1/CI2 of that constructor's class; see
	// SemanticGraph::inherited_root). Set when sema declares it.
	bool inherits_full;
	// The declaration ordinal where its definition is (a template's
	// definition context, 14.6.4.2; see Analyzer::definition_point).
	uint32_t definition_ordinal;
	// 1 + where its definition's declaration starts (a unit location; an
	// instantiation's is its definition's; 0 for none, an implicit
	// member's): its debug location (lowir.md "Debug Locations").
	uint32_t definition_location;
	// A virtual function's slot in its class's virtual table once the class
	// completes (a destructor's complete entry; its deleting entry is the
	// next slot).
	uint32_t vtable_slot;
	// A delegating constructor (12.6.2p6): its mem-initializer and the
	// target constructor it selects (the plan then has no subobjects).
	const syntax::MemInitializer* delegation;
	FunctionEntity* delegate;
	// Templates: an instantiated function (see `templ`), named by an
	// explicit instantiation definition (14.7.2p8-p9), and the state of its
	// definition's instantiation.
	bool instantiated;
	bool explicit_instantiation;
	// Named by an explicit instantiation declaration (14.7.2p10): its
	// implicit instantiation is suppressed unless it is inline.
	bool extern_instantiation;
	// Declared by `template<>` (14.7.3): a function template's
	// specialization or an instance's member whose definition is its own.
	bool explicit_specialization;
	FactState instantiation;
	// Semantics mode, a virtual function (10.3): declared pure (10.4p2),
	// with the virt-specifiers override and final (9.2p9, 10.3p4-p5) (its
	// slot is `vtable_slot`).
	bool pure;
	bool is_override;
	bool is_final;
	// The function it overrides in its class's primary base chain, whose
	// slot it takes (10.3p2; null when it introduces its own).
	const FunctionEntity* overridden;
	// Templates: an instantiated function (a function template
	// specialization, `primary` its template, or a member of a class
	// specialization) is emitted where odr-used from its template's
	// definition, whose body is copied and analyzed once on demand
	// (`instantiation`); its template facts (null for none).
	FunctionTemplateFacts* templ;
};

inline TemplateHead* HeadOf(const FunctionEntity* function)
{
	return function->templ != nullptr ? function->templ->head : nullptr;
}

inline const Type* const* ArgumentsOf(const FunctionEntity* function)
{
	return function->templ != nullptr ? function->templ->arguments : nullptr;
}

inline FunctionEntity* PatternOf(const FunctionEntity* function)
{
	return function->templ != nullptr ? function->templ->pattern : nullptr;
}

// A closure type's capture (5.1.2p14-p15): the variable or parameter
// captured, and the closure's unnamed member: a reference bound to it (the
// course's form of the unspecified members of a capture by reference), or
// an object of its type direct-initialized from it (by copy; a class
// object's by `constructor`, 5.1.2p21).
struct Capture
{
	const Entity* entity;
	const VariableEntity* member;
	const FunctionEntity* constructor;
};

// A closure type's facts (5.1.2p3), kept beside the class: its function
// call operator; a captureless lambda's conversion function to pointer to
// function and the function whose address it yields, which has the call
// operator's effect (5.1.2p6: its body, without an object); the pointer
// member holding a captured `this` (5.1.2p10, the first member), and the
// captures in member order. A closure of a class's member-specification (a
// default member initializer's, a member function's default argument's) is
// numbered among its class's (`member_context`): the same type in every
// unit defining the class.
struct ClosureFacts
{
	FunctionEntity* call;
	const FunctionEntity* conversion;
	const FunctionEntity* invoker;
	const VariableEntity* this_member;
	const Capture* captures;
	uint32_t capture_count;
	bool member_context;
	// A captureless lambda's (5.1.2p6): its conversion function and the
	// function it yields are declared where first needed (a conversion
	// considered, the function named, a direct call; see
	// Analyzer::closure_conversion).
	bool convertible;
	// A member_context closure's Itanium 5.1.8 context: the data member
	// whose default member initializer holds the lambda, or the member
	// function whose default argument for parameter `default_argument`
	// does; its number among that context's closures of its lambda-sig
	// (null context: the class's member-specification at large).
	const Entity* context;
	uint32_t default_argument;
	uint16_t context_number;
	// Its number among its scope's closures of its lambda-sig as Itanium
	// 5.1.8 counts them, an ellipsis (z) being one of the lambda-sig's
	// types; the class's local_discriminator counts the course's sequence,
	// where `(...)` and `()` share one (see Analyzer::number_closure).
	uint16_t closure_number;
	// A templated lambda (C++20's, hosted compilations): its expression,
	// whose call operator `call` is a member function template.
	const syntax::LambdaExpr* lambda;
	// The function whose body holds the lambda (null outside one) and the
	// expression's token anchors (LambdaExpr::clause_token, body_token).
	const FunctionEntity* enclosing;
	uint32_t clause_token;
	uint32_t body_token;
};

// A literal operator's name is `operator"" ` and its ud-suffix (the lookup
// key semantic analysis interns for it).
const char kLiteralOperatorPrefix[] = "operator\"\" ";

inline bool IsLiteralOperator(const FunctionEntity* function)
{
	return !function->is_operator && function->operator_token == TokenKind::KW_OPERATOR;
}

const char* LiteralOperatorSuffix(const FunctionEntity* function);

// A defaulted move constructor or move assignment operator that is defined
// as deleted: overload resolution ignores it (12.8p11, p23 as amended by
// CWG 1402), unlike one declared deleted.
inline bool IgnoredByOverloadResolution(const FunctionEntity* function)
{
	return function->deleted && function->defaulted &&
		(function->copy_kind == CopyKind::MoveConstructor || function->copy_kind == CopyKind::MoveAssignment);
}

} // namespace sema
