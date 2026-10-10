#pragma once

#include <cstdint>
#include <vector>

#include "native/mir/mir.h"
#include "native/x86/emitter.h"
#include "support/flat_map.h"

// The host EH tables of one function, as the System V x86-64 psABI's
// unwinder and the Itanium C++ ABI personality (__gxx_personality_v0)
// read them: the call frame instructions of its FDE in .eh_frame, and its
// LSDA in .gcc_except_table. Inputs are the emitter's unwind facts (final
// offsets) and the selector's landings, action chains and type table.
//
// LSDA: LPStart omitted (the function's start), call sites as ULEB128
// offsets, type entries as pc-relative indirect 4-byte fields reaching a
// cell that holds the type information's address (a default-PIE link
// needs no text relocation). Call sites cover the potentially throwing
// calls only (sparse, pa26 README 12): consecutive calls with the same
// landing and action share one range; a call with another landing or
// action, or an unprotected one, ends the range (README 9). Action
// records are shared by chains with a common tail.
namespace native {

// A type-table field of an LSDA (offset from the LSDA's start) and the
// type-information image symbol its cell holds.
struct LsdaTypeField
{
	uint32_t offset;
	uint32_t type;
};

// encode_lsda's working storage, reused from function to function by its
// owner (cleared, not freed): action records (filter, next record + 1),
// their index, per landing its chain's first record and whether a call
// site names it, the records' positions and sizes, and the call-site table
// and the rest being built.
struct LsdaScratch
{
	struct Record
	{
		int32_t filter;
		uint32_t next;
	};
	struct KeyHash
	{
		size_t operator()(uint64_t key) const { return static_cast<size_t>(key); }
	};
	std::vector<Record> records;
	FlatMap<uint64_t, uint32_t, KeyHash> index;
	std::vector<uint32_t> first;
	std::vector<uint8_t> used;
	std::vector<uint32_t> position;
	std::vector<uint32_t> size;
	std::vector<uint8_t> rest;
	std::vector<uint8_t> sites;
};

// The CIE every FDE of an object refers to: version 1, code alignment 1,
// data alignment -8, return address column 16, pc-relative 4-byte FDE
// addresses and the initial rule (CFA = rsp + 8, the return address at
// CFA - 8). With `personality` its augmentation adds the personality
// (pc-relative indirect, 4 bytes) and the LSDA pointer encoding; returns
// the personality field's offset in `out` (0 without).
uint32_t encode_cie(bool personality, std::vector<uint8_t>& out);

// The call frame instructions of a function's FDE: the frame setup
// (push rbp; mov rbp, rsp; callee-saved stores), the stack argument bytes
// pushed at each landing call (GNU_args_size: the unwinder pops them
// before entering a landing) when `lsda`, and the epilogue (remembered
// and restored around a resume terminal that follows it).
void encode_frame_instructions(const UnwindFacts& facts, bool lsda, std::vector<uint8_t>& out);

// An FDE for CIE `cie` (an offset in .eh_frame, where the FDE starts at
// out.size()): returns the offsets of its pc-begin field and, with
// `lsda`, of its LSDA pointer field, for the caller's relocations.
void encode_fde(uint32_t cie, uint32_t code_bytes, bool lsda, const uint8_t* instructions, size_t count,
	std::vector<uint8_t>& out, uint32_t& pc_field, uint32_t& lsda_field);

// Appends the LSDA when some call reaches a landing (else nothing, and
// returns false); its type-table fields are appended to `types`.
bool encode_lsda(const UnwindFacts& facts, const Function& function, std::vector<uint8_t>& out,
	std::vector<LsdaTypeField>& types, LsdaScratch& scratch);

} // namespace native
