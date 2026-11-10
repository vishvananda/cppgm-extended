#include "lower/program_lowering.h"

#include "lowir/assembly_builder.h"
#include "lowir/assembly.h"

namespace lower {

void ProgramLowering::assembly_declarations(const syntax::Slice<syntax::Decl*>& list)
{
	for (const syntax::Decl* decl : list)
	{
		if (decl->kind == syntax::DeclKind::Asm)
		{
			const auto& source = static_cast<const syntax::AsmDecl&>(*decl);
			asm_declaration(source);
		}
		else if (decl->kind == syntax::DeclKind::Namespace)
			assembly_declarations(static_cast<const syntax::NamespaceDecl&>(*decl).declarations);
		else if (decl->kind == syntax::DeclKind::Linkage)
			assembly_declarations(static_cast<const syntax::LinkageDecl&>(*decl).declarations);
	}
}

void ProgramLowering::asm_declaration(const syntax::AsmDecl& decl)
{
	assembly::Packet packet;
	packet.text = decl.assembly_source;
	packet.text_size = decl.assembly_size;
	packet.definition.piece_count = 1;
	asm_model::Piece piece;
	piece.count = decl.assembly_size;
	packet.pieces.data = &piece;
	packet.pieces.size = 1;
	lowir::AssemblyGlobal item;
	item.definition = lowir::add_assembly(program_, packet, nullptr, debug_location(decl.location));
	program_.assembly->globals.push_back(std::move(item));
}

} // namespace lower
