#include "lower/program_lowering.h"

#include <cstring>

#include "sema/asm_data.h"
#include "support/internal_error.h"

namespace lower {

void ProgramLowering::asm_declaration(const syntax::AsmDecl& decl)
{
	if (decl.sections == nullptr)
		throw InternalError("namespace asm has no analyzed section facts", decl.location);
	for (uint32_t i = 0; i < decl.section_count; ++i)
	{
		const sema::AsmDataSection& section = decl.sections[i];
		const lowir::SymbolId symbol = new_symbol("__cppgm_asm_data");
		lowir::Global global = lowir::Global();
		global.init = lowir::GlobalInit::Data;
		global.first_item = static_cast<uint32_t>(program_.data_items.size());
		global.item_count = section.size;
		for (uint32_t k = 0; k < section.size; ++k)
		{
			lowir::DataItem item = lowir::DataItem();
			item.kind = lowir::DataItemKind::Scalar;
			item.type = lowir::Type::scalar(lowir::TypeKind::U8);
			item.constant.kind = lowir::ValueKind::Integer;
			item.constant.bits.low = section.bytes[k];
			program_.data_items.push_back(item);
		}
		program_.declare_global(symbol, lowir::SymbolKind::GlobalDefinition, global);
		lowir::SymbolMetadata& metadata = program_.symbols[symbol].metadata;
		metadata.binding = ir_model::SBM_INTERNAL;
		metadata.flags |= ir_model::SF_OBJECT_ROOT;
		metadata.section = intern(section.name, static_cast<uint32_t>(std::strlen(section.name)));
		metadata.storage = section.strings ? ir_model::GSM_METADATA_STRINGS : ir_model::GSM_METADATA;
		++counters_.globals;
	}
}

} // namespace lower
