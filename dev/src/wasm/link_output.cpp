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
		contents.unsigned_leb(signature.results.size());
		for (ValueType type : signature.results)
			contents.byte(static_cast<uint8_t>(type));
	}
	output.section(1, contents);
}

} // namespace

void Linker::tag_imports(Bytes& output) const
{
	for (uint32_t i = 0; i < imported_tags_; ++i)
	{
		const ObjectRef ref = ordered_tags_[i];
		const ObjectTag& tag = objects_[ref.first].tags[ref.second];
		output.string(tag.module);
		output.string(tag.name);
		output.byte(4);
		output.byte(0);
		output.unsigned_leb(type_indices_[ref.first][tag.type]);
	}
}

void Linker::tag_section(Bytes& output) const
{
	if (ordered_tags_.size() == imported_tags_)
		return;
	Bytes tags;
	tags.unsigned_leb(ordered_tags_.size() - imported_tags_);
	for (size_t i = imported_tags_; i < ordered_tags_.size(); ++i)
	{
		const ObjectRef ref = ordered_tags_[i];
		tags.byte(0);
		tags.unsigned_leb(type_indices_[ref.first][objects_[ref.first].tags[ref.second].type]);
	}
	output.section(13, tags);
}

void Linker::emit(Bytes& output)
{
	uint32_t init_type = 0;
	if (ctor_index_ != kAbsent)
	{
		init_type = static_cast<uint32_t>(types_.size());
		types_.push_back(Signature{ {}, {} });
	}
	output.fixed(0x6d736100, 4);
	output.fixed(1, 4);
	type_section(types_, output);
	if (imported_ != 0 || imported_tags_ != 0)
	{
		Bytes imports;
		imports.unsigned_leb(imported_ + imported_tags_);
		for (uint32_t i = 0; i < imported_; ++i)
		{
			const ObjectRef ref = ordered_functions_[i];
			const ObjectFunction& function = objects_[ref.first].functions[ref.second];
			imports.string(function.module);
			imports.string(function.name);
			imports.byte(0);
			imports.unsigned_leb(type_indices_[ref.first][function.type]);
		}
		tag_imports(imports);
		output.section(2, imports);
	}
	Bytes functions;
	functions.unsigned_leb(ordered_functions_.size() - imported_ + (ctor_index_ != kAbsent));
	for (uint32_t i = imported_; i < ordered_functions_.size(); ++i)
	{
		const ObjectRef ref = ordered_functions_[i];
		functions.unsigned_leb(type_indices_[ref.first][objects_[ref.first].functions[ref.second].type]);
	}
	if (ctor_index_ != kAbsent)
		functions.unsigned_leb(init_type);
	output.section(3, functions);
	Bytes table;
	table.unsigned_leb(1);
	table.byte(0x70);
	table.byte(0);
	table.unsigned_leb(ordered_functions_.size() + 1 + (ctor_index_ != kAbsent));
	output.section(4, table);
	Bytes memory;
	memory.unsigned_leb(1);
	memory.byte(0);
	memory.unsigned_leb((stack_top_ + 65535) / 65536);
	output.section(5, memory);
	tag_section(output);
	Bytes globals;
	globals.unsigned_leb(3 + address_globals_.size());
	for (uint32_t index = 0; index < 3; ++index)
	{
		globals.byte(0x7f);
		globals.byte(1);
		globals.byte(0x41);
		globals.signed_leb(index == 0 ? stack_top_ : 0);
		globals.byte(0x0b);
	}
	std::vector<ObjectRef> addresses(address_globals_.size());
	for (const auto& address : address_globals_)
		addresses[address.second - 3] = address.first;
	for (ObjectRef address : addresses)
	{
		globals.byte(0x7f);
		globals.byte(1);
		globals.byte(0x41);
		globals.signed_leb(static_cast<int32_t>(symbol_value(address)));
		globals.byte(0x0b);
	}
	output.section(6, globals);
	std::map<std::string, std::pair<uint8_t, uint32_t>> exported;
	const auto add_export = [&](const std::string& name, ObjectRef ref) {
		const uint8_t kind = symbol(ref).kind;
		if ((kind != 0 && kind != 4) || name == "memory")
			throw WasmError("Wasm export is not a function/tag or conflicts with memory: " + name);
		const auto entry = std::make_pair(kind, kind == 0 ? function_index(ref) : symbol_value(ref));
		const auto inserted = exported.emplace(name, entry);
		if (!inserted.second && inserted.first->second != entry)
			throw WasmError("conflicting Wasm export: " + name);
	};
	for (const std::string& name : options_.exports)
		add_export(name, definitions_.at(name));
	if (options_.target == target_model::Target::Wasi && !options_.no_startfiles)
		add_export("_start", definitions_.at("_start"));
	for (const auto& entry : definitions_)
		if ((symbol(entry.second).flags & 0x20) != 0)
			add_export(entry.first, entry.second);
	for (uint32_t index = 0; index < objects_.size(); ++index)
		if (active_[index])
			for (const ObjectExport& entry : objects_[index].exports)
				add_export(entry.name, indexed_symbol(index, entry.kind, entry.index));
	Bytes exports;
	exports.unsigned_leb(exported.size() + 1);
	exports.string("memory");
	exports.byte(2);
	exports.unsigned_leb(0);
	for (const auto& entry : exported)
	{
		exports.string(entry.first);
		exports.byte(entry.second.first);
		exports.unsigned_leb(entry.second.second);
	}
	output.section(7, exports);
	if (ctor_index_ != kAbsent && options_.target == target_model::Target::Wasm32)
	{
		Bytes start;
		start.unsigned_leb(ctor_index_);
		output.section(8, start);
	}
	if (!ordered_functions_.empty() || ctor_index_ != kAbsent)
	{
		Bytes elements;
		elements.unsigned_leb(1);
		elements.byte(0);
		elements.byte(0x41);
		elements.signed_leb(1);
		elements.byte(0x0b);
		elements.unsigned_leb(ordered_functions_.size() + (ctor_index_ != kAbsent));
		for (uint32_t i = 0; i < ordered_functions_.size(); ++i)
			elements.unsigned_leb(i);
		if (ctor_index_ != kAbsent)
			elements.unsigned_leb(ctor_index_);
		output.section(9, elements);
	}
	Bytes code;
	code.unsigned_leb(ordered_functions_.size() - imported_ + (ctor_index_ != kAbsent));
	for (uint32_t i = imported_; i < ordered_functions_.size(); ++i)
	{
		const ObjectRef ref = ordered_functions_[i];
		const Bytes body = relocated(ref.first, objects_[ref.first].functions[ref.second].code);
		code.unsigned_leb(body.data.size());
		code.append(body);
	}
	if (ctor_index_ != kAbsent)
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
