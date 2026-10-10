#pragma once

#include <cstdint>
#include <utility>
#include <vector>

#include "sema/semantic_graph.h"
#include "syntax/ast.h"

namespace sema {

class Analyzer;

// Evaluates the integral constant expressions of expressions the analysis
// has not typed (types mode, a template definition's non-dependent ones;
// semantics mode evaluates typed expressions by sema/constant_interpreter.h),
// as PA6 needs them (array bounds,
// enumerator values, static_assert, const variable initializers, 5.19):
// integer, character and boolean literals, enumerators, const integral
// variables and references initialized with constant expressions,
// sizeof and alignof of types, casts to integral and enumeration types,
// and the arithmetic, relational, logical and conditional operators with
// the integral promotions and usual arithmetic conversions. Operands that
// are not evaluated (the unselected side of `&&`, `||` and `?:`) are only
// typed. An expression that is not constant yields false with a location
// and reason; ill-formed operands (a scoped enumeration mixed with an
// integer, an undeclared name) are errors of the translation unit.
class ConstantEvaluator
{
public:
	ConstantEvaluator(Analyzer& analyzer, Scope* scope);

	bool evaluate(const syntax::Expr& expr, Constant& value);
	// The type of an operand of sizeof or decltype that is not an
	// id-expression: typed without being evaluated.
	bool type_of(const syntax::Expr& expr, const Type*& type);

	uint32_t failure_location() const { return failure_location_; }
	const char* failure() const { return failure_; }

private:
	bool eval(const syntax::Expr& expr, Constant& value);
	bool chain(const syntax::BinaryExpr& expr, Constant& value);
	bool combine(const syntax::BinaryExpr& node, const Constant& left, Constant& value);
	bool logical(const syntax::BinaryExpr& node, const Constant& left, Constant& value);
	bool arithmetic(TokenKind op, const Constant& left, const Constant& right, uint32_t location, Constant& value);
	bool shift(TokenKind op, const Constant& left, const Constant& right, uint32_t location, Constant& value);
	bool compare(TokenKind op, const Constant& left, const Constant& right, uint32_t location, Constant& value);
	bool unary(const syntax::UnaryExpr& expr, Constant& value);
	bool conditional(const syntax::ConditionalExpr& expr, Constant& value);
	bool cast(const Type* type, const syntax::Expr* operand, uint32_t location, Constant& value);
	bool functional_cast(const syntax::FunctionalCastExpr& expr, Constant& value);
	bool identifier(const syntax::IdExpr& expr, Constant& value);
	bool entity_value(const Entity& entity, uint32_t location, Constant& value);
	// The target type of a cast: recorded by the typing, else analyzed.
	const Type* cast_target(const syntax::Expr& expr, const syntax::TypeId* type);
	bool literal(const syntax::LiteralExpr& expr, Constant& value);
	bool string_element(const syntax::SubscriptExpr& expr, Constant& value);
	bool size_of(const syntax::SizeofExpr& expr, Constant& value);
	bool operand_type(const syntax::Expr& expr, const Type*& type);
	bool to_bool(const Constant& value, uint32_t location, bool& result);
	Constant boolean(bool value) const;
	void check_integral(const Constant& value, uint32_t location);
	bool fail(uint32_t location, const char* why);

	Analyzer& analyzer_;
	Scope* scope_;
	// Nonzero while typing an operand that is not evaluated.
	uint32_t skipping_;
	uint32_t failure_location_;
	const char* failure_;
	std::vector<const syntax::BinaryExpr*> spine_;
};

} // namespace sema
