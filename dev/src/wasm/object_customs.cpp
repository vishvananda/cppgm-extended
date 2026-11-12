#include "wasm/object_reader.h"

namespace wasm {

void read_object_customs(
	const std::vector<std::pair<std::string, Bytes>>& customs, const std::vector<uint8_t>& sections, Object& object)
{
	bool linking = false;
	for (const auto& custom : customs)
	{
		Cursor input(custom.second);
		if (custom.first == "linking")
		{
			if (linking)
				throw WasmError("duplicate Wasm linking metadata");
			linking = true;
			read_linking(input, object);
		}
		else if (custom.first == "target_features")
		{
			const uint32_t count = input.u32();
			for (uint32_t i = 0; i < count; ++i)
			{
				const char prefix = input.byte();
				object.features.push_back({ prefix, input.string() });
			}
		}
		else
			continue;
		input.done();
	}
	if (!linking)
		throw WasmError("Wasm input lacks relocatable-object linking metadata");
	for (const auto& custom : customs)
		if (custom.first.compare(0, 6, "reloc.") == 0)
		{
			Cursor input(custom.second);
			const uint32_t target = input.u32();
			if (target >= sections.size())
				throw WasmError("invalid Wasm relocation target section");
			if (sections[target] == 0)
				input.bytes(); // Discarded custom/debug sections need no relocation.
			else
				read_relocations(input, object, sections[target]);
			input.done();
		}
}

} // namespace wasm
