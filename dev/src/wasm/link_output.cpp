#include "wasm/linker.h"

#include <set>

namespace wasm {

namespace {

void type_section(const std::vector<Signature>& types, Bytes& output)
{
	Bytes contents;
	contents.unsigned_leb(types.size());
	for (const auto& signature : types)
	{
		contents.byte(0x60);
		contents.unsigned_leb(signature.parameters.size());
		for (ValueType type : signature.parameters)
			contents.byte(static_cast<uint8_t>(type));
		contents.unsigned_leb(signature.result == ValueType::Void ? 0 : 1);
		if (signature.result != ValueType::Void)
			contents.byte(static_cast<uint8_t>(signature.result));
	}
	output.section(1, contents);
}

} // namespace

void Linker::emit(Bytes& output)
{
	uint32_t init_type = 0;
	if (!initializers_.empty())
	{
		init_type = static_cast<uint32_t>(types_.size());
		types_.push_back(Signature{ {}, ValueType::Void });
	}
	output.fixed(0x6d736100, 4);
	output.fixed(1, 4);
	type_section(types_, output);
	if (imported_ != 0)
	{
		Bytes imports;
		imports.unsigned_leb(imported_);
		for (uint32_t i = 0; i < imported_; ++i)
		{
			const ObjectRef ref = ordered_functions_[i];
			const ObjectFunction& function = objects_[ref.first].functions[ref.second];
			imports.string(function.module);
			imports.string(function.name);
			imports.byte(0);
			imports.unsigned_leb(type_indices_[ref.first][function.type]);
		}
		output.section(2, imports);
	}
	Bytes functions;
	functions.unsigned_leb(ordered_functions_.size() - imported_ + !initializers_.empty());
	for (uint32_t i = imported_; i < ordered_functions_.size(); ++i)
	{
		const ObjectRef ref = ordered_functions_[i];
		functions.unsigned_leb(type_indices_[ref.first][objects_[ref.first].functions[ref.second].type]);
	}
	if (!initializers_.empty())
		functions.unsigned_leb(init_type);
	output.section(3, functions);
	Bytes table;
	table.unsigned_leb(1);
	table.byte(0x70);
	table.byte(0);
	table.unsigned_leb(ordered_functions_.size() + 1);
	output.section(4, table);
	Bytes memory;
	memory.unsigned_leb(1);
	memory.byte(0);
	memory.unsigned_leb((stack_top_ + 65535) / 65536);
	output.section(5, memory);
	Bytes globals;
	globals.unsigned_leb(3);
	for (uint32_t index = 0; index < 3; ++index)
	{
		globals.byte(0x7f);
		globals.byte(1);
		globals.byte(0x41);
		globals.signed_leb(index == 0 ? stack_top_ : 0);
		globals.byte(0x0b);
	}
	output.section(6, globals);
	std::set<std::string> exported(options_.exports.begin(), options_.exports.end());
	for (const auto& entry : definitions_)
		if ((symbol(entry.second).flags & 0x20) != 0)
			exported.insert(entry.first);
	Bytes exports;
	exports.unsigned_leb(exported.size() + 1);
	exports.string("memory");
	exports.byte(2);
	exports.unsigned_leb(0);
	for (const std::string& name : exported)
	{
		const ObjectRef ref = definitions_.at(name);
		if (symbol(ref).kind != 0 || name == "memory")
			throw WasmError("Wasm export is not a function or conflicts with memory: " + name);
		exports.string(name);
		exports.byte(0);
		exports.unsigned_leb(function_index(ref));
	}
	output.section(7, exports);
	if (!initializers_.empty())
	{
		Bytes start;
		start.unsigned_leb(ordered_functions_.size());
		output.section(8, start);
	}
	if (!ordered_functions_.empty())
	{
		Bytes elements;
		elements.unsigned_leb(1);
		elements.byte(0);
		elements.byte(0x41);
		elements.signed_leb(1);
		elements.byte(0x0b);
		elements.unsigned_leb(ordered_functions_.size());
		for (uint32_t i = 0; i < ordered_functions_.size(); ++i)
			elements.unsigned_leb(i);
		output.section(9, elements);
	}
	Bytes code;
	code.unsigned_leb(ordered_functions_.size() - imported_ + !initializers_.empty());
	for (uint32_t i = imported_; i < ordered_functions_.size(); ++i)
	{
		const ObjectRef ref = ordered_functions_[i];
		const Bytes body = relocated(ref.first, objects_[ref.first].functions[ref.second].code);
		code.unsigned_leb(body.data.size());
		code.append(body);
	}
	if (!initializers_.empty())
	{
		Bytes body;
		body.byte(0);
		for (const auto& initializer : initializers_)
		{
			body.byte(0x10);
			body.unsigned_leb(function_index(initializer.second));
		}
		body.byte(0x0b);
		code.unsigned_leb(body.data.size());
		code.append(body);
	}
	output.section(10, code);
	Bytes data;
	data.unsigned_leb(ordered_data_.size());
	for (ObjectRef ref : ordered_data_)
	{
		const Bytes contents = relocated(ref.first, objects_[ref.first].data[ref.second].contents);
		data.byte(0);
		data.byte(0x41);
		data.signed_leb(data_.at(ref));
		data.byte(0x0b);
		data.unsigned_leb(contents.data.size());
		data.append(contents);
	}
	output.section(11, data);
}

} // namespace wasm
