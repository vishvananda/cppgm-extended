#pragma once

#include <cstdint>
#include <utility>
#include <vector>

#include "lowir/program.h"
#include "native/mir/mir.h"
#include "native/mir/type_kinds.h"

// The per-function facts the selector computes before emission: value
// classes and placement flags, lazily formed addresses, block facts and
// pending parallel transfers.
namespace native {

// Host EH: a jump that resumes through the function's resume terminal
// (FunctionSelector::eh_sites_).
const uint32_t kResumeTerminal = 0xffffffffu;
// The register allocator's block weights: per execution of the function's
// entry (register_allocation.cpp block_weights).
const uint64_t kWeightScale = 64;

// The fixed register a materialized integer comparison reads an immediate
// from (a clobber of that instruction, so it stays allocatable).
const Reg kCompareScratch = RDX;

// How an integer division or remainder selects (emit_divide.cpp).
enum class DivideForm : uint8_t
{
	Hardware,      // div/idiv (rax, rdx)
	UnsignedShift, // unsigned by 2^k: shift or mask (every level)
	SignedShift,   // signed by +-2^k: biased arithmetic shift (O1+)
	Exact,         // IF_EXACT: shift and multiply by the inverse (O1+)
	UnsignedMagic, // multiply-high by a magic number (rax, rdx; O1+)
	SignedMagic
};

enum class VClass : uint8_t
{
	None,
	Gpr,
	Xmm,
	Memory // f80, i128 and objects: always in frame storage
};

enum TempFlag : uint16_t
{
	TF_GLOBAL = 1 << 0, // used outside its defining block: frame home
	TF_LAZY = 1 << 1,   // address folded into its memory uses
	TF_FOLDED = 1 << 2, // sole-use load folded into its consumer
	TF_FUSED = 1 << 3,  // compare fused into the following branch
	TF_PARAM = 1 << 4,
	TF_DEAD = 1 << 5,      // unused pure value: no code
	TF_ALIAS = 1 << 6,     // stands for another value (copy, const)
	TF_VALUE_USE = 1 << 7, // some use needs the value, not an address
	TF_PINNED = 1 << 8,    // parameter kept in its incoming register
	TF_DEFINED = 1 << 9,   // defined in reachable code (or a parameter)
	TF_HOMED = 1 << 10,    // currently lives only in its home
	TF_STORED = 1 << 11,   // an object call result stored straight into its copy's destination
	TF_TESTED = 1 << 12,   // and/shl folded into its compare-and-branch's test or bt (O1+)
	TF_RMW = 1 << 13       // operation on a folded load its only use stores back there: done in memory (O1+)
};

// A lazily formed address: base (frame slot, symbol, temp register, or
// the frame home of an object-typed value) plus index temp * scale plus
// displacement.
struct AddrExpr
{
	enum Kind : uint8_t
	{
		Absolute,
		Frame,
		Symbol,
		Temp,
		Home
	};
	Kind kind = Absolute;
	uint8_t scale = 1;
	uint32_t base = lowir::kNoId;
	uint32_t index = lowir::kNoId;
	int64_t disp = 0;
};

struct TempInfo
{
	uint32_t block = lowir::kNoId; // layout position of the definition
	int32_t def = -1;              // position in that block (-1: start)
	int32_t last = -1;             // last use in the defining block
	uint32_t uses = 0;
	uint32_t user = lowir::kNoId; // the using instruction when uses == 1
	uint16_t flags = 0;
	VClass cls = VClass::None;
	Reg hint = kNoReg;
	Reg reg = kNoReg;             // current register while emitting
	uint32_t forbid = 0;          // registers its local interval may not use
	Reg greg = kNoReg;            // register of a cross-block value's hull
	int32_t block_last = -1;      // last use in a block it is retained into
	int32_t home = 0;             // frame offset of its home (0: none)
	uint32_t addr = lowir::kNoId; // AddrExpr of an address value
};

struct BlockInfo
{
	lowir::BlockId id = 0;
	bool reachable = false;
	bool handler = false;
	bool has_phi = false;
	uint32_t preds = 0;
	uint32_t count = 0;          // instructions
	uint32_t mir = lowir::kNoId; // MIR block index
};

// A switch lowered as tests (emit_control.cpp): runs of consecutive case
// values per target and at most one bit-test window, in value order;
// with four or more, a decision tree whose subtrees' blocks follow the
// function's (SwitchNode: the tests [first, past) knowing lo <= selector
// <= hi, in MIR block `mir`).
struct SwitchTest
{
	int64_t lo;
	int64_t hi;
	uint32_t target; // MIR block (a window's targets are in its masks)
	bool window;
};

struct SwitchTree
{
	std::vector<SwitchTest> tests;
	std::vector<std::pair<uint64_t, uint32_t>> masks; // the window's: bits from its lo, target
	Reg selector;
	lowir::TypeKind kind;
	bool sign;
	bool tree;
	uint32_t fallback;
	uint32_t location;
};

struct SwitchNode
{
	uint32_t tree;
	uint32_t first;
	uint32_t past;
	int64_t lo;
	int64_t hi;
	uint32_t mir;
};

// A function's switches lowered as tests and their deferred subtrees (in
// MIR block order, from `extra_mir`, the first block after its own).
struct SwitchLowering
{
	std::vector<SwitchTree> trees;
	std::vector<SwitchNode> nodes;
	uint32_t extra_mir = 0;
};

// An edge transfer block between a conditional terminator and a phi block.
struct EdgeBlock
{
	uint32_t from; // layout positions
	uint32_t to;
	uint32_t mir;
};

// A transfer of a value spilled around loops or homed in some blocks
// (loop_split.h) on the edge from -> to (layout positions): into its home
// entering them, back into its register `reg` leaving them.
struct SplitMove
{
	uint32_t from;
	uint32_t to;
	uint32_t temp;
	bool store;
	Reg reg = kNoReg;
};

// The values homed in some blocks (LoopSplitter::plan_homes): per block
// (CSR) those homed there; while emitting, those taken out of their
// registers in the current block, with the registers.
struct SplitHomes
{
	std::vector<uint32_t> first;
	std::vector<uint32_t> temps;
	std::vector<std::pair<uint32_t, Reg>> cleared;
};

enum SlotState : uint8_t
{
	SS_UNUSED,
	SS_NORMAL,
	SS_DEAD,    // only stored to: stores dropped
	SS_PROMOTED // one entry store: loads stand for the stored value
};

enum InstMark : uint8_t
{
	IM_DEAD = 1, // emits nothing
	IM_REACHABLE = 2
};

// The condition code of an integer comparison predicate.
inline Cond int_cond(lowir::ComparePredicate pred)
{
	static const Cond conds[] = { CC_E, CC_NE, CC_L, CC_LE, CC_G, CC_GE, CC_B, CC_BE, CC_A, CC_AE };
	return conds[static_cast<unsigned>(pred)];
}

// A pending parallel transfer (operands.cpp): dst <- src, or dst <- the
// address of the memory operand src when `address` is set. A memory source
// holding a narrower value (the home of a value an elided extension stands
// for) is read at `src_type`, extending, and written at `type`.
struct Move
{
	Operand dst;
	Operand src;
	lowir::TypeKind type = lowir::TypeKind::I64;      // memory width; F32/F64
	lowir::TypeKind src_type = lowir::TypeKind::Void; // Void: `type`
	bool address = false;

	lowir::TypeKind read_type() const { return src_type == lowir::TypeKind::Void ? type : src_type; }
};

} // namespace native
