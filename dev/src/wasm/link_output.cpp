#include "wasm/linker.h"

#include <utility>

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
	if (imported_ != 0 || imported_tags_ != 0 || !imported_globals_.empty())
	{
		Bytes imports;
		imports.unsigned_leb(imported_ + imported_tags_ + imported_globals_.size());
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
		global_imports(imports);
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
	global_section(output);
	std::map<std::string, std::pair<uint8_t, uint32_t>> exported;
	const auto add_export = [&](const std::string& name, ObjectRef ref) {
		const uint8_t kind = symbol(ref).kind;
		if ((kind != 0 && kind != 2 && kind != 4) || name == "memory")
			throw WasmError("Wasm export is not a function/tag or conflicts with memory: " + name);
		const auto entry = std::make_pair(
			kind == 2 ? uint8_t(3) : kind, kind == 0 ? function_index(ref) : symbol_value(ref));
		const auto inserted = exported.emplace(name, entry);
		if (!inserted.second && inserted.first->second != entry)
			throw WasmError("conflicting Wasm export: " + name);
	};
	for (const std::string& name : options_.exports)
		add_export(name, definitions_.at(name));
	if (options_.target == target_model::Target::Wasi && !options_.no_startfiles)
		add_export("_start", definitions_.at("_start"));
	std::set<ObjectRef> explicitly_named;
	for (uint32_t index = 0; index < objects_.size(); ++index)
		if (active_[index])
			for (const ObjectExport& entry : objects_[index].exports)
			{
				const ObjectRef ref = indexed_symbol(index, entry.kind == 3 ? 2 : entry.kind, entry.index);
				add_export(entry.name, ref);
				explicitly_named.insert(selected(ref));
			}
	for (uint32_t object = 0; object < objects_.size(); ++object)
		if (active_[object])
			for (uint32_t s = 0; s < objects_[object].symbols.size(); ++s)
			{
				const ObjectRef ref{ object, s };
				if ((symbol(ref).flags & 0x20) != 0 && !explicitly_named.count(selected(ref)))
					add_export(symbol(ref).name, ref);
			}
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
		Bytes body;
		if (weak_functions_.count(ref))
		{
			// An absent weak function has a null table address, but a direct
			// call still needs a valid function index with its declared type.
			body.byte(0);
			body.byte(0x00);
			body.byte(0x0b);
		}
		else
			body = relocated(ref.first, objects_[ref.first].functions[ref.second].code);
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
	data_section(output);
}

void Linker::data_section(Bytes& output) const
{
	std::vector<std::pair<uint32_t, Bytes>> segments;
	for (ObjectRef ref : ordered_data_)
	{
		Bytes contents = relocated(ref.first, objects_[ref.first].data[ref.second].contents);
		// Defined linear memory starts at zero. Preserve the full layout,
		// but omit trailing zero initialization after applying relocations.
		while (!contents.data.empty() && contents.data.back() == 0)
			contents.data.pop_back();
		if (!contents.data.empty())
			segments.push_back({ data_.at(ref), std::move(contents) });
	}
	if (segments.empty())
		return;
	Bytes data;
	data.unsigned_leb(segments.size());
	for (const auto& segment : segments)
	{
		data.byte(0);
		data.byte(0x41);
		data.signed_leb(segment.first);
		data.byte(0x0b);
		data.unsigned_leb(segment.second.data.size());
		data.append(segment.second);
	}
	output.section(11, data);
}

} // namespace wasm
