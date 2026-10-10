#pragma once

#include <string>
#include <vector>

#include "syntax/ast.h"

namespace syntax {

// Compact spellings of syntax used in AST dump labels: names, type-ids,
// expressions (lambda bodies included) and the statements and declarations
// they contain, as their tokens, concatenated without white space except
// after the keywords that must stay separate from what follows (cv-
// qualifiers, class-keys, `enum`, `typename`, `template`), and after
// `operator` in a qualified name before a word. A template-id's arguments
// are written as written, so `vector<pair<size_t, size_t> >` is
// `vector<pair<size_t,size_t>>`.
class SyntaxRenderer
{
public:
	explicit SyntaxRenderer(std::string& out) : out_(out) {}

	// With `typename_keyword`, a typename-specifier keeps its keyword (it
	// does inside template arguments, not as a dump label's type name).
	void name(const Name& name, bool typename_keyword);
	void type_id(const TypeId& type);
	void expression(const Expr& expr);
	void template_arguments(const Slice<TemplateArgument>& arguments);
	void declarator(const Declarator& declarator);
	void specifiers(const Slice<DeclSpecifier>& specifiers);
	void parameters(const ParameterClause& clause);
	void function_qualifier(const FunctionQualifier& qualifier);
	void expression_list(const Slice<Expr*>& list);
	void lambda_introducer(const LambdaExpr& lambda);
	void statement(const Stmt& stmt);
	void declaration(const Decl& decl);

private:
	void text(const TokenSpelling& spelling) { out_.append(spelling.text, spelling.length); }
	void text(const char* text) { out_ += text; }
	void identifier(const Identifier* identifier);
	void component(const NameComponent& component, bool qualified);
	void specifier(const DeclSpecifier& specifier);
	void initializer(const Initializer& initializer);
	void operand(const Expr& expr);
	void chain_rest(const Expr& expr);
	void call_like(const Expr& expr);
	void lambda(const LambdaExpr& lambda);
	void class_specifier(const ClassSpecifier& specifier);
	void enum_specifier(const EnumSpecifier& specifier);
	void init_declarators(const Slice<InitDeclarator>& declarators);
	void function_body(const FunctionBody& body);
	void handlers(const Slice<Handler>& handlers);
	void condition(const Condition& condition);
	void template_parameters(const TemplateParameterList& list);

	std::string& out_;
	// Chain nodes whose left operand is being rendered (see expression).
	std::vector<const Expr*> spine_;
};

// The left operand of a node the parser builds in a loop (binary and
// assignment operators, calls, subscripts, member accesses, postfix
// increments), or null. Such chains can be much deeper than the nesting the
// parser admits, so consumers walk them without recursing.
const Expr* ChainOperand(const Expr& expr);

std::string RenderName(const Name& name);
std::string RenderTypeId(const TypeId& type);
std::string RenderExpression(const Expr& expr);

} // namespace syntax
