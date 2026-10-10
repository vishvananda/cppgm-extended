#pragma once

#include <cstdint>
#include <memory>
#include <vector>

#include "ir_symbol_model.h"
#include "lowir/instruction.h"
#include "lowir/local_kinds.h"
#include "lowir/name_table.h"
#include "lowir/types.h"
#include "support/memory/arena.h"

// The typed LowIR program model (lowir.md). Producers construct it directly
// (FunctionBuilder, lowir/builder.h); the text reader and writer are adapters
// for the explicit `lowir` tool. Entities are dense ids into vectors owned by
// the program or by one function body, and names are interned NameIds, so
// cross references never go through spellings.
namespace lowir {

// ---------------------------------------------------------------------------
// Function bodies

// Temporaries are single-definition values. The first parameter_count
// temporaries of a body are its parameters (def == kNoId); the others are
// defined by the instruction `def` (kNoId while only forward referenced).
// A temporary's name is its spelling, unless it is an inlined copy
// (TempKind::Inlined, lowir/local_kinds.h), whose printed name derives
// from it.
struct Temp
{
	NameId name;
	Type type;
	InstId def;
	TempKind kind;
};

// A slot's name; or, for a kind other than Plain, what its generated name
// derives from: the source name a shadow, range-for variable or inlined
// copy shares (`name`), an unnamed parameter's position (`index`).
struct Slot
{
	NameId name;
	SlotKind kind;
	uint8_t reserved;
	uint16_t index;
	Type type;
};

// Instructions of a block form a list through Instruction::next. A block
// of a kind other than Plain prints its kind's name (an inlined copy's
// derives from the name of the block it copies).
struct Block
{
	NameId name;
	InstId first;
	InstId last;
	BlockKind kind;
};

// The spelling a temporary, block or slot has of its own: 0 for one of a
// presentation kind, whose name (if any) is only what its printed name
// derives from.
inline NameId own_name(const Temp& temp)
{
	return temp.kind == TempKind::Plain ? temp.name : 0;
}
inline NameId own_name(const Block& block)
{
	return block.kind == BlockKind::Plain ? block.name : 0;
}
inline NameId own_name(const Slot& slot)
{
	return slot.kind == SlotKind::Plain ? slot.name : 0;
}

// Object mode (Program::object_facts; the call's or va_arg's
// `[classes=<argument>:<classes>]`, `[classes=<classes>]` in the text):
// the classification (ObjectClassBits, below) of an object argument that
// call `call` passes as argument `argument` beyond its boundary's
// parameters (to `...`), or of the object the va_arg instruction `call`
// reads (argument 0), as an object-mode lowering states it. A body's
// table is ordered by `call` (the selector searches it so); a pass that
// copies such an instruction gives the copy its own entries.
struct ArgumentClasses
{
	InstId call;
	uint32_t argument;
	uint8_t classes;
};

// The body of a function definition. `layout` is the block order; blocks
// may be created before they are placed. Operand lists longer than an
// instruction's inline operands live in `operands`, literals in `literals`,
// debug locations in `locations`.
struct FunctionBody
{
	uint32_t parameter_count = 0;
	std::vector<Temp> temps;
	std::vector<Slot> slots;
	std::vector<Block> blocks;
	std::vector<BlockId> layout;
	std::vector<Instruction> instructions;
	std::vector<Value> operands;
	std::vector<Literal> literals;
	std::vector<DebugLocation> locations;
	std::vector<ArgumentClasses> argument_classes;

	DebugLocation location_of(const Instruction& inst) const
	{
		return inst.location == 0 ? DebugLocation{ 0, 0, 0 } : locations[inst.location - 1];
	}
};

// ---------------------------------------------------------------------------
// Signatures and symbols

// Object mode (Program::object_facts: `classes=` on a parameter,
// `result_classes=` on a boundary), as `obj<bytes x align>` carries no
// field types: the x86-64 psABI classification (3.2.3) of a direct object
// parameter or result (or an i128 one holding a member function pointer's
// pair), which an object-mode lowering states from the class's layout and
// a host object's call boundary follows: ObjectClassBits and
// EightbyteClass of ir_symbol_model.h.

// The parameter's pass, alias, classes (object mode) and object_bytes
// metadata are the scaffold's record.
struct Parameter : ir_model::ParameterMetadata
{
	NameId name;
	Type type;
};

// A boundary's arity, effects, unwind, return and query metadata, and the
// classes of an object result (object mode).
typedef ir_model::FunctionBoundaryMetadata BoundaryMetadata;

// A function or call boundary: parameters [first_parameter,
// first_parameter + parameter_count) of Program::parameters.
struct Signature
{
	uint32_t first_parameter;
	uint32_t parameter_count;
	Type result;
	BoundaryMetadata boundary;
};

// The named-value metadata (role, linkage, binding, storage and the
// SymbolFlag bits) is the scaffold's record; the names are interned.
struct SymbolMetadata : ir_model::SymbolFacts
{
	NameId object = 0;
	NameId section = 0;
	SymbolId tls_for = kNoId;
};

enum class SymbolKind : uint8_t
{
	// Referenced but not (yet) declared or defined.
	Unresolved,
	GlobalDeclaration,
	FunctionDeclaration,
	GlobalDefinition,
	FunctionDefinition
};

inline bool is_function_kind(SymbolKind kind)
{
	return kind == SymbolKind::FunctionDeclaration || kind == SymbolKind::FunctionDefinition;
}
inline bool is_global_kind(SymbolKind kind)
{
	return kind == SymbolKind::GlobalDeclaration || kind == SymbolKind::GlobalDefinition;
}

struct Symbol
{
	NameId name;
	SymbolKind kind;
	// Index into Program::globals or Program::functions.
	uint32_t detail;
	SymbolMetadata metadata;
};

// GlobalInit and DataItemKind are lowir_model.h's.
struct DataItem
{
	DataItemKind kind;
	Type type;
	Constant constant;
	SymbolId address;
	// The address addend or the byte count of `zero`.
	int64_t amount;
};

struct Global
{
	// A scalar global's type; a structured global's storage type when it
	// states one (an object type: its minimum alignment and span).
	bool typed;
	Type type;
	GlobalInit init;
	Constant constant;
	SymbolId address;
	int64_t addend;
	uint32_t first_item;
	uint32_t item_count;
};

struct Function
{
	SignatureId signature;
	DebugLocation location;
	std::unique_ptr<FunctionBody> body; // null for a declaration
};

struct ObjectAlias
{
	NameId object;
	SymbolId target;
};

// ---------------------------------------------------------------------------
// Program

// One LowIR unit. Symbols are created on first reference, so a use may
// precede the declaration or definition (in a later input file, for
// example); `order` records declarations and definitions as they are made,
// the presentation order within each top-level phase.
// A jump thunk (Program::jump_thunks): `this` (parameter `this_index`)
// moved by `adjustment`, then, when `vcall_offset` is nonzero, by the
// value at that offset from the address point of the table it then
// points to (Itanium 2.5.3's virtual adjustment), and a jump to `target`.
struct JumpThunk
{
	SymbolId symbol;
	SymbolId target;
	uint32_t this_index;
	int64_t adjustment;
	int64_t vcall_offset;
};

struct Program
{
	Program();

	Program(const Program&) = delete;
	Program& operator=(const Program&) = delete;

	// The symbol named `name`, created Unresolved if new.
	SymbolId symbol_for(NameId name);
	// kNoId if no symbol has the name.
	SymbolId find_symbol(NameId name) const { return name < symbol_by_name.size() ? symbol_by_name[name] : kNoId; }

	// Give an Unresolved symbol its declaration or definition. The caller
	// checks that the symbol is Unresolved.
	void declare_global(SymbolId symbol, SymbolKind kind, const Global& global);
	void declare_function(SymbolId symbol, SymbolKind kind, SignatureId signature);

	SignatureId add_signature(const Signature& signature);
	const Parameter* parameters_of(const Signature& signature) const
	{
		return parameters.data() + signature.first_parameter;
	}

	const char* spelling(NameId name) const { return names.spelling(name); }

	// The boundary a call crosses: its explicit signature, else a direct
	// callee's (an Unresolved callee's forward boundary, if a producer
	// stated one); null for an indirect call without a signature.
	const Signature* call_boundary(const Instruction& call) const;
	// The boundary of a function referenced before it is declared or
	// defined (a producer streaming definitions to a back end states it:
	// the declaration or definition that follows has the same).
	void set_forward_boundary(SymbolId symbol, SignatureId signature);
	SignatureId forward_boundary(SymbolId symbol) const
	{
		return symbol < forward_boundaries.size() ? forward_boundaries[symbol] : kNoId;
	}
	// The binding and storage of the declaration that follows for a global
	// referenced before it is declared or defined, as a streaming producer
	// states them (the unit's definition, if one follows, supersedes them),
	// and whether the unit defines it; SBM_DEFAULT and GSM_DEFAULT when none
	// were stated.
	void set_forward_global(SymbolId symbol, uint8_t binding, uint8_t storage, bool defined);
	uint8_t forward_binding(SymbolId symbol) const;
	uint8_t forward_storage(SymbolId symbol) const;
	bool forward_defined(SymbolId symbol) const;
	// True if control does not continue past `inst` to the next block: a
	// terminator, or a call whose boundary is return=noreturn. Such a call
	// may end a block, but unlike a terminator it may also be followed by
	// (unreachable) instructions.
	bool may_end_block(const Instruction& inst) const;
	// Whether `symbol` computes strlen (C 7.24.6.3, C++11 21.7): the
	// compiler's builtin (object=cppgm_builtin_strlen) or the C library's
	// strlen (a reserved name, C++11 17.6.4.3.2). Callers check the call's
	// shape.
	bool strlen_function(SymbolId symbol) const;
	// Whether `symbol` is the compiler's builtin memcpy
	// (object=cppgm_builtin_memcpy).
	bool builtin_memcpy(SymbolId symbol) const;
	// Whether `symbol` is memset or memcpy (C 7.24.6.1, 7.24.2.1): the
	// compiler's builtin or the C library's (reserved names).
	bool memset_function(SymbolId symbol) const;
	bool memcpy_function(SymbolId symbol) const;
	bool memcmp_function(SymbolId symbol) const; // C 7.24.4.1
	// Whether `symbol` is the runtime's trap (`object=cppgm_builtin_trap`,
	// __builtin_trap), which the back end encodes as a trap instruction.
	bool trap_function(SymbolId symbol) const;

	Arena arena;
	NameTable names;
	std::vector<Symbol> symbols;
	std::vector<SymbolId> symbol_by_name;
	std::vector<SymbolId> order;
	std::vector<Global> globals;
	std::vector<DataItem> data_items;
	std::vector<Function> functions;
	std::vector<Signature> signatures;
	std::vector<Parameter> parameters;
	std::vector<ObjectAlias> aliases;
	std::vector<SignatureId> forward_boundaries;
	std::vector<uint8_t> forward_globals;
	// A host-object program (the lowering's object mode, or text the
	// toolchain reads): its text states the facts the object back end reads
	// beyond the course's LowIR (x87 and SSEUP classes, classes of other
	// types and of arguments to `...`, repeated data, NaN payloads), so an
	// object compiled from it is the one compiled from the program; a
	// reader of course LowIR rejects them.
	bool object_facts = false;
	// Object mode, not in the LowIR text: thunks the object back end emits
	// as `this` adjusted in its register and a jump to the target, so the
	// arguments past a variadic function's fixed ones stay where the caller
	// put them (Itanium 5.1.4.2's thunks as GCC emits them). The body each
	// still has (a forwarding call) states the same adjustment for any
	// other reader of the program.
	std::vector<JumpThunk> jump_thunks;
};

// A consumer of function definitions as a producer completes them (in
// order); the producer releases each body after the append. A consumer
// that selects code (`needs_boundaries`) is given every direct callee's
// boundary first (Program::set_forward_boundary).
class DefinitionSink
{
public:
	virtual ~DefinitionSink() {}
	virtual void append(const Program& program, SymbolId function) = 0;
	// The definitions' final order, by append index, if not that of the
	// appends (the LowIR view's presentation order; an object's layout keeps
	// its appends' order and is never asked).
	virtual void reorder(const std::vector<uint32_t>& appends) { (void)appends; }
	virtual bool needs_boundaries() const { return false; }
	// Whether it reads Program::jump_thunks (an object back end): a jump
	// thunk then needs no body; another consumer is given the body.
	virtual bool reads_jump_thunks() const { return false; }
};

} // namespace lowir
