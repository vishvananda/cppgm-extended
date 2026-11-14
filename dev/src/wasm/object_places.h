#pragma once

#include <vector>

#include "lowir/program.h"
#include "wasm/abi.h"

namespace wasm {

// Objects of a function that take the storage of another object, so that the
// copy between the two is no code. On this target an aggregate passes and
// returns through memory: the caller owns a copy of each by-value argument,
// which the callee may write, and the result object is storage that only the
// callee reaches during the call.
struct ObjectPlaces
{
	// Per slot: the by-value parameter whose incoming object is the slot's
	// storage (kNoId: none). The function copies that parameter into the
	// slot before anything else touches the slot, and reads the parameter
	// nowhere else.
	std::vector<uint32_t> parameter_of;
	// The slot whose storage is the caller's result object (kNoId: none).
	uint32_t result = lowir::kNoId;
	// Per slot: nonzero when the slot is the object of one by-value call
	// argument and nothing else reads it. Every write of it is in the call's
	// block and before the call, so the callee takes the slot itself.
	std::vector<uint8_t> argument;
	// Per temporary: the slot that receives the temporary's only use, a
	// complete copy right after the call that makes it (kNoId: none). The
	// call writes its result into that slot.
	std::vector<uint32_t> slot_of;

	bool shared(uint32_t slot) const { return parameter_of[slot] != lowir::kNoId || slot == result || target[slot]; }
	// Per slot: nonzero when a call result goes into the slot.
	std::vector<uint8_t> target;
};

// `eh_sites`: per instruction, nonzero for a call with a handler in this
// function (null: no call has one).
ObjectPlaces plan_object_places(const lowir::Program& program, const lowir::FunctionBody& body,
	const lowir::Signature& signature, const Boundary& boundary, const std::vector<uint32_t>* eh_sites);

} // namespace wasm
