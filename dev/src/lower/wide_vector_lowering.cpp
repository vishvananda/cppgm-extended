// The 256- and 512-bit forms of GCC's and Clang's x86 builtins (AVX2 and
// AVX-512, lex/gnu_builtins.h's VectorLanewise), computed in 128-bit pieces.
// A lanewise operation computes each piece of its result from the
// operands' same pieces with its SSE form (lower/sse_lowering.cpp), as the
// wide instructions define pshufb, palignr and their lane shifts on each
// 128-bit lane. A mask of lanes gathers the pieces' pmovmskb masks, lane i
// at bit i; an AVX-512 masked form keeps its passthrough vector's lanes
// where the mask's bits are clear. The code uses no AVX instruction.

#include "lower/function_lowering.h"

#include "sema/expression_facts.h"

namespace lower {

namespace {

const lowir::Type kI32 = lowir::Type::scalar(lowir::TypeKind::I32);
const lowir::Type kI64 = lowir::Type::scalar(lowir::TypeKind::I64);
const lowir::Type kV128 = lowir::Type::scalar(lowir::TypeKind::V128);

lowir::Type BitsType(uint64_t bytes)
{
	return lowir::Type::scalar(bytes == 1 ? lowir::TypeKind::I8
			: bytes == 2                  ? lowir::TypeKind::I16
			: bytes == 4                  ? lowir::TypeKind::I32
										  : lowir::TypeKind::I64);
}

bool IsVector(const sema::Type* type)
{
	return type->unqualified->kind == sema::TypeKind::Vector;
}

} // namespace

uint64_t FunctionLowering::vector_pieces(const LanewiseCall& call)
{
	// The 128-bit pieces of the call's widest vector.
	uint64_t pieces = 1;
	if (IsVector(call.result))
		pieces = std::max(pieces, lowering_.size_of(call.result, 0) / 16);
	for (uint32_t i = 0; i < call.count; ++i)
		if (IsVector(call.types[i]))
			pieces = std::max(pieces, lowering_.size_of(call.types[i], 0) / 16);
	return pieces;
}

const sema::Type* FunctionLowering::piece_type(const sema::Type* vector)
{
	// A vector of the lanes of 16 bytes.
	const sema::Type* lane = vector->unqualified->element;
	return lowering_.graph().types.vector(lane, 16 / lowering_.size_of(lane, 0));
}

RValue FunctionLowering::vector_piece(lowir::Value address, uint64_t piece)
{
	Place place = Place();
	place.storage =
		index(lowir::Type::scalar(lowir::TypeKind::I8), ir_model::IPK_FIELD, address, integer(kI64, piece * 16)).value;
	place.untyped = true;
	return load(kV128, place);
}

void FunctionLowering::set_vector_piece(lowir::Value address, uint64_t piece, const RValue& value)
{
	Place place = Place();
	place.storage =
		index(lowir::Type::scalar(lowir::TypeKind::I8), ir_model::IPK_FIELD, address, integer(kI64, piece * 16)).value;
	place.untyped = true;
	store(kV128, value, place);
}

RValue FunctionLowering::vector_pair(uint64_t low, uint64_t high)
{
	// The two 64-bit halves of a 128-bit constant.
	const RValue a = integer(kI64, low), b = integer(kI64, high);
	const RValue first = vector_instruction(lowir::VectorOp::Fromi64, &a);
	const RValue second = vector_instruction(lowir::VectorOp::Fromi64, &b);
	return vector_instruction(lowir::VectorOp::Punpcklqdq, &first, &second);
}

RValue FunctionLowering::byte_lane_mask(const RValue& bits)
{
	// A 16-bit mask's bits as byte lanes of all ones or zero: each byte of
	// the mask spread to eight lanes, and each lane's own bit tested.
	const uint64_t weights = 0x8040201008040201ull;
	const RValue spread = vector_pair(0, 0x0101010101010101ull);
	const RValue tested = vector_pair(weights, weights);
	const RValue word = convert_op(lowir::ConvertOp::Trunc, kI32, bits);
	const RValue value = vector_instruction(lowir::VectorOp::Fromi32, &word);
	const RValue lanes = vector_instruction(lowir::VectorOp::Pshufb, &value, &spread);
	const RValue picked = vector_instruction(lowir::VectorOp::Pand, &lanes, &tested);
	return vector_instruction(lowir::VectorOp::Pcmpeqb, &picked, &tested);
}

RValue FunctionLowering::masked_lanes(
	const sema::Type* vector, const RValue& value, const RValue& passthrough, const RValue& mask)
{
	// AVX-512's merge masking: a lane whose mask bit is clear is the
	// passthrough's. A mask of every lane keeps the value.
	vector = vector->unqualified;
	const uint64_t lanes = vector->bound;
	const uint64_t all = lanes >= 64 ? ~uint64_t(0) : (uint64_t(1) << lanes) - 1;
	uint64_t bits = 0;
	if (is_integer_literal(mask, bits) && (bits & all) == all)
		return value;
	const uint64_t bytes = lowering_.size_of(vector->element, 0);
	const RValue wide = mask.type == kI64 ? mask : convert_op(lowir::ConvertOp::ZExt, kI64, mask);
	const lowir::Value chosen = vector_storage(value);
	const lowir::Value kept = vector_storage(passthrough);
	lowir::SlotId slot = lowir::kNoId;
	const lowir::Value out = zeroed_vector(vector, slot);
	if (bytes == 1)
	{
		for (uint64_t piece = 0; piece * 16 < lanes; ++piece)
		{
			const RValue part = binary(lowir::BinaryOp::UShr, kI64, wide, integer(kI64, piece * 16));
			const RValue lane_mask = byte_lane_mask(part);
			const RValue a = vector_piece(kept, piece), b = vector_piece(chosen, piece);
			set_vector_piece(out, piece, vector_instruction(lowir::VectorOp::Pblendvb, &a, &b, &lane_mask));
		}
		return finish_lanes(vector, slot);
	}
	for (uint64_t i = 0; i < lanes; ++i)
	{
		const RValue bit = binary(
			lowir::BinaryOp::And, kI64, binary(lowir::BinaryOp::UShr, kI64, wide, integer(kI64, i)), integer(kI64, 1));
		const RValue set = compare(lowir::ComparePredicate::Ne, kI64, bit, integer(kI64, 0));
		set_lane_bits(out, bytes, i, select_value(set, lane_bits(chosen, bytes, i), lane_bits(kept, bytes, i)));
	}
	return finish_lanes(vector, slot);
}

RValue FunctionLowering::wide_lanewise(LanewiseCall& call, const syntax::CallExpr& expr)
{
	RValue passthrough, mask;
	if (call.masked)
	{
		call.count -= 2;
		passthrough = call.values[call.count];
		mask = call.values[call.count + 1];
	}
	RValue result;
	switch (call.operation)
	{
	case VectorLanewiseOp::SignMask:
	case VectorLanewiseOp::MaskEqual:
	case VectorLanewiseOp::MaskGreater:
	case VectorLanewiseOp::MaskTest:
	case VectorLanewiseOp::MaskCompare:
	case VectorLanewiseOp::TestZero:
		return wide_mask(call, expr);
	case VectorLanewiseOp::BlendMask:
	case VectorLanewiseOp::SelectMask:
		return wide_select(call);
	case VectorLanewiseOp::Compress:
	case VectorLanewiseOp::CompressStore:
		return wide_compress(call);
	case VectorLanewiseOp::PermuteIndexFirst:
	case VectorLanewiseOp::PermuteIndexSecond:
		result = wide_permute(call);
		break;
	case VectorLanewiseOp::ExtractPiece:
	case VectorLanewiseOp::InsertPiece:
	case VectorLanewiseOp::Widen:
	case VectorLanewiseOp::Narrow:
		result = wide_piece_move(call);
		break;
	case VectorLanewiseOp::Extend:
	case VectorLanewiseOp::ExtendUnsigned:
		result = wide_extend(call, expr);
		break;
	default:
		result = piecewise_vector(call, expr);
		break;
	}
	return call.masked ? masked_lanes(call.result, result, passthrough, mask) : result;
}

RValue FunctionLowering::piecewise_vector(const LanewiseCall& call, const syntax::CallExpr& expr)
{
	// Each piece of the result from the operands' same pieces; an operand
	// that is no vector (a count, an immediate) goes to every piece, and a
	// shift by an int count is SSE2's.
	const sema::Type* result = call.result->unqualified;
	if (!IsVector(result))
		throw LoweringError(expr.location, "a packed operation without a vector result in 128-bit pieces");
	const uint64_t pieces = lowering_.size_of(result, 0) / 16;
	lowir::Value sources[4];
	for (uint32_t i = 0; i < call.count; ++i)
		if (IsVector(call.types[i]))
			sources[i] = vector_storage(call.values[i]);
	const bool int_shift = (call.operation == VectorLanewiseOp::ShiftLeft ||
							   call.operation == VectorLanewiseOp::ShiftRightLogical ||
							   call.operation == VectorLanewiseOp::ShiftRightArithmetic) &&
		call.count == 2 && !IsVector(call.types[1]);
	lowir::SlotId slot = generated_slot(lowir::SlotKind::Vector, lowering_.storage_type(result));
	Place storage = Place();
	storage.storage = lowir::Value::slot(slot);
	const lowir::Value out = address_of(storage);
	for (uint64_t p = 0; p < pieces; ++p)
	{
		LanewiseCall piece = call;
		piece.masked = false;
		piece.result = piece_type(result);
		for (uint32_t i = 0; i < call.count; ++i)
			if (IsVector(call.types[i]))
			{
				piece.types[i] = piece_type(call.types[i]);
				piece.values[i] = vector_piece(sources[i], p);
			}
		RValue value;
		if (int_shift)
		{
			const unsigned operation = call.operation == VectorLanewiseOp::ShiftLeft ? 0
				: call.operation == VectorLanewiseOp::ShiftRightLogical              ? 1
																					 : 2;
			value = shift_lanes(piece.result, piece.values[0], call.values[1], operation);
		}
		else
			value = lanewise_call(piece, expr);
		set_vector_piece(out, p, value);
	}
	return finish_lanes(result, slot);
}

RValue FunctionLowering::wide_mask(const LanewiseCall& call, const syntax::CallExpr& expr)
{
	// Each piece's 16 lanes' bits from pmovmskb, the piece's at bit 16 *
	// piece; ptest's zero flag of the whole vector from every piece's.
	using lowir::VectorOp;
	const VectorLanewiseOp op = call.operation;
	const lowir::Type type = lowering_.storage_type(call.result);
	const sema::Type* vector = call.types[0];
	const uint64_t pieces = lowering_.size_of(vector, 0) / 16;
	const lowir::Value a = vector_storage(call.values[0]);
	const bool paired = call.count > 1 && IsVector(call.types[1]);
	const lowir::Value b = paired ? vector_storage(call.values[1]) : a;
	uint64_t predicate = 0;
	if (op == VectorLanewiseOp::MaskCompare && !is_integer_literal(call.values[2], predicate))
		throw LoweringError(expr.location, "a mask comparison whose predicate is not a constant");
	const RValue zero = vector_instruction(VectorOp::Pzero);
	RValue result = integer(type, op == VectorLanewiseOp::TestZero ? 1 : 0);
	for (uint64_t p = 0; p < pieces; ++p)
	{
		const RValue x = vector_piece(a, p), y = vector_piece(b, p);
		if (op == VectorLanewiseOp::TestZero)
		{
			const RValue tested = vector_instruction(VectorOp::Ptestz, &x, &y, nullptr, -1, kI32);
			result = binary(lowir::BinaryOp::And, kI32, result, tested);
			continue;
		}
		// The lanes as all ones or zero, and whether their bits are negated.
		RValue lanes = x;
		bool negate = false;
		if (op == VectorLanewiseOp::MaskEqual || (op == VectorLanewiseOp::MaskCompare && predicate % 4 == 0))
			lanes = vector_instruction(VectorOp::Pcmpeqb, &x, &y);
		else if (op == VectorLanewiseOp::MaskGreater || (op == VectorLanewiseOp::MaskCompare && predicate % 4 == 2))
			lanes = vector_instruction(VectorOp::Pcmpgtb, &x, &y);
		else if (op == VectorLanewiseOp::MaskCompare && predicate % 4 == 1)
			lanes = vector_instruction(VectorOp::Pcmpgtb, &y, &x);
		else if (op == VectorLanewiseOp::MaskCompare)
			lanes = zero;
		else if (op == VectorLanewiseOp::MaskTest)
		{
			const RValue both = vector_instruction(VectorOp::Pand, &x, &y);
			lanes = vector_instruction(VectorOp::Pcmpeqb, &both, &zero);
		}
		// eq lt le false: 0 1 2 3; their negations ne nlt nle true: 4 5 6 7
		// (le is not gt, nle gt).
		if (op == VectorLanewiseOp::MaskCompare)
			negate = (predicate >= 4) != (predicate % 4 == 2);
		else
			negate = op == VectorLanewiseOp::MaskTest;
		RValue bits = vector_instruction(VectorOp::Pmovmskb, &lanes, nullptr, nullptr, -1, kI32);
		if (negate)
			bits = binary(lowir::BinaryOp::Xor, kI32, bits, integer(kI32, 0xffff));
		if (type != kI32)
			bits = convert_op(lowir::ConvertOp::ZExt, type, bits);
		if (p != 0)
			bits = binary(lowir::BinaryOp::Shl, type, bits, integer(type, p * 16));
		result = binary(lowir::BinaryOp::Or, type, result, bits);
	}
	if (op == VectorLanewiseOp::SignMask || op == VectorLanewiseOp::TestZero)
		return result;
	const RValue& mask = call.values[call.count - 1];
	const RValue given = mask.type == type ? mask : convert_op(lowir::ConvertOp::ZExt, type, mask);
	return binary(lowir::BinaryOp::And, type, result, given);
}

RValue FunctionLowering::wide_select(const LanewiseCall& call)
{
	// Byte lanes of the second vector where the mask's bits are set: GCC's
	// blend (vector, second, mask), Clang's select (mask, second, vector).
	const bool blend = call.operation == VectorLanewiseOp::BlendMask;
	const sema::Type* vector = call.result->unqualified;
	const RValue& mask = call.values[blend ? 2 : 0];
	const RValue& other = call.values[blend ? 0 : 2];
	return masked_lanes(vector, call.values[1], other, mask);
}

RValue FunctionLowering::wide_compress(const LanewiseCall& call)
{
	// The lanes whose mask bits are set, in order, by a loop over the set
	// bits: into a copy of the passthrough vector, or to memory.
	const bool to_memory = call.operation == VectorLanewiseOp::CompressStore;
	const uint32_t source_index = to_memory ? 1 : 0;
	const sema::Type* vector = call.types[source_index];
	const uint64_t lanes = vector->unqualified->bound;
	const uint64_t bytes = lowering_.size_of(vector->unqualified->element, 0);
	const lowir::Type lane = BitsType(bytes);
	const lowir::Value source = vector_storage(call.values[source_index]);
	lowir::SlotId slot = lowir::kNoId;
	lowir::Value target = call.values[0].value;
	if (!to_memory)
	{
		slot = generated_slot(lowir::SlotKind::Vector, lowering_.storage_type(vector));
		Place copy = Place();
		copy.storage = lowir::Value::slot(slot);
		store(lowering_.storage_type(vector), call.values[1], copy);
		target = address_of(copy);
	}
	const RValue& given = call.values[2];
	RValue mask = given.type == kI64 ? given : convert_op(lowir::ConvertOp::ZExt, kI64, given);
	if (lanes < 64)
		mask = binary(lowir::BinaryOp::And, kI64, mask, integer(kI64, (uint64_t(1) << lanes) - 1));
	Place left = Place(), next = Place();
	left.storage = lowir::Value::slot(generated_slot(lowir::SlotKind::Vector, kI64));
	next.storage = lowir::Value::slot(generated_slot(lowir::SlotKind::Vector, kI64));
	store(kI64, mask, left);
	store(kI64, integer(kI64, 0), next);
	const lowir::BlockId test = new_block(lowir::BlockKind::VectorCompress);
	const lowir::BlockId body = new_block(lowir::BlockKind::VectorCompressLane);
	const lowir::BlockId end = new_block(lowir::BlockKind::VectorCompressEnd);
	jump(test);
	place(test);
	const RValue bits = load(kI64, left);
	branch(compare(lowir::ComparePredicate::Ne, kI64, bits, integer(kI64, 0)), body, end);
	place(body);
	const RValue first = convert_op(lowir::ConvertOp::ZExt, kI64, bit_count(sema::Intrinsic::CountTrailingZeros, bits));
	Place from = Place(), to = Place();
	from.storage = index(lowir::Type::scalar(lowir::TypeKind::I8), ir_model::IPK_FIELD, source,
		binary(lowir::BinaryOp::Mul, kI64, first, integer(kI64, bytes)))
					   .value;
	from.untyped = true;
	const RValue at = load(kI64, next);
	to.storage = index(lowir::Type::scalar(lowir::TypeKind::I8), ir_model::IPK_FIELD, target,
		binary(lowir::BinaryOp::Mul, kI64, at, integer(kI64, bytes)))
					 .value;
	to.untyped = true;
	store(lane, load(lane, from), to);
	store(kI64, binary(lowir::BinaryOp::Add, kI64, at, integer(kI64, 1)), next);
	store(kI64, binary(lowir::BinaryOp::And, kI64, bits, binary(lowir::BinaryOp::Sub, kI64, bits, integer(kI64, 1))),
		left);
	jump(test);
	place(end);
	return to_memory ? RValue() : finish_lanes(vector, slot);
}

RValue FunctionLowering::wide_permute(const LanewiseCall& call)
{
	// vpermt2q and vpermi2q: lane i is the two tables' lane its index's
	// low bits select, the first table's below the lanes' count; GCC's form
	// keeps the first table's lane where its mask's bit is clear.
	const bool index_first = call.operation == VectorLanewiseOp::PermuteIndexFirst;
	const sema::Type* vector = call.result->unqualified;
	const uint64_t lanes = vector->bound;
	const uint64_t bytes = lowering_.size_of(vector->element, 0);
	const uint64_t total = lanes * bytes;
	const lowir::Type type = lowering_.storage_type(vector);
	const RValue& indexes = call.values[index_first ? 0 : 1];
	const RValue& first = call.values[index_first ? 1 : 0];
	Place tables = Place();
	tables.storage = lowir::Value::slot(generated_slot(lowir::SlotKind::Vector, lowir::Type::object(total * 2, 4)));
	const lowir::Value base = address_of(tables);
	for (uint32_t table = 0; table < 2; ++table)
	{
		Place half = Place();
		half.storage = index(
			lowir::Type::scalar(lowir::TypeKind::I8), ir_model::IPK_FIELD, base, integer(kI64, table * total))
						   .value;
		half.untyped = true;
		store(type, table == 0 ? first : call.values[2], half);
	}
	const lowir::Value selectors = vector_storage(indexes);
	lowir::SlotId slot = lowir::kNoId;
	const lowir::Value out = zeroed_vector(vector, slot);
	for (uint64_t i = 0; i < lanes; ++i)
	{
		RValue at = lane_bits(selectors, bytes, i);
		if (bytes != 8)
			at = convert_op(lowir::ConvertOp::ZExt, kI64, at);
		at = binary(lowir::BinaryOp::And, kI64, at, integer(kI64, lanes * 2 - 1));
		Place lane = Place();
		lane.storage = index(lowir::Type::scalar(lowir::TypeKind::I8), ir_model::IPK_FIELD, base,
			binary(lowir::BinaryOp::Mul, kI64, at, integer(kI64, bytes)))
						   .value;
		lane.untyped = true;
		set_lane_bits(out, bytes, i, load(BitsType(bytes), lane));
	}
	const RValue result = finish_lanes(vector, slot);
	return index_first ? masked_lanes(vector, result, first, call.values[3]) : result;
}

RValue FunctionLowering::wide_piece_move(const LanewiseCall& call)
{
	// vextractf128 and vextracti32x4 read the piece an index's low bits
	// select, vinsertf128 replaces it; a cast to a wider vector leaves the
	// rest zero, to a narrower one reads the low piece.
	const sema::Type* result = call.result->unqualified;
	const auto piece_offset = [&](const RValue& selector, uint64_t pieces) {
		const RValue low = binary(lowir::BinaryOp::And, selector.type, selector, integer(selector.type, pieces - 1));
		const RValue wide = low.type == kI64 ? low : convert_op(lowir::ConvertOp::ZExt, kI64, low);
		return binary(lowir::BinaryOp::Mul, kI64, wide, integer(kI64, 16));
	};
	Place place = Place();
	place.untyped = true;
	switch (call.operation)
	{
	case VectorLanewiseOp::ExtractPiece:
	{
		const lowir::Value source = vector_storage(call.values[0]);
		place.storage = index(lowir::Type::scalar(lowir::TypeKind::I8), ir_model::IPK_FIELD, source,
			piece_offset(call.values[1], lowering_.size_of(call.types[0], 0) / 16))
							.value;
		return load(kV128, place);
	}
	case VectorLanewiseOp::InsertPiece:
	{
		const lowir::SlotId slot = generated_slot(lowir::SlotKind::Vector, lowering_.storage_type(result));
		Place copy = Place();
		copy.storage = lowir::Value::slot(slot);
		store(lowering_.storage_type(result), call.values[0], copy);
		place.storage = index(lowir::Type::scalar(lowir::TypeKind::I8), ir_model::IPK_FIELD, address_of(copy),
			piece_offset(call.values[2], lowering_.size_of(result, 0) / 16))
							.value;
		store(kV128, call.values[1], place);
		return finish_lanes(result, slot);
	}
	case VectorLanewiseOp::Widen:
	{
		lowir::SlotId slot = lowir::kNoId;
		set_vector_piece(zeroed_vector(result, slot), 0, call.values[0]);
		return finish_lanes(result, slot);
	}
	default:
		return vector_piece(vector_storage(call.values[0]), 0);
	}
}

RValue FunctionLowering::wide_extend(const LanewiseCall& call, const syntax::CallExpr& expr)
{
	// Each result piece extends the source's next lanes; the source is
	// stored with 16 zero bytes after it, so that every piece reads 16.
	const sema::Type* result = call.result->unqualified;
	const sema::Type* source = call.types[0];
	const uint64_t pieces = lowering_.size_of(result, 0) / 16;
	const uint64_t chunk = 16 * lowering_.size_of(source->element, 0) / lowering_.size_of(result->element, 0);
	const uint64_t bytes = lowering_.size_of(source, 0);
	const lowir::Type padded = lowir::Type::object(bytes + 16, 4);
	const lowir::SlotId staged = generated_slot(lowir::SlotKind::Vector, padded);
	lowir::Instruction zero = lowir::make_instruction(
		lowir::Opcode::ZeroInit, lowir::Type::scalar(lowir::TypeKind::Void));
	zero.ops[0] = lowir::Value::slot(staged);
	zero.immediate = static_cast<int64_t>(padded.bytes);
	zero.sub = static_cast<uint8_t>(padded.align_log2);
	emit(zero);
	Place stage = Place();
	stage.storage = lowir::Value::slot(staged);
	const lowir::Value base = address_of(stage);
	Place whole = Place();
	whole.storage = base;
	whole.untyped = true;
	store(lowering_.storage_type(source), call.values[0], whole);
	lowir::SlotId slot = lowir::kNoId;
	const lowir::Value out = zeroed_vector(result, slot);
	for (uint64_t p = 0; p < pieces; ++p)
	{
		LanewiseCall piece = call;
		piece.masked = false;
		piece.result = piece_type(result);
		piece.types[0] = piece_type(source);
		Place part = Place();
		part.storage =
			index(lowir::Type::scalar(lowir::TypeKind::I8), ir_model::IPK_FIELD, base, integer(kI64, p * chunk)).value;
		part.untyped = true;
		piece.values[0] = load(kV128, part);
		set_vector_piece(out, p, lanewise_call(piece, expr));
	}
	return finish_lanes(result, slot);
}

} // namespace lower
