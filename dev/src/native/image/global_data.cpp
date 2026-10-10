#include "native/image/global_data.h"

#include <cstring>

#include "lowir/literals.h"
#include "native/select/call_abi.h"

namespace native {

using lowir::Type;
using lowir::TypeKind;

namespace {

uint32_t item_align(Type type)
{
	const uint32_t align = value_align(type);
	return align > 16 ? 16 : align;
}

// The bytes of a literal stored as `type`.
void write_constant(uint8_t* out, Type type, const lowir::Constant& c)
{
	lowir::Literal bits = c.bits;
	if (type.is_float())
	{
		if (c.kind != lowir::ValueKind::Float || c.float_kind != type.kind())
		{
			const long double value = c.kind == lowir::ValueKind::Float
				? lowir::float_value(c.float_kind, c.bits)
				: static_cast<long double>(static_cast<int64_t>(c.bits.low));
			bits = lowir::float_literal(type.kind(), value);
		}
		const unsigned low_bytes = type.kind() == TypeKind::F32 ? 4 : 8;
		for (unsigned k = 0; k < low_bytes; ++k)
			out[k] = static_cast<uint8_t>(bits.low >> (8 * k));
		if (type.kind() == TypeKind::F80)
		{
			out[8] = static_cast<uint8_t>(bits.high);
			out[9] = static_cast<uint8_t>(bits.high >> 8);
		}
		return;
	}
	const uint32_t bytes = value_bytes(type);
	for (uint32_t k = 0; k < bytes && k < 8; ++k)
		out[k] = static_cast<uint8_t>(bits.low >> (8 * k));
	for (uint32_t k = 8; k < bytes; ++k)
		out[k] = static_cast<uint8_t>(bits.high >> (8 * (k - 8)));
}

Type item_type(const lowir::DataItem& item)
{
	return item.kind == lowir::DataItemKind::Address ? Type::scalar(TypeKind::Ptr) : item.type;
}

// Whether a constant stored as `type` has only zero bytes.
bool constant_is_zero(Type type, const lowir::Constant& c)
{
	uint8_t bytes[16] = { 0 };
	write_constant(bytes, type, c);
	for (const uint8_t byte : bytes)
	{
		if (byte != 0)
			return false;
	}
	return true;
}

} // namespace

// Typed items at their natural alignment, `zero` items as raw bytes; a
// stated storage type's alignment and span at least (lowir.md "Structured
// Global Data Initializers"), else 16 when every item is `zero`.
uint64_t global_size(const lowir::Program& program, const lowir::Global& g, uint32_t& align)
{
	if (g.init != lowir::GlobalInit::Data)
	{
		const Type type = g.typed ? g.type : Type::scalar(TypeKind::Ptr);
		align = item_align(type);
		return value_bytes(type) == 0 ? 8 : value_bytes(type);
	}
	uint64_t size = 0;
	align = 1;
	bool typed = false;
	for (uint32_t i = 0; i < g.item_count; ++i)
	{
		const lowir::DataItem& item = program.data_items[g.first_item + i];
		if (item.kind == lowir::DataItemKind::Zero)
		{
			size += static_cast<uint64_t>(item.amount);
			continue;
		}
		const uint32_t a = item_align(item_type(item));
		typed = true;
		if (a > align)
			align = a;
		size = ((size + a - 1) & ~static_cast<uint64_t>(a - 1)) + value_bytes(item_type(item));
	}
	if (g.typed)
	{
		const uint32_t stated = lowir::data_alignment(g.type);
		if (stated > align)
			align = stated;
		if (g.type.bytes > size)
			size = g.type.bytes;
	}
	else if (!typed)
		align = 16;
	return size == 0 ? 1 : size;
}

void write_global(const lowir::Program& program, const lowir::Global& g, std::vector<uint8_t>& out, uint64_t base,
	std::vector<GlobalAddress>& addresses)
{
	if (g.init != lowir::GlobalInit::Data)
	{
		const Type type = g.typed ? g.type : Type::scalar(TypeKind::Ptr);
		if (g.init == lowir::GlobalInit::Constant)
			write_constant(out.data() + base, type, g.constant);
		else if (g.init == lowir::GlobalInit::Address)
			addresses.push_back(GlobalAddress{ 0, g.address, g.addend });
		return;
	}
	uint64_t at = 0;
	for (uint32_t i = 0; i < g.item_count; ++i)
	{
		const lowir::DataItem& item = program.data_items[g.first_item + i];
		if (item.kind == lowir::DataItemKind::Zero)
		{
			at += static_cast<uint64_t>(item.amount);
			continue;
		}
		const Type type = item_type(item);
		const uint32_t a = item_align(type);
		at = (at + a - 1) & ~static_cast<uint64_t>(a - 1);
		if (item.kind == lowir::DataItemKind::Address)
			addresses.push_back(GlobalAddress{ at, item.address, item.amount });
		else
			write_constant(out.data() + base + at, type, item.constant);
		at += value_bytes(type);
	}
}

bool global_is_zero(const lowir::Program& program, const lowir::Global& g)
{
	switch (g.init)
	{
	case lowir::GlobalInit::Zero:
		return true;
	case lowir::GlobalInit::Constant:
		return constant_is_zero(g.typed ? g.type : Type::scalar(TypeKind::Ptr), g.constant);
	case lowir::GlobalInit::Data:
		for (uint32_t i = 0; i < g.item_count; ++i)
		{
			const lowir::DataItem& item = program.data_items[g.first_item + i];
			if (item.kind == lowir::DataItemKind::Address ||
				(item.kind == lowir::DataItemKind::Scalar && !constant_is_zero(item.type, item.constant)))
				return false;
		}
		return true;
	default:
		return false;
	}
}

bool global_has_addresses(const lowir::Program& program, const lowir::Global& g)
{
	if (g.init == lowir::GlobalInit::Address)
		return true;
	if (g.init != lowir::GlobalInit::Data)
		return false;
	for (uint32_t i = 0; i < g.item_count; ++i)
		if (program.data_items[g.first_item + i].kind == lowir::DataItemKind::Address)
			return true;
	return false;
}

} // namespace native
