#include "wasm/linker.h"

namespace wasm {

bool Linker::synthetic_data(const std::string& name, uint32_t& value) const
{
	if (name == "__stack_low")
		value = 0;
	else if (name == "__stack_high")
		value = stack_top_;
	else if (name == "__data_end" || name == "__heap_base")
		value = heap_base_;
	else if (name == "__wasm_first_page_end")
		value = 65536;
	else if (name == "__global_base" || name == "__dso_handle")
		value = data_base_;
	else if (name == "__heap_end")
		value = (heap_base_ + 65535) & ~uint32_t(65535);
	else
		return false;
	return true;
}

uint32_t Linker::function_index(ObjectRef ref) const
{
	ref = selected(ref);
	if (symbol(ref).undefined() && symbol(ref).name == "__wasm_call_ctors" && ctor_index_ != kAbsent)
		return ctor_index_;
	const auto found = functions_.find({ ref.first, symbol(ref).index });
	if (symbol(ref).kind != 0 || found == functions_.end())
		throw WasmError("unresolved Wasm function relocation: " + symbol(ref).name);
	return found->second;
}

uint32_t Linker::symbol_value(ObjectRef ref) const
{
	ref = selected(ref);
	const auto& source = symbol(ref);
	if (source.kind == 5)
		return 0;
	if (source.kind == 4)
	{
		const auto tag = tags_.find({ ref.first, source.index });
		if (tag == tags_.end())
			throw WasmError("unresolved Wasm tag relocation: " + source.name);
		return tag->second;
	}
	if (source.kind == 2)
	{
		uint32_t runtime = 0;
		if (runtime_global(ref, runtime))
			return static_cast<uint32_t>(imported_globals_.size()) + runtime;
		const auto found = globals_.find({ ref.first, source.index });
		if (found == globals_.end())
			throw WasmError("unresolved Wasm global relocation: " + source.name);
		return found->second;
	}
	if (source.undefined() && !host_import(ref))
	{
		uint32_t value = 0;
		if (source.kind == 1 && synthetic_data(source.name, value))
			return value;
		if (source.weak())
			return 0;
	}
	if (source.kind == 0)
		return function_index(ref) + 1;
	const auto found = data_.find({ ref.first, source.index });
	if (source.kind != 1 || found == data_.end())
		throw WasmError("unresolved Wasm data relocation: " + source.name);
	return found->second + source.offset;
}

void Linker::prepare_address_globals()
{
	const auto collect = [&](uint32_t object, const ObjectChunk& chunk) {
		for (const Relocation& relocation : chunk.relocations)
			if ((relocation.kind == RelocationKind::GlobalIndex || relocation.kind == RelocationKind::GlobalIndexI32) &&
				objects_[object].symbols[relocation.index].kind <= 1)
			{
				const ObjectRef target = selected({ object, relocation.index });
				if (address_globals_.size() >= UINT32_MAX - 3 - imported_globals_.size())
					throw WasmError("too many Wasm address globals");
				address_globals_.emplace(
					target, static_cast<uint32_t>(imported_globals_.size() + 3 + address_globals_.size()));
			}
	};
	for (ObjectRef function : ordered_functions_)
		collect(function.first, objects_[function.first].functions[function.second].code);
	for (ObjectRef data : ordered_data_)
		collect(data.first, objects_[data.first].data[data.second].contents);
}

Bytes Linker::relocated(uint32_t object, const ObjectChunk& chunk, bool constant) const
{
	Bytes result = chunk.bytes;
	for (const Relocation& relocation : chunk.relocations)
	{
		uint32_t value;
		if (relocation.kind == RelocationKind::TypeIndex)
			value = type_indices_[object][relocation.index];
		else if ((relocation.kind == RelocationKind::GlobalIndex ||
					 relocation.kind == RelocationKind::GlobalIndexI32) &&
			objects_[object].symbols[relocation.index].kind <= 1)
			value = constant ? symbol_value({ object, relocation.index })
							 : address_globals_.at(selected({ object, relocation.index }));
		else if (relocation.kind == RelocationKind::FunctionIndex ||
			relocation.kind == RelocationKind::FunctionIndexI32)
			value = function_index({ object, relocation.index });
		else
			value = symbol_value({ object, relocation.index });
		bool constant_address = false;
		if (constant && relocation.kind == RelocationKind::GlobalIndex)
		{
			const ObjectRef target = selected({ object, relocation.index });
			uint32_t runtime = 0;
			if (symbol(target).kind <= 1 || (runtime_global(target, runtime) && runtime != 0))
			{
				if (relocation.offset == 0 || result.data[relocation.offset - 1] != 0x23)
					throw WasmError("Wasm constant global relocation does not target global.get");
				constant_address = true;
				result.data[relocation.offset - 1] = 0x41;
				value = symbol(target).kind <= 1 ? symbol_value(target) : 0;
			}
		}
		const int64_t sum = int64_t(value) + relocation.addend;
		if (sum < INT32_MIN || sum > UINT32_MAX)
			throw WasmError("Wasm relocation value overflows 32 bits");
		value = static_cast<uint32_t>(sum);
		if (relocation.kind == RelocationKind::TableIndexI32 || relocation.kind == RelocationKind::MemoryAddressI32 ||
			relocation.kind == RelocationKind::GlobalIndexI32 || relocation.kind == RelocationKind::FunctionIndexI32)
		{
			for (unsigned i = 0; i < 4; ++i)
				result.data[relocation.offset + i] = static_cast<uint8_t>(value >> (i * 8));
		}
		else
		{
			const bool signed_value = constant_address || relocation.kind == RelocationKind::TableIndex ||
				relocation.kind == RelocationKind::MemoryAddressSigned ||
				relocation.kind == RelocationKind::MemoryAddressRelative ||
				relocation.kind == RelocationKind::TableIndexRelative;
			const uint64_t extended = signed_value ? uint64_t(int64_t(int32_t(value))) : uint64_t(value);
			for (unsigned i = 0; i < 5; ++i)
				result.data[relocation.offset + i] = ((extended >> (7 * i)) & 0x7f) | (i == 4 ? 0 : 0x80);
		}
	}
	return result;
}

} // namespace wasm
