#pragma once

#include <cstdint>

#include "lowir/program.h"

// Shared vocabulary of the host EH analysis (host_eh.cpp: states;
// host_eh_tables.cpp: chains, type table, selector translation).
namespace native {
namespace host_eh {

// The largest catch selector.
const int64_t kMaxSelector = 1 << 16;

inline bool bare_cleanup(const lowir::Instruction& inst)
{
	return inst.op == lowir::Opcode::EhCleanup && inst.ops[0].kind != lowir::ValueKind::Block;
}

// A landing clause: a catch, a catch-all, a filter or a bare cleanup.
inline bool clause(const lowir::Instruction& inst)
{
	return inst.op == lowir::Opcode::EhCatch || inst.op == lowir::Opcode::EhCatchAll ||
		inst.op == lowir::Opcode::EhFilter || bare_cleanup(inst);
}

// A selector's bit in a selector mask: 0..62 exactly, the rest share bit 63.
inline uint64_t selector_bit(int64_t selector)
{
	return static_cast<uint64_t>(1) << (selector >= 0 && selector <= 62 ? selector : 63);
}

} // namespace host_eh
} // namespace native
