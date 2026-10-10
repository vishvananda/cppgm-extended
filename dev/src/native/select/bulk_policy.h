#pragma once

#include <cstdint>

// The fixed-size bulk-memory forms shared by the selector (which must know
// the registers an operation clobbers) and the emitter (which encodes it).
namespace native {

// copy_bytes through 32 bytes, or through 64 bytes with at least
// eight-byte alignment, uses reserved-scratch vector and scalar chunks;
// larger or more weakly aligned copies use `rep movsb` (rdi, rsi, rcx).
// At O1+ (`wide`: Function::wide_bulk) every copy through 128 bytes is
// direct: unaligned 16-byte chunks start no microcoded string operation.
inline bool direct_copy(uint64_t bytes, uint64_t align, bool wide)
{
	return bytes <= 32 || (bytes <= 64 && align >= 8) || (wide && bytes <= 128);
}

// zero_bytes of these sizes is always smaller as direct stores (one
// scalar store, or one cleared vector store for 16 bytes) than the
// `rep stosb` setup; other sizes compare exact encodings in the emitter
// and may clobber rdi, rcx and rax. At O1+ every size through 128 bytes
// is stored directly (a cleared xmm15 in 16-byte chunks, then scalars).
inline bool always_direct_zero(uint64_t bytes, bool wide)
{
	return bytes == 1 || bytes == 2 || bytes == 4 || bytes == 8 || bytes == 16 || (wide && bytes <= 128);
}

} // namespace native
