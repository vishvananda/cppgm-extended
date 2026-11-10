#include "lower/function_lowering.h"

#include "asm/packet.h"
#include "lowir/assembly_builder.h"
#include "sema/asm_statements.h"

namespace lower {
namespace {
void append_source(const syntax::Stmt* statement, std::string& text, uint32_t& environment)
{
	if (statement->kind == syntax::StmtKind::Compound)
	{
		for (const syntax::Stmt* child : static_cast<const syntax::CompoundStmt*>(statement)->items)
			append_source(child, text, environment);
	}
	else if (statement->kind == syntax::StmtKind::Asm)
	{
		const syntax::AsmStmt& source = *static_cast<const syntax::AsmStmt*>(statement);
		environment = source.assembly_environment;
		const assembly::Packet& packet = *source.program->packet;
		for (uint32_t i = 0; i < packet.pieces.size; ++i)
		{
			const asm_model::Piece& piece = packet.pieces.data[i];
			text.append(packet.text + piece.first, piece.count);
		}
		text += '\n';
	}
}
}

void FunctionLowering::lower_naked_definition()
{
	// These are user source fragments, joined before their sole instruction
	// parse. Labels may cross basic statements, as in one assembler body.
	std::string text;
	uint32_t environment = 0;
	append_source(function_->body->body, text, environment);
	assembly::Packet packet;
	packet.definition.environment = lowering_.assembly_environment(environment);
	packet.definition.dialect = asm_model::Dialect::Att;
	packet.definition.memory = asm_model::Memory::ReadWrite;
	packet.definition.properties = asm_model::AP_SIDE_EFFECT;
	packet.definition.piece_count = 1;
	packet.text = text.data();
	packet.text_size = static_cast<uint32_t>(text.size());
	asm_model::Piece piece;
	piece.kind = asm_model::PieceKind::Text;
	piece.count = packet.text_size;
	packet.pieces.data = &piece;
	packet.pieces.size = 1;
	lowir::Instruction inst = lowir::make_instruction(lowir::Opcode::Asm, lowir::Type::scalar(lowir::TypeKind::Void));
	inst.immediate = lowir::add_assembly(
		lowering_.program(), packet, nullptr, lowering_.debug_location(function_->body->body->location));
	builder_.append(inst);
	terminate(lowir::make_instruction(lowir::Opcode::Unreachable, lowir::Type::scalar(lowir::TypeKind::Void)));
	builder_.finish();
}

} // namespace lower
