#include "lowir/assembly_builder.h"

#include "lowir/assembly.h"

namespace lowir {
namespace {

template<typename T>
uint32_t append(std::vector<T>& into, assembly::View<T> from)
{
	const uint32_t first = static_cast<uint32_t>(into.size());
	if (from.size != 0)
		into.insert(into.end(), from.data, from.data + from.size);
	return first;
}

} // namespace

uint32_t add_assembly(Program& program, const assembly::Packet& packet, const Type* types, DebugLocation location)
{
	if (!program.assembly)
		program.assembly.reset(new AssemblyModule());
	AssemblyModule& module = *program.assembly;
	const uint32_t text = static_cast<uint32_t>(module.text.size());
	module.text.append(packet.text, packet.text_size);
	const uint32_t registers = append(module.registers, packet.registers);
	const uint32_t clobbers = append(module.clobbers, packet.clobbers);
	const uint32_t immediates = append(module.immediates, packet.immediates);
	const uint32_t groups = static_cast<uint32_t>(module.groups.size());
	for (uint32_t i = 0; i < packet.groups.size; ++i)
	{
		AssemblyRegisterGroup group = packet.groups.data[i];
		group.first_register += registers;
		module.groups.push_back(group);
	}
	const uint32_t alternatives = static_cast<uint32_t>(module.alternatives.size());
	for (uint32_t i = 0; i < packet.alternatives.size; ++i)
	{
		AssemblyAlternative alternative = packet.alternatives.data[i];
		alternative.first_group += groups;
		alternative.first_immediate += immediates;
		module.alternatives.push_back(alternative);
	}
	const uint32_t operands = static_cast<uint32_t>(module.operands.size());
	for (uint32_t i = 0; i < packet.operands.size; ++i)
	{
		AssemblyOperand operand;
		static_cast<asm_model::Operand&>(operand) = packet.operands.data[i];
		operand.type = types[i];
		operand.first_alternative += alternatives;
		module.operands.push_back(operand);
	}
	const uint32_t pieces = static_cast<uint32_t>(module.pieces.size());
	for (uint32_t i = 0; i < packet.pieces.size; ++i)
	{
		AssemblyPiece piece = packet.pieces.data[i];
		if (piece.kind == asm_model::PieceKind::Text)
			piece.first += text;
		module.pieces.push_back(piece);
	}
	AssemblyTemplate definition;
	static_cast<asm_model::Template&>(definition) = packet.definition;
	definition.first_operand += operands;
	definition.first_piece += pieces;
	definition.first_clobber += clobbers;
	definition.location = location;
	const uint32_t id = static_cast<uint32_t>(module.templates.size());
	const std::string name = "_cppgm_asm." + std::to_string(id);
	definition.name = program.names.intern(name.data(), static_cast<uint32_t>(name.size()));
	module.templates.push_back(definition);
	return id;
}

} // namespace lowir
