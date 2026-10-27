#pragma once

#include <cstddef>
#include <cstdint>

struct Identifier;

// The analysis's hashed keys (sema/analyzer.h's maps, by owner): dependent
// member placeholders by (qualifier, name); decltype and value placeholders
// by their operand's structural words, and their substitutions by
// (placeholder, arguments, depth, context); checked (entity, entity) pairs;
// virtual function chains and virtual bases; local numbering and closure
// contexts for mangling; specializations by (template, arguments).

namespace sema {

struct Entity;
struct Type;
struct ClassEntity;
struct FunctionEntity;

struct PlaceholderKey
{
	const Type* qualifier;
	const Identifier* name;
	bool operator==(const PlaceholderKey& other) const { return qualifier == other.qualifier && name == other.name; }
};

struct PlaceholderHash
{
	size_t operator()(const PlaceholderKey& key) const;
};

struct DecltypeKey
{
	const uint64_t* words;
	uint32_t count;
	bool operator==(const DecltypeKey& other) const;
};

struct DecltypeHash
{
	size_t operator()(const DecltypeKey& key) const;
};

struct SubstitutionKey
{
	const Entity* placeholder;
	const Type* const* arguments;
	uint32_t count;
	uint32_t depth;
	const Entity* context;
	bool operator==(const SubstitutionKey& other) const;
};

struct SubstitutionHash
{
	size_t operator()(const SubstitutionKey& key) const;
};

// A default template argument where its declaration gave it: the
// type-id and the template parameter scope (a class template instance's
// member template shares its pattern's type-id, in a scope of its own).
struct DefaultKey
{
	const void* type;
	const void* scope;
	bool operator==(const DefaultKey& other) const { return type == other.type && scope == other.scope; }
};

struct DefaultKeyHash
{
	size_t operator()(const DefaultKey& key) const
	{
		return (reinterpret_cast<uintptr_t>(key.type) * 0x9E3779B97F4A7C15ull) ^ reinterpret_cast<uintptr_t>(key.scope);
	}
};

struct EntityPair
{
	const Entity* first;
	const Entity* second;
	bool operator==(const EntityPair& other) const { return first == other.first && second == other.second; }
};

struct EntityPairHash
{
	size_t operator()(const EntityPair& key) const;
};

struct FunctionOrderKey
{
	const FunctionEntity* first;
	const FunctionEntity* second;
	const Type* first_type;
	const Type* second_type;
	const void* first_head;
	const void* second_head;
	const Entity* access;
	uint32_t arguments;
	int32_t element;
	uint8_t form;
	bool operator==(const FunctionOrderKey& other) const
	{
		return first == other.first && second == other.second && first_type == other.first_type &&
			second_type == other.second_type && first_head == other.first_head && second_head == other.second_head &&
			access == other.access && arguments == other.arguments && element == other.element && form == other.form;
	}
};

struct FunctionOrderHash
{
	size_t operator()(const FunctionOrderKey& key) const
	{
		uint64_t hash = reinterpret_cast<uintptr_t>(key.first);
		const void* identities[] = { key.second, key.first_type, key.second_type, key.first_head, key.second_head,
			key.access };
		for (const void* identity : identities)
			hash = (hash ^ reinterpret_cast<uintptr_t>(identity)) * 0x9e3779b97f4a7c15ull;
		return static_cast<size_t>(
			hash ^ (uint64_t(key.arguments) << 32) ^ (uint64_t(uint32_t(key.element)) << 8) ^ key.form);
	}
};

struct ChainKey
{
	const ClassEntity* cls;
	const FunctionEntity* function;
	const ClassEntity* vbase;
	bool operator==(const ChainKey& other) const
	{
		return cls == other.cls && function == other.function && vbase == other.vbase;
	}
};

struct ChainHash
{
	size_t operator()(const ChainKey& key) const
	{
		return (reinterpret_cast<uintptr_t>(key.cls) * 0x9E3779B97F4A7C15ull) ^
			reinterpret_cast<uintptr_t>(key.function) ^ (reinterpret_cast<uintptr_t>(key.vbase) * 31);
	}
};

struct VirtualBaseKey
{
	const ClassEntity* cls;
	const ClassEntity* base;
	bool operator==(const VirtualBaseKey& other) const { return cls == other.cls && base == other.base; }
};

struct VirtualBaseHash
{
	size_t operator()(const VirtualBaseKey& key) const
	{
		return (reinterpret_cast<uintptr_t>(key.cls) * 0x9E3779B97F4A7C15ull) ^
			(reinterpret_cast<uintptr_t>(key.base) * 31);
	}
};

struct LocalNumberKey
{
	const FunctionEntity* function;
	const void* key;
	bool operator==(const LocalNumberKey& other) const { return function == other.function && key == other.key; }
};

struct LocalNumberHash
{
	size_t operator()(const LocalNumberKey& key) const;
};

struct ClosureContextKey
{
	const Entity* context;
	uint32_t default_argument;
	const void* signature;
	bool operator==(const ClosureContextKey& other) const
	{
		return context == other.context && default_argument == other.default_argument && signature == other.signature;
	}
};

struct ClosureContextHash
{
	size_t operator()(const ClosureContextKey& key) const;
};

struct SpecializationKey
{
	const Entity* primary;
	const Type* const* arguments;
	uint32_t count;
	bool operator==(const SpecializationKey& other) const;
};

struct SpecializationHash
{
	size_t operator()(const SpecializationKey& key) const;
};

} // namespace sema
