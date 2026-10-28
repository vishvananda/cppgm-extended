#include "native/object/dwarf_types.h"

#include "native/object/dwarf_format.h"
#include "support/flat_map.h"

namespace native {
namespace {

enum AttributeMask : uint32_t
{
	MName = 1,
	MType = 2,
	MBytes = 4,
	MEncoding = 8,
	MOffset = 16,
	MBits = 32,
	MBitOffset = 64,
	MConstant = 128,
	MDeclaration = 256,
	MArtificial = 512,
	MContaining = 1024,
	MVirtuality = 2048,
	MEnumClass = 4096,
	MExternal = 8192,
	MConstExpr = 16384
};

uint64_t specification(const lowir::DebugType& type)
{
	using lowir::DebugKind;
	uint32_t tag = dwarf::UnspecifiedType;
	bool children = false;
	uint32_t mask = type.name != 0 ? MName : 0;
	if (type.type != 0)
		mask |= MType;
	switch (type.kind)
	{
	case DebugKind::Base:
		tag = dwarf::BaseType;
		mask |= MBytes | MEncoding;
		break;
	case DebugKind::Pointer:
		tag = dwarf::PointerType;
		mask |= MBytes;
		break;
	case DebugKind::Reference:
		tag = dwarf::ReferenceType;
		mask |= MBytes;
		break;
	case DebugKind::RvalueReference:
		tag = dwarf::RvalueReferenceType;
		mask |= MBytes;
		break;
	case DebugKind::Const:
		tag = dwarf::ConstType;
		break;
	case DebugKind::Volatile:
		tag = dwarf::VolatileType;
		break;
	case DebugKind::Array:
		tag = dwarf::ArrayType;
		children = true;
		break;
	case DebugKind::Structure:
		tag = dwarf::StructureType;
		children = true;
		mask |= MBytes;
		break;
	case DebugKind::Union:
		tag = dwarf::UnionType;
		children = true;
		mask |= MBytes;
		break;
	case DebugKind::Enumeration:
		tag = dwarf::EnumerationType;
		children = true;
		mask |= MBytes;
		break;
	case DebugKind::Enumerator:
		tag = dwarf::Enumerator;
		mask |= MConstant;
		break;
	case DebugKind::Member:
		tag = dwarf::Member;
		mask |= (type.flags & lowir::DF_BIT_FIELD) != 0 ? MBits | MBitOffset : MOffset;
		break;
	case DebugKind::Inheritance:
		tag = dwarf::Inheritance;
		mask |= MOffset;
		break;
	case DebugKind::Namespace:
		tag = dwarf::Namespace;
		children = true;
		break;
	case DebugKind::Alias:
		tag = dwarf::Typedef;
		break;
	case DebugKind::Subroutine:
		tag = dwarf::SubroutineType;
		children = true;
		break;
	case DebugKind::Parameter:
		tag = dwarf::FormalParameter;
		break;
	case DebugKind::Constant:
		tag = dwarf::Member;
		mask |= MConstant | MDeclaration | MExternal | MConstExpr;
		break;
	case DebugKind::Method:
		tag = dwarf::Subprogram;
		children = true;
		break;
	default:
		break;
	}
	if ((type.flags & lowir::DF_DECLARATION) != 0)
		mask |= MDeclaration;
	if ((type.flags & lowir::DF_ARTIFICIAL) != 0)
		mask |= MArtificial;
	if ((type.flags & lowir::DF_POLYMORPHIC) != 0)
		mask |= MContaining;
	if ((type.flags & lowir::DF_VIRTUAL_METHOD) != 0)
		mask |= MVirtuality;
	if ((type.flags & lowir::DF_SCOPED_ENUM) != 0)
		mask |= MEnumClass;
	return (uint64_t(mask) << 16) | (children ? 0x100 : 0) | tag;
}

struct SpecHash
{
	size_t operator()(uint64_t key) const { return key * 0x9e3779b97f4a7c15ull; }
};
struct Attribute
{
	uint32_t mask, name, form;
};
const Attribute kAttributes[] = { { MName, dwarf::Name, dwarf::StringPointer },
	{ MType, dwarf::TypeReference, dwarf::Reference4 }, { MBytes, dwarf::ByteSize, dwarf::Unsigned },
	{ MEncoding, dwarf::Encoding, dwarf::Data1 }, { MOffset, dwarf::DataMemberLocation, dwarf::Unsigned },
	{ MBits, dwarf::BitSize, dwarf::Unsigned }, { MBitOffset, dwarf::DataBitOffset, dwarf::Unsigned },
	{ MConstant, dwarf::ConstantValue, dwarf::Data8 }, { MDeclaration, dwarf::Declaration, dwarf::FlagPresent },
	{ MArtificial, dwarf::Artificial, dwarf::FlagPresent }, { MContaining, dwarf::ContainingType, dwarf::Reference4 },
	{ MVirtuality, dwarf::Virtuality, dwarf::Data1 }, { MEnumClass, dwarf::EnumClass, dwarf::FlagPresent },
	{ MExternal, dwarf::External, dwarf::FlagPresent }, { MConstExpr, dwarf::ConstExpr, dwarf::FlagPresent } };

} // namespace

DwarfTypes::DwarfTypes(object::ObjectFile& object, const lowir::Program& program, uint32_t information,
	uint32_t strings, uint32_t strings_symbol)
	: object_(object), program_(program), debug_(*program.debug), information_(information), strings_(strings),
	  strings_symbol_(strings_symbol)
{
	const uint32_t count = static_cast<uint32_t>(debug_.types.size());
	codes_.resize(count);
	first_child_.assign(count, lowir::kNoId);
	next_child_.assign(count, lowir::kNoId);
	offsets_.resize(count);
	FlatMap<uint64_t, uint32_t, SpecHash> known;
	for (uint32_t id = 1; id < count; ++id)
	{
		const uint64_t spec = specification(debug_.types[id]);
		const uint32_t* found = known.find(spec);
		if (found != nullptr)
			codes_[id] = *found;
		else
		{
			codes_[id] = 10 + static_cast<uint32_t>(specifications_.size());
			known.insert(spec, codes_[id]);
			specifications_.push_back(spec);
		}
	}
	for (uint32_t id = count; --id != 0;)
	{
		const uint32_t parent = debug_.types[id].parent;
		next_child_[id] = first_child_[parent];
		first_child_[parent] = id;
	}
}

void DwarfTypes::abbreviations(DwarfBytes& out)
{
	for (uint32_t code = 3; code <= 6; ++code)
	{
		out.uleb(code);
		out.uleb(dwarf::Variable);
		out.fixed(0, 1);
		out.uleb(dwarf::Name);
		out.uleb(dwarf::StringPointer);
		if (code <= 4)
		{
			out.uleb(dwarf::TypeReference);
			out.uleb(dwarf::Reference4);
		}
		if ((code & 1) != 0)
		{
			out.uleb(dwarf::Location);
			out.uleb(dwarf::ExpressionLocation);
		}
		out.uleb(0);
		out.uleb(0);
	}
	out.uleb(7);
	out.uleb(dwarf::SubrangeType);
	out.fixed(0, 1);
	out.uleb(dwarf::Count);
	out.uleb(dwarf::Unsigned);
	out.uleb(0);
	out.uleb(0);
	for (uint32_t i = 0; i < specifications_.size(); ++i)
	{
		const uint64_t spec = specifications_[i];
		out.uleb(10 + i);
		out.uleb(spec & 0xff);
		out.fixed((spec & 0x100) != 0, 1);
		for (const Attribute& attribute : kAttributes)
			if (((spec >> 16) & attribute.mask) != 0)
			{
				out.uleb(attribute.name);
				out.uleb(attribute.form);
			}
		out.uleb(0);
		out.uleb(0);
	}
}

void DwarfTypes::name(DwarfBytes& out, lowir::NameId id)
{
	const uint32_t offset = DwarfBytes(object_.sections[strings_].bytes).text(id != 0 ? program_.spelling(id) : "");
	object_.sections[information_].relocations.push_back(
		object::Relocation{ out.size(), object::R_X86_64_32, strings_symbol_, offset });
	out.fixed(0, 4);
}

void DwarfTypes::reference(DwarfBytes& out, uint32_t id)
{
	references_.push_back(Reference{ out.size(), id });
	out.fixed(0, 4);
}

void DwarfTypes::attributes(DwarfBytes& out, const lowir::DebugType& type, uint32_t id, uint32_t mask)
{
	if ((mask & MName) != 0)
		name(out, type.name);
	if ((mask & MType) != 0)
		reference(out, type.type);
	if ((mask & MBytes) != 0)
		out.uleb(type.bytes);
	if ((mask & MEncoding) != 0)
	{
		const uint8_t encodings[] = { 0, 2, 5, 6, 7, 8, 4, 3, 1 };
		out.fixed(encodings[static_cast<unsigned>(type.encoding)], 1);
	}
	if ((mask & MOffset) != 0)
		out.uleb(type.bit_offset / 8);
	if ((mask & MBits) != 0)
		out.uleb(type.count);
	if ((mask & MBitOffset) != 0)
		out.uleb(type.bit_offset);
	if ((mask & MConstant) != 0)
		out.fixed(type.constant, 8);
	if ((mask & MContaining) != 0)
		reference(out, id);
	if ((mask & MVirtuality) != 0)
		out.fixed(1, 1);
}

void DwarfTypes::entry(DwarfBytes& out, uint32_t id)
{
	offsets_[id] = out.size();
	const lowir::DebugType& type = debug_.types[id];
	out.uleb(codes_[id]);
	const uint64_t spec = specifications_[codes_[id] - 10];
	attributes(out, type, id, static_cast<uint32_t>(spec >> 16));
	if ((spec & 0x100) != 0)
	{
		if (type.kind == lowir::DebugKind::Array)
		{
			out.uleb(7);
			out.uleb(type.count);
		}
		children(out, id);
		out.uleb(0);
	}
}

void DwarfTypes::children(DwarfBytes& out, uint32_t parent)
{
	for (uint32_t id = first_child_[parent]; id != lowir::kNoId; id = next_child_[id])
		entry(out, id);
}

void DwarfTypes::entries(DwarfBytes& out)
{
	children(out, 0);
}

void DwarfTypes::finish(DwarfBytes& out)
{
	for (const Reference& reference : references_)
		out.patch32(reference.offset, offsets_[reference.type]);
}

void DwarfTypes::locals(DwarfBytes& out, lowir::SymbolId function, const std::vector<DwarfLines::Variable>& variables,
	uint32_t first, uint32_t count)
{
	if (function >= debug_.functions.size())
		return;
	const std::vector<lowir::DebugLocal>& locals = debug_.functions[function].locals;
	// Dense slot ids are source identities. Do not match a variable by its
	// display name: shadowing declarations may have identical spellings.
	uint32_t maximum = 0;
	for (const lowir::DebugLocal& local : locals)
		maximum = std::max(maximum, local.slot + 1);
	std::vector<const DwarfLines::Variable*> locations(maximum, nullptr);
	for (uint32_t i = 0; i < count; ++i)
	{
		const DwarfLines::Variable& variable = variables[first + i];
		if (variable.slot < maximum)
			locations[variable.slot] = &variable;
	}
	for (const lowir::DebugLocal& local : locals)
	{
		const DwarfLines::Variable* variable = locations[local.slot];
		out.uleb((local.type != 0 ? 3 : 5) + (variable != nullptr ? 0 : 1));
		name(out, local.name);
		if (local.type != 0)
			reference(out, local.type);
		if (variable != nullptr)
		{
			std::vector<uint8_t> expression;
			DwarfBytes bytes(expression);
			bytes.fixed(variable->aligned ? 0x7c : 0x91, 1); // DW_OP_breg12 or DW_OP_fbreg
			bytes.sleb(variable->offset);
			out.uleb(expression.size());
			for (uint8_t byte : expression)
				out.fixed(byte, 1);
		}
	}
}

} // namespace native
