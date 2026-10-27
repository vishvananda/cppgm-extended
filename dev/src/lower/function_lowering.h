#pragma once

#include <algorithm>
#include <cstdint>
#include <vector>

#include "lower/program_lowering.h"
#include "sema/integral_arithmetic.h"

// One function body lowered from its statements and expressions' semantic
// facts (lower/statement_lowering.cpp, lower/expression_lowering.cpp,
// lower/condition_lowering.cpp). Values carry their LowIR operand and type;
// storage is a slot or global used directly as a load/store operand, or a
// pointer temporary. Generated temporaries, slots and blocks are unnamed
// (printed by ordinal), so no source name can redirect them.
namespace sema {
struct AsmInstruction;
}

namespace lower {

// A prvalue: its operand and the LowIR type it has. A truth value from
// `cmp` is i64 whatever its C++ type's storage (bool is u8 in memory).
struct RValue
{
	lowir::Value value;
	lowir::Type type;
};

// A glvalue's storage. `known` holds the value an assignment or prefix
// increment just stored, which a following read of the same non-volatile
// glvalue uses instead of loading it again.
struct Place
{
	lowir::Value storage;
	bool is_volatile;
	bool has_known;
	RValue known;
	// An atomic object's (sema::kAtomic): read and written by atomic loads
	// and stores (sequentially consistent, C11 7.17.7).
	bool is_atomic;
	// Storage not accessed through an object of the access's type (a
	// bit-field's storage unit, a slot whose bytes are read as another
	// type): its accesses are not `typed` (lowir.md "Instruction Facts").
	// Addresses reached through a union member are tracked by temporary
	// instead (FunctionLowering::untyped_addresses_).
	bool untyped = false;
};

// Where a conversion's operand stands: the course conventions fold a
// literal's widening in some contexts only (see convert_integer).
enum class ConversionContext : uint8_t
{
	// Multiplicative, bitwise and shift operands, conditional arms.
	Operand,
	// Additive operands, relational and equality operands, unary operands.
	Additive,
	Comparison,
	Unary,
	Initialization,
	Argument,
	Return,
	Assignment,
	CompoundAssignment,
	Cast,
	// A class aggregate's member (a literal narrowed to another signedness
	// keeps its conversion) and a bit-field's assigned value (a narrowed
	// literal always does).
	MemberInitialization,
	BitFieldAssignment,
	// An array element initialized member by member (a narrowed literal
	// always keeps its conversion).
	ArrayElement,
	// An additive operand beside sizeof or alignof (a literal's widening
	// always folds).
	BesideSize
};

class FunctionLowering
{
public:
	// A definition's body, or (with `function` null) the initialization
	// function that runs dynamic initializers.
	FunctionLowering(ProgramLowering& program, lowir::SymbolId symbol, const sema::FunctionEntity* function,
		EntryVariant variant = EntryVariant::Complete);

	void lower_definition();
	// Dynamic initialization of a namespace-scope variable (3.6.2), and of
	// a reference's temporary.
	void lower_dynamic_initializer(lowir::SymbolId global, const sema::VariableEntity* variable,
		const syntax::Initializer* initializer, uint32_t location);
	void lower_temporary_initializer(
		lowir::SymbolId global, const sema::Type* type, const syntax::Expr& expr, uint32_t location);
	// A class object of static storage duration, constructed by the
	// initialization function; one destroyed by the finalization function.
	void construct_global(lowir::SymbolId global, const sema::VariableEntity* variable,
		const syntax::Initializer* initializer, uint32_t location);
	void begin_static_initializer();
	void finalize_global(const Finalizer& finalizer);
	// The destruction of an object of static storage duration constructed
	// now, registered with atexit (lower/local_static_lowering.cpp); of a
	// thread-local one (`thread`), with __cxa_thread_atexit.
	void register_destruction(lowir::SymbolId object, const sema::Type* type, uint32_t location, bool thread = false);
	// A class aggregate's initialization helper (`this` and a value per
	// named member, each member stored from it).
	void lower_aggregate_helper(const sema::ClassEntity* entity, uint32_t count);
	// A thread-local object's guarded per-thread initialization, and its
	// wrapper returning the thread's object after running it.
	void lower_thread_local_initializer(lowir::SymbolId global, lowir::SymbolId guard,
		const sema::VariableEntity* variable, const syntax::Initializer* initializer, uint32_t location, bool dynamic);
	void lower_thread_local_wrapper(lowir::SymbolId init, lowir::SymbolId global);
	// Object mode: the unit's thread-local initialization function's body
	// (see ProgramLowering::unit_thread_local_init).
	void lower_thread_local_inits(lowir::SymbolId guard, const std::vector<lowir::SymbolId>& inits);
	// A class template specialization's static data member's guarded
	// initialization (unordered, 3.6.2p2; none for a static one, not
	// `dynamic`), setting `constructed` if not kNoId and registering its
	// destruction if `registered`.
	void lower_unordered_initializer(lowir::SymbolId global, lowir::SymbolId guard, lowir::SymbolId constructed,
		const sema::VariableEntity* variable, const syntax::Initializer* initializer, uint32_t location, bool dynamic,
		bool registered);
	void lower_terminate_helper(lowir::SymbolId begin_catch, lowir::SymbolId terminate);
	// A body passing its parameters to `target` and returning its result.
	void lower_forwarder(lowir::SymbolId self, lowir::SymbolId target);
	// A this-adjusting thunk's body: `target` called with the parameters,
	// the one at `this_index` moved by `adjustment` bytes.
	// `result`: a covariant thunk's adjustment of the pointer or reference
	// returned.
	void lower_thunk(lowir::SymbolId self, lowir::SymbolId target, uint32_t this_index, int64_t adjustment,
		int64_t vcall, const ProgramLowering::ResultAdjustment* result);
	// The initialization function outlives units: entity maps restart.
	void begin_unit();
	// The class of an object of `type` or of an array of it; whether its
	// destructor is not trivial (12.4p5).
	static const sema::ClassEntity* object_class(const sema::Type* type);
	// Whether computing a glvalue's address again (`read`: its value)
	// repeats no side effect.
	static bool side_effect_free(const syntax::Expr* expr, bool read, uint32_t depth);
	static bool needs_destruction(const sema::Type* type);
	// Ends the initialization function.
	void finish_initializer();

private:
	// The debug location of the construct being lowered (a statement, an
	// expression): the instructions it emits carry it (ProgramLowering::
	// debug_location), the enclosing construct's again when it ends.
	class DebugScope
	{
	public:
		DebugScope(FunctionLowering& lowering, uint32_t location) : lowering_(lowering), active_(lowering.debug_)
		{
			if (!active_)
				return;
			saved_ = lowering.builder_.current_location();
			const lowir::DebugLocation here = lowering.lowering_.debug_location(location);
			if (here.present())
				lowering.builder_.set_current_location(here);
		}
		~DebugScope()
		{
			if (active_)
				lowering_.builder_.set_current_location(saved_);
		}
		DebugScope(const DebugScope&) = delete;
		DebugScope& operator=(const DebugScope&) = delete;

	private:
		FunctionLowering& lowering_;
		bool active_;
		lowir::DebugLocation saved_ = lowir::DebugLocation{ 0, 0, 0 };
	};
	friend class DebugScope;

	struct Local
	{
		// Its slot; a lambda's capture's (`captured`): the offset of the
		// closure's member, the captured entity's copy or (`reference`) the
		// reference member holding its address.
		lowir::SlotId slot;
		// The slot holds the address the reference is bound to.
		bool reference;
		// A lambda's capture (in its function call operator's body).
		bool captured = false;
		// A class parameter passed by address, or a local constructed in
		// the result's destination: the object's address (else none).
		lowir::Value address;
		// The alive objects once its declaration completed (its own, a
		// reference's extended temporaries, last); 0 before.
		size_t depth;
	};

	struct JumpTargets
	{
		lowir::BlockId break_target;
		// kNoId inside a switch statement.
		lowir::BlockId continue_target;
		// The automatic objects alive where the statement began, which a
		// jump to its targets leaves alive, and the exception-handling
		// contexts it began in (see EhContext).
		size_t cleanup_depth;
		size_t context_depth;
	};

	// An automatic class object (or array of them) whose destructor runs
	// when its scope ends (6.7p2).
	struct Cleanup
	{
		// The object's storage: its slot (addressed again where it is
		// destroyed), or a class parameter's address.
		lowir::Value storage;
		const sema::Type* type;
		// The declared variable (keys the returns sharing a continuation).
		const sema::Entity* variable;
		// This object's blocks in the unwinding and shared-return chains
		// (kNoId until made): its predecessors are lexically fixed, so one
		// block serves every point where it is the innermost alive.
		lowir::BlockId unwind;
		lowir::BlockId shared_return;
		lowir::BlockId conditional;
		// A temporary of the full-expression being lowered (12.2p3).
		bool temporary;
		// A temporary made on some paths of its full-expression only: its
		// u8 flag, set once constructed, guards its destruction.
		bool guarded;
		lowir::SlotId flag;
		// The local constructed in the result's destination (12.8p31): a
		// return leaves it to the caller.
		bool result;
		// An array temporary whose elements' addresses were kept as they
		// were made (a std::initializer_list's array): their first's index
		// in element_values_ + 1 (0: none), destroyed last to first there.
		uint32_t saved_elements;
		// Its unwinding blocks inside exception-handling contexts, with and
		// without the landing's marker still pushed (0: not made; the entry
		// block is none of them).
		lowir::BlockId eh_marked;
		lowir::BlockId eh_unmarked;
		// A local aggregate's member being initialized (aggregate_object).
		bool member;
		// An array whose elements are being made (aggregate_array_object):
		// `flag` counts those made, destroyed last first.
		bool counted;
		// A new-expression's storage while its object is initialized: freed
		// by this deallocation function (5.3.4p18-p20; a placement one's
		// further arguments from element_values_ at `saved_elements`).
		const sema::FunctionEntity* deallocator;
		// A block-scope static's guard (`storage` its symbol) while a host
		// object's thread-safe initialization runs: an exception leaving it
		// aborts the initialization (see local_static_variable).
		bool aborts_guard;
		// (Members after this point are never listed by an initializer.)
		// The exception-handling context stack eh_marked and eh_unmarked
		// were made under (see EhContext::stack): a landing under another
		// stack makes its own.
		uint32_t eh_stack;
		// Its block in the lexical unwinding chains (see lexical_handler)
		// and the context stack it was made under (0: not made).
		lowir::BlockId lexical;
		uint32_t lexical_stack;
		// A temporary made in a branch its full-expression outlives; a
		// tracked one's address where it was made.
		bool branched;
		lowir::Value made_at;
	};

	// The array aggregate_array_object (or aggregate_elements) is making
	// element by element: its count slot, the elements made so far and
	// their number; whether its rows are counted apart (each pushed when it
	// begins, above the temporaries made before it, as clang orders them),
	// and the slot its rows share.
	struct ElementCount
	{
		lowir::SlotId slot;
		uint64_t made;
		uint64_t total;
		bool rows;
		lowir::SlotId row_slot;
	};

	// A try statement's block or a handler being lowered
	// (lower/exception_lowering.cpp): the objects alive when it began
	// (`floor`), the statement, its dispatch, entry and end blocks and its
	// first selector; the context's unwinding block by whether the landing's
	// marker is still pushed (a try's entry, a handler's exit; 0: not made),
	// and a handler's `eh_cleanup` landing.
	struct EhContext
	{
		bool is_try;
		// A class or unknown exception may destroy a throwing dynamic object
		// at retirement. Scalar catches need no second-throw guard.
		bool catch_unwind_throws;
		size_t floor;
		const syntax::TryStmt* statement;
		lowir::BlockId dispatch;
		lowir::BlockId entry;
		lowir::BlockId end;
		lowir::BlockId chain[2];
		lowir::BlockId landing;
		// The identity of the context stack up to this context (unique per
		// push), and the stack its chain blocks were made under: unwinding
		// blocks are shared only between landings under one complete stack
		// (the README's continuation keys).
		uint32_t stack;
		uint32_t chain_stack;
		// Its block in the lexical unwinding chains (a handler's exit, a
		// try's entry) and the stack it was made under.
		lowir::BlockId lexical;
		uint32_t lexical_stack;
		// A try without a `...` handler inside another: its dispatch also
		// advertises the enclosing statements' clauses (clause_chains_ from
		// `chain_first`, `chain_count` of them, up to one with `...`).
		uint32_t chain_first;
		uint32_t chain_count;
		// Objects or handlers lie between the try and the enclosing one (or
		// the function's start): its dispatch states a cleanup (retired at
		// once) when its handlers may miss.
		bool unwinds;
		// A handler whose caught type's destructor may throw: its catch's
		// end may (see throwing_end_catch).
		bool end_throws;
		// Its exit's block in the shared return chain (0: not made), and
		// its identity among the scan's contexts (the try statement, or the
		// handler).
		lowir::BlockId return_exit;
		const void* key;
	};

	// One try statement's clauses in an enclosing clause chain.
	struct ClauseSet
	{
		const syntax::TryStmt* statement;
	};

	// What a chain block of the exception-handling contexts does.
	enum class ChainAction : uint8_t
	{
		None,
		// Pops `pops` records and jumps to a try's entry (`tail`).
		TryEntry,
		// Leaves a handler: its record and, with 2 `pops`, the marker.
		HandlerExit,
		// A region's landing: the try's clauses and a marker (none without
		// `statement`), then the chain (`tail`).
		Dispatch,
		// The lexical chains (see lexical_handler): a handler's exit (its
		// record, then its catch), a try's entry (its region popped), and a
		// scope-end destruction's landing (the clauses, a cleanup clause
		// retired at once).
		LexicalHandlerExit,
		LexicalTryEntry,
		LexicalDispatch,
		// A detached body's unwinding reaches its subobjects' cleanup entry
		// (`tail`) with `pops` records popped.
		BodyEntry,
		// The shared return chain inside contexts (context_return_entry):
		// the context at index `pops` left, or the object at depth `pops`
		// destroyed (in its lexical landing).
		ReturnExit,
		ReturnObject
	};

	// Where a throwing call's lowering began inside a full-expression's
	// region segment: its region opens there, before its operands.
	struct CallMark
	{
		lowir::BlockId block;
		lowir::InstId after;
		// A declared object's constructor call: a temporary's region does
		// not open where it began (pending_mark_region).
		bool declared;
		// The objects alive where it began: a region opening later opens
		// there only if none was registered since (open_region).
		size_t depth;
		// Its operands branched with its region opened where it began
		// (paused_call_region): the call itself opens none.
		bool regioned;
	};

	// What creates_temporaries looks for in its calls mode: a call's class
	// prvalue materialized as a temporary the full-expression destroys
	// (the root's only when `root`: a discarded one); whether one is of a
	// member class template's specialization, and whether one's destructor
	// may throw.
	struct CallTemporaries
	{
		bool root;
		bool member_template;
		bool throwing_destructor;
	};

	// The declarations whose objects are alive at a label, innermost first:
	// a persistent chain (index into live_nodes_, kNoLive at the bottom).
	struct LiveNode
	{
		const sema::Entity* variable;
		uint32_t parent;
	};
	static const uint32_t kNoLive = 0xffffffffu;

	// An address re-derived for each store: a slot or global addressed
	// again, `this` loaded again, or a fixed pointer; then the steps pushed
	// on path_steps_ from `first_step` on (member offsets, base offsets,
	// array element indexes of the element's LowIR type).
	struct AddressPath
	{
		enum class Root : uint8_t
		{
			Storage,
			This,
			Value
		};
		Root root;
		lowir::Value base;
		size_t first_step;
	};

	struct PathStep
	{
		enum class Kind : uint8_t
		{
			Field,
			Base,
			Element,
			// An array element by its index scaled to a byte offset.
			ScaledElement
		};
		Kind kind;
		uint64_t amount;
		lowir::Type element;
		// A scaled element's size.
		uint64_t size;
		// A field reached through a union (see through_union).
		bool untyped;
	};

	// A bit-field's storage unit (9.6): its byte offset in the class, the
	// field's bit position and width in it, the declared type's LowIR type
	// and the type the unit is read and written in (`unit_unsigned`: read
	// as unsigned, which a 64- or 128-bit LowIR type does not spell); a
	// shared unit (only read-modify-written) and a split one's bytes (read
	// and written piecewise, see sema::DataMember).
	struct BitField
	{
		const sema::VariableEntity* variable;
		uint64_t unit_offset;
		unsigned shift;
		unsigned width;
		bool is_signed;
		bool unit_unsigned;
		bool shared;
		unsigned split;
		lowir::Type declared;
		lowir::Type unit;
	};

	// An update of the bit-field (or, for a conditional's other operand, the
	// object) an lvalue designates: an assignment (5.17p2), a compound
	// assignment (5.17p7) or an increment (5.3.2, 5.2.6). The right operand
	// is evaluated once, when first needed.
	struct BitFieldUpdate
	{
		enum class Kind : uint8_t
		{
			Assign,
			Compound,
			Increment
		};
		Kind kind;
		// Compound: the arithmetic operator; increment: a decrement, and
		// whether the result is the new value (prefix) or the old.
		uint32_t op;
		bool decrement;
		bool prefix;
		const syntax::BinaryExpr* source;
		// The designated field's type, and a compound assignment's
		// operation type.
		const sema::Type* type;
		const sema::Type* operation;
		bool evaluated;
		RValue right;
		uint32_t location;
	};

	// A case or default label of the switch statements being lowered and
	// its block, found by one scan of the switch's body.
	struct SwitchLabels
	{
		const syntax::Stmt* statement;
		lowir::BlockId block;
	};

	// A short-circuit operator of a condition chain: its right operand's
	// block and the targets that operand branches to.
	struct ChainTargets
	{
		lowir::BlockId right;
		lowir::BlockId if_true;
		lowir::BlockId if_false;
	};

	// One conditional of a nested chain being lowered: its result slot, its
	// else and end blocks, and whether the chain continues in its then
	// operand (else in its else operand).
	struct Choice
	{
		const syntax::ConditionalExpr* expr;
		lowir::Type storage;
		Place slot;
		lowir::BlockId else_block;
		lowir::BlockId end;
		bool nested_then;
		// The operands' temporaries end with their branches (the
		// full-expression's root), else they are guarded.
		bool local;
		// The nested level is in an operand never evaluated.
		bool unreachable_nested;
	};

	// A subobject a constructor or destructor lowers the destruction of: a
	// base, a member, or one element (`element` >= 0) of an array member;
	// or a deleting entry's deallocation of the object, its last action.
	struct Subobject
	{
		const sema::BaseClass* base;
		const sema::DataMember* member;
		int64_t element;
		bool deallocation;
	};

	// Where an array's elements are: a local's slot (addressed again for
	// each element) or a member of `*this`.
	struct ArrayBase
	{
		Place place;
		const sema::DataMember* member;
	};

	// A cleanup continuation block made but not yet placed: the terminal
	// of its chain (`resume` or the shared return) or the destruction of
	// one automatic object continuing at `tail`. A shared return's
	// destruction that may throw runs in a region whose handler is the
	// unwinding chain of the objects under it (`below`, kNoId for none).
	struct ChainBlock
	{
		lowir::BlockId block;
		bool terminal;
		bool unwinding;
		bool catch_unwind_throws;
		Cleanup cleanup;
		lowir::BlockId tail;
		lowir::BlockId below;
		ChainAction action;
		uint32_t pops;
		const syntax::TryStmt* statement;
		uint32_t chain_first;
		uint32_t chain_count;
	};

	// A block whose content is emitted after the current block ends: a
	// constructor's cleanup of one subobject, continuing at `tail` (none:
	// the cleanup's end and `resume`).
	struct DeferredCleanup
	{
		lowir::BlockId block;
		Subobject subobject;
		lowir::BlockId tail;
		// A detached body's cleanup entry (see body_target_; kNoId
		// otherwise): the landing pops its record and jumps there.
		lowir::BlockId entry;
		// The exception-handling contexts entered where it was pushed.
		size_t contexts;
	};

	// One scalar of an aggregate's initialization at its byte offset: an
	// expression, a string literal's code unit, or nothing (zero).
	struct InitElement
	{
		const syntax::Expr* expr;
		const sema::Type* type;
		uint64_t offset;
		bool has_unit;
		uint64_t unit;
	};

	// ------------------------------------------------- blocks
	// A block of the construct `kind` names (lowir/local_kinds.h).
	lowir::BlockId new_block(lowir::BlockKind kind);
	// Places `block` next in layout and continues emission there.
	void place(lowir::BlockId block);
	void jump(lowir::BlockId target);
	void branch(const RValue& condition, lowir::BlockId if_true, lowir::BlockId if_false);
	void terminate(const lowir::Instruction& inst);
	lowir::InstId emit(const lowir::Instruction& inst);
	// An index or copy of an address reached through a union member is one
	// too (mark_untyped).
	lowir::Value emit_value(lowir::Instruction inst, lowir::Type type);
	// A source variable's slot: the body's first of its name prints as the
	// name, a later one as its shadow; an unnamed variable's as `__slot`;
	// a range-for statement's implicit variables by their count.
	lowir::SlotId new_slot(lowir::NameId name, lowir::Type type);
	// A slot the lowering makes for `kind`'s construct (lowir/local_kinds.h).
	lowir::SlotId generated_slot(lowir::SlotKind kind, lowir::Type type);
	// An unnamed parameter's slot, printed as the parameter (`index` its
	// position in the signature).
	lowir::SlotId parameter_slot(uint32_t index, lowir::Type type);
	// Addresses reached through a union member (lowir.md "Instruction
	// Facts"): a projection of a member reached through a union
	// (through_union), and every index or copy of one. Loads and stores
	// through them are not `typed`: GCC and clang allow a union to pun its
	// members, not pointers into one (a pointer stored and loaded again is
	// an ordinary one).
	void mark_untyped(lowir::Value address);
	bool untyped_address(lowir::Value address) const
	{
		return address.kind == lowir::ValueKind::Temp && address.index < untyped_addresses_.size() &&
			untyped_addresses_[address.index] != 0;
	}
	// Whether the projection of `member` from an object of `holder` (an
	// aggregate initialization's class) reaches it through a union: a
	// union's member, an anonymous union's included, or a member of an
	// anonymous class within one (a projection folds the anonymous
	// objects' offsets).
	static bool through_union(const sema::VariableEntity* member, const sema::ClassEntity* holder);
	// Whether a member named in its class (by a mem-initializer, or alone)
	// is a union's: the class an anonymous class's members are injected
	// into decides (9.5p5), a namespace-scope anonymous union's members are.
	static bool union_member(const sema::VariableEntity* member);

	// ------------------------------------------ instructions
	RValue integer(lowir::Type type, uint64_t bits);
	RValue integer(lowir::Type type, const lowir::Literal& bits);
	// An integral constant of `type` (IntegralLiteral).
	RValue integer(const sema::Type* type, uint64_t bits);
	RValue floating(lowir::Type type, const lowir::Literal& bits);
	RValue constant(lowir::Type type, uint64_t bits);
	RValue load(lowir::Type type, const Place& place);
	// The place's volatile and atomic qualification by the type of its
	// object.
	static void qualify(Place& place, const sema::Type* type);
	bool atomic_step(const Place& place, const sema::Type* type, lowir::Type storage);
	// An atomic object's read-modify-write with no single LowIR operation
	// (C11 6.5.16.2p3, 6.5.2.4p2): its value read (begin), the new value
	// computed from it, then stored by a compare-exchange retried from the
	// read while another write intervened (end). `bits` is the integer of
	// the object's size (atomic_bits: none for other sizes).
	struct AtomicLoop
	{
		lowir::Type bits;
		lowir::SlotId expected;
		lowir::BlockId retry;
	};
	static bool atomic_bits(lowir::Type storage, lowir::Type& bits);
	RValue begin_atomic_loop(const Place& object, lowir::Type storage, AtomicLoop& loop);
	void end_atomic_loop(const Place& object, lowir::Type storage, const AtomicLoop& loop, const RValue& updated);
	void store(lowir::Type type, const RValue& value, const Place& place);
	// Each emitting helper appends at the insertion point. C++ leaves the
	// order of a call's arguments unspecified (g++ evaluates them right to
	// left, cppgm++ left to right), so two emitting operands of one call are
	// emitted by separate statements, the last first: the output must not
	// depend on the compiler that built this one (PA34's inception).
	RValue binary(lowir::BinaryOp op, lowir::Type type, const RValue& left, const RValue& right, uint8_t flags = 0);
	RValue compare(lowir::ComparePredicate predicate, lowir::Type type, const RValue& left, const RValue& right);
	RValue unary(lowir::UnaryOp op, const RValue& operand);
	RValue convert_op(lowir::ConvertOp op, lowir::Type to, const RValue& value);
	RValue copy(lowir::Type to, const RValue& value);
	// An i128 argument to `...` and __builtin_va_arg's read of one: an
	// object of its bytes (lower/expression_lowering.cpp).
	lowir::Type variadic_wide_type(const sema::Type* type);
	lowir::Value variadic_wide_object(const RValue& value, const sema::Type* type);
	lowir::Value address_of(const Place& place);
	RValue index(lowir::Type element, uint8_t projection, lowir::Value base, const RValue& offset);
	bool is_integer_literal(const RValue& value, uint64_t& bits);

	// ---------------------------------------------- statements
	void statement(const syntax::Stmt& stmt);
	bool reachable_label(const syntax::Stmt& stmt);
	void compound(const syntax::CompoundStmt& stmt);
	void substatement(const syntax::Stmt& stmt);
	// A GNU statement expression: its statements, then its last
	// expression's value (scalar) or the class object it copy-initializes
	// at `object` (lower/statement_lowering.cpp). ExpressionScope keeps the
	// enclosing expression's state meanwhile: state a full-expression,
	// construction or initialization records belongs there too.
	struct ExpressionScope;
	RValue statement_expression(const syntax::StatementExpr& expr, lowir::Value object);
	void enter_label(lowir::BlockId block);
	lowir::BlockId goto_label(const Identifier* label);
	void declaration_statement(const syntax::Decl& decl);
	void local_variable(const syntax::InitDeclarator& init);
	void local_object_variable(const sema::VariableEntity* variable, const syntax::InitDeclarator& init);
	// A function-local static object (lower/local_static_lowering.cpp).
	struct StaticInitialization;
	void complete_static_initialization(const StaticInitialization& initialization, size_t& first);
	void local_static_variable(const sema::VariableEntity* variable, const syntax::InitDeclarator& init);
	bool initializer_temporaries(const sema::VariableEntity* variable, const syntax::Initializer* init);
	// A call of Itanium's one-time construction API with a guard's address.
	RValue guard_call(ProgramLowering::GuardCall call, lowir::SymbolId guard);
	void local_initializer(const sema::Type* type, const Local& local, const Place& place,
		const syntax::Initializer& written, uint32_t location, const sema::Entity* variable);
	void unreachable_declaration(const syntax::Decl& decl);
	void initialize_storage(
		const sema::Type* type, const Place& place, const syntax::Initializer* initializer, uint32_t location);
	void initialize_object(const sema::Type* type, lowir::Value base, const syntax::Expr& init, uint32_t location);
	void flatten_initializer(const sema::Type* type, const syntax::Expr* init, uint64_t offset, uint32_t location,
		std::vector<InitElement>& out);
	void if_statement(const syntax::IfStmt& stmt);
	struct AsmState
	{
		std::vector<lowir::Value> addresses;
		std::vector<lowir::Value> homes;
		std::vector<RValue> values;
		std::vector<lowir::Type> types;
		std::vector<uint32_t> widths;
		std::vector<bool> volatiles;
		uint32_t outputs;
		uint32_t registers;
		RValue zero;
		RValue carry;
	};
	void asm_bind_operands(const syntax::AsmStmt& stmt, AsmState& state);
	Place asm_place(const AsmState& state, uint32_t index);
	void asm_store(const AsmState& state, uint32_t index, RValue value);
	RValue asm_source(const AsmState& state, const sema::AsmInstruction& instruction);
	void asm_instruction(const sema::AsmInstruction& instruction, AsmState& state);
	void asm_bit_instruction(const sema::AsmInstruction& instruction, AsmState& state, const RValue& source);
	void asm_capture_flags(const sema::AsmInstruction& instruction, AsmState& state, const RValue& before,
		const RValue& after, const RValue& source);
	void asm_set_flag(const sema::AsmInstruction& instruction, AsmState& state);
	// A GNU asm statement's lifted operations (sema/asm_statements.h).
	void asm_statement(const syntax::AsmStmt& stmt);
	void condition_branch(
		const syntax::Expr& expr, lowir::BlockId if_true, lowir::BlockId if_false, bool fold_literals);
	void switch_statement(const syntax::ConditionalLoopStmt& stmt);
	void scan_switch_labels(const syntax::Stmt& stmt, std::vector<SwitchLabels>& out);
	lowir::BlockId switch_label(const syntax::Stmt& stmt);
	void while_statement(const syntax::ConditionalLoopStmt& stmt);
	void do_statement(const syntax::DoStmt& stmt);
	void for_statement(const syntax::ForStmt& stmt);
	void range_for_statement(const syntax::RangeForStmt& stmt);
	void range_variable(const syntax::InitDeclarator& range);
	RValue condition_variable(const syntax::ConditionDeclaration& declaration, bool to_bool, bool& region);
	void return_statement(const syntax::ValueStmt& stmt);
	void returned_value(const syntax::ValueStmt& stmt);
	void class_return(const syntax::ValueStmt& stmt, const sema::Type* result);
	void initialize_result(const syntax::Expr& expr, const sema::Conversion* conversion,
		const sema::ClassEntity* target, lowir::Value object, uint32_t location);
	void returned_local(const syntax::Stmt& stmt, const sema::Entity*& local, bool& single);
	void return_from_body(const lowir::Instruction& inst);
	// A destructor's body in the region that destroys its subobjects, then
	// their destruction.
	void destructor_body(uint32_t location, bool calls_complete);
	// A synthesized copy or move constructor's construction cleanup (the
	// course's form, copy_lowering.cpp): the subobjects (or array elements)
	// copied so far that a later copy's exception destroys, at their saved
	// addresses; each one's handler block destroys it, then continues with
	// the one below's (the first's with resume), made bottom-up when a
	// region first lands there and placed after that region. A base with
	// virtual bases (`base`) is destroyed by its base-object entry.
	struct ConstructionAction
	{
		lowir::Value address;
		const sema::Type* type;
		lowir::BlockId handler;
		const sema::BaseClass* base;
	};
	bool construction_chain_ = false;
	std::vector<ConstructionAction> construction_actions_;
	lowir::BlockId construction_resume_ = lowir::kNoId;
	std::vector<size_t> construction_pending_;
	lowir::Value copied_address_;
	void construction_made(const sema::Type* type, lowir::Value address, const sema::BaseClass* base = nullptr);
	lowir::BlockId construction_handler();
	void place_construction_handlers();
	void copy_elements_loop(const sema::SubobjectInit& init, const sema::Type* element, uint64_t count,
		lowir::Value destination, lowir::Value source, bool guarded, uint32_t location);
	// The body's end, then the cleanup terminate landings still waiting.
	void finish_body();
	void finish_body_end();
	bool block_targeted(lowir::BlockId block);
	// The targets of branches folded into jumps (see block_targeted).
	std::vector<lowir::BlockId> folded_targets_;

	// --------------------------- class objects (lower/object_lowering.cpp)
	void call_special(const sema::FunctionEntity* function, EntryVariant variant, lowir::Value object,
		const syntax::Slice<syntax::Expr*>& written, uint32_t location, bool elision = false);
	bool copies_storage(const sema::FunctionEntity* function);
	bool copy_called(const sema::FunctionEntity* function);
	void call_constructor(const sema::FunctionEntity* constructor, lowir::Value object, const syntax::Expr& argument,
		const sema::Conversion* conversion, uint32_t location, bool elision = false);
	void construct_from(
		const sema::FunctionEntity* constructor, lowir::Value object, lowir::Value source, uint32_t location);
	// A copy or move of a conditional's class prvalue (its private
	// temporary) into a copy-initialized object: [elision=copy].
	bool elidable_source(const syntax::Expr& expr, const sema::FunctionEntity* constructor);
	void initialize_from(
		const syntax::Expr& expr, const sema::Conversion* conversion, lowir::Value object, uint32_t location);
	void construct(const sema::Type* type, lowir::Value object, const syntax::Initializer* init,
		const sema::FunctionEntity* constructor, EntryVariant variant, uint32_t location);
	void value_initialize(const sema::Type* type, lowir::Value object, const sema::FunctionEntity* constructor,
		EntryVariant variant, uint32_t location);
	void zero_bytes(lowir::Value object, uint64_t size, uint64_t alignment);
	void zero_object(const sema::Type* type, lowir::Value object, uint32_t location);
	void null_data_member_pointers(const sema::Type* type, lowir::Value object, uint64_t offset, bool complete);
	void materialize(const syntax::Expr& expr, lowir::Value object, uint32_t location);
	void materialize_extended(
		const syntax::Expr& expr, lowir::Value object, const sema::Entity* owner, bool persistent, uint32_t location);
	void materialize_prvalue(const syntax::Expr& expr, lowir::Value object, uint32_t location);
	lowir::Value element_address(lowir::Value array, uint64_t element, uint64_t size);
	lowir::Value slot_address_again(lowir::Value address);
	void destroy(const sema::Type* type, lowir::Value object, uint32_t location, bool row = false);
	// `guarded`: on a normal path, an array element's destructor that may
	// throw leaves the elements before it destroyed (15.2p2).
	void destroy_at(const sema::Type* type, const Place& place, uint32_t location, bool guarded);
	void destroy_array_loop(const sema::Type* element, lowir::Value base, uint64_t count, uint32_t location);

	// ------------------- polymorphic classes (lower/dispatch_lowering.cpp)
	// 12.7p4: the object's virtual table pointer set to its class's table.
	void store_vptr(uint32_t location);
	void store_table(const Place& object, const sema::ClassEntity* entity, uint32_t segment);
	void tables_stored(uint32_t location);
	bool destructor_stores_tables() const;
	// Virtual bases (lower/virtual_base_lowering.cpp): a virtual base's
	// address from the object's table; in a base-object entry taking a VTT,
	// `this`'s from the construction table the VTT names (its table and
	// rows loaded once per block).
	lowir::Value virtual_base_address(lowir::Value object, const sema::ClassEntity* cls, const sema::ClassEntity* base);
	bool is_this_value(lowir::Value object) const;
	RValue construction_table();
	RValue construction_row(const sema::ClassEntity* cls, const sema::ClassEntity* base);
	RValue virtual_base_pointer(const RValue& value, const sema::ClassEntity* from, const sema::ClassEntity* to,
		bool nonnull, uint32_t location);
	// A virtual base subobject of an object (none: `this`).
	lowir::Value virtual_base_object(
		lowir::Value object, const sema::ClassEntity* cls, const sema::ClassEntity* base, uint32_t location);
	void hidden_base_arguments(size_t base, uint32_t location);
	// The VTT part pushed right after the object, before the visible
	// arguments are evaluated (the course's order).
	void hidden_base_vtt(uint32_t location);
	lowir::Value hidden_vtt_value();
	// A base subobject's construction (initialize_subobjects), and a
	// class's virtual bases in construction order; the index of each
	// virtual base's initialization in the constructor's plan by its
	// position in VirtualBases (PointerMap::kMissing for none).
	void construct_base(const sema::SubobjectInit& init, uint32_t index, uint32_t location);
	const std::vector<const sema::BaseClass*>& virtual_base_order(const sema::ClassEntity* entity);
	const std::vector<uint32_t>& virtual_base_inits(const sema::ClassEntity* entity);
	// A virtual call's callee: the function address in the slot of the
	// object's table.
	lowir::Value virtual_callee(lowir::Value object, uint32_t slot);
	// A member function call's callee: the final overrider when the call
	// dispatches (`signature` then its boundary), else the function.
	lowir::Value member_callee(
		const sema::FunctionEntity* function, lowir::Value object, bool dispatch, lowir::SignatureId& signature);
	// A deleting entry: whether it is the complete destructor's code (else
	// a call of the complete entry), the call, and the deallocation.
	bool deleting_inlines_body(uint32_t location);
	void call_complete_destructor(uint32_t location);
	void deallocate_object(uint32_t location);
	// 5.3.5p3, 12.5p4: a delete-expression of an object with a virtual
	// destructor calls the deleting entry of its dynamic type.
	void virtual_deletion(const sema::ClassEntity* entity, const RValue& pointer);
	// A delete-expression's non-virtual destruction of its object.
	void destroy_deleted(const sema::ClassEntity* entity, lowir::Value object);

	// --------- synthesized copy and move members (lower/copy_lowering.cpp)
	void copy_members(uint32_t location);
	static bool trivial_subobject(const sema::SubobjectInit& init);
	uint64_t subobject_offset(const sema::SubobjectInit& init, uint32_t location);
	lowir::Value source_object();
	lowir::Value subobject_of(lowir::Value object, const sema::SubobjectInit& init, uint32_t location);
	void copy_span(lowir::Value source, lowir::Value destination, uint64_t size, uint64_t alignment);
	void copy_subobject(const sema::SubobjectInit& init, bool assignment, uint32_t location);
	void copy_call(const sema::SubobjectInit& init, lowir::Value destination, lowir::Value source, uint32_t location);
	void call_copy(const sema::FunctionEntity* function, EntryVariant variant, lowir::Value destination,
		lowir::Value source, bool throws, uint32_t location);
	lowir::Value copied_parameter(
		const sema::FunctionEntity* constructor, const sema::Type* parameter, lowir::Value source, uint32_t location);
	void copy_elements(const sema::SubobjectInit& init, const sema::Type* type, uint32_t location);
	uint32_t copy_bit_fields(uint32_t first, uint32_t location);
	bool unit_transferable(uint32_t first, uint32_t last, const BitField& field, uint32_t location);
	static bool copied_as_bytes(const sema::SubobjectInit& init);
	void subobject_bytes(const sema::SubobjectInit& init, uint64_t& from, uint64_t& to, uint32_t location);

	// ------------- conversion functions (lower/conversion_lowering.cpp)
	static bool conversion_function(const sema::Conversion* conversion);
	static bool elides_conversion(const sema::FunctionEntity* constructor, const sema::Conversion* conversion);
	lowir::Value conversion_object(
		const syntax::Expr& expr, const sema::ExprFacts* facts, const sema::FunctionEntity* function);
	RValue call_conversion(const syntax::Expr& expr, const sema::ExprFacts* facts, const sema::FunctionEntity* function,
		lowir::Value destination);
	lowir::Value conversion_temporary(const syntax::Expr& expr, const sema::FunctionEntity* function, bool persistent);
	RValue converted_operand(const syntax::Expr& expr);
	RValue converted_value(const syntax::Expr& expr, const sema::Conversion* conversion, ConversionContext context);
	lowir::Value converted_reference(
		const syntax::Expr& expr, const sema::Conversion* conversion, const sema::Type* reference, bool persistent);

	// ------------------------ cleanup regions (cleanup_lowering.cpp)
	bool may_throw(const syntax::Expr* expr);
	bool call_may_throw(const sema::FunctionEntity* function) const;
	bool branch_temporaries(const syntax::Expr* expr, unsigned depth);
	// Tracked temporary lifetimes (tracked_lifetime_lowering.cpp): whether
	// the expression makes a temporary in a branch its full-expression
	// outlives (`root`: it is the full-expression); the tracking of the
	// full-expression whose segment began (its expressions, `discarded`: an
	// expression statement's); a tracked temporary's flag cleared at the
	// start; the regions' landing (a placeholder until the chain is made,
	// each region's marker noted); the chain, made and placed after the
	// full-expression's first block; the end.
	bool guarded_temporaries(const syntax::Expr* expr, bool root, unsigned depth);
	bool operand_temporaries(const syntax::Expr* operand, const sema::Type* object);
	void begin_tracking(const syntax::Expr* const* exprs, size_t count, bool root, bool discarded);
	void track_reset(const lowir::Instruction& clear);
	lowir::BlockId tracked_target();
	lowir::Value tracked_storage(lowir::Value address);
	// Its regions land on the chain (an aggregate's members being made
	// unwind through their own chain, its temporaries flag-checked there).
	bool tracked_region() const { return tracked_.active && !tracked_.construction && aggregate_members_ == SIZE_MAX; }
	void note_region(lowir::InstId marker);
	const CallMark* earliest_mark() const;
	void finish_tracking();
	void place_tracked(const std::vector<ChainBlock>& made);
	void end_tracking();
	bool begin_full_expression(const syntax::Expr* expr, bool discarded = false, bool throws = false);
	// `constructor`: the function the initializer's arguments call, when
	// known (its parameters the caller destroys make temporaries).
	bool begin_initializer(const syntax::Initializer* init, const sema::FunctionEntity* constructor = nullptr);
	// Itanium 3.1.2.3: a class parameter passed by address whose destructor
	// runs is the caller's temporary, destroyed when the call's
	// full-expression ends (as GCC and Clang).
	bool caller_destroyed(const sema::Type* parameter);
	bool invokes_caller_destroyed(const syntax::Expr& e, const sema::ExprFacts* facts);
	bool caller_destroys(const sema::Type* function);
	bool caller_destroys(const sema::FunctionEntity* function);
	bool begin_segment();
	void end_full_expression(bool began);
	void drop_segment_floor();
	void begin_construction(const syntax::Initializer* init);
	void end_construction();
	bool branched_regions(const syntax::Expr* root);
	void before_throwing_call();
	void paused_call_region(lowir::Opcode op);
	// Opens the alive objects' region (`empty`: even with none alive, for
	// a temporary about to be constructed): at a pending full-expression
	// start, else at the innermost throwing call's start in this block,
	// else here.
	void open_region(bool empty, bool here = false);
	void close_region();
	bool drop_empty_region();
	// The landing blocks pending when an empty region was dropped.
	size_t dropped_chains_ = 0;
	// The kind of the block a region's end continues in (a throw's
	// allocation's, the course's spelling, else a call's).
	lowir::BlockKind region_end_kind_ = lowir::BlockKind::CallUnwindEnd;
	// A region's landing blocks wait for the next placement (a class
	// return's result, see class_return).
	bool defer_landings_ = false;
	// A class return's result construction opens its region where it is
	// called (see class_return).
	bool result_here_ = false;
	// The temporaries registered so far (temporary_alive).
	uint32_t temporaries_made_ = 0;
	// An object just joined the alive chain: the open region splits (its
	// handler destroys it from here on), else the region opens no earlier
	// than here.
	void split_region();
	// Class temporaries (12.2): whether evaluating the expression makes one
	// that is destroyed; one made (alive until its full-expression ends);
	// those of the full-expression that began at `first` destroyed (and,
	// with `pop`, forgotten).
	// `reachable`: a conditional's operand never evaluated is skipped.
	// `object`: the expression initializes an object of that class itself
	// (an array element), which its prvalue of the class or its converting
	// constructor makes (12.8p31, 8.5p16): that object is no temporary.
	bool creates_temporaries(const syntax::Expr* expr, CallTemporaries* calls = nullptr, bool reachable = false,
		const sema::Type* object = nullptr);
	void scan_default_temporaries(
		const sema::FunctionEntity* function, uint32_t written, const syntax::Expr* parameter_object);
	void scan_constructor_temporaries(const syntax::Expr& expr, const syntax::Expr* parameter_object);
	static bool CallResult(const syntax::Expr& e, const sema::ExprFacts* facts);
	bool destroyed_temporary(const sema::Type* type);
	// False for one made in a conditional's operand that is never
	// evaluated (sema::kThenUnreachable): none is registered.
	bool temporary_alive(lowir::Value object, const sema::Type* type, uint32_t saved_elements = 0);
	// `completed`: the declared object the full-expression initialized,
	// destroyed first when a temporary's destructor throws (15.2p1: the
	// reverse order of the completion of their construction).
	void destroy_temporaries(size_t first, bool pop = true, const Cleanup* completed = nullptr);
	// `zeroed`: its construction calls nothing (under hidden regions it
	// opens its region itself).
	void temporary_region(bool zeroed = false, bool branched = false);
	bool pending_mark_region();
	void open_deferred_region();
	void temporary_done(bool list = false);
	void region_event();
	bool chain_empty() const { return cleanups_.size() <= std::max(segment_floor_, region_floor()); }
	void begin_call(bool throws);
	void note_call_arguments(const sema::FunctionEntity* function, const syntax::Slice<syntax::Expr*>& written);
	void end_call(bool throws);
	// The unwinding chain of the first `count` alive objects.
	lowir::BlockId unwind_entry(size_t count);
	lowir::BlockId return_entry();
	lowir::BlockId context_return_entry();
	void place_return_block(const ChainBlock& block);
	enum class ChainKind : uint8_t
	{
		Unwind,
		// Unwinding with more than eight objects alive.
		Conditional,
		Return
	};
	lowir::BlockId chain_entry(ChainKind kind, lowir::BlockId& terminal, size_t count);
	// The course's spellings of a chain's terminal, and of an alive
	// object's unwinding block: a construction's prefix (a member being
	// initialized, an array's made elements, a new-expression's storage),
	// a guarded temporary's conditional dispatch, else a cleanup action.
	static lowir::BlockKind chain_terminal_kind(ChainKind kind);
	static lowir::BlockKind unwind_block_kind(const Cleanup& cleanup);
	// 15.2p1: the handler of the alive object at `depth` whose destructor
	// may throw on a normal path, destroying the objects under it (kNoId
	// when it cannot throw or none is under it); `contexts`: the
	// exception-handling contexts still entered (a jump out of the others
	// left them, 15.1p2).
	lowir::BlockId destruction_handler(size_t depth, size_t contexts = SIZE_MAX);
	// 6.6p2, 15.2p1: the landing of a local destroyed at its scope's end
	// whose destructor may throw, when objects or handlers lie under it
	// inside the innermost try (lexical_cleanup_lowering.cpp): the lexical
	// chain of the first `count` objects, apart from the calls' chains.
	lowir::BlockId lexical_handler(size_t depth, size_t contexts);
	lowir::BlockId lexical_entry(size_t count);
	void place_lexical_block(const ChainBlock& block);
	void place_lexical_blocks();
	// An element handler's end (see construct_elements).
	void element_exit();
	std::vector<ChainBlock> lexical_pending_;
	// A lexical entry identifies the complete context stack. Returned-result
	// destruction adds one distinct dispatch for that same entry.
	std::vector<lowir::BlockId> lexical_dispatch_;
	bool lexical_placing_ = false;
	// A constructor's or destructor's body detached from its subobjects'
	// cleanup (a try, a local whose destructor may throw, a statement that
	// may throw while a local is alive): its unwinding reaches their
	// cleanup entry (`body_target_`, while the body is lowered) instead of
	// resuming into it; `body_depth_` the contexts entered at its start.
	bool body_unwinds(const syntax::Stmt& stmt, bool& alive);
	// A detached body's cleanup landing (the enclosing tries' clauses, its
	// record popped, then its entry, placed), and the cleanup's end.
	void open_detached_cleanup(lowir::BlockId entry);
	void close_detached_cleanup(bool detached);
	bool detached_body();
	lowir::BlockId body_unwind_target() const;
	lowir::BlockId body_terminal(int key);
	bool body_detached_ = false;
	lowir::BlockId body_target_ = lowir::kNoId;
	// The initializer a local reference's binding (bind_extended) lowers.
	const syntax::Expr* extension_root_ = nullptr;
	// The declaration whose initializer's segment is pending: its start,
	// before the declared object's address (kNoId: none).
	lowir::BlockId declared_start_block_ = lowir::kNoId;
	lowir::InstId declared_start_after_ = lowir::kNoId;
	// A destructor's detached body's cleanup entry, for its epilogue, and
	// a constructor's latest subobject's.
	lowir::BlockId body_entry_ = lowir::kNoId;
	lowir::BlockId constructor_entry_ = lowir::kNoId;
	size_t body_depth_ = 0;
	lowir::BlockId body_terminals_[2] = { lowir::kNoId, lowir::kNoId };
	lowir::BlockId body_terminal_target_ = lowir::kNoId;
	uint32_t body_terminal_stack_ = 0;
	bool destructor_may_throw(const sema::Type* type) const;
	void place_chain_blocks();
	void place_late_blocks();
	void place_blocks(const std::vector<ChainBlock>& blocks);
	void emit_marker(lowir::Opcode op, lowir::BlockId target);
	lowir::Value emit_call(lowir::Type result, lowir::Value callee, size_t base, lowir::SignatureId signature,
		bool may_throw, bool elision = false);
	uint32_t count_returns(const syntax::Stmt& stmt, uint32_t live);
	uint32_t declared_live(const sema::Binding* binding, const syntax::Initializer* initializer, uint32_t live);
	void scan_body();
	// 15.5.1: a function with a non-throwing exception-specification whose
	// body may throw calls std::terminate when an exception leaves it.
	bool initializer_may_throw(const syntax::Initializer* init);
	bool statement_may_throw(const syntax::Stmt& stmt);
	bool declarator_may_throw(const syntax::InitDeclarator& init);
	bool definition_may_throw();
	bool definition_statements_may_throw();
	bool plan_may_throw(const sema::SubobjectInit* inits, uint32_t count);
	// `outside_tries`: a landing no try statement encloses (the unwinding
	// chains' terminal), whatever the contexts where it is placed.
	void emit_resume(bool outside_tries = false);
	void emit_terminate();
	lowir::Value zero_result();
	void place_terminate_landing();
	void place_unexpected_landing();
	void goto_statement(const syntax::GotoStmt& stmt);
	// `contexts`: a return inside exception-handling contexts may share
	// (the course's form for a scalar result).
	bool shared_return(bool contexts = false);
	void flush_deferred();
	// A region's landing calling std::terminate for a destruction during
	// unwinding (`shared`: the function's one for its scopes' objects).
	lowir::BlockId cleanup_terminate(bool shared);
	void place_terminates();
	// Arrays of class objects: elements constructed in order (unrolled up
	// to eight, else a loop), each constructed element destroyed if a later
	// one's construction throws.
	lowir::Value array_base(const ArrayBase& where, uint32_t location);
	void construct_elements(const sema::Type* element, const ArrayBase& where, uint64_t count,
		const sema::FunctionEntity* constructor, uint32_t location);
	void destroy_elements(const sema::Type* element, const ArrayBase& where, uint64_t first, uint32_t location);
	// 15.2p3: a destruction during unwinding whose destructor exits by an
	// exception calls std::terminate (15.5.1p1): it runs in an explicit
	// region whose handler, `guard` (made when kNoId), does; kNoId when the
	// destructor cannot throw (`throws`). The guard is placed after a
	// terminated block.
	lowir::BlockId open_unwind_guard(bool throws, lowir::BlockId guard = lowir::kNoId);
	bool subobject_destructor_may_throw(const Subobject& subobject) const;
	void close_unwind_guard(lowir::BlockId guard);
	void place_unwind_guard(lowir::BlockId guard);
	// Elements `count` - 1 down to 0 whose destructor may throw: each
	// element's handler destroys the ones before it and resumes.
	void destroy_elements_guarded(const sema::Type* element, const ArrayBase& where, uint64_t count, uint32_t location);
	// An array element's default construction whose call makes
	// temporaries (a by-value parameter the caller destroys, a default
	// argument's temporary) ends them before the next element (12.2p4). Inside its scope the enclosing full-expression's state is
	// set aside and calls open no region; each temporary made has an
	// explicit region of its own (temporary_alive) whose handler destroys
	// it and unwinds on.
	struct ElementScope
	{
		bool active;
		size_t first;
		size_t handlers;
		bool full_expression;
		bool construction;
		bool region_open;
		bool eager_pending;
		bool nothrow_segment;
		bool stale_region;
		bool deferred_region;
		bool region_segment;
		bool hidden_eh;
		bool segment_temporaries;
		bool tracked;
		bool share;
	};
	// Whether the construction makes temporaries; `throws`: whether it may
	// throw (the constructor, or a default argument).
	bool element_temporaries(const sema::FunctionEntity* constructor, bool* throws = nullptr);
	// `share`: consecutive elements under the same EH state reuse the
	// previous element's temporaries' slots and handler blocks (their
	// lifetimes end before the next element begins).
	ElementScope begin_element_temporaries(bool active, bool share = false);
	void end_element_temporaries(const ElementScope& scope);
	// A caller-destroyed argument's slot (a shared element's reused one).
	lowir::SlotId argument_slot(lowir::Type type, lowir::SlotKind kind);
	// The EH state elements must agree on to share handler blocks.
	struct ElementKey
	{
		uint32_t markers;
		uint32_t regions;
		bool open;
		size_t cleanups;
		size_t contexts;
		uint32_t explicit_regions;
		bool operator==(const ElementKey& o) const
		{
			return markers == o.markers && regions == o.regions && open == o.open && cleanups == o.cleanups &&
				contexts == o.contexts && explicit_regions == o.explicit_regions;
		}
	};
	ElementKey element_key() const;
	// Constructors' and destructors' subobject cleanups.
	void subobject_constructed(const Subobject& subobject, uint32_t location);
	void destroy_subobject(const Subobject& subobject, uint32_t location, bool unwinding);
	// A destruction during unwinding is inside its terminate region.
	bool terminate_wrapped_ = false;
	void destruction_order(std::vector<Subobject>& out, uint32_t location);
	bool destructor_regions(uint32_t location);
	void destructor_epilogue(uint32_t location);
	void destroy_local(const Cleanup& cleanup, lowir::BlockId below, bool unwinding);
	// `returning`: a return, which leaves the result's local constructed.
	void destroy_cleanups(size_t depth, bool returning = false, size_t contexts = SIZE_MAX);
	void leave_scope(size_t depth);
	// `type`: the object's, when not the variable's (a range-based for's
	// __range bound to a class prvalue, see range_variable).
	void local_object(const sema::VariableEntity* variable, const Place& place, const syntax::Initializer* initializer,
		uint32_t location, const sema::Type* type = nullptr, const StaticInitialization* initialization = nullptr);
	void construct_array(const sema::Type* type, const Place& place, const syntax::Initializer* init,
		const sema::VariableEntity* variable, uint32_t location);
	lowir::Value subobject_address(const sema::BaseClass* base, const sema::DataMember* member, uint32_t location);
	// A member's offset in `*this` (an object of the function's class, an
	// anonymous class's own object in its special member functions), and
	// the offset of the member's class object there.
	uint64_t this_member_offset(const sema::VariableEntity* member, uint32_t location);
	uint64_t this_holder_offset(const sema::VariableEntity* member, uint32_t location);
	void initialize_subobjects(uint32_t location);
	bool tail_occupied(uint32_t index, uint32_t location);
	uint64_t subobject_bit_limit();
	void forward_parameters(const sema::FunctionEntity* constructor, lowir::Value object, uint32_t location);
	const syntax::Initializer* mem_initializer_view(const syntax::MemInitializer* written, syntax::Initializer& view);
	// Whether the member's initialization writes anything.
	bool initialize_member(const sema::SubobjectInit& init, uint32_t location);
	void initialize_scalar_member(const sema::VariableEntity* variable, const sema::DataMember* member,
		const syntax::Initializer& written, uint32_t location);

	// ------------------- aggregates, bit-fields (lower/aggregate_lowering.cpp)
	lowir::Value path_address(const AddressPath& path);
	bool aggregate_stores(const sema::ClassEntity* entity);
	void aggregate_class(const sema::ClassEntity* entity, uint8_t cv, const AddressPath& path,
		const syntax::BracedInitList* list, uint32_t& next, uint32_t location);
	void aggregate_item(const sema::Type* type, const AddressPath& path, const syntax::BracedInitList* list,
		uint32_t& next, bool value_first, uint32_t location);
	// `element`: an array element (an omitted one's default construction
	// ends its temporaries, see begin_element_temporaries).
	void aggregate_class_item(const sema::ClassEntity* cls, const sema::Type* type, const AddressPath& path,
		const syntax::BracedInitList* list, uint32_t& next, uint32_t location, bool element = false);
	void materialize_member(
		const syntax::Expr& element, const AddressPath& path, uint32_t location, lowir::Value object = lowir::Value());
	bool member_may_throw(const sema::ClassEntity* cls, const syntax::Expr* element);
	static bool does_nothing(const sema::FunctionEntity* constructor);
	void aggregate_array(const sema::Type* type, const AddressPath& path, const syntax::Expr* item, uint32_t location);
	RValue member_operand(const syntax::Expr& expr);
	void aggregate_object(const sema::Type* type, const AddressPath& path, const syntax::BracedInitList* list,
		bool root_address, uint32_t location);
	// Whether an expression names the entity (evaluated operands only).
	bool mentions(const syntax::Expr* root, const sema::Entity* entity);
	void aggregate_array_object(const sema::Type* type, const Place& place, const syntax::BracedInitList& list,
		const sema::VariableEntity* variable, uint32_t location);
	// 15.2p2 (1.8p2): an array of destructible class objects whose
	// elements' initialization may throw is one object of the chain while
	// they are made, the count made in a slot (SIZE_MAX: not counted); an
	// element made, the members its initialization tracked (from `first`)
	// leave the chain.
	bool counts_elements(const sema::Type* type, const syntax::Expr* list);
	// Whether an omitted object of the class (8.5.1p7, 8.5p7) may be
	// initialized by a constructor that may throw.
	bool omitted_may_throw(const sema::ClassEntity* cls, uint32_t depth);
	size_t push_counted(
		const sema::Type* type, lowir::Value storage, ElementCount& count, lowir::SlotId slot = lowir::kNoId);
	// A row of the counted array begins (SIZE_MAX: not counted apart) and
	// ends, the parent counting its elements.
	size_t begin_row(const sema::Type* type, lowir::Value storage, ElementCount& row);
	void end_row(ElementCount* parent, const ElementCount& row, size_t counted);
	// A local aggregate's aggregate member made: one object of the chain in
	// place of the members its initialization tracked (from `first`).
	void member_made(const sema::Type* type, lowir::Value storage, size_t first);
	void end_counted(size_t counted);
	void element_made(ElementCount& count, size_t first);
	void drop_member_cleanups(size_t first);
	void aggregate_elements(const sema::Type* type, const AddressPath& path, const syntax::BracedInitList* list,
		uint32_t& next, uint32_t location);
	PathStep element_step(const sema::Type* element, uint32_t location);
	void skip_item(const sema::Type* type, const syntax::BracedInitList* list, uint32_t& next);
	bool helper_members(
		const sema::ClassEntity* entity, const syntax::BracedInitList* list, uint32_t& next, bool* full = nullptr);
	bool helper_eligible(
		const sema::ClassEntity* entity, const sema::Type* element, const syntax::BracedInitList& list, uint64_t bound);
	bool helper_shape(const sema::ClassEntity* entity, const syntax::BracedInitList* list, uint32_t& count);
	void aggregate_helper_call(const sema::ClassEntity* entity, uint32_t count, lowir::Value object,
		const syntax::BracedInitList* list, uint32_t& next, uint32_t location, unsigned cv = 0);
	void structured_class(const sema::ClassEntity* entity, uint8_t cv, lowir::Value object,
		const syntax::BracedInitList* list, uint32_t& next, uint32_t location);
	// `field`: the object is a class member, whose scalar array elements
	// are indexed in their type from the member's address.
	void structured_item(const sema::Type* type, lowir::Value address, const syntax::BracedInitList* list,
		uint32_t& next, uint32_t location, bool field = false, bool array_element = false);
	void structured_array(const sema::Type* type, lowir::Value address, const syntax::BracedInitList* list,
		uint32_t& next, uint32_t location, bool field);
	lowir::Value structured_element(
		lowir::Value array, const sema::Type* element, uint64_t index, uint32_t location, bool field);
	void copy_object(const sema::Type* type, lowir::Value source, lowir::Value destination, uint32_t location);
	lowir::Value va_list_tag(const syntax::Expr& list);
	RValue gnu_intrinsic(const syntax::CallExpr& expr, sema::Intrinsic kind);
	RValue bit_count(sema::Intrinsic kind, RValue x);
	RValue overflow_intrinsic(const syntax::CallExpr& expr, sema::Intrinsic kind);
	RValue sign_bit(const RValue& x);
	RValue flt_rounds();
	// Binary16 and binary128 values (lower/soft_float_lowering.cpp): objects
	// of their bytes, operated on by the runtime's soft-float routines
	// (`second`: a binary routine's right operand).
	RValue soft_call(SoftFloatHelper which, const RValue& first, const RValue* second = nullptr);
	RValue soft_constant(SoftFloat kind, uint64_t low, uint64_t high);
	RValue soft_literal(SoftFloat kind, const char* text, uint32_t length, uint32_t location);
	RValue soft_zero(SoftFloat kind);
	RValue soft_convert(const RValue& value, const sema::Type* from, const sema::Type* to);
	RValue bfloat_convert(const RValue& value, const sema::Type* from, const sema::Type* to);
	RValue bfloat_widened(const RValue& value);
	bool bfloat_operation(const syntax::Expr& expr) const;
	RValue bfloat_float(const syntax::Expr& expr, const RValue& value);
	RValue bfloat_arithmetic(const syntax::Expr& expr);
	RValue bfloat_promoted(const syntax::Expr& expr);
	RValue bfloat_compound(const syntax::BinaryExpr& expr, TokenKind op, const sema::Type* left_type, const RValue& old,
		const RValue& right);
	RValue soft_arithmetic(lowir::BinaryOp op, SoftFloat kind, const RValue& left, const RValue& right);
	RValue soft_negate(SoftFloat kind, const RValue& value);
	RValue soft_compare(lowir::ComparePredicate predicate, SoftFloat kind, const RValue& left, const RValue& right);
	RValue soft_magnitude_at_least(SoftFloat kind, const RValue& x, uint64_t half_bits, uint64_t quad_high);
	// Complex values (lower/complex_lowering.cpp): __builtin_complex's, a
	// part of one (`imaginary`: __imag__) as a value or a glvalue, and a
	// zero object value (value-initialization).
	RValue make_complex(const syntax::CallExpr& expr);
	RValue complex_part(const syntax::Expr& operand, bool imaginary);
	Place complex_part_place(const syntax::Expr& operand, bool imaginary);
	RValue zero_object_value(lowir::Type type);
	// Complex arithmetic: a value of its parts and the parts of a value; a
	// conversion to a complex type, a value's truth, equality; -z and ~z;
	// + - * / (`left_complex`, `right_complex`: an operand of the complex
	// type, else of its part type), a product and a quotient of parts.
	RValue complex_value(const sema::Type* type, const RValue& real, const RValue& imaginary);
	void complex_parts(const RValue& value, const sema::Type* type, RValue& real, RValue& imaginary);
	RValue complex_convert(
		const RValue& value, const sema::Type* from, const sema::Type* to, ConversionContext context);
	RValue complex_truth(const RValue& value, const sema::Type* from);
	RValue complex_equal(bool equal, const sema::Type* type, const RValue& left, const RValue& right);
	RValue part_negate(const sema::Type* part, const RValue& value);
	RValue complex_unary(TokenKind op, const sema::Type* type, const RValue& value);
	RValue complex_arithmetic(TokenKind op, const sema::Type* type, const RValue& left, bool left_complex,
		const RValue& right, bool right_complex);
	RValue complex_helper_call(
		bool divide, const sema::Type* type, const RValue& a, const RValue& b, const RValue& c, const RValue& d);
	RValue complex_product(const sema::Type* type, const RValue& a, const RValue& b, const RValue& c, const RValue& d);
	RValue complex_quotient(const sema::Type* type, const RValue& a, const RValue& b, const RValue& c, const RValue& d);
	// Vectors (lower/vector_lowering.cpp): a value's storage, a lane read or
	// written (a bool lane's bit), a zeroed vector's slot, the comparison of
	// two vectors (`result` the truth vector's type), the conversion and
	// reduction builtins; __builtin_bit_cast.
	lowir::Value vector_storage(const RValue& value);
	RValue vector_lane(lowir::Value address, const sema::Type* vector, uint64_t lane);
	void set_vector_lane(lowir::Value address, const sema::Type* vector, uint64_t lane, const RValue& value);
	lowir::Value zeroed_vector(const sema::Type* vector, lowir::SlotId& slot);
	RValue vector_list(const sema::Type* vector, const syntax::BracedInitList& list);
	Place vector_lane_place(const syntax::SubscriptExpr& expr);
	RValue vector_lane_value(const syntax::SubscriptExpr& expr);
	RValue vector_compare(const syntax::BinaryExpr& expr, const RValue& left, const RValue& right);
	RValue vector_arithmetic(const syntax::BinaryExpr& expr, const RValue& left, const RValue& right);
	RValue vector_operation(TokenKind op, const sema::Type* vector, const RValue& left, bool left_vector,
		const RValue& right, bool right_vector);
	RValue vector_unary(TokenKind op, const sema::Type* vector, const RValue& value);
	RValue convert_vector(const syntax::BuiltinTraitExpr& expr);
	RValue reduce_vector(const syntax::CallExpr& expr, sema::Intrinsic kind);
	RValue vector_lanes(const syntax::CallExpr& expr, sema::Intrinsic kind);
	RValue shift_vector_bytes(const syntax::CallExpr& expr, sema::Intrinsic kind);
	RValue truncate_vector(const syntax::CallExpr& expr);
	RValue reinterpret_bytes(const RValue& value, const sema::Type* to);
	RValue bit_cast(const syntax::BuiltinTraitExpr& expr);
	// A builtin's value: __builtin_va_arg's argument, a complex part, a
	// vector conversion, a bit cast.
	RValue builtin_trait_value(const syntax::BuiltinTraitExpr& trait);
	RValue atomic_intrinsic(const syntax::CallExpr& expr, sema::Intrinsic kind);
	RValue atomic_update(
		sema::Intrinsic kind, lowir::Type type, lowir::Value target, const RValue& value, lowir::MemoryOrder order);
	lowir::MemoryOrder memory_order(const syntax::Expr& order);
	BitField bit_field(const sema::VariableEntity* variable, uint32_t location);
	RValue read_bit_field(const BitField& field, const Place& unit, bool typed = true);
	RValue load_unit(const BitField& field, lowir::Type type, const Place& unit);
	void store_unit(const BitField& field, lowir::Type type, const RValue& value, const Place& unit);
	// A bit-field's storage unit, whose accesses are not `typed`.
	static Place untyped_unit(const Place& unit);
	RValue declared_width(const BitField& field, const RValue& value, bool typed);
	RValue sign_extend(const BitField& field, const RValue& value);
	RValue write_bit_field(const BitField& field, const Place& unit, const RValue& value, bool typed = true);
	const sema::Type* field_operation_type(const BitField& field);
	void merge_bit_field(const BitField& field, lowir::Type type, const Place& unit, const RValue& masked);
	RValue to_unit(const BitField& field, const RValue& value, const sema::Type* from);
	Place bit_field_unit(const syntax::MemberExpr* access, const syntax::Expr& expr, const BitField& field);
	uint64_t member_end_bits(const sema::DataMember* member, uint32_t location);
	void initialize_bit_field(const sema::DataMember* member, const sema::VariableEntity* variable,
		const AddressPath& path, const syntax::Expr* element, bool value_first, uint32_t location, uint8_t cv = 0);
	// A bit-field member access (the member access or implicit member, or
	// a structured binding of a bit-field), or null, and the member it
	// designates; its value, and its assignment, compound assignment and
	// increments (their results being the value the field then holds).
	const syntax::MemberExpr* bit_field_access(const syntax::Expr& expr, bool& found);
	static const sema::VariableEntity* designated_bit_field(const syntax::Expr& expr);
	RValue bit_field_value(const syntax::Expr& expr);
	RValue bit_field_assignment(const syntax::BinaryExpr& expr);
	RValue bit_field_compound_assignment(const syntax::BinaryExpr& expr);
	RValue bit_field_increment(const syntax::UnaryExpr& expr, bool prefix);
	// The update applied to the lvalue `target`: a comma's right operand
	// after its left (5.18p1), a conditional's selected operand (5.16p4),
	// otherwise the designated field.
	RValue update_bit_field(const syntax::Expr& target, BitFieldUpdate& update);
	BitFieldUpdate assignment_update(const syntax::BinaryExpr& expr);
	// The field a non-conditional bit-field lvalue designates and its unit,
	// its side effects evaluated once (a nested assignment or increment
	// applied); false for a conditional or comma designator.
	bool resolve_bit_field(const syntax::Expr& target, BitField& field, Place& unit);
	RValue apply_bit_field_update(const BitField& field, const Place& unit, BitFieldUpdate& update);
	RValue apply_object_update(const syntax::Expr& target, BitFieldUpdate& update);
	const RValue& update_operand(BitFieldUpdate& update);

	// --------------------------------------------- expressions
	RValue rvalue(const syntax::Expr& expr);
	RValue operand(const syntax::Expr& expr, ConversionContext context);
	RValue apply_conversion(const syntax::Expr& expr, const RValue& value, ConversionContext context);
	RValue apply_converted(
		const syntax::Expr& expr, const sema::Conversion* conversion, const RValue& value, ConversionContext context);
	RValue fit(const RValue& value, const sema::Type* type);
	RValue convert(const RValue& value, const sema::Type* from, const sema::Type* to, ConversionContext context,
		bool literal = false);
	// An implicit conversion's standard conversion by its types alone (a
	// user-defined conversion's second one, a reference's temporary):
	// convert's, and 4.11p2's offset for a pointer to member of a derived
	// class (convert keeps a pointer to member's bits, as a
	// reinterpret_cast does, 5.2.10p10).
	RValue convert_implicit(const RValue& value, const sema::Type* from, const sema::Type* to,
		ConversionContext context, uint32_t location);
	RValue convert_integer(const RValue& value, const sema::Type* from, lowir::Type to, ConversionContext context,
		bool to_signed, bool literal);
	RValue to_bool_truth(const RValue& value, const sema::Type* from);
	RValue zero_value(const sema::Type* type);
	Place glvalue(const syntax::Expr& expr);
	Place variable_place(const sema::Entity* entity, const sema::Type* type, uint32_t location);
	lowir::Value address(const syntax::Expr& expr);
	bool constant_address(const sema::ConstValue& value, lowir::Value& out, uint32_t location);
	bool constant_rvalue(const sema::ConstValue& value, const sema::Type* type, RValue& out, uint32_t location);
	// The value of `this`; a pointer to a class object converted to a
	// pointer to its base class subobject (4.10p3: each step to the direct
	// base at its offset).
	lowir::Value this_pointer(uint32_t location);
	// The object `this` designates in the source (9.3.2p1) and its class:
	// the member function's own, or in a lambda's function call operator
	// the enclosing function's, which the closure captured (5.1.2p10).
	lowir::Value object_pointer(uint32_t location);
	const sema::ClassEntity* object_class() const;
	// A lambda's function call operator: its captures as locals.
	void closure_captures();
	// 5.1.2p6: the closure object of a captureless lambda-expression is
	// never needed; a call of it calls the function with its operator's
	// effect through that function's address.
	lowir::Value function_address(const sema::FunctionEntity* function, uint32_t location);
	void closure_object(const syntax::LambdaExpr& lambda, lowir::Value object, uint32_t location);
	void closure_member_region();
	void copy_captured_elements(const sema::Type* type, const sema::FunctionEntity* constructor,
		lowir::Value destination, lowir::Value source, uint32_t location);
	// How to_base reaches a virtual base: through the object's table; at
	// its offset in a complete object of class `from` (a named object of
	// class type or a temporary, whose class is its dynamic type); or
	// through the table of an object a pointer that may be null designates
	// (`p->m`: a null pointer stays null, read before no table).
	enum class BaseAccess : uint8_t
	{
		Table,
		Complete,
		Nullable
	};
	// The base subobject of `object`, an object of class `from`.
	lowir::Value to_base(lowir::Value object, const sema::ClassEntity* from, const sema::ClassEntity* to,
		uint32_t location, BaseAccess access = BaseAccess::Table);
	// The access a member of the object `p->` or `e.` designates takes.
	static BaseAccess member_base_access(const syntax::Expr& base, bool arrow);
	// 5.2.9p2, p11: a base subobject's address to its derived object's.
	lowir::Value to_derived(
		lowir::Value object, const sema::ClassEntity* from, const sema::ClassEntity* to, uint32_t location);
	// A pointer's conversion to a base class pointer (4.10p3) or a static
	// cast to a derived class pointer (5.2.9p11), null kept.
	// `reserve`: a null-tested conversion through a virtual base also
	// reserves the course's unused result slot (see the definition).
	RValue pointer_to_base(const RValue& value, const sema::ClassEntity* from, const sema::ClassEntity* to,
		uint32_t location, bool nonnull = false, bool reserve = true);
	RValue pointer_to_derived(const RValue& value, const sema::ClassEntity* from, const sema::ClassEntity* to,
		uint32_t location, bool nonnull);
	RValue adjusted_pointer(const RValue& value, uint64_t delta, bool null_test);
	// A non-static data member of the object `object` points to, which has
	// class `from` (the member's class or one derived from it, or an
	// anonymous class's object holding it or held with it, 9.5p5).
	Place member_place(lowir::Value object, const sema::ClassEntity* from, const sema::VariableEntity* member,
		const sema::Type* type, uint32_t location, BaseAccess access = BaseAccess::Table);
	// The object of class `from` moved to the object of class `to` when
	// both are the same named class's object or anonymous member objects
	// within it (9.5p5); unchanged otherwise. `from` becomes `to` then.
	lowir::Value anonymous_object(
		lowir::Value object, const sema::ClassEntity*& from, const sema::ClassEntity* to, uint32_t location);
	// The object `this` designates as the class `typed` (its type in the
	// expression): an anonymous class's member's default member
	// initializer sees that class's object (see anonymous_object).
	lowir::Value object_pointer_as(const sema::ClassEntity* typed, uint32_t location);
	// The address of a member call's object (the implicit object argument,
	// 13.3.1p4), converted to the callee's class.
	lowir::Value call_object(const syntax::CallExpr& expr, const sema::FunctionEntity* function);
	lowir::Value object_address(const syntax::Expr& expr);
	// A class prvalue materialized in a temporary object of the frame,
	// destroyed at the end of its full-expression (12.2p3).
	lowir::Value temporary_object(
		const syntax::Expr& expr, bool persistent = false, lowir::SlotKind kind = lowir::SlotKind::Tmpobj);
	// An array prvalue `A{...}` materialized likewise (8.5.4p3, 12.2p1).
	lowir::Value array_temporary(const syntax::FunctionalCastExpr& expr, bool persistent = false,
		lowir::SlotKind kind = lowir::SlotKind::Arraytmp);
	lowir::Value array_temporary(const syntax::Expr& expr, const syntax::BracedInitList& list, bool persistent,
		lowir::SlotKind kind = lowir::SlotKind::Arraytmp);
	lowir::Value array_temporary(const syntax::Expr& expr, const syntax::BracedInitList& list, const sema::Type* type,
		bool persistent, const sema::Entity* owner, lowir::SlotKind kind = lowir::SlotKind::Arraytmp);
	// ------------ std::initializer_list (lower/initializer_list_lowering.cpp)
	// The object referring to the array the list's elements initialize, a
	// temporary `owner` extends (persistent: in a global, as a static
	// owner's); an initializer-list constructor's call with one. The list
	// that initializes such an object itself, and whether its array is
	// destroyed.
	void initializer_list_object(const syntax::BracedInitList& list, lowir::Value object,
		const sema::ClassEntity* entity, const sema::Entity* owner, bool persistent, uint32_t location);
	static const syntax::BracedInitList* list_object(const syntax::Expr& expr);
	static const syntax::BracedInitList* elided_list(const syntax::Initializer* init, const sema::Type* type);
	bool destroyed_list_array(const syntax::Expr& expr);
	void list_constructor_call(const sema::FunctionEntity* constructor, EntryVariant variant, lowir::Value object,
		const syntax::BracedInitList& list, uint32_t location);
	lowir::Value backing_array(
		const syntax::BracedInitList& list, const sema::Type* array, bool persistent, const sema::Entity* owner);
	lowir::Value counted_elements(const syntax::BracedInitList& list, const sema::Type* array, lowir::Value base,
		bool persistent, const sema::Entity* owner, const Place& storage);
	lowir::Value chained_elements(const syntax::BracedInitList& list, const sema::Type* array, lowir::Value base,
		bool persistent, const sema::Entity* owner, const Place& storage);
	void enclose_element_region(size_t& covered);
	void finish_backing_array(const syntax::BracedInitList& list, lowir::Value base, const sema::Type* array,
		bool persistent, const sema::Entity* owner, const Place& storage, uint32_t saved);
	RValue operator_call(const syntax::Expr& expr, lowir::Value destination = lowir::Value());
	RValue intrinsic(const syntax::CallExpr& expr);
	RValue truth_i32(const RValue& truth);
	RValue classify_nan(const RValue& x);
	RValue classify_infinite(const RValue& x);
	RValue classify_finite(const RValue& x);
	RValue classify_normal(const RValue& x);
	void push_argument(const syntax::Expr& argument, const sema::Type* parameter);
	// The callee's default argument for parameter `index` (8.3.6), its
	// source location builtins reporting the call at `location` unless an
	// enclosing default argument's use already is (source_site_).
	void push_default_argument(
		const sema::FunctionEntity* callee, uint32_t index, const sema::Type* parameter, uint32_t location);
	RValue source_location(sema::Intrinsic kind, uint32_t location);
	RValue quiet_bit(const RValue& x);
	RValue wide_overflow(sema::Intrinsic kind, const RValue& a, const sema::IntegralKind& left_kind, const RValue& b,
		const sema::IntegralKind& right_kind, const Place& target, lowir::Type stored,
		const sema::IntegralKind& result_kind);
	void push_converted(const syntax::Expr& argument, const sema::Type* parameter, const sema::Conversion* conversion);
	// ------------------ new and delete (lower/allocation_lowering.cpp)
	RValue allocation(const syntax::NewExpr& expr);
	// A single object's new-expression tests the allocated pointer before
	// initializing it (5.3.4p13).
	bool tests_allocation(const syntax::NewExpr& expr) const;
	RValue array_allocation(const syntax::NewExpr& expr);
	// An array new-expression's sizes (5.3.4p6-p12): the innermost element
	// type, the elements and bytes per element of the first bound, the
	// cookie's bytes, the count (without parentheses) and its promoted
	// type, a constant's value, the elements of the first bound a braced
	// list initializes, and the most whose size, cookie included,
	// std::ptrdiff_t holds.
	struct ArrayNewShape
	{
		const sema::Type* element;
		const syntax::Expr* count;
		const sema::Type* count_type;
		uint64_t per_element;
		uint64_t element_size;
		uint64_t stride;
		uint64_t cookie_bytes;
		uint64_t value;
		uint64_t listed;
		uint64_t largest;
		bool cookie;
		bool constant;
	};
	ArrayNewShape array_new_shape(const syntax::NewExpr& expr);
	// 5.3.4p7: whether the count may be invalid (some value of a run-time
	// count's type, or the constant, is negative, beyond `limit`, the
	// largest valid count, or below the listed elements).
	bool count_may_be_invalid(const ArrayNewShape& shape, uint64_t& limit) const;
	// Whether the new-expression throws std::bad_array_new_length for an
	// invalid count.
	bool length_may_throw(const syntax::NewExpr& expr);
	// An invalid count's end (see array_allocation): the throw of
	// std::bad_array_new_length; `invalid` (none: always) chooses it.
	void invalid_length(const RValue* invalid);
	// Whether an array of `element` carries a cookie (Itanium 2.7): one
	// whose element class has a non-trivial destructor or whose class's
	// usual operator delete[] takes the size (`sized`,
	// sema::kSizedArrayDelete), never one the reserved placement allocation
	// function `allocation` makes.
	bool array_cookie(const sema::Type* element, bool sized, const sema::FunctionEntity* allocation) const;
	RValue constant_value(lowir::Type type, uint64_t bits);
	void zero_bytes(const RValue& storage, const RValue& size);
	void null_member_pointer_elements(
		const RValue& elements, const sema::Type* element, const RValue& count, uint32_t location);
	size_t new_list_elements(const syntax::NewExpr& expr, const syntax::BracedInitList& list, const RValue& elements,
		const RValue& raw, const RValue& size, uint32_t placement, bool rest_throws);
	void construct_new_elements(const sema::Type* element, const RValue& elements, const RValue& count,
		const RValue& raw, const RValue& size_argument, const sema::FunctionEntity* constructor,
		const sema::FunctionEntity* deallocator, uint32_t location, uint32_t placement = 0);
	// `placement`: a placement deallocation function's further arguments
	// in element_values_ (their first's index + 1), else none.
	void free_storage(const sema::FunctionEntity* deallocator, lowir::Value raw, const RValue& size, uint32_t location,
		uint32_t placement = 0);
	RValue deletion(const syntax::DeleteExpr& expr);
	// Pushes the deleted storage on the alive chain (its index); `block`: a
	// sized array deallocation function's size argument, else none.
	size_t deleted_storage(
		const sema::FunctionEntity* deallocator, lowir::Value storage, const sema::Type* object, lowir::Value block);
	void allocation_initializer(
		const syntax::NewExpr& expr, const sema::Type* type, const sema::ClassEntity* entity, const RValue& object);
	bool initialization_may_throw(const syntax::NewExpr& expr, const sema::ClassEntity* entity);
	RValue load_glvalue(const syntax::Expr& expr);
	bool read_when_discarded(const syntax::Expr& expr);
	void read_discarded(const syntax::Expr& expr);
	void discard(const syntax::Expr& expr);
	void discard_to_void(const syntax::Expr& expr);
	void imaged_discard_slots(const syntax::Expr& expr, uint32_t depth);
	RValue literal(const syntax::LiteralExpr& expr, lowir::Value destination = lowir::Value());
	RValue keyword_literal(const syntax::KeywordLiteralExpr& expr);
	RValue size_query(const syntax::Expr& expr);
	RValue binary_expression(const syntax::BinaryExpr& expr);
	RValue arithmetic(const syntax::BinaryExpr& expr, const RValue* left_value);
	RValue arithmetic_op(uint32_t op, const sema::Type* type, const RValue& left, const RValue& right);
	RValue pointer_offset(const RValue& pointer, const RValue& offset, const sema::Type* index_type,
		const sema::Type* pointer_type, bool subtract, bool literal, uint32_t location);
	RValue pointer_difference(
		const RValue& left, const RValue& right, const sema::Type* pointer_type, uint32_t location);
	RValue comparison(const syntax::BinaryExpr& expr, const RValue* left_value);
	Place assignment(const syntax::BinaryExpr& expr);
	Place compound_assignment(const syntax::BinaryExpr& expr);
	RValue unary_expression(const syntax::UnaryExpr& expr);
	// sema-recorded pointers to members (lower/member_pointer_lowering.cpp).
	RValue member_pointer_value(const syntax::UnaryExpr& expr, uint64_t adjustment = 0);
	RValue member_pointer_constant(const sema::ConstValue& value, uint32_t location);
	RValue member_function_word(const sema::FunctionEntity* function, uint64_t adjustment);
	uint64_t member_adjustment(const sema::Type* from, const sema::Type* to, uint32_t location);
	RValue member_pointer_converted(const RValue& value, uint64_t adjustment);
	RValue member_pointer_truth(const RValue& value);
	RValue member_pointer_null(const RValue& value, bool null);
	RValue member_pointer_equal(const RValue& left, const RValue& right, bool equal);
	bool member_conversion_operand(const syntax::Expr& expr, const sema::Conversion* conversion, RValue& out);
	RValue member_pointer_cast(const syntax::Expr& expr, const syntax::Expr& operand_expr);
	Place member_pointer_place(const syntax::BinaryExpr& expr);
	lowir::Value member_pointer_object(const syntax::BinaryExpr& expr);
	lowir::Value member_pointer_callee(const syntax::BinaryExpr& access, size_t object_argument);
	RValue stepped(const RValue& value, const sema::Type* type, bool decrement, uint32_t location);
	Place prefix_increment(const syntax::UnaryExpr& expr);
	RValue postfix_increment(const syntax::UnaryExpr& expr);
	Place subscript(const syntax::SubscriptExpr& expr);
	Place member(const syntax::MemberExpr& expr);
	void member_object(const syntax::MemberExpr& expr);
	RValue cast(const syntax::Expr& expr, const syntax::Expr& operand);
	RValue call(const syntax::CallExpr& expr, lowir::Value destination = lowir::Value());
	lowir::Type result_type(const sema::Type* returned, const sema::Type* function);
	lowir::Type result_type(const sema::FunctionEntity* function);
	void copy_result(const RValue& value, const sema::Type* type, lowir::Value destination, uint32_t location);
	void arguments(const syntax::Slice<syntax::Expr*>& written, const sema::Type* function,
		const sema::FunctionEntity* callee, uint32_t location);
	lowir::Value bind_reference(
		const syntax::Expr& expr, const sema::Type* reference, ConversionContext context, bool persistent = false);
	// 12.2p5 for a local reference or aggregate reference member: the
	// temporaries it is bound to live until `owner`'s scope ends.
	lowir::Value bind_extended(const syntax::Expr& expr, const sema::Type* reference, const sema::Entity* owner);
	const sema::Type* binding_temporary(const syntax::Expr& expr);
	void extend_temporary(const sema::Entity* owner);
	bool extends_temporary(const syntax::Expr* expr);
	bool aggregate_extends(const syntax::Expr* list);
	lowir::Value bind_converted(const syntax::Expr& expr, const sema::Conversion* conversion,
		const sema::Type* reference, ConversionContext context, bool persistent);
	lowir::Value temporary_storage(const sema::Type* type, bool persistent, lowir::SlotKind kind);
	static const syntax::Expr* reference_initializer(const syntax::Expr* single);
	lowir::Value block_call(const syntax::CallExpr& expr, lowir::Value destination, bool throws, lowir::Type& result,
		lowir::SignatureId& signature);
	lowir::Value scalar_temporary(const syntax::Expr& expr, lowir::Value chosen, const sema::Type* source,
		lowir::Type storage, const sema::Type* type, bool persistent, lowir::SlotKind kind);
	void extension_path(const syntax::Expr* expr);
	const syntax::Expr* full_root(const syntax::Expr* expr);
	bool branch_local(const syntax::Expr& expr) const;
	void begin_guard_scope();
	void persistent_alive(lowir::Value storage, const sema::Type* type);

	// ------------------- exceptions (lower/exception_lowering.cpp)
	// A handler's selector, numbered through the function as its clause is
	// first emitted (the course's order).
	uint32_t handler_selector(const syntax::Handler& handler);
	lowir::SymbolId handler_type_information(const sema::Type* type);
	// The statement's clauses (true: one is `...`), then, without one, the
	// enclosing statements' of its chain.
	bool emit_clauses(const syntax::TryStmt& stmt);
	void emit_try_clauses(const syntax::TryStmt& stmt, uint32_t chain_first, uint32_t chain_count);
	void try_statement(const syntax::TryStmt& stmt);
	// A try statement's region opened (its context pushed, the enclosing
	// innermost try returned) and, after the code it guards, closed with
	// its dispatch and handlers.
	size_t begin_try(const syntax::TryStmt& stmt);
	void end_try(size_t enclosing);
	// A function-try-block (15.3p4): the try statement the parser made
	// around the whole function body, whose handlers, for a constructor
	// or destructor, rethrow when control reaches their end (15.3p15).
	bool function_try_block() const { return function_->body != nullptr && function_->body->try_block != nullptr; }
	size_t begin_function_try();
	void handler(const syntax::TryStmt& stmt, const syntax::Handler& written, const RValue& exception,
		lowir::BlockId landing, const EhContext& attempt);
	void catch_variable(
		const sema::VariableEntity* variable, const RValue& caught, const Place& record, uint32_t location);
	void call_end_catch(bool unwinding = false);
	// The end of a catch whose exception object's destructor may throw, a
	// handler's exit to the first `count` objects inside the first
	// `contexts` exception-handling contexts.
	void throwing_end_catch(size_t contexts, size_t count);
	void handler_landing(const EhContext& handler_context);
	void unmatched(const EhContext& attempt);
	void leave_contexts(uint32_t extra);
	void emit_clause_chain(uint32_t first, uint32_t count);
	static bool catches_all(const syntax::TryStmt& stmt);
	const EhContext* innermost_try() const;
	size_t region_floor() const;
	// Inside a constructor's or destructor's function-try-block (the
	// outermost try) with its subobjects' cleanup record pushed
	// (`context`: of the try context at that index instead of the
	// innermost).
	bool function_try_subobjects() const { return function_try_subobjects(innermost_try_); }
	bool function_try_subobjects(size_t context) const;
	void exit_context(size_t index);
	lowir::BlockId eh_unwind_entry(size_t count);
	lowir::BlockId region_landing(size_t count);
	// CWG 2176: while a return destroys the locals after its result is
	// complete (returning_result_), the handler of a local at `depth`
	// whose destructor throws destroys that result (returned_result_)
	// first, then the objects under the local; the innermost try's clauses
	// come before it.
	lowir::BlockId result_landing(size_t depth);
	void place_eh_block(const ChainBlock& block);
	bool landing_try(EhContext& attempt) const;
	void landing_dispatch(const EhContext& attempt);
	void landing_exit(const EhContext& attempt);
	void conditional_operand(const syntax::Expr& operand_expr, lowir::Value object, uint32_t location);
	void throw_statement(const syntax::Expr* operand, uint32_t location);
	void throw_expression(const syntax::ThrowExpr& expr);
	void throw_object(const syntax::Expr* operand, uint32_t location, bool statement);
	void nested_throw_exit();
	void exception_object(
		const syntax::Expr& operand, const sema::Type* type, lowir::Value object, bool statement, uint32_t location);
	lowir::Value throw_allocation(size_t base, bool statement);
	void noreturn_exit();
	void zero_return(bool fallback = true);
	void patch_fallback_slots();
	struct FallbackPatch
	{
		lowir::InstId zero;
		lowir::InstId ret;
	};
	std::vector<FallbackPatch> fallback_patches_;

	// -------------- run-time types (lower/dynamic_type_lowering.cpp)
	Place typeid_place(const syntax::TypeTraitExpr& expr);
	RValue type_identity(const syntax::BinaryExpr& expr);
	RValue dynamic_cast_pointer(const syntax::CastExpr& expr);
	lowir::Value most_derived_object(lowir::Value object);
	Place dynamic_cast_reference(const syntax::CastExpr& expr);
	lowir::Value dynamic_type_information(lowir::Value object, RuntimeFunction failure, bool test_null = true);
	lowir::Value runtime_cast(lowir::Value object, const sema::ClassEntity* from, const sema::ClassEntity* to,
		bool reference, uint32_t location);
	void runtime_failure(RuntimeFunction failure);
	lowir::Value address_of_symbol(lowir::SymbolId symbol);

	// ------------------------------------ conditions and choices
	bool literal_truth(const syntax::Expr& expr, bool& value) const;
	void branch_condition(
		const syntax::Expr& expr, lowir::BlockId if_true, lowir::BlockId if_false, bool fold_literals);
	RValue truth(const syntax::Expr& expr);
	RValue truth_operand(const syntax::Expr& expr);
	RValue normalized_truth(const syntax::Expr& expr);
	RValue logical_value(const syntax::BinaryExpr& expr);
	RValue logical_step(const syntax::BinaryExpr& expr, const RValue* left_truth, bool local);
	// Rejects nesting beyond the shared stack budget (support/stack_budget.h).
	void check_stack(uint32_t location) const;
	RValue conditional_value(const syntax::ConditionalExpr& expr);
	void choice_operand(const syntax::Expr& operand_expr, const Choice& level);
	bool continues_choice(const syntax::Expr& expr) const;
	static bool unreachable_operand(const syntax::ConditionalExpr& expr, const syntax::Expr& operand);
	Place conditional_place(const syntax::ConditionalExpr& expr);
	void conditional_discard(const syntax::ConditionalExpr& expr);

	ProgramLowering& lowering_;
	lowir::Program& program_;
	const sema::FunctionEntity* function_;
	// A non-static member function's `this` (9.3.2), held in a slot of its
	// own; kNoId elsewhere.
	lowir::SlotId this_slot_;
	// A lambda's function call operator whose closure captured `this`: the
	// pointer member's offset, and the enclosing function's class.
	bool closure_this_ = false;
	uint64_t closure_this_offset_ = 0;
	const sema::ClassEntity* closure_this_class_ = nullptr;
	// An indirect class result's destination (`%ret`), else none, and the
	// local every return statement returns, constructed there (or null).
	lowir::Value result_destination_;
	const sema::Entity* returned_local_ = nullptr;
	// A class prvalue is being materialized only to be passed or bound.
	bool passing_empty_ = false;
	// A [[no_unique_address]] empty member is being initialized: its byte
	// lies under another member's, so no zeroing or copy of an empty class
	// writes it (see OverlapScope).
	bool overlapping_empty_ = false;
	// Sets overlapping_empty_ while a member's initialization runs.
	struct OverlapScope
	{
		OverlapScope(bool& flag, const sema::DataMember* member);
		~OverlapScope() { flag_ = saved_; }
		bool& flag_;
		bool saved_;
	};
	// A base subobject's value-initialization zeroes its data only (its
	// non-virtual size), as does its storage copy: its tail is not its own
	// (see tail_occupied).
	bool zero_data_only_ = false;
	// The outermost default argument's use (or the constructor running a
	// member's brace-or-equal-initializer) being lowered: the position the
	// source location builtins report (sema::Intrinsic::SourceFile);
	// kNoSourceSite while none is.
	static const uint32_t kNoSourceSite = 0xffffffffu;
	uint32_t source_site_ = kNoSourceSite;
	// A local's direct-initialization is constructing it, or a
	// new-expression its object (see call_special).
	bool local_direct_init_ = false;
	bool allocation_copy_ = false;
	// A constructor of a class whose implicit assignment operator the unit
	// odr-uses initializes bit-fields in the course's other order.
	// Regions opened so far (a conditional's second operand reopens one
	// its first opened).
	uint32_t regions_opened_ = 0;
	// The full-expression's root: a conditional or logical expression there
	// destroys its operands' temporaries in their branches; one elsewhere
	// keeps them to the full-expression's end, guarded (12.2p3).
	const syntax::Expr* choice_root_ = nullptr;
	uint32_t guard_scopes_ = 0;
	uint32_t guarded_branches_ = 0;
	// The tracked full-expression being lowered (see guarded_temporaries):
	// where its flags are cleared (after `after` in `block`; the first
	// cleared), its regions' landing (a placeholder until `made`), the
	// objects alive at its start and its first region marker's index in
	// tracked_markers_.
	struct TrackedLifetimes
	{
		bool active;
		bool made;
		// A new-expression's storage was freed by a landing: from there the
		// regions land on their own chains, the temporaries' destruction in
		// a region landing on the objects under them (the course's
		// construction cleanup).
		bool construction;
		lowir::BlockId block;
		lowir::InstId after;
		lowir::InstId first_reset;
		lowir::BlockId head;
		size_t floor;
		size_t markers;
	};
	TrackedLifetimes tracked_ = TrackedLifetimes();
	// A class return's result class while its full-expression begins.
	const sema::ClassEntity* result_root_ = nullptr;
	std::vector<lowir::InstId> tracked_markers_;
	// Conditional operands being lowered that are never evaluated.
	uint32_t unreachable_operands_ = 0;
	// Where the guards' flags are cleared: before the outermost guarded
	// branching.
	lowir::BlockId guard_block_ = lowir::kNoId;
	lowir::InstId guard_after_ = lowir::kNoId;
	lowir::FunctionBuilder builder_;
	// The program takes debug locations (DebugScope).
	const bool debug_;
	// By temporary: 1 for an address reached through a union member
	// (mark_untyped); empty until the first.
	std::vector<uint8_t> untyped_addresses_;
	// A base-object entry of a class with virtual bases: its VTT (a hidden
	// parameter), the construction table VTT[0] loaded in a block and the
	// rows read from it there (construction_table, construction_row).
	lowir::Value vtt_param_ = lowir::Value::none();
	lowir::BlockId vtt_table_block_ = lowir::kNoId;
	RValue vtt_table_;
	std::vector<std::pair<const sema::ClassEntity*, RValue>> vtt_rows_;
	// The VTT position of the base whose base-object entry the next
	// special member call is (set by the construction or destruction that
	// makes it).
	struct HiddenBase
	{
		bool active;
		uint32_t vtt_index;
		// The VTT part already follows the object (hidden_base_vtt).
		bool vtt_pushed;
	};
	HiddenBase hidden_base_ = HiddenBase{ false, 0 };
	// A constructor of a class with virtual bases until it stores its
	// tables; the values of `this` loaded meanwhile, or throughout a
	// base-object entry taking a VTT.
	bool tables_pending_ = false;
	std::vector<lowir::Value> this_values_;
	std::vector<const sema::BaseClass*> virtual_order_;
	std::vector<uint32_t> virtual_inits_;
	// The class of the object the last member call named (call_object).
	const sema::ClassEntity* call_object_class_ = nullptr;
	lowir::Type result_type_;
	// The body's slot scope (see ProgramLowering::claim_slot_name).
	uint32_t slot_scope_;
	bool is_main_;
	bool terminated_;
	// A namespace-scope object's initialization is being lowered: the
	// temporaries its aggregate reference members bind persist.
	bool static_object_ = false;
	// A local aggregate's members being initialized: where their cleanups
	// start (SIZE_MAX: none tracked; aggregate_object), and whether a
	// member's region was opened.
	size_t aggregate_members_ = SIZE_MAX;
	// An object's destruction is being lowered (destroy_local).
	bool destroying_ = false;
	// A region opened while a tracked aggregate's member was made; the
	// objects alive when the open region opened.
	bool member_landing_ = false;
	size_t region_depth_ = 0;
	// The open region's ordinal among those opened, and the count when
	// the tracked aggregate being made began.
	uint32_t open_serial_ = 0;
	uint32_t aggregate_serial_ = 0;
	// The open region's landing; a tracked construction's region reopened
	// at a class conditional's second operand (its first member keeps it).
	lowir::BlockId region_entry_ = lowir::kNoId;
	bool reopened_construction_ = false;
	// A complete local aggregate's address taken again (see
	// aggregate_object), and its temporaries' destruction's landing
	// destroying it from there (local_object, which resets it to none).
	lowir::Value completion_address_ = lowir::Value::none();
	bool completed_landing_ = false;
	bool member_regions_ = false;
	// The array whose elements structured_item counts (null inside an
	// element), and whether the class item being made is its element.
	ElementCount* element_count_ = nullptr;
	bool counted_item_ = false;
	// A static or thread-local object's initializer is being lowered: no
	// cleanup regions (see begin_static_initializer).
	bool static_initializer_ = false;
	// Initializing a function-local static object: its destruction is
	// registered with atexit, not a cleanup of the scope; the body's
	// static declarations so far (their globals' ordinals).
	bool static_local_ = false;
	uint32_t local_static_ordinal_ = 0;
	// The course LowIR's static reference (or block-scope static
	// std::initializer_list variable) being initialized: the temporaries
	// its initializer extends (12.2p5) are the static's to destroy. A
	// reference's each have a constructed flag; a local static's are
	// destroyed by one function registered once its initialization
	// completes (`made`, in construction order), a namespace-scope
	// reference's by the finalizer.
	struct StaticTemporaries
	{
		bool reference;
		std::vector<Finalizer> made;
	};
	struct StaticInitialization
	{
		const sema::VariableEntity* variable;
		lowir::SymbolId global;
		lowir::SymbolId guard;
		uint32_t location;
		size_t aborting;
		bool thread_safe;
		const StaticTemporaries& temporaries;
	};
	StaticTemporaries* static_temporaries_ = nullptr;
	bool static_temporary(lowir::SymbolId storage, const sema::Type* type, uint32_t location);
	const syntax::BracedInitList* list_variable(const sema::VariableEntity* variable, const syntax::Initializer* init);
	void register_static_temporaries(lowir::SymbolId global, const StaticTemporaries& temporaries);
	// A static reference's binding: the class prvalues whose temporaries
	// it extends (12.2p5: the bound object or the complete object of the
	// bound subobject), which persist in globals.
	std::vector<const syntax::Expr*> persistent_path_;
	// A local reference's (or aggregate's) binding is being lowered: the
	// temporaries on persistent_path_ live until its scope ends instead.
	const sema::Entity* scope_extension_ = nullptr;
	// A local aggregate being initialized (or the constructor whose member
	// is), whose reference members' temporaries and std::initializer_list
	// members' arrays live as long as it (until it exits; 12.2p5, 8.5.4p6).
	const sema::Entity* aggregate_extension_ = nullptr;
	// The result slot of the conditional last evaluated as a prvalue (a
	// reference bound to its value takes it as the temporary).
	lowir::Value conditional_slot_ = lowir::Value::none();
	// Declarations control cannot reach get storage only.
	bool unreachable_storage_;
	// Locals by entity (index into locals_).
	PointerMap local_index_;
	std::vector<Local> locals_;
	std::vector<JumpTargets> jumps_;
	// The automatic objects alive at the point being lowered, in order of
	// construction.
	std::vector<Cleanup> cleanups_;
	std::vector<PathStep> path_steps_;
	// The unit (its offset + 1) the initialization being lowered last
	// wrote a bit-field of, 0 for none, and the bit after the subobjects
	// that initialization has written in its object so far: a bit-field's
	// first write into a unit stores the whole unit only when none of them
	// lies in it.
	uint64_t bit_unit_;
	uint64_t bit_high_;
	// The bit a constructor's or assignment's whole unit store or base
	// storage copy may not pass (see subobject_bit_limit).
	uint64_t bit_limit_ = UINT64_MAX;
	std::vector<const sema::DataMember*> member_scratch_;
	// The element addresses of arrays the cleanups destroy at them (see
	// Cleanup::saved_elements).
	std::vector<lowir::Value> element_values_;
	// The slots of the definition's parameters, in order.
	std::vector<lowir::SlotId> parameter_slots_;
	std::vector<SwitchLabels> switch_labels_;
	// The first of the innermost switch's labels in switch_labels_, each
	// label's block by statement, and the compound statements known to hold
	// a label (1) or not (0).
	size_t switch_first_;
	PointerMap switch_blocks_;
	PointerMap label_memo_;
	PointerMap labels_;
	std::vector<lowir::Value> scratch_;
	// The arguments of the calls being lowered, innermost last (see call).
	std::vector<lowir::Value> arguments_;
	// Object mode: the psABI classes of class objects among them passed to
	// `...` as their bytes (position in arguments_, classes), for the
	// call's body.argument_classes.
	std::vector<std::pair<size_t, uint8_t>> variadic_classes_;
	// The left spines of the operator chains being lowered, and the
	// targets of condition chains.
	std::vector<const syntax::BinaryExpr*> spine_;
	std::vector<ChainTargets> targets_;
	std::vector<Choice> choices_;

	// Cleanup regions (cleanup_lowering.cpp). A call that may throw while
	// automatic objects are alive runs in an `eh_try` region whose handler
	// destroys them (one block per object, interned by object, ending in
	// `resume`); a full-expression holding such a call opens its region at
	// its start (else before the call), closes it before a terminator and
	// spills its call results to slots.
	lowir::BlockId resume_block_;
	// Returns sharing one continuation (more than one return with the same
	// objects alive): the value's slot, the chain and its terminal.
	lowir::BlockId return_terminal_;
	lowir::SlotId return_slot_;
	PointerMap return_counts_;
	// The objects alive at each label (its innermost live node + 1, 0 for
	// none), and the try blocks and handlers enclosing it (the scan's
	// count so far: `scan_contexts_`).
	PointerMap label_live_;
	PointerMap label_contexts_;
	uint32_t scan_contexts_ = 0;
	// The returns inside contexts of each innermost alive object and
	// context (the scan's: `scan_context_`).
	struct ContextReturns
	{
		const sema::Entity* variable;
		const void* context;
		uint32_t count;
	};
	std::vector<ContextReturns> context_returns_;
	const void* scan_context_ = nullptr;
	bool body_scanned_ = false;
	// A statement expression's labels, scanned when it begins: the objects
	// alive there (+ 1) stay alive at them (`scan_floor_` while scanning).
	PointerMap label_floors_;
	uint32_t scan_floor_ = 0;
	std::vector<LiveNode> live_nodes_;
	// may_throw's and creates_temporaries' work list.
	std::vector<const syntax::Expr*> throw_scan_;
	std::vector<CallMark> call_marks_;
	std::vector<ChainBlock> chain_blocks_;
	// place_chain_blocks' spare buffer.
	std::vector<ChainBlock> chain_spare_;
	// copy_members' virtual bases in copy order and whether each is
	// destroyed when a later copy throws.
	std::vector<uint32_t> virtual_copies_;
	std::vector<uint8_t> virtual_tracked_;
	// The chain of more than eight alive objects: its resume block and the
	// blocks placed at the function's end.
	lowir::BlockId conditional_resume_ = lowir::kNoId;
	std::vector<ChainBlock> late_blocks_;
	bool region_open_;
	// The region closes at terminators (full-expressions, and constructions
	// whose paths open or close regions unequally, see begin_construction),
	// else at the end of the construction it wraps (`construction_` while
	// one is lowered).
	bool region_segment_;
	bool construction_ = false;
	// Where the construction began: its region opens there when its first
	// throwing call is in another block (a conditional operand's), so that
	// every path to its end passes the region's start.
	lowir::BlockId construction_block_ = lowir::kNoId;
	lowir::InstId construction_after_ = lowir::kNoId;
	size_t construction_depth_ = 0;
	bool full_expression_;
	// A full-expression that may not throw but materializes a call's class
	// prvalue as a temporary it destroys: the course brackets its calls in
	// regions all the same, the first opened at its start; their handlers
	// destroy only its temporaries (those from `segment_floor_` on) unless
	// one is a member class template's specialization (see
	// begin_full_expression).
	bool nothrow_segment_ = false;
	size_t segment_floor_ = 0;
	// The segment's full-expression makes temporaries (its region may open
	// at its start, covering the calls before them).
	bool segment_temporaries_ = false;
	// The course's hidden regions: a full-expression making temporaries in
	// a return or condition (`hidden_context_`, read by the next
	// begin_full_expression), or in a conditional branch, treats every call
	// but a destructor as one that may throw (`hidden_eh_`, behavior-
	// neutral).
	bool hidden_context_ = false;
	bool hidden_eh_ = false;
	// A temporary's region waits for its construction (temporary_region).
	bool deferred_region_ = false;
	// A declared object's initialization begins (local_object): its first
	// call, if a constructor's, is the object's construction; that call's
	// mark is a declared object's (`declared_construction_`).
	bool declaring_ = false;
	bool declared_construction_ = false;
	// An object joined the chain under the open region, whose landing does
	// not destroy it (see before_throwing_call).
	bool stale_region_ = false;
	// The next call's arguments make temporaries (see begin_call). The
	// calls whose written arguments note_call_arguments found clean, keyed
	// by their first (`record_clean_`: creates_temporaries collects the
	// nested calls it walks in clean_scratch_).
	bool eager_call_ = false;
	bool record_clean_ = false;
	std::vector<const syntax::Expr*> clean_scratch_;
	PointerMap clean_arguments_;
	// The block where a temporary's construction last closed the region
	// without reopening it (nothing to destroy): there the region reopens
	// where the next call or temporary construction is met.
	lowir::BlockId split_block_ = lowir::kNoId;
	// Inside a conditional's second operand whose region reopened at its
	// start (the course's form): temporaries neither split nor open it.
	uint32_t reopened_choices_ = 0;
	// Handler blocks being placed (see place_blocks), or temporaries whose
	// destructors may throw being destroyed (see destroy_temporaries): the
	// regions are explicit ones, calls open none.
	uint32_t explicit_regions_ = 0;
	// Array element scopes entered (begin_element_temporaries) and the
	// temporaries made in them with their handler blocks, innermost last.
	struct ElementHandler
	{
		Cleanup cleanup;
		lowir::BlockId block;
		bool shared;
	};
	uint32_t element_scopes_ = 0;
	std::vector<ElementHandler> element_handlers_;
	std::vector<ElementHandler> element_scratch_;
	// A sharing element scope (begin_element_temporaries): the previous
	// shared scope's handlers and caller-destroyed argument slots (in
	// creation order) with the EH state after it, the current scope's
	// slots, and EH markers emitted so far.
	struct ElementSlot
	{
		lowir::SlotId slot;
		lowir::Type type;
	};
	bool element_sharing_ = false;
	std::vector<ElementHandler> element_shared_;
	std::vector<ElementSlot> element_slots_;
	std::vector<ElementSlot> element_slots_made_;
	ElementKey element_key_ = ElementKey();
	uint32_t markers_ = 0;
	// A full-expression's region not yet opened: it opens at the recorded
	// position (after `segment_after_`, kNoId: the block's start) when a
	// call that may throw is met before the first terminator.
	bool eager_pending_;
	lowir::BlockId segment_block_;
	lowir::InstId segment_after_;
	// A constructor's cleanup region (`eh_cleanup`) is open; its cleanup
	// blocks wait for the end of the current block.
	// A constructor's or destructor's base entry (12.6.2p6: a delegating
	// constructor's target and cleanup use base entries too).
	bool base_entry_;
	// A virtual destructor's deleting entry.
	bool deleting_;
	bool constructor_region_;
	lowir::BlockId constructor_cleanup_;
	std::vector<DeferredCleanup> deferred_;
	// The cleanup terminate landings made but not yet placed (placed when
	// the block making them ends), with their selectors, and the function's
	// shared one.
	struct TerminateBlock
	{
		lowir::BlockId block;
		uint32_t selector;
	};
	std::vector<TerminateBlock> pending_terminates_;
	lowir::BlockId lexical_terminate_ = lowir::kNoId;
	// A destructor's body region: its return epilogue (kNoId without
	// returns) and end blocks.
	bool destructor_region_;
	lowir::BlockId destructor_epilogue_;
	lowir::BlockId destructor_cleanup_;
	// More than eight subobjects: the slot recording which of their
	// destructions is running (0: the body).
	lowir::SlotId destructor_progress_;
	// The terminate guard of a non-throwing function (kNoId without one):
	// its landing block and the slot holding the exception; returns leave
	// its region (unless `guard_returns_` is off, on the paths that end in
	// std::terminate). In a host object a function with a dynamic-
	// exception-specification has the guard of its specification instead
	// (`unexpected_specification_`): a filter whose violation calls the
	// unexpected handler (15.5.2), which an exception unwinds to.
	lowir::BlockId terminate_landing_;
	lowir::SlotId exception_slot_;
	bool guard_returns_;
	const sema::SemanticGraph::ThrowSpecification* unexpected_specification_ = nullptr;
	// A return's completed class result needing destruction while the
	// locals are destroyed (see result_landing).
	Cleanup returned_result_ = Cleanup();
	bool returning_result_ = false;
	// begin_full_expression's temporaries known by the caller (a host
	// object's constructor parameters, see begin_initializer).
	bool forced_temporaries_ = false;
	std::vector<Subobject> subobjects_;
	// The exception-handling contexts being lowered, innermost last; the
	// try statements' first selectors (numbered once per function) and the
	// count so far; the dispatch block of each region's chain entry
	// (+ 1, 0: none).
	std::vector<EhContext> eh_contexts_;
	size_t innermost_try_ = SIZE_MAX;
	// eh_unwind_entry's scratch: the chain elements still to make, top
	// first (an object's index or a handler context's, and its key).
	struct EhElement
	{
		size_t object;
		size_t context;
		int key;
	};
	std::vector<EhElement> eh_elements_;
	// The context stacks pushed so far, and the resume block of chains
	// under a context stack without a try (with its stack).
	uint32_t eh_stack_count_ = 0;
	lowir::BlockId eh_resume_block_ = lowir::kNoId;
	uint32_t eh_resume_stack_ = 0;
	lowir::BlockId lexical_resume_ = lowir::kNoId;
	uint32_t lexical_resume_stack_ = 0;
	lowir::BlockId lexical_body_ = lowir::kNoId;
	// The chain blocks of the objects and contexts alive when each context
	// was pushed, restored when it pops (see save_chain_memos): a landing
	// back under an enclosing stack reuses that stack's blocks.
	struct ChainMemo
	{
		lowir::BlockId blocks[2];
		uint32_t stack;
		lowir::BlockId lexical;
		uint32_t lexical_stack;
	};
	std::vector<ChainMemo> chain_memos_;
	std::vector<size_t> chain_memo_marks_;
	void save_chain_memos();
	void restore_chain_memos();
	PointerMap handler_selectors_;
	uint32_t selector_count_ = 0;
	bool rethrow_handlers_ = false;
	std::vector<lowir::BlockId> landing_dispatch_;
	std::vector<ClauseSet> clause_chains_;
};

} // namespace lower
