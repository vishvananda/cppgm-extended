#include "wasm/linker.h"

#include <algorithm>

namespace wasm {

bool Linker::runtime_global(ObjectRef ref, uint32_t& index) const
{
	const auto& source = symbol(ref);
	if (source.kind != 2 || !source.undefined())
		return false;
	const ObjectGlobal& global = objects_[ref.first].global_imports[source.index];
	if (global.module != "env")
		return false;
	if (global.name == "__stack_pointer")
		index = 0;
	else if (global.name == "__memory_base")
		index = 1;
	else if (global.name == "__table_base")
		index = 2;
	else
		return false;
	if (global.type != ValueType::I32 || (index == 0 && !global.mutable_value))
		throw WasmError(objects_[ref.first].path + ": Wasm runtime global type/mutability mismatch: " + global.name);
	return true;
}

void Linker::prepare_globals()
{
	std::map<ObjectRef, uint32_t> positions;
	for (uint32_t i = 0; i < ordered_globals_.size(); ++i)
		positions.emplace(ordered_globals_[i], i);
	std::vector<std::vector<uint32_t>> users(ordered_globals_.size());
	std::vector<uint32_t> incoming(ordered_globals_.size());
	for (uint32_t i = 0; i < ordered_globals_.size(); ++i)
	{
		const ObjectRef ref = ordered_globals_[i];
		const Object& object = objects_[ref.first];
		if (ref.second < object.global_imports.size())
			continue;
		std::set<uint32_t> dependencies;
		for (const auto& relocation :
			object.global_definitions[ref.second - object.global_imports.size()].initializer.relocations)
			if (relocation.kind == RelocationKind::GlobalIndex && object.symbols[relocation.index].kind == 2)
			{
				const ObjectRef target = selected({ ref.first, relocation.index });
				const auto found = positions.find({ target.first, symbol(target).index });
				if (found != positions.end())
					dependencies.insert(found->second);
			}
		incoming[i] = static_cast<uint32_t>(dependencies.size());
		for (uint32_t dependency : dependencies)
			users[dependency].push_back(i);
	}
	std::set<uint32_t> ready;
	for (uint32_t i = 0; i < incoming.size(); ++i)
		if (incoming[i] == 0)
			ready.insert(i);
	std::vector<ObjectRef> ordered;
	while (!ready.empty())
	{
		const uint32_t next = *ready.begin();
		ready.erase(ready.begin());
		ordered.push_back(ordered_globals_[next]);
		for (uint32_t user : users[next])
			if (--incoming[user] == 0)
				ready.insert(user);
	}
	if (ordered.size() != ordered_globals_.size())
		throw WasmError("Wasm global initializers form a cycle after symbol resolution");
	ordered_globals_.swap(ordered);
	const uint64_t base = imported_globals_.size() + 3 + address_globals_.size();
	if (base + ordered_globals_.size() > UINT32_MAX)
		throw WasmError("too many Wasm globals");
	for (uint32_t i = 0; i < imported_globals_.size(); ++i)
		globals_[imported_globals_[i]] = i;
	for (uint32_t i = 0; i < ordered_globals_.size(); ++i)
		globals_[ordered_globals_[i]] = static_cast<uint32_t>(base + i);
}

void Linker::global_imports(Bytes& output) const
{
	for (ObjectRef ref : imported_globals_)
	{
		const ObjectGlobal& global = objects_[ref.first].global_imports[ref.second];
		output.string(global.module);
		output.string(global.name);
		output.byte(3);
		output.byte(static_cast<uint8_t>(global.type));
		output.byte(global.mutable_value ? 1 : 0);
	}
}

void Linker::global_section(Bytes& output) const
{
	Bytes globals;
	globals.unsigned_leb(3 + address_globals_.size() + ordered_globals_.size());
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
		addresses[address.second - imported_globals_.size() - 3] = address.first;
	for (ObjectRef address : addresses)
	{
		globals.byte(0x7f);
		globals.byte(1);
		globals.byte(0x41);
		globals.signed_leb(static_cast<int32_t>(symbol_value(address)));
		globals.byte(0x0b);
	}
	for (ObjectRef ref : ordered_globals_)
	{
		const Object& object = objects_[ref.first];
		const ValueType type = global_type(object, ref.second);
		globals.byte(static_cast<uint8_t>(type));
		globals.byte(mutable_global(object, ref.second) ? 1 : 0);
		if (ref.second >= object.global_imports.size())
			globals.append(relocated(
				ref.first, object.global_definitions[ref.second - object.global_imports.size()].initializer, true));
		else
		{
			Function zero;
			if (type == ValueType::V128)
				zero.emit(Op::V128Const, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 });
			else
				zero.emit(is_reference(type)     ? Op::RefNull
						: type == ValueType::I32 ? Op::I32Const
						: type == ValueType::I64 ? Op::I64Const
						: type == ValueType::F32 ? Op::F32Const
												 : Op::F64Const,
					{ is_reference(type) ? block_type(type) : 0 });
			zero.emit(Op::End);
			encode_expression(zero, globals);
		}
	}
	output.section(6, globals);
}

} // namespace wasm
