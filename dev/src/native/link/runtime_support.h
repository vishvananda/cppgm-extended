#pragma once

#include "native/link/routine_builder.h"

// The runtime library's members built as LowIR (native/link/
// runtime_library.h), one routine set each:
// - allocation: `__cppgm_allocate`/`__cppgm_release` (malloc and free)
//   behind the operator new/delete names: 16-byte headers holding the
//   block size, free lists per 16-byte class up to 1 KiB and per power of
//   two up to 128 KiB carved from 1 MiB mappings, larger blocks mapped and
//   unmapped alone; exhaustion makes malloc return null and operator new
//   terminate; calloc and realloc.
// - exit handlers: `atexit` and `__cxa_atexit` push onto the entry
//   member's list, run most recent first at exit.
// - exceptions (Itanium 2.4 surface over the handler stack): a 64-byte
//   header precedes each exception object (caught-stack and in-flight
//   links, type, destructor, adjusted pointer, handler count, selector).
//   __cxa_throw records the exception as in flight and dispatches; each
//   catch clause of a landing calls `__cppgm_eh_catch` (15.3p3 matching
//   with libsupc++'s pointer qualification rules and an unambiguous public
//   base search; the adjusted object for __cxa_begin_catch); begin_catch
//   takes the exception off the in-flight list, restoring the enclosing
//   exception's payload and selector for the landing still running;
//   end_catch destroys it with its last handler unless rethrown.
// - dynamic cast: `__dynamic_cast` (5.2.7p8) by a walk of the most derived
//   object's base graph from its type information.
// - bad cast: `__cxa_bad_cast`, `__cxa_bad_typeid` and
//   `__cxa_throw_bad_array_new_length` throw std::bad_cast, std::bad_typeid
//   and std::bad_array_new_length objects.
// - wide division: libgcc's __divti3, __udivti3, __modti3 and __umodti3,
//   which the native back end calls for i128 division: one 64-bit divide
//   when both operands fit in 64 bits, else restoring shift-subtract
//   division over the 128 bits.
// - floating conversions: libgcc's __floatti{s,d,x}f, __floatunti{s,d,x}f,
//   __fix{s,d,x}fti and __fixuns{s,d,x}fti, which the native back end calls
//   for i128/floating conversions (4.9, 4.10 as GCC defines them: round to
//   nearest even; truncation, out-of-range values saturating).
namespace native {

enum SupportMember : uint8_t
{
	kAllocation,
	kExitHandlers,
	kExceptions,
	kDynamicCast,
	kBadCast,
	kWideDivision,
	kFloatConversion,
	kSupportMembers
};

// The member defining `name`, or kSupportMembers.
SupportMember support_member_of(const char* name);

void build_support_member(RuntimeProgram& program, SupportMember member);

} // namespace native
