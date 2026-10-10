#pragma once

#include <cstdint>

// Presentation names of a function body's locals: the course's spellings
// (its lowering's block labels and generated slots), so that a student
// comparing output with a reference sees familiar names. Names are free
// (lowir.md: the comparison renames them), so a producer records only what
// made a block or slot, as a one-byte kind (and, for a few kinds, a source
// name it already has); the text writer (lowir/local_names.h) spells them
// only when it writes, and a program that is never printed pays nothing.
//
// - A block of a generated kind prints as `<stem>_<id>`, its stem naming
//   the construct (`^if_then_3`, `^lor_rhs_4`). Ids are dense in creation
//   order, so a function's labels number its blocks as they were made.
// - A slot of a generated kind prints as `<stem>__<n>` (`$cond__1`), n
//   counting the function's generated slots in order; a shadowing
//   declaration's slot as `<name>__shadow<n>`, the name's n-th slot of the
//   function; an unnamed parameter's as the parameter (`$__param1`); a
//   range-for statement's implicit variables (6.5.4) as `__range<n>`,
//   `__begin<n>`, `__end<n>` and `__idx<n>`, n counting them in order.
// - The optimizer's blocks and slots have kinds of their own; an inlined
//   copy of a named block, slot or temporary prints as `<name>_<id>`,
//   `<name>__<n>` or `<name>_<n>`.
// - An unnamed temporary prints as `t<n>`, n counting the body's
//   temporaries after its parameters from 1; an unnamed parameter as
//   `__param<i>` in a definition and `arg<i>` elsewhere, i its position.
// - Anything else prints its name, or its id when it has none.
//
// A generated spelling ends in a number that the entity's id or ordinal
// makes unique among generated ones. A slot's or temporary's ordinal skips
// a number whose spelling an explicit name has (a source variable
// `cond__1` or `t3`, or lowiropt input naming `%t3`), as the course's do;
// any other spelling equal to an explicit name takes trailing underscores
// until it is free, which no generated spelling has.
namespace lowir {

enum class BlockKind : uint8_t
{
	Plain,
	// Statements (6.4-6.6).
	IfThen,
	IfElse,
	IfEnd,
	WhileCond,
	WhileBody,
	WhileEnd,
	DoBody,
	DoCond,
	DoEnd,
	ForCond,
	ForBody,
	ForIter,
	ForEnd,
	SwitchDispatch,
	SwitchCase,
	SwitchDefault,
	SwitchEnd,
	Goto,
	Block,
	Unreachable,
	CondTrueCleanup,
	CondFalseCleanup,
	// Expressions.
	LandRhs,
	LandShort,
	LandEnd,
	LorRhs,
	LorShort,
	LorEnd,
	CondThen,
	CondElse,
	CondEnd,
	CondaddrThen,
	CondaddrElse,
	CondaddrEnd,
	CondobjThen,
	CondobjElse,
	CondobjEnd,
	DiscardCondThen,
	DiscardCondElse,
	DiscardCondEnd,
	BasecastNull,
	BasecastAdjust,
	BasecastEnd,
	MemberPointerVirtual,
	MemberPointerDirect,
	MemberPointerEnd,
	NullMemberCond,
	NullMemberBody,
	NullMemberEnd,
	DynCastScan,
	DynCastFound,
	DynCastFail,
	DynCastEnd,
	TypeidScan,
	TypeidFail,
	AtomicRetry,
	AtomicDone,
	// Dynamic allocation (5.3.4, 5.3.5).
	NewInit,
	NewEnd,
	ArrayNewLengthValid,
	ArrayNewLengthInvalid,
	ArrayNewCtorCond,
	ArrayNewCtorBody,
	ArrayNewCtorEnd,
	ArrayNewCtorCleanup,
	ArrayNewCtorCont,
	ArrayNewCtorResume,
	ArrayNewCtorCleanupCond,
	ArrayNewCtorCleanupBody,
	ZeroinitCond,
	ZeroinitBody,
	ZeroinitEnd,
	ArrayDtorCond,
	ArrayDtorBody,
	ArrayDtorEnd,
	DeleteNonnull,
	DeleteEnd,
	ArrayDeleteNonnull,
	ArrayDeleteEnd,
	ArrayDeleteDtorCond,
	ArrayDeleteDtorBody,
	ArrayDeleteDtorEnd,
	// Objects and their lifetimes.
	ConditionalTemporaryDestroy,
	ConditionalTemporaryDone,
	RefDestroy,
	RefDestroyDone,
	ConstructorArrayCond,
	ConstructorArrayBody,
	ConstructorArrayEnd,
	ConstructorArrayCleanup,
	ConstructorArrayCleanupBody,
	ConstructorArrayResume,
	DestructorArrayCond,
	DestructorArrayBody,
	DestructorArrayEnd,
	DestructorArrayCleanup,
	DestructorArrayCleanupBody,
	DestructorArrayResume,
	CopyArrayCond,
	CopyArrayBody,
	CopyArrayEnd,
	CopyArrayCleanup,
	AssignArrayCond,
	AssignArrayBody,
	AssignArrayEnd,
	ConstructionArrayCleanup,
	ConstructionArrayCleanupBody,
	ConstructionArrayCleanupEnd,
	InitlistConstructorCleanup,
	InitlistConstructorEnd,
	InitlistConstructorCleanupCondition,
	InitlistConstructorCleanupBody,
	InitlistConstructorResume,
	LocalStaticReady,
	LocalStaticInit,
	LocalStaticCtorRun,
	LocalStaticCtorDone,
	LocalStaticDestroy,
	LocalStaticFiniNext,
	// Cleanups and exception handling (15).
	CallUnwindDispatch,
	CallUnwindEnd,
	ThrowAllocUnwindEnd,
	CleanupAction,
	CleanupResume,
	ConstructionCleanup,
	ReturnCleanupAction,
	ReturnCleanupTerminal,
	ReturnCleanupExit,
	ConditionalCleanupDispatch,
	ConditionalCleanupRoute,
	ConditionalCleanupResume,
	ConditionalCleanupLanding,
	LexicalCleanupDispatch,
	LexicalCleanupTerminate,
	LexicalUnwind,
	TryEnd,
	CatchDispatch,
	CatchEntry,
	CatchBody,
	CatchNext,
	CatchCleanup,
	FunctionExceptionLanding,
	FunctionExceptionAction,
	FunctionExceptionResume,
	ConstructorCleanup,
	ConstructorCleanupEntry,
	DestructorCleanup,
	DestructorCleanupEntry,
	DestructorEnd,
	DestructorSuffixCleanup,
	DestructorSuffixNext,
	DestructorReturnEpilogue,
	ReturnNull,
	AdjustReturn,
	// Constructs of this implementation without a course spelling.
	ComplexNan,
	ComplexRecompute,
	ComplexByD,
	ComplexByC,
	ComplexEnd,
	VectorTake,
	VectorJoin,
	VectorUnordered,
	// The optimizer's blocks (lowiropt, cppgm++ -O1 and above).
	InlineEntry,
	InlineCont,
	InlineCopy,
	Split,
	Thread,
	LoopPreheader,
	LoopCopy,
	LoopExit,
	FillFast,
	FillSized,
	FillSame,
	WideLoop,
	WideTail,
	Unrolled,
	Dead,
	Count
};

enum class SlotKind : uint8_t
{
	Plain,
	// `<stem>__<n>`: the function's generated slots.
	Cond,
	Land,
	Lor,
	Condaddr,
	Argobj,
	Retobj,
	Retref,
	Arg,
	Argarr,
	Call,
	Callobj,
	Refcall,
	Lifetime,
	Refarg,
	Tmpobj,
	Tmpref,
	Arraytmp,
	Baseobj,
	Classvalue,
	Stmtobj,
	Catch,
	FunctionException,
	ThrowAlloc,
	Initlist,
	Discard,
	Discardarr,
	CleanupReturn,
	Basecast,
	MemberPointerCallee,
	DynCast,
	ArrayNewIndex,
	ArrayNewSize,
	ZeroinitOffset,
	ArrayDtorIndex,
	ArrayDeleteIndex,
	ConstructorArrayIndex,
	DestructorArrayIndex,
	DestructorProgress,
	CopyArrayIndex,
	AssignArrayIndex,
	InitlistConstructorIndex,
	AtomicArg,
	AtomicExpected,
	AtomicResult,
	CatchPointer,
	CatchValue,
	FloatingSignbit,
	NullMemberElement,
	NullMemberIndex,
	AnonymousUnionStorage,
	Complex,
	Vector,
	SoftFloat,
	Bits,
	Asm,
	// The optimizer's.
	Retmerge,
	Retmergeobj,
	Aggregate,
	Field,
	Phi,
	Demote,
	Wide,
	Inlined,
	// The last generated kind; those after it are spelled otherwise.
	LastGenerated = Inlined,
	// `<name>__shadow<n>`: a declaration shadowing a name of the function.
	Shadow,
	// `__param<i>`: an unnamed parameter's slot (Slot::index).
	Parameter,
	// `<name><n>`: a range-for statement's implicit variable.
	RangeFor,
	Count
};

// A temporary's: its own name, or an inlined copy's (`<name>_<n>`).
enum class TempKind : uint8_t
{
	Plain,
	Inlined
};

const char* block_kind_stem(BlockKind kind);
const char* slot_kind_stem(SlotKind kind);

} // namespace lowir
