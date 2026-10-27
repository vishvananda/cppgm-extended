// GNU and Clang vector values (vector_size, ext_vector_type): an object of
// the lanes' bytes (LowIR has no vector type; a bool vector's lanes are its
// bits, Clang's layout), compared lane by lane, converted by
// __builtin_convertvector, combined by __builtin_reduce_*, and copied,
// passed and returned whole. Lane operations are unrolled (64 lanes at
// most). __builtin_bit_cast of any trivially copyable types is here too: the
// operand's bytes read as the type. At a host object's call boundary a
// vector of at most 4 bytes is INTEGER, of 8 SSE (one double lane MEMORY),
// of 16 SSE and SSEUP, larger ones MEMORY (no AVX), as GCC passes them
// (Clang returns a one-double vector and a 32-byte one in XMM registers).
// GCC's x86 lane builtins (__builtin_ia32_vec_init/ext/set) and vector
// casts (a vector's bytes as another vector or integer) are here too.

#include "lower/function_lowering.h"

#include "sema/conversions.h"
#include "sema/expression_facts.h"
#include "sema/integral_arithmetic.h"

namespace lower {

namespace {

const uint64_t kMaxUnrolledLanes = 64;

const syntax::Expr* WithoutParentheses(const syntax::Expr* expr)
{
	while (expr->kind == syntax::ExprKind::Paren)
		expr = static_cast<const syntax::ParenExpr*>(expr)->inner;
	return expr;
}

bool BoolLanes(const sema::Type* vector)
{
	return vector->unqualified->element->unqualified->fundamental == FundamentalType::Bool;
}

bool SignedLane(const sema::Type* lane)
{
	sema::IntegralKind kind;
	return sema::IsFloating(lane) || (sema::IntegralKindOf(lane, kind) && kind.is_signed && kind.bits > 1);
}

} // namespace

uint8_t ProgramLowering::vector_classes(const sema::Type* type)
{
	if (!object_mode_)
		return 0;
	const sema::Type* vector = type->unqualified;
	const sema::Type* lane = vector->element->unqualified;
	const uint64_t bytes = size_of(vector, 0);
	// GCC's (followed where Clang's differs): one floating lane has no
	// vector mode (V1SF, V1DF, V1XF), so MEMORY; the 4-byte floating
	// vector (V2HF) is SSE, other vectors of at most 4 bytes INTEGER; an
	// 8-byte one SSE (__m64's; Clang's 33- to 64-lane bool vectors too).
	if (vector->bound == 1 && sema::IsFloating(lane))
		return lowir::OC_STATED | lowir::OC_MEMORY;
	if (bytes <= 4)
		return sema::IsFloating(lane) ? lowir::OC_STATED | lowir::EB_SSE : lowir::OC_STATED | lowir::EB_INTEGER;
	if (bytes == 8)
		return lowir::OC_STATED | lowir::EB_SSE;
	if (bytes == 16)
		return static_cast<uint8_t>(lowir::OC_STATED | lowir::EB_SSE | (lowir::EB_SSEUP << 2));
	return lowir::OC_STATED | lowir::OC_MEMORY;
}

lowir::Value FunctionLowering::vector_storage(const RValue& value)
{
	// A vector value in a slot of its own, addressed.
	Place storage = Place();
	storage.storage = lowir::Value::slot(generated_slot(lowir::SlotKind::Vector, value.type));
	store(value.type, value, storage);
	return address_of(storage);
}

RValue FunctionLowering::vector_lane(lowir::Value address, const sema::Type* vector, uint64_t lane)
{
	// A lane's value in its storage type (a bool lane's bit as a u8).
	const lowir::Type i64 = lowir::Type::scalar(lowir::TypeKind::I64);
	const lowir::Type i8 = lowir::Type::scalar(lowir::TypeKind::I8);
	const lowir::Type u8 = lowir::Type::scalar(lowir::TypeKind::U8);
	Place place = Place();
	if (BoolLanes(vector))
	{
		place.storage = index(i8, ir_model::IPK_FIELD, address, integer(i64, lane / 8)).value;
		RValue byte = load(u8, place);
		if (lane % 8 != 0)
			byte = binary(lowir::BinaryOp::UShr, u8, byte, integer(u8, lane % 8));
		return binary(lowir::BinaryOp::And, u8, byte, integer(u8, 1));
	}
	const sema::Type* element = vector->unqualified->element;
	place.storage = index(i8, ir_model::IPK_FIELD, address, integer(i64, lane * lowering_.size_of(element, 0))).value;
	return load(lowering_.storage_type(element), place);
}

void FunctionLowering::set_vector_lane(
	lowir::Value address, const sema::Type* vector, uint64_t lane, const RValue& value)
{
	// A bool lane's bit or-ed into its byte (the storage starts zeroed).
	const lowir::Type i64 = lowir::Type::scalar(lowir::TypeKind::I64);
	const lowir::Type i8 = lowir::Type::scalar(lowir::TypeKind::I8);
	const lowir::Type u8 = lowir::Type::scalar(lowir::TypeKind::U8);
	Place place = Place();
	if (BoolLanes(vector))
	{
		place.storage = index(i8, ir_model::IPK_FIELD, address, integer(i64, lane / 8)).value;
		RValue bit = value.type.kind() == lowir::TypeKind::U8 ? value : convert_op(lowir::ConvertOp::Trunc, u8, value);
		if (lane % 8 != 0)
			bit = binary(lowir::BinaryOp::Shl, u8, bit, integer(u8, lane % 8));
		store(u8, binary(lowir::BinaryOp::Or, u8, load(u8, place), bit), place);
		return;
	}
	const sema::Type* element = vector->unqualified->element;
	place.storage = index(i8, ir_model::IPK_FIELD, address, integer(i64, lane * lowering_.size_of(element, 0))).value;
	store(lowering_.storage_type(element), value, place);
}

lowir::Value FunctionLowering::zeroed_vector(const sema::Type* vector, lowir::SlotId& slot)
{
	const lowir::Type type = lowering_.storage_type(vector);
	if (vector->unqualified->bound > kMaxUnrolledLanes)
		throw LoweringError(0, "a vector operation on more than 64 lanes");
	slot = generated_slot(lowir::SlotKind::Vector, type);
	lowir::Instruction zero = lowir::make_instruction(
		lowir::Opcode::ZeroInit, lowir::Type::scalar(lowir::TypeKind::Void));
	zero.ops[0] = lowir::Value::slot(slot);
	zero.immediate = static_cast<int64_t>(type.bytes);
	zero.sub = static_cast<uint8_t>(type.align_log2);
	emit(zero);
	Place place = Place();
	place.storage = lowir::Value::slot(slot);
	return address_of(place);
}

Place FunctionLowering::vector_lane_place(const syntax::SubscriptExpr& expr)
{
	// A vector lvalue's lane: its storage plus the index's lane offsets.
	const syntax::Expr& base = *WithoutParentheses(expr.base);
	const lowir::Value address = address_of(glvalue(base));
	const lowir::Type i64 = lowir::Type::scalar(lowir::TypeKind::I64);
	RValue offset = convert(rvalue(*expr.index), WithoutParentheses(expr.index)->facts->type,
		lowering_.graph().types.fundamental(FundamentalType::LongInt), ConversionContext::Additive);
	offset = binary(
		lowir::BinaryOp::Mul, i64, offset, integer(i64, lowering_.size_of(expr.facts->type, expr.location)));
	Place place = Place();
	place.storage = index(lowir::Type::scalar(lowir::TypeKind::I8), ir_model::IPK_ARRAY_ELEMENT, address, offset).value;
	qualify(place, expr.facts->type);
	return place;
}

RValue FunctionLowering::vector_lane_value(const syntax::SubscriptExpr& expr)
{
	// A vector prvalue's lane, from the slot holding it; a bool vector's
	// lane, its bit (the index's byte shifted by its bit position).
	const syntax::Expr& base = *WithoutParentheses(expr.base);
	const sema::Type* vector = base.facts->type;
	if (!BoolLanes(vector))
	{
		if (base.facts->category != sema::ValueCategory::PRValue)
			return load(lowering_.storage_type(expr.facts->type), vector_lane_place(expr));
		const lowir::Value address = vector_storage(rvalue(base));
		const lowir::Type i64 = lowir::Type::scalar(lowir::TypeKind::I64);
		RValue offset = convert(rvalue(*expr.index), WithoutParentheses(expr.index)->facts->type,
			lowering_.graph().types.fundamental(FundamentalType::LongInt), ConversionContext::Additive);
		offset = binary(
			lowir::BinaryOp::Mul, i64, offset, integer(i64, lowering_.size_of(expr.facts->type, expr.location)));
		Place place = Place();
		place.storage =
			index(lowir::Type::scalar(lowir::TypeKind::I8), ir_model::IPK_ARRAY_ELEMENT, address, offset).value;
		return load(lowering_.storage_type(expr.facts->type), place);
	}
	const lowir::Value address = base.facts->category != sema::ValueCategory::PRValue ? address_of(glvalue(base))
																					  : vector_storage(rvalue(base));
	const lowir::Type i64 = lowir::Type::scalar(lowir::TypeKind::I64);
	const lowir::Type u8 = lowir::Type::scalar(lowir::TypeKind::U8);
	const RValue position = convert(rvalue(*expr.index), WithoutParentheses(expr.index)->facts->type,
		lowering_.graph().types.fundamental(FundamentalType::LongInt), ConversionContext::Additive);
	Place byte = Place();
	byte.storage = index(lowir::Type::scalar(lowir::TypeKind::I8), ir_model::IPK_ARRAY_ELEMENT, address,
		binary(lowir::BinaryOp::Shr, i64, position, integer(i64, 3)))
					   .value;
	const RValue shift = convert_op(
		lowir::ConvertOp::Trunc, u8, binary(lowir::BinaryOp::And, i64, position, integer(i64, 7)));
	return binary(lowir::BinaryOp::And, u8, binary(lowir::BinaryOp::UShr, u8, load(u8, byte), shift), integer(u8, 1));
}

RValue FunctionLowering::vector_list(const sema::Type* vector, const syntax::BracedInitList& list)
{
	// Each element converted to its lane (as the analysis recorded), the
	// lanes after them zero (GNU's list-initialization).
	lowir::SlotId slot = lowir::kNoId;
	const lowir::Value out = zeroed_vector(vector, slot);
	const sema::Type* lane = vector->unqualified->element;
	for (uint32_t i = 0; i < list.elements.size; ++i)
		set_vector_lane(out, vector, i, fit(operand(*list.elements[i], ConversionContext::Initialization), lane));
	Place place = Place();
	place.storage = lowir::Value::slot(slot);
	return load(lowering_.storage_type(vector), place);
}

RValue FunctionLowering::vector_compare(const syntax::BinaryExpr& expr, const RValue& left, const RValue& right)
{
	// Each lane pair compared (a scalar operand, converted to the lane type,
	// in every lane; signed or floating lanes by their order, binary16 and
	// binary128 ones as such, unsigned ones as unsigned): -1 or 0 in the
	// truth vector's lane (a bool vector's 1 or 0).
	const TokenKind op = expr.op;
	const bool left_vector = WithoutParentheses(expr.left)->facts->type->unqualified->kind == sema::TypeKind::Vector;
	const bool right_vector = WithoutParentheses(expr.right)->facts->type->unqualified->kind == sema::TypeKind::Vector;
	const sema::Type* vector =
		(left_vector ? WithoutParentheses(expr.left) : WithoutParentheses(expr.right))->facts->type;
	const sema::Type* result = expr.facts->type;
	const sema::Type* lane = vector->unqualified->element->unqualified;
	const bool is_signed = SignedLane(lane);
	const SoftFloat soft = SoftFloatOf(lane);
	lowir::ComparePredicate predicate = lowir::ComparePredicate::Eq;
	switch (op)
	{
	case TokenKind::OP_EQ:
		predicate = lowir::ComparePredicate::Eq;
		break;
	case TokenKind::OP_NE:
		predicate = lowir::ComparePredicate::Ne;
		break;
	case TokenKind::OP_LT:
		predicate = is_signed ? lowir::ComparePredicate::Lt : lowir::ComparePredicate::Ult;
		break;
	case TokenKind::OP_LE:
		predicate = is_signed ? lowir::ComparePredicate::Le : lowir::ComparePredicate::Ule;
		break;
	case TokenKind::OP_GT:
		predicate = is_signed ? lowir::ComparePredicate::Gt : lowir::ComparePredicate::Ugt;
		break;
	default:
		predicate = is_signed ? lowir::ComparePredicate::Ge : lowir::ComparePredicate::Uge;
		break;
	}
	const lowir::Value a = left_vector ? vector_storage(left) : lowir::Value::none();
	const lowir::Value b = right_vector ? vector_storage(right) : lowir::Value::none();
	lowir::SlotId slot = lowir::kNoId;
	const lowir::Value out = zeroed_vector(result, slot);
	const lowir::Type truth_type = lowering_.storage_type(result->unqualified->element);
	const lowir::Type i64 = lowir::Type::scalar(lowir::TypeKind::I64);
	for (uint64_t i = 0; i < vector->unqualified->bound; ++i)
	{
		const RValue x = left_vector ? vector_lane(a, vector, i) : left;
		const RValue y = right_vector ? vector_lane(b, vector, i) : right;
		const RValue truth = soft != SoftFloat::None ? soft_compare(predicate, soft, x, y)
													 : compare(predicate, x.type, x, y);
		if (BoolLanes(result))
			set_vector_lane(out, result, i, truth);
		else
		{
			const RValue mask = binary(lowir::BinaryOp::Sub, i64, integer(i64, 0), truth);
			set_vector_lane(out, result, i,
				truth_type.kind() == lowir::TypeKind::I64 ? mask
														  : convert_op(lowir::ConvertOp::Trunc, truth_type, mask));
		}
	}
	Place place = Place();
	place.storage = lowir::Value::slot(slot);
	return load(lowering_.storage_type(result), place);
}

RValue FunctionLowering::vector_arithmetic(const syntax::BinaryExpr& expr, const RValue& left, const RValue& right)
{
	return vector_operation(expr.op, expr.facts->type, left,
		WithoutParentheses(expr.left)->facts->type->unqualified->kind == sema::TypeKind::Vector, right,
		WithoutParentheses(expr.right)->facts->type->unqualified->kind == sema::TypeKind::Vector);
}

RValue FunctionLowering::vector_operation(TokenKind op, const sema::Type* vector, const RValue& left, bool left_vector,
	const RValue& right, bool right_vector)
{
	// Each lane pair combined in the lane type (a scalar operand, converted
	// to it, in every lane), unpromoted, as GCC and Clang compute vector
	// operations: integral lanes wrap at their width.
	const sema::Type* lane = vector->unqualified->element->unqualified;
	const lowir::Value a = left_vector ? vector_storage(left) : lowir::Value::none();
	const lowir::Value b = right_vector ? vector_storage(right) : lowir::Value::none();
	lowir::SlotId slot = lowir::kNoId;
	const lowir::Value out = zeroed_vector(vector, slot);
	for (uint64_t i = 0; i < vector->unqualified->bound; ++i)
	{
		const RValue x = left_vector ? vector_lane(a, vector, i) : left;
		const RValue y = right_vector ? vector_lane(b, vector, i) : right;
		set_vector_lane(out, vector, i, arithmetic_op(static_cast<uint32_t>(op), lane, x, y));
	}
	Place place = Place();
	place.storage = lowir::Value::slot(slot);
	return load(lowering_.storage_type(vector), place);
}

RValue FunctionLowering::vector_unary(TokenKind op, const sema::Type* vector, const RValue& value)
{
	// - and ~ of each lane in the lane type.
	const sema::Type* lane = vector->unqualified->element->unqualified;
	const lowir::Value a = vector_storage(value);
	lowir::SlotId slot = lowir::kNoId;
	const lowir::Value out = zeroed_vector(vector, slot);
	for (uint64_t i = 0; i < vector->unqualified->bound; ++i)
	{
		const RValue x = vector_lane(a, vector, i);
		RValue y;
		if (op == TokenKind::OP_COMPL)
			y = unary(lowir::UnaryOp::BitNot, x);
		else if (x.type.is_object() && SoftFloatOf(lane) != SoftFloat::None)
			y = soft_negate(SoftFloatOf(lane), x);
		else
			y = unary(lowir::UnaryOp::Neg, x);
		set_vector_lane(out, vector, i, y);
	}
	Place place = Place();
	place.storage = lowir::Value::slot(slot);
	return load(lowering_.storage_type(vector), place);
}

RValue FunctionLowering::convert_vector(const syntax::BuiltinTraitExpr& expr)
{
	// Each lane converted as a value of its type to the result's lane type
	// (to bool: nonzero).
	const sema::Type* from = WithoutParentheses(expr.operand)->facts->type->unqualified;
	const sema::Type* to = expr.facts->type->unqualified;
	const lowir::Value source = vector_storage(operand(*expr.operand, ConversionContext::Operand));
	lowir::SlotId slot = lowir::kNoId;
	const lowir::Value out = zeroed_vector(to, slot);
	const sema::Type* from_lane = from->element->unqualified;
	const sema::Type* to_lane = to->element->unqualified;
	for (uint64_t i = 0; i < to->bound; ++i)
	{
		RValue value = vector_lane(source, from, i);
		if (BoolLanes(from))
		{
			// A bool lane's bit is its value.
			set_vector_lane(out, to, i,
				convert(value, lowering_.graph().types.fundamental(FundamentalType::Bool), to_lane,
					ConversionContext::Cast));
			continue;
		}
		value = convert(value, from_lane, to_lane, ConversionContext::Cast);
		set_vector_lane(out, to, i, value);
	}
	Place place = Place();
	place.storage = lowir::Value::slot(slot);
	return load(lowering_.storage_type(to), place);
}

RValue FunctionLowering::shift_vector_bytes(const syntax::CallExpr& expr, sema::Intrinsic kind)
{
	// The complete vector moves by bytes, across its original lane
	// boundaries. The sixteen byte reads are bounded even for a wrapper's
	// still-variable immediate: an out-of-range position selects zero.
	const lowir::Type i64 = lowir::Type::scalar(lowir::TypeKind::I64);
	const lowir::Type i8 = lowir::Type::scalar(lowir::TypeKind::I8);
	const sema::Type* vector = expr.facts->type;
	const lowir::Value source = vector_storage(operand(*expr.arguments[0], ConversionContext::Argument));
	const bool runtime = (expr.facts->value >> 16 & 1) != 0;
	const uint64_t bytes = expr.facts->value >> 8 & 0xff;
	const bool left = kind == sema::Intrinsic::VectorShiftLeftBytes;
	RValue count;
	if (runtime)
	{
		const RValue bits = operand(*expr.arguments[1], ConversionContext::Argument);
		count = convert_op(
			lowir::ConvertOp::ZExt, i64, binary(lowir::BinaryOp::UShr, bits.type, bits, integer(bits.type, 3)));
	}
	else
		discard(*expr.arguments[1]);
	lowir::SlotId slot = lowir::kNoId;
	const lowir::Value out = zeroed_vector(vector, slot);
	for (uint64_t i = 0; i < 16; ++i)
	{
		if (!runtime && (left ? i < bytes : i + bytes >= 16))
			continue;
		RValue offset = integer(i64, left ? i - bytes : i + bytes);
		RValue valid;
		if (runtime)
		{
			valid = compare(left ? lowir::ComparePredicate::Ule : lowir::ComparePredicate::Ult, i64, count,
				integer(i64, left ? i : 16 - i));
			offset = binary(left ? lowir::BinaryOp::Sub : lowir::BinaryOp::Add, i64, integer(i64, i), count);
			offset = binary(lowir::BinaryOp::And, i64, offset, integer(i64, 15));
		}
		Place byte = Place();
		byte.storage = index(i8, ir_model::IPK_FIELD, source, offset).value;
		RValue value = load(i8, byte);
		if (runtime)
			value = binary(lowir::BinaryOp::Mul, i8, value, convert_op(lowir::ConvertOp::Trunc, i8, valid));
		byte.storage = index(i8, ir_model::IPK_FIELD, out, integer(i64, i)).value;
		store(i8, value, byte);
	}
	Place place = Place();
	place.storage = lowir::Value::slot(slot);
	return load(lowering_.storage_type(vector), place);
}

RValue FunctionLowering::reduce_vector(const syntax::CallExpr& expr, sema::Intrinsic kind)
{
	// The lanes combined in order: bitwise, sums and products in the lane
	// type (bool lanes as their bits), max and min by the lanes' order (a
	// NaN lane taken only when every lane is NaN: Clang's IEEE maxNum and
	// minNum).
	const sema::Type* vector = WithoutParentheses(expr.arguments[0])->facts->type->unqualified;
	const sema::Type* lane = vector->element->unqualified;
	if (vector->bound > kMaxUnrolledLanes)
		throw LoweringError(expr.location, "a vector operation on more than 64 lanes");
	const lowir::Value source = vector_storage(operand(*expr.arguments[0], ConversionContext::Argument));
	RValue result = vector_lane(source, vector, 0);
	const bool is_signed = SignedLane(lane);
	const SoftFloat soft = SoftFloatOf(lane);
	const auto lanes_compare = [&](lowir::ComparePredicate predicate, const RValue& x, const RValue& y) {
		return soft != SoftFloat::None ? soft_compare(predicate, soft, x, y) : compare(predicate, x.type, x, y);
	};
	for (uint64_t i = 1; i < vector->bound; ++i)
	{
		const RValue x = vector_lane(source, vector, i);
		switch (kind)
		{
		case sema::Intrinsic::ReduceAnd:
			result = binary(lowir::BinaryOp::And, result.type, result, x);
			break;
		case sema::Intrinsic::ReduceOr:
			result = binary(lowir::BinaryOp::Or, result.type, result, x);
			break;
		case sema::Intrinsic::ReduceXor:
			result = binary(lowir::BinaryOp::Xor, result.type, result, x);
			break;
		case sema::Intrinsic::ReduceAdd:
			result = binary(lowir::BinaryOp::Add, result.type, result, x);
			break;
		case sema::Intrinsic::ReduceMul:
			result = binary(lowir::BinaryOp::Mul, result.type, result, x);
			break;
		default:
		{
			// The larger (smaller) of the two through a slot: kept unless the
			// new lane orders after (before) it or it is a NaN.
			const bool max = kind == sema::Intrinsic::ReduceMax;
			Place best = Place();
			best.storage = lowir::Value::slot(generated_slot(lowir::SlotKind::Vector, result.type));
			store(result.type, result, best);
			const lowir::ComparePredicate predicate = max
				? (is_signed ? lowir::ComparePredicate::Gt : lowir::ComparePredicate::Ugt)
				: (is_signed ? lowir::ComparePredicate::Lt : lowir::ComparePredicate::Ult);
			const lowir::BlockId take = new_block(lowir::BlockKind::VectorTake);
			const lowir::BlockId join = new_block(lowir::BlockKind::VectorJoin);
			if (sema::IsFloating(lane))
			{
				const lowir::BlockId unordered = new_block(lowir::BlockKind::VectorUnordered);
				branch(lanes_compare(predicate, x, result), take, unordered);
				place(unordered);
				branch(lanes_compare(lowir::ComparePredicate::Ne, result, result), take, join);
			}
			else
				branch(compare(predicate, x.type, x, result), take, join);
			place(take);
			store(result.type, x, best);
			jump(join);
			place(join);
			result = load(result.type, best);
			break;
		}
		}
	}
	return result;
}

RValue FunctionLowering::vector_lanes(const syntax::CallExpr& expr, sema::Intrinsic kind)
{
	// GCC's lane builtins: init stores each operand in its lane; ext reads
	// the lane the analysis's constant selector names (bits 8-15 of the
	// facts' value) from the operand's storage, set replaces it in a copy.
	// A selector read at run time (bit 16) names the lane of its low bits,
	// as the instructions' immediates do (the lane counts are powers of 2).
	const uint64_t selected = expr.facts->value >> 8 & 0xff;
	const bool run_time = (expr.facts->value >> 16 & 1) != 0;
	if (kind == sema::Intrinsic::VectorInit)
	{
		const sema::Type* vector = expr.facts->type->unqualified;
		lowir::SlotId slot = lowir::kNoId;
		const lowir::Value out = zeroed_vector(vector, slot);
		for (uint32_t i = 0; i < expr.arguments.size; ++i)
			set_vector_lane(
				out, vector, i, fit(operand(*expr.arguments[i], ConversionContext::Argument), vector->element));
		Place place = Place();
		place.storage = lowir::Value::slot(slot);
		return load(lowering_.storage_type(vector), place);
	}
	const sema::ExprFacts* argument = WithoutParentheses(expr.arguments[0])->facts;
	// Lane builtins consume the converted vector, including a class conversion.
	const sema::Type* vector =
		(argument->conversion != nullptr ? argument->conversion->result : argument->type)->unqualified;
	const lowir::Value source = vector_storage(operand(*expr.arguments[0], ConversionContext::Argument));
	const sema::Type* element = vector->element;
	const auto lane_at = [&](const syntax::Expr& selector) {
		const lowir::Type i64 = lowir::Type::scalar(lowir::TypeKind::I64);
		const RValue n = convert_op(lowir::ConvertOp::SExt, i64,
			fit(operand(selector, ConversionContext::Argument),
				lowering_.graph().types.fundamental(FundamentalType::Int)));
		Place lane = Place();
		lane.storage = index(lowering_.storage_type(element), ir_model::IPK_ARRAY_ELEMENT, source,
			binary(lowir::BinaryOp::And, i64, n, integer(i64, vector->bound - 1)))
						   .value;
		return lane;
	};
	if (kind == sema::Intrinsic::VectorExtract)
	{
		if (run_time)
			return load(lowering_.storage_type(element), lane_at(*expr.arguments[1]));
		discard(*expr.arguments[1]);
		return vector_lane(source, vector, selected);
	}
	const RValue value = fit(operand(*expr.arguments[1], ConversionContext::Argument), element);
	if (run_time)
		store(lowering_.storage_type(element), value, lane_at(*expr.arguments[2]));
	else
	{
		discard(*expr.arguments[2]);
		set_vector_lane(source, vector, selected, value);
	}
	Place place = Place();
	place.storage = source;
	return load(lowering_.storage_type(vector), place);
}

RValue FunctionLowering::bit_cast(const syntax::BuiltinTraitExpr& expr)
{
	const sema::Type* to = expr.facts->type->unqualified;
	const sema::Type* from = WithoutParentheses(expr.operand)->facts->type;
	if (from->unqualified->kind == sema::TypeKind::Array)
	{
		Place source = glvalue(*expr.operand);
		source.untyped = true;
		return reinterpret_bytes(load(lowering_.storage_type(from), source), to);
	}
	return reinterpret_bytes(
		fit(operand(*expr.operand, ConversionContext::Operand), WithoutParentheses(expr.operand)->facts->type), to);
}

RValue FunctionLowering::reinterpret_bytes(const RValue& value, const sema::Type* to)
{
	// The value stored in a slot of its type, its bytes read as `to`'s (of
	// their size: a bit cast, a vector cast); neither access is `typed`
	// (lowir.md "Instruction Facts": the bytes are read as another type).
	Place storage = Place();
	storage.storage = lowir::Value::slot(generated_slot(lowir::SlotKind::Vector, value.type));
	storage.untyped = true;
	store(value.type, value, storage);
	Place bytes = Place();
	bytes.storage = address_of(storage);
	bytes.untyped = true;
	return load(lowering_.storage_type(to->unqualified), bytes);
}

} // namespace lower
