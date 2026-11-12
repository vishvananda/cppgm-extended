#pragma once

#include <cstdint>
#include <vector>
#include "machine/exception_tables.h"

// The exception models of the native backend and the host model's
// per-function tables.
//
// EhModel::Course is the course's handler-stack runtime ABI (eh_push and
// eh_pop records, native/runtime/runtime.h): lowir2native and link mode.
// EhModel::Host is the host's Itanium tables (host objects): a landing pad
// stores the exception object and selector the personality delivers in a
// frame slot pair, each potentially throwing call names its landing
// (Inst::eh), and the emitter records the call-site ranges that the LSDA
// and the unwind table describe (native/object/host_unwind.h).
namespace native {

enum class EhModel : uint8_t
{
	Course,
	Host
};

// A host landing: its pad's MIR block, its selector block's (kNoId when
// the pad stores the delivered filter as the selector), its LSDA action
// chain, a run of EhTables::filters (in matching order: a positive filter
// is a catch type's table index, 0 a cleanup), and its selector
// translation, pairs (filter, selector) in EhTables::map: the selector of
// the first pair with the delivered filter, else 0.
using EhLanding = machine::ExceptionLanding;

// Only the native emitter assigns a frame location to the shared tables.
struct EhTables : machine::ExceptionTables
{
	int32_t slot = 0;
	void clear()
	{
		machine::ExceptionTables::clear();
		slot = 0;
	}
};

} // namespace native
