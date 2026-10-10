#include "sema/conversions.h"

#include <vector>

#include "sema/integral_arithmetic.h"

namespace sema {

namespace {

bool IsFundamental(const Type* type, FundamentalType which)
{
	type = type->unqualified;
	return type->kind == TypeKind::Fundamental && type->fundamental == which;
}

bool IsBool(const Type* type)
{
	return IsFundamental(type, FundamentalType::Bool);
}

bool IsNullptrT(const Type* type)
{
	return IsFundamental(type, FundamentalType::NullptrT);
}

bool IsPointerLike(const Type* type)
{
	type = type->unqualified;
	return type->kind == TypeKind::Pointer || type->kind == TypeKind::MemberPointer;
}

// 3.9.3p5 (as CWG 1059 clarified): an array's cv-qualifiers are its
// elements'.
uint8_t CvOf(const Type* type)
{
	while (type->kind == TypeKind::Array)
		type = type->element;
	return type->cv;
}

// The class of a (possibly cv-qualified) class type.
const ClassEntity* ClassEntityOf(const Type* type)
{
	type = type->unqualified;
	return type->kind == TypeKind::Class ? static_cast<const ClassEntity*>(type->entity) : nullptr;
}

// 4.5: `source` (cv-unqualified) promotes to `target`.
bool IsIntegralPromotion(TypeTable& types, const Type* source, const Type* target)
{
	if (!IsIntegralOrUnscopedEnum(source) || target->kind != TypeKind::Fundamental || !IsIntegralType(target))
		return false;
	if (source->kind == TypeKind::Enum)
	{
		// 4.5p4: an enumeration with a fixed underlying type also promotes
		// to that type.
		const EnumEntity* entity = static_cast<const EnumEntity*>(source->entity);
		if (entity->fixed && entity->underlying == target)
			return true;
	}
	return PromotedType(types, source) == target && source != target;
}

// Equal but for the cv-qualifiers of array elements (an array's are its
// elements', 3.9.3p5).
bool SameArrayShape(const Type* a, const Type* b)
{
	while (a->kind == TypeKind::Array && b->kind == TypeKind::Array)
	{
		if (a->bound != b->bound)
			return false;
		a = a->element;
		b = b->element;
	}
	return a->unqualified == b->unqualified;
}

// The pointer and member-pointer levels of a type, outermost first, with
// the cv-qualifiers below each level; false once the structure differs.
bool SimilarLevels(const Type* a, const Type* b, bool& added, bool& convertible)
{
	// 4.4p4: walking down from the first level below the top, a cv added at
	// level j needs `const` at every level 0 < k < j.
	added = false;
	convertible = true;
	bool const_so_far = true;
	for (unsigned level = 0;; ++level)
	{
		a = a->unqualified;
		b = b->unqualified;
		if (a->kind != b->kind)
			return false;
		if (a->kind == TypeKind::MemberPointer && a->owner != b->owner)
			return false;
		if (a->kind != TypeKind::Pointer && a->kind != TypeKind::MemberPointer)
			return level != 0 && a == b;
		const Type* ea = a->element;
		const Type* eb = b->element;
		const uint8_t ca = CvOf(ea), cb = CvOf(eb);
		if ((ca & ~cb) != 0)
			convertible = false;
		if (ca != cb)
		{
			added = true;
			if (!const_so_far)
				convertible = false;
		}
		const_so_far = const_so_far && (cb & kConst) != 0;
		// A pointer to an array ends the chain: the arrays match but for
		// their elements' cv-qualifiers, compared above as the array's.
		if (ea->kind == TypeKind::Function || eb->kind == TypeKind::Function)
			return ea == eb;
		if (ea->kind == TypeKind::Array || eb->kind == TypeKind::Array)
			return SameArrayShape(ea, eb);
		if (ea->unqualified->kind != TypeKind::Pointer && ea->unqualified->kind != TypeKind::MemberPointer)
			return ea->unqualified == eb->unqualified;
		a = ea;
		b = eb;
	}
}

// The base class a derived-to-base conversion reaches: the class a
// pointer points to, or the class object or bound reference's class.
const Type* DerivationTarget(const Conversion& conversion)
{
	const Type* result = conversion.result->unqualified;
	return result->kind == TypeKind::Pointer ? result->element : result;
}

} // namespace

bool IsFloating(const Type* type)
{
	type = type->unqualified;
	return type->kind == TypeKind::Fundamental && IsFloatingFundamental(type->fundamental);
}

bool IsComplex(const Type* type)
{
	return type->unqualified->kind == TypeKind::Complex;
}

bool IsArithmetic(const Type* type)
{
	return IsIntegralType(type) || IsFloating(type);
}

bool IsIntegralOrUnscopedEnum(const Type* type)
{
	return IsIntegralType(type) || (type->unqualified->kind == TypeKind::Enum && !IsScopedEnumeration(type));
}

bool IsScalar(const Type* type)
{
	type = type->unqualified;
	return IsArithmetic(type) || type->kind == TypeKind::Enum || type->kind == TypeKind::Pointer ||
		type->kind == TypeKind::MemberPointer || IsNullptrT(type);
}

bool IsObjectPointer(const Type* type)
{
	type = type->unqualified;
	return type->kind == TypeKind::Pointer && type->element->kind != TypeKind::Function;
}

uint32_t IndirectVirtualBases(const ClassEntity* cls)
{
	uint32_t count = 0;
	for (uint32_t i = 0; i < VirtualBaseCount(cls); ++i)
		count += VirtualBases(cls)[i].direct ? 0 : 1;
	return count;
}

const ClassEntity* MemberClassOf(const Entity* member)
{
	const ClassEntity* home = static_cast<const ClassEntity*>(member->owner->entity);
	while (member->kind == EntityKind::Variable && home->anonymous && home->storage != nullptr &&
		home->storage->entity->owner->kind == ScopeKind::Class)
		home = static_cast<const ClassEntity*>(home->storage->entity->owner->entity);
	return home;
}

bool ConversionRules::is_base_of(const ClassEntity* base, const ClassEntity* derived)
{
	// Depth-first over the acyclic base graph, each class once per query
	// (its mark), so shared bases do not multiply the walk. A derived class
	// that is an instance is defined first (its bases are then complete).
	if (base == nullptr || derived == nullptr || base == derived)
		return false;
	if (!derived->complete && completer_ != nullptr)
		completer_->complete(derived);
	if (derived->bases == nullptr)
		return false;
	++visit_;
	std::vector<const ClassEntity*>& stack = base_stack_;
	stack.clear();
	stack.push_back(derived);
	while (!stack.empty())
	{
		const ClassEntity* current = stack.back();
		stack.pop_back();
		for (const BaseClass* edge = current->bases; edge != nullptr; edge = edge->next)
		{
			ClassEntity* next = static_cast<ClassEntity*>(edge->base);
			if (next == base)
				return true;
			if (next->derivation_visit != visit_)
			{
				next->derivation_visit = visit_;
				stack.push_back(next);
			}
		}
	}
	return false;
}

const Type* ConversionRules::without_cv(const Type* type)
{
	if (type->kind != TypeKind::Array)
		return type->unqualified;
	return types_.array(without_cv(type->element), type->bound);
}

bool ConversionRules::reference_related(const Type* a, const Type* b)
{
	a = without_cv(a->canonical);
	b = without_cv(b->canonical);
	if (a == b)
		return true;
	return is_base_of(ClassEntityOf(a), ClassEntityOf(b));
}

bool ConversionRules::qualification_convertible(const Type* from, const Type* to)
{
	bool added = false, convertible = false;
	return SimilarLevels(from, to, added, convertible) && convertible;
}

bool ConversionRules::casts_away_constness(const Type* from, const Type* to)
{
	// Over the pointer levels both types have, from the outermost, the
	// pointed-to qualifiers of `from` admit no qualification conversion
	// (4.4p4) to those of `to`; member pointers count as pointers.
	bool const_so_far = true;
	for (;;)
	{
		from = from->unqualified;
		to = to->unqualified;
		if (!IsPointerLike(from) || !IsPointerLike(to))
			return false;
		const uint8_t cf = CvOf(from->element), ct = CvOf(to->element);
		if ((cf & ~ct) != 0 || (cf != ct && !const_so_far))
			return true;
		const_so_far = const_so_far && (ct & kConst) != 0;
		from = from->element;
		to = to->element;
	}
}

bool ConversionRules::const_castable(const Type* from, const Type* to)
{
	// 5.2.11p3: the same pointer and member pointer levels (of one class
	// each) down to one object type or void; a pointer to function is such
	// an object type below a level (its function type is not qualified).
	for (unsigned levels = 0;; ++levels)
	{
		from = from->unqualified;
		to = to->unqualified;
		if (!IsPointerLike(from) || !IsPointerLike(to))
			return levels != 0 && !IsPointerLike(from) && !IsPointerLike(to) && from->kind != TypeKind::Function &&
				SameArrayShape(from, to);
		if (from->kind != to->kind || (from->kind == TypeKind::MemberPointer && from->owner != to->owner))
			return false;
		if (from->element->kind == TypeKind::Function || to->element->kind == TypeKind::Function)
			return levels != 0 && from->canonical == to->canonical;
		from = from->element;
		to = to->element;
	}
}

bool ConversionRules::implicit(const Operand& from, const Type* target, bool direct, Conversion& out)
{
	target = target->canonical;
	out = Conversion();
	out.target = target;
	if (target->is_reference())
		return reference(from, target, out);
	return standard(from, target, direct, out);
}

bool ConversionRules::standard(const Operand& from, const Type* target, bool direct, Conversion& out)
{
	out.form = ConversionForm::Standard;
	out.rank = ConversionRank::Exact;
	const Type* goal = target->unqualified;
	const Type* source = from.type->canonical;
	// 13.3.3.1p6: a class operand converts to its own class (identity) or
	// to a base (a derived-to-base Conversion); user-defined conversions
	// are not considered here.
	if (source->unqualified->kind == TypeKind::Class || goal->kind == TypeKind::Class)
	{
		if (source->unqualified == goal)
		{
			out.result = goal;
			return true;
		}
		if (goal->kind == TypeKind::Class && ClassEntityOf(source) != nullptr &&
			is_base_of(ClassEntityOf(goal), ClassEntityOf(source)))
		{
			out.second = ValueConversion::DerivedToBase;
			out.rank = ConversionRank::Conversion;
			out.result = goal;
			return true;
		}
		return false;
	}
	// The lvalue transformation (4.1, 4.2, 4.3).
	if (source->kind == TypeKind::Array)
	{
		out.first = LvalueTransform::ArrayToPointer;
		source = types_.pointer(source->element);
	}
	else if (source->kind == TypeKind::Function)
	{
		out.first = LvalueTransform::FunctionToPointer;
		source = types_.pointer(source);
	}
	else
	{
		if (from.category != ValueCategory::PRValue)
			out.first = LvalueTransform::LvalueToRvalue;
		// 3.10p4: a prvalue of non-class type is cv-unqualified.
		source = source->unqualified;
	}
	if (IsVoid(source) || IsVoid(goal))
		return false;
	if (source == goal)
	{
		out.result = goal;
		return true;
	}
	return value_conversion(source, from.flags, goal, direct, out);
}

bool ConversionRules::value_conversion(
	const Type* source, uint16_t flags, const Type* goal, bool direct, Conversion& out)
{
	// A qualification adjustment alone (4.4).
	if (IsPointerLike(source) && IsPointerLike(goal) && qualification_convertible(source, goal))
	{
		out.qualification = true;
		out.result = goal;
		return true;
	}
	// 4.5, 4.6: promotions.
	if (IsIntegralPromotion(types_, source, goal))
	{
		out.second = ValueConversion::IntegralPromotion;
		out.rank = ConversionRank::Promotion;
		out.result = goal;
		return true;
	}
	if (IsFundamental(source, FundamentalType::Float) && IsFundamental(goal, FundamentalType::Double))
	{
		out.second = ValueConversion::FloatingPromotion;
		out.rank = ConversionRank::Promotion;
		out.result = goal;
		return true;
	}
	out.rank = ConversionRank::Conversion;
	out.result = goal;
	// 4.12: to bool (a GNU complex value too, true unless both parts are
	// zero).
	if (IsBool(goal))
	{
		if (IsArithmetic(source) || IsIntegralOrUnscopedEnum(source) || IsComplex(source))
		{
			out.second = ValueConversion::Boolean;
			return true;
		}
		if (IsPointerLike(source) || (direct && IsNullptrT(source)))
		{
			out.second = ValueConversion::PointerBoolean;
			return true;
		}
		return false;
	}
	// GNU: to a complex type from a real or complex one (GCC converts no
	// complex value to a real type implicitly in C++); a complex value
	// whose parts promote (4.5, 4.6: _Complex short to _Complex int,
	// _Complex char32_t to _Complex unsigned, complex float to complex
	// double) is promoted, as GCC ranks it (Clang alike but for the
	// character types' parts; a bool part promotes in neither).
	if (IsComplex(goal) && (IsArithmetic(source) || IsIntegralOrUnscopedEnum(source) || IsComplex(source)))
	{
		out.second = ValueConversion::Complex;
		const Type* from = IsComplex(source) ? source->element->unqualified : nullptr;
		const Type* to = goal->unqualified->element->unqualified;
		if (from == nullptr)
			out.rank = ConversionRank::ComplexReal;
		else if ((IsFundamental(from, FundamentalType::Float) && IsFundamental(to, FundamentalType::Double)) ||
			(!IsBool(from) && IsIntegralPromotion(types_, from, to)))
			out.rank = ConversionRank::Promotion;
		return true;
	}
	// 4.7, 4.8, 4.9.
	if (IsIntegralType(goal) && IsIntegralOrUnscopedEnum(source))
	{
		out.second = ValueConversion::IntegralConversion;
		return true;
	}
	if (IsFloating(goal) && IsFloating(source))
	{
		// Clang converts between bfloat16 and binary16 by the usual
		// arithmetic conversions only (no initialization, cast or overload
		// converts one to the other).
		const FundamentalType from = source->unqualified->fundamental;
		const FundamentalType to = goal->unqualified->fundamental;
		if (!arithmetic_ &&
			((from == FundamentalType::BFloat16 && to == FundamentalType::Float16) ||
				(from == FundamentalType::Float16 && to == FundamentalType::BFloat16)))
			return false;
		out.second = ValueConversion::FloatingConversion;
		return true;
	}
	if ((IsFloating(goal) && IsIntegralOrUnscopedEnum(source)) || (IsIntegralType(goal) && IsFloating(source)))
	{
		out.second = ValueConversion::FloatingIntegral;
		return true;
	}
	// 4.10p1, 4.11p1: a null pointer constant (an integer literal zero, or
	// a std::nullptr_t prvalue) to a pointer, member pointer or nullptr_t.
	const bool null_constant = (flags & kNullLiteral) != 0 || IsNullptrT(source);
	if (null_constant && (IsPointerLike(goal) || (IsNullptrT(goal) && (flags & kNullLiteral) != 0)))
	{
		out.second = ValueConversion::NullPointer;
		return true;
	}
	// 4.11p2: a pointer to member of a class to one of a class derived
	// from it, then a qualification adjustment.
	if (source->kind == TypeKind::MemberPointer && goal->kind == TypeKind::MemberPointer)
	{
		const ClassEntity* base = ClassEntityOf(source->owner);
		const ClassEntity* derived = ClassEntityOf(goal->owner);
		if (base == nullptr || derived == nullptr || !is_base_of(base, derived))
			return false;
		const Type* adjusted = types_.member_pointer(goal->owner, source->element);
		out.second = ValueConversion::MemberToDerived;
		out.qualification = adjusted != goal;
		return adjusted == goal || qualification_convertible(adjusted, goal);
	}
	if (source->kind != TypeKind::Pointer || goal->kind != TypeKind::Pointer)
		return false;
	// 4.10p2: pointer to cv T (an object type) to pointer to cv void, then
	// a qualification adjustment.
	const Type* pointee = source->element;
	if (IsVoid(goal->element) && pointee->kind != TypeKind::Function && !IsVoid(pointee))
	{
		const Type* adjusted = types_.pointer(
			types_.qualified(types_.fundamental(FundamentalType::Void), CvOf(pointee)));
		out.second = ValueConversion::PointerToVoid;
		out.qualification = adjusted != goal;
		return adjusted == goal || qualification_convertible(adjusted, goal);
	}
	// 4.10p3: pointer to a class to pointer to its base class.
	const ClassEntity* derived = ClassEntityOf(pointee);
	const ClassEntity* base = ClassEntityOf(goal->element);
	if (derived != nullptr && base != nullptr && is_base_of(base, derived))
	{
		const Type* adjusted = types_.pointer(types_.qualified(base->type, pointee->cv));
		out.second = ValueConversion::DerivedToBase;
		out.qualification = adjusted != goal;
		return adjusted == goal || qualification_convertible(adjusted, goal);
	}
	return false;
}

bool ConversionRules::reference(const Operand& from, const Type* target, Conversion& out)
{
	// 8.5.3p5.
	out.form = ConversionForm::Reference;
	out.rank = ConversionRank::Exact;
	const bool rvalue_reference = target->kind == TypeKind::RValueReference;
	out.rvalue_reference = rvalue_reference;
	const Type* referenced = target->element;
	const Type* source = from.type->canonical;
	const bool related = reference_related(referenced, source);
	const bool compatible = related && (CvOf(source) & ~CvOf(referenced)) == 0;
	const bool function_lvalue = source->kind == TypeKind::Function && from.category == ValueCategory::LValue;
	out.function_lvalue = function_lvalue;
	const bool derived = related && without_cv(source) != without_cv(referenced);
	const bool const_lvalue = !rvalue_reference && CvOf(referenced) == kConst;
	// CWG 2352, as both hosts and the course apply it in C++11 mode: a
	// pointer whose pointer converts to a pointer to the referenced type by
	// a qualification conversion is reference-compatible with it, an lvalue
	// bound directly (`const int* const& r = p` for an `int* p`).
	if (!related && !rvalue_reference && from.category == ValueCategory::LValue && IsPointerLike(source->unqualified) &&
		IsPointerLike(referenced->unqualified) &&
		qualification_convertible(types_.pointer(source), types_.pointer(referenced->canonical)))
	{
		out.result = referenced;
		out.qualification = true;
		return true;
	}
	// An lvalue that is not a bit-field binds an lvalue reference directly;
	// a const one binds to a temporary copy of a bit-field's value. To
	// another lvalue reference a bit-field is an lvalue of its type in the
	// conversion sequence (13.3.3.1.4p4), a binding the analysis rejects
	// once it is used (8.5.3p5, see Analyzer::convert).
	if (!rvalue_reference && from.category == ValueCategory::LValue && compatible &&
		((from.flags & kBitField) == 0 || !const_lvalue))
	{
		out.result = referenced;
		if (derived)
		{
			out.second = ValueConversion::DerivedToBase;
			out.rank = ConversionRank::Conversion;
		}
		return true;
	}
	if (!rvalue_reference && !const_lvalue)
		return false;
	// An xvalue, class or array prvalue, or function lvalue binds a const
	// lvalue or an rvalue reference directly.
	const bool direct_kind = from.category == ValueCategory::XValue ||
		(from.category == ValueCategory::PRValue &&
			(source->unqualified->kind == TypeKind::Class || source->kind == TypeKind::Array)) ||
		function_lvalue;
	if (direct_kind && compatible && (from.flags & kBitField) == 0)
	{
		out.result = referenced;
		if (derived)
		{
			out.second = ValueConversion::DerivedToBase;
			out.rank = ConversionRank::Conversion;
		}
		return true;
	}
	if (related)
	{
		// The cv-qualification may not decrease, and an rvalue reference
		// does not bind an lvalue of a related type.
		if ((CvOf(source) & ~CvOf(referenced)) != 0)
			return false;
		if (rvalue_reference && from.category == ValueCategory::LValue)
			return false;
	}
	if (referenced->kind == TypeKind::Function)
		return false;
	// Otherwise a temporary of the referenced type is copy-initialized from
	// the operand (8.5.3p5, last bullet).
	Conversion temporary = Conversion();
	temporary.target = referenced->unqualified;
	if (!standard(from, referenced->unqualified, false, temporary))
		return false;
	out.first = temporary.first;
	out.second = temporary.second;
	out.qualification = temporary.qualification;
	out.rank = temporary.rank;
	out.temporary = true;
	out.result = referenced;
	return true;
}

int ConversionRules::compare(const Conversion& a, const Conversion& b)
{
	// 13.3.3.2p2: a standard conversion sequence beats an ellipsis one.
	const bool a_ellipsis = a.form == ConversionForm::Ellipsis;
	const bool b_ellipsis = b.form == ConversionForm::Ellipsis;
	if (a_ellipsis || b_ellipsis)
		return a_ellipsis == b_ellipsis ? 0 : a_ellipsis ? -1 : 1;
	// p2: a standard conversion sequence beats a user-defined one; p3: two
	// user-defined ones by different constructors are indistinguishable,
	// by one constructor they compare by their second standard conversion.
	const bool a_user = a.rank == ConversionRank::UserDefined;
	const bool b_user = b.rank == ConversionRank::UserDefined;
	if (a_user != b_user)
		return a_user ? -1 : 1;
	// p3: of two list-initialization sequences, one converting to
	// std::initializer_list<X> beats one that does not.
	if (a.initializer_list != b.initializer_list)
		return a.initializer_list ? 1 : -1;
	if (a_user && a.constructor != b.constructor)
		return 0;
	// One conversion function: its second standard conversions decide.
	if (a_user && a.after != nullptr && b.after != nullptr)
		return compare(*a.after, *b.after);
	// p3: a proper subsequence (the second and third steps, lvalue
	// transformations excluded; identity is a subsequence of any other).
	const bool a_identity = a.second == ValueConversion::None && !a.qualification;
	const bool b_identity = b.second == ValueConversion::None && !b.qualification;
	if (a_identity != b_identity)
		return a_identity ? 1 : -1;
	if (a.second == b.second && a.qualification != b.qualification)
		return a.qualification ? -1 : 1;
	if (a.rank != b.rank)
		return a.rank < b.rank ? 1 : -1;
	// p4: of the same rank, a conversion that does not convert a pointer,
	// member pointer or nullptr_t to bool beats one that does.
	const bool a_bool = a.second == ValueConversion::PointerBoolean;
	const bool b_bool = b.second == ValueConversion::PointerBoolean;
	if (a_bool != b_bool)
		return a_bool ? -1 : 1;
	// B* to A* beats B* to void*.
	if (a.second == ValueConversion::DerivedToBase && b.second == ValueConversion::PointerToVoid)
		return 1;
	if (b.second == ValueConversion::DerivedToBase && a.second == ValueConversion::PointerToVoid)
		return -1;
	// Of two derived-to-base conversions from one class (of pointers, of
	// class objects or reference bindings), the one to the more derived
	// base is better.
	if (a.second == ValueConversion::DerivedToBase && b.second == ValueConversion::DerivedToBase)
	{
		const ClassEntity* ca = ClassEntityOf(DerivationTarget(a));
		const ClassEntity* cb = ClassEntityOf(DerivationTarget(b));
		if (ca != cb && ca != nullptr && cb != nullptr)
		{
			if (is_base_of(cb, ca))
				return 1;
			if (is_base_of(ca, cb))
				return -1;
		}
	}
	// Of two pointer to member conversions from one class, the one to the
	// less derived class is better (p4: A::* to B::* beats A::* to C::*).
	if (a.second == ValueConversion::MemberToDerived && b.second == ValueConversion::MemberToDerived)
	{
		const ClassEntity* ca = ClassEntityOf(a.result->unqualified->owner);
		const ClassEntity* cb = ClassEntityOf(b.result->unqualified->owner);
		if (ca != cb && ca != nullptr && cb != nullptr)
		{
			if (is_base_of(ca, cb))
				return 1;
			if (is_base_of(cb, ca))
				return -1;
		}
	}
	// Differing only in their qualification conversions, yielding similar
	// types: the less qualified result is better.
	if (a.second == b.second && a.qualification && b.qualification && a.form == ConversionForm::Standard &&
		b.form == ConversionForm::Standard && a.result != b.result && IsPointerLike(a.result) &&
		IsPointerLike(b.result))
	{
		if (qualification_convertible(a.result, b.result))
			return 1;
		if (qualification_convertible(b.result, a.result))
			return -1;
	}
	if (a.form == ConversionForm::Reference && b.form == ConversionForm::Reference)
	{
		// An rvalue reference bound to an rvalue beats an lvalue reference.
		const bool a_rvalue = a.rvalue_reference && !a.function_lvalue;
		const bool b_rvalue = b.rvalue_reference && !b.function_lvalue;
		if (a_rvalue != b_rvalue && !a.function_lvalue && !b.function_lvalue && !a.implicit_object &&
			!b.implicit_object && (a_rvalue ? !b.rvalue_reference : !a.rvalue_reference))
			return a_rvalue ? 1 : -1;
		// An lvalue reference bound to a function lvalue beats an rvalue one.
		if (a.function_lvalue && b.function_lvalue && a.rvalue_reference != b.rvalue_reference)
			return a.rvalue_reference ? -1 : 1;
		// References to the same type but for cv (an array's its elements',
		// 3.9.3p5): the less qualified wins.
		const Type* ra = a.target->element;
		const Type* rb = b.target->element;
		const uint8_t cva = CvOf(ra), cvb = CvOf(rb);
		if (cva != cvb && without_cv(ra->canonical) == without_cv(rb->canonical))
		{
			if ((cva & ~cvb) == 0)
				return 1;
			if ((cvb & ~cva) == 0)
				return -1;
		}
	}
	return 0;
}

} // namespace sema
