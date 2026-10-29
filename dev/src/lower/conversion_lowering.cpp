#include "lower/function_lowering.h"

#include "lowir/literals.h"
#include "sema/conversions.h"
#include "sema/integral_arithmetic.h"

// The standard conversions of a value (clause 4: integral, floating,
// pointer, boolean and complex ones, the course's literal folding), the
// zero of a value-initialized scalar; and user-defined conversions by
// conversion functions (12.3.2): the function called on the operand's
// object (a glvalue's address, a prvalue's temporary), its result then
// converted by the second standard conversion the analysis recorded, bound
// to a reference, or initializing a class object in place. A converted
// built-in operand (kUserConverted) is the call itself; its own facts give
// its object.
namespace lower {

namespace {

const syntax::Expr* WithoutParentheses(const syntax::Expr* expr)
{
	while (expr->kind == syntax::ExprKind::Paren)
		expr = static_cast<const syntax::ParenExpr*>(expr)->inner;
	return expr;
}

bool IsPointerType(const sema::Type* type)
{
	return type->unqualified->kind == sema::TypeKind::Pointer;
}

bool IsNullptrType(const sema::Type* type)
{
	type = type->unqualified;
	return type->kind == sema::TypeKind::Fundamental && type->fundamental == FundamentalType::NullptrT;
}

bool IsBoolType(const sema::Type* type)
{
	type = type->unqualified;
	return type->kind == sema::TypeKind::Fundamental && type->fundamental == FundamentalType::Bool;
}

bool IsSigned(const sema::Type* type)
{
	sema::IntegralKind kind;
	return sema::IntegralKindOf(type, kind) && kind.is_signed && kind.bits > 1;
}

// Whether a literal's widening folds into the literal (the course's
// immediates convention, by context and by the signedness of the literal's
// type and the target's): signed to signed folds in every folding context;
// an additive operand also folds unsigned to unsigned and bool; an
// equality or relational operand folds bool; a result folds all but signed
// to unsigned; assignments, multiplicative, bitwise and shift operands and
// conditional arms never fold.
bool WideningFolds(ConversionContext context, const sema::Type* from, bool to_signed)
{
	if (context == ConversionContext::BesideSize)
		return true;
	const bool from_bool = IsBoolType(from);
	const bool from_signed = !from_bool && IsSigned(from);
	if (from_signed && to_signed)
		return context != ConversionContext::Operand && context != ConversionContext::Assignment &&
			context != ConversionContext::CompoundAssignment;
	switch (context)
	{
	case ConversionContext::Additive:
	case ConversionContext::Unary:
		return from_bool || (!from_signed && !to_signed);
	case ConversionContext::Comparison:
		return from_bool;
	case ConversionContext::Return:
		return !from_signed || to_signed;
	default:
		return false;
	}
}

} // namespace

RValue FunctionLowering::fit(const RValue& value, const sema::Type* type)
{
	// A truth value (i64) stored as the course bool (u8).
	if (!IsBoolType(type) || value.type.kind() != lowir::TypeKind::I64)
		return value;
	return convert_integer(
		value, type, lowir::Type::scalar(lowir::TypeKind::U8), ConversionContext::Initialization, false, false);
}

RValue FunctionLowering::convert(
	const RValue& value, const sema::Type* from, const sema::Type* to, ConversionContext context, bool literal)
{
	from = from->unqualified;
	to = to->unqualified;
	if (from == to || sema::IsVoid(to))
		return value;
	const lowir::Type target = lowering_.storage_type(to);
	const bool from_float = sema::IsFloating(from);
	const bool to_float = sema::IsFloating(to);
	const bool from_pointer = IsPointerType(from) || IsNullptrType(from) || from->kind == sema::TypeKind::MemberPointer;
	// 4.4: a pointer to member keeps its value through a qualification
	// conversion (and a reinterpret_cast, 5.2.10p10).
	if (from->kind == sema::TypeKind::MemberPointer && to->kind == sema::TypeKind::MemberPointer)
		return value;
	if (to->kind == sema::TypeKind::Complex)
		return complex_convert(value, from, to, context);
	if (IsBoolType(to))
	{
		if (IsBoolType(from))
			return value;
		return to_bool_truth(value, from);
	}
	if ((from_float || to_float) && (SoftFloatOf(from) != SoftFloat::None || SoftFloatOf(to) != SoftFloat::None))
		return soft_convert(value, from, to);
	if (to_float)
	{
		if (from_float)
		{
			if (value.type == target)
				return value;
			return convert_op(lowir::float_rank(target.kind()) > lowir::float_rank(value.type.kind())
					? lowir::ConvertOp::FPExt
					: lowir::ConvertOp::FPTrunc,
				target, value);
		}
		return convert_op(IsSigned(from) ? lowir::ConvertOp::SIToFP : lowir::ConvertOp::UIToFP, target, value);
	}
	if (from_float)
		return convert_op(IsSigned(to) ? lowir::ConvertOp::FPToSI : lowir::ConvertOp::FPToUI, target, value);
	if (target.is_pointer())
	{
		// A pointer keeps its bits, a pointer to a class adjusted to its base
		// class subobject (4.10p3); nullptr and integers are retyped (4.10,
		// 5.2.10p5).
		if (value.type.is_pointer() && !IsNullptrType(from))
		{
			const sema::ClassEntity* derived = IsPointerType(from) ? sema::ClassOf(from->element) : nullptr;
			const sema::ClassEntity* base = IsPointerType(to) ? sema::ClassOf(to->element) : nullptr;
			if (derived != nullptr && base != nullptr && derived != base && context != ConversionContext::Cast)
				return pointer_to_base(value, derived, base, 0);
			return value;
		}
		return copy(target, value);
	}
	if (from_pointer)
	{
		// 5.2.10p4: a pointer to an integer (retyped, or narrowed).
		if (lowir::integer_bits(target.kind()) == 64)
			return copy(target, value);
		return convert_op(lowir::ConvertOp::Trunc, target, value);
	}
	return convert_integer(value, from, target, context, IsSigned(to), literal);
}

RValue FunctionLowering::convert_implicit(
	const RValue& value, const sema::Type* from, const sema::Type* to, ConversionContext context, uint32_t location)
{
	from = from->unqualified;
	to = to->unqualified;
	if (from->kind == sema::TypeKind::MemberPointer && to->kind == sema::TypeKind::MemberPointer &&
		sema::ClassOf(from->owner) != sema::ClassOf(to->owner))
		return member_pointer_converted(value, member_adjustment(from, to, location));
	return convert(value, from, to, context);
}

RValue FunctionLowering::convert_integer(const RValue& value, const sema::Type* from, lowir::Type to,
	ConversionContext context, bool to_signed, bool literal)
{
	// 4.5, 4.7: widths by LowIR kind, extension by the source's
	// signedness; a same-width change of type or signedness is a `copy`
	// (bool's truth values are 0 and 1 either way). A literal value folds
	// when narrowed or retyped; a literal as written also folds when
	// widened, except into an unsigned type from a signed one, as an
	// initializer, argument, result or cast, or an additive, relational or
	// unary operand (the course's immediates convention).
	const bool from_signed = from != nullptr && IsSigned(from);
	const bool sign_change = from != nullptr && !IsBoolType(from) && from_signed != to_signed;
	if (value.type == to && !sign_change)
		return value;
	const unsigned from_bits = lowir::integer_bits(value.type.kind());
	const unsigned to_bits = lowir::integer_bits(to.kind());
	uint64_t bits = 0;
	if (is_integer_literal(value, bits))
	{
		// A 128-bit result keeps a 128-bit literal's value; a narrower one
		// extends by the source's signedness (4.7p2, p3).
		if (to_bits == 128 && from_bits == 128)
			return integer(to, builder_.body().literals[value.value.index]);
		const bool widening_folds = literal && from != nullptr && to_bits > from_bits &&
			WideningFolds(context, from, to_signed) && context != ConversionContext::ArrayElement;
		// A literal narrowed as a class aggregate's member keeps its
		// conversion unless both types are signed; as a bit-field's assigned
		// value it always does.
		const bool keeps_narrowing = to_bits < from_bits &&
			(context == ConversionContext::BitFieldAssignment || context == ConversionContext::ArrayElement ||
				(context == ConversionContext::MemberInitialization && !(from_signed && to_signed)));
		if ((to_bits <= from_bits && !keeps_narrowing) || widening_folds)
		{
			const unsigned width = to_bits < from_bits ? to_bits : from_bits;
			uint64_t folded = width >= 64 ? bits : bits & ((uint64_t(1) << width) - 1);
			const bool extend_sign = to_bits <= from_bits ? to_signed : from_signed;
			if (extend_sign && width < 64 && (folded >> (width - 1)) != 0)
				folded |= ~((uint64_t(1) << width) - 1);
			if (to_bits == 128)
			{
				lowir::Literal wide = lowir::integer_literal(static_cast<int64_t>(folded));
				if (!extend_sign)
					wide.high = 0;
				return integer(to, wide);
			}
			return integer(to, folded);
		}
	}
	if (to_bits == from_bits)
		return copy(to, value);
	if (to_bits < from_bits)
		return convert_op(lowir::ConvertOp::Trunc, to, value);
	return convert_op(from_signed ? lowir::ConvertOp::SExt : lowir::ConvertOp::ZExt, to, value);
}

RValue FunctionLowering::to_bool_truth(const RValue& value, const sema::Type* from)
{
	// 4.12: zero, null and false are false; the truth is i64. A pointer to
	// member by its truth (member_pointer_truth); a 128-bit integer is zero
	// by both halves.
	if (from->unqualified->kind == sema::TypeKind::MemberPointer)
		return member_pointer_null(value, false);
	if (from->unqualified->kind == sema::TypeKind::Complex)
		return complex_truth(value, from);
	const SoftFloat soft = value.type.is_object() ? SoftFloatOf(from) : SoftFloat::None;
	if (soft != SoftFloat::None)
		return soft_compare(lowir::ComparePredicate::Ne, soft, value, soft_zero(soft));
	const RValue zero = value.type.is_float() ? floating(value.type, lowir::float_literal(value.type.kind(), 0.0L))
											  : integer(value.type, 0);
	return compare(lowir::ComparePredicate::Ne, value.type, value, zero);
}

RValue FunctionLowering::zero_value(const sema::Type* type)
{
	// 8.5p5-p7: a value-initialized scalar is zero (a pointer to data
	// member's null value).
	const lowir::Type storage = lowering_.storage_type(type);
	if (storage.is_float())
		return floating(storage, lowir::float_literal(storage.kind(), 0.0L));
	if (storage.is_object() && SoftFloatOf(type) != SoftFloat::None)
		return soft_zero(SoftFloatOf(type));
	if (type->unqualified->kind == sema::TypeKind::Complex || type->unqualified->kind == sema::TypeKind::Vector)
		return zero_object_value(storage);
	if (type->unqualified->kind == sema::TypeKind::MemberPointer && storage.kind() != lowir::TypeKind::I128)
		return integer(storage, static_cast<uint64_t>(lowering_.null_data_member()));
	return integer(storage, 0);
}

bool FunctionLowering::conversion_function(const sema::Conversion* conversion)
{
	return conversion != nullptr && conversion->constructor != nullptr && !conversion->constructor->constructor;
}

lowir::Value FunctionLowering::conversion_object(
	const syntax::Expr& expr, const sema::ExprFacts* facts, const sema::FunctionEntity* function)
{
	// The implicit object argument (13.3.1p4): the operand evaluated with
	// its own facts, converted to the function's class.
	const syntax::Expr* e = WithoutParentheses(&expr);
	const sema::ExprFacts* saved = e->facts;
	e->facts = facts;
	const lowir::Value object = object_address(*e);
	e->facts = saved;
	return to_base(object, sema::ClassOf(facts->type), sema::ClassOf(function->owner->entity->type), e->location);
}

RValue FunctionLowering::call_conversion(const syntax::Expr& expr, const sema::ExprFacts* facts,
	const sema::FunctionEntity* function, lowir::Value destination)
{
	// The call: a class result initializes `destination` (through it as
	// an indirect result, or copied from the call's value).
	const sema::ClosureFacts* closure = lowering_.graph().closure_facts(sema::ClassOf(function->owner->entity->type));
	if (closure != nullptr && closure->conversion == function)
	{
		// 5.1.2p6: a captureless closure's conversion yields the address of
		// the function with its operator's effect (the course's form: no
		// call); the operand is evaluated only for its effects.
		const syntax::Expr* e = WithoutParentheses(&expr);
		const sema::ExprFacts* saved = e->facts;
		e->facts = facts;
		if (e->kind != syntax::ExprKind::Lambda && !side_effect_free(e, false, 0))
			conversion_object(expr, facts, function);
		e->facts = saved;
		RValue address;
		address.type = lowir::Type::scalar(lowir::TypeKind::Ptr);
		address.value = function_address(closure->invoker, expr.location);
		return address;
	}
	const bool throws = call_may_throw(function);
	begin_call(throws);
	const size_t base = arguments_.size();
	const sema::Type* returned = function->type->canonical->element;
	if (lowering_.indirect_result(function))
		arguments_.push_back(destination);
	const lowir::Value object = conversion_object(expr, facts, function);
	arguments_.push_back(object);
	// 12.3.2p5: an implicit conversion names no qualified-id; a virtual
	// conversion function's final overrider is called (5.2.2p1).
	lowir::SignatureId signature = lowir::kNoId;
	const lowir::Value callee = member_callee(function, object, function->is_virtual, signature);
	RValue value;
	value.type = result_type(function);
	value.value = emit_call(value.type, callee, base, signature, throws);
	end_call(throws);
	if (value.type.kind() == lowir::TypeKind::Obj)
		copy_result(value, returned, destination, expr.location);
	return value;
}

lowir::Value FunctionLowering::conversion_temporary(
	const syntax::Expr& expr, const sema::FunctionEntity* function, bool persistent)
{
	// A class prvalue result materialized in a temporary (12.2p1),
	// destroyed with its full-expression (12.2p3) or persistent.
	const sema::Type* returned = function->type->canonical->element;
	const bool destroyed = !persistent && destroyed_temporary(returned);
	if (destroyed)
		temporary_region();
	Place temporary = Place();
	temporary.storage = temporary_storage(returned, persistent, lowir::SlotKind::Arg);
	const lowir::Value address = address_of(temporary);
	open_deferred_region();
	call_conversion(expr, WithoutParentheses(&expr)->facts, function, address);
	if (destroyed)
		temporary_alive(address, returned);
	else if (persistent)
		persistent_alive(temporary.storage, returned);
	return address;
}

RValue FunctionLowering::converted_operand(const syntax::Expr& expr)
{
	// A kUserConverted operand's value: the call's result, read when it is
	// a reference.
	const sema::ExprFacts* facts = WithoutParentheses(&expr)->facts;
	const sema::FunctionEntity* function = static_cast<const sema::FunctionEntity*>(facts->entity);
	const RValue value = call_conversion(expr, facts->original, function, lowir::Value());
	if (facts->category == sema::ValueCategory::PRValue || facts->type->kind == sema::TypeKind::Array ||
		facts->type->kind == sema::TypeKind::Function)
		return value;
	Place place = Place();
	place.storage = value.value;
	qualify(place, facts->type);
	return load(lowering_.storage_type(facts->type), place);
}

RValue FunctionLowering::converted_value(
	const syntax::Expr& expr, const sema::Conversion* conversion, ConversionContext context)
{
	// An operand converted by a conversion function to a scalar: the
	// result (read through a scalar reference, or decayed through an array
	// or function reference), then the second standard conversion.
	const sema::ExprFacts* facts = WithoutParentheses(&expr)->facts;
	const sema::FunctionEntity* function = conversion->constructor;
	RValue value = call_conversion(expr, facts, function, lowir::Value());
	const sema::Type* returned = function->type->canonical->element;
	const sema::Type* result = returned->is_reference() ? returned->element : returned;
	const sema::Conversion* after = conversion->after;
	const bool decays = after != nullptr &&
		(after->first == sema::LvalueTransform::ArrayToPointer ||
			after->first == sema::LvalueTransform::FunctionToPointer);
	if (decays)
		result = lowering_.graph().types.pointer(
			after->first == sema::LvalueTransform::ArrayToPointer ? result->element : result);
	else if (returned->is_reference())
	{
		Place place = Place();
		place.storage = value.value;
		value = load(lowering_.storage_type(result), place);
	}
	if (after == nullptr || after->result == nullptr || after->form == sema::ConversionForm::Reference)
		return value;
	return convert_implicit(value, result, after->result, context, expr.location);
}

lowir::Value FunctionLowering::converted_reference(
	const syntax::Expr& expr, const sema::Conversion* conversion, const sema::Type* reference, bool persistent)
{
	// A reference bound through a conversion function (13.3.1.6): to the
	// glvalue its reference result designates, else to a temporary its
	// result initializes (a class prvalue materialized in it).
	const sema::ExprFacts* facts = WithoutParentheses(&expr)->facts;
	const sema::FunctionEntity* function = conversion->constructor;
	const sema::Type* returned = function->type->canonical->element;
	const sema::Conversion* after = conversion->after;
	if (returned->is_reference())
	{
		const lowir::Value bound = call_conversion(expr, facts, function, lowir::Value()).value;
		if (after != nullptr && after->second == sema::ValueConversion::DerivedToBase)
			return to_base(bound, sema::ClassOf(returned->element), sema::ClassOf(reference->element), expr.location);
		return bound;
	}
	const sema::Type* object = reference->element;
	if (sema::ClassOf(returned) != nullptr)
	{
		const lowir::Value address = conversion_temporary(expr, function, persistent);
		if (after != nullptr && after->second == sema::ValueConversion::DerivedToBase)
			return to_base(address, sema::ClassOf(returned), sema::ClassOf(object), expr.location);
		return address;
	}
	RValue value = call_conversion(expr, facts, function, lowir::Value());
	value = fit(convert_implicit(value, returned, object, ConversionContext::Initialization, expr.location), object);
	Place temporary = Place();
	temporary.storage = temporary_storage(object, persistent, lowir::SlotKind::Tmpref);
	store(lowering_.storage_type(object), value, temporary);
	return address_of(temporary);
}

} // namespace lower
