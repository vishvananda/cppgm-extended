#include "wasm/linker.h"

#include <algorithm>
#include "wasm/object_reader.h"

namespace wasm {

ObjectRef Linker::indexed_symbol(uint32_t object, uint8_t kind, uint32_t value) const
{
	for (uint32_t index = 0; index < objects_[object].symbols.size(); ++index)
		if (objects_[object].symbols[index].kind == kind && objects_[object].symbols[index].index == value)
			return ObjectRef{ object, index };
	throw WasmError("Wasm indexed entry has no linking symbol");
}

ObjectRef Linker::selected(ObjectRef ref) const
{
	const ObjectSymbol& source = symbol(ref);
	if (source.local())
		return ref;
	const auto found = definitions_.find(source.name);
	return found == definitions_.end() ? ref : found->second;
}

bool Linker::host_import(ObjectRef ref) const
{
	const ObjectSymbol& source = symbol(ref);
	if ((source.kind != 0 && source.kind != 2 && source.kind != 4) || !source.undefined())
		return false;
	if (source.kind == 2)
		return objects_[ref.first].global_imports[source.index].module != "env" || (source.flags & 0x40) != 0;
	if (source.kind == 4)
		return objects_[ref.first].tags[source.index].module != "env" || (source.flags & 0x40) != 0;
	const ObjectFunction& function = objects_[ref.first].functions[source.index];
	return function.module != "env" || (source.flags & 0x40) != 0;
}

void Linker::resolve()
{
	active_.resize(objects_.size(), false);
	for (uint32_t i = 0; i < objects_.size(); ++i)
		if (objects_[i].archive_member)
			for (uint32_t j = 0; j < objects_[i].symbols.size(); ++j)
			{
				const ObjectSymbol& candidate = objects_[i].symbols[j];
				if (!candidate.undefined() && !candidate.local() && candidate.kind != 3)
					archive_index_.emplace(candidate.name, ObjectRef{ i, j });
			}
	for (uint32_t i = 0; i < objects_.size(); ++i)
		if (!objects_[i].archive_member)
			activate(i);
}

ObjectRef Linker::lookup(const std::string& name)
{
	if (definitions_.find(name) == definitions_.end())
	{
		const auto member = archive_index_.find(name);
		if (member != archive_index_.end())
			activate(member->second.first);
	}
	const auto found = definitions_.find(name);
	return found == definitions_.end() ? ObjectRef{ kAbsent, kAbsent } : found->second;
}

void Linker::activate(uint32_t i)
{
	if (active_[i])
		return;
	active_[i] = true;
	pending_objects_.push_back(i);
	if (objects_[i].archive_member)
		validate_object_code(objects_[i]);
	const Object& object = objects_[i];
	for (const auto& group : object.groups)
	{
		const bool duplicate = !groups_.insert(group.name).second;
		for (const auto& member : group.members)
		{
			const bool valid = member.first == 0 ? member.second < object.data.size()
				: member.first == 1
				? member.second < object.functions.size() && !object.functions[member.second].imported
				: member.first == 2 ? member.second >= object.global_imports.size() &&
					member.second < object.global_imports.size() + object.global_definitions.size()
				: member.first == 3 ? member.second < object.tags.size() && !object.tags[member.second].imported
									: false;
			if (!valid)
				throw WasmError("invalid or unsupported Wasm COMDAT member");
			if (duplicate)
				(member.first == 0          ? discarded_data_
						: member.first == 1 ? discarded_functions_
						: member.first == 2 ? discarded_globals_
											: discarded_tags_)
					.insert({ i, member.second });
		}
	}
	for (uint32_t j = 0; j < object.symbols.size(); ++j)
	{
		const ObjectSymbol& candidate = object.symbols[j];
		if (candidate.undefined() || candidate.local() || candidate.kind == 3)
			continue;
		if ((candidate.kind == 0 && discarded_functions_.count({ i, candidate.index })) ||
			(candidate.kind == 1 && discarded_data_.count({ i, candidate.index })) ||
			(candidate.kind == 4 && discarded_tags_.count({ i, candidate.index })) ||
			(candidate.kind == 2 && discarded_globals_.count({ i, candidate.index })))
			continue;
		const auto inserted = definitions_.emplace(candidate.name, ObjectRef{ i, j });
		if (!inserted.second)
		{
			const ObjectSymbol& prior = symbol(inserted.first->second);
			if (prior.kind != candidate.kind || (!prior.weak() && !candidate.weak()))
				throw WasmError("duplicate or incompatible Wasm definition: " + candidate.name);
			if (prior.weak() && !candidate.weak())
				inserted.first->second = { i, j };
		}
	}
	for (uint32_t j = 0; j < object.symbols.size(); ++j)
	{
		const auto& source = object.symbols[j];
		if ((source.flags & 0xa0) != 0 ||
			(source.kind == 1 && !source.undefined() && (object.data[source.index].flags & 4) != 0))
			roots_.push_back({ i, j });
	}
	for (const auto& init : object.initializers)
	{
		const ObjectRef ref{ i, init.second };
		if (!discarded_functions_.count({ i, symbol(ref).index }))
		{
			roots_.push_back(ref);
			initializers_.push_back({ init.first, ref });
		}
	}
	for (const ObjectExport& entry : object.exports)
		roots_.push_back(indexed_symbol(i, entry.kind == 3 ? 2 : entry.kind, entry.index));
}

void Linker::validate_symbols() const
{
	for (uint32_t i = 0; i < objects_.size(); ++i)
		if (active_[i])
			for (uint32_t j = 0; j < objects_[i].symbols.size(); ++j)
			{
				const ObjectRef ref{ i, j }, definition = selected(ref);
				const ObjectSymbol& source = symbol(ref);
				const ObjectSymbol& target = symbol(definition);
				if (source.kind != target.kind)
					throw WasmError("Wasm symbol kind mismatch: " + source.name);
				if (source.kind == 0 &&
					!(objects_[i].types[objects_[i].functions[source.index].type] ==
						objects_[definition.first].types[objects_[definition.first].functions[target.index].type]))
					throw WasmError("Wasm function signature mismatch: " + source.name);
				if (source.kind == 2 &&
					(global_type(objects_[i], source.index) != global_type(objects_[definition.first], target.index) ||
						mutable_global(objects_[i], source.index) !=
							mutable_global(objects_[definition.first], target.index)))
					throw WasmError("Wasm global type/mutability mismatch: " + source.name);
				if (source.kind == 4 &&
					!(objects_[i].types[objects_[i].tags[source.index].type] ==
						objects_[definition.first].types[objects_[definition.first].tags[target.index].type]))
					throw WasmError("Wasm tag signature mismatch: " + source.name);
			}
}

void Linker::resolve_archive_references()
{
	// Archive extraction precedes section collection. An otherwise dead
	// reference can bring in a member whose strong definition replaces a
	// weak fallback (the supplied libc uses this for stream finalization).
	while (next_pending_ < pending_objects_.size())
	{
		const uint32_t index = pending_objects_[next_pending_++];
		for (const ObjectSymbol& source : objects_[index].symbols)
			if (source.undefined() && !source.weak() && source.kind != 3)
				lookup(source.name);
	}
}

void Linker::dependencies(uint32_t object, const ObjectChunk& chunk)
{
	for (const Relocation& relocation : chunk.relocations)
		if (relocation.kind != RelocationKind::TypeIndex)
			visit({ object, relocation.index });
}

void Linker::visit(ObjectRef ref)
{
	if (symbol(ref).undefined() && !symbol(ref).weak() && !host_import(ref))
		lookup(symbol(ref).name);
	ref = selected(ref);
	const auto& source = symbol(ref);
	uint32_t runtime = 0;
	if (source.kind == 5 || (source.kind == 2 && runtime_global(ref, runtime)))
		return;
	if (source.undefined() && !host_import(ref))
	{
		if (source.kind == 0 && source.name == "__wasm_call_ctors")
		{
			const Signature& type = objects_[ref.first].types[objects_[ref.first].functions[source.index].type];
			if (!type.parameters.empty() || !type.results.empty())
				throw WasmError("Wasm constructor wrapper signature is not void()");
			needs_ctors_ = true;
			return;
		}
		uint32_t value = 0;
		if (source.kind == 1 && synthetic_data(source.name, value))
			return;
		if (!source.weak() || source.kind == 4)
			throw WasmError("undefined Wasm symbol: " + source.name);
		if (source.kind == 0)
			weak_functions_.insert({ ref.first, source.index });
		else if (source.kind != 2)
			return;
	}
	const ObjectRef key{ ref.first, source.index };
	if (source.kind == 0)
	{
		if (!functions_.emplace(key, 0).second)
			return;
		ordered_functions_.push_back(key);
	}
	else if (source.kind == 1)
	{
		if (!data_.emplace(key, 0).second)
			return;
		ordered_data_.push_back(key);
	}
	else if (source.kind == 2)
	{
		if (!globals_.emplace(key, 0).second)
			return;
		(source.undefined() && host_import(ref) ? imported_globals_ : ordered_globals_).push_back(key);
	}
	else if (source.kind == 4)
	{
		if (!tags_.emplace(key, 0).second)
			return;
		ordered_tags_.push_back(key);
	}
	else
		throw WasmError("unsupported live Wasm symbol kind");
}

void Linker::collect()
{
	for (;;)
	{
		resolve_archive_references();
		const auto before = std::count(active_.begin(), active_.end(), true);
		functions_.clear();
		weak_functions_.clear();
		data_.clear();
		tags_.clear();
		globals_.clear();
		ordered_globals_.clear();
		imported_globals_.clear();
		ordered_functions_.clear();
		ordered_data_.clear();
		ordered_tags_.clear();
		if (options_.target == target_model::Target::Wasi && !options_.no_startfiles)
		{
			const ObjectRef entry = lookup("_start");
			if (entry.first == kAbsent || symbol(entry).kind != 0)
				throw WasmError("WASI command has no _start function");
			const Object& object = objects_[entry.first];
			const Signature& type = object.types[object.functions[symbol(entry).index].type];
			if (!type.parameters.empty() || !type.results.empty())
				throw WasmError("WASI _start signature is not void()");
			visit(entry);
		}
		for (const std::string& name : options_.exports)
		{
			const ObjectRef ref = lookup(name);
			if (ref.first == kAbsent)
				throw WasmError("undefined Wasm export: " + name);
			visit(ref);
		}
		size_t next_root = 0, next_function = 0, next_data = 0, next_global = 0;
		while (next_root < roots_.size() || next_function < ordered_functions_.size() ||
			next_data < ordered_data_.size() || next_global < ordered_globals_.size())
		{
			while (next_root < roots_.size())
				visit(roots_[next_root++]);
			if (next_function < ordered_functions_.size())
			{
				const ObjectRef ref = ordered_functions_[next_function++];
				dependencies(ref.first, objects_[ref.first].functions[ref.second].code);
			}
			if (next_data < ordered_data_.size())
			{
				const ObjectRef ref = ordered_data_[next_data++];
				dependencies(ref.first, objects_[ref.first].data[ref.second].contents);
			}
			if (next_global < ordered_globals_.size())
			{
				const ObjectRef ref = ordered_globals_[next_global++];
				const auto& object = objects_[ref.first];
				if (ref.second >= object.global_imports.size())
					dependencies(
						ref.first, object.global_definitions[ref.second - object.global_imports.size()].initializer);
			}
		}
		if (before == std::count(active_.begin(), active_.end(), true))
			break;
	}
	std::stable_sort(initializers_.begin(), initializers_.end(),
		[](const std::pair<uint32_t, ObjectRef>& a, const std::pair<uint32_t, ObjectRef>& b) {
			return a.first < b.first;
		});
	std::stable_partition(ordered_functions_.begin(), ordered_functions_.end(), [&](ObjectRef ref) {
		return objects_[ref.first].functions[ref.second].imported && !weak_functions_.count(ref);
	});
	for (uint32_t i = 0; i < ordered_functions_.size(); ++i)
	{
		functions_[ordered_functions_[i]] = i;
		const ObjectRef ref = ordered_functions_[i];
		imported_ += objects_[ref.first].functions[ref.second].imported && !weak_functions_.count(ref);
	}
	std::stable_partition(ordered_tags_.begin(), ordered_tags_.end(),
		[&](ObjectRef ref) { return objects_[ref.first].tags[ref.second].imported; });
	for (uint32_t i = 0; i < ordered_tags_.size(); ++i)
	{
		tags_[ordered_tags_[i]] = i;
		imported_tags_ += objects_[ordered_tags_[i].first].tags[ordered_tags_[i].second].imported;
	}
	uint64_t end = 1024;
	for (ObjectRef ref : ordered_data_)
	{
		const ObjectData& data = objects_[ref.first].data[ref.second];
		const uint64_t align = uint64_t(1) << data.alignment;
		end = (end + align - 1) & ~(align - 1);
		if (end + data.contents.bytes.data.size() > 0x7fd00000u)
			throw WasmError("Wasm linked data exceeds the supported memory range");
		data_[ref] = static_cast<uint32_t>(end);
		end += data.contents.bytes.data.size();
	}
	stack_floor_ = static_cast<uint32_t>((end + 15) & ~uint64_t(15));
	stack_top_ = stack_floor_ + 1024 * 1024;
	if (needs_ctors_ || !initializers_.empty())
		ctor_index_ = static_cast<uint32_t>(ordered_functions_.size());
}

void Linker::run(Bytes& output)
{
	resolve();
	collect();
	validate_symbols();
	validate_link_features(objects_, active_);
	prepare_address_globals();
	prepare_globals();
	for (uint32_t object_index = 0; object_index < objects_.size(); ++object_index)
	{
		const Object& object = objects_[object_index];
		std::vector<uint32_t> indices;
		if (!active_[object_index])
		{
			type_indices_.push_back(std::move(indices));
			continue;
		}
		for (const Signature& type : object.types)
		{
			auto found = std::find(types_.begin(), types_.end(), type);
			if (found == types_.end())
			{
				indices.push_back(static_cast<uint32_t>(types_.size()));
				types_.push_back(type);
			}
			else
				indices.push_back(static_cast<uint32_t>(found - types_.begin()));
		}
		type_indices_.push_back(std::move(indices));
	}
	emit(output);
}

void link_objects(const std::vector<Object>& objects, const backend::Options& options, Bytes& output)
{
	Linker(objects, options).run(output);
}

} // namespace wasm
