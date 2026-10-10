#include "lower/function_lowering.h"

#include "sema/constant_values.h"
#include "sema/conversions.h"

// Pointers to members (5.3.1p3, 4.11, 5.5), as Itanium 2.3 lays
// them out (course LowIR and host objects alike). A pointer to data member
// is the member's offset in the pointer's class (i64), its null value -1: a
// zero-initialized one holds -1 (8.5p5, null_data_member_pointers) and its
// truth is the value plus one. A pointer to member function is two words
// (i128): the function's address, or one plus the offset of its table slot
// for a virtual function, and the offset of the function's class subobject
// in the pointer's class (the this-adjustment, high word). A conversion to
// a pointer to member of a derived class adds the base's offset to a
// non-null value (4.11p2; 5.2.9p12's inverse subtracts it), so null values
// stay null (a member function's null is its zero function word alone,
// member_pointer_null). `.*` and `->*` reach the pointer's class subobject
// of the object, then the member at the offset, or call the function with
// the object adjusted by the high word and, for an odd function word,
// looked up in the adjusted object's table (whatever the pointer's class: a
// polymorphic derived class's virtual function at a non-zero offset may
// become a non-polymorphic base's by 5.2.9p12, perhaps in another
// translation unit).
namespace lower {

namespace {

const uint64_t kSlotBytes = 8;

const syntax::Expr* WithoutParentheses(const syntax::Expr* expr)
{
	while (expr->kind == syntax::ExprKind::Paren)
		expr = static_cast<const syntax::ParenExpr*>(expr)->inner;
	return expr;
}

// `&C::m` written as such (5.3.1p3), or null.
const syntax::UnaryExpr* MemberAddress(const syntax::Expr& expr)
{
	const syntax::Expr* e = WithoutParentheses(&expr);
	if (e->kind != syntax::ExprKind::Unary || static_cast<const syntax::UnaryExpr*>(e)->op != TokenKind::OP_AMP ||
		(e->facts->flags & sema::kOperatorCall) != 0 || e->facts->type == nullptr ||
		e->facts->type->unqualified->kind != sema::TypeKind::MemberPointer)
		return nullptr;
	return static_cast<const syntax::UnaryExpr*>(e);
}

} // namespace

RValue FunctionLowering::member_pointer_value(const syntax::UnaryExpr& expr, uint64_t adjustment)
{
	// 5.3.1p3: `&C::m` of a member of C, its adjustment (a conversion to a
	// derived class's pointer folded) added.
	const sema::Entity* entity = WithoutParentheses(expr.operand)->facts->entity;
	if (entity->kind != sema::EntityKind::Function)
	{
		const sema::VariableEntity* member = static_cast<const sema::VariableEntity*>(entity);
		return constant(lowir::Type::scalar(lowir::TypeKind::I64),
			lowering_.member_home_offset(member, expr.location) + adjustment);
	}
	return member_function_word(static_cast<const sema::FunctionEntity*>(entity), adjustment);
}

RValue FunctionLowering::member_function_word(const sema::FunctionEntity* function, uint64_t adjustment)
{
	// A member function's i128: its address (a virtual one's slot offset
	// plus one), zero-extended, the adjustment shifted into the high word.
	const lowir::Type i64 = lowir::Type::scalar(lowir::TypeKind::I64);
	const lowir::Type i128 = lowir::Type::scalar(lowir::TypeKind::I128);
	RValue word;
	if (function->is_virtual)
		word = convert_op(lowir::ConvertOp::ZExt, i128, integer(i64, 1 + function->vtable_slot * kSlotBytes));
	else
	{
		lowering_.demand(function);
		RValue address;
		address.value = address_of_symbol(lowering_.function_symbol(function));
		address.type = lowir::Type::scalar(lowir::TypeKind::Ptr);
		word = convert_op(lowir::ConvertOp::ZExt, i128, copy(i64, address));
	}
	if (adjustment == 0)
		return word;
	return binary(lowir::BinaryOp::Add, i128, word,
		binary(lowir::BinaryOp::Shl, i128, integer(i128, adjustment), integer(i128, 64)));
}

RValue FunctionLowering::member_pointer_constant(const sema::ConstValue& value, uint32_t location)
{
	// A constant pointer to member (a template argument's, 14.3.2): a data
	// member's offset as an immediate, a member function's word
	// as `&C::f` makes it, its adjustment added.
	const lowir::Type i64 = lowir::Type::scalar(lowir::TypeKind::I64);
	const lowir::Type i128 = lowir::Type::scalar(lowir::TypeKind::I128);
	const sema::Entity* member = static_cast<const sema::Entity*>(value.address.entity);
	const bool function = value.type->element->kind == sema::TypeKind::Function;
	if (member == nullptr)
		return function ? integer(i128, 0) : integer(i64, static_cast<uint64_t>(lowering_.null_data_member()));
	const uint64_t adjustment = static_cast<uint64_t>(value.address.offset);
	if (!function)
		return integer(
			i64, lowering_.member_home_offset(static_cast<const sema::VariableEntity*>(member), location) + adjustment);
	return member_function_word(static_cast<const sema::FunctionEntity*>(member), adjustment);
}

uint64_t FunctionLowering::member_adjustment(const sema::Type* from, const sema::Type* to, uint32_t location)
{
	// The offset of the class of member pointer type `from` in the derived
	// class of `to` (4.11p2).
	return lowering_.base_offset(
		sema::ClassOf(to->unqualified->owner), sema::ClassOf(from->unqualified->owner), location);
}

RValue FunctionLowering::member_pointer_converted(const RValue& value, uint64_t adjustment)
{
	// 4.11p2: a non-null value gains the adjustment (a negative one for
	// 5.2.9p12), the null value stays null.
	if (adjustment == 0)
		return value;
	const lowir::Type i64 = lowir::Type::scalar(lowir::TypeKind::I64);
	const lowir::Type i128 = lowir::Type::scalar(lowir::TypeKind::I128);
	if (value.type.kind() != lowir::TypeKind::I128)
	{
		const RValue present = compare(
			lowir::ComparePredicate::Ne, i64, value, integer(i64, static_cast<uint64_t>(lowering_.null_data_member())));
		return binary(
			lowir::BinaryOp::Add, i64, value, binary(lowir::BinaryOp::Mul, i64, present, integer(i64, adjustment)));
	}
	const RValue word = convert_op(lowir::ConvertOp::Trunc, i64, value);
	const RValue present = compare(lowir::ComparePredicate::Ne, i64, word, integer(i64, 0));
	const RValue scaled = convert_op(
		lowir::ConvertOp::SExt, i128, binary(lowir::BinaryOp::Mul, i64, present, integer(i64, adjustment)));
	return binary(lowir::BinaryOp::Add, i128, value, binary(lowir::BinaryOp::Shl, i128, scaled, integer(i128, 64)));
}

RValue FunctionLowering::member_pointer_truth(const RValue& value)
{
	// An i64 that is zero exactly when a pointer to member is null (4.12):
	// a data member's value plus one (null is -1); a member function's
	// function word (Itanium 2.3: zero whatever the adjustment, which
	// converted null values carry).
	const lowir::Type i64 = lowir::Type::scalar(lowir::TypeKind::I64);
	if (value.type.kind() != lowir::TypeKind::I128)
		return binary(lowir::BinaryOp::Add, i64, value, integer(i64, 1));
	return convert_op(lowir::ConvertOp::Trunc, i64, value);
}

RValue FunctionLowering::member_pointer_null(const RValue& value, bool null)
{
	// Whether a pointer to member is (not) null: its truth compared with
	// zero.
	const lowir::Type i64 = lowir::Type::scalar(lowir::TypeKind::I64);
	return compare(null ? lowir::ComparePredicate::Eq : lowir::ComparePredicate::Ne, i64, member_pointer_truth(value),
		integer(i64, 0));
}

RValue FunctionLowering::member_pointer_equal(const RValue& left, const RValue& right, bool equal)
{
	// 5.10p2 on host objects' pointers to member functions: equal function
	// words, and equal adjustments unless both are null (Itanium 2.3):
	// both values equal, or both function words zero (`left == right ||
	// (lw | rw) == 0`; its negation `left != right && (lw | rw) != 0`).
	const lowir::Type i64 = lowir::Type::scalar(lowir::TypeKind::I64);
	const lowir::ComparePredicate same = equal ? lowir::ComparePredicate::Eq : lowir::ComparePredicate::Ne;
	const RValue values = compare(same, left.type, left, right);
	// (Operands emitted last first: see FunctionLowering::binary.)
	const RValue right_word = convert_op(lowir::ConvertOp::Trunc, i64, right);
	const RValue words = binary(lowir::BinaryOp::Or, i64, convert_op(lowir::ConvertOp::Trunc, i64, left), right_word);
	const RValue null = compare(same, i64, words, integer(i64, 0));
	return binary(equal ? lowir::BinaryOp::Or : lowir::BinaryOp::And, i64, values, null);
}

bool FunctionLowering::member_conversion_operand(
	const syntax::Expr& expr, const sema::Conversion* conversion, RValue& out)
{
	// An operand converted to a pointer to member of a derived class: `&C::m`
	// with its adjustment folded, else its value adjusted.
	if (conversion == nullptr || conversion->second != sema::ValueConversion::MemberToDerived ||
		conversion->form == sema::ConversionForm::Reference || conversion->result == nullptr)
		return false;
	const uint64_t adjustment = member_adjustment(
		WithoutParentheses(&expr)->facts->type, conversion->result, expr.location);
	if (const syntax::UnaryExpr* address = MemberAddress(expr))
		out = member_pointer_value(*address, adjustment);
	else
		out = member_pointer_converted(rvalue(expr), adjustment);
	return true;
}

RValue FunctionLowering::member_pointer_cast(const syntax::Expr& expr, const syntax::Expr& operand_expr)
{
	// 5.2.9p12: to a pointer to member of a base class, the base's offset
	// taken from a non-null value; otherwise the conversion recorded.
	RValue value;
	if (member_conversion_operand(operand_expr, WithoutParentheses(&operand_expr)->facts->conversion, value))
		return value;
	const sema::Type* from = WithoutParentheses(&operand_expr)->facts->type;
	const uint64_t adjustment = member_adjustment(expr.facts->type, from, expr.location);
	return member_pointer_converted(rvalue(operand_expr), 0 - adjustment);
}

lowir::Value FunctionLowering::member_pointer_object(const syntax::BinaryExpr& expr)
{
	// 5.5: `.*`'s object (materialized if a prvalue) or `->*`'s pointer,
	// as an object of the pointer's class (its base).
	const sema::ExprFacts* left = WithoutParentheses(expr.left)->facts;
	const sema::Type* pointer = WithoutParentheses(expr.right)->facts->type->unqualified;
	const bool arrow = expr.op == TokenKind::OP_ARROWSTAR;
	const lowir::Value object = arrow ? operand(*expr.left, ConversionContext::Operand).value
									  : object_address(*expr.left);
	const sema::Type* object_type = arrow ? left->type->unqualified->element : left->type;
	const sema::ClassEntity* from = sema::ClassOf(object_type);
	const sema::ClassEntity* to = sema::ClassOf(pointer->owner);
	// A base at offset zero is the object's own address (the path is
	// walked once); a virtual base (or a base in one) is found through the
	// object's table.
	if (from == to)
		return object;
	const ProgramLowering::BaseRoute route = lowering_.base_route(from, to, expr.location);
	if (route.via != nullptr)
		return to_base(object, from, to, expr.location);
	return route.offset == 0 ? object
							 : index(lowir::Type::scalar(lowir::TypeKind::I8), ir_model::IPK_NONE, object,
								   integer(lowir::Type::scalar(lowir::TypeKind::I64), route.offset))
								   .value;
}

Place FunctionLowering::member_pointer_place(const syntax::BinaryExpr& expr)
{
	// 5.5: the data member at the pointer's offset (its value).
	const lowir::Value object = member_pointer_object(expr);
	const RValue offset = operand(*expr.right, ConversionContext::Operand);
	Place place = Place();
	place.storage = index(lowir::Type::scalar(lowir::TypeKind::I8), ir_model::IPK_FIELD, object, offset).value;
	// A member of a union object (lowir.md "Instruction Facts").
	const sema::Type* object_type = WithoutParentheses(expr.left)->facts->type;
	const sema::ClassEntity* cls = sema::ClassOf(
		expr.op == TokenKind::OP_ARROWSTAR ? object_type->unqualified->element : object_type);
	if (cls != nullptr && cls->key == TokenKind::KW_UNION)
		mark_untyped(place.storage);
	qualify(place, expr.facts->type);
	return place;
}

lowir::Value FunctionLowering::member_pointer_callee(const syntax::BinaryExpr& access, size_t object_argument)
{
	// 5.5p6 through a pointer to member function, after the object and the
	// arguments: the object adjusted by the high word, the low word the
	// function's address, or for a virtual function (odd) one plus its
	// slot's offset in the adjusted object's table.
	const lowir::Type i64 = lowir::Type::scalar(lowir::TypeKind::I64);
	const lowir::Type i128 = lowir::Type::scalar(lowir::TypeKind::I128);
	const lowir::Type pointer = lowir::Type::scalar(lowir::TypeKind::Ptr);
	const lowir::Type byte = lowir::Type::scalar(lowir::TypeKind::I8);
	// `&C::f` written as the operand, not virtual, is called directly (the
	// symbol itself); a template parameter's member function (no
	// adjustment, 14.3.2p5), not virtual, at its address.
	const sema::ExprFacts* right = WithoutParentheses(access.right)->facts;
	if (const syntax::UnaryExpr* address = MemberAddress(*access.right))
	{
		const sema::Entity* entity = WithoutParentheses(address->operand)->facts->entity;
		if (entity != nullptr && entity->kind == sema::EntityKind::Function &&
			!static_cast<const sema::FunctionEntity*>(entity)->is_virtual &&
			(right->conversion == nullptr || right->conversion->second == sema::ValueConversion::None))
		{
			const sema::FunctionEntity* target = static_cast<const sema::FunctionEntity*>(entity);
			lowering_.demand(target);
			return lowir::Value::symbol(lowering_.function_symbol(target));
		}
	}
	const sema::EnumeratorEntity* constant = right->entity != nullptr &&
			right->entity->kind == sema::EntityKind::Enumerator
		? static_cast<const sema::EnumeratorEntity*>(right->entity)
		: nullptr;
	if (constant != nullptr && constant->argument != nullptr && constant->argument->entity != nullptr &&
		(right->conversion == nullptr || right->conversion->second == sema::ValueConversion::None) &&
		!static_cast<const sema::FunctionEntity*>(constant->argument->entity)->is_virtual)
	{
		const sema::FunctionEntity* target = static_cast<const sema::FunctionEntity*>(constant->argument->entity);
		lowering_.demand(target);
		return address_of_symbol(lowering_.function_symbol(target));
	}
	// The value may come from any class's member (a 5.2.9p12 cast in
	// another translation unit reaches a base's pointer from a derived
	// class's virtual function at a non-zero offset), so both words are
	// always read, whatever the static class's layout.
	const RValue bits = operand(*access.right, ConversionContext::Operand);
	const RValue adjustment = convert_op(
		lowir::ConvertOp::Trunc, i64, binary(lowir::BinaryOp::Shr, i128, bits, integer(i128, 64)));
	const RValue word = convert_op(lowir::ConvertOp::Trunc, i64, bits);
	const RValue is_virtual = compare(
		lowir::ComparePredicate::Ne, i64, binary(lowir::BinaryOp::And, i64, word, integer(i64, 1)), integer(i64, 0));
	arguments_[object_argument] = index(byte, ir_model::IPK_NONE, arguments_[object_argument], adjustment).value;
	Place callee = Place();
	callee.storage = lowir::Value::slot(generated_slot(lowir::SlotKind::MemberPointerCallee, pointer));
	const lowir::BlockId virtual_block = new_block(lowir::BlockKind::MemberPointerVirtual);
	const lowir::BlockId direct_block = new_block(lowir::BlockKind::MemberPointerDirect);
	const lowir::BlockId end = new_block(lowir::BlockKind::MemberPointerEnd);
	branch(is_virtual, virtual_block, direct_block);
	place(virtual_block);
	Place vptr = Place();
	vptr.storage = arguments_[object_argument];
	Place entry = Place();
	const lowir::Value table = load(pointer, vptr).value;
	entry.storage =
		index(byte, ir_model::IPK_NONE, table, binary(lowir::BinaryOp::Sub, i64, word, integer(i64, 1))).value;
	store(pointer, load(pointer, entry), callee);
	jump(end);
	place(direct_block);
	store(pointer, copy(pointer, word), callee);
	jump(end);
	place(end);
	return load(pointer, callee).value;
}

} // namespace lower
