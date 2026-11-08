#include "native/asm/binding.h"

#include <algorithm>

#include "asm/registers.h"
#include "native/backend_error.h"
#include "native/asm/constraints.h"

namespace native {
namespace {

using namespace asm_model;
using namespace mir_model;

[[noreturn]] void unsupported(const char* what)
{
	throw BackendError(std::string("assembly capability is not implemented yet: ") + what);
}

Reg physical(const Register& reg)
{
	if (reg.number >= 16 || (reg.bank != RegisterBank::Gpr && reg.bank != RegisterBank::Vector))
		unsupported("this register bank or extension");
	return static_cast<Reg>(reg.number + (reg.bank == RegisterBank::Vector ? 16 : 0));
}

unsigned width(lowir::Type type)
{
	if (type.kind() == lowir::TypeKind::V128)
		return 128;
	if (type.kind() == lowir::TypeKind::F32)
		return 32;
	if (type.kind() == lowir::TypeKind::F64)
		return 64;
	if (type.is_pointer())
		return 64;
	return std::max(8u, lowir::integer_bits(type.kind()));
}

void expand(std::string& source, const lowir::AssemblyModule& module, const lowir::AssemblyTemplate& definition,
	const AssemblyPlan& plan, std::vector<assembly::SymbolUse>& symbols, std::vector<assembly::LabelUse>& labels)
{
	for (uint32_t i = 0; i < definition.piece_count; ++i)
	{
		const Piece& piece = module.pieces[definition.first_piece + i];
		if (piece.kind == PieceKind::Text)
		{
			source.append(module.text, piece.first, piece.count);
			continue;
		}
		if (piece.kind == PieceKind::Unique)
		{
			// Each parsed packet has its own label namespace. The emitter
			// allocates fresh label identities for every cloned occurrence.
			source += '0';
			continue;
		}
		if (piece.format == Format::Suffix)
		{
			const unsigned bits = piece.bits != 0
				? piece.bits
				: width(module.operands[definition.first_operand + piece.operand].type);
			const lowir::TypeKind kind = module.operands[definition.first_operand + piece.operand].type.kind();
			const char suffix = kind == lowir::TypeKind::F32 ? 's'
				: kind == lowir::TypeKind::F64               ? 'l'
				: kind == lowir::TypeKind::F80               ? 't'
				: bits == 8                                  ? 'b'
				: bits == 16                                 ? 'w'
				: bits == 32                                 ? 'l'
				: bits == 64                                 ? 'q'
															 : 0;
			if (suffix == 0)
				throw BackendError("invalid assembly mnemonic width modifier");
			source += suffix;
			continue;
		}
		if (plan.locations[piece.operand] == AL_LABEL)
		{
			if (piece.format != Format::Default && piece.format != Format::Label)
				unsupported("this outgoing label formatting modifier");
			const std::string marker = "__cppgm_label";
			labels.push_back(assembly::LabelUse{
				static_cast<uint32_t>(source.size()), static_cast<uint32_t>(marker.size()), piece.operand });
			source += marker;
			continue;
		}
		if (plan.locations[piece.operand] == AL_SYMBOL)
		{
			if (piece.format != Format::Default && piece.format != Format::Bare)
				unsupported("this symbol formatting modifier");
			if (definition.dialect == Dialect::Att && piece.format == Format::Default)
				source += '$';
			const std::string marker = "__cppgm_symbol";
			symbols.push_back(assembly::SymbolUse{ static_cast<uint32_t>(source.size()),
				static_cast<uint32_t>(marker.size()), plan.symbols[piece.operand] });
			source += marker;
			continue;
		}
		if (plan.locations[piece.operand] == AL_IMMEDIATE)
		{
			lowir::Literal bits = plan.constants[piece.operand];
			if (piece.format == Format::Negated)
			{
				bits.low = uint64_t(0) - bits.low;
				bits.high = ~bits.high + (bits.low == 0);
			}
			if (bits.high != 0 && (bits.high != ~uint64_t(0) || static_cast<int64_t>(bits.low) >= 0))
				unsupported("immediates exceeding 64 bits");
			if (piece.format != Format::Default && piece.format != Format::Bare && piece.format != Format::Negated)
				unsupported("this immediate formatting modifier");
			if (definition.dialect == Dialect::Att && piece.format == Format::Default)
				source += '$';
			source += bits.high != 0 ? std::to_string(static_cast<int64_t>(bits.low)) : std::to_string(bits.low);
			continue;
		}
		const Reg r = plan.registers[piece.operand];
		if (plan.locations[piece.operand] == AL_MEMORY)
		{
			if (piece.format != Format::Default)
				unsupported("this memory formatting modifier");
			const Register address{ RegisterBank::Gpr, r, 64, 0 };
			const std::string name = assembly::register_name(address);
			source += definition.dialect == Dialect::Att ? "(%" + name + ")" : "[" + name + "]";
			continue;
		}
		Register reg{ r < 16 ? RegisterBank::Gpr : RegisterBank::Vector, static_cast<uint8_t>(r < 16 ? r : r - 16),
			static_cast<uint16_t>(
				r >= 16 ? 128 : width(module.operands[definition.first_operand + piece.operand].type)),
			0 };
		if (piece.bits != 0)
			reg.bits = piece.bits;
		reg.offset = piece.offset;
		if (piece.format != Format::Default && piece.format != Format::Bare)
			unsupported("this register formatting modifier");
		const std::string name = assembly::register_name(reg);
		if (name.empty())
			throw BackendError("assembly modifier requests an invalid register view");
		if (definition.dialect == Dialect::Att && piece.format != Format::Bare)
			source += '%';
		source += name;
	}
}

} // namespace

std::shared_ptr<const AssemblyPlan> bind_assembly(const lowir::AssemblyModule& module, uint32_t id,
	const lowir::FunctionBody& body, const std::vector<lowir::Value>& arguments)
{
	const lowir::AssemblyTemplate& definition = module.templates[id];
	if ((definition.properties & (AP_UNWIND | AP_NORETURN)) != 0)
		unsupported("unwinding or divergent assembly");
	std::shared_ptr<AssemblyPlan> plan(new AssemblyPlan());
	plan->definition = id;
	for (uint32_t i = 0; i < definition.operand_count; ++i)
		if ((module.operands[definition.first_operand + i].flags & AO_EXIT_TAG) != 0)
			plan->exit_operand = i;
	for (uint32_t i = 0; i < definition.operand_count; ++i)
		if ((module.operands[definition.first_operand + i].flags & AO_FLAG) != 0 &&
			module.operands[definition.first_operand + i].type.kind() == lowir::TypeKind::I128)
			unsupported("128-bit flag outputs");
	uint32_t clobbers = 0;
	for (uint32_t i = 0; i < definition.clobber_count; ++i)
	{
		const Reg reg = physical(module.clobbers[definition.first_clobber + i]);
		if (reg == RSP || reg == RBP)
			unsupported("stack or frame register clobbers");
		clobbers |= reg_bit(reg);
	}
	static_cast<AssemblyBinding&>(*plan) = assign_assembly_operands(module, definition, clobbers, body, arguments);
	plan->writes = clobbers;
	for (uint32_t i = 0; i < definition.operand_count; ++i)
	{
		const lowir::AssemblyOperand& operand = module.operands[definition.first_operand + i];
		if (plan->registers[i] == kNoReg)
			continue;
		const uint32_t bit = reg_bit(plan->registers[i]);
		plan->writes |= bit; // input setup also overwrites its selected register
		if ((operand.flags & AO_READ) != 0 || plan->locations[i] == AL_MEMORY)
			plan->reads |= bit;
	}
	std::string source;
	std::vector<assembly::SymbolUse> symbols;
	std::vector<assembly::LabelUse> labels;
	expand(source, module, definition, *plan, symbols, labels);
	try
	{
		plan->instructions = assembly::parse(source, definition.dialect, symbols, labels);
	}
	catch (const assembly::AssemblyError& error)
	{
		throw BackendError("assembly line " + std::to_string(error.line) + ": " + error.message);
	}
	append_assembly_exits(*plan, definition);
	return plan;
}

} // namespace native
