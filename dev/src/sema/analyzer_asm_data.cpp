#include "sema/analyzer.h"

namespace sema {

void Analyzer::asm_declaration(const syntax::AsmDecl& decl, Scope* scope)
{
	if (declaration_scope(scope)->kind != ScopeKind::Namespace || template_depth_ != 0)
		error(decl.location, "an asm definition outside namespace scope");
	StringLiteralFacts facts;
	std::vector<uint64_t> units;
	if (!StringLiteralUnits(decl.text, facts, units) || facts.element != FundamentalType::Char ||
		facts.suffix_length != 0 || units.empty())
		error(decl.location, "an asm definition needs an ordinary string literal");
	std::string text;
	text.reserve(units.size() - 1);
	for (size_t i = 0; i + 1 < units.size(); ++i)
		text.push_back(static_cast<char>(units[i]));
	decl.assembly_source = arena_.copy_string(text.data(), text.size());
	decl.assembly_size = static_cast<uint32_t>(text.size());
}

} // namespace sema
