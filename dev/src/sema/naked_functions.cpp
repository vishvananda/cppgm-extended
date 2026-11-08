#include "sema/analyzer.h"

namespace sema {
namespace {
const syntax::Stmt* invalid_statement(const syntax::Stmt* statement)
{
	if (statement->kind == syntax::StmtKind::Compound)
	{
		for (const syntax::Stmt* child : static_cast<const syntax::CompoundStmt*>(statement)->items)
			if (const syntax::Stmt* invalid = invalid_statement(child))
				return invalid;
		return nullptr;
	}
	if (statement->kind == syntax::StmtKind::Expression &&
		static_cast<const syntax::ExpressionStmt*>(statement)->expression == nullptr)
		return nullptr;
	if (statement->kind == syntax::StmtKind::Asm)
	{
		const syntax::AsmStmt& source = *static_cast<const syntax::AsmStmt*>(statement);
		if (!source.is_extended && !source.is_goto)
			return nullptr;
	}
	return statement;
}
}

void Analyzer::check_naked_definition(const FunctionEntity* function, const syntax::FunctionBody& body)
{
	if (function->constructor || function->destructor || function->always_inline || function->is_constexpr ||
		body.ctor_initializer != nullptr || body.try_block != nullptr)
		error(body.body->location, "naked function cannot require C++ entry, exit or inline machinery");
	if (const syntax::Stmt* invalid = invalid_statement(body.body))
		error(invalid->location, "naked function permits only basic asm and empty statements");
}

} // namespace sema
