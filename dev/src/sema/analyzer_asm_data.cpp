#include "sema/analyzer.h"

#include "sema/asm_data.h"

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
	std::vector<AsmDataImage> images;
	std::string failure;
	if (!LiftAsmData(text, images, failure))
		error(decl.location, failure.c_str());
	AsmDataSection* sections = arena_.allocate_array<AsmDataSection>(images.size());
	for (size_t i = 0; i < images.size(); ++i)
	{
		const AsmDataImage& image = images[i];
		sections[i] = AsmDataSection{ arena_.copy_string(image.name.data(), image.name.size()),
			arena_.copy_array(image.bytes.data(), image.bytes.size()), static_cast<uint32_t>(image.bytes.size()),
			image.strings };
	}
	decl.sections = sections;
	decl.section_count = static_cast<uint32_t>(images.size());
}

} // namespace sema
