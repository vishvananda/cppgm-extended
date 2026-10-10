#pragma once

#include <cstdint>

// The handler record an `eh_push` fills (the runtime's handler stack,
// native/runtime/runtime.h): the previous top, the landing address, RBP,
// RSP and the callee-saved GPRs at the push. The saved RBP (16-byte
// aligned by every prologue) carries the record's state in its low bits:
// kEhCleanup for a cleanup record, kEhActive while its landing runs; a try
// record's push stores RBP itself. The dispatcher enters a landing with
// the saved RSP.
//
// At O1 and above a record lives below the static frame
// (Function::eh_records_pushed): the push lowers RSP by kEhRecordBytes and
// fills the record at the new RSP, so the saved RSP is the record itself
// and a landing runs with its record still on the stack; the pop (eh_end,
// or a catch landing's `exception` read taking its record) raises RSP past
// it. Records nest as their regions do, so a frame holds one record per
// open region, not one per push. At O0, and in a function with
// `stack_alloc` (whose allocations last until the epilogue, so no pop may
// raise RSP), each record has its own frame slot instead (Inst::aux2). The
// selector places records, the emitter fills them, the runtime walks them.
namespace native {

const uint32_t kEhRecordBytes = 80;
enum : int32_t
{
	kEhPrev = 0,
	kEhTarget = 8,
	kEhRbp = 16,
	kEhRsp = 24,
	kEhSaved = 32
};
enum : uint8_t
{
	kEhTry = 0,
	kEhCleanup = 1,
	kEhActive = 2,
	kEhStateBits = 3
};

} // namespace native
