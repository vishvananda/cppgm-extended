#include "lowering/objects/static_initialization.h"
#include "lowering/support/errors.h"

#include <algorithm>
#include <cstdio>

namespace cppgm
{
namespace lowering
{

using namespace semantic;
using namespace lowering::ir;

bool StaticInitializerLowering::AppendConstantElement(TypeId type,
	const ConstexprObjectElement& element, std::vector<Global::DataItem>* items)
{
	if (element.object_value) return AppendConstantObject(element.object, items);
	if (element.address_value)
	{
		if (element.address >= graph_.constant_addresses.size())
			ThrowLoweringInternal("invalid static object address identity");
		const ConstexprAddressValue& address = graph_.constant_addresses[element.address];
		StaticAddressInitializer initializer;
		initializer.kind = address.kind;
		initializer.identity = address.identity;
		initializer.offset = address.offset;
		Global value;
		if (!LowerAddress(initializer, &value)) return false;
		if (value.initializer_kind != Global::ADDRESS_VALUE)
		{
			AppendZero(program_.SizeOf(type), items);
			return true;
		}
		Global::DataItem item;
		item.kind = Global::DataItem::ADDRESS_ITEM;
		item.type = LowPtr();
		item.symbol = value.address_symbol;
		item.offset = value.address_offset;
		items->push_back(item);
		return true;
	}
	Global::DataItem item;
	item.type = types_.LowerStorage(type);
	if (element.scalar.kind == CONSTEXPR_SCALAR_FLOATING)
	{
		char buffer[128];
		const char* suffix = item.type.kind == LOW_F32 ? "F" :
			item.type.kind == LOW_F80 ? "L" : "";
		const int size = std::snprintf(buffer, sizeof(buffer), "%#.21Lg%s",
			element.scalar.floating, suffix);
		if (size <= 0 || static_cast<std::size_t>(size) >= sizeof(buffer) ||
			!DecodeFloatingLiteral(buffer, item.type, &item.floating_low,
				&item.integer_high)) return false;
		item.kind = Global::DataItem::FLOATING_ITEM;
		if (output_.retain_local_names)
			item.floating_spelling = output_.strings.intern(buffer);
	}
	else if (element.scalar.kind == CONSTEXPR_SCALAR_MEMBER_POINTER &&
		program_.types.IsFunction(program_.types.Get(
			program_.types.RemoveTopCv(type)).child))
	{
		const BindingId binding = element.scalar.member_pointer;
		if (binding == kNoBinding)
			AppendZero(program_.SizeOf(type), items);
		else
		{
			if (program_.bindings[binding].virtual_function ||
				!SymbolForBinding(binding, &item.symbol)) return false;
			item.kind = Global::DataItem::ADDRESS_ITEM;
			item.type = LowPtr();
			items->push_back(item);
			item = Global::DataItem();
			item.kind = Global::DataItem::INTEGER_ITEM;
			item.type = LowI64();
			item.integer_value = element.scalar.integral;
			items->push_back(item);
		}
		return true;
	}
	else
	{
		item.kind = Global::DataItem::INTEGER_ITEM;
		item.integer_value = element.scalar.integral;
		item.integer_high = element.scalar.integral_high;
	}
	items->push_back(item);
	return true;
}

bool StaticInitializerLowering::AppendConstantObject(std::uint32_t object,
	std::vector<Global::DataItem>* items)
{
	if (object >= graph_.constant_objects.size())
		ThrowLoweringInternal("invalid static object value identity");
	const ConstexprObjectValue& value = graph_.constant_objects[object];
	const TypeRecord& type = program_.types.Get(value.type);
	if (type.kind == TYPE_ARRAY)
	{
		if (value.element_count != type.bound) return false;
		for (std::size_t i = 0; i < value.element_count; ++i)
			if (!AppendConstantElement(type.child,
				graph_.constant_object_elements[value.first_element + i], items))
				return false;
		return true;
	}
	if (type.kind != TYPE_NAMED ||
		program_.entities[type.entity].polymorphic_class) return false;
	const EntityRecord& entity = program_.entities[type.entity];
	if (entity.direct_base_count > value.element_count) return false;
	const std::size_t member_count = value.element_count - entity.direct_base_count;
	std::vector<std::pair<std::uint64_t, std::uint32_t> > parts;
	for (std::size_t i = 0; i < value.element_count; ++i)
	{
		const std::uint32_t index = value.first_element + i;
		const ConstexprObjectElement& element = graph_.constant_object_elements[index];
		std::uint64_t offset = 0;
		if (i < member_count)
		{
			if (element.member == kNoBinding) return false;
			offset = program_.BindingLayout(program_.bindings[element.member]).member_offset;
		}
		else
		{
			const DirectBaseEdge& base = program_.DirectBase(type.entity, i - member_count);
			if (base.virtual_base || !element.object_value) return false;
			if (program_.entities[base.entity].empty_class) continue;
			offset = base.offset;
		}
		parts.push_back(std::make_pair(offset, index));
	}
	std::sort(parts.begin(), parts.end());
	std::size_t cursor = 0, bit_item = 0;
	std::uint64_t bit_offset = ~std::uint64_t(0);
	for (std::size_t i = 0; i < parts.size(); ++i)
	{
		const ConstexprObjectElement& element = graph_.constant_object_elements[parts[i].second];
		const TypeId part_type = element.member == kNoBinding ?
			graph_.constant_objects[element.object].type : program_.bindings[element.member].type;
		if (element.member != kNoBinding && program_.bindings[element.member].bit_field)
		{
			const BindingLayoutFact& layout = program_.BindingLayout(program_.bindings[element.member]);
			if (layout.bit_width == 0) { bit_offset = ~std::uint64_t(0); continue; }
			if (layout.bit_storage_bits > 64 || element.object_value || element.address_value)
				return false;
			const std::uint64_t mask = layout.bit_width == 64 ? ~std::uint64_t(0) :
				(std::uint64_t(1) << layout.bit_width) - 1;
			const std::uint64_t bits = (static_cast<std::uint64_t>(element.scalar.integral) & mask)
				<< layout.bit_offset;
			if (parts[i].first != bit_offset)
			{
				if (parts[i].first < cursor) return false;
				AppendZero(parts[i].first - cursor, items);
				Global::DataItem item;
				item.kind = Global::DataItem::INTEGER_ITEM;
				item.type = layout.bit_storage_bits == 8 ? LowU8() :
					layout.bit_storage_bits == 16 ? LowU16() :
					layout.bit_storage_bits == 32 ? LowU32() : LowU64();
				bit_item = items->size();
				items->push_back(item);
				bit_offset = parts[i].first;
				cursor = parts[i].first + layout.bit_storage_bits / 8;
			}
			(*items)[bit_item].integer_value |= bits;
			continue;
		}
		bit_offset = ~std::uint64_t(0);
		if (parts[i].first < cursor) return false;
		AppendZero(parts[i].first - cursor, items);
		if (!AppendConstantElement(part_type, element, items)) return false;
		cursor = parts[i].first + program_.SizeOf(part_type);
	}
	const std::size_t size = program_.SizeOf(value.type);
	if (cursor > size) return false;
	AppendZero(size - cursor, items);
	return true;
}

}
}
