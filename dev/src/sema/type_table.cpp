#include "sema/type_table.h"

#include <cstring>

namespace sema {

namespace {

// One field into the running hash: rotate, combine, multiply (each field's
// bits reach the high half, which the final avalanche folds down).
uint64_t Mix(uint64_t hash, uint64_t value)
{
	return (((hash << 5) | (hash >> 59)) ^ value) * 0x517cc1b727220a95ull;
}

uint64_t Bits(const void* pointer)
{
	return static_cast<uint64_t>(reinterpret_cast<uintptr_t>(pointer));
}

Type Key(TypeKind kind)
{
	Type key;
	std::memset(&key, 0, sizeof key);
	key.kind = kind;
	return key;
}

} // namespace

TypeTable::TypeTable(Arena& arena) : arena_(arena), slots_(256), count_(0)
{
	for (unsigned i = 0; i < kFundamentalTypeCount; ++i)
	{
		Type key = Key(TypeKind::Fundamental);
		key.fundamental = static_cast<FundamentalType>(i);
		fundamentals_[i] = intern(key);
	}
}

uint32_t TypeTable::hash_of(const Type& type)
{
	// The byte fields nine bits apart: no byte-aligned layout, so no host
	// compiler reads them as one wide load, which could not be forwarded
	// from the byte stores callers just made building the key.
	uint64_t wide = static_cast<uint64_t>(type.kind) | static_cast<uint64_t>(type.cv) << 9 |
		static_cast<uint64_t>(type.function_cv) << 18 | static_cast<uint64_t>(type.ref) << 27 |
		static_cast<uint64_t>(type.variadic) << 36 | static_cast<uint64_t>(type.fundamental) << 45;
	wide = Mix(wide, type.bound);
	wide = Mix(wide, Bits(type.element));
	wide = Mix(wide, Bits(type.owner));
	wide = Mix(wide, Bits(type.entity));
	for (uint32_t i = 0; i < type.param_count; ++i)
		wide = Mix(wide, Bits(type.params[i]));
	// A final avalanche for the low bits the table indexes by.
	uint32_t hash = static_cast<uint32_t>(wide ^ (wide >> 32));
	hash ^= hash >> 16;
	hash *= 0x7feb352du;
	hash ^= hash >> 15;
	hash *= 0x846ca68bu;
	return hash ^ (hash >> 16);
}

bool TypeTable::same(const Type& a, const Type& b)
{
	if (a.kind != b.kind || a.cv != b.cv || a.function_cv != b.function_cv || a.ref != b.ref ||
		a.variadic != b.variadic || a.fundamental != b.fundamental || a.bound != b.bound || a.element != b.element ||
		a.owner != b.owner || a.entity != b.entity || a.param_count != b.param_count)
		return false;
	for (uint32_t i = 0; i < a.param_count; ++i)
	{
		if (a.params[i] != b.params[i])
			return false;
	}
	return true;
}

void TypeTable::grow()
{
	std::vector<Slot, ZeroedAllocator<Slot>> old(slots_.size() * 2);
	old.swap(slots_);
	const uint32_t mask = static_cast<uint32_t>(slots_.size() - 1);
	mask_ = mask;
	for (const Slot& slot : old)
	{
		if (slot.type == nullptr)
			continue;
		uint32_t index = slot.hash & mask;
		while (slots_[index].type != nullptr)
			index = (index + 1) & mask;
		slots_[index] = slot;
	}
}

const Type* TypeTable::intern(const Type& key)
{
	// Slots carry the hash, so a probe touches a type only on a hash match.
	const uint32_t hash = hash_of(key);
	const uint32_t mask = mask_;
	uint32_t index = hash & mask;
	while (slots_[index].type != nullptr)
	{
		if (slots_[index].hash == hash && same(*slots_[index].type, key))
			return slots_[index].type;
		index = (index + 1) & mask;
	}
	return insert(key, hash, index);
}

const Type* TypeTable::insert(const Type& key, uint32_t hash, uint32_t index)
{
	// A new type at the empty slot intern's probe ended at (apart, so that
	// a found type's path saves no registers for this one's work).
	Type* type = static_cast<Type*>(arena_.allocate(sizeof(Type), alignof(Type)));
	*type = key;
	type->hash = hash;
	if (key.param_count != 0)
		type->params = arena_.copy_array(key.params, key.param_count);
	type->unqualified = type;
	type->canonical = type;
	slots_[index] = Slot{ hash, type };
	if (++count_ * 2 > slots_.size())
		grow();
	// Both refer only to already interned (or equal) types, so the table
	// can be searched again; the recursion follows the type's structure.
	if (key.cv != 0)
	{
		Type bare = *type;
		bare.cv = 0;
		type->unqualified = intern(bare);
	}
	type->canonical = canonical_of(*type);
	return type;
}

const Type* TypeTable::canonical_of(const Type& type)
{
	Type key = type;
	bool changed = false;
	if ((type.kind == TypeKind::TemplateParameter || type.kind == TypeKind::TemplateTemplateParameter) &&
		type.entity != nullptr)
	{
		// A template parameter is canonically its position (14.5.6.1): the
		// positional entity every declaration's parameter there shares.
		const TemplateParameterEntity* parameter = static_cast<const TemplateParameterEntity*>(type.entity);
		if (parameter->canonical == nullptr || parameter->canonical == parameter)
			return &type;
		key.entity = parameter->canonical;
		return intern(key);
	}
	if (type.element != nullptr && type.element->canonical != type.element)
	{
		key.element = type.element->canonical;
		changed = true;
	}
	if (type.owner != nullptr && type.owner->canonical != type.owner)
	{
		key.owner = type.owner->canonical;
		changed = true;
	}
	if (type.kind == TypeKind::Pack && type.param_count != 0)
	{
		// A pack's elements are template arguments: canonical, unadjusted.
		const size_t base = scratch_.size();
		for (uint32_t i = 0; i < type.param_count; ++i)
		{
			changed = changed || type.params[i]->canonical != type.params[i];
			scratch_.push_back(type.params[i]->canonical);
		}
		const Type* result = &type;
		if (changed)
		{
			key.params = scratch_.data() + base;
			result = intern(key);
		}
		scratch_.resize(base);
		return result;
	}
	if (type.kind == TypeKind::Function && type.param_count != 0)
	{
		// Nested interning may grow the scratch vector, so it is indexed
		// until the key is complete; intern() copies the parameters before
		// it recurses.
		const size_t base = scratch_.size();
		for (uint32_t i = 0; i < type.param_count; ++i)
		{
			const Type* adjusted = adjusted_parameter(type.params[i]);
			changed = changed || adjusted != type.params[i];
			scratch_.push_back(adjusted);
		}
		const Type* result = &type;
		if (changed)
		{
			key.params = scratch_.data() + base;
			result = intern(key);
		}
		scratch_.resize(base);
		return result;
	}
	if (!changed)
		return &type;
	return intern(key);
}

const Type* TypeTable::adjusted_parameter(const Type* type)
{
	const Type* canonical = type->canonical;
	if (canonical->kind == TypeKind::Array)
		return pointer(canonical->element);
	if (canonical->kind == TypeKind::Function)
		return pointer(canonical);
	return canonical->unqualified;
}

const Type* TypeTable::qualified_anew(const Type* type, uint8_t cv)
{
	switch (type->kind)
	{
	case TypeKind::Array:
		return array(qualified(type->element, cv), type->bound, type->owner);
	case TypeKind::LValueReference:
	case TypeKind::RValueReference:
	case TypeKind::Function:
	case TypeKind::Value:
	case TypeKind::Pack:
	case TypeKind::Expansion:
		return type;
	default:
		break;
	}
	Type key = *type;
	key.cv = static_cast<uint8_t>(type->cv | cv);
	return intern(key);
}

const Type* TypeTable::pointer(const Type* pointee)
{
	Type key = Key(TypeKind::Pointer);
	key.element = pointee;
	return intern(key);
}

const Type* TypeTable::reference(const Type* referee, bool rvalue)
{
	if (referee->is_reference())
	{
		rvalue = rvalue && referee->kind == TypeKind::RValueReference;
		referee = referee->element;
	}
	Type key = Key(rvalue ? TypeKind::RValueReference : TypeKind::LValueReference);
	key.element = referee;
	return intern(key);
}

const Type* TypeTable::member_pointer(const Type* owner, const Type* member)
{
	Type key = Key(TypeKind::MemberPointer);
	key.owner = owner;
	key.element = member;
	return intern(key);
}

const Type* TypeTable::array(const Type* element, uint64_t bound, const Type* dependent_bound)
{
	Type key = Key(TypeKind::Array);
	key.element = element;
	key.bound = bound;
	key.owner = dependent_bound;
	return intern(key);
}

const Type* TypeTable::vector(const Type* lane, uint64_t lanes, const Type* dependent_size, bool bytes)
{
	Type key = Key(TypeKind::Vector);
	key.element = lane;
	key.bound = lanes;
	key.owner = dependent_size;
	key.variadic = dependent_size != nullptr && bytes;
	return intern(key);
}

const Type* TypeTable::block_pointer(const Type* function)
{
	Type key = Key(TypeKind::BlockPointer);
	key.element = function;
	return intern(key);
}

const Type* TypeTable::complex(const Type* element)
{
	Type key = Key(TypeKind::Complex);
	key.element = element;
	return intern(key);
}

const Type* TypeTable::function(
	const Type* result, const Type* const* params, uint32_t count, bool variadic, uint8_t cv, RefQualifier ref)
{
	Type key = Key(TypeKind::Function);
	key.element = result;
	key.params = params;
	key.param_count = count;
	key.variadic = variadic;
	key.function_cv = cv;
	key.ref = ref;
	return intern(key);
}

const Type* TypeTable::find_function(
	const Type* result, const Type* const* params, uint32_t count, bool variadic, uint8_t cv, RefQualifier ref) const
{
	Type key = Key(TypeKind::Function);
	key.element = result;
	key.params = params;
	key.param_count = count;
	key.variadic = variadic;
	key.function_cv = cv;
	key.ref = ref;
	const uint32_t hash = hash_of(key);
	const uint32_t mask = mask_;
	for (uint32_t index = hash & mask; slots_[index].type != nullptr; index = (index + 1) & mask)
	{
		if (slots_[index].hash == hash && same(*slots_[index].type, key))
			return slots_[index].type;
	}
	return nullptr;
}

const Type* TypeTable::named(TypeKind kind, Entity* entity)
{
	Type key = Key(kind);
	key.entity = entity;
	return intern(key);
}

const Type* TypeTable::value(const Type* type, uint64_t bits)
{
	Type key = Key(TypeKind::Value);
	key.element = type;
	key.bound = bits;
	return intern(key);
}

const Type* TypeTable::address(const Type* type, Entity* entity)
{
	Type key = Key(TypeKind::Value);
	key.element = type;
	key.entity = entity;
	return intern(key);
}

const Type* TypeTable::pack(const Type* const* elements, uint32_t count)
{
	Type key = Key(TypeKind::Pack);
	key.params = elements;
	key.param_count = count;
	return intern(key);
}

const Type* TypeTable::expansion(const Type* pattern)
{
	Type key = Key(TypeKind::Expansion);
	key.element = pattern;
	return intern(key);
}

const Type* TypeTable::bound_pack(const Type* pack, Entity* parameter, uint32_t level)
{
	Type key = Key(TypeKind::BoundPack);
	key.element = pack;
	key.entity = parameter;
	key.bound = level;
	return intern(key);
}

} // namespace sema
