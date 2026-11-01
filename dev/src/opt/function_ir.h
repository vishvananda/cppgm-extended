#pragma once

#include <cstdint>
#include <vector>

#include "lowir/program.h"

// Shared facts and edits of one function body for the optimizer's passes
// (opt/optimizer.h). Blocks, temporaries and instructions keep their dense
// ids; removed instructions are unlinked from their block and compacted
// away once at the end (compact_body), so a pass never renumbers ids.
namespace opt {

// Calls f(Value&) for each operand slot of `inst`: ops[0..2], then its extra
// operands (call arguments, phi (block, value) pairs, switch (case, block)
// pairs). Callers select the kinds they need.
template<typename F>
void for_each_operand(lowir::FunctionBody& body, lowir::Instruction& inst, F f)
{
	for (lowir::Value& value : inst.ops)
		f(value);
	for (uint32_t k = 0; k < inst.extra_count; ++k)
		f(body.operands[inst.extra_first + k]);
}

template<typename F>
void for_each_operand(const lowir::FunctionBody& body, const lowir::Instruction& inst, F f)
{
	for (const lowir::Value& value : inst.ops)
		f(value);
	for (uint32_t k = 0; k < inst.extra_count; ++k)
		f(body.operands[inst.extra_first + k]);
}

// Calls f(BlockId&) for each ordinary successor label of a terminator
// (repeats included).
template<typename F>
void for_each_successor(lowir::FunctionBody& body, lowir::Instruction& term, F f)
{
	using lowir::Opcode;
	if (term.op == Opcode::Jump)
		f(term.ops[0].index);
	else if (term.op == Opcode::Branch)
	{
		f(term.ops[1].index);
		f(term.ops[2].index);
	}
	else if (term.op == Opcode::Switch)
	{
		f(term.ops[1].index);
		for (uint32_t k = 1; k < term.extra_count; k += 2)
			f(body.operands[term.extra_first + k].index);
	}
}

// Whether an instruction has no effect beyond its result: removable when
// unused, and (except Load) reusable for an equal computation.
inline bool is_pure(const lowir::Instruction& inst)
{
	switch (inst.op)
	{
	case lowir::Opcode::Const:
	case lowir::Opcode::Copy:
	case lowir::Opcode::Phi:
	case lowir::Opcode::Addr:
	case lowir::Opcode::Index:
	case lowir::Opcode::Unary:
	case lowir::Opcode::Binary:
	case lowir::Opcode::Compare:
	case lowir::Opcode::Select:
	case lowir::Opcode::Convert:
	case lowir::Opcode::Vector:
		return true;
	default:
		return false;
	}
}

// Whether an instruction may transfer control to an active handler: a call
// whose boundary may unwind, or throw/resume.
bool may_unwind(const lowir::Program& program, const lowir::Instruction& inst);
// eh_* instructions and throw, resume, exception and exception_selector.
inline bool is_eh_instruction(lowir::Opcode op)
{
	switch (op)
	{
	case lowir::Opcode::EhTry:
	case lowir::Opcode::EhCleanup:
	case lowir::Opcode::EhCatch:
	case lowir::Opcode::EhFilter:
	case lowir::Opcode::EhCatchAll:
	case lowir::Opcode::EhEnd:
	case lowir::Opcode::Throw:
	case lowir::Opcode::Resume:
	case lowir::Opcode::Exception:
	case lowir::Opcode::ExceptionSelector:
		return true;
	default:
		return false;
	}
}

// Whether a call may write memory visible to other code (its boundary's
// effects).
bool call_may_write_memory(const lowir::Program& program, const lowir::Instruction& call);
// Whether an instruction may write memory visible to other code.
inline bool may_write_memory(const lowir::Program& program, const lowir::Instruction& inst)
{
	switch (inst.op)
	{
	case lowir::Opcode::Store:
	case lowir::Opcode::AtomicStore:
	case lowir::Opcode::AtomicAddFetch:
	case lowir::Opcode::AtomicExchange:
	case lowir::Opcode::AtomicCompareExchange:
	case lowir::Opcode::AtomicThreadFence:
	case lowir::Opcode::AtomicSignalFence:
	case lowir::Opcode::AtomicLoad:
	case lowir::Opcode::VaStart:
	case lowir::Opcode::VaArg:
	case lowir::Opcode::CopyObject:
	case lowir::Opcode::ZeroInit:
	case lowir::Opcode::Throw:
	case lowir::Opcode::Resume:
		return true;
	case lowir::Opcode::Call:
		return call_may_write_memory(program, inst);
	default:
		return is_eh_instruction(inst.op);
	}
}

// The byte size a value of `type` occupies in memory (index's element
// size).
inline uint64_t storage_bytes(lowir::Type type)
{
	switch (type.kind())
	{
	case lowir::TypeKind::Void:
		return 0;
	case lowir::TypeKind::I1:
	case lowir::TypeKind::I8:
	case lowir::TypeKind::U8:
		return 1;
	case lowir::TypeKind::I16:
	case lowir::TypeKind::U16:
		return 2;
	case lowir::TypeKind::I32:
	case lowir::TypeKind::U32:
	case lowir::TypeKind::F32:
		return 4;
	case lowir::TypeKind::I64:
	case lowir::TypeKind::F64:
	case lowir::TypeKind::Ptr:
		return 8;
	case lowir::TypeKind::I128:
	case lowir::TypeKind::F80:
	case lowir::TypeKind::V128:
		return 16;
	case lowir::TypeKind::Obj:
		return type.bytes;
	}
	return 0;
}

enum BlockFlag : uint8_t
{
	BF_PLACED = 1,
	BF_REACHABLE = 2,
	BF_HANDLER = 4, // the target of an eh_try or eh_cleanup
	BF_EH = 8       // contains an eh_* instruction or throw/resume
};

// Children of each block in the dominator tree (CSR), in RPO order.
struct DomChildren
{
	std::vector<uint32_t> first, children;
};

// Control flow of one body, indexed by BlockId: distinct ordinary
// successors and predecessors (from placed blocks), handler edges from the
// block holding eh_try/eh_cleanup, reverse postorder over both kinds of
// edge from the entry, and the dominator tree (Cooper, Harvey and Kennedy)
// with preorder intervals for O(1) dominance queries. O(blocks + edges).
struct Cfg
{
	uint32_t block_count = 0;
	std::vector<uint32_t> succ_first, succs;
	std::vector<uint32_t> pred_first, preds;
	std::vector<uint32_t> handler_first, handlers;
	std::vector<uint8_t> flags;
	std::vector<lowir::BlockId> rpo;
	std::vector<uint32_t> rpo_index;  // kNoId when unreachable
	std::vector<lowir::BlockId> idom; // kNoId for the entry and unreachable blocks
	std::vector<uint32_t> dom_in, dom_out;
	DomChildren tree; // the dominator tree (with dom_in and dom_out)
	// The builders' scratch (contents meaningless between builds).
	std::vector<uint32_t> scratch;
	std::vector<std::pair<lowir::BlockId, uint32_t>> scratch_stack;

	uint32_t succ_count(lowir::BlockId b) const { return succ_first[b + 1] - succ_first[b]; }
	uint32_t pred_count(lowir::BlockId b) const { return pred_first[b + 1] - pred_first[b]; }
	bool reachable(lowir::BlockId b) const { return (flags[b] & BF_REACHABLE) != 0; }
	// Whether a dominates b (reflexive); both reachable.
	bool dominates(lowir::BlockId a, lowir::BlockId b) const
	{
		return dom_in[a] <= dom_in[b] && dom_out[b] <= dom_out[a];
	}
};

void build_cfg(const lowir::Program& program, const lowir::FunctionBody& body, Cfg& cfg);
// The edges, flags and reverse postorder alone (no dominators: idom,
// dom_in and dom_out stay as they were), and the rest of build_cfg for a
// CFG built so.
void build_cfg_light(const lowir::Program& program, const lowir::FunctionBody& body, Cfg& cfg);
void complete_cfg(Cfg& cfg);

void build_dom_children(const Cfg& cfg, DomChildren& out);

// The block of each instruction (kNoId for unlinked ones), from layout.
void map_instruction_blocks(const lowir::FunctionBody& body, std::vector<lowir::BlockId>& block_of);

// Use counts of every temporary over linked instructions.
void count_uses(const lowir::FunctionBody& body, std::vector<uint32_t>& uses);

// New instructions, temporaries and literals (appended; ids stay dense).
lowir::InstId append_instruction(lowir::FunctionBody& body, const lowir::Instruction& inst);
// The stated argument classes (FunctionBody::argument_classes) of `from`'s
// call or va_arg `source`, appended to `body`'s for its copy `copy` (the
// newest instruction: the table stays in instruction order). `from` may
// be `body`.
void copy_argument_classes(
	lowir::FunctionBody& body, const lowir::FunctionBody& from, lowir::InstId source, lowir::InstId copy);
// A copy `inst` (its operands already mapped) of instruction `source` of
// `from` (may be `body`), appended to `body` with its argument classes.
// Every pass that clones instructions appends them so.
inline lowir::InstId append_copy(
	lowir::FunctionBody& body, const lowir::FunctionBody& from, lowir::InstId source, const lowir::Instruction& inst)
{
	const bool classes = !from.argument_classes.empty() &&
		(inst.op == lowir::Opcode::Call || inst.op == lowir::Opcode::VaArg);
	const lowir::InstId id = append_instruction(body, inst);
	if (classes)
		copy_argument_classes(body, from, source, id);
	return id;
}
lowir::TempId append_temp(lowir::FunctionBody& body, lowir::Type type, lowir::InstId def);
lowir::Value integer_value(lowir::FunctionBody& body, lowir::Literal bits);
lowir::Value literal_value(
	lowir::FunctionBody& body, lowir::ValueKind kind, lowir::TypeKind float_kind, lowir::Literal bits);
// A new unnamed block of presentation kind `kind` (lowir/local_kinds.h).
lowir::BlockId append_block(lowir::FunctionBody& body, lowir::BlockKind kind);
// A copy of `count` operands at the end of body.operands; returns the first.
uint32_t append_operands(lowir::FunctionBody& body, const lowir::Value* values, uint32_t count);

// Inserts `inst` (linked nowhere) after `after` in `block` (at the start
// when `after` is kNoId).
void insert_after(lowir::FunctionBody& body, lowir::BlockId block, lowir::InstId after, lowir::InstId inst);
// Unlinks `inst` from `block`, given its predecessor in the list.
void unlink_after(lowir::FunctionBody& body, lowir::BlockId block, lowir::InstId prev, lowir::InstId inst);
// The instruction before `inst` in its block (linear in the block).
lowir::InstId previous_in_block(const lowir::FunctionBody& body, lowir::BlockId block, lowir::InstId inst);

// A value as a phi input of `type`: an integer literal in a floating phi
// becomes the floating literal of its value (phis take no integer literal
// of a floating type).
lowir::Value phi_input(lowir::FunctionBody& body, lowir::Type type, lowir::Value value);

// Removes the (block, value) pair for `pred` from every phi of `block`.
void remove_phi_incoming(lowir::FunctionBody& body, lowir::BlockId block, lowir::BlockId pred);
// Renames predecessor `from` to `to` in every phi of `block`.
void rename_phi_incoming(lowir::FunctionBody& body, lowir::BlockId block, lowir::BlockId from, lowir::BlockId to);
// The incoming value of `pred` in a phi, or a None value.
lowir::Value phi_incoming(const lowir::FunctionBody& body, const lowir::Instruction& phi, lowir::BlockId pred);

// Replaces every use of temporary `from` with `to` in linked instructions
// (a full scan: for occasional rewrites only).
void replace_all_uses(lowir::FunctionBody& body, lowir::TempId from, lowir::Value to);

// Drops unlinked instructions, unused temporaries' definitions stay; makes
// instruction, operand, literal and location storage dense, and keeps the
// explicit names of temporaries, slots and blocks distinct (the writer
// gives the others names that print uniquely: lowir/local_names.h).
void compact_body(lowir::Program& program, lowir::FunctionBody& body);

// Instructions in placed blocks.
uint64_t instruction_count(const lowir::FunctionBody& body);

// The integer `type` value stored at `offset` bytes into a readonly,
// non-thread-local global definition no other object can replace (its
// data items laid out at their natural alignment, as the back end places
// them): a scalar item of that exact place and width, or zero bytes.
// False for address items, partial overlaps and other globals.
bool read_readonly_integer(const lowir::Program& program, lowir::SymbolId symbol, int64_t offset, lowir::Type type,
	lowir::Constant& out, const std::vector<uint8_t>* unwritten = nullptr);

// Per symbol: an internal global definition the unit never writes after
// its static initialization (`unwritten`), so its bytes are its data:
// every use of its address reads through it (loads, copies from it,
// address arithmetic and comparisons, kept in private slots on the way);
// no other global's data holds its address. readonly storage aside, the
// const tables lowering keeps in writable storage (pa10/README.md: only
// const scalars are readonly). O(instructions).
void find_unwritten_globals(const lowir::Program& program, std::vector<uint8_t>& unwritten);

// The address a pointer-sized load at `offset` into constant global
// `symbol` reads: an address item there, its symbol in `target`; with no
// `addend`, only one without an addend, else its addend there too
// (read_readonly_integer's rules otherwise).
bool read_readonly_address(const lowir::Program& program, lowir::SymbolId symbol, int64_t offset,
	const std::vector<uint8_t>* unwritten, lowir::SymbolId& target, int64_t* addend = nullptr);

// Whether `symbol`'s bytes are its data: readonly storage, or unwritten.
bool constant_contents(const lowir::Program& program, lowir::SymbolId symbol, const std::vector<uint8_t>* unwritten);

} // namespace opt
