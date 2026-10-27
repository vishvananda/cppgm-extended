// GNU type traits (lex/gnu_builtins.h), which hosted standard library
// headers use for C++11's 20.9.4 traits: predicates on the analysis's class
// facts, and the initializations and assignments the constructible and
// assignable traits ask about, analyzed in a soft substitution of their own
// (an invalid one is the trait's false, not an error).

#include "sema/analyzer.h"

#include <algorithm>
#include <cstring>

#include "lex/identifier_table.h"
#include "sema/aggregates.h"
#include "sema/conversions.h"
#include "sema/integral_arithmetic.h"
#include "sema/instantiation_context.h"

namespace sema {

namespace {

// A scalar type, or a GNU vector or complex type, which the class-property
// traits (trivial, standard-layout, POD ...) answer for as for a scalar
// (both host compilers).
bool ScalarLike(const Type* type)
{
	type = type->unqualified;
	return IsScalar(type) || type->kind == TypeKind::Vector || type->kind == TypeKind::Complex;
}

const Type* WithoutArrays(const Type* type)
{
	while (type->kind == TypeKind::Array)
		type = type->element;
	return type;
}

ClassEntity* ClassType(const Type* type)
{
	type = type->unqualified;
	return type->kind == TypeKind::Class ? static_cast<ClassEntity*>(type->entity) : nullptr;
}

bool IsNullptrType(const Type* type)
{
	type = type->unqualified;
	return type->kind == TypeKind::Fundamental && type->fundamental == FundamentalType::NullptrT;
}

bool IsUnion(const ClassEntity* entity)
{
	return entity->key == TokenKind::KW_UNION;
}

// 8.4.2p4: declared, neither defaulted nor deleted on its first
// declaration.
bool UserProvidedConstructor(const FunctionEntity* constructor)
{
	return !constructor->implicit && !constructor->defaulted && !constructor->deleted;
}

} // namespace

ExprFacts* Analyzer::builtin_trait(const syntax::BuiltinTraitExpr& expr, Scope* scope)
{
	if (expr.trait == GnuTrait::DeclvalPrvalue)
		return new_facts(expr, expr.type, ValueCategory::PRValue);
	if (expr.trait == GnuTrait::OffsetofBase)
	{
		const Type* type = type_id(*expr.types[0], scope);
		return type == nullptr ? nullptr : new_facts(expr, type, ValueCategory::LValue);
	}
	if (expr.trait == GnuTrait::Offsetof)
	{
		uint64_t offset = 0;
		if (!offsetof_value(expr, scope, offset))
			return nullptr;
		ExprFacts* facts = new_facts(
			expr, types_.fundamental(FundamentalType::UnsignedLongInt), ValueCategory::PRValue);
		facts->value = offset;
		facts->flags |= kFolded;
		return facts;
	}
	if (expr.trait == GnuTrait::Declval)
	{
		// std::declval<T>(): an lvalue of T& 's referent or of a function
		// (5p6: T&& names a function as an lvalue), else an xvalue.
		const Type* type = expr.type;
		ValueCategory category = ValueCategory::XValue;
		if (type->kind == TypeKind::LValueReference || type->kind == TypeKind::Function ||
			(type->kind == TypeKind::RValueReference && type->element->kind == TypeKind::Function))
			category = ValueCategory::LValue;
		if (type->is_reference())
			type = type->element;
		return new_facts(expr, type, category);
	}
	if (expr.trait == GnuTrait::TypeofOperand && expr.operand == nullptr)
	{
		// GNU typeof of a type-id: that type, as the decltype of an
		// expression of the category a reference type gives (facts never
		// hold a reference).
		const Type* type = type_id(*expr.types[0], scope);
		if (type == nullptr)
			return nullptr;
		ValueCategory category = ValueCategory::PRValue;
		if (type->kind == TypeKind::LValueReference)
			category = ValueCategory::LValue;
		else if (type->kind == TypeKind::RValueReference)
			category = ValueCategory::XValue;
		return new_facts(expr, type->is_reference() ? type->element : type, category);
	}
	if (expr.trait == GnuTrait::TypeofOperand)
	{
		// GNU typeof (an unevaluated operand): the expression's type, cv
		// kept (facts never hold a reference).
		const ExprFacts* operand = typed_value(*expr.operand, scope);
		if (operand == nullptr)
			return nullptr;
		// A class prvalue operand is a temporary (12.2p1; no decltype
		// exemption for a call, as GCC and Clang have it).
		check_temporary_destruction(operand, expr.location);
		return new_facts(expr, operand->type, ValueCategory::PRValue);
	}
	if (expr.trait == GnuTrait::ConvertVector)
	{
		// Clang: a vector operand and a vector type of as many lanes, each
		// lane converted to the type's (4.7-4.9; to bool, 4.12): a prvalue.
		const ExprFacts* vector = typed_value(*expr.operand, scope);
		const Type* type = type_id(*expr.types[0], scope);
		if (vector == nullptr || type == nullptr)
			return nullptr;
		const Type* from = vector->type->unqualified;
		const Type* to = type->unqualified;
		if (dependent_type(from) || dependent_type(to))
			return new_facts(expr, to, ValueCategory::PRValue);
		if (from->kind != TypeKind::Vector || to->kind != TypeKind::Vector || from->bound != to->bound)
		{
			substitution_error(expr.location, "__builtin_convertvector of no vector, or to a vector of other lanes");
			return nullptr;
		}
		convert(*expr.operand, from, false);
		return new_facts(expr, to, ValueCategory::PRValue);
	}
	if (expr.trait == GnuTrait::BitCast)
	{
		// GCC and Clang (C++20's std::bit_cast): the operand's object
		// representation as a value of the type, both trivially copyable
		// and of one size: a prvalue.
		const ExprFacts* operand = typed_value(*expr.operand, scope);
		const Type* type = type_id(*expr.types[0], scope);
		if (operand == nullptr || type == nullptr)
			return nullptr;
		const Type* to = type->unqualified;
		const Type* from = operand->type->unqualified;
		if (dependent_type(from) || dependent_type(to))
			return new_facts(expr, to, ValueCategory::PRValue);
		uint64_t to_size = 0, from_size = 0, alignment = 0;
		if (to->is_reference() || to->kind == TypeKind::Array || to->kind == TypeKind::Function ||
			!object_layout(to, to_size, alignment) || !object_layout(from, from_size, alignment) ||
			to_size != from_size || !trivially_copyable(to) || !trivially_copyable(from))
		{
			substitution_error(
				expr.location, "__builtin_bit_cast between types of different sizes or not trivially copyable");
			return nullptr;
		}
		// An array contributes its object bytes; it does not decay.
		if (from->kind != TypeKind::Array)
			convert(*expr.operand, from, false);
		return new_facts(expr, to, ValueCategory::PRValue);
	}
	if (expr.trait == GnuTrait::VaArg)
	{
		// The list an lvalue of __builtin_va_list (or a parameter of that
		// type, adjusted); a scalar or complete trivially copyable class
		// argument type (passed as its bytes, 5.2.2p7; a reference, and a
		// class passed through an address, are not read), its value a
		// prvalue.
		const ExprFacts* list = typed_value(*expr.operand, scope);
		const Type* type = type_id(*expr.types[0], scope);
		if (list == nullptr || type == nullptr)
			return nullptr;
		const Type* tag = list->type->kind == TypeKind::Array || list->type->kind == TypeKind::Pointer
			? list->type->element
			: list->type;
		if (ClassOf(tag) == nullptr || tag->unqualified->entity->name == nullptr ||
			std::strcmp(tag->unqualified->entity->name->spelling, "__va_list_tag") != 0)
			error(expr.operand->location, "a variadic argument builtin's operand is no __builtin_va_list");
		uint64_t size = 0, alignment = 0;
		if (template_depth_ == 0 && !IsScalar(type->unqualified) &&
			(ClassOf(type) == nullptr || !object_layout(type->unqualified, size, alignment) ||
				!trivially_copyable(type->unqualified)))
			error(expr.location, "__builtin_va_arg of a type that is no scalar or trivially copyable class type");
		return new_facts(expr, type->unqualified, ValueCategory::PRValue);
	}
	if (expr.trait == GnuTrait::RealPart || expr.trait == GnuTrait::ImagPart)
	{
		// GNU: a complex operand's real or imaginary part, of its category;
		// an arithmetic operand's value, and zero (a prvalue).
		const ExprFacts* operand = typed(*expr.operand, scope);
		if (operand == nullptr)
			return nullptr;
		const Type* type = operand->type->unqualified;
		if (type->kind == TypeKind::Complex)
			return new_facts(expr, types_.qualified(type->element, operand->type->cv), operand->category);
		if (!IsArithmetic(type))
			error(expr.location, "__real__ or __imag__ of an operand that is no arithmetic or complex value");
		return new_facts(
			expr, operand->type, expr.trait == GnuTrait::RealPart ? operand->category : ValueCategory::PRValue);
	}
	if (expr.trait == GnuTrait::BitInt)
	{
		// Typed only where its width is known (no template's definition).
		substitution_error(expr.location, "a _BitInt type is not supported");
		return nullptr;
	}
	if (expr.trait == GnuTrait::IntegerPack)
	{
		// Only a template argument pack expansion's pattern expands it
		// (expanded_argument).
		substitution_error(expr.location, "__integer_pack outside a template argument pack expansion");
		return nullptr;
	}
	SmallList<const Type*, 4> types;
	if (!trait_types(expr, scope, types))
		return nullptr;
	if (IsGnuTypeTransform(expr.trait))
	{
		// The resulting type as the decltype of an operand of the category
		// a reference type gives (facts never hold a reference; cv kept;
		// an underlying type a prvalue's).
		const Type* type = trait_type(expr.trait, types[0], expr.location);
		if (type == nullptr)
			return nullptr;
		ValueCategory category = ValueCategory::PRValue;
		if (type->kind == TypeKind::LValueReference)
			category = ValueCategory::LValue;
		else if (type->kind == TypeKind::RValueReference)
			category = ValueCategory::XValue;
		return new_facts(expr, type->is_reference() ? type->element : type, category);
	}
	uint64_t value = 0;
	if (!trait_value(expr.trait, types.data(), static_cast<uint32_t>(types.size()), expr.location, scope, value))
		return nullptr;
	ExprFacts* facts = new_facts(expr, trait_result_type(expr.trait), ValueCategory::PRValue);
	facts->value = value;
	facts->flags |= kFolded;
	return facts;
}

const Type* Analyzer::trait_result_type(GnuTrait trait)
{
	// bool, but std::size_t for __array_rank.
	return types_.fundamental(trait == GnuTrait::ArrayRank ? FundamentalType::UnsignedLongInt : FundamentalType::Bool);
}

namespace {

// 20.9.7.3: the signed or unsigned integer type of an integral type's
// rank (a character type's or enumeration's: the smallest of its size).
FundamentalType SignedCounterpart(FundamentalType type, uint64_t size, bool to_signed)
{
	static const FundamentalType kSigned[] = { FundamentalType::SignedChar, FundamentalType::ShortInt,
		FundamentalType::Int, FundamentalType::LongInt, FundamentalType::LongLongInt, FundamentalType::Int128 };
	static const FundamentalType kUnsigned[] = { FundamentalType::UnsignedChar, FundamentalType::UnsignedShortInt,
		FundamentalType::UnsignedInt, FundamentalType::UnsignedLongInt, FundamentalType::UnsignedLongLongInt,
		FundamentalType::UnsignedInt128 };
	for (unsigned i = 0; i < 6; ++i)
	{
		if (type == kSigned[i] || type == kUnsigned[i])
			return to_signed ? kSigned[i] : kUnsigned[i];
	}
	switch (size)
	{
	case 1:
		return to_signed ? FundamentalType::SignedChar : FundamentalType::UnsignedChar;
	case 2:
		return to_signed ? FundamentalType::ShortInt : FundamentalType::UnsignedShortInt;
	case 4:
		return to_signed ? FundamentalType::Int : FundamentalType::UnsignedInt;
	case 8:
		return to_signed ? FundamentalType::LongInt : FundamentalType::UnsignedLongInt;
	default:
		return to_signed ? FundamentalType::Int128 : FundamentalType::UnsignedInt128;
	}
}

} // namespace

const Type* Analyzer::trait_type(GnuTrait trait, const Type* type, uint32_t location)
{
	// A type transform's resulting type: the underlying type of a complete
	// enumeration type (7.2p5), or a 20.9.7 transformation's.
	if (trait != GnuTrait::UnderlyingType)
		return transformed_type(trait, type, location);
	const EnumEntity* entity = type->unqualified->kind == TypeKind::Enum
		? static_cast<const EnumEntity*>(type->unqualified->entity)
		: nullptr;
	if (entity == nullptr || entity->underlying == nullptr)
	{
		substitution_error(location, "__underlying_type of a type that is no complete enumeration");
		return nullptr;
	}
	// Not fixed, it is implementation-defined (7.2p6): GCC's and Clang's
	// is unsigned when no enumerator is negative (unsigned int for `enum
	// { e }`, unsigned long beyond it). `underlying` keeps the type the
	// enumeration promotes to (4.5p3: int first), which its values,
	// conversions and storage use.
	if (entity->fixed || entity->negative)
		return entity->underlying;
	return types_.fundamental(SignedCounterpart(entity->underlying->fundamental, 0, false));
}

const Type* Analyzer::transformed_type(GnuTrait trait, const Type* type, uint32_t location)
{
	// 20.9.7's transformations by their GNU builtins' names, as Clang
	// defines them (an add_*_reference or add_pointer of a type no
	// reference can name, void and a function type with cv- or
	// ref-qualifiers, is that type; CWG 2101). Null for a substitution's
	// failure (make_signed of a type no integral or enumeration type).
	const Type* bare = type->unqualified;
	const bool abominable = bare->kind == TypeKind::Function &&
		(bare->function_cv != 0 || bare->ref != RefQualifier::None);
	const bool referenceable = !abominable && bare != types_.fundamental(FundamentalType::Void);
	const Type* referee = type->is_reference() ? type->element : type;
	switch (trait)
	{
	case GnuTrait::AddLvalueReference:
	case GnuTrait::AddRvalueReference:
		return referenceable ? types_.reference(type, trait == GnuTrait::AddRvalueReference) : type;
	case GnuTrait::AddPointer:
		return referenceable || bare == types_.fundamental(FundamentalType::Void) ? types_.pointer(referee) : type;
	case GnuTrait::Decay:
		// 20.9.7.6: an array's element pointer, a function's pointer (an
		// abominable one's add_pointer is itself, GCC's answer), otherwise
		// without cv-qualifiers.
		if (referee->kind == TypeKind::Array)
			return types_.pointer(referee->element);
		if (referee->unqualified->kind == TypeKind::Function)
			return abominable ? type : types_.pointer(referee->unqualified);
		return referee->unqualified;
	case GnuTrait::RemoveAllExtents:
		while (type->kind == TypeKind::Array)
			type = type->element;
		return type;
	case GnuTrait::RemoveExtent:
		return type->kind == TypeKind::Array ? type->element : type;
	case GnuTrait::RemoveConst:
		return without_cv(type, kConst);
	case GnuTrait::RemoveVolatile:
		return without_cv(type, kVolatile);
	case GnuTrait::RemoveCv:
		return without_cv(type, kConst | kVolatile);
	case GnuTrait::RemoveCvref:
		return without_cv(referee, kConst | kVolatile);
	case GnuTrait::RemovePointer:
		return bare->kind == TypeKind::Pointer ? bare->element : type;
	case GnuTrait::RemoveReference:
	case GnuTrait::RemoveReferenceGnu:
		return referee;
	case GnuTrait::RemoveRestrict:
		return type;
	case GnuTrait::AddAtomic:
		// C11 6.7.2.4p3: not of an array, function, atomic or
		// cv-qualified type (a reference names no object type).
		if (type->kind == TypeKind::Array || type->kind == TypeKind::Function || type->is_reference() ||
			type->cv != 0 || IsVoid(type))
		{
			substitution_error(location, "_Atomic of an array, function, reference, void or qualified type");
			return nullptr;
		}
		return types_.qualified(type, kAtomic);
	case GnuTrait::MakeSigned:
	case GnuTrait::MakeUnsigned:
	{
		const Type* integral = bare;
		if (integral->kind == TypeKind::Enum)
		{
			const EnumEntity* entity = static_cast<const EnumEntity*>(integral->entity);
			integral = entity->underlying != nullptr ? entity->underlying : nullptr;
		}
		uint64_t size = 0, alignment = 0;
		if (integral == nullptr || !IsIntegralType(integral) || integral->fundamental == FundamentalType::Bool ||
			!type_size(integral, size, alignment, location))
		{
			substitution_error(
				location, "make_signed or make_unsigned of a type that is no integral or enumeration type");
			return nullptr;
		}
		// An enumeration's counterpart is by its size (20.9.7.3), not
		// its underlying type's rank.
		const FundamentalType from = bare->kind == TypeKind::Enum ? FundamentalType::Bool : integral->fundamental;
		const Type* result = types_.fundamental(SignedCounterpart(from, size, trait == GnuTrait::MakeSigned));
		return types_.qualified(result, type->cv);
	}
	default:
		return type;
	}
}

bool Analyzer::trait_types(const syntax::BuiltinTraitExpr& expr, Scope* scope, SmallList<const Type*, 4>& out)
{
	for (const syntax::TypeId* id : expr.types)
	{
		const Type* type = type_id(*id, scope);
		if (type == nullptr)
			return false;
		out.push_back(type);
	}
	return true;
}

bool Analyzer::trait_constant(const syntax::BuiltinTraitExpr& expr, Scope* scope, Constant& value)
{
	// A trait read before semantics mode types it (a template definition's
	// arguments, types mode): a constant unless an operand is dependent.
	if (expr.trait >= GnuTrait::UnderlyingType)
		return false;
	SmallList<const Type*, 4> types;
	if (!trait_types(expr, scope, types))
		return false;
	for (const Type* type : types)
	{
		if (dependent_type(type))
			return false;
	}
	if (expr.trait == GnuTrait::Offsetof)
	{
		// Its designator's subscripts are typed in semantics mode only.
		uint64_t offset = 0;
		if (!semantic() || template_depth_ != 0 || !offsetof_value(expr, scope, offset))
			return false;
		value = Constant{ offset, types_.fundamental(FundamentalType::UnsignedLongInt) };
		return true;
	}
	uint64_t result = 0;
	if (!trait_value(expr.trait, types.data(), static_cast<uint32_t>(types.size()), expr.location, scope, result))
		return false;
	value = Constant{ result, trait_result_type(expr.trait) };
	return true;
}

bool Analyzer::trait_value(
	GnuTrait trait, const Type* const* types, uint32_t count, uint32_t location, Scope* scope, uint64_t& out)
{
	// The traits that ask about a class require it complete (14.7.1p1: a
	// specialization is instantiated); an incomplete one is an error, as
	// GCC has it. The shape traits read the type alone.
	const Type* type = types[0];
	ClassEntity* entity = ClassType(type);
	if (GnuTraitNeedsComplete(trait))
	{
		// The class-property traits ask about a class operand only (an
		// array is none, and every array is an aggregate, 8.5.1p1); an
		// array of unknown bound is not destructible (20.9.4.3). Their
		// element is not asked about (GCC and Clang alike).
		const bool unknown_bound = type->kind == TypeKind::Array && type->bound == 0 && type->owner == nullptr;
		const bool element_unasked = trait <= GnuTrait::IsAggregate ||
			(unknown_bound && trait >= GnuTrait::IsDestructible && trait <= GnuTrait::IsTriviallyDestructible);
		ClassEntity* object = ClassType(element_unasked ? type : WithoutArrays(type));
		if (object != nullptr && !require_complete(object))
			return substitution_error(location, "a type trait of an incomplete class type");
	}
	if (trait < GnuTrait::IsEmpty)
	{
		out = shape_trait(trait, type) ? 1 : 0;
		return true;
	}
	bool value = false;
	switch (trait)
	{
	case GnuTrait::IsEmpty:
		value = entity != nullptr && !IsUnion(entity) && entity->empty;
		break;
	case GnuTrait::IsPolymorphic:
		value = entity != nullptr && entity->polymorphic;
		break;
	case GnuTrait::IsAbstract:
		value = entity != nullptr && entity->virtuals != nullptr && entity->virtuals->pure_count != 0;
		break;
	case GnuTrait::IsFinal:
		value = entity != nullptr && entity->is_final;
		break;
	case GnuTrait::IsAggregate:
		// 8.5.1p1: an array or an aggregate class; a GNU vector type is one
		// too (both host compilers; Clang's complex types, GCC's not).
		value = type->kind == TypeKind::Array || type->unqualified->kind == TypeKind::Vector ||
			(entity != nullptr && IsAggregateClass(entity));
		break;
	case GnuTrait::IsTrivial:
		value = trivially_copyable(type) && trivially_default_constructible(type);
		break;
	case GnuTrait::IsTriviallyCopyable:
		value = trivially_copyable(type);
		break;
	case GnuTrait::HasUniqueObjectRepresentations:
		value = unique_object_representations(type);
		break;
	case GnuTrait::IsStandardLayout:
		value = standard_layout(type);
		break;
	case GnuTrait::IsPod:
		// 9p10, 3.9p9: trivial and standard-layout.
		value = trivially_copyable(type) && trivially_default_constructible(type) && standard_layout(type);
		break;
	case GnuTrait::IsLiteralType:
		value = trait_literal_type(type);
		break;
	case GnuTrait::HasTrivialDestructor:
	{
		// GCC: a trivial type or a reference, or a class (array of class)
		// whose destructor is trivial.
		if (type->is_reference() || (trivially_copyable(type) && trivially_default_constructible(type)))
		{
			value = true;
			break;
		}
		const ClassEntity* object = ClassType(WithoutArrays(type));
		value = object != nullptr && (object->destructor == nullptr || object->destructor->trivial);
		break;
	}
	case GnuTrait::HasVirtualDestructor:
		value = entity != nullptr && entity->destructor != nullptr && entity->destructor->is_virtual;
		break;
	case GnuTrait::IsDestructible:
	case GnuTrait::IsNothrowDestructible:
	case GnuTrait::IsTriviallyDestructible:
	{
		bool nothrow = false, trivial = false;
		value = trait_destructible(type, location, scope, nothrow, trivial);
		if (trait == GnuTrait::IsNothrowDestructible)
			value = value && nothrow;
		else if (trait == GnuTrait::IsTriviallyDestructible)
			value = value && trivial;
		break;
	}
	case GnuTrait::HasTrivialConstructor:
	case GnuTrait::HasTrivialCopy:
	case GnuTrait::HasTrivialAssign:
	case GnuTrait::HasNothrowConstructor:
	case GnuTrait::HasNothrowCopy:
	case GnuTrait::HasNothrowAssign:
		value = legacy_trait(trait, type);
		break;
	case GnuTrait::ArrayRank:
	{
		// The number of array dimensions (std::rank).
		uint64_t rank = 0;
		for (const Type* array = type; array->kind == TypeKind::Array; array = array->element)
			++rank;
		out = rank;
		return true;
	}
	case GnuTrait::IsSame:
		// The same type, cv-qualifiers included (types are interned).
		value = type->canonical == types[1]->canonical;
		break;
	case GnuTrait::IsBaseOf:
	{
		// A class is its own base here; a different derived class must be
		// complete. Unions are no classes of this trait, and neither is a
		// reference that names a class.
		ClassEntity* base = type->is_reference() ? nullptr : entity;
		ClassEntity* derived = types[1]->is_reference() ? nullptr : ClassType(types[1]);
		if (base == nullptr || derived == nullptr || IsUnion(base) || IsUnion(derived))
			break;
		if (base == derived)
		{
			value = true;
			break;
		}
		if (!require_complete(derived))
			return substitution_error(location, "__is_base_of of an incomplete derived class");
		value = conversions_.is_base_of(base, derived);
		break;
	}
	case GnuTrait::IsAssignable:
	case GnuTrait::IsNothrowAssignable:
	case GnuTrait::IsTriviallyAssignable:
	{
		bool nothrow = false, trivial = false;
		value = trait_assignable(types[0], types[1], location, scope, nothrow, trivial);
		if (trait == GnuTrait::IsNothrowAssignable)
			value = value && nothrow;
		else if (trait == GnuTrait::IsTriviallyAssignable)
			value = value && trivial;
		break;
	}
	case GnuTrait::IsConvertible:
	case GnuTrait::IsConvertibleTo:
	case GnuTrait::IsNothrowConvertible:
	{
		bool nothrow = false;
		value = trait_convertible(types[0], types[1], location, scope, nothrow);
		if (trait == GnuTrait::IsNothrowConvertible)
			value = value && nothrow;
		break;
	}
	case GnuTrait::ReferenceBindsToTemporary:
	case GnuTrait::ReferenceConstructsFromTemporary:
	case GnuTrait::ReferenceConvertsFromTemporary:
		value = trait_temporary_binding(trait, types[0], types[1], location, scope);
		break;
	case GnuTrait::IsConstructible:
	case GnuTrait::IsNothrowConstructible:
	case GnuTrait::IsTriviallyConstructible:
	{
		bool nothrow = false, trivial = false;
		value = trait_constructible(types[0], types + 1, count - 1, location, scope, nothrow, trivial);
		if (trait == GnuTrait::IsNothrowConstructible)
			value = value && nothrow;
		else if (trait == GnuTrait::IsTriviallyConstructible)
			value = value && trivial;
		break;
	}
	case GnuTrait::IsInvocable:
	case GnuTrait::IsNothrowInvocable:
	{
		bool nothrow = false;
		value = trait_invocable(types, count, location, scope, nothrow);
		if (trait == GnuTrait::IsNothrowInvocable)
			value = value && nothrow;
		break;
	}
	default:
		return false;
	}
	out = value ? 1 : 0;
	return true;
}

bool Analyzer::offsetof_value(const syntax::BuiltinTraitExpr& expr, Scope* scope, uint64_t& offset)
{
	// 18.2p4: the offset in bytes of the designated member (or element)
	// from the start of an object of the type: its designator typed as
	// member accesses and subscripts of such an object, then summed --
	// data members by their class's layout (through the non-virtual base
	// and anonymous member objects that hold them), elements by their
	// constant indexes. A bit-field has no offset.
	const ExprFacts* designator = typed(*expr.operand, scope);
	if (designator == nullptr || failed())
		return false;
	offset = 0;
	for (const syntax::Expr* e = expr.operand; e->kind != syntax::ExprKind::BuiltinTrait;)
	{
		if (e->kind == syntax::ExprKind::Subscript)
		{
			const syntax::SubscriptExpr& subscript = static_cast<const syntax::SubscriptExpr&>(*e);
			Constant index;
			uint64_t size = 0, alignment = 0;
			if (!integral_constant(*subscript.index, scope, index) ||
				!type_size(e->facts->type, size, alignment, e->location))
				return substitution_error(e->location, "an offsetof subscript that is no constant");
			offset += index.bits * size;
			e = subscript.base;
			continue;
		}
		const syntax::MemberExpr& access = static_cast<const syntax::MemberExpr&>(*e);
		const ClassEntity* cls = ClassOf(access.base->facts->type);
		const Entity* member = e->facts->entity;
		uint64_t bytes = 0;
		if (cls == nullptr || member == nullptr || member->kind != EntityKind::Variable ||
			!member_offset(const_cast<ClassEntity*>(cls), member, bytes, e->location))
			return substitution_error(e->location, "an offsetof designator naming no data member");
		offset += bytes;
		e = access.base;
	}
	return true;
}

bool Analyzer::member_offset(ClassEntity* cls, const Entity* member, uint64_t& bytes, uint32_t location)
{
	uint64_t size = 0, alignment = 0;
	if (!type_size(cls->type, size, alignment, location))
		return false;
	for (const DataMember* data = cls->members; data != nullptr; data = data->next)
	{
		if (data->variable == member)
		{
			if (data->bit_field)
				return substitution_error(location, "offsetof of a bit-field");
			bytes = data->offset / 8;
			return true;
		}
		// An anonymous union's or struct's object holds its members.
		ClassEntity* anonymous = data->variable->name == nullptr ? ClassType(data->variable->type) : nullptr;
		uint64_t inner = 0;
		if (anonymous != nullptr && member_offset(anonymous, member, inner, location))
		{
			bytes = data->offset / 8 + inner;
			return true;
		}
	}
	for (const BaseClass* base = cls->bases; base != nullptr; base = base->next)
	{
		uint64_t inner = 0;
		if (!base->is_virtual && member_offset(ClassType(base->base->type), member, inner, location))
		{
			bytes = base->offset + inner;
			return true;
		}
	}
	return false;
}

bool Analyzer::shape_trait(GnuTrait trait, const Type* type)
{
	// The primary and composite type categories (20.9.4.1, 20.9.4.2) and
	// type properties (20.9.4.3) that read the type's form alone.
	// cv-qualification belongs to the type: a reference has none, an array
	// its elements'.
	const Type* bare = type->unqualified;
	const bool function = type->kind == TypeKind::Function;
	const bool void_type = IsVoid(type);
	const bool fundamental = IsArithmetic(bare) || void_type || IsNullptrType(bare);
	switch (trait)
	{
	case GnuTrait::IsClass:
		return ClassType(type) != nullptr && !IsUnion(ClassType(type));
	case GnuTrait::IsEnum:
		return bare->kind == TypeKind::Enum;
	case GnuTrait::IsUnion:
		return ClassType(type) != nullptr && IsUnion(ClassType(type));
	case GnuTrait::IsVoid:
		return void_type;
	case GnuTrait::IsNullPointer:
		return IsNullptrType(bare);
	case GnuTrait::IsIntegral:
		return IsIntegralType(bare);
	case GnuTrait::IsFloatingPoint:
		return IsFloating(bare);
	case GnuTrait::IsArray:
		return type->kind == TypeKind::Array;
	case GnuTrait::IsBoundedArray:
		return type->kind == TypeKind::Array && (type->bound != 0 || type->owner != nullptr);
	case GnuTrait::IsUnboundedArray:
		return type->kind == TypeKind::Array && type->bound == 0 && type->owner == nullptr;
	case GnuTrait::IsPointer:
		return bare->kind == TypeKind::Pointer;
	case GnuTrait::IsLvalueReference:
		return type->kind == TypeKind::LValueReference;
	case GnuTrait::IsRvalueReference:
		return type->kind == TypeKind::RValueReference;
	case GnuTrait::IsReference:
		return type->is_reference();
	case GnuTrait::IsMemberObjectPointer:
		return bare->kind == TypeKind::MemberPointer && bare->element->kind != TypeKind::Function;
	case GnuTrait::IsMemberFunctionPointer:
		return bare->kind == TypeKind::MemberPointer && bare->element->kind == TypeKind::Function;
	case GnuTrait::IsMemberPointer:
		return bare->kind == TypeKind::MemberPointer;
	case GnuTrait::IsFunction:
		return function;
	case GnuTrait::IsArithmetic:
		return IsArithmetic(bare);
	case GnuTrait::IsFundamental:
		return fundamental;
	case GnuTrait::IsObject:
		return !function && !type->is_reference() && !void_type;
	case GnuTrait::IsScalar:
		return IsScalar(bare);
	case GnuTrait::IsCompound:
		return !fundamental;
	case GnuTrait::IsConst:
		return (WithoutArrays(type)->cv & kConst) != 0 && !type->is_reference();
	case GnuTrait::IsVolatile:
		return (WithoutArrays(type)->cv & kVolatile) != 0 && !type->is_reference();
	case GnuTrait::IsSigned:
	case GnuTrait::IsUnsigned:
	{
		// An arithmetic type for which T(-1) < T(0) holds, or does not: a
		// floating type is signed; an enumeration is neither.
		IntegralKind kind;
		if (IsFloating(bare))
			return trait == GnuTrait::IsSigned;
		if (!IsIntegralType(bare) || !IntegralKindOf(bare, kind))
			return false;
		return kind.is_signed == (trait == GnuTrait::IsSigned);
	}
	case GnuTrait::IsReferenceable:
		// An object or reference type, or a function type without cv- or
		// ref-qualifiers (17.3.20).
		return !void_type && !(function && (type->function_cv != 0 || type->ref != RefQualifier::None));
	case GnuTrait::IsScopedEnum:
		return bare->kind == TypeKind::Enum && IsScopedEnumeration(bare);
	default:
		return false;
	}
}

bool Analyzer::trait_destructible(const Type* type, uint32_t location, Scope* scope, bool& nothrow, bool& trivial)
{
	// 20.9.4.3: a reference type, or an object type whose
	// remove_all_extents U makes `std::declval<U&>().~U()` well-formed
	// (not an array of unknown bound); `nothrow` when that destructor
	// cannot throw, `trivial` when it is trivial (12.4p5).
	nothrow = true;
	trivial = true;
	if (type->is_reference())
		return true;
	if (IsVoid(type) || type->kind == TypeKind::Function ||
		(type->kind == TypeKind::Array && type->bound == 0 && type->owner == nullptr))
		return false;
	const Type* object = WithoutArrays(type);
	ClassEntity* cls = ClassType(object);
	if (cls == nullptr)
		return true;
	if (!require_complete(cls) || cls->scope == nullptr)
		return false;
	{
		SoftSubstitution soft(*this);
		check_destruction(object, scope, location);
		if (soft.failed())
			return false;
	}
	if (cls->destructor != nullptr)
	{
		nothrow = function_nothrow(cls->destructor);
		trivial = cls->destructor->trivial;
	}
	return true;
}

bool Analyzer::trait_convertible(const Type* from, const Type* to, uint32_t location, Scope* scope, bool& nothrow)
{
	// 20.9.6p4: `To test() { return std::declval<From>(); }` is well-formed,
	// an implicit conversion (copy-initialization, 8.5p15) of To from the
	// value; void converts to void only, and no array or function type is
	// returned (8.3.5p8).
	nothrow = true;
	if (IsVoid(to) || IsVoid(from))
		return IsVoid(to) && IsVoid(from);
	if (to->kind == TypeKind::Array || to->kind == TypeKind::Function)
		return false;
	if (from->kind == TypeKind::Function && (from->function_cv != 0 || from->ref != RefQualifier::None))
		return false;
	// 10.4p3: nor an abstract class.
	const ClassEntity* returned = to->is_reference() ? nullptr : ClassType(to);
	if (returned != nullptr && require_complete(const_cast<ClassEntity*>(returned)) && returned->virtuals != nullptr &&
		returned->virtuals->pure_count != 0)
		return false;
	syntax::Expr* operand = declval_operand(from, location);
	syntax::Initializer init = syntax::Initializer();
	init.kind = syntax::InitializerKind::Assign;
	init.location = location;
	init.arguments = syntax::Slice<syntax::Expr*>{ &operand, 1 };
	init.expression = operand;
	{
		SoftSubstitution soft(*this);
		++unevaluated_;
		const uint32_t saved_throwing = potentially_throwing_;
		BodyContext* saved = body_;
		BodyContext context = BodyContext();
		if (body_ == nullptr)
			body_ = &context;
		const FunctionEntity* constructor = initialize_object(to, init, scope, location);
		// The returned object is destroyed by the caller (whose destructor
		// may throw: no nothrow conversion, both host compilers).
		if (!soft.failed())
			check_destruction(to, scope, location);
		const ClassEntity* destroyed = to->is_reference() ? nullptr : ClassType(WithoutArrays(to));
		if (destroyed != nullptr && destroyed->destructor != nullptr && !function_nothrow(destroyed->destructor))
			nothrow = false;
		body_ = saved;
		potentially_throwing_ = saved_throwing;
		--unevaluated_;
		if (soft.failed())
			return false;
		nothrow = nothrow && (constructor == nullptr || function_nothrow(constructor));
	}
	const Conversion* conversion = operand->facts != nullptr ? operand->facts->conversion : nullptr;
	nothrow = nothrow && (conversion == nullptr || !conversion_may_throw(conversion, 0));
	return true;
}

bool Analyzer::trait_temporary_binding(
	GnuTrait trait, const Type* target, const Type* source, uint32_t location, Scope* scope)
{
	// Clang's __reference_binds_to_temporary (T, U) and C++23's
	// reference_constructs_from_temporary: T is a reference type and
	// `T t(VAL<U>);` -- VAL<U> a prvalue of a non-reference U, as Clang
	// reads both -- binds t to a temporary (12.2p1), one a conversion
	// creates or a class or array prvalue bound directly (8.5.3p5);
	// reference_converts_from_temporary by copy-initialization.
	if (!target->is_reference() || IsVoid(source) || source->kind == TypeKind::Function)
		return false;
	const bool prvalue = !source->is_reference();
	// A prvalue of a non-class, non-array type has its type without
	// cv-qualifiers (3.10p4): `const int` binds int&& to a temporary.
	if (prvalue && source->kind != TypeKind::Array && ClassType(source) == nullptr)
		source = source->unqualified;
	syntax::Expr* operand = declval_operand(source, location, prvalue);
	syntax::Initializer init = syntax::Initializer();
	init.kind = trait == GnuTrait::ReferenceConvertsFromTemporary ? syntax::InitializerKind::Assign
																  : syntax::InitializerKind::Parenthesized;
	init.location = location;
	init.arguments = syntax::Slice<syntax::Expr*>{ &operand, 1 };
	init.expression = operand;
	{
		SoftSubstitution soft(*this);
		++unevaluated_;
		BodyContext* saved = body_;
		BodyContext context = BodyContext();
		if (body_ == nullptr)
			body_ = &context;
		initialize_object(target, init, scope, location);
		body_ = saved;
		--unevaluated_;
		if (soft.failed())
			return false;
	}
	const Conversion* binding = operand->facts != nullptr ? operand->facts->conversion : nullptr;
	if (binding == nullptr || binding->form != ConversionForm::Reference)
		return false;
	const Type* referee = WithoutArrays(source);
	return binding->temporary ||
		(prvalue && (source->kind == TypeKind::Array || ClassType(referee) != nullptr) &&
			target->element->kind != TypeKind::Function);
}

bool Analyzer::trait_invocable(const Type* const* types, uint32_t count, uint32_t location, Scope* scope, bool& nothrow)
{
	// C++17's is_invocable (23.15.4.3): INVOKE(std::declval<Fn>(),
	// std::declval<Args>()...) (20.8.2p1) is well-formed as an unevaluated
	// operand; `nothrow` when it cannot throw. __builtin_invoke analyzes
	// INVOKE.
	nothrow = false;
	for (uint32_t i = 0; i < count; ++i)
	{
		if (IsVoid(types[i]) || (types[i]->kind == TypeKind::Function && i != 0))
			return false;
	}
	// An abominable function type (cv- or ref-qualified) has no value to
	// call (no declval of it).
	if (count != 0 && types[0]->kind == TypeKind::Function &&
		(types[0]->function_cv != 0 || types[0]->ref != RefQualifier::None))
		return false;
	// The callable type itself is complete (Table 44's precondition; GCC
	// rejects an incomplete class), a reference to one being complete.
	ClassEntity* callee = ClassType(types[0]);
	if (callee != nullptr && !types[0]->is_reference() && !require_complete(callee))
		return substitution_error(location, "a type trait of an incomplete class type");
	syntax::Expr** arguments = static_cast<syntax::Expr**>(
		arena_.allocate(sizeof(syntax::Expr*) * count, alignof(syntax::Expr*)));
	for (uint32_t i = 0; i < count; ++i)
		arguments[i] = declval_operand(types[i], location);
	syntax::CallExpr* call = static_cast<syntax::CallExpr*>(
		arena_.allocate(sizeof(syntax::CallExpr), alignof(syntax::CallExpr)));
	*call = syntax::CallExpr();
	call->kind = syntax::ExprKind::Call;
	call->location = location;
	call->arguments = syntax::Slice<syntax::Expr*>{ arguments, count };
	{
		SoftSubstitution soft(*this);
		++unevaluated_;
		const uint32_t saved_throwing = potentially_throwing_;
		BodyContext* saved = body_;
		BodyContext context = BodyContext();
		if (body_ == nullptr)
			body_ = &context;
		const ExprFacts* facts = builtin_invoke(*call, scope);
		body_ = saved;
		potentially_throwing_ = saved_throwing;
		--unevaluated_;
		if (facts == nullptr || soft.failed())
			return false;
	}
	nothrow = !may_throw(*call, 0);
	return true;
}

bool Analyzer::integer_pack_argument(const syntax::TemplateArgument& argument, Scope* scope, TypeList& out)
{
	// A template argument pack expansion whose pattern is `__integer_pack
	// (N)` (GCC's): the values 0, 1, ..., N-1 of N's type once N is known;
	// in a template definition with N dependent, an expansion of its
	// placeholder. False when the argument is no such expansion.
	const syntax::Expr* pattern = argument.expression;
	while (pattern != nullptr && pattern->kind == syntax::ExprKind::Paren)
		pattern = static_cast<const syntax::ParenExpr*>(pattern)->inner;
	if (pattern == nullptr || pattern->kind != syntax::ExprKind::BuiltinTrait ||
		static_cast<const syntax::BuiltinTraitExpr*>(pattern)->trait != GnuTrait::IntegerPack)
		return false;
	const syntax::Expr& count = *static_cast<const syntax::BuiltinTraitExpr*>(pattern)->operand;
	const Type* value = template_value(count, scope);
	if (value == nullptr)
		return true;
	if (value->kind != TypeKind::Value)
	{
		out.push_back(types_.expansion(value));
		return true;
	}
	const Type* type = value->element;
	const uint64_t bound = value->bound;
	if (IsSignedConstant(Constant{ bound, type }) && static_cast<int64_t>(bound) < 0)
	{
		substitution_error(count.location, "__integer_pack of a negative count");
		return true;
	}
	for (uint64_t i = 0; i < bound; ++i)
		out.push_back(types_.value(type, i));
	return true;
}

bool Analyzer::trivially_copyable(const Type* type)
{
	// 3.9p9, 9p6: a scalar type (GNU vector and complex types too), a
	// trivially copyable class (no non-trivial copy or move constructor or
	// assignment operator, deleted ones by their triviality as GCC counts
	// them, and a trivial destructor), or an array of one.
	type = WithoutArrays(type);
	if (ScalarLike(type))
		return true;
	ClassEntity* entity = ClassType(type);
	if (entity == nullptr || type->is_reference() || !require_complete(entity) || entity->scope == nullptr)
		return false;
	if (entity->destructor != nullptr && !entity->destructor->trivial)
		return false;
	SmallList<const FunctionEntity*, 8> members;
	copy_members(entity, false, members);
	copy_members(entity, true, members);
	for (const FunctionEntity* function : members)
	{
		if (!copy_member_trivial(function, entity))
			return false;
	}
	return true;
}

bool Analyzer::unique_object_representations(const Type* type)
{
	// [meta.unary.prop]: arrays have their element's property, including
	// arrays of unknown bound. Every bit of a trivially copyable object
	// must contribute to its value; floating and reference types do not
	// provide that guarantee in the GCC-compatible x86-64 environment.
	type = WithoutArrays(type);
	uint64_t bits = 0, size = 0, alignment = 0;
	return representation_bits(type, bits) && object_layout(type, size, alignment) && size <= UINT64_MAX / 8 &&
		bits == size * 8;
}

bool Analyzer::representation_bits(const Type* type, uint64_t& bits)
{
	check_stack(0);
	type = type->unqualified;
	if (ClassEntity* entity = ClassType(type))
	{
		uint64_t size = 0, alignment = 0;
		if (!object_layout(type, size, alignment))
			return false;
		const uint32_t known = representation_index_.find(entity);
		if (known != PointerMap::kMissing)
		{
			bits = representation_bits_[known];
			return bits != UINT64_MAX;
		}
		bits = 0;
		if (!trivially_copyable(type) || !class_representation_bits(entity, bits))
			bits = UINT64_MAX;
		representation_index_.set(entity, static_cast<uint32_t>(representation_bits_.size()));
		representation_bits_.push_back(bits);
		return bits != UINT64_MAX;
	}
	if (type->kind == TypeKind::Array)
	{
		if (type->bound == 0 || !unique_object_representations(type->element))
			return false;
	}
	else if (type->kind == TypeKind::Vector)
	{
		if (!IsIntegralType(type->element))
			return false;
	}
	else if (!IsIntegralType(type) && type->kind != TypeKind::Enum && type->kind != TypeKind::Pointer &&
		type->kind != TypeKind::MemberPointer && type->kind != TypeKind::BlockPointer)
	{
		return false;
	}
	uint64_t size = 0, alignment = 0;
	if (!object_layout(type, size, alignment) || size > UINT64_MAX / 8)
		return false;
	bits = size * 8;
	return true;
}

bool Analyzer::class_representation_bits(ClassEntity* entity, uint64_t& bits)
{
	// Base tail padding may be reused by later subobjects. Count value
	// bits, rather than sizeof(base), using the completed typed layout.
	// A member's own padding cannot establish its unique representation.
	bits = 0;
	for (const BaseClass* base = entity->bases; base != nullptr; base = base->next)
	{
		uint64_t payload = 0;
		if (!representation_bits(base->base->type, payload) || payload > UINT64_MAX - bits)
			return false;
		bits += payload;
	}
	for (const DataMember* member = entity->members; member != nullptr; member = member->next)
	{
		if (member->bit_field && member->variable->name == nullptr)
			continue;
		const Type* type = member->variable->type;
		uint64_t payload = 0;
		if (!unique_object_representations(type) || !representation_bits(type, payload))
			return false;
		if (member->bit_field)
			payload = std::min(payload, member->width);
		if (IsUnion(entity))
		{
			if (entity->size > UINT64_MAX / 8 || payload != entity->size * 8)
				return false;
			bits = payload;
		}
		else
		{
			if (payload > UINT64_MAX - bits)
				return false;
			bits += payload;
		}
	}
	return true;
}

void Analyzer::copy_members(ClassEntity* entity, bool assignment, SmallList<const FunctionEntity*, 8>& out)
{
	// The class's copy and move constructors (or assignment operators):
	// the declared ones and the implicitly declared ones (12.8p7, p9, p18,
	// p20), which its scope does not bind.
	implicit_copy_members(entity);
	const NameEntry* entry = names_.find(entity->scope, assignment ? assign_key_ : constructor_key_);
	for (const FunctionList* node = entry != nullptr ? entry->functions : nullptr; node != nullptr; node = node->next)
	{
		const FunctionEntity* function = static_cast<const FunctionEntity*>(node->binding->entity);
		if (function->copy_kind != CopyKind::None)
			out.push_back(function);
	}
	const FunctionEntity* implicit[] = { assignment ? entity->copy_assignment : entity->copy_constructor,
		assignment ? entity->move_assignment : entity->move_constructor };
	for (const FunctionEntity* function : implicit)
	{
		if (function != nullptr && std::find(out.begin(), out.end(), function) == out.end())
			out.push_back(function);
	}
}

bool Analyzer::copy_member_trivial(const FunctionEntity* function, ClassEntity* entity)
{
	// A deleted one (8.4.3p4: on its first declaration, or implicitly) is
	// not user-provided (8.4.2p4): trivial when the members of its kind its
	// subobjects' copies select are (12.8p12, p25; GCC's and Clang's
	// answer), whatever its plan recorded (a selected subobject member that
	// is deleted itself is still selected, and may be non-trivial); one
	// whose parameters are not an implicit declaration's (more of them, or
	// the class by value) is not (CWG 1734's parameter-type-list rule, which
	// both hosts apply).
	const Type* type = function->type->canonical;
	return function->deleted ? type->param_count == 1 && type->params[0]->is_reference() &&
			subobjects_member(entity, function->copy_kind, false)
							 : function->trivial;
}

const FunctionEntity* Analyzer::selected_copy_member(ClassEntity* entity, CopyKind kind)
{
	// The class's member of `kind`, else for a move the copy one (which a
	// copy of an rvalue selects then, 12.8p9, p20).
	if (!require_complete(entity) || entity->scope == nullptr)
		return nullptr;
	const bool assignment = kind == CopyKind::CopyAssignment || kind == CopyKind::MoveAssignment;
	SmallList<const FunctionEntity*, 8> members;
	copy_members(entity, assignment, members);
	const CopyKind fallback = assignment ? CopyKind::CopyAssignment : CopyKind::CopyConstructor;
	const FunctionEntity* found = nullptr;
	for (const FunctionEntity* function : members)
	{
		if (function->copy_kind == kind)
			return function;
		if (function->copy_kind == fallback && found == nullptr)
			found = function;
	}
	return found;
}

bool Analyzer::copy_selection(ClassEntity* entity, CopyKind kind, SmallList<const FunctionEntity*, 8>& best)
{
	// The members a copy of a const lvalue of the class selects, by their
	// first parameter (the rest defaulted; deleted ones take part, 8.4.3):
	// a const reference beats a const volatile one (13.3.3.2p3) unless its
	// assignment operator is volatile-qualified (worse on the object), a
	// by-value parameter (assignment) ties with either (13.3.3.1p6: an
	// identity conversion); several left are ambiguous. False for a move
	// kind, or
	// when no member binds a const source (the copy then takes a non-const
	// one, 12.8p8, p18).
	if (kind != CopyKind::CopyConstructor && kind != CopyKind::CopyAssignment)
		return false;
	SmallList<const FunctionEntity*, 8> members;
	copy_members(entity, kind == CopyKind::CopyAssignment, members);
	bool const_reference = false;
	for (const FunctionEntity* function : members)
	{
		const Type* parameter = function->copy_kind == kind ? function->type->canonical->params[0] : nullptr;
		if (parameter != nullptr && parameter->is_reference() && parameter->element->cv == kConst &&
			(function->type->canonical->function_cv & kVolatile) == 0)
			const_reference = true;
	}
	for (const FunctionEntity* function : members)
	{
		if (function->copy_kind != kind)
			continue;
		const Type* parameter = function->type->canonical->params[0];
		const bool viable = !parameter->is_reference() ||
			((parameter->element->cv & kConst) != 0 && (parameter->element->cv == kConst || !const_reference));
		if (viable)
			best.push_back(function);
	}
	return !best.empty();
}

bool Analyzer::subobjects_member(ClassEntity* entity, CopyKind kind, bool nothrow)
{
	// No virtual function or base (for triviality), and for every direct
	// base and non-static data member of class type (or array of one) the
	// member of `kind` it selects trivial, or for `nothrow` trivial or not
	// throwing (15.4p14: an implicit member's exception-specification).
	check_stack(0);
	if (!nothrow && (entity->dynamic || entity->polymorphic))
		return false;
	// Answered once per class, kind and question (a class's subobjects
	// shared along several paths are asked once): two bits each, known and
	// the answer.
	const unsigned bit = 2 * ((static_cast<unsigned>(kind) - 1) * 2 + (nothrow ? 1 : 0));
	uint32_t known = copy_subobject_answers_.find(entity);
	if (known == PointerMap::kMissing)
		known = 0;
	if ((known >> bit & 1) != 0)
		return (known >> (bit + 1) & 1) != 0;
	const bool answer = subobject_members_select(entity, kind, nothrow);
	known = copy_subobject_answers_.find(entity);
	copy_subobject_answers_.set(
		entity, (known == PointerMap::kMissing ? 0 : known) | 1u << bit | (answer ? 2u : 0u) << bit);
	return answer;
}

bool Analyzer::subobject_members_select(ClassEntity* entity, CopyKind kind, bool nothrow)
{
	// subobjects_member's rule over the direct bases and members.
	const auto selects = [&](const Type* type) {
		ClassEntity* cls = type->is_reference() ? nullptr : ClassType(WithoutArrays(type));
		if (cls == nullptr)
			return true;
		const FunctionEntity* member = selected_copy_member(cls, kind);
		// A copy kind's selection as overload resolution makes it; an
		// ambiguous one selects none: the copy that would make it is
		// deleted, adds no exception, and is trivial when each tied member is
		// trivial or deleted (GCC's and Clang's answers).
		SmallList<const FunctionEntity*, 8> best;
		if (member != nullptr && copy_selection(cls, kind, best))
		{
			if (best.size() > 1)
			{
				if (nothrow)
					return true;
				for (const FunctionEntity* tied : best)
				{
					if (!tied->deleted && !copy_member_trivial(tied, cls))
						return false;
				}
				return true;
			}
			member = best[0];
		}
		return member != nullptr &&
			(copy_member_trivial(member, cls) ||
				(nothrow &&
					(member->deleted && member->implicit ? subobjects_member(cls, kind, true)
														 : function_nothrow(member))));
	};
	for (const BaseClass* base = entity->bases; base != nullptr; base = base->next)
	{
		if (!selects(base->base->type))
			return false;
	}
	for (const DataMember* member = entity->members; member != nullptr; member = member->next)
	{
		if (member->variable->kind == EntityKind::Variable && !selects(member->variable->type))
			return false;
	}
	return true;
}

bool Analyzer::legacy_trait(GnuTrait trait, const Type* type)
{
	// GCC's pre-C++11 traits, as its manual defines them: true for a
	// trivial type (trivially copyable for the copy traits; a reference
	// for the copy constructor ones), false for an assignment to a const
	// object or a reference; otherwise a class (array of class, for the
	// default constructor) whose members of that kind are all trivial, or
	// all non-throwing.
	const bool assign = trait == GnuTrait::HasTrivialAssign || trait == GnuTrait::HasNothrowAssign;
	const bool copy = trait == GnuTrait::HasTrivialCopy || trait == GnuTrait::HasNothrowCopy;
	const bool nothrow = trait >= GnuTrait::HasNothrowConstructor;
	if (assign && (type->is_reference() || (WithoutArrays(type)->cv & kConst) != 0))
		return false;
	if (copy && type->is_reference())
		return true;
	if (assign || copy ? trivially_copyable(type) : trivially_copyable(type) && trivially_default_constructible(type))
		return true;
	ClassEntity* entity = ClassType(assign || copy ? type : WithoutArrays(type));
	if (entity == nullptr || !require_complete(entity) || entity->scope == nullptr)
		return false;
	if (!assign && !copy)
	{
		// The default constructor (see trivially_default_constructible),
		// trivial by 12.1p5's conditions even when defined as deleted.
		const FunctionEntity* constructor = queried_default_constructor(entity);
		if (constructor == nullptr)
			return false;
		return gnu_trivial_default_constructor(entity) || (nothrow && function_nothrow(constructor));
	}
	// The copy (not move) members.
	SmallList<const FunctionEntity*, 8> members;
	copy_members(entity, assign, members);
	const CopyKind kind = assign ? CopyKind::CopyAssignment : CopyKind::CopyConstructor;
	bool found = false;
	for (const FunctionEntity* function : members)
	{
		if ((assign || copy) && function->copy_kind != kind)
			continue;
		found = true;
		// An implicitly deleted one's exception-specification is its
		// subobjects' (15.4p14), which the analysis does not state.
		const bool throws = function->deleted && function->implicit
			? !subobjects_member(entity, function->copy_kind, true)
			: !function_nothrow(function);
		if (nothrow ? throws && !copy_member_trivial(function, entity) : !copy_member_trivial(function, entity))
			return false;
	}
	return found;
}

bool Analyzer::gnu_trivial_default_constructor(ClassEntity* entity)
{
	// The default constructor (see trivially_default_constructible),
	// trivial by 12.1p5's conditions even when defined as deleted, as GCC
	// counts it (its literal-type and legacy traits); answered once per
	// class (subobjects shared along several paths are asked once).
	const uint32_t known = trivial_default_classes_.find(entity);
	if (known != PointerMap::kMissing)
		return known != 0;
	const FunctionEntity* constructor = queried_default_constructor(entity);
	const bool trivial = constructor != nullptr &&
		(constructor->trivial ||
			(constructor->deleted && !UserProvidedConstructor(constructor) && subobjects_default_trivially(entity)));
	trivial_default_classes_.set(entity, trivial ? 1 : 0);
	return trivial;
}

bool Analyzer::default_constructor_shape_trivial(ClassEntity* entity)
{
	// GCC's trivial default constructor by the class's declarations alone
	// (12.1p5's conditions, whether or not it is defined as deleted; no
	// constructor is planned): no user-provided one callable without
	// arguments, and an implicit or defaulted (or deleted) one there is; no
	// virtual function or base, no brace-or-equal-initializer, and every
	// subobject of class type (or array of one) alike. Answered once per
	// class.
	check_stack(0);
	if (!require_complete(entity) || entity->scope == nullptr || entity->dynamic || entity->polymorphic)
		return false;
	const uint32_t known = trivial_shape_classes_.find(entity);
	if (known != PointerMap::kMissing)
		return known != 0;
	const bool trivial = default_constructor_declarations_trivial(entity);
	trivial_shape_classes_.set(entity, trivial ? 1 : 0);
	return trivial;
}

bool Analyzer::default_constructor_declarations_trivial(ClassEntity* entity)
{
	// default_constructor_shape_trivial's rule for a complete class.
	const NameEntry* entry = names_.find(entity->scope, constructor_key_);
	bool declared = false, found = false;
	for (const FunctionList* node = entry != nullptr ? entry->functions : nullptr; node != nullptr; node = node->next)
	{
		const FunctionEntity* function = static_cast<const FunctionEntity*>(node->binding->entity);
		if (function->implicit || function->inherited != nullptr)
			continue;
		declared = true;
		const Type* type = function->type->canonical;
		bool callable = !function->templated;
		for (uint32_t i = 0; callable && i < type->param_count; ++i)
			callable = function->default_arguments != nullptr && function->default_arguments[i] != nullptr;
		if (!callable)
			continue;
		if (UserProvidedConstructor(function))
			return false;
		found = true;
	}
	if (declared && !found)
		return false;
	const auto trivial = [&](const Type* type) {
		ClassEntity* cls = type->is_reference() ? nullptr : ClassType(WithoutArrays(type));
		return cls == nullptr || default_constructor_shape_trivial(cls);
	};
	for (const BaseClass* base = entity->bases; base != nullptr; base = base->next)
	{
		if (!trivial(base->base->type))
			return false;
	}
	for (const DataMember* member = entity->members; member != nullptr; member = member->next)
	{
		if (member->variable->kind != EntityKind::Variable)
			continue;
		const VariableEntity* variable = static_cast<const VariableEntity*>(member->variable);
		if (variable->member_initializer != nullptr || !trivial(variable->type))
			return false;
	}
	return true;
}

bool Analyzer::subobjects_default_trivially(ClassEntity* entity)
{
	// 12.1p5: no virtual function or base, no brace-or-equal-initializer,
	// and every direct base and non-static data member of class type with
	// a trivial default constructor.
	if (entity->dynamic || entity->polymorphic)
		return false;
	for (const BaseClass* base = entity->bases; base != nullptr; base = base->next)
	{
		if (!legacy_trait(GnuTrait::HasTrivialConstructor, base->base->type))
			return false;
	}
	for (const DataMember* member = entity->members; member != nullptr; member = member->next)
	{
		if (member->variable->kind != EntityKind::Variable)
			continue;
		const VariableEntity* variable = static_cast<const VariableEntity*>(member->variable);
		if (variable->member_initializer != nullptr)
			return false;
		const ClassEntity* cls = ClassType(WithoutArrays(variable->type));
		if (cls != nullptr && !variable->type->is_reference() &&
			!legacy_trait(GnuTrait::HasTrivialConstructor, WithoutArrays(variable->type)))
			return false;
	}
	return true;
}

bool Analyzer::subobjects_copy_trivially(ClassEntity* entity)
{
	// No virtual function or base, and every direct base and non-static
	// data member trivially copyable (references aside).
	if (entity->dynamic || entity->polymorphic)
		return false;
	for (const BaseClass* base = entity->bases; base != nullptr; base = base->next)
	{
		if (!trivially_copyable(base->base->type))
			return false;
	}
	for (const DataMember* member = entity->members; member != nullptr; member = member->next)
	{
		const Type* type = member->variable->type;
		if (member->variable->kind == EntityKind::Variable && !type->is_reference() && !trivially_copyable(type))
			return false;
	}
	return true;
}

bool Analyzer::trait_literal_type(const Type* type)
{
	// __is_literal_type as GCC answers it: 3.9p10, a class with a trivial
	// default constructor counting as one with a constexpr constructor
	// (GCC's finalize_literal_type_property); no function type is one.
	if (type->kind == TypeKind::Function)
		return false;
	if (literal_type(type))
		return true;
	ClassEntity* entity = ClassType(WithoutArrays(type));
	if (entity == nullptr || !require_complete(entity) || entity->scope == nullptr)
		return false;
	// Answered once per complete class (its subobjects' answers are each
	// class's own: a chain of members would otherwise be walked per query).
	const uint32_t known = trait_literal_classes_.find(entity);
	if (known != PointerMap::kMissing)
		return known != 0;
	// The default constructor trivial as GCC counts it, even defined as
	// deleted (`class C { const int m; };` is literal there).
	bool literal = (entity->destructor == nullptr || entity->destructor->trivial) &&
		default_constructor_shape_trivial(entity);
	for (const BaseClass* base = entity->bases; literal && base != nullptr; base = base->next)
		literal = !base->is_virtual && trait_literal_type(base->base->type);
	for (const DataMember* member = entity->members; literal && member != nullptr; member = member->next)
	{
		const Type* member_type = member->variable->type;
		literal = member->variable->kind != EntityKind::Variable || member_type->is_reference() ||
			(trait_literal_type(member_type) && (WithoutArrays(member_type)->cv & kVolatile) == 0);
	}
	trait_literal_classes_.set(entity, literal ? 1 : 0);
	return literal;
}

bool Analyzer::trivially_default_constructible(const Type* type)
{
	// The rest of a trivial type (9p6): a scalar (GNU vector and complex
	// types too), or a class with a trivial default constructor (an
	// implicit one, or one defaulted on its first declaration and trivial),
	// or an array of one.
	type = WithoutArrays(type);
	if (ScalarLike(type))
		return true;
	ClassEntity* entity = ClassType(type);
	if (entity == nullptr || type->is_reference() || !require_complete(entity) || entity->scope == nullptr)
		return false;
	// The default constructor: the implicit one, or the declared one
	// callable without arguments (a defaulted one's facts made on demand).
	const FunctionEntity* constructor = queried_default_constructor(entity);
	return constructor != nullptr && constructor->trivial && !constructor->deleted;
}

bool Analyzer::standard_layout(const Type* type)
{
	// 3.9p9, 9p7: a scalar type (GNU vector and complex types too), a
	// standard-layout class, or an array of one.
	type = WithoutArrays(type);
	if (ScalarLike(type))
		return true;
	ClassEntity* entity = ClassType(type);
	if (entity == nullptr || type->is_reference() || !require_complete(entity))
		return false;
	return standard_layout_class(entity, 0);
}

bool Analyzer::standard_layout_class(ClassEntity* entity, uint32_t depth)
{
	// No virtual functions or bases; all non-static data members of one
	// access and of standard-layout types (references allowed); bases of
	// standard-layout classes; non-static data members in at most one class
	// of the hierarchy; no base of the first member's type.
	check_stack(0);
	if (entity->dynamic || entity->polymorphic)
		return false;
	const DataMember* first = nullptr;
	for (const DataMember* member = entity->members; member != nullptr; member = member->next)
	{
		if (member->variable->kind != EntityKind::Variable)
			continue;
		if (first == nullptr)
			first = member;
		else if (member->access != first->access)
			return false;
		// 9p7: no member of reference type or of a non-standard-layout
		// class type.
		if (!standard_layout(member->variable->type))
			return false;
	}
	bool base_has_members = false;
	for (const BaseClass* base = entity->bases; base != nullptr; base = base->next)
	{
		ClassEntity* base_class = ClassType(base->base->type);
		if (base_class == nullptr || base->is_virtual || !standard_layout_class(base_class, depth + 1))
			return false;
		if (has_data_members(base_class))
		{
			if (base_has_members || first != nullptr)
				return false;
			base_has_members = true;
		}
		if (first != nullptr && ClassType(WithoutArrays(first->variable->type)) == base_class)
			return false;
	}
	return true;
}

bool Analyzer::has_data_members(const ClassEntity* entity)
{
	for (const DataMember* member = entity->members; member != nullptr; member = member->next)
	{
		if (member->variable->kind == EntityKind::Variable)
			return true;
	}
	for (const BaseClass* base = entity->bases; base != nullptr; base = base->next)
	{
		const ClassEntity* base_class = ClassType(base->base->type);
		if (base_class != nullptr && has_data_members(base_class))
			return true;
	}
	return false;
}

syntax::Expr* Analyzer::declval_operand(const Type* type, uint32_t location, bool prvalue)
{
	syntax::BuiltinTraitExpr* operand = static_cast<syntax::BuiltinTraitExpr*>(
		arena_.allocate(sizeof(syntax::BuiltinTraitExpr), alignof(syntax::BuiltinTraitExpr)));
	*operand = syntax::BuiltinTraitExpr();
	operand->kind = syntax::ExprKind::BuiltinTrait;
	operand->location = location;
	operand->trait = prvalue ? GnuTrait::DeclvalPrvalue : GnuTrait::Declval;
	operand->type = type;
	return operand;
}

bool Analyzer::trait_constructible(const Type* target, const Type* const* arguments, uint32_t count, uint32_t location,
	Scope* scope, bool& nothrow, bool& trivial)
{
	// 20.9.4.3p6: `T t(std::declval<Args>()...);` is well-formed (for no
	// arguments, T is value-initialized: `T()`), a definition that also
	// destroys t; `nothrow` when it calls nothing that may throw, `trivial`
	// when it calls no non-trivial function.
	nothrow = true;
	trivial = true;
	// A function type's declval is a function designator lvalue (5p6), an
	// argument that binds a reference to function.
	for (uint32_t i = 0; i < count; ++i)
	{
		if (IsVoid(arguments[i]) ||
			(arguments[i]->kind == TypeKind::Function &&
				(arguments[i]->function_cv != 0 || arguments[i]->ref != RefQualifier::None)))
			return false;
	}
	if (IsVoid(target) || target->kind == TypeKind::Function)
		return false;
	if (target->kind == TypeKind::Array)
	{
		// An array of known bound is value-initialized; nothing else
		// initializes an array from parenthesized values in C++11.
		if (target->bound == 0 || count != 0)
			return false;
		return trait_constructible(WithoutArrays(target), nullptr, 0, location, scope, nothrow, trivial);
	}
	if (target->is_reference() ? count != 1 : (ClassType(target) == nullptr && count > 1))
		return false;
	ClassEntity* entity = target->is_reference() ? nullptr : ClassType(target);
	if (entity != nullptr &&
		(!require_complete(entity) || entity->scope == nullptr ||
			(entity->virtuals != nullptr && entity->virtuals->pure_count != 0)))
		return false;
	SmallList<syntax::Expr*, 4> operands;
	for (uint32_t i = 0; i < count; ++i)
		operands.push_back(declval_operand(arguments[i], location));
	syntax::Initializer init = syntax::Initializer();
	init.kind = syntax::InitializerKind::Parenthesized;
	init.location = location;
	init.arguments = syntax::Slice<syntax::Expr*>{ operands.empty() ? nullptr : operands.data(),
		static_cast<uint32_t>(operands.size()) };
	const FunctionEntity* constructor = nullptr;
	// The object the definition destroys (12.4p11): t, or the temporary a
	// reference binds to (12.2p1).
	const Type* destroyed = target;
	{
		SoftSubstitution soft(*this);
		++unevaluated_;
		const uint32_t saved_throwing = potentially_throwing_;
		BodyContext* saved = body_;
		BodyContext context = BodyContext();
		if (body_ == nullptr)
			body_ = &context;
		constructor = initialize_object(target, init, scope, location);
		if (target->is_reference())
		{
			const Conversion* binding = operands[0]->facts != nullptr ? operands[0]->facts->conversion : nullptr;
			destroyed = binding != nullptr && binding->form == ConversionForm::Reference && binding->temporary
				? binding->result
				: nullptr;
		}
		// Its destructor is neither deleted nor inaccessible.
		if (destroyed != nullptr && !soft.failed())
			check_destruction(destroyed, scope, location);
		body_ = saved;
		potentially_throwing_ = saved_throwing;
		--unevaluated_;
		if (soft.failed())
			return false;
	}
	if (constructor != nullptr)
	{
		nothrow = function_nothrow(constructor);
		trivial = constructor->trivial;
		// The default arguments the call uses are evaluated in it (5.2.2p4,
		// 8.3.6p9).
		const Type* type = constructor->type->canonical;
		for (uint32_t i = count; nothrow && constructor->default_arguments != nullptr && i < type->param_count; ++i)
			nothrow = constructor->default_arguments[i] == nullptr || !may_throw(*constructor->default_arguments[i], 0);
	}
	// Nor does its destruction throw (as GCC and Clang count it).
	const ClassEntity* destroyed_class = destroyed != nullptr ? ClassType(WithoutArrays(destroyed)) : nullptr;
	if (destroyed_class != nullptr && destroyed_class->destructor != nullptr &&
		!function_nothrow(destroyed_class->destructor))
		nothrow = false;
	// The definition's object is destroyed too, as GCC counts it.
	const ClassEntity* object = target->is_reference() ? nullptr : ClassType(WithoutArrays(target));
	if (object != nullptr && object->destructor != nullptr && !object->destructor->trivial)
		trivial = false;
	else if (entity != nullptr && count == 0)
	{
		// Value-initialization that runs no constructor zeroes the object.
		trivial = trivially_default_constructible(target);
	}
	for (const syntax::Expr* operand : operands)
	{
		const Conversion* conversion = operand->facts != nullptr ? operand->facts->conversion : nullptr;
		if (conversion != nullptr && conversion->constructor != nullptr)
		{
			trivial = false;
			nothrow = nothrow && function_nothrow(conversion->constructor);
		}
	}
	return true;
}

bool Analyzer::trait_assignable(
	const Type* target, const Type* source, uint32_t location, Scope* scope, bool& nothrow, bool& trivial)
{
	// 20.9.4.3: `std::declval<T>() = std::declval<U>()` is well-formed as an
	// unevaluated operand.
	nothrow = false;
	trivial = false;
	// A function source is an lvalue (std::declval<F>() is F&&); an
	// abominable one has no declval.
	if (IsVoid(target) || IsVoid(source) || target->kind == TypeKind::Function ||
		(source->kind == TypeKind::Function && (source->function_cv != 0 || source->ref != RefQualifier::None)))
		return false;
	syntax::BinaryExpr* assignment = static_cast<syntax::BinaryExpr*>(
		arena_.allocate(sizeof(syntax::BinaryExpr), alignof(syntax::BinaryExpr)));
	*assignment = syntax::BinaryExpr();
	assignment->kind = syntax::ExprKind::Assign;
	assignment->location = location;
	assignment->op = TokenKind::OP_ASS;
	assignment->left = declval_operand(target, location);
	assignment->right = declval_operand(source, location);
	{
		SoftSubstitution soft(*this);
		++unevaluated_;
		const uint32_t saved_throwing = potentially_throwing_;
		BodyContext* saved = body_;
		BodyContext context = BodyContext();
		if (body_ == nullptr)
			body_ = &context;
		const ExprFacts* facts = typed(*assignment, scope);
		body_ = saved;
		potentially_throwing_ = saved_throwing;
		--unevaluated_;
		if (facts == nullptr || soft.failed())
			return false;
	}
	nothrow = !may_throw(*assignment, 0);
	const ExprFacts* facts = assignment->facts;
	const FunctionEntity* function = (facts->flags & kOperatorCall) != 0 && facts->entity != nullptr &&
			facts->entity->kind == EntityKind::Function
		? static_cast<const FunctionEntity*>(facts->entity)
		: nullptr;
	trivial = function != nullptr ? function->trivial : (facts->flags & kOperatorCall) == 0;
	const Conversion* conversion = assignment->right->facts != nullptr ? assignment->right->facts->conversion : nullptr;
	if (conversion != nullptr && conversion->constructor != nullptr)
		trivial = false;
	return true;
}

} // namespace sema
