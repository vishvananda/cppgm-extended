// GNU complex values (`_Complex T`): an object of the two parts' bytes,
// real then imaginary (LowIR has no complex type), built by
// __builtin_complex or a list of the parts, read and written part by part
// through __real__ and __imag__, and copied, passed and returned whole. At
// a host object's call boundary a complex float or double is its two
// fields' classes (psABI 3.2.3), a complex long double COMPLEX_X87.
// Arithmetic (C99 6.5.5-6.5.9 as GCC computes it) splits a value into its
// parts in a slot and joins the results again: + - and a real operand's
// products and quotients part by part (G.5.1); a product of two complex
// values (ac - bd, ad + bc), recomputed by libgcc's __mul?c3 when both
// parts are NaN (Annex G's recovery, as GCC and Clang call it at -O0;
// always for binary16 and binary128 parts, which are calls anyway); a
// floating quotient by __div?c3, an integral one by Smith's formula (GCC's
// "wide" division). Conversions go part by part (a real value's imaginary
// part zero); a value is true unless both parts are zero.

#include "lower/function_lowering.h"

#include <cstring>
#include <string>

#include "sema/conversions.h"
#include "sema/expression_facts.h"
#include "sema/integral_arithmetic.h"

namespace lower {

namespace {

bool SignedPart(const sema::Type* part)
{
	sema::IntegralKind kind;
	return sema::IntegralKindOf(part, kind) && kind.is_signed;
}

} // namespace

bool ObjectValueType(const sema::Type* type)
{
	return SoftFloatOf(type) != SoftFloat::None || type->unqualified->kind == sema::TypeKind::Complex ||
		type->unqualified->kind == sema::TypeKind::Vector;
}

uint8_t ProgramLowering::complex_classes(const sema::Type* type)
{
	if (!object_mode_)
		return 0;
	const sema::Type* part = type->unqualified->element->unqualified;
	if (part->kind == sema::TypeKind::Fundamental &&
		(part->fundamental == FundamentalType::LongDouble || part->fundamental == FundamentalType::Float64x))
		return lowir::OC_STATED | lowir::OC_COMPLEX_X87;
	const uint64_t bytes = size_of(part, 0);
	if (2 * bytes > 16)
		return lowir::OC_STATED | lowir::OC_MEMORY;
	uint8_t classes[2] = { lowir::EB_NONE, lowir::EB_NONE };
	classify_eightbytes(part, 0, classes);
	classify_eightbytes(part, bytes, classes);
	return static_cast<uint8_t>(lowir::OC_STATED | classes[0] | (classes[1] << 2));
}

RValue FunctionLowering::zero_object_value(lowir::Type type)
{
	if (type.is_vector())
	{
		lowir::Instruction zero = lowir::make_instruction(lowir::Opcode::Vector, type);
		zero.sub = static_cast<uint8_t>(lowir::VectorOp::Pzero);
		RValue value = RValue();
		value.type = type;
		value.value = emit_value(zero, type);
		return value;
	}
	Place storage = Place();
	storage.storage = lowir::Value::slot(generated_slot(lowir::SlotKind::Complex, type));
	lowir::Instruction zero = lowir::make_instruction(
		lowir::Opcode::ZeroInit, lowir::Type::scalar(lowir::TypeKind::Void));
	zero.ops[0] = storage.storage;
	zero.immediate = static_cast<int64_t>(type.bytes);
	zero.sub = static_cast<uint8_t>(type.align_log2);
	emit(zero);
	return load(type, storage);
}

lowir::SymbolId ProgramLowering::complex_helper(bool divide, const sema::Type* part)
{
	// libgcc's __mul?c3 and __div?c3 (sc float, dc double, xc long double,
	// hc binary16, tc binary128): the four parts in, the complex result out,
	// declared as the C functions they are on first reference.
	const sema::Type* bare = part->unqualified;
	const SoftFloat soft = SoftFloatOf(bare);
	const lowir::Type storage = storage_type(bare);
	const char mode = soft == SoftFloat::Half    ? 'h'
		: soft == SoftFloat::Quad                ? 't'
		: storage.kind() == lowir::TypeKind::F32 ? 's'
		: storage.kind() == lowir::TypeKind::F64 ? 'd'
												 : 'x';
	const char* const kModes = "sdxht";
	const size_t index = static_cast<size_t>(std::strchr(kModes, mode) - kModes) + (divide ? 5 : 0);
	if (complex_helpers_.empty())
		complex_helpers_.assign(10, lowir::kNoId);
	if (complex_helpers_[index] != lowir::kNoId)
		return complex_helpers_[index];
	const std::string name = std::string(divide ? "__div" : "__mul") + mode + "c3";
	const lowir::SymbolId symbol = new_symbol("__cppgm_libgcc_" + name.substr(2));
	lowir::Signature signature = lowir::Signature();
	signature.first_parameter = static_cast<uint32_t>(program_.parameters.size());
	for (uint32_t i = 0; i < 4; ++i)
	{
		lowir::Parameter parameter = lowir::Parameter();
		parameter.name = argument_name(i);
		parameter.type = storage;
		parameter.classes = soft_float_classes(soft);
		program_.parameters.push_back(parameter);
	}
	signature.parameter_count = 4;
	const sema::Type* complex = graph().types.complex(bare);
	signature.result = storage_type(complex);
	signature.boundary.result_classes = complex_classes(complex);
	signature.boundary.unwind = ir_model::CUM_NO;
	program_.declare_function(symbol, lowir::SymbolKind::FunctionDeclaration, program_.add_signature(signature));
	lowir::SymbolMetadata& metadata = program_.symbols[symbol].metadata;
	metadata.linkage = ir_model::LLM_C;
	metadata.binding = ir_model::SBM_STRONG;
	metadata.object = intern(name.data(), static_cast<uint32_t>(name.size()));
	complex_helpers_[index] = symbol;
	++counters_.declarations;
	return symbol;
}

RValue FunctionLowering::make_complex(const syntax::CallExpr& expr)
{
	// The parts evaluated in order.
	const RValue real = operand(*expr.arguments[0], ConversionContext::Argument);
	const RValue imaginary = operand(*expr.arguments[1], ConversionContext::Argument);
	return complex_value(expr.facts->type, real, imaginary);
}

RValue FunctionLowering::complex_value(const sema::Type* type, const RValue& real, const RValue& imaginary)
{
	// Each part stored in its half of a slot, read as one value.
	const sema::Type* part = type->unqualified->element;
	const lowir::Type part_type = lowering_.storage_type(part);
	const lowir::Type whole = lowering_.storage_type(type);
	Place storage = Place();
	storage.storage = lowir::Value::slot(generated_slot(lowir::SlotKind::Complex, whole));
	const lowir::Value address = address_of(storage);
	Place half = Place();
	half.storage = address;
	store(part_type, real, half);
	const lowir::Type i64 = lowir::Type::scalar(lowir::TypeKind::I64);
	half.storage = index(lowir::Type::scalar(lowir::TypeKind::I8), ir_model::IPK_FIELD, address,
		integer(i64, lowering_.size_of(part, 0)))
					   .value;
	store(part_type, imaginary, half);
	return load(whole, storage);
}

void FunctionLowering::complex_parts(const RValue& value, const sema::Type* type, RValue& real, RValue& imaginary)
{
	// The value stored in a slot, each half read.
	const sema::Type* part = type->unqualified->element;
	const lowir::Type part_type = lowering_.storage_type(part);
	Place storage = Place();
	storage.storage = lowir::Value::slot(generated_slot(lowir::SlotKind::Complex, value.type));
	store(value.type, value, storage);
	const lowir::Value address = address_of(storage);
	Place half = Place();
	half.storage = address;
	real = load(part_type, half);
	const lowir::Type i64 = lowir::Type::scalar(lowir::TypeKind::I64);
	half.storage = index(lowir::Type::scalar(lowir::TypeKind::I8), ir_model::IPK_FIELD, address,
		integer(i64, lowering_.size_of(part, 0)))
					   .value;
	imaginary = load(part_type, half);
}

RValue FunctionLowering::complex_convert(
	const RValue& value, const sema::Type* from, const sema::Type* to, ConversionContext context)
{
	// Part by part; a real value is the real part, the imaginary one zero.
	const sema::Type* part = to->unqualified->element;
	// (Operands emitted last first: see FunctionLowering::binary.)
	if (from->unqualified->kind != sema::TypeKind::Complex)
	{
		const RValue zero = zero_value(part);
		return complex_value(to, fit(convert(value, from, part, context), part), zero);
	}
	const sema::Type* source = from->unqualified->element;
	RValue real, imaginary;
	complex_parts(value, from, real, imaginary);
	const RValue imaginary_part = fit(convert(imaginary, source, part, context), part);
	return complex_value(to, fit(convert(real, source, part, context), part), imaginary_part);
}

RValue FunctionLowering::complex_truth(const RValue& value, const sema::Type* from)
{
	// 4.12 as C99 6.3.1.2 has it: false only when both parts are zero.
	const sema::Type* part = from->unqualified->element;
	RValue real, imaginary;
	complex_parts(value, from, real, imaginary);
	const RValue a = to_bool_truth(real, part);
	const RValue b = to_bool_truth(imaginary, part);
	return binary(lowir::BinaryOp::Or, a.type, a, b);
}

RValue FunctionLowering::complex_equal(bool equal, const sema::Type* type, const RValue& left, const RValue& right)
{
	// C99 6.5.9p4: equal when both parts are (!= its negation).
	const sema::Type* part = type->unqualified->element;
	const lowir::Type part_type = lowering_.storage_type(part);
	const lowir::ComparePredicate predicate = equal ? lowir::ComparePredicate::Eq : lowir::ComparePredicate::Ne;
	RValue a, b, c, d;
	complex_parts(left, type, a, b);
	complex_parts(right, type, c, d);
	const SoftFloat soft = part_type.is_object() ? SoftFloatOf(part) : SoftFloat::None;
	const RValue reals = soft != SoftFloat::None ? soft_compare(predicate, soft, a, c)
												 : compare(predicate, part_type, a, c);
	const RValue imaginaries = soft != SoftFloat::None ? soft_compare(predicate, soft, b, d)
													   : compare(predicate, part_type, b, d);
	if (reals.type != imaginaries.type)
		throw LoweringError(0, "complex part comparisons of different truth types");
	return binary(equal ? lowir::BinaryOp::And : lowir::BinaryOp::Or, reals.type, reals, imaginaries);
}

RValue FunctionLowering::part_negate(const sema::Type* part, const RValue& value)
{
	if (value.type.is_object() && SoftFloatOf(part) != SoftFloat::None)
		return soft_negate(SoftFloatOf(part), value);
	return unary(lowir::UnaryOp::Neg, value);
}

RValue FunctionLowering::complex_unary(TokenKind op, const sema::Type* type, const RValue& value)
{
	// -z negates both parts, GNU ~z (the conjugate) the imaginary one.
	const sema::Type* part = type->unqualified->element;
	RValue real, imaginary;
	complex_parts(value, type, real, imaginary);
	if (op == TokenKind::OP_MINUS)
		real = part_negate(part, real);
	return complex_value(type, real, part_negate(part, imaginary));
}

RValue FunctionLowering::complex_arithmetic(TokenKind op, const sema::Type* type, const RValue& left, bool left_complex,
	const RValue& right, bool right_complex)
{
	// (a + bi) op (c + di), a real operand's imaginary part absent: it
	// takes no part in the operation (C99 G.5.1), but for a real dividend
	// of a complex divisor (GCC's x / (c + di) is (x + 0i) / (c + di)).
	const sema::Type* part = type->unqualified->element;
	const uint32_t token = static_cast<uint32_t>(op);
	RValue a = left, b, c = right, d;
	if (left_complex)
		complex_parts(left, type, a, b);
	if (right_complex)
		complex_parts(right, type, c, d);
	RValue real, imaginary;
	switch (op)
	{
	case TokenKind::OP_PLUS:
	case TokenKind::OP_MINUS:
		real = arithmetic_op(token, part, a, c);
		imaginary = !right_complex ? b
			: !left_complex        ? (op == TokenKind::OP_MINUS ? part_negate(part, d) : d)
								   : arithmetic_op(token, part, b, d);
		break;
	case TokenKind::OP_STAR:
		if (left_complex && right_complex)
			return complex_product(type, a, b, c, d);
		real = arithmetic_op(token, part, a, c);
		imaginary = left_complex ? arithmetic_op(token, part, b, c) : arithmetic_op(token, part, a, d);
		break;
	default:
		if (!right_complex)
		{
			real = arithmetic_op(token, part, a, c);
			imaginary = arithmetic_op(token, part, b, c);
			break;
		}
		return complex_quotient(type, a, left_complex ? b : zero_value(part), c, d);
	}
	return complex_value(type, real, imaginary);
}

RValue FunctionLowering::complex_helper_call(
	bool divide, const sema::Type* type, const RValue& a, const RValue& b, const RValue& c, const RValue& d)
{
	const size_t base = arguments_.size();
	arguments_.push_back(a.value);
	arguments_.push_back(b.value);
	arguments_.push_back(c.value);
	arguments_.push_back(d.value);
	RValue result;
	result.type = lowering_.storage_type(type);
	result.value = emit_call(result.type,
		lowir::Value::symbol(lowering_.complex_helper(divide, type->unqualified->element)), base, lowir::kNoId, false);
	return result;
}

RValue FunctionLowering::complex_product(
	const sema::Type* type, const RValue& a, const RValue& b, const RValue& c, const RValue& d)
{
	// (ac - bd) + (ad + bc)i; of floating parts recomputed by __mul?c3 when
	// both are NaN (an infinite operand's product, C99 G.5.1p4).
	const sema::Type* part = type->unqualified->element;
	const lowir::Type part_type = lowering_.storage_type(part);
	if (part_type.is_object())
		return complex_helper_call(false, type, a, b, c, d);
	const uint32_t times = static_cast<uint32_t>(TokenKind::OP_STAR);
	const RValue bd = arithmetic_op(times, part, b, d);
	const RValue real = arithmetic_op(
		static_cast<uint32_t>(TokenKind::OP_MINUS), part, arithmetic_op(times, part, a, c), bd);
	const RValue bc = arithmetic_op(times, part, b, c);
	const RValue imaginary = arithmetic_op(
		static_cast<uint32_t>(TokenKind::OP_PLUS), part, arithmetic_op(times, part, a, d), bc);
	const RValue product = complex_value(type, real, imaginary);
	if (!part_type.is_float())
		return product;
	const lowir::Type whole = lowering_.storage_type(type);
	Place result = Place();
	result.storage = lowir::Value::slot(generated_slot(lowir::SlotKind::Complex, whole));
	store(whole, product, result);
	const lowir::BlockId real_nan = new_block(lowir::BlockKind::ComplexNan);
	const lowir::BlockId recompute = new_block(lowir::BlockKind::ComplexRecompute);
	const lowir::BlockId end = new_block(lowir::BlockKind::ComplexEnd);
	branch(compare(lowir::ComparePredicate::Ne, part_type, real, real), real_nan, end);
	place(real_nan);
	branch(compare(lowir::ComparePredicate::Ne, part_type, imaginary, imaginary), recompute, end);
	place(recompute);
	store(whole, complex_helper_call(false, type, a, b, c, d), result);
	jump(end);
	place(end);
	return load(whole, result);
}

RValue FunctionLowering::complex_quotient(
	const sema::Type* type, const RValue& a, const RValue& b, const RValue& c, const RValue& d)
{
	// Floating parts: __div?c3 (C99 G.5.1's scaling and recovery, as GCC
	// calls it). Integral ones: Smith's formula as GCC expands it, by the
	// smaller of |c| and |d| -- if |c| < |d|, r = c/d, the quotient
	// ((ar + b) + (br - a)i) / (cr + d); else r = d/c, ((br + a) + (b -
	// ar)i) / (dr + c).
	const sema::Type* part = type->unqualified->element;
	const lowir::Type part_type = lowering_.storage_type(part);
	if (sema::IsFloating(part))
		return complex_helper_call(true, type, a, b, c, d);
	const uint32_t plus = static_cast<uint32_t>(TokenKind::OP_PLUS);
	const uint32_t minus = static_cast<uint32_t>(TokenKind::OP_MINUS);
	const uint32_t times = static_cast<uint32_t>(TokenKind::OP_STAR);
	const uint32_t over = static_cast<uint32_t>(TokenKind::OP_DIV);
	const lowir::Type whole = lowering_.storage_type(type);
	Place result = Place();
	result.storage = lowir::Value::slot(generated_slot(lowir::SlotKind::Complex, whole));
	// A signed part's magnitude: (x ^ s) - s for its sign mask s.
	const auto magnitude = [&](const RValue& x) {
		if (!SignedPart(part))
			return x;
		const RValue sign = binary(
			lowir::BinaryOp::Shr, part_type, x, integer(part_type, lowir::integer_bits(part_type.kind()) - 1));
		return binary(lowir::BinaryOp::Sub, part_type, binary(lowir::BinaryOp::Xor, part_type, x, sign), sign);
	};
	const RValue magnitude_c = magnitude(c);
	const RValue magnitude_d = magnitude(d);
	const lowir::BlockId by_d = new_block(lowir::BlockKind::ComplexByD);
	const lowir::BlockId by_c = new_block(lowir::BlockKind::ComplexByC);
	const lowir::BlockId end = new_block(lowir::BlockKind::ComplexEnd);
	branch(compare(SignedPart(part) ? lowir::ComparePredicate::Lt : lowir::ComparePredicate::Ult, part_type,
			   magnitude_c, magnitude_d),
		by_d, by_c);
	place(by_d);
	{
		const RValue ratio = arithmetic_op(over, part, c, d);
		const RValue divisor = arithmetic_op(plus, part, arithmetic_op(times, part, c, ratio), d);
		const RValue real = arithmetic_op(plus, part, arithmetic_op(times, part, a, ratio), b);
		const RValue imaginary = arithmetic_op(minus, part, arithmetic_op(times, part, b, ratio), a);
		const RValue imaginary_quotient = arithmetic_op(over, part, imaginary, divisor);
		store(whole, complex_value(type, arithmetic_op(over, part, real, divisor), imaginary_quotient), result);
		jump(end);
	}
	place(by_c);
	{
		const RValue ratio = arithmetic_op(over, part, d, c);
		const RValue divisor = arithmetic_op(plus, part, arithmetic_op(times, part, d, ratio), c);
		const RValue real = arithmetic_op(plus, part, arithmetic_op(times, part, b, ratio), a);
		const RValue imaginary = arithmetic_op(minus, part, b, arithmetic_op(times, part, a, ratio));
		const RValue imaginary_quotient = arithmetic_op(over, part, imaginary, divisor);
		store(whole, complex_value(type, arithmetic_op(over, part, real, divisor), imaginary_quotient), result);
		jump(end);
	}
	place(end);
	return load(whole, result);
}

Place FunctionLowering::complex_part_place(const syntax::Expr& operand, bool imaginary)
{
	// A glvalue's part: its storage, or the bytes after its real part.
	const sema::Type* type = operand.facts->type;
	Place place = glvalue(operand);
	const lowir::Value address = address_of(place);
	Place part = Place();
	part.storage = address;
	if (imaginary)
	{
		const lowir::Type i64 = lowir::Type::scalar(lowir::TypeKind::I64);
		part.storage = index(lowir::Type::scalar(lowir::TypeKind::I8), ir_model::IPK_FIELD, address,
			integer(i64, lowering_.size_of(type->unqualified->element, operand.location)))
						   .value;
	}
	qualify(part, type);
	return part;
}

RValue FunctionLowering::complex_part(const syntax::Expr& operand, bool imaginary)
{
	// A prvalue's part from the slot holding it.
	const sema::Type* type = operand.facts->type;
	const lowir::Type part_type = lowering_.storage_type(type->unqualified->element);
	if (operand.facts->category != sema::ValueCategory::PRValue)
		return load(part_type, complex_part_place(operand, imaginary));
	const lowir::Type whole = lowering_.storage_type(type);
	Place storage = Place();
	storage.storage = lowir::Value::slot(generated_slot(lowir::SlotKind::Complex, whole));
	store(whole, rvalue(operand), storage);
	const lowir::Value address = address_of(storage);
	Place part = Place();
	part.storage = address;
	if (imaginary)
	{
		const lowir::Type i64 = lowir::Type::scalar(lowir::TypeKind::I64);
		part.storage = index(lowir::Type::scalar(lowir::TypeKind::I8), ir_model::IPK_FIELD, address,
			integer(i64, lowering_.size_of(type->unqualified->element, operand.location)))
						   .value;
	}
	return load(part_type, part);
}

} // namespace lower
