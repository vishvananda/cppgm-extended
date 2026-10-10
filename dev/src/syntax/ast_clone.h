#pragma once

#include <utility>
#include <vector>

#include "support/memory/arena.h"
#include "syntax/ast.h"

// Copies of syntax subtrees for template instantiation. Semantic
// analysis records facts on some nodes (expressions, declared bindings,
// case values, anonymous union storage; see the mutable members in
// syntax/ast_*.h), so each specialization analyzes its own copy of the
// fact-bearing nodes of a pattern and of the nodes leading to them. Any
// subtree without such nodes (names, specifiers and declarators without
// expressions) is shared with the pattern: a clone returns its input when
// nothing below it changed. Nothing is reparsed; copies live in the
// given arena (the unit's semantic arena) as long as the unit.
namespace syntax {

// The parameter packs of an instantiation (14.5.3): a copy made for
// it expands each pack expansion into one copy of its pattern per element,
// in which the pattern's pack names are renamed to names the
// instantiation binds to the elements. So analysis and lowering never see
// an unexpanded pack in an instantiation.
class PackExpander
{
public:
	// The number of elements of the pack `name` denotes where the copy is
	// analyzed, or -1 when it names no pack.
	virtual int32_t pack_length(const Identifier* name) = 0;
	// The name that denotes element `index` of the pack `name`.
	virtual const Identifier* element_name(const Identifier* name, uint32_t index) = 0;
	// An expansion whose packs have different lengths (14.5.3p7): an
	// error of the instantiation (does not return).
	virtual void mismatch(uint32_t location) = 0;

protected:
	~PackExpander() {}
};

class SyntaxCloner
{
public:
	// `bodies`: function bodies of member declarations are copied too;
	// otherwise they stay shared (a class instantiation copies a body only
	// when the member is demanded). `expander`: the instantiation's packs,
	// if it has any.
	SyntaxCloner(Arena& arena, bool bodies, PackExpander* expander = nullptr)
		: arena_(arena), bodies_(bodies), expander_(expander), collecting_(nullptr)
	{}

	// The packs an expression names outside the pack expansions nested in
	// it (by the expander's pack_length), without copying it for use, and
	// (`nested`) those the expansions nested in it name; how many
	// expansions (template argument, expression, fold) it nests, and how
	// many sizeof... and `this` expressions.
	void collect_packs(
		const Expr* expr, std::vector<const Identifier*>& out, std::vector<const Identifier*>* nested = nullptr);
	uint32_t nested_expansions() const { return expansions_; }
	uint32_t pack_sizes() const { return pack_sizes_; }
	uint32_t this_uses() const { return this_uses_; }
	// The function parameter packs the copied parameter clauses expanded,
	// in order (name, null for an unnamed one, and element count), which
	// the copied body then expands too.
	const std::vector<std::pair<const Identifier*, uint32_t>>& parameter_packs() const { return parameter_packs_; }
	// A copy of an expression that is one element of an expansion the
	// caller expands (each pack's name, outside sizeof... and the nested
	// expansions of its own, renamed to its element's, `renames`).
	Expr* element_expr(const Expr* expr, const std::vector<std::pair<const Identifier*, const Identifier*>>& renames);

	FunctionBody* body(const FunctionBody& body);
	ClassSpecifier* class_specifier(const ClassSpecifier* specifier);
	Initializer* initializer(const Initializer* initializer);
	Expr* expr(const Expr* expr);
	Stmt* stmt(const Stmt* stmt);
	Decl* decl(const Decl* decl);
	TypeId* type_id(const TypeId* type);
	Declarator* declarator(const Declarator* declarator);
	// `leading`: the name's first component may name a pack (not a
	// member access's member).
	Name* name(const Name* name, bool leading = true);
	ParameterClause* parameter_clause(const ParameterClause* clause);
	EnumSpecifier* enum_specifier(const EnumSpecifier* specifier);

private:
	template<typename T>
	T* copy(const T* node);
	template<typename T>
	Slice<T*> pointers(const Slice<T*>& list, T* (SyntaxCloner::*each)(const T*));
	Slice<Expr*> exprs(const Slice<Expr*>& list);
	Slice<DeclSpecifier> specifiers(const Slice<DeclSpecifier>& list);
	bool specifier(const DeclSpecifier& in, DeclSpecifier& out);
	bool component(const NameComponent& in, NameComponent& out, bool leading);
	Attribute* attributes(const Attribute* list) { return list != nullptr ? attribute_list(list) : nullptr; }
	Attribute* attribute_list(const Attribute* list);
	bool suffix(const DeclaratorSuffix& in, DeclaratorSuffix& out);
	bool qualifier(const FunctionQualifier& in, FunctionQualifier& out);
	ParameterDeclaration* parameter(const ParameterDeclaration* parameter);
	CompoundStmt* compound(const CompoundStmt* stmt);
	bool condition(const Condition& in, Condition& out);
	Slice<Handler> handlers(const Slice<Handler>& list);
	Slice<InitDeclarator> init_declarators(const Slice<InitDeclarator>& list);
	CtorInitializer* ctor_initializer(const CtorInitializer* list);
	Expr* call_like(const Expr* expr);
	Expr* type_expr(const Expr* expr);

	// Pack expansion (see PackExpander).
	int32_t length(const Identifier* name) const;
	const Identifier* renamed(const Identifier* name) const;
	template<typename Copy>
	int32_t pattern_packs(Copy copy, std::vector<const Identifier*>& packs, uint32_t location);
	void rename_to(const std::vector<const Identifier*>& packs, uint32_t index);
	Slice<Expr*> expanded_exprs(const Slice<Expr*>& list);
	Expr* expanded_fold(const FoldExpr* fold);
	Slice<TemplateArgument> expanded_arguments(const Slice<TemplateArgument>& list);
	Slice<TypeId*> expanded_type_ids(const Slice<TypeId*>& list);
	ParameterDeclaration* parameter_element(const ParameterDeclaration* parameter);
	Slice<ParameterDeclaration*> expanded_parameters(const Slice<ParameterDeclaration*>& list);
	Slice<BaseSpecifier> bases(const Slice<BaseSpecifier>& list);
	Slice<LambdaCapture> captures(const Slice<LambdaCapture>& list);
	Slice<MemInitializer> mem_initializers(const Slice<MemInitializer>& list);
	void shadow(const Declarator* declarator, int32_t length);
	void shadow_declared(const Decl* decl);
	void shadow_template_packs(const TemplateParameterList* list);

	Arena& arena_;
	bool bodies_;
	PackExpander* expander_;
	// While copying one element of an expansion: each pack name of the
	// pattern and the name of its element.
	std::vector<std::pair<const Identifier*, const Identifier*>> renames_;
	// While copying a pattern only to find its packs: the packs found
	// outside nested expansions.
	std::vector<const Identifier*>* collecting_;
	std::vector<std::pair<const Identifier*, uint32_t>> parameter_packs_;
	uint32_t expansions_ = 0;
	std::vector<const Identifier*>* nested_ = nullptr;
	template<typename Walk>
	void nested_expansion(Walk walk);
	uint32_t pack_sizes_ = 0;
	uint32_t this_uses_ = 0;
	// Pack names that the copied declarations redeclare where they are in
	// scope: -1 for a block-scope declaration that hides a function
	// parameter pack (3.3.3), -2 for a pack of a template declared in the
	// copy, which that template's instantiations expand.
	std::vector<std::pair<const Identifier*, int32_t>> shadows_;
};

} // namespace syntax
