#pragma once

#include <cstdint>
#include <memory>
#include <vector>

#include "lowir/program.h"
#include "native/mir/mir.h"

// The x86-64 System V classification of a LowIR call boundary, shared by
// callers (argument setup) and callees (incoming parameter locations).
// LowIR object types carry no field types: a direct `obj<N x A>` of at
// most 16 bytes takes the psABI classes (3.2.3) an object-mode lowering
// states for host objects (lowir::ObjectClassBits: each eightbyte in the
// next GPR or XMM register, in order, or the whole object in memory when
// they do not all fit); without them every eightbyte is INTEGER (the
// course's boundary). Larger objects, f80 and i128 values without two
// free GPRs are passed in memory.
namespace native {

enum class ArgClass : uint8_t
{
	None,    // void (or an empty class: no register, no stack)
	Gpr,     // one or two GPR eightbytes
	Xmm,     // f32/f64
	Memory,  // stack (or, for a result, hidden destination pointer)
	X87,     // f80 result (or a long double object's) in st0
	X87Pair, // a complex long double result: real part in st0, imaginary in st1
	Mixed    // a classified object's eightbytes in XMM (and GPR) registers
};

// One eightbyte (or a whole stack value) of an argument.
struct ArgPart
{
	Reg reg;         // kNoReg: on the stack at `stack`
	int32_t stack;   // offset in the outgoing area / from rbp+16
	uint32_t offset; // byte offset of this part within the value
	uint32_t size;   // bytes (a stack part may be a whole object)
};

struct ArgLayout
{
	ArgClass cls;
	uint32_t first_part;
	uint32_t part_count;
};

struct CallLayout
{
	bool sret = false; // hidden destination pointer in rdi
	std::vector<ArgLayout> args;
	std::vector<ArgPart> parts;
	uint32_t stack_bytes = 0;  // outgoing stack area, unaligned
	uint32_t stack_align = 16; // the area's alignment: 16, or a stack argument's beyond
	uint32_t gprs = 0;         // GPRs used (including sret)
	uint32_t xmms = 0;
	ArgClass result = ArgClass::None;
	uint32_t result_parts = 0; // register result eightbytes (Gpr: 1 or 2)
	ArgPart result_part[2];    // Gpr and Mixed: their registers, in order
};

// Layouts by call instruction for one function at a time, each computed
// once (the selector's prepass, allocator hints, frame check and emission
// share a call's layout for its emission-time argument types); the layouts
// keep their capacity from one function to the next. `scratch` holds a
// layout for other argument types.
struct CallLayoutCache
{
	std::vector<uint32_t> entry; // per instruction: its layout this function, or kNoId
	std::vector<std::unique_ptr<CallLayout>> layouts;
	uint32_t used = 0;
	CallLayout scratch;

	void reset(size_t instructions)
	{
		entry.assign(instructions, lowir::kNoId);
		used = 0;
	}
	const CallLayout* known(uint32_t inst) const
	{
		return entry[inst] != lowir::kNoId ? layouts[entry[inst]].get() : nullptr;
	}
	// A new layout for `inst` (not yet known), to be filled.
	CallLayout& add(uint32_t inst)
	{
		if (used == layouts.size())
			layouts.emplace_back(new CallLayout());
		entry[inst] = used;
		return *layouts[used++];
	}
};

// The position of an integer argument register in kArgGprs.
inline unsigned arg_gpr_slot(Reg reg)
{
	unsigned k = 0;
	while (k < 5 && kArgGprs[k] != reg)
		++k;
	return k;
}

// The width an object's register part moves at: an XMM eightbyte as f32
// (a lone float or binary16) or f64, an SSE and SSEUP pair (binary128) as
// all 128 bits (i128), a GPR eightbyte as i64 (homes are padded).
inline lowir::TypeKind part_type(const ArgPart& part)
{
	if (!is_xmm(part.reg))
		return lowir::TypeKind::I64;
	if (part.size > 8)
		return lowir::TypeKind::I128;
	return part.size <= 4 ? lowir::TypeKind::F32 : lowir::TypeKind::F64;
}

// Size in bytes of a LowIR value of type `type` in memory (f80 and i128
// occupy 16 bytes); alignment likewise.
uint32_t value_bytes(lowir::Type type);
uint32_t value_align(lowir::Type type);

// The types call `inst` passes its arguments at under boundary `sig`: a
// named parameter's type; past them (`...`) the argument's own, a long
// double literal f80, another floating literal f64, an integer literal
// i64, a slot its object, a symbol a pointer.
void call_argument_types(const lowir::FunctionBody& body, const lowir::Instruction& inst, const lowir::Signature& sig,
	const lowir::Parameter* params, std::vector<lowir::Type>& out);

// Classifies a boundary with argument types `params` (count n: the
// boundary's parameters, then any variadic arguments) and result type;
// `classes` (null: none stated) and `result_classes` are their
// lowir::ObjectClassBits.
void classify_call(const lowir::Type* params, const uint8_t* classes, uint32_t count, lowir::Type result,
	uint8_t result_classes, CallLayout& layout);

} // namespace native
