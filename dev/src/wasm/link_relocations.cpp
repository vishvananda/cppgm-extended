#include "wasm/linker.h"

namespace wasm {

uint32_t Linker::function_index(ObjectRef ref) const
{
	ref = selected(ref);
	const auto found = functions_.find({ ref.first, symbol(ref).index });
	if (symbol(ref).kind != 0 || found == functions_.end())
		throw WasmError("unresolved Wasm function relocation: " + symbol(ref).name);
	return found->second;
}

uint32_t Linker::symbol_value(ObjectRef ref) const
{
	ref = selected(ref);
	const auto& source = symbol(ref);
	if (source.kind == 2 || source.kind == 5)
		return 0;
	if (source.undefined() && !host_import(ref))
	{
		if (source.name == "__stack_low" || source.name == "__data_end")
			return stack_floor_;
		if (source.name == "__stack_high" || source.name == "__heap_base")
			return stack_top_;
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

Bytes Linker::relocated(uint32_t object, const ObjectChunk& chunk) const
{
	Bytes result = chunk.bytes;
	for (const Relocation& relocation : chunk.relocations)
	{
		uint32_t value;
		if (relocation.kind == RelocationKind::TypeIndex)
			value = type_indices_[object][relocation.index];
		else if (relocation.kind == RelocationKind::FunctionIndex)
			value = function_index({ object, relocation.index });
		else
			value = symbol_value({ object, relocation.index });
		const int64_t sum = int64_t(value) + relocation.addend;
		if (sum < INT32_MIN || sum > UINT32_MAX)
			throw WasmError("Wasm relocation value overflows 32 bits");
		value = static_cast<uint32_t>(sum);
		if (relocation.kind == RelocationKind::TableIndexI32 || relocation.kind == RelocationKind::MemoryAddressI32)
		{
			for (unsigned i = 0; i < 4; ++i)
				result.data[relocation.offset + i] = static_cast<uint8_t>(value >> (i * 8));
		}
		else
		{
			const bool signed_value = relocation.kind == RelocationKind::TableIndex ||
				relocation.kind == RelocationKind::MemoryAddressSigned;
			const uint64_t extended = signed_value ? uint64_t(int64_t(int32_t(value))) : uint64_t(value);
			for (unsigned i = 0; i < 5; ++i)
				result.data[relocation.offset + i] = ((extended >> (7 * i)) & 0x7f) | (i == 4 ? 0 : 0x80);
		}
	}
	return result;
}

} // namespace wasm
