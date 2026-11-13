#pragma once

#include <stdint.h>

// Shared LowIR records; lowir_interface.h adds batched construction. The C projection
// uses fixed-width codes; C++ uses the vocabulary's typed enums and Type.
// The records are common to producers, the optimizer and the backends.
// Containers, allocation, name storage and analyses remain implementation work.
#ifdef __cplusplus
#include <cstring>
#include "lowir_model.h"
#include "lowir_local_model.h"
namespace lowir_model {
typedef uint32_t SymbolId;
typedef uint32_t SignatureId;
typedef uint32_t TempId;
typedef uint32_t SlotId;
typedef uint32_t BlockId;
typedef uint32_t InstId;
const uint32_t kNoId = UINT32_MAX;
#define LOWIR_RECORD(name) name
#define LOWIR_ENUM(name) name
#define LOWIR_TYPE Type
#else
#define LOWIR_RECORD(name) Lowir##name
#define LOWIR_ENUM(name) uint8_t
#define LOWIR_TYPE uint64_t
#endif

typedef struct LOWIR_RECORD(Literal)
{
	uint64_t low;
	uint64_t high;
} LOWIR_RECORD(Literal);

typedef struct LOWIR_RECORD(Value)
{
	LOWIR_ENUM(ValueKind) kind;
	// The kind of a Float literal: F32, F64 or F80.
	LOWIR_ENUM(TypeKind) float_kind;
#ifndef __cplusplus
	// Zero. In C++ these two bytes are padding, so GCC keeps a Value in one
	// 8-byte word; batched construction checks that a producer wrote zero.
	uint16_t reserved;
#endif
	uint32_t index;

#ifdef __cplusplus
	static Value none() { return make(ValueKind::None, 0); }
	static Value temp(TempId id) { return make(ValueKind::Temp, id); }
	static Value slot(SlotId id) { return make(ValueKind::Slot, id); }
	static Value symbol(SymbolId id) { return make(ValueKind::Symbol, id); }
	static Value block(BlockId id) { return make(ValueKind::Block, id); }

	bool is(ValueKind k) const { return kind == k; }
	bool is_literal() const { return kind == ValueKind::Integer || kind == ValueKind::Float; }

	bool operator==(const Value& other) const
	{
		return kind == other.kind && float_kind == other.float_kind && index == other.index;
	}

	static Value make(ValueKind kind, uint32_t index)
	{
		Value value;
		value.kind = kind;
		value.float_kind = TypeKind::Void;
		value.index = index;
		return value;
	}
#endif
} LOWIR_RECORD(Value);

typedef struct LOWIR_RECORD(Instruction)
{
	LOWIR_ENUM(Opcode) op;
	uint8_t sub;
	uint8_t flags;
	// Opcode-local property byte: atomics use MemoryOrder, Index uses
	// IndexBounds through the accessors below. The record stays 64 bytes.
	LOWIR_ENUM(MemoryOrder) order;
	uint32_t result;
	uint32_t next;
	uint32_t location;
	LOWIR_TYPE type;
	LOWIR_RECORD(Value) ops[3];
	uint32_t extra_first;
	uint32_t extra_count;
	int64_t immediate;

#ifdef __cplusplus
	IndexBounds index_bounds() const { return static_cast<IndexBounds>(order); }
	void set_index_bounds(IndexBounds bounds) { order = static_cast<MemoryOrder>(bounds); }
#endif
} LOWIR_RECORD(Instruction);

typedef struct LOWIR_RECORD(Slot)
{
	uint32_t name;
	LOWIR_ENUM(SlotKind) kind;
	uint8_t reserved;
	uint16_t index;
	LOWIR_TYPE type;
} LOWIR_RECORD(Slot);

typedef struct LOWIR_RECORD(BodyCounts)
{
	uint32_t instructions, operands, literals, slots, blocks;
} LOWIR_RECORD(BodyCounts);

typedef struct LOWIR_RECORD(BodyBuffers)
{
	LOWIR_RECORD(Instruction) * instructions;
	LOWIR_RECORD(Value) * operands;
	LOWIR_RECORD(Literal) * literals;
	LOWIR_RECORD(Slot) * slots;
	uint32_t* layout;
	LOWIR_RECORD(BodyCounts) capacity;
} LOWIR_RECORD(BodyBuffers);

#undef LOWIR_RECORD
#undef LOWIR_ENUM
#undef LOWIR_TYPE

#ifdef __cplusplus
inline Type convert_source_type(const Instruction& inst)
{
	Type type;
	std::memcpy(&type, &inst.immediate, sizeof type);
	return type;
}

inline void set_convert_source_type(Instruction& inst, Type type)
{
	std::memcpy(&inst.immediate, &type, sizeof type);
}

// The type of an instruction's result temporary: a comparison's truth
// value is i64, an address is ptr, otherwise the instruction's type.
inline Type instruction_result_type(const Instruction& inst, Type pointer_type)
{
	switch (inst.op)
	{
	case Opcode::Compare:
	case Opcode::AtomicCompareExchange:
	case Opcode::ExceptionSelector:
		return Type::scalar(TypeKind::I64);
	case Opcode::Addr:
	case Opcode::Index:
	case Opcode::StackAlloc:
	case Opcode::StackSave:
		return pointer_type;
	default:
		return inst.type;
	}
}

// The parameter's pass, alias, classes (object mode) and object_bytes
// metadata are the scaffold's record.
struct Parameter : ir_model::ParameterMetadata
{
	uint32_t name;
	Type type;
};

// A boundary's arity, effects, unwind, return and query metadata, and the
// classes of an object result (object mode).
typedef ir_model::FunctionBoundaryMetadata BoundaryMetadata;

// A function or call boundary: parameters [first_parameter,
// first_parameter + parameter_count) of Program::parameters.
struct Signature
{
	uint32_t first_parameter;
	uint32_t parameter_count;
	Type result;
	BoundaryMetadata boundary;
};

static_assert(sizeof(Value) == 8 && sizeof(Instruction) == 64 && sizeof(Literal) == 16, "LowIR records");
} // namespace lowir_model

typedef lowir_model::Literal LowirLiteral;
typedef lowir_model::Value LowirValue;
typedef lowir_model::Instruction LowirInstruction;
typedef lowir_model::Slot LowirSlot;
typedef lowir_model::BodyCounts LowirBodyCounts;
typedef lowir_model::BodyBuffers LowirBodyBuffers;
#endif
