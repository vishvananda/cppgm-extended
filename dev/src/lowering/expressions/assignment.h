#ifndef CPPGM_LOWERING_EXPRESSIONS_ASSIGNMENT_H
#define CPPGM_LOWERING_EXPRESSIONS_ASSIGNMENT_H

#include "lowering/support/sequences.h"
#include "lowering/support/errors.h"
#include "lowering/ir/model.h"
#include "semantic/model/graph.h"

#include <cstdint>
#include <string>

namespace cppgm
{
namespace lowering
{

using namespace semantic;
using namespace semantic;
using namespace lowering::ir;
using namespace lowering::support;

template <class Derived>
class AssignmentLowering
{
public:
	BindingId BitFieldBinding(std::uint32_t node) const
	{
		const Derived& derived = static_cast<const Derived&>(*this);
		const DumpNode& record = derived.arena_.nodes[node];
		if (record.kind != DUMP_MEMBER_EXPRESSION ||
			record.binding == kNoBinding ||
			record.binding >= derived.program_.bindings.size() ||
			!derived.program_.bindings[record.binding].bit_field)
			return kNoBinding;
		return record.binding;
	}

	std::uint64_t BitFieldMask(const BindingRecord& field) const
	{
		const Derived& derived = static_cast<const Derived&>(*this);
		const BindingLayoutFact& layout = derived.program_.BindingLayout(field);
		if (layout.bit_width == 0 || layout.bit_width > 64)
			ThrowLoweringInternal("invalid canonical bit-field width");
		return layout.bit_width == 64 ? ~std::uint64_t(0) :
			(std::uint64_t(1) << layout.bit_width) - 1;
	}

	LowType BitFieldAccessType(const BindingRecord& field) const
	{
		const Derived& derived = static_cast<const Derived&>(*this);
		const LowType declared = derived.LowerExpressionType(field.type);
		const BindingLayoutFact& layout = derived.program_.BindingLayout(field);
		if (declared.width < 32 ||
			(!declared.is_signed && declared.width == 32 &&
			 layout.bit_width < declared.width))
			return LowI32();
		return declared;
	}

	// A bit-field is read and written as its storage unit, which can be
	// narrower or wider than its declared type (a unit shared by adjacent
	// fields of different types, or narrowed to stay within the bytes its
	// run may touch); the unit takes the declared type's signedness.
	LowType BitFieldMemoryType(const BindingRecord& field) const
	{
		const Derived& derived = static_cast<const Derived&>(*this);
		const LowType storage = derived.LowerExpressionType(field.type);
		const LowType access = BitFieldAccessType(field);
		const std::size_t unit =
			derived.program_.BindingLayout(field).bit_storage_bits;
		if (derived.program_.BindingLayout(field).bit_storage_split != 0)
			return storage.is_signed ? LowI64() : LowU64();
		if (unit == 0 || unit == storage.width)
			return storage.width == access.width ? access : storage;
		if (unit == access.width) return access;
		switch (unit)
		{
		case 8: return storage.is_signed ? LowI8() : LowU8();
		case 16: return storage.is_signed ? LowI16() : LowU16();
		case 32: return storage.is_signed ? LowI32() : LowU32();
		case 64: return storage.is_signed ? LowI64() : LowU64();
		default: ThrowLoweringInternal("unsupported bit-field storage unit");
		}
	}

	// The type an initializing store uses: the declared type where the unit
	// has its width, else the unit.
	LowType BitFieldStoreType(BindingId binding, const LowType& declared) const
	{
		const Derived& derived = static_cast<const Derived&>(*this);
		const BindingRecord& field = derived.program_.bindings[binding];
		return derived.program_.BindingLayout(field).bit_storage_bits ==
			declared.width ? declared : BitFieldMemoryType(field);
	}

	Operand FinishBitFieldValue(BindingId binding, Operand value,
		const LowType& type)
	{
		Derived& derived = static_cast<Derived&>(*this);
		const BindingRecord& field = derived.program_.bindings[binding];
		const BindingLayoutFact& layout = derived.program_.BindingLayout(field);
		Operand normalized = value;
		const LowType declared = derived.LowerExpressionType(field.type);
		if (declared.is_signed && layout.bit_width < type.width)
		{
			const std::size_t shift_count = type.width - layout.bit_width;
			const Operand shifted = derived.Temp(type);
			Instruction shift_left(Instruction::BINARY);
			shift_left.dest = shifted.id;
			shift_left.op = LOW_OP_SHL;
			shift_left.type = type;
			shift_left.first = normalized;
			shift_left.second = Operand(shift_count, type);
			derived.Emit(shift_left);
			normalized = derived.Temp(type);
			Instruction shift_right(Instruction::BINARY);
			shift_right.dest = normalized.id;
			shift_right.op = LOW_OP_SHR;
			shift_right.type = type;
			shift_right.first = shifted;
			shift_right.second = Operand(shift_count, type);
			derived.Emit(shift_right);
		}
		return derived.Convert(normalized, declared, false);
	}

	Operand NormalizeBitFieldValue(BindingId binding, Operand value,
		const LowType& type)
	{
		Derived& derived = static_cast<Derived&>(*this);
		const BindingRecord& field = derived.program_.bindings[binding];
		const Operand masked = derived.Temp(type);
		Instruction mask(Instruction::BINARY);
		mask.dest = masked.id;
		mask.op = LOW_OP_AND;
		mask.type = type;
		mask.first = value;
		mask.second = Operand(
			static_cast<std::int64_t>(BitFieldMask(field)), type);
		derived.Emit(mask);
		return FinishBitFieldValue(binding, masked, type);
	}

	// A bit-field reached through a volatile glvalue is a volatile object
	// even when declared without volatile (3.9.3p3); the caller states it.
	bool SetVolatileBitFieldAccess(bool access)
	{
		Derived& derived = static_cast<Derived&>(*this);
		const bool previous = derived.volatile_bit_field_access_;
		derived.volatile_bit_field_access_ = access;
		return previous;
	}

	bool BitFieldAccessIsVolatile(BindingId binding) const
	{
		const Derived& derived = static_cast<const Derived&>(*this);
		return derived.volatile_bit_field_access_ ||
			derived.TypeIsVolatile(derived.program_.bindings[binding].type);
	}

	// The naturally sized pieces of a split unit's bytes, lowest first.
	static std::size_t SplitPieces(std::uint32_t bytes, std::uint32_t* offsets,
		std::uint32_t* widths)
	{
		std::size_t count = 0;
		for (std::uint32_t at = 0; at < bytes; ++count)
		{
			std::uint32_t width = 8;
			while (width > bytes - at) width /= 2;
			offsets[count] = at;
			widths[count] = width;
			at += width;
		}
		return count;
	}

	// A bit-field's storage unit, a split one assembled from its pieces.
	Operand LoadBitFieldUnit(BindingId binding, const Operand& storage,
		const LowType& type)
	{
		Derived& derived = static_cast<Derived&>(*this);
		const BindingRecord& field = derived.program_.bindings[binding];
		const BindingLayoutFact& layout = derived.program_.BindingLayout(field);
		const bool is_volatile = BitFieldAccessIsVolatile(binding);
		if (layout.bit_storage_split == 0)
			return derived.LoadStorage(storage, type, is_volatile);
		std::uint32_t offsets[4], widths[4];
		const std::size_t count =
			SplitPieces(layout.bit_storage_split, offsets, widths);
		Operand unit;
		for (std::size_t i = 0; i < count; ++i)
		{
			const LowType piece_type = widths[i] == 1 ? LowU8() :
				widths[i] == 2 ? LowU16() : widths[i] == 4 ? LowU32() : LowU64();
			const Operand address = offsets[i] == 0 ? storage :
				derived.IndexAddress(LowI8(), storage,
					Operand(static_cast<std::int64_t>(offsets[i]), LowI64()), false);
			Operand piece = derived.Convert(
				derived.LoadStorage(address, piece_type, is_volatile), type, false);
			if (offsets[i] != 0)
			{
				const Operand shifted = derived.Temp(type);
				Instruction shift(Instruction::BINARY);
				shift.dest = shifted.id;
				shift.op = LOW_OP_SHL;
				shift.type = type;
				shift.first = piece;
				shift.second = Operand(static_cast<std::int64_t>(8 * offsets[i]), type);
				derived.Emit(shift);
				piece = shifted;
			}
			if (i == 0)
			{
				unit = piece;
				continue;
			}
			const Operand combined = derived.Temp(type);
			Instruction combine(Instruction::BINARY);
			combine.dest = combined.id;
			combine.op = LOW_OP_OR;
			combine.type = type;
			combine.first = unit;
			combine.second = piece;
			derived.Emit(combine);
			unit = combined;
		}
		return unit;
	}

	Operand LoadBitField(BindingId binding, const Operand& storage)
	{
		Derived& derived = static_cast<Derived&>(*this);
		const BindingRecord& field = derived.program_.bindings[binding];
		const BindingLayoutFact& layout = derived.program_.BindingLayout(field);
		const LowType type = BitFieldMemoryType(field);
		Operand value = LoadBitFieldUnit(binding, storage, type);
		if (layout.bit_offset != 0)
		{
			const Operand shifted = derived.Temp(type);
			Instruction shift(Instruction::BINARY);
			shift.dest = shifted.id;
			shift.op = type.is_signed ? LOW_OP_SHR : LOW_OP_USHR;
			shift.type = type;
			shift.first = value;
			shift.second = Operand(layout.bit_offset, type);
			derived.Emit(shift);
			value = shifted;
		}
		return NormalizeBitFieldValue(binding, value, type);
	}

	Operand PrepareBitFieldValue(BindingId binding, Operand value,
		const LowType& type, bool mask_first = true,
		Operand* normalized_value = 0)
	{
		Derived& derived = static_cast<Derived&>(*this);
		const BindingRecord& field = derived.program_.bindings[binding];
		const BindingLayoutFact& layout = derived.program_.BindingLayout(field);
		// A constant fits a unit wider or narrower than the declared type
		// directly.
		value = derived.Convert(value, type, layout.bit_storage_bits !=
			derived.LowerExpressionType(field.type).width);
		const Operand masked = derived.Temp(type);
		Instruction mask(Instruction::BINARY);
		mask.dest = masked.id;
		mask.op = LOW_OP_AND;
		mask.type = type;
		mask.first = mask_first ? Operand(
			static_cast<std::int64_t>(BitFieldMask(field)), type) : value;
		mask.second = mask_first ? value : Operand(
			static_cast<std::int64_t>(BitFieldMask(field)), type);
		derived.Emit(mask);
		if (normalized_value)
			*normalized_value = FinishBitFieldValue(binding, masked, type);
		Operand positioned = masked;
		if (layout.bit_offset != 0)
		{
			positioned = derived.Temp(type);
			Instruction shift(Instruction::BINARY);
			shift.dest = positioned.id;
			shift.op = LOW_OP_SHL;
			shift.type = type;
			shift.first = masked;
			shift.second = Operand(layout.bit_offset, type);
			derived.Emit(shift);
		}
		return positioned;
	}

	Operand ClearBitFieldStorage(BindingId binding, const Operand& storage,
		const LowType& type)
	{
		Derived& derived = static_cast<Derived&>(*this);
		const BindingRecord& field = derived.program_.bindings[binding];
		const BindingLayoutFact& layout = derived.program_.BindingLayout(field);
		const Operand old = LoadBitFieldUnit(binding, storage, type);
		const std::uint64_t unit_mask = layout.bit_storage_bits == 64 ?
			~std::uint64_t(0) :
			(std::uint64_t(1) << layout.bit_storage_bits) - 1;
		const std::uint64_t clear_mask = unit_mask &
			~(BitFieldMask(field) << layout.bit_offset);
		const Operand cleared = derived.Temp(type);
		Instruction clear(Instruction::BINARY);
		clear.dest = cleared.id;
		clear.op = LOW_OP_AND;
		clear.type = type;
		clear.first = old;
		clear.second = Operand(static_cast<std::int64_t>(clear_mask), type);
		derived.Emit(clear);
		return cleared;
	}

	Operand CombineBitFieldValue(const Operand& cleared,
		const Operand& positioned, const LowType& type)
	{
		Derived& derived = static_cast<Derived&>(*this);
		const Operand stored = derived.Temp(type);
		Instruction combine(Instruction::BINARY);
		combine.dest = stored.id;
		combine.op = LOW_OP_OR;
		combine.type = type;
		combine.first = cleared;
		combine.second = positioned;
		derived.Emit(combine);
		return stored;
	}

	Operand MergeBitFieldStore(BindingId binding, const Operand& storage,
		const Operand& positioned, const LowType& type, bool preserve)
	{
		if (!preserve) return positioned;
		const Operand cleared = ClearBitFieldStorage(binding, storage, type);
		return CombineBitFieldValue(cleared, positioned, type);
	}

	void EmitBitFieldStore(BindingId binding, const LowType& type, const Operand& value,
		const Operand& storage)
	{
		Derived& derived = static_cast<Derived&>(*this);
		const bool is_volatile = BitFieldAccessIsVolatile(binding);
		const std::uint32_t split = derived.program_.BindingLayout(
			derived.program_.bindings[binding]).bit_storage_split;
		if (split == 0)
		{
			Instruction store(Instruction::STORE);
			store.type = type;
			store.volatile_access = is_volatile;
			store.first = value;
			store.second = storage;
			derived.Emit(store);
			return;
		}
		// A split unit is stored piecewise, each piece its own bytes.
		std::uint32_t offsets[4], widths[4];
		const std::size_t count = SplitPieces(split, offsets, widths);
		for (std::size_t i = 0; i < count; ++i)
		{
			Operand piece = value;
			if (offsets[i] != 0)
			{
				piece = derived.Temp(type);
				Instruction shift(Instruction::BINARY);
				shift.dest = piece.id;
				shift.op = LOW_OP_USHR;
				shift.type = type;
				shift.first = value;
				shift.second = Operand(static_cast<std::int64_t>(8 * offsets[i]), type);
				derived.Emit(shift);
			}
			const LowType piece_type = widths[i] == 1 ? LowU8() :
				widths[i] == 2 ? LowU16() : widths[i] == 4 ? LowU32() : LowU64();
			Instruction store(Instruction::STORE);
			store.type = piece_type;
			store.volatile_access = is_volatile;
			store.first = derived.Convert(piece, piece_type, false);
			store.second = offsets[i] == 0 ? storage : derived.IndexAddress(LowI8(),
				storage, Operand(static_cast<std::int64_t>(offsets[i]), LowI64()),
				false);
			derived.Emit(store);
		}
	}

	Operand CommitBitFieldStore(BindingId binding, const Operand& storage,
		const Operand& positioned, const LowType& type, bool preserve)
	{
		const Operand stored = MergeBitFieldStore(
			binding, storage, positioned, type, preserve);
		EmitBitFieldStore(binding, type, stored, storage);
		return positioned;
	}

	Operand StoreBitField(BindingId binding, const Operand& storage,
		Operand value, bool preserve)
	{
		Derived& derived = static_cast<Derived&>(*this);
		const LowType type = BitFieldMemoryType(
			derived.program_.bindings[binding]);
		Operand normalized;
		const Operand positioned = PrepareBitFieldValue(
			binding, value, type, false, &normalized);
		CommitBitFieldStore(binding, storage, positioned, type, preserve);
		return normalized;
	}

	bool PreserveInitializedBitField(BindingId binding)
	{
		Derived& derived = static_cast<Derived&>(*this);
		const BindingRecord& member = derived.program_.bindings[binding];
		const BindingLayoutFact& layout = derived.program_.BindingLayout(member);
		// A shared unit also holds another subobject's bytes.
		if (layout.bit_storage_shared)
		{
			derived.initialized_bit_field_unit_valid_ = false;
			return true;
		}
		const bool preserve = derived.initialized_bit_field_unit_valid_ &&
			derived.initialized_bit_field_owner_ == member.member_owner &&
			derived.initialized_bit_field_offset_ == layout.member_offset;
		derived.initialized_bit_field_unit_valid_ = true;
		derived.initialized_bit_field_owner_ = member.member_owner;
		derived.initialized_bit_field_offset_ = layout.member_offset;
		return preserve;
	}

	void InitializeBitField(BindingId binding, const Operand& value,
		const Operand& destination, const LowType& declared)
	{
		const LowType type = BitFieldStoreType(binding, declared);
		const bool preserve = PreserveInitializedBitField(binding);
		const Operand positioned = PrepareBitFieldValue(binding, value, type);
		const Operand stored = preserve ? CombineBitFieldValue(
			ClearBitFieldStorage(binding, destination, type),
			positioned, type) : positioned;
		EmitBitFieldStore(binding, type, stored, destination);
	}

	Operand LowerAssignment(const DumpNode& record,
		const NodeChildren& children)
	{
		return LowerAssignmentCore(record, children, false);
	}

	Operand LowerAssignmentCore(const DumpNode& record,
		const NodeChildren& children, bool return_storage)
	{
		Derived& derived = static_cast<Derived&>(*this);
		if (children.size() != 2)
			ThrowLoweringInternal("invalid semantic assignment");
		const int op = static_cast<int>(record.operation_kind) - 1;
		const LowType type = derived.LowerExpressionType(record.type);
		const BindingId bit_field = BitFieldBinding(children[0]);
		if (bit_field != kNoBinding)
		{
			const bool previous_volatile = SetVolatileBitFieldAccess(
				derived.TypeIsVolatile(derived.arena_.nodes[children[0]].type));
			const Operand storage = derived.LowerStorage(children[0]);
			Operand value;
			if (op == OP_ASS)
				value = derived.LowerConvertedValue(children[1], type, false);
			else
			{
				Operand left = LoadBitField(bit_field, storage);
				if (record.operand_type == kNoType)
					ThrowLoweringInternal(
						"bit-field compound assignment has no operand type");
				const LowType operation_type = derived.LowerType(record.operand_type);
				const Operand right = derived.LowerConvertedValue(
					children[1], operation_type, false);
				left = derived.Convert(left, operation_type, false);
				value = derived.Temp(operation_type);
				Instruction binary(Instruction::BINARY);
				binary.dest = value.id;
				binary.type = operation_type;
				binary.first = left;
				binary.second = right;
				binary.op = op == OP_PLUSASS ? LOW_OP_ADD : op == OP_MINUSASS ? LOW_OP_SUB :
					op == OP_STARASS ? LOW_OP_MUL : op == OP_DIVASS ?
						(operation_type.is_signed || IsFloating(operation_type) ?
							LOW_OP_DIV : LOW_OP_UDIV) :
					op == OP_MODASS ? (operation_type.is_signed ? LOW_OP_MOD : LOW_OP_UMOD) :
					op == OP_BANDASS ? LOW_OP_AND : op == OP_BORASS ? LOW_OP_OR :
					op == OP_XORASS ? LOW_OP_XOR : op == OP_LSHIFTASS ? LOW_OP_SHL : op == OP_RSHIFTASS ?
						(operation_type.is_signed ? LOW_OP_SHR : LOW_OP_USHR) : LOW_OP_NONE;
				if (binary.op == LOW_OP_NONE)
					ThrowLoweringSource(
						"unsupported bit-field compound assignment");
				binary.no_wrap = SignedArithmeticNoWrap(binary.op, operation_type);
				derived.Emit(binary);
				value = derived.Convert(value, type, false);
			}
			const Operand stored = StoreBitField(
				bit_field, storage, value, true);
			(void)SetVolatileBitFieldAccess(previous_volatile);
			return return_storage ? storage : stored;
		}

		Operand storage;
		Operand value;
		const bool volatile_access =
			derived.TypeIsVolatile(derived.arena_.nodes[children[0]].type);
		if (op == OP_ASS)
		{
			bool union_member = false;
			const DumpNode& destination = derived.arena_.nodes[children[0]];
			if (destination.kind == DUMP_MEMBER_EXPRESSION &&
				destination.binding != kNoBinding)
			{
				const EntityId owner = derived.program_.bindings[
					destination.binding].member_owner;
				union_member = owner != kNoEntity &&
					derived.program_.entities[owner].flavor == NAMED_UNION;
			}
			value = derived.LowerConvertedValue(children[1], type,
				union_member ||
				record.target_typed_scalar_immediate ||
				derived.CanonicalizeImmediateConversion(children[1]));
			storage = derived.LowerStorage(children[0]);
		}
		else if ((op == OP_PLUSASS || op == OP_MINUSASS) &&
			derived.IsPointerLikeType(derived.arena_.nodes[children[0]].type))
		{
			storage = derived.LowerStorage(children[0]);
			const Operand left =
				derived.LoadStorage(storage, LowPtr(), volatile_access);
			value = derived.ApplyPointerOffset(left,
				derived.LowerValue(children[1]),
				derived.PointeeType(derived.arena_.nodes[children[0]].type),
				op == OP_MINUSASS);
		}
		else if (op == OP_PLUSASS && record.reverse_pointer_compound_assignment)
		{
			storage = derived.LowerStorage(children[0]);
			const Operand offset =
				derived.LoadStorage(storage, type, volatile_access);
			value = derived.ApplyPointerOffset(
				derived.LowerValue(children[1]), offset,
				derived.PointeeType(derived.arena_.nodes[children[1]].type), false);
			value = derived.Convert(value, type, false);
		}
		else
		{
			storage = derived.LowerStorage(children[0]);
			Operand left = derived.Temp(type);
			Instruction load(Instruction::LOAD);
			load.dest = left.id;
			load.type = type;
			load.first = storage;
			load.volatile_access = volatile_access;
			derived.Emit(load);
			if (record.operand_type == kNoType)
				ThrowLoweringInternal(
					"compound assignment is missing its PA7 operand type");
			const LowType operation_type = derived.LowerType(record.operand_type);
			const Operand right = derived.LowerConvertedValue(
				children[1], operation_type, false);
			left = derived.Convert(left, operation_type, false);
			value = derived.Temp(operation_type);
			Instruction binary(Instruction::BINARY);
			binary.dest = value.id;
			binary.type = operation_type;
			binary.first = left;
			binary.second = right;
			binary.op = op == OP_PLUSASS ? LOW_OP_ADD : op == OP_MINUSASS ? LOW_OP_SUB :
				op == OP_STARASS ? LOW_OP_MUL : op == OP_DIVASS ?
					(operation_type.is_signed || IsFloating(operation_type) ?
						LOW_OP_DIV : LOW_OP_UDIV) :
				op == OP_MODASS ? (operation_type.is_signed ? LOW_OP_MOD : LOW_OP_UMOD) :
				op == OP_BANDASS ? LOW_OP_AND : op == OP_BORASS ? LOW_OP_OR :
				op == OP_XORASS ? LOW_OP_XOR : op == OP_LSHIFTASS ? LOW_OP_SHL : op == OP_RSHIFTASS ?
					(operation_type.is_signed ? LOW_OP_SHR : LOW_OP_USHR) : LOW_OP_NONE;
			if (binary.op == LOW_OP_NONE)
				ThrowLoweringSource("unsupported PA10 compound assignment");
			binary.no_wrap = SignedArithmeticNoWrap(binary.op, operation_type);
			derived.Emit(binary);
			value = derived.Convert(value, type, false);
		}
		Instruction store(Instruction::STORE);
		store.type = type;
		store.first = value;
		store.second = storage;
		store.volatile_access = volatile_access;
		derived.Emit(store);
		return return_storage ? storage : value;
	}
};

}
}

#endif
