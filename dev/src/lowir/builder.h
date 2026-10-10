#pragma once

#include "lowir/program.h"

namespace lowir {

// An instruction with no operands, result or location.
Instruction make_instruction(Opcode op, Type type);

// The bits of a 64-bit integer literal, sign extended to 128 bits.
Literal integer_literal(int64_t value);

// Appends to one function definition's body. The body's parameters are its
// first temporaries, created from the signature when the builder is made.
// Blocks are created, then placed in layout order; instructions go to the
// end of the insertion block. The builder records what it is given: the
// reader checks external input, and validate_constructed_program
// (lowir/validator.h) checks a constructed program in audit builds.
class FunctionBuilder
{
public:
	FunctionBuilder(Program& program, SymbolId function);

	Program& program() { return program_; }
	FunctionBody& body() { return body_; }
	TempId parameter(uint32_t index) const { return index; }

	// A slot or block named `name`, or unnamed with a presentation kind
	// (lowir/local_kinds.h) and, for a slot, its kind's position or source
	// name.
	SlotId add_slot(NameId name, Type type, SlotKind kind = SlotKind::Plain, uint16_t index = 0);
	BlockId add_block(NameId name, BlockKind kind = BlockKind::Plain);
	void place_block(BlockId block) { body_.layout.push_back(block); }
	void set_insertion_block(BlockId block) { insertion_ = block; }
	BlockId insertion_block() const { return insertion_; }

	// A temporary with no definition yet (a forward reference).
	TempId add_temp(NameId name);
	// Records `inst` as the definition of `temp`, with the given type.
	void bind(TempId temp, Type type, InstId inst);

	Value literal(ValueKind kind, TypeKind float_kind, Literal bits);
	Value integer(int64_t value) { return literal(ValueKind::Integer, TypeKind::Void, integer_literal(value)); }

	// Stores `count` operands in the body's operand pool; returns the first.
	uint32_t add_operands(const Value* values, uint32_t count);

	// Links `inst` at the end of the insertion block.
	InstId append(const Instruction& inst);
	// Links `inst` into `block` after `after` (kNoId: at its start).
	InstId insert_after(BlockId block, InstId after, const Instruction& inst);
	// The last instruction of `block` (kNoId while it is empty).
	InstId last_instruction(BlockId block) const { return body_.blocks[block].last; }
	// Appends `inst` and defines a new temporary named `name` as its result.
	TempId append_value(Instruction inst, Type result_type, NameId name);

	TempId binary(BinaryOp op, Type type, Value lhs, Value rhs, NameId name);
	TempId load(Type type, Value storage, NameId name);
	void store(Type type, Value value, Value storage);
	// A call with an explicit signature (kNoId for a direct call without one).
	TempId call(Type result, Value callee, const Value* args, uint32_t count, SignatureId signature, NameId name,
		uint16_t flags = 0);
	void jump(BlockId target);
	void branch(Value condition, BlockId if_true, BlockId if_false);
	void ret(Type type, Value value);

	void set_location(InstId inst, DebugLocation location);
	// The location the instructions appended or inserted from now on carry
	// unless they have one (an absent one: none).
	void set_current_location(DebugLocation location)
	{
		if (location.file != current_.file || location.line != current_.line || location.column != current_.column)
		{
			current_ = location;
			current_index_ = 0;
		}
	}
	DebugLocation current_location() const { return current_; }

	// Ends construction: releases the body's growth slack, so a finished
	// body holds only what it uses.
	void finish();

private:
	void locate(InstId inst);

	Program& program_;
	FunctionBody& body_;
	BlockId insertion_;
	// The current location, and its entry in the body's locations once an
	// instruction has it (1 + index; 0 before).
	DebugLocation current_;
	uint32_t current_index_;
};

} // namespace lowir
