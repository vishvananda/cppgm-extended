#include "lowir/assembly_validation.h"

#include <cstddef>

#include "lowir/assembly.h"

namespace lowir {
namespace {

using namespace asm_model;

bool invalid(const char* message, const char*& error)
{
	error = message;
	return false;
}

bool slice(uint32_t first, uint32_t count, std::size_t size)
{
	return first <= size && count <= size - first;
}

bool register_view(const Register& reg)
{
	const unsigned bank = static_cast<unsigned>(reg.bank);
	const unsigned counts[] = { 32, 32, 8, 8, 8, 8, 6, 16, 16 };
	if (bank >= sizeof counts / sizeof counts[0] || reg.number >= counts[bank])
		return false;
	if (reg.offset != 0)
		return reg.bank == RegisterBank::Gpr && reg.number < 4 && reg.bits == 8 && reg.offset == 8;
	if (reg.bits == 0)
		return true;
	switch (reg.bank)
	{
	case RegisterBank::Gpr:
	case RegisterBank::Mask:
		return reg.bits == 8 || reg.bits == 16 || reg.bits == 32 || reg.bits == 64;
	case RegisterBank::Vector:
		return reg.bits == 128 || reg.bits == 256 || reg.bits == 512;
	case RegisterBank::X87:
		return reg.bits == 80;
	case RegisterBank::Mmx:
	case RegisterBank::Control:
	case RegisterBank::Debug:
		return reg.bits == 64;
	case RegisterBank::Tile:
		return reg.bits == 8192;
	case RegisterBank::Segment:
		return reg.bits == 16;
	}
	return false;
}

bool integer_range(const Immediate& value)
{
	if (value.low.high != value.high.high)
	{
		const uint64_t sign = value.is_signed ? uint64_t(1) << 63 : 0;
		return (value.low.high ^ sign) < (value.high.high ^ sign);
	}
	return value.low.low <= value.high.low;
}

uint16_t register_units(const Register& reg)
{
	if (reg.bits == 0)
		return 0xffff;
	if (reg.bank == RegisterBank::Gpr || reg.bank == RegisterBank::Mask)
		return static_cast<uint16_t>(((1u << (reg.bits / 8)) - 1) << (reg.offset / 8));
	if (reg.bank == RegisterBank::Vector)
		return static_cast<uint16_t>((1u << (reg.bits / 128)) - 1);
	return 1;
}

bool register_group(const AssemblyModule& module, const RegisterGroup& group)
{
	if (group.register_count == 0 || !slice(group.first_register, group.register_count, module.registers.size()))
		return false;
	uint16_t used[9 * 32] = {};
	for (uint32_t i = 0; i < group.register_count; ++i)
	{
		const Register& reg = module.registers[group.first_register + i];
		if (reg.bits == 0 && group.register_count != 1)
			return false;
		const uint32_t unit = static_cast<uint32_t>(reg.bank) * 32 + reg.number;
		const uint16_t mask = register_units(reg);
		if ((used[unit] & mask) != 0)
			return false;
		used[unit] |= mask;
	}
	return true;
}

bool tables(const AssemblyModule& module, const char*& error)
{
	for (const Register& reg : module.registers)
		if (!register_view(reg))
			return invalid("invalid assembly register view", error);
	for (const Register& reg : module.clobbers)
		if (!register_view(reg))
			return invalid("invalid assembly register clobber", error);
	for (const RegisterGroup& group : module.groups)
		if (!register_group(module, group))
			return invalid("invalid assembly register group", error);
	for (const Immediate& value : module.immediates)
	{
		if (static_cast<unsigned>(value.kind) > static_cast<unsigned>(ImmediateKind::Relocatable))
			return invalid("invalid assembly immediate domain", error);
		if (value.kind == ImmediateKind::IntegerRange && !integer_range(value))
			return invalid("reversed assembly immediate range", error);
	}
	for (const Alternative& alt : module.alternatives)
	{
		if ((alt.locations & ~(AL_REGISTER | AL_MEMORY | AL_IMMEDIATE | AL_SYMBOL | AL_LABEL)) != 0 ||
			!slice(alt.first_group, alt.group_count, module.groups.size()) ||
			!slice(alt.first_immediate, alt.immediate_count, module.immediates.size()))
			return invalid("invalid assembly alternative", error);
		if (alt.matching == asm_model::kNoId && alt.locations == 0)
			return invalid("assembly alternative has no location", error);
		if ((alt.locations & AL_REGISTER) != 0 && alt.group_count == 0 && alt.matching == asm_model::kNoId)
			return invalid("assembly register alternative has no domain", error);
		if (alt.group_count != 0 && (alt.locations & AL_REGISTER) == 0)
			return invalid("assembly register domain without a register location", error);
		if (alt.immediate_count != 0 && (alt.locations & (AL_IMMEDIATE | AL_SYMBOL)) == 0)
			return invalid("assembly immediate domain without an immediate location", error);
	}
	return true;
}

struct Argument
{
	Type type = Type::scalar(TypeKind::Void);
	bool present = false;
};

bool argument(std::vector<Argument>& arguments, uint32_t index, Type type, const char*& error)
{
	if (index == lowir::kNoId)
		return true;
	Argument& arg = arguments[index];
	if (arg.present && arg.type != type)
		return invalid("inconsistent assembly argument types", error);
	arg.type = type;
	arg.present = true;
	return true;
}

bool operand(const AssemblyModule& module, const AssemblyTemplate& site, uint32_t index,
	std::vector<Argument>& arguments, uint32_t& columns, const char*& error)
{
	const AssemblyOperand& op = module.operands[site.first_operand + index];
	if (static_cast<unsigned>(op.type.kind()) > static_cast<unsigned>(TypeKind::Obj) ||
		(op.flags & ~(AO_READ | AO_WRITE | AO_COMMUTATIVE | AO_FLAG | AO_VOLATILE | AO_NO_MEMORY | AO_EXIT_TAG)) != 0 ||
		!slice(op.first_alternative, op.alternative_count, module.alternatives.size()) ||
		(op.input != lowir::kNoId && op.input >= site.argument_count) ||
		(op.output != lowir::kNoId && op.output >= site.argument_count))
		return invalid("invalid assembly operand boundary", error);
	if ((op.flags & (AO_READ | AO_WRITE)) == 0 || ((op.flags & AO_READ) != 0) != (op.input != lowir::kNoId) ||
		(op.output != lowir::kNoId && (op.flags & AO_WRITE) == 0) || (op.input_storage && (op.flags & AO_READ) == 0))
		return invalid("inconsistent assembly operand access", error);
	if ((op.flags & AO_FLAG) != 0)
	{
		if ((op.flags & AO_EXIT_TAG) != 0 || (op.flags & (AO_READ | AO_WRITE)) != AO_WRITE ||
			op.alternative_count != 0 || op.condition >= 16 || op.type.kind() < TypeKind::I1 ||
			op.type.kind() > TypeKind::I128)
			return invalid("invalid assembly flag output", error);
	}
	else if ((op.flags & AO_EXIT_TAG) != 0)
	{
		if (op.flags != (AO_WRITE | AO_EXIT_TAG) || op.type.kind() != TypeKind::I32 || op.output == lowir::kNoId ||
			op.alternative_count != 0 || op.input_storage || op.condition != 0)
			return invalid("invalid assembly exit-tag output", error);
	}
	else if (op.alternative_count == 0)
		return invalid("assembly operand has no alternatives", error);
	if (op.alternative_count != 0)
	{
		if (columns != 0 && columns != op.alternative_count)
			return invalid("assembly alternative columns disagree", error);
		columns = op.alternative_count;
	}
	if ((op.flags & AO_COMMUTATIVE) != 0 &&
		(index + 1 == site.operand_count || (op.flags & (AO_READ | AO_WRITE)) != AO_READ ||
			(module.operands[site.first_operand + index + 1].flags & (AO_READ | AO_WRITE | AO_COMMUTATIVE)) != AO_READ))
		return invalid("invalid assembly commutative pair", error);
	const Type ptr = Type::scalar(TypeKind::Ptr);
	if (!argument(arguments, op.input, op.input_storage ? ptr : op.type, error) ||
		!argument(arguments, op.output, ptr, error))
		return false;
	for (uint32_t i = 0; i < op.alternative_count; ++i)
	{
		const Alternative& alt = module.alternatives[op.first_alternative + i];
		if ((alt.locations & AL_LABEL) != 0 &&
			(alt.locations != AL_LABEL || op.flags != AO_READ || op.type.kind() != TypeKind::I32 || op.input_storage ||
				alt.matching != asm_model::kNoId || alt.early_write))
			return invalid("invalid assembly label operand", error);
		if (alt.matching != asm_model::kNoId &&
			(alt.matching >= site.operand_count || alt.matching == index ||
				(op.flags & (AO_READ | AO_WRITE)) != AO_READ ||
				(module.operands[site.first_operand + alt.matching].flags & AO_WRITE) == 0))
			return invalid("invalid assembly matching operand", error);
		if (alt.early_write && (op.flags & AO_WRITE) == 0)
			return invalid("early write on an assembly input", error);
		if (op.type.is_void() && alt.locations != AL_LABEL)
			return invalid("void assembly data operand", error);
	}
	return true;
}

bool template_packet(const AssemblyModule& module, const AssemblyTemplate& site, const char*& error)
{
	const unsigned properties = AP_SIDE_EFFECT | AP_DUPLICABLE | AP_STACK | AP_RED_ZONE | AP_UNWIND | AP_NORETURN |
		AP_INLINE_COST_SMALL;
	const unsigned states = AS_CONDITION_FLAGS | AS_X87_STATUS | AS_MXCSR_STATUS | AS_X87_CONTROL | AS_MXCSR_CONTROL |
		AS_DIRECTION_FLAG;
	if (static_cast<unsigned>(site.dialect) > static_cast<unsigned>(Dialect::Intel) ||
		static_cast<unsigned>(site.memory) > static_cast<unsigned>(Memory::ReadWrite) ||
		(site.properties & ~properties) != 0 || ((site.state_read | site.state_write) & ~states) != 0 ||
		!slice(site.first_operand, site.operand_count, module.operands.size()) ||
		!slice(site.first_piece, site.piece_count, module.pieces.size()) ||
		!slice(site.first_clobber, site.clobber_count, module.clobbers.size()))
		return invalid("invalid assembly template boundary", error);
	// At most two invocation arguments per operand: a value/storage input and
	// an output address. Reject oversized packets before allocating scratch.
	if (site.argument_count > uint64_t(site.operand_count) * 2)
		return invalid("assembly argument count exceeds its operands", error);
	std::vector<Argument> arguments(site.argument_count);
	uint32_t columns = 0;
	for (uint32_t i = 0; i < site.operand_count; ++i)
		if (!operand(module, site, i, arguments, columns, error))
			return false;
	uint32_t labels = 0, selectors = 0;
	for (uint32_t i = 0; i < site.operand_count; ++i)
	{
		const AssemblyOperand& op = module.operands[site.first_operand + i];
		if ((site.properties & AP_NORETURN) != 0 && (op.flags & AO_WRITE) != 0)
			return invalid("nonreturning assembly cannot have outputs", error);
		selectors += (op.flags & AO_EXIT_TAG) != 0;
		bool label = false, other = false;
		for (uint32_t a = 0; a < op.alternative_count; ++a)
		{
			const uint8_t locations = module.alternatives[op.first_alternative + a].locations;
			label = label || locations == AL_LABEL;
			other = other || locations != AL_LABEL;
		}
		if (label &&
			(other || (op.flags & AO_COMMUTATIVE) != 0 ||
				(i != 0 && (module.operands[site.first_operand + i - 1].flags & AO_COMMUTATIVE) != 0)))
			return invalid("assembly labels cannot mix locations or commutative pairs", error);
		labels += label;
	}
	if ((labels != 0 && selectors != 1) || (labels == 0 && selectors != 0))
		return invalid("assembly labels require exactly one exit-tag output", error);
	for (const Argument& arg : arguments)
		if (!arg.present)
			return invalid("unreferenced assembly invocation argument", error);
	for (uint32_t i = 0; i < site.piece_count; ++i)
	{
		const Piece& piece = module.pieces[site.first_piece + i];
		if (static_cast<unsigned>(piece.kind) > static_cast<unsigned>(PieceKind::Unique) ||
			static_cast<unsigned>(piece.format) > static_cast<unsigned>(Format::Suffix))
			return invalid("invalid assembly template piece", error);
		if (piece.kind == PieceKind::Text && !slice(piece.first, piece.count, module.text.size()))
			return invalid("assembly text piece is out of bounds", error);
		if (piece.kind == PieceKind::Operand &&
			(piece.operand >= site.operand_count ||
				(module.operands[site.first_operand + piece.operand].flags & (AO_FLAG | AO_EXIT_TAG)) != 0))
			return invalid("invalid assembly operand reference", error);
		if (piece.kind == PieceKind::Operand)
		{
			const AssemblyOperand& op = module.operands[site.first_operand + piece.operand];
			const bool label = op.alternative_count != 0 &&
				module.alternatives[op.first_alternative].locations == AL_LABEL;
			if ((piece.format == Format::Label && !label) ||
				(label &&
					((piece.format != Format::Default && piece.format != Format::Label) || piece.bits != 0 ||
						piece.offset != 0)))
				return invalid("invalid assembly label formatting", error);
		}
	}
	return true;
}

} // namespace

bool validate_assembly_module(const AssemblyModule& module, const char*& error)
{
	error = nullptr;
	if (!tables(module, error))
		return false;
	for (const AssemblyTemplate& site : module.templates)
		if (!template_packet(module, site, error))
			return false;
	return true;
}

} // namespace lowir
