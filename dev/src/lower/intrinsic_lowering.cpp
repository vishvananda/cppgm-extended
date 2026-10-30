// GCC's builtins that lowering expands in place (sema::Intrinsic): the
// floating classification and constants, and the hosted headers'
// addresses, stack storage, variadic argument lists, byte swaps and bit
// counts.

#include "lower/function_lowering.h"

#include <limits>

#include "lowir/literals.h"
#include "sema/expression_facts.h"
#include "sema/integral_arithmetic.h"
#include "sema/pretty_function.h"
#include "sema/string_literals.h"

namespace lower {

RValue FunctionLowering::intrinsic(const syntax::CallExpr& expr)
{
	// GCC's floating classification expanded over the operand's value: NaN
	// as x != x, infinity as x == ±inf, normal as |x| at least the least
	// normal value and finite, each truth an i32; fpclassify sums the
	// given codes weighted by the five exclusive truths.
	const sema::Intrinsic kind = static_cast<sema::Intrinsic>(expr.facts->value & 0xff);
	if (kind == sema::Intrinsic::Nan || kind == sema::Intrinsic::Inf)
	{
		const lowir::Type type = lowering_.storage_type(expr.facts->type);
		return floating(type,
			lowir::float_literal(type.kind(),
				kind == sema::Intrinsic::Nan
					? sema::BuiltinNan(expr.facts->type->unqualified->fundamental, *expr.arguments[0])
					: std::numeric_limits<long double>::infinity()));
	}
	if (kind == sema::Intrinsic::NanSignaling)
	{
		// A signaling NaN: exponent all ones, the quiet bit clear and the
		// payload below it (GCC's: the bit below the quiet bit for none);
		// binary16's and binary128's the type's default one. Its operand
		// is evaluated.
		discard(*expr.arguments[0]);
		const lowir::Type type = lowering_.storage_type(expr.facts->type);
		const uint64_t payload = sema::BuiltinNanPayload(*expr.arguments[0]);
		lowir::Literal bits = lowir::special_float(type.kind(), lowir::SpecialFloat::SignalingNaN, false);
		if (type.kind() == lowir::TypeKind::F32 || type.kind() == lowir::TypeKind::F64 ||
			type.kind() == lowir::TypeKind::F80)
		{
			const unsigned width = type.kind() == lowir::TypeKind::F32 ? 22
				: type.kind() == lowir::TypeKind::F64                  ? 51
																	   : 62;
			const uint64_t mask = (uint64_t(1) << width) - 1;
			const uint64_t fraction = (payload & mask) != 0 ? payload & mask : uint64_t(1) << (width - 1);
			bits.low = type.kind() == lowir::TypeKind::F32 ? 0x7f800000ull | fraction
				: type.kind() == lowir::TypeKind::F64      ? 0x7ff0000000000000ull | fraction
														   : 0x8000000000000000ull | fraction;
			bits.high = type.kind() == lowir::TypeKind::F80 ? 0x7fff : 0;
		}
		return floating(type, bits);
	}
	if (kind >= sema::Intrinsic::AddressOf)
		return gnu_intrinsic(expr, kind);
	if (kind >= sema::Intrinsic::IsGreater)
	{
		// The quiet comparisons, of operands converted to their common
		// type: an ordered predicate (false for a NaN), less-or-greater
		// both ordered ones, unordered a NaN on either side.
		const RValue x = bfloat_float(*expr.arguments[0], operand(*expr.arguments[0], ConversionContext::Operand));
		const RValue y = bfloat_float(*expr.arguments[1], operand(*expr.arguments[1], ConversionContext::Operand));
		static const lowir::ComparePredicate kPredicates[] = { lowir::ComparePredicate::Gt, lowir::ComparePredicate::Ge,
			lowir::ComparePredicate::Lt, lowir::ComparePredicate::Le };
		const lowir::Type i32 = lowir::Type::scalar(lowir::TypeKind::I32);
		const SoftFloat soft = SoftFloatOfValue(x.type);
		const auto ordered = [&](lowir::ComparePredicate predicate) {
			return truth_i32(
				soft != SoftFloat::None ? soft_compare(predicate, soft, x, y) : compare(predicate, x.type, x, y));
		};
		// (Operands emitted last first: see FunctionLowering::binary.)
		if (kind == sema::Intrinsic::IsLessGreater)
		{
			const RValue greater = ordered(lowir::ComparePredicate::Gt);
			return binary(lowir::BinaryOp::Or, i32, ordered(lowir::ComparePredicate::Lt), greater);
		}
		if (kind == sema::Intrinsic::IsUnordered)
		{
			const RValue y_nan = classify_nan(y);
			return binary(lowir::BinaryOp::Or, i32, classify_nan(x), y_nan);
		}
		return ordered(kPredicates[static_cast<uint8_t>(kind) - static_cast<uint8_t>(sema::Intrinsic::IsGreater)]);
	}
	const syntax::Expr& operand_expr = *expr.arguments[expr.arguments.size - 1];
	const RValue x = bfloat_float(operand_expr, operand(operand_expr, ConversionContext::Operand));
	const lowir::Type i32 = lowir::Type::scalar(lowir::TypeKind::I32);
	switch (kind)
	{
	case sema::Intrinsic::SignBit:
		return sign_bit(x);
	case sema::Intrinsic::IsNan:
		return classify_nan(x);
	case sema::Intrinsic::IsInf:
		return classify_infinite(x);
	case sema::Intrinsic::IsSignaling:
	{
		// A NaN whose quiet bit is clear.
		const RValue signaling = binary(lowir::BinaryOp::Xor, i32, quiet_bit(x), integer(i32, 1));
		return binary(lowir::BinaryOp::And, i32, classify_nan(x), signaling);
	}
	case sema::Intrinsic::IsInfSign:
	{
		// infinite - 2 * (infinite & sign).
		const RValue infinite = classify_infinite(x);
		const RValue negative = binary(lowir::BinaryOp::And, i32, infinite, sign_bit(x));
		return binary(lowir::BinaryOp::Sub, i32, infinite, binary(lowir::BinaryOp::Add, i32, negative, negative));
	}
	case sema::Intrinsic::IsFinite:
		return classify_finite(x);
	case sema::Intrinsic::IsNormal:
		return classify_normal(x);
	default:
		break;
	}
	RValue codes[5];
	for (uint32_t i = 0; i < 5; ++i)
		codes[i] = operand(*expr.arguments[i], ConversionContext::Argument);
	const RValue nan = classify_nan(x);
	const RValue infinite = classify_infinite(x);
	const RValue not_nan = binary(lowir::BinaryOp::Sub, i32, integer(i32, 1), nan);
	const RValue not_infinite = binary(lowir::BinaryOp::Sub, i32, integer(i32, 1), infinite);
	const RValue finite = binary(lowir::BinaryOp::And, i32, not_nan, not_infinite);
	const SoftFloat soft = SoftFloatOfValue(x.type);
	const RValue zero = truth_i32(soft != SoftFloat::None
			? soft_compare(lowir::ComparePredicate::Eq, soft, x, soft_zero(soft))
			: compare(
				  lowir::ComparePredicate::Eq, x.type, x, floating(x.type, lowir::float_literal(x.type.kind(), 0.0L))));
	const RValue normal = classify_normal(x);
	const RValue not_zero = binary(lowir::BinaryOp::Sub, i32, integer(i32, 1), zero);
	const RValue not_normal = binary(lowir::BinaryOp::Sub, i32, integer(i32, 1), normal);
	const RValue subnormal = binary(
		lowir::BinaryOp::And, i32, finite, binary(lowir::BinaryOp::And, i32, not_zero, not_normal));
	const RValue truths[5] = { nan, infinite, normal, subnormal, zero };
	RValue sum = integer(i32, 0);
	for (uint32_t i = 0; i < 5; ++i)
		sum = binary(lowir::BinaryOp::Add, i32, sum, binary(lowir::BinaryOp::Mul, i32, codes[i], truths[i]));
	return sum;
}

lowir::Value FunctionLowering::va_list_tag(const syntax::Expr& list)
{
	// A variadic argument list's __va_list_tag: a __builtin_va_list object
	// (an array of one) at its address, a parameter of that type (adjusted
	// to a pointer, 8.3.5p5) at the address it holds.
	if (list.facts->type->kind == sema::TypeKind::Pointer)
		return operand(list, ConversionContext::Operand).value;
	return address(list);
}

RValue FunctionLowering::gnu_intrinsic(const syntax::CallExpr& expr, sema::Intrinsic kind)
{
	// The GCC builtins of hosted headers (sema::Intrinsic): addresses and
	// stack storage directly, the variadic argument list through LowIR's
	// va_start, byte swaps by LowIR's bswap, and the bit counts as
	// straight-line arithmetic (LowIR has no such operation).
	if (kind >= sema::Intrinsic::AtomicLoadN)
		return atomic_intrinsic(expr, kind);
	const lowir::Type pointer = lowir::Type::scalar(lowir::TypeKind::Ptr);
	const auto list_tag = [this](const syntax::Expr& list) { return va_list_tag(list); };
	switch (kind)
	{
	case sema::Intrinsic::AddressOf:
	{
		RValue result;
		result.value = address(*expr.arguments[0]);
		result.type = pointer;
		return result;
	}
	case sema::Intrinsic::Alloca:
	{
		const RValue size = operand(*expr.arguments[0], ConversionContext::Argument);
		lowir::Instruction inst = lowir::make_instruction(lowir::Opcode::StackAlloc, pointer);
		inst.ops[0] = size.value;
		RValue result;
		result.value = emit_value(inst, pointer);
		result.type = pointer;
		return result;
	}
	case sema::Intrinsic::VaStart:
	{
		lowir::Instruction inst = lowir::make_instruction(
			lowir::Opcode::VaStart, lowir::Type::scalar(lowir::TypeKind::Void));
		inst.ops[0] = list_tag(*expr.arguments[0]);
		emit(inst);
		return RValue();
	}
	case sema::Intrinsic::VaEnd:
		// The x86-64 psABI's va_end does nothing.
		address(*expr.arguments[0]);
		return RValue();
	case sema::Intrinsic::VaCopy:
	{
		const lowir::Value destination = list_tag(*expr.arguments[0]);
		const lowir::Value source = list_tag(*expr.arguments[1]);
		const sema::Type* list = expr.arguments[0]->facts->type;
		while (list->kind == sema::TypeKind::Array || list->kind == sema::TypeKind::Pointer)
			list = list->element;
		copy_object(list, source, destination, expr.location);
		return RValue();
	}
	case sema::Intrinsic::ByteSwap:
		return unary(lowir::UnaryOp::ByteSwap, operand(*expr.arguments[0], ConversionContext::Argument));
	case sema::Intrinsic::Prefetch:
		// A hint: the address operand's effects only.
		for (const syntax::Expr* argument : expr.arguments)
			discard(*argument);
		return RValue();
	case sema::Intrinsic::Launder:
		// The pointer's bits are unchanged. LowIR memory effects and object
		// lifetime events determine the loads through the returned pointer.
		return operand(*expr.arguments[0], ConversionContext::Argument);
	case sema::Intrinsic::AssumeAligned:
	{
		const RValue value = operand(*expr.arguments[0], ConversionContext::Argument);
		for (uint32_t i = 1; i < expr.arguments.size; ++i)
			discard(*expr.arguments[i]);
		return value;
	}
	case sema::Intrinsic::FltRounds:
		return flt_rounds();
	case sema::Intrinsic::ObjectSize:
		// Its operands are not evaluated.
		return integer(lowir::Type::scalar(lowir::TypeKind::I64), (expr.facts->value >> 8 & 1) != 0 ? 0 : ~uint64_t(0));
	case sema::Intrinsic::MakeComplex:
		return make_complex(expr);
	case sema::Intrinsic::ReduceAnd:
	case sema::Intrinsic::ReduceOr:
	case sema::Intrinsic::ReduceXor:
	case sema::Intrinsic::ReduceAdd:
	case sema::Intrinsic::ReduceMul:
	case sema::Intrinsic::ReduceMax:
	case sema::Intrinsic::ReduceMin:
		return reduce_vector(expr, kind);
	case sema::Intrinsic::VectorInit:
	case sema::Intrinsic::VectorExtract:
	case sema::Intrinsic::VectorInsert:
		return vector_lanes(expr, kind);
	case sema::Intrinsic::VectorShiftLeftBytes:
	case sema::Intrinsic::VectorShiftRightBytes:
		return shift_vector_bytes(expr, kind);
	case sema::Intrinsic::VectorTruncate:
		return truncate_vector(expr);
	case sema::Intrinsic::VectorByteMask:
		return vector_byte_mask(expr);
	case sema::Intrinsic::VectorByteMinMax:
		return vector_byte_minmax(expr);
	case sema::Intrinsic::VectorAndNot:
		return vector_andnot(expr);
	case sema::Intrinsic::VectorUnpack:
		return unpack_vector(expr);
	case sema::Intrinsic::VectorShiftLanes:
		return shift_vector_lanes(expr);
	case sema::Intrinsic::VectorShuffle:
		return shuffle_vector(expr);
	case sema::Intrinsic::SourceFile:
	case sema::Intrinsic::SourceLine:
	case sema::Intrinsic::SourceFunction:
	case sema::Intrinsic::SourceColumn:
		return source_location(kind, expr.location);
	case sema::Intrinsic::IsConstantEvaluated:
		// Code runs only outside constant evaluation.
		return integer(lowir::Type::scalar(lowir::TypeKind::I64), 0);
	case sema::Intrinsic::AddOverflow:
	case sema::Intrinsic::SubOverflow:
	case sema::Intrinsic::MulOverflow:
		return overflow_intrinsic(expr, kind);
	case sema::Intrinsic::GenericCountLeadingZeros:
	case sema::Intrinsic::GenericCountTrailingZeros:
	case sema::Intrinsic::GenericPopCount:
	{
		// Counted in the operand's width, a narrower one widened to 32
		// bits (its leading zeros less the widening's); clzg and ctzg give
		// their second operand for 0, chosen by a `select` (lowir.md
		// "Unary and Binary Operations").
		RValue x = operand(*expr.arguments[0], ConversionContext::Argument);
		const RValue given = x;
		const uint32_t bits = static_cast<uint32_t>(
			lowering_.size_of(expr.arguments[0]->facts->type, expr.location) * 8);
		const sema::Intrinsic base = static_cast<sema::Intrinsic>(static_cast<uint8_t>(kind) -
			static_cast<uint8_t>(sema::Intrinsic::GenericCountLeadingZeros) +
			static_cast<uint8_t>(sema::Intrinsic::CountLeadingZeros));
		if (bits < 32)
			x = convert_op(lowir::ConvertOp::ZExt, lowir::Type::scalar(lowir::TypeKind::U32), x);
		const lowir::Type i32 = lowir::Type::scalar(lowir::TypeKind::I32);
		RValue count;
		if (bits == 128)
		{
			// By 64-bit halves: the population's sum; the leading (trailing)
			// zeros of the high (low) half, or 64 and the other half's when
			// it is 0.
			const lowir::Type i64 = lowir::Type::scalar(lowir::TypeKind::I64);
			const RValue low = convert_op(lowir::ConvertOp::Trunc, i64, x);
			const RValue high = convert_op(lowir::ConvertOp::Trunc, i64,
				binary(lowir::BinaryOp::UShr, x.type, x, integer(x.type, lowir::Literal{ 64, 0 })));
			const RValue low_count = bit_count(base, low), high_count = bit_count(base, high);
			if (base == sema::Intrinsic::PopCount)
				count = binary(lowir::BinaryOp::Add, i32, low_count, high_count);
			else
			{
				const bool leading = base == sema::Intrinsic::CountLeadingZeros;
				const RValue first = leading ? high_count : low_count;
				const RValue second = binary(
					lowir::BinaryOp::Add, i32, integer(i32, 64), leading ? low_count : high_count);
				const RValue empty = truth_i32(
					compare(lowir::ComparePredicate::Eq, i64, leading ? high : low, integer(i64, 0)));
				count = binary(lowir::BinaryOp::Add, i32, first,
					binary(lowir::BinaryOp::Mul, i32, empty, binary(lowir::BinaryOp::Sub, i32, second, first)));
			}
		}
		else
			count = bit_count(base, x);
		if (base == sema::Intrinsic::CountLeadingZeros && bits < 32)
			count = binary(lowir::BinaryOp::Sub, i32, count, integer(i32, 32 - bits));
		if (expr.arguments.size == 2)
		{
			const RValue nonzero = compare(lowir::ComparePredicate::Ne, given.type, given, integer(given.type, 0));
			const RValue zero = operand(*expr.arguments[1], ConversionContext::Argument);
			lowir::Instruction choice = lowir::make_instruction(lowir::Opcode::Select, i32);
			choice.ops[0] = nonzero.value;
			choice.ops[1] = count.value;
			choice.ops[2] = zero.value;
			count.value = emit_value(choice, i32);
			count.type = i32;
		}
		return count;
	}
	default:
		break;
	}
	return bit_count(kind, operand(*expr.arguments[0], ConversionContext::Argument));
}

RValue FunctionLowering::source_location(sema::Intrinsic kind, uint32_t location)
{
	// The outermost default argument's use (or the constructor running a
	// member's initializer) being lowered, else the call's own position;
	// the name `__func__` holds in the function evaluating it (none in a
	// namespace-scope initialization), its characters' array shared with
	// equal string literals.
	if (source_site_ != kNoSourceSite)
		location = source_site_;
	const lowir::Type i32 = lowir::Type::scalar(lowir::TypeKind::I32);
	if (kind == sema::Intrinsic::SourceLine)
		return integer(i32, lowering_.source_line(location));
	if (kind == sema::Intrinsic::SourceColumn)
		return integer(i32, 0);
	// A captureless lambda's static invoker runs its function call
	// operator's body: the name is the operator's (5.1.2p6).
	const sema::FunctionEntity* function = function_;
	const sema::ClosureFacts* closure = function != nullptr && function->member && function->owner != nullptr &&
			function->owner->kind == sema::ScopeKind::Class && function->owner->entity != nullptr
		? lowering_.graph().closure_facts(static_cast<const sema::ClassEntity*>(function->owner->entity))
		: nullptr;
	if (closure != nullptr && closure->invoker == function && closure->call != nullptr)
		function = closure->call;
	Place text = Place();
	text.storage = lowir::Value::symbol(kind == sema::Intrinsic::SourceFile
			? lowering_.predefined_name(lowering_.source_file(location).c_str())
			: lowering_.predefined_name(function != nullptr ? sema::PredefinedFunctionName(function).c_str() : ""));
	RValue value;
	value.type = lowir::Type::scalar(lowir::TypeKind::Ptr);
	value.value = address_of(text);
	return value;
}

RValue FunctionLowering::bit_count(sema::Intrinsic kind, RValue x)
{
	// clz, ctz or popcount of a 32- or 64-bit operand, an i32.
	const lowir::Type type = x.type;
	const uint32_t width = type.kind() == lowir::TypeKind::I64 ? 64 : 32;
	const lowir::Type i32 = lowir::Type::scalar(lowir::TypeKind::I32);
	RValue count = integer(type, 0);
	if (kind == sema::Intrinsic::FindFirstSet)
	{
		// As for the generic zero-count builtins, Select masks the unused
		// undefined scan result for zero (lowir.md, Unary and Binary Operations).
		count = binary(lowir::BinaryOp::Add, i32, bit_count(sema::Intrinsic::CountTrailingZeros, x), integer(i32, 1));
		const RValue nonzero = compare(lowir::ComparePredicate::Ne, type, x, integer(type, 0));
		lowir::Instruction choice = lowir::make_instruction(lowir::Opcode::Select, i32);
		choice.ops[0] = nonzero.value;
		choice.ops[1] = count.value;
		choice.ops[2] = integer(i32, 0).value;
		count.value = emit_value(choice, i32);
		count.type = i32;
		return count;
	}
	if (kind == sema::Intrinsic::PopCount)
	{
		// Bits summed in parallel (pairs, nibbles, bytes), the byte sums
		// gathered by one multiplication.
		const uint64_t m1 = 0x5555555555555555ull, m2 = 0x3333333333333333ull, m4 = 0x0f0f0f0f0f0f0f0full;
		const uint64_t h01 = 0x0101010101010101ull;
		x = binary(lowir::BinaryOp::Sub, type, x,
			binary(lowir::BinaryOp::And, type, binary(lowir::BinaryOp::UShr, type, x, integer(type, 1)),
				integer(type, m1)));
		const RValue high_pairs = binary(
			lowir::BinaryOp::And, type, binary(lowir::BinaryOp::UShr, type, x, integer(type, 2)), integer(type, m2));
		x = binary(lowir::BinaryOp::Add, type, binary(lowir::BinaryOp::And, type, x, integer(type, m2)), high_pairs);
		x = binary(lowir::BinaryOp::And, type,
			binary(lowir::BinaryOp::Add, type, x, binary(lowir::BinaryOp::UShr, type, x, integer(type, 4))),
			integer(type, m4));
		count = binary(lowir::BinaryOp::UShr, type, binary(lowir::BinaryOp::Mul, type, x, integer(type, h01)),
			integer(type, width - 8));
	}
	else
		// LowIR's bit scans (undefined for 0, as GCC's).
		count = unary(kind == sema::Intrinsic::CountLeadingZeros ? lowir::UnaryOp::CountLeadingZeros
																 : lowir::UnaryOp::CountTrailingZeros,
			x);
	// An int: a u32 count retyped by a copy in the course's LowIR (its
	// convention); a host object's back end reads the count as it is (it
	// fits either type), so -O0 does not renormalize it.
	if (width != 32)
		return convert_op(lowir::ConvertOp::Trunc, i32, count);
	return count.type == i32 || lowering_.object_mode() ? count : copy(i32, count);
}

lowir::MemoryOrder FunctionLowering::memory_order(const syntax::Expr& order)
{
	// A constant order of GCC's encoding (LowIR's); any other is seq_cst,
	// as GCC takes an order that is no constant.
	uint64_t bits = 0;
	if (is_integer_literal(operand(order, ConversionContext::Argument), bits) && bits < lowir::kMemoryOrderCount)
		return static_cast<lowir::MemoryOrder>(bits);
	return lowir::MemoryOrder::SeqCst;
}

RValue FunctionLowering::atomic_intrinsic(const syntax::CallExpr& expr, sema::Intrinsic kind)
{
	// GCC's __atomic builtins (sema::Intrinsic) as LowIR's atomic
	// operations, on the object's scalar type (a pointer's as i64: GCC's
	// arithmetic is unscaled, a __c11_atomic builtin's on an _Atomic(T*)
	// counts T's, as Clang's; the generic forms' and __c11_atomic's
	// floating-point or class object of 1, 2, 4 or 8 bytes as the integer
	// of its size, its bytes loaded and stored as that integer, a value
	// operand's and a floating result's bits reinterpreted through a slot,
	// never converted). Adds and subtracts are atomic_add_fetch; the
	// bitwise updates are compare-exchange loops (lowir.md).
	using sema::Intrinsic;
	const syntax::Slice<syntax::Expr*>& arguments = expr.arguments;
	const lowir::Type void_type = lowir::Type::scalar(lowir::TypeKind::Void);
	// A __sync builtin's implied order (+ 1 above the kind), else the
	// order operand at `index`.
	const uint64_t implied = expr.facts->value >> 8;
	const auto order_at = [&](uint32_t index) {
		return implied != 0 ? static_cast<lowir::MemoryOrder>(implied - 1) : memory_order(*arguments[index]);
	};
	if (kind == Intrinsic::AtomicThreadFence || kind == Intrinsic::AtomicSignalFence)
	{
		lowir::Instruction inst = lowir::make_instruction(
			kind == Intrinsic::AtomicThreadFence ? lowir::Opcode::AtomicThreadFence : lowir::Opcode::AtomicSignalFence,
			void_type);
		inst.order = order_at(0);
		emit(inst);
		return RValue();
	}
	const bool byte = kind == Intrinsic::AtomicTestAndSet || kind == Intrinsic::AtomicClear;
	const sema::Type* pointer = arguments[0]->facts->type->unqualified;
	const lowir::Type object_type = byte ? lowir::Type::scalar(lowir::TypeKind::U8)
										 : lowering_.storage_type(pointer->element);
	lowir::Type stored = object_type;
	const bool reinterpreted = !stored.is_integer() && stored.kind() != lowir::TypeKind::Ptr &&
		pointer->element->unqualified->kind != sema::TypeKind::Class;
	if (!stored.is_integer() && stored.kind() != lowir::TypeKind::Ptr)
	{
		const uint64_t size = lowering_.size_of(pointer->element, expr.location);
		stored = lowir::Type::scalar(size == 1 ? lowir::TypeKind::I8
				: size == 2                    ? lowir::TypeKind::I16
				: size == 4                    ? lowir::TypeKind::I32
											   : lowir::TypeKind::I64);
	}
	const bool is_pointer = stored.kind() == lowir::TypeKind::Ptr;
	const lowir::Type type = is_pointer ? lowir::Type::scalar(lowir::TypeKind::I64) : stored;
	const lowir::Value target = operand(*arguments[0], ConversionContext::Argument).value;
	// A value's bits as another type of its size (not `typed` accesses).
	const auto reinterpret = [&](const RValue& value, lowir::Type to) {
		Place bits = Place();
		bits.storage = lowir::Value::slot(generated_slot(lowir::SlotKind::Bits, value.type));
		bits.untyped = true;
		store(value.type, value, bits);
		return load(to, bits);
	};
	// A value operand (or, through a pointer, the generic forms'; a class
	// object's, a __c11_atomic builtin's, materialized and read as the
	// integer of its size; a floating one's bits).
	const auto value_operand = [&](uint32_t i, bool through_pointer) {
		RValue value;
		const sema::Type* operand_type = arguments[i]->facts->type;
		if (through_pointer)
		{
			Place place = Place();
			place.storage = operand(*arguments[i], ConversionContext::Argument).value;
			value = load(stored, place);
		}
		else if (!through_pointer && operand_type->unqualified->kind == sema::TypeKind::Class)
		{
			Place temporary = Place();
			temporary.storage = lowir::Value::slot(
				generated_slot(lowir::SlotKind::Argobj, lowering_.storage_type(operand_type->unqualified)));
			initialize_from(*arguments[i], arguments[i]->facts->conversion, address_of(temporary), expr.location);
			value = load(stored, temporary);
		}
		else
		{
			value = operand(*arguments[i], ConversionContext::Argument);
			if (reinterpreted)
				value = reinterpret(value, type);
		}
		return value.type.kind() == lowir::TypeKind::Ptr ? copy(type, value) : value;
	};
	// The object's bits as stored, and as the builtin's result.
	const auto stored_bits = [&](const RValue& value) { return is_pointer ? copy(stored, value) : value; };
	const auto result_of = [&](const RValue& value) {
		return reinterpreted ? reinterpret(value, object_type) : stored_bits(value);
	};
	const auto store_through = [&](uint32_t i, const RValue& value) {
		Place place = Place();
		place.storage = operand(*arguments[i], ConversionContext::Argument).value;
		store(stored, stored_bits(value), place);
	};
	switch (kind)
	{
	case Intrinsic::AtomicLoadN:
	case Intrinsic::AtomicLoad:
	{
		lowir::Instruction inst = lowir::make_instruction(lowir::Opcode::AtomicLoad, type);
		inst.ops[0] = target;
		const lowir::Value destination = kind == Intrinsic::AtomicLoad
			? operand(*arguments[1], ConversionContext::Argument).value
			: lowir::Value();
		inst.order = order_at(arguments.size - 1);
		RValue value;
		value.value = emit_value(inst, type);
		value.type = type;
		if (kind == Intrinsic::AtomicLoadN)
			return result_of(value);
		Place place = Place();
		place.storage = destination;
		store(stored, stored_bits(value), place);
		return RValue();
	}
	case Intrinsic::AtomicStoreN:
	case Intrinsic::AtomicStore:
	case Intrinsic::AtomicClear:
	{
		// __sync_lock_release stores 0.
		const RValue value = byte || (implied != 0 && arguments.size < 2)
			? integer(type, 0)
			: value_operand(1, kind == Intrinsic::AtomicStore);
		lowir::Instruction inst = lowir::make_instruction(lowir::Opcode::AtomicStore, type);
		inst.ops[0] = value.value;
		inst.ops[1] = target;
		inst.order = order_at(arguments.size - 1);
		emit(inst);
		return RValue();
	}
	case Intrinsic::AtomicExchangeN:
	case Intrinsic::AtomicExchange:
	case Intrinsic::AtomicTestAndSet:
	{
		const RValue value = byte ? integer(type, 1) : value_operand(1, kind == Intrinsic::AtomicExchange);
		lowir::Instruction inst = lowir::make_instruction(lowir::Opcode::AtomicExchange, type);
		inst.ops[0] = target;
		inst.ops[1] = value.value;
		inst.order = order_at(arguments.size - 1);
		RValue previous;
		previous.value = emit_value(inst, type);
		previous.type = type;
		if (kind == Intrinsic::AtomicTestAndSet)
			return compare(lowir::ComparePredicate::Ne, type, previous, integer(type, 0));
		if (kind == Intrinsic::AtomicExchangeN)
			return result_of(previous);
		store_through(2, previous);
		return RValue();
	}
	case Intrinsic::AtomicCompareExchangeN:
	case Intrinsic::AtomicCompareExchange:
	{
		const lowir::Value expected = operand(*arguments[1], ConversionContext::Argument).value;
		const RValue desired = value_operand(2, kind == Intrinsic::AtomicCompareExchange);
		operand(*arguments[3], ConversionContext::Argument);
		lowir::Instruction inst = lowir::make_instruction(lowir::Opcode::AtomicCompareExchange, type);
		inst.ops[0] = target;
		inst.ops[1] = expected;
		inst.ops[2] = desired.value;
		inst.order = memory_order(*arguments[4]);
		inst.sub = static_cast<uint8_t>(memory_order(*arguments[5]));
		RValue result;
		result.type = lowir::Type::scalar(lowir::TypeKind::I64);
		result.value = emit_value(inst, result.type);
		return result;
	}
	case Intrinsic::SyncBoolCompareSwap:
	case Intrinsic::SyncValCompareSwap:
	{
		// A compare-exchange of `old` (held in a slot, which a failure
		// overwrites with what *p held): whether it succeeded, or the
		// slot's value -- what *p held either way.
		Place expected = Place();
		expected.storage = lowir::Value::slot(generated_slot(lowir::SlotKind::AtomicExpected, type));
		store(type, value_operand(1, false), expected);
		const RValue desired = value_operand(2, false);
		lowir::Instruction inst = lowir::make_instruction(lowir::Opcode::AtomicCompareExchange, type);
		inst.ops[0] = target;
		inst.ops[1] = address_of(expected);
		inst.ops[2] = desired.value;
		inst.order = lowir::MemoryOrder::SeqCst;
		inst.sub = static_cast<uint8_t>(lowir::MemoryOrder::SeqCst);
		RValue result;
		result.type = lowir::Type::scalar(lowir::TypeKind::I64);
		result.value = emit_value(inst, result.type);
		if (kind == Intrinsic::SyncBoolCompareSwap)
			return result;
		return result_of(load(type, expected));
	}
	default:
		break;
	}
	RValue value = value_operand(1, false);
	// __c11_atomic_fetch_add/sub on an _Atomic(T*) (GCC's builtins take no
	// _Atomic object) add or subtract that many T's.
	const sema::Type* pointee = is_pointer ? pointer->element->unqualified->element->unqualified : nullptr;
	const bool additive = kind == Intrinsic::AtomicFetchAdd || kind == Intrinsic::AtomicFetchSub ||
		kind == Intrinsic::AtomicAddFetch || kind == Intrinsic::AtomicSubFetch;
	if (additive && pointee != nullptr && (pointer->element->cv & sema::kAtomic) != 0 &&
		pointee->kind != sema::TypeKind::Function &&
		!(pointee->kind == sema::TypeKind::Fundamental && pointee->fundamental == FundamentalType::Void))
		value = binary(lowir::BinaryOp::Mul, type, value, integer(type, lowering_.size_of(pointee, expr.location)));
	return result_of(atomic_update(kind, type, target, value, order_at(2)));
}

RValue FunctionLowering::atomic_update(
	sema::Intrinsic kind, lowir::Type type, lowir::Value target, const RValue& value, lowir::MemoryOrder order)
{
	// `__atomic_<op>_fetch` (the new value) and `__atomic_fetch_<op>` (the
	// old). A subtraction adds the negated value; the bitwise updates retry
	// a compare-exchange of the value last seen (a failed exchange writes
	// the current value back into the expected slot) until it succeeds.
	using sema::Intrinsic;
	const bool fetch_first = kind >= Intrinsic::AtomicFetchAdd;
	const Intrinsic op = fetch_first
		? static_cast<Intrinsic>(static_cast<uint8_t>(kind) -
			  (static_cast<uint8_t>(Intrinsic::AtomicFetchAdd) - static_cast<uint8_t>(Intrinsic::AtomicAddFetch)))
		: kind;
	if (op == Intrinsic::AtomicAddFetch || op == Intrinsic::AtomicSubFetch)
	{
		const RValue delta = op == Intrinsic::AtomicAddFetch ? value : unary(lowir::UnaryOp::Neg, value);
		lowir::Instruction inst = lowir::make_instruction(lowir::Opcode::AtomicAddFetch, type);
		inst.ops[0] = target;
		inst.ops[1] = delta.value;
		inst.order = order;
		RValue updated;
		updated.value = emit_value(inst, type);
		updated.type = type;
		return fetch_first ? binary(lowir::BinaryOp::Sub, type, updated, delta) : updated;
	}
	Place expected = Place();
	expected.storage = lowir::Value::slot(generated_slot(lowir::SlotKind::AtomicExpected, type));
	lowir::Instruction first = lowir::make_instruction(lowir::Opcode::AtomicLoad, type);
	first.ops[0] = target;
	first.order = lowir::MemoryOrder::Relaxed;
	RValue seen;
	seen.value = emit_value(first, type);
	seen.type = type;
	store(type, seen, expected);
	const lowir::BlockId retry = new_block(lowir::BlockKind::AtomicRetry);
	const lowir::BlockId done = new_block(lowir::BlockKind::AtomicDone);
	jump(retry);
	place(retry);
	const RValue previous = load(type, expected);
	const lowir::BinaryOp bitwise = op == Intrinsic::AtomicOrFetch ? lowir::BinaryOp::Or
		: op == Intrinsic::AtomicXorFetch                          ? lowir::BinaryOp::Xor
																   : lowir::BinaryOp::And;
	RValue updated = binary(bitwise, type, previous, value);
	if (op == Intrinsic::AtomicNandFetch)
		updated = unary(lowir::UnaryOp::BitNot, updated);
	lowir::Instruction exchange = lowir::make_instruction(lowir::Opcode::AtomicCompareExchange, type);
	exchange.ops[0] = target;
	exchange.ops[1] = address_of(expected);
	exchange.ops[2] = updated.value;
	exchange.order = order;
	// A failure only reads: no release part (C++11 29.6.5p20).
	exchange.sub = static_cast<uint8_t>(order == lowir::MemoryOrder::Release ? lowir::MemoryOrder::Relaxed
			: order == lowir::MemoryOrder::AcqRel                            ? lowir::MemoryOrder::Acquire
																			 : order);
	RValue exchanged;
	exchanged.type = lowir::Type::scalar(lowir::TypeKind::I64);
	exchanged.value = emit_value(exchange, exchanged.type);
	branch(exchanged, done, retry);
	place(done);
	return fetch_first ? previous : updated;
}

RValue FunctionLowering::overflow_intrinsic(const syntax::CallExpr& expr, sema::Intrinsic kind)
{
	// a op b computed exactly and stored through r modulo r's type; the
	// result whether that changed it. One type for all three: the
	// operation in it, its overflow read from the operands and the wrapped
	// result (a 32-bit multiplication in 64 bits); otherwise in 128 bits,
	// where sums and differences of 64-bit operands are exact, and
	// products too (as unsigned when both operands are unsigned), the
	// result then checked against r's type's range.
	const sema::Type* left_type = expr.arguments[0]->facts->conversion != nullptr
		? expr.arguments[0]->facts->conversion->result
		: expr.arguments[0]->facts->type;
	const sema::Type* right_type = expr.arguments[1]->facts->conversion != nullptr
		? expr.arguments[1]->facts->conversion->result
		: expr.arguments[1]->facts->type;
	const RValue a = operand(*expr.arguments[0], ConversionContext::Argument);
	const RValue b = operand(*expr.arguments[1], ConversionContext::Argument);
	const RValue pointer = operand(*expr.arguments[2], ConversionContext::Argument);
	const sema::Type* result_type = expr.arguments[2]->facts->type->unqualified;
	result_type = result_type->element->unqualified;
	sema::IntegralKind left_kind, right_kind, result_kind;
	sema::IntegralKindOf(left_type, left_kind);
	sema::IntegralKindOf(right_type, right_kind);
	sema::IntegralKindOf(result_type, result_kind);
	const lowir::Type stored = lowering_.storage_type(result_type);
	const lowir::Type i64 = lowir::Type::scalar(lowir::TypeKind::I64);
	Place target = Place();
	target.storage = pointer.value;
	const bool same = left_type->unqualified == result_type && right_type->unqualified == result_type;
	if (same && (kind != sema::Intrinsic::MulOverflow || result_kind.bits == 32) && result_kind.bits >= 32)
	{
		const lowir::Type type = a.type;
		if (kind == sema::Intrinsic::MulOverflow)
		{
			const lowir::ConvertOp widen = result_kind.is_signed ? lowir::ConvertOp::SExt : lowir::ConvertOp::ZExt;
			// (Operands emitted last first: see FunctionLowering::binary.)
			const RValue wide_b = convert_op(widen, i64, b);
			const RValue wide = binary(lowir::BinaryOp::Mul, i64, convert_op(widen, i64, a), wide_b);
			const RValue narrow = convert_op(lowir::ConvertOp::Trunc, type, wide);
			store(stored, narrow, target);
			return compare(lowir::ComparePredicate::Ne, i64, wide, convert_op(widen, i64, narrow));
		}
		const bool add = kind == sema::Intrinsic::AddOverflow;
		const RValue r = binary(add ? lowir::BinaryOp::Add : lowir::BinaryOp::Sub, type, a, b);
		store(stored, r, target);
		if (!result_kind.is_signed)
			return add ? compare(lowir::ComparePredicate::Ult, type, r, a)
					   : compare(lowir::ComparePredicate::Ult, type, a, b);
		// Signed: the operands' signs agree (a sum) or differ (a
		// difference) and the result's differs from a's.
		const RValue result_sign = binary(lowir::BinaryOp::Xor, type, add ? b : a, r);
		const RValue signs = binary(lowir::BinaryOp::And, type,
			add ? binary(lowir::BinaryOp::Xor, type, a, r) : binary(lowir::BinaryOp::Xor, type, a, b), result_sign);
		return compare(lowir::ComparePredicate::Lt, type, signs, integer(type, 0));
	}
	if (left_kind.bits > 64 || right_kind.bits > 64 || result_kind.bits > 64)
		return wide_overflow(kind, a, left_kind, b, right_kind, target, stored, result_kind);
	const lowir::Type i128 = lowir::Type::scalar(lowir::TypeKind::I128);
	const auto widen = [&](const RValue& value, const sema::IntegralKind& kind) {
		return convert_op(kind.is_signed ? lowir::ConvertOp::SExt : lowir::ConvertOp::ZExt, i128, value);
	};
	const lowir::BinaryOp op = kind == sema::Intrinsic::AddOverflow ? lowir::BinaryOp::Add
		: kind == sema::Intrinsic::SubOverflow                      ? lowir::BinaryOp::Sub
																	: lowir::BinaryOp::Mul;
	const RValue wide_b = widen(b, right_kind);
	const RValue r = binary(op, i128, widen(a, left_kind), wide_b);
	store(stored, convert_op(lowir::ConvertOp::Trunc, stored, r), target);
	const uint32_t bits = result_kind.bits;
	const uint64_t max = result_kind.is_signed ? (uint64_t(1) << (bits - 1)) - 1
		: bits == 64                           ? ~uint64_t(0)
											   : (uint64_t(1) << bits) - 1;
	const RValue high = convert_op(lowir::ConvertOp::ZExt, i128, integer(i64, max));
	if (kind == sema::Intrinsic::MulOverflow && !left_kind.is_signed && !right_kind.is_signed)
		return compare(lowir::ComparePredicate::Ugt, i128, r, high);
	const RValue low = result_kind.is_signed ? convert_op(lowir::ConvertOp::SExt, i128, integer(i64, ~max))
											 : integer(i128, 0);
	const RValue below = compare(lowir::ComparePredicate::Lt, i128, r, low);
	return binary(lowir::BinaryOp::Or, i64, compare(lowir::ComparePredicate::Gt, i128, r, high), below);
}

RValue FunctionLowering::wide_overflow(sema::Intrinsic kind, const RValue& a, const sema::IntegralKind& left_kind,
	const RValue& b, const sema::IntegralKind& right_kind, const Place& target, lowir::Type stored,
	const sema::IntegralKind& result_kind)
{
	// A 128-bit operand or result: each operand as a sign and a 128-bit
	// magnitude, the exact result likewise with a carry (a magnitude of
	// 2^128 or more, beyond every result type), as the constant
	// interpreter computes it. Choices are masks: sel(c, x, y) is
	// y ^ ((x ^ y) & -c) for a truth c.
	const lowir::Type i128 = lowir::Type::scalar(lowir::TypeKind::I128);
	const RValue zero = integer(i128, lowir::Literal{ 0, 0 });
	const RValue one = integer(i128, lowir::Literal{ 1, 0 });
	const auto truth = [&](const RValue& value) { return convert_op(lowir::ConvertOp::ZExt, i128, value); };
	const auto select = [&](const RValue& c, const RValue& x, const RValue& y) {
		const RValue mask = binary(lowir::BinaryOp::Sub, i128, zero, c);
		return binary(lowir::BinaryOp::Xor, i128, y,
			binary(lowir::BinaryOp::And, i128, binary(lowir::BinaryOp::Xor, i128, x, y), mask));
	};
	const auto magnitude = [&](const RValue& value, const sema::IntegralKind& operand, RValue& negative) {
		const RValue wide = operand.bits == 128
			? value
			: convert_op(operand.is_signed ? lowir::ConvertOp::SExt : lowir::ConvertOp::ZExt, i128, value);
		if (!operand.is_signed)
		{
			negative = zero;
			return wide;
		}
		negative = truth(compare(lowir::ComparePredicate::Lt, i128, wide, zero));
		const RValue mask = binary(lowir::BinaryOp::Sub, i128, zero, negative);
		return binary(lowir::BinaryOp::Sub, i128, binary(lowir::BinaryOp::Xor, i128, wide, mask), mask);
	};
	RValue negative_a, negative_b;
	const RValue magnitude_a = magnitude(a, left_kind, negative_a);
	const RValue magnitude_b = magnitude(b, right_kind, negative_b);
	RValue negative, result, carry;
	if (kind == sema::Intrinsic::MulOverflow)
	{
		// By 64-bit halves: with both high halves set the product is past
		// 2^128; else one cross product, which must fit 64 bits.
		negative = binary(lowir::BinaryOp::Xor, i128, negative_a, negative_b);
		const RValue shift = integer(i128, lowir::Literal{ 64, 0 });
		const RValue low_mask = integer(i128, lowir::Literal{ ~uint64_t(0), 0 });
		const RValue a1 = binary(lowir::BinaryOp::UShr, i128, magnitude_a, shift);
		const RValue a0 = binary(lowir::BinaryOp::And, i128, magnitude_a, low_mask);
		const RValue b1 = binary(lowir::BinaryOp::UShr, i128, magnitude_b, shift);
		const RValue b0 = binary(lowir::BinaryOp::And, i128, magnitude_b, low_mask);
		// (Operands emitted last first: see FunctionLowering::binary.)
		const RValue b_high = truth(compare(lowir::ComparePredicate::Ne, i128, b1, zero));
		const RValue both_high = binary(
			lowir::BinaryOp::And, i128, truth(compare(lowir::ComparePredicate::Ne, i128, a1, zero)), b_high);
		const RValue a0_b1 = binary(lowir::BinaryOp::Mul, i128, a0, b1);
		const RValue cross = binary(lowir::BinaryOp::Add, i128, binary(lowir::BinaryOp::Mul, i128, a1, b0), a0_b1);
		const RValue cross_high = truth(
			compare(lowir::ComparePredicate::Ne, i128, binary(lowir::BinaryOp::UShr, i128, cross, shift), zero));
		const RValue low = binary(lowir::BinaryOp::Mul, i128, a0, b0);
		result = binary(lowir::BinaryOp::Add, i128, low, binary(lowir::BinaryOp::Shl, i128, cross, shift));
		const RValue low_carry = truth(compare(lowir::ComparePredicate::Ult, i128, result, low));
		carry = binary(lowir::BinaryOp::Or, i128, binary(lowir::BinaryOp::Or, i128, both_high, cross_high), low_carry);
	}
	else
	{
		// A difference adds b of the other sign; signs alike add the
		// magnitudes (with a carry), unlike subtract the smaller.
		const RValue sign_b = kind == sema::Intrinsic::SubOverflow ? binary(lowir::BinaryOp::Xor, i128, negative_b, one)
																   : negative_b;
		const RValue alike = truth(compare(lowir::ComparePredicate::Eq, i128, negative_a, sign_b));
		const RValue sum = binary(lowir::BinaryOp::Add, i128, magnitude_a, magnitude_b);
		const RValue a_larger = truth(compare(lowir::ComparePredicate::Uge, i128, magnitude_a, magnitude_b));
		const RValue b_minus_a = binary(lowir::BinaryOp::Sub, i128, magnitude_b, magnitude_a);
		const RValue difference = select(
			a_larger, binary(lowir::BinaryOp::Sub, i128, magnitude_a, magnitude_b), b_minus_a);
		result = select(alike, sum, difference);
		negative = select(alike, negative_a, select(a_larger, negative_a, sign_b));
		carry = binary(
			lowir::BinaryOp::And, i128, alike, truth(compare(lowir::ComparePredicate::Ult, i128, sum, magnitude_a)));
	}
	// Stored modulo 2^N; representable when below 2^(N-1) (2^N unsigned)
	// in magnitude, or 2^(N-1) itself negative and signed, never negative
	// and unsigned (but zero).
	const RValue mask = binary(lowir::BinaryOp::Sub, i128, zero, negative);
	const RValue value = binary(lowir::BinaryOp::Sub, i128, binary(lowir::BinaryOp::Xor, i128, result, mask), mask);
	store(stored, result_kind.bits == 128 ? value : convert_op(lowir::ConvertOp::Trunc, stored, value), target);
	const unsigned bits = result_kind.bits - (result_kind.is_signed ? 1 : 0);
	const lowir::Literal limit_bits = bits >= 128 ? lowir::Literal{ ~uint64_t(0), ~uint64_t(0) }
		: bits >= 64                              ? lowir::Literal{ ~uint64_t(0), (uint64_t(1) << (bits - 64)) - 1 }
												  : lowir::Literal{ (uint64_t(1) << bits) - 1, 0 };
	const RValue limit = integer(i128, limit_bits);
	RValue fits = truth(compare(lowir::ComparePredicate::Ule, i128, result, limit));
	if (result_kind.is_signed)
		fits = binary(lowir::BinaryOp::Or, i128, fits,
			binary(lowir::BinaryOp::And, i128, negative,
				truth(compare(
					lowir::ComparePredicate::Eq, i128, result, binary(lowir::BinaryOp::Add, i128, limit, one)))));
	else
	{
		const RValue is_zero = truth(compare(lowir::ComparePredicate::Eq, i128, result, zero));
		fits = binary(lowir::BinaryOp::And, i128, fits,
			binary(lowir::BinaryOp::Or, i128, binary(lowir::BinaryOp::Xor, i128, negative, one), is_zero));
	}
	fits = binary(lowir::BinaryOp::And, i128, fits, binary(lowir::BinaryOp::Xor, i128, carry, one));
	return compare(lowir::ComparePredicate::Eq, i128, fits, zero);
}

RValue FunctionLowering::sign_bit(const RValue& x)
{
	// The operand's representation through memory: its sign is the top
	// bit of a float's or double's last bytes (an x87 value's bytes 8-9; a
	// binary16's bytes 0-1, a binary128's 8-15).
	const lowir::Type type = x.type;
	Place value = Place();
	value.storage = lowir::Value::slot(generated_slot(lowir::SlotKind::FloatingSignbit, type));
	store(type, x, value);
	const SoftFloat soft = SoftFloatOfValue(type);
	const bool extended = type.kind() == lowir::TypeKind::F80;
	const lowir::Type bits = lowir::Type::scalar(type.kind() == lowir::TypeKind::F32 ? lowir::TypeKind::I32
			: extended || soft == SoftFloat::Half                                    ? lowir::TypeKind::I16
																					 : lowir::TypeKind::I64);
	Place word = Place();
	word.storage = extended       ? element_address(address_of(value), 4, 2)
		: soft == SoftFloat::Quad ? element_address(address_of(value), 1, 8)
								  : address_of(value);
	return truth_i32(compare(lowir::ComparePredicate::Lt, bits, load(bits, word), integer(bits, 0)));
}

RValue FunctionLowering::quiet_bit(const RValue& x)
{
	// The top fraction bit through memory, an i32: a float's bit 22, a
	// double's 51, an x87 value's significand bit 62, a binary16's bit 9,
	// a binary128's bit 111 (its last eight bytes' 47).
	const lowir::Type type = x.type;
	Place value = Place();
	value.storage = lowir::Value::slot(generated_slot(lowir::SlotKind::FloatingSignbit, type));
	store(type, x, value);
	const SoftFloat soft = SoftFloatOfValue(type);
	const lowir::Type bits = lowir::Type::scalar(type.kind() == lowir::TypeKind::F32 ? lowir::TypeKind::I32
			: soft == SoftFloat::Half                                                ? lowir::TypeKind::I16
																					 : lowir::TypeKind::I64);
	const unsigned position = type.kind() == lowir::TypeKind::F32 ? 22
		: type.kind() == lowir::TypeKind::F64                     ? 51
		: type.kind() == lowir::TypeKind::F80                     ? 62
		: soft == SoftFloat::Half                                 ? 9
																  : 47;
	Place word = Place();
	word.storage = soft == SoftFloat::Quad ? element_address(address_of(value), 1, 8) : address_of(value);
	const RValue bit = binary(lowir::BinaryOp::And, bits,
		binary(lowir::BinaryOp::UShr, bits, load(bits, word), integer(bits, position)), integer(bits, 1));
	const lowir::Type i32 = lowir::Type::scalar(lowir::TypeKind::I32);
	return bits.kind() == lowir::TypeKind::I32
		? bit
		: convert_op(bits.kind() == lowir::TypeKind::I16 ? lowir::ConvertOp::ZExt : lowir::ConvertOp::Trunc, i32, bit);
}

RValue FunctionLowering::flt_rounds()
{
	// FLT_ROUNDS of the C library's fegetround(): its x87/SSE rounding
	// control (bits 10-11: nearest, down, up, toward zero) maps to 1, 3,
	// 2, 0 -- (45 >> ((mode >> 9) & 6)) & 3.
	const lowir::Type i32 = lowir::Type::scalar(lowir::TypeKind::I32);
	RValue mode;
	mode.type = i32;
	mode.value = emit_call(
		i32, lowir::Value::symbol(lowering_.fegetround_function()), arguments_.size(), lowir::kNoId, false);
	const RValue shift = binary(
		lowir::BinaryOp::And, i32, binary(lowir::BinaryOp::Shr, i32, mode, integer(i32, 9)), integer(i32, 6));
	return binary(
		lowir::BinaryOp::And, i32, binary(lowir::BinaryOp::Shr, i32, integer(i32, 45), shift), integer(i32, 3));
}

lowir::SymbolId ProgramLowering::fegetround_function()
{
	// The C library's `int fegetround(void)` (C99 7.6.3.1), declared once.
	if (fegetround_ != lowir::kNoId)
		return fegetround_;
	fegetround_ = new_symbol("__cppgm_libc_fegetround");
	lowir::Signature signature = lowir::Signature();
	signature.first_parameter = static_cast<uint32_t>(program_.parameters.size());
	signature.result = lowir::Type::scalar(lowir::TypeKind::I32);
	program_.declare_function(fegetround_, lowir::SymbolKind::FunctionDeclaration, program_.add_signature(signature));
	lowir::SymbolMetadata& metadata = program_.symbols[fegetround_].metadata;
	metadata.linkage = ir_model::LLM_C;
	metadata.binding = ir_model::SBM_STRONG;
	metadata.object = intern("fegetround", 10);
	return fegetround_;
}

RValue FunctionLowering::bfloat_float(const syntax::Expr& expr, const RValue& value)
{
	// A bfloat16 operand of a floating classification or quiet comparison
	// as the float holding it (its class, sign and quiet bit kept), which
	// the float forms classify and compare.
	const syntax::Expr* e = &expr;
	while (e->kind == syntax::ExprKind::Paren)
		e = static_cast<const syntax::ParenExpr*>(e)->inner;
	const sema::Conversion* conversion = e->facts->conversion;
	const sema::Type* type = conversion != nullptr && conversion->constructor == nullptr &&
			conversion->form != sema::ConversionForm::Reference && conversion->result != nullptr
		? conversion->result
		: e->facts->type;
	return value.type.is_object() && SoftFloatOf(type) == SoftFloat::BFloat ? bfloat_widened(value) : value;
}

lowir::SymbolId ProgramLowering::trap_function()
{
	// `void cppgm_builtin_trap(void)`, declared once: a call the optimizer
	// keeps (an `unreachable` alone it may assume is never reached).
	if (trap_ != lowir::kNoId)
		return trap_;
	trap_ = new_symbol("__cppgm_builtin_trap");
	lowir::Signature signature = lowir::Signature();
	signature.first_parameter = static_cast<uint32_t>(program_.parameters.size());
	signature.result = lowir::Type::scalar(lowir::TypeKind::Void);
	signature.boundary.unwind = ir_model::CUM_NO;
	signature.boundary.returns = ir_model::CRM_NORETURN;
	program_.declare_function(trap_, lowir::SymbolKind::FunctionDeclaration, program_.add_signature(signature));
	lowir::SymbolMetadata& metadata = program_.symbols[trap_].metadata;
	metadata.binding = ir_model::SBM_STRONG;
	metadata.object = intern("cppgm_builtin_trap", 18);
	return trap_;
}

lowir::SymbolId ProgramLowering::memset_function()
{
	// `void* memset(void*, int, size_t)` (C 7.24.6.1) under the name the
	// optimizer's fill loops give it (opt/loop_passes.cpp), declared once.
	if (memset_ != lowir::kNoId)
		return memset_;
	memset_ = new_symbol("__cppgm_builtin_memset");
	lowir::Signature signature = lowir::Signature();
	signature.first_parameter = static_cast<uint32_t>(program_.parameters.size());
	const lowir::Type types[3] = { lowir::Type::scalar(lowir::TypeKind::Ptr), lowir::Type::scalar(lowir::TypeKind::I32),
		lowir::Type::scalar(lowir::TypeKind::I64) };
	for (uint32_t i = 0; i < 3; ++i)
	{
		lowir::Parameter parameter = lowir::Parameter();
		parameter.name = argument_name(i);
		parameter.type = types[i];
		program_.parameters.push_back(parameter);
	}
	signature.parameter_count = 3;
	signature.result = types[0];
	signature.boundary.unwind = ir_model::CUM_NO;
	program_.declare_function(memset_, lowir::SymbolKind::FunctionDeclaration, program_.add_signature(signature));
	lowir::SymbolMetadata& metadata = program_.symbols[memset_].metadata;
	metadata.binding = ir_model::SBM_STRONG;
	metadata.object = intern("cppgm_builtin_memset", 20);
	return memset_;
}

RValue FunctionLowering::truth_i32(const RValue& truth)
{
	return convert_op(lowir::ConvertOp::Trunc, lowir::Type::scalar(lowir::TypeKind::I32), truth);
}

RValue FunctionLowering::classify_nan(const RValue& x)
{
	const SoftFloat soft = SoftFloatOfValue(x.type);
	if (soft != SoftFloat::None)
		return truth_i32(soft_compare(lowir::ComparePredicate::Ne, soft, x, x));
	return truth_i32(compare(lowir::ComparePredicate::Ne, x.type, x, x));
}

RValue FunctionLowering::classify_infinite(const RValue& x)
{
	// A binary16 or binary128 value: |x| at least its infinity.
	const SoftFloat soft = SoftFloatOfValue(x.type);
	if (soft != SoftFloat::None)
		return soft_magnitude_at_least(soft, x, 0x7c00, 0x7fff000000000000ull);
	const RValue positive = floating(
		x.type, lowir::float_literal(x.type.kind(), std::numeric_limits<long double>::infinity()));
	const RValue negative = unary(lowir::UnaryOp::Neg, positive);
	const RValue is_positive = truth_i32(compare(lowir::ComparePredicate::Eq, x.type, x, positive));
	const RValue is_negative = truth_i32(compare(lowir::ComparePredicate::Eq, x.type, x, negative));
	return binary(lowir::BinaryOp::Or, lowir::Type::scalar(lowir::TypeKind::I32), is_positive, is_negative);
}

RValue FunctionLowering::classify_finite(const RValue& x)
{
	const lowir::Type i32 = lowir::Type::scalar(lowir::TypeKind::I32);
	const RValue not_nan = binary(lowir::BinaryOp::Sub, i32, integer(i32, 1), classify_nan(x));
	const RValue not_infinite = binary(lowir::BinaryOp::Sub, i32, integer(i32, 1), classify_infinite(x));
	return binary(lowir::BinaryOp::And, i32, not_nan, not_infinite);
}

RValue FunctionLowering::classify_normal(const RValue& x)
{
	// |x| >= the least normal value of the operand's type, and finite.
	const SoftFloat soft = SoftFloatOfValue(x.type);
	if (soft != SoftFloat::None)
	{
		const RValue large = soft_magnitude_at_least(soft, x, 0x0400, 0x0001000000000000ull);
		return binary(lowir::BinaryOp::And, lowir::Type::scalar(lowir::TypeKind::I32), classify_finite(x), large);
	}
	long double least = std::numeric_limits<long double>::min();
	if (x.type.kind() == lowir::TypeKind::F64)
		least = std::numeric_limits<double>::min();
	else if (x.type.kind() == lowir::TypeKind::F32)
		least = std::numeric_limits<float>::min();
	const lowir::Type i32 = lowir::Type::scalar(lowir::TypeKind::I32);
	const RValue positive = floating(x.type, lowir::float_literal(x.type.kind(), least));
	const RValue negative = unary(lowir::UnaryOp::Neg, positive);
	const RValue above = truth_i32(compare(lowir::ComparePredicate::Ge, x.type, x, positive));
	const RValue below = truth_i32(compare(lowir::ComparePredicate::Le, x.type, x, negative));
	const RValue large = binary(lowir::BinaryOp::Or, i32, above, below);
	return binary(lowir::BinaryOp::And, i32, classify_finite(x), large);
}

// The atomic read-modify-writes of `_Atomic` objects' compound assignments
// and increments (expression_lowering.cpp; C11 6.5.16.2p3, 6.5.2.4p2).

bool FunctionLowering::atomic_step(const Place& place, const sema::Type* type, lowir::Type storage)
{
	// ++ and -- of an atomic integer (not bool): one atomic
	// read-modify-write (C11 6.5.2.4p2).
	const sema::Type* value = type->unqualified;
	return place.is_atomic && storage.is_integer() &&
		!(value->kind == sema::TypeKind::Fundamental && value->fundamental == FundamentalType::Bool);
}

bool FunctionLowering::atomic_bits(lowir::Type storage, lowir::Type& bits)
{
	// An integer is its own bits; a pointer or floating value (a long
	// double's 16 bytes with their padding), a binary16 or binary128
	// object, those of the integer of its size.
	uint64_t size = 0;
	switch (storage.kind())
	{
	case lowir::TypeKind::F32:
		size = 4;
		break;
	case lowir::TypeKind::F64:
	case lowir::TypeKind::Ptr:
		size = 8;
		break;
	case lowir::TypeKind::F80:
		size = 16;
		break;
	case lowir::TypeKind::Obj:
		size = storage.bytes;
		break;
	default:
		bits = storage;
		return storage.is_integer();
	}
	if (size != 1 && size != 2 && size != 4 && size != 8 && size != 16)
		return false;
	bits = lowir::Type::scalar(size == 1 ? lowir::TypeKind::I8
			: size == 2                  ? lowir::TypeKind::I16
			: size == 4                  ? lowir::TypeKind::I32
			: size == 8                  ? lowir::TypeKind::I64
										 : lowir::TypeKind::I128);
	return true;
}

RValue FunctionLowering::begin_atomic_loop(const Place& object, lowir::Type storage, AtomicLoop& loop)
{
	// The object's bits read into the expected slot (a failed exchange
	// writes what the object held there), then each try reads them as the
	// object's type.
	loop.expected = generated_slot(lowir::SlotKind::AtomicExpected, loop.bits);
	Place expected = Place();
	expected.storage = lowir::Value::slot(loop.expected);
	lowir::Instruction first = lowir::make_instruction(lowir::Opcode::AtomicLoad, loop.bits);
	first.ops[0] = object.storage;
	first.order = lowir::MemoryOrder::Relaxed;
	RValue seen;
	seen.value = emit_value(first, loop.bits);
	seen.type = loop.bits;
	store(loop.bits, seen, expected);
	loop.retry = new_block(lowir::BlockKind::AtomicRetry);
	jump(loop.retry);
	place(loop.retry);
	if (storage == loop.bits)
		return load(storage, expected);
	if (storage.kind() == lowir::TypeKind::Ptr)
		return copy(storage, load(loop.bits, expected));
	Place view = Place();
	view.storage = address_of(expected);
	return load(storage, view);
}

void FunctionLowering::end_atomic_loop(
	const Place& object, lowir::Type storage, const AtomicLoop& loop, const RValue& updated)
{
	// The new value's bits (over a copy of the old ones: a long double's
	// padding unchanged), exchanged for the expected ones, else retried.
	Place expected = Place();
	expected.storage = lowir::Value::slot(loop.expected);
	RValue desired = updated;
	if (storage.kind() == lowir::TypeKind::Ptr)
		desired = copy(loop.bits, updated);
	else if (storage != loop.bits)
	{
		Place fresh = Place();
		fresh.storage = lowir::Value::slot(generated_slot(lowir::SlotKind::AtomicExpected, loop.bits));
		store(loop.bits, load(loop.bits, expected), fresh);
		Place view = Place();
		view.storage = address_of(fresh);
		store(storage, updated, view);
		desired = load(loop.bits, fresh);
	}
	lowir::Instruction exchange = lowir::make_instruction(lowir::Opcode::AtomicCompareExchange, loop.bits);
	exchange.ops[0] = object.storage;
	exchange.ops[1] = address_of(expected);
	exchange.ops[2] = desired.value;
	exchange.order = lowir::MemoryOrder::SeqCst;
	exchange.sub = static_cast<uint8_t>(lowir::MemoryOrder::SeqCst);
	RValue exchanged;
	exchanged.type = lowir::Type::scalar(lowir::TypeKind::I64);
	exchanged.value = emit_value(exchange, exchanged.type);
	const lowir::BlockId done = new_block(lowir::BlockKind::AtomicDone);
	branch(exchanged, done, loop.retry);
	place(done);
}

} // namespace lower
