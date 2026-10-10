#pragma once

#include <cstdint>
#include <vector>

#include "sema/expression_facts.h"
#include "sema/type_table.h"

namespace sema {

// What a conversion needs to know about its source expression.
struct Operand
{
	// No reference type (5p5).
	const Type* type;
	ValueCategory category;
	// ExprFacts flags: kNullLiteral, kBitField.
	uint16_t flags;
};

// The implicit conversions of PA7's procedural slice over interned types:
// standard conversion sequences (4, 13.3.3.1.1) with the lvalue
// transformations, integral and floating promotions and conversions,
// null pointer, void pointer, derived-to-base and boolean conversions and
// qualification adjustments; reference bindings (8.5.3); and their ranking
// (13.3.3.2). User-defined conversions are a later stage's: a class
// operand converts only to its own class or a base.
// Completes a class that is a template instance not yet defined (14.7.1p1:
// whether one class derives from another affects the program's meaning).
class ClassCompleter
{
public:
	virtual bool complete(const ClassEntity* entity) = 0;

protected:
	~ClassCompleter() {}
};

class ConversionRules
{
public:
	explicit ConversionRules(TypeTable& types) : types_(types), visit_(0), completer_(nullptr), arithmetic_(false) {}

	void set_completer(ClassCompleter* completer) { completer_ = completer; }
	// The conversions of a built-in operator's operands to the type of the
	// usual arithmetic conversions (5p10), which alone convert between
	// bfloat16 and binary16 (Clang's rule).
	void set_arithmetic_operand(bool on) { arithmetic_ = on; }

	// The implicit conversion sequence that copy-initializes an object or
	// reference of type `target` from `from` (8.5p16), false if there is
	// none. `direct` selects direct-initialization, which also converts
	// std::nullptr_t to bool (4.12).
	bool implicit(const Operand& from, const Type* target, bool direct, Conversion& out);

	// 13.3.3.2: positive when `a` is the better conversion sequence,
	// negative when `b` is, zero when they are indistinguishable.
	int compare(const Conversion& a, const Conversion& b);

	// 10: `base` is a proper base class of `derived` (a walk of the acyclic
	// base graph visiting each class once).
	bool is_base_of(const ClassEntity* base, const ClassEntity* derived);
	// A fresh mark for another walk of the base graph through
	// ClassEntity::derivation_visit (one that runs no derivation query
	// while it walks).
	uint32_t new_visit() { return ++visit_; }
	// 8.5.3p4: same class or `a` a base class of `b`, ignoring cv.
	bool reference_related(const Type* a, const Type* b);
	// A type without its cv-qualifiers (an array's are its elements').
	const Type* without_cv(const Type* type);

	// 4.4: a pointer or pointer to member `from` converts to the similar
	// type `to` by adding cv-qualifiers below the top level, with `const`
	// at every intermediate level where a lower one changes (4.4p4).
	static bool qualification_convertible(const Type* from, const Type* to);
	// 5.2.11p8-p10: converting pointer (or member pointer) type `from` to
	// `to` casts away constness, which only const_cast may do.
	static bool casts_away_constness(const Type* from, const Type* to);
	// 5.2.11p3, p5: the pointer types differ at most in cv-qualifiers at
	// each level (what const_cast converts between).
	static bool const_castable(const Type* from, const Type* to);

private:
	bool standard(const Operand& from, const Type* target, bool direct, Conversion& out);
	bool reference(const Operand& from, const Type* target, Conversion& out);
	bool value_conversion(const Type* source, uint16_t flags, const Type* target, bool direct, Conversion& out);

	TypeTable& types_;
	// Scratch of is_base_of, reused across queries, and its visit mark.
	std::vector<const ClassEntity*> base_stack_;
	uint32_t visit_;
	ClassCompleter* completer_;
	bool arithmetic_;
};

// Type classification shared by the conversion rules and expression typing.
bool IsArithmetic(const Type* type);
bool IsFloating(const Type* type);
// A GNU complex type (`_Complex T`).
bool IsComplex(const Type* type);
// Integral or unscoped enumeration (the operands of integral promotion).
bool IsIntegralOrUnscopedEnum(const Type* type);
bool IsScalar(const Type* type);
// A pointer to an object type (or to void).
bool IsObjectPointer(const Type* type);
inline bool IsVoid(const Type* type)
{
	type = type->unqualified;
	return type->kind == TypeKind::Fundamental && type->fundamental == FundamentalType::Void;
}
// The canonical cv-unqualified class entity of a class type, or null.
inline const ClassEntity* ClassOf(const Type* type)
{
	type = type->unqualified;
	return type->kind == TypeKind::Class ? static_cast<const ClassEntity*>(type->entity) : nullptr;
}
// The class a non-static member is named in and a pointer to it is of:
// an anonymous union's or struct's member is its enclosing class's
// (9.5p5), through nested ones; at namespace or block scope the outermost
// anonymous class (which holds the unnamed object).
const ClassEntity* MemberClassOf(const Entity* member);
// How many of a class's virtual bases are no direct bases of it.
uint32_t IndirectVirtualBases(const ClassEntity* cls);

} // namespace sema
