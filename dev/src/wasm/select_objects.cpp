#include "wasm/selector.h"

namespace wasm {

using namespace lowir;

namespace {

// A copy or a fill of at most this many bytes is loads and stores: the engine
// checks the bounds of each memory.copy and memory.fill, and it checks no load
// or store.
constexpr uint64_t kInlineBytes = 64;

struct Piece
{
	Op load;
	Op store;
	uint32_t bytes;
};

Piece piece_of(uint64_t bytes)
{
	if (bytes >= 8)
		return Piece{ Op::I64Load, Op::I64Store, 8 };
	if (bytes >= 4)
		return Piece{ Op::I32Load, Op::I32Store, 4 };
	if (bytes >= 2)
		return Piece{ Op::I32Load16U, Op::I32Store16, 2 };
	return Piece{ Op::I32Load8U, Op::I32Store8, 1 };
}

} // namespace

void Selector::copy_bytes(const Place& destination, const Place& source, uint64_t bytes)
{
	// One object in the storage of another: the copy between them is no code.
	if (destination.local != kNoId && destination.local == source.local && destination.offset == source.offset)
		return;
	if (bytes == 0 || bytes > kInlineBytes || bytes > UINT32_MAX - destination.offset ||
		bytes > UINT32_MAX - source.offset)
	{
		place_address(destination);
		place_address(source);
		out_.emit(Op::I32Const, { bytes });
		out_.emit(Op::MemoryCopy, { 0, 0 });
		return;
	}
	// Every load precedes every store: the two ranges may overlap.
	Piece pieces[kInlineBytes / 8 + 3];
	uint32_t offsets[kInlineBytes / 8 + 3];
	unsigned count = 0;
	for (uint64_t at = 0; at < bytes; at += pieces[count++].bytes)
	{
		pieces[count] = piece_of(bytes - at);
		offsets[count] = static_cast<uint32_t>(at);
		place_base(destination);
		place_base(source);
		out_.emit(pieces[count].load, { 0, source.offset + at });
	}
	while (count-- != 0)
		out_.emit(pieces[count].store, { 0, destination.offset + offsets[count] });
}

void Selector::zero_bytes(const Place& destination, uint64_t bytes)
{
	if (bytes == 0 || bytes > kInlineBytes || bytes > UINT32_MAX - destination.offset)
	{
		place_address(destination);
		out_.emit(Op::I32Const, { 0 });
		out_.emit(Op::I32Const, { bytes });
		out_.emit(Op::MemoryFill, { 0 });
		return;
	}
	for (uint64_t at = 0; at < bytes;)
	{
		const Piece piece = piece_of(bytes - at);
		place_base(destination);
		out_.emit(piece.bytes == 8 ? Op::I64Const : Op::I32Const, { 0 });
		out_.emit(piece.store, { 0, destination.offset + at });
		at += piece.bytes;
	}
}

void Selector::copy_object(Value source, Value destination, uint64_t bytes)
{
	copy_bytes(place_of(destination), place_of(source), bytes);
}

bool Selector::object_instruction(const lowir::Instruction& inst)
{
	if (!memory_value(inst.type))
		return false;
	switch (inst.op)
	{
	case Opcode::Load:
	case Opcode::Copy:
		if (inst.type.kind() == TypeKind::I128 && inst.op == Opcode::Copy)
			return false;
		copy_object(inst.ops[0], Value::temp(inst.result), storage_bytes(inst.type));
		return true;
	case Opcode::Store:
		if (inst.type.kind() == TypeKind::I128)
		{
			wide_part(inst.ops[0], 0);
			wide_part(inst.ops[0], 1);
			store_wide(inst.ops[1]);
			return true;
		}
		copy_object(inst.ops[0], inst.ops[1], storage_bytes(inst.type));
		return true;
	default:
		return false;
	}
}

} // namespace wasm
