#include "lower/program_lowering.h"

#include <cstring>

#include "sema/conversions.h"
#include "types/fundamental_type.h"

// Type information objects of any type (Itanium C++ ABI 2.9.5) beside the
// classes' (vtable_lowering.cpp), and the C++ runtime's functions the
// lowering calls (typeid's and dynamic_cast's failures and search, the
// exception runtime). An object is the runtime kind's vtable address past
// its header, the type's name and the kind's own fields; each is weak, its
// ABI name shared by every unit (a type of internal or no linkage: the
// unit's own), except `void`'s, which the runtime defines (the course's
// form). A runtime function is declared once per program with its role.
namespace lower {

namespace {

// The course's presentation of a fundamental type's name in its symbols:
// its simple-type-specifiers joined by `_`.
std::string FundamentalSpelling(FundamentalType type)
{
	if (type == FundamentalType::NullptrT)
		return "nullptr_t";
	std::string spelling = FundamentalTypeName(type);
	for (char& c : spelling)
	{
		if (c == ' ')
			c = '_';
	}
	return spelling;
}

// Itanium 2.9.5p7: __pbase_type_info's flags for a pointee's or member
// type's qualifiers, and a pointee of incomplete class type (0x8), or a
// member pointer's incomplete class (0x10).
uint32_t PointeeFlags(const sema::Type* pointee)
{
	uint32_t flags = 0;
	if ((pointee->cv & sema::kConst) != 0)
		flags |= 1;
	if ((pointee->cv & sema::kVolatile) != 0)
		flags |= 2;
	// The target or a type it points to incomplete (GCC's reading of
	// __incomplete_mask: `Inc**` and `Inc* const*` too).
	const sema::Type* target = pointee->unqualified;
	while (target->kind == sema::TypeKind::Pointer)
		target = target->element->unqualified;
	const sema::ClassEntity* cls = target->kind == sema::TypeKind::Class ? sema::ClassOf(target) : nullptr;
	if (cls != nullptr && !cls->complete)
		flags |= 8;
	return flags;
}

struct RuntimeSpec
{
	const char* spelling;
	const char* object;
	ir_model::SymbolRole role;
	// Parameters (`p` ptr, `l` i64) and the result (`v` void, `p` ptr).
	const char* parameters;
	char result;
	bool noreturn;
};

// Indexed by RuntimeFunction.
const RuntimeSpec kRuntime[] = {
	{ "__external_runtime____cxa_bad_typeid", "__cxa_bad_typeid", ir_model::SR_BAD_TYPEID, "", 'v', true },
	{ "__external_runtime____cxa_bad_cast", "__cxa_bad_cast", ir_model::SR_BAD_CAST, "", 'v', true },
	{ "__external_runtime____dynamic_cast", "__dynamic_cast", ir_model::SR_DYNAMIC_CAST, "pppl", 'p', false },
	{ "__external_runtime____cxa_allocate_exception", "__cxa_allocate_exception", ir_model::SR_EH_ALLOCATE_EXCEPTION,
		"l", 'p', false },
	{ "__external_runtime____cxa_throw", "__cxa_throw", ir_model::SR_EH_THROW, "ppp", 'v', true },
	{ "__external_runtime____cxa_rethrow", "__cxa_rethrow", ir_model::SR_EH_RETHROW, "", 'v', true },
	{ "__external_runtime____cxa_begin_catch", "__cxa_begin_catch", ir_model::SR_EH_BEGIN_CATCH, "p", 'p', false },
	{ "__external_runtime____cxa_end_catch", "__cxa_end_catch", ir_model::SR_EH_END_CATCH, "", 'v', false },
	{ "__external_runtime____cxa_call_unexpected", "__cxa_call_unexpected", ir_model::SR_NONE, "p", 'v', true },
	{ "__external_runtime____cxa_throw_bad_array_new_length", "__cxa_throw_bad_array_new_length",
		ir_model::SR_BAD_ARRAY_NEW_LENGTH, "", 'v', true },
};

static_assert(sizeof(kRuntime) / sizeof(kRuntime[0]) == static_cast<size_t>(RuntimeFunction::Count),
	"one entry per runtime function");

} // namespace

lowir::SymbolId ProgramLowering::runtime_function(RuntimeFunction which)
{
	const size_t index = static_cast<size_t>(which);
	if (runtime_functions_[index] != lowir::kNoId)
		return runtime_functions_[index];
	const RuntimeSpec& spec = kRuntime[index];
	const lowir::SymbolId symbol = new_symbol(spec.spelling);
	lowir::Signature signature = lowir::Signature();
	signature.first_parameter = static_cast<uint32_t>(program_.parameters.size());
	const uint32_t count = static_cast<uint32_t>(std::strlen(spec.parameters));
	for (uint32_t i = 0; i < count; ++i)
	{
		lowir::Parameter parameter = lowir::Parameter();
		parameter.name = argument_name(i);
		parameter.type = lowir::Type::scalar(spec.parameters[i] == 'l' ? lowir::TypeKind::I64 : lowir::TypeKind::Ptr);
		program_.parameters.push_back(parameter);
	}
	signature.parameter_count = count;
	signature.result = lowir::Type::scalar(spec.result == 'p' ? lowir::TypeKind::Ptr : lowir::TypeKind::Void);
	if (spec.noreturn)
		signature.boundary.returns = ir_model::CRM_NORETURN;
	program_.declare_function(symbol, lowir::SymbolKind::FunctionDeclaration, program_.add_signature(signature));
	lowir::SymbolMetadata& metadata = program_.symbols[symbol].metadata;
	metadata.role = spec.role;
	metadata.linkage = ir_model::LLM_C;
	metadata.binding = ir_model::SBM_STRONG;
	metadata.object = intern(spec.object, static_cast<uint32_t>(std::strlen(spec.object)));
	runtime_functions_[index] = symbol;
	++counters_.declarations;
	return symbol;
}

lowir::SymbolId ProgramLowering::type_information(const sema::Type* type)
{
	// The object typeid designates for `type` (cv-unqualified, 5.2.8p5).
	// A host object's of a fundamental type, or a pointer to one (or to a
	// const one), is the C++ runtime's (Itanium 2.9.2), declared.
	type = type->canonical->unqualified;
	if (type->kind == sema::TypeKind::Class)
		return class_information(static_cast<const sema::ClassEntity*>(type->entity));
	if (host_objects_ && RuntimeTypeInformation(type))
		return exception_type_information(type);
	const uint32_t found = type_informations_.find(type);
	if (found != PointerMap::kMissing)
		return found;
	if (type->kind == sema::TypeKind::Fundamental && type->fundamental == FundamentalType::Void)
	{
		if (void_information_ == lowir::kNoId)
		{
			void_information_ = new_symbol("__external_rtti__void");
			lowir::Global global = lowir::Global();
			global.typed = false;
			global.init = lowir::GlobalInit::None;
			program_.declare_global(void_information_, lowir::SymbolKind::GlobalDeclaration, global);
			lowir::SymbolMetadata& metadata = program_.symbols[void_information_].metadata;
			metadata.role = ir_model::SR_RTTI_DATA;
			metadata.binding = ir_model::SBM_STRONG;
			metadata.object = intern("_ZTIv", 5);
			++counters_.declarations;
		}
		type_informations_.set(type, void_information_);
		return void_information_;
	}
	const bool fundamental = type->kind == sema::TypeKind::Fundamental;
	const std::string label = fundamental ? FundamentalSpelling(type->fundamental)
										  : "type_" + type_object(type, abi_mangle::ABI_TARGET_TYPE);
	const std::string object = type_object(type, abi_mangle::ABI_TARGET_TYPEINFO);
	const bool internal = type_locality(type) != Locality::External;
	lowir::SymbolId symbol = lowir::kNoId;
	if (internal || local_type_information(type))
	{
		symbol = new_symbol("__rtti_" + label);
		symbol_objects_[symbol] = local_object(object, false);
	}
	else
		symbol = external_symbol(object, "__rtti_" + label);
	type_informations_.set(type, symbol);
	if (program_.symbols[symbol].kind != lowir::SymbolKind::Unresolved)
		return symbol;
	// The referenced objects first: defining them appends data of their own.
	TypeInfoKind kind = TypeInfoKind::Fundamental;
	lowir::SymbolId pointee = lowir::kNoId;
	lowir::SymbolId owner = lowir::kNoId;
	switch (type->kind)
	{
	case sema::TypeKind::Pointer:
		kind = TypeInfoKind::Pointer;
		pointee = type_information(type->element);
		break;
	case sema::TypeKind::MemberPointer:
		kind = TypeInfoKind::PointerToMember;
		pointee = type_information(type->element);
		owner = type_information(type->owner);
		break;
	case sema::TypeKind::Function:
		kind = TypeInfoKind::Function;
		break;
	case sema::TypeKind::Array:
		kind = TypeInfoKind::Array;
		break;
	case sema::TypeKind::Enum:
		kind = TypeInfoKind::Enum;
		break;
	default:
		break;
	}
	// A pointee the C++ runtime defines is the unit's own definition, not
	// the runtime's declaration a throw or handler names (the course's
	// form; see defined_type_information).
	if (!host_objects_ && pointee != lowir::kNoId &&
		program_.symbols[pointee].kind == lowir::SymbolKind::GlobalDeclaration &&
		RuntimeTypeInformation(type->element->canonical->unqualified))
		pointee = defined_type_information(type->element->canonical->unqualified);
	const lowir::SymbolId vtable = rtti_vtable(kind);
	const lowir::SymbolId name = type_name_object(type, fundamental ? "_" + label : label, internal);
	const uint32_t first = data_item(AddressItem(vtable, 16));
	data_item(AddressItem(name, 0));
	if (pointee != lowir::kNoId)
	{
		const sema::ClassEntity* holder = owner != lowir::kNoId ? sema::ClassOf(type->owner) : nullptr;
		data_item(ScalarItem(
			lowir::TypeKind::I32, PointeeFlags(type->element) | (holder != nullptr && !holder->complete ? 0x10 : 0)));
		data_item(AddressItem(pointee, 0));
	}
	if (owner != lowir::kNoId)
		data_item(AddressItem(owner, 0));
	lowir::SymbolMetadata metadata;
	metadata.binding = internal || local_type_information(type) ? ir_model::SBM_INTERNAL : ir_model::SBM_WEAK;
	define_data(symbol, first, metadata);
	return symbol;
}

bool ProgramLowering::RuntimeTypeInformation(const sema::Type* type)
{
	// A fundamental type T (but void, declared on its own) or a pointer to
	// T or to const T: the C++ runtime defines its type information
	// (Itanium 2.9.2).
	const sema::Type* base = type->kind == sema::TypeKind::Pointer ? type->element : type;
	const bool pointer = base != type;
	return base->kind == sema::TypeKind::Fundamental && (base->cv & sema::kVolatile) == 0 &&
		(pointer || base->fundamental != FundamentalType::Void);
}

lowir::SymbolId ProgramLowering::exception_type_information(const sema::Type* type)
{
	// `__external_rtti__<name>` for a fundamental type T or a pointer to T
	// or to const T (`pointer_to_[const_]<name>`), declared: the C++
	// runtime defines those (Itanium 2.9.2); another type's is the
	// program's own.
	type = type->canonical->unqualified;
	const sema::Type* base = type->kind == sema::TypeKind::Pointer ? type->element : type;
	const bool pointer = base != type;
	if (!RuntimeTypeInformation(type))
		return type_information(type);
	const uint32_t found = exception_informations_.find(type);
	if (found != PointerMap::kMissing)
		return found;
	std::string spelling = "__external_rtti__";
	if (pointer)
		spelling += (base->cv & sema::kConst) != 0 ? "pointer_to_const_" : "pointer_to_";
	spelling += FundamentalSpelling(base->fundamental);
	const lowir::SymbolId symbol = external_symbol(type_object(type, abi_mangle::ABI_TARGET_TYPEINFO), spelling);
	if (program_.symbols[symbol].kind == lowir::SymbolKind::Unresolved)
	{
		lowir::Global global = lowir::Global();
		global.typed = false;
		global.init = lowir::GlobalInit::None;
		program_.declare_global(symbol, lowir::SymbolKind::GlobalDeclaration, global);
		lowir::SymbolMetadata& metadata = program_.symbols[symbol].metadata;
		metadata.role = ir_model::SR_RTTI_DATA;
		metadata.binding = ir_model::SBM_STRONG;
		metadata.object = symbol_objects_[symbol];
		++counters_.declarations;
	}
	exception_informations_.set(type, symbol);
	return symbol;
}

void ProgramLowering::thrown_type(const sema::Type* type)
{
	// The course's LowIR units define, for each type a throw-expression
	// names, that type's own information object (beside the runtime's
	// declaration the throw names for a fundamental type or a pointer to
	// one) and an internal zero-filled object of the exception object's
	// size. Nothing refers to either; object mode leaves both out.
	if (object_mode_)
		return;
	type = type->canonical->unqualified;
	if (thrown_types_.find(type) != PointerMap::kMissing)
		return;
	thrown_types_.set(type, 1);
	const lowir::SymbolId information = defined_type_information(type);
	std::string stem = program_.spelling(program_.symbols[information].name);
	if (stem.compare(0, 7, "__rtti_") == 0)
		stem.erase(0, 7);
	const lowir::SymbolId symbol = new_symbol("__ehobj_" + stem);
	lowir::DataItem zero = lowir::DataItem();
	zero.kind = lowir::DataItemKind::Zero;
	zero.amount = static_cast<int64_t>(size_of(type, 0));
	const uint32_t first = data_item(zero);
	lowir::SymbolMetadata metadata;
	metadata.binding = ir_model::SBM_INTERNAL;
	define_data(symbol, first, metadata);
	program_.symbols[symbol].metadata.storage = ir_model::GSM_DEFAULT;
}

void ProgramLowering::caught_type(const sema::Type* type)
{
	// A handler's caught type has its own type information defined too
	// (the course's LowIR units; see thrown_type).
	if (!object_mode_)
		defined_type_information(type->canonical->unqualified);
}

lowir::SymbolId ProgramLowering::defined_type_information(const sema::Type* type)
{
	// type_information's object, but defined (weak) for a type whose
	// object the runtime defines (a fundamental type, a pointer to one),
	// apart from the runtime's declaration a throw or handler names.
	if (!RuntimeTypeInformation(type))
		return type_information(type);
	const uint32_t found = defined_informations_.find(type);
	if (found != PointerMap::kMissing)
		return found;
	const uint32_t typeid_found = type_informations_.find(type);
	if (typeid_found != PointerMap::kMissing &&
		program_.symbols[typeid_found].kind == lowir::SymbolKind::GlobalDefinition)
		return typeid_found;
	const bool pointer = type->kind == sema::TypeKind::Pointer;
	const lowir::SymbolId pointee = pointer ? defined_type_information(type->element->canonical->unqualified)
											: lowir::kNoId;
	const std::string label = pointer ? "type_" + type_object(type, abi_mangle::ABI_TARGET_TYPE)
									  : FundamentalSpelling(type->fundamental);
	const lowir::SymbolId symbol = new_symbol("__rtti_" + label);
	defined_informations_.set(type, symbol);
	const lowir::SymbolId vtable = rtti_vtable(pointer ? TypeInfoKind::Pointer : TypeInfoKind::Fundamental);
	const lowir::SymbolId name = type_name_object(type, pointer ? label : "_" + label, false);
	const uint32_t first = data_item(AddressItem(vtable, 16));
	data_item(AddressItem(name, 0));
	if (pointer)
	{
		data_item(ScalarItem(lowir::TypeKind::I32, PointeeFlags(type->element)));
		data_item(AddressItem(pointee, 0));
	}
	lowir::SymbolMetadata metadata;
	metadata.binding = ir_model::SBM_INTERNAL;
	define_data(symbol, first, metadata);
	return symbol;
}

lowir::SymbolId ProgramLowering::class_information(const sema::ClassEntity* entity, bool table)
{
	// A class's type information; with `table`, a polymorphic class's
	// table as well where the unit defines it (a run-time check's classes,
	// the course's form; Itanium 5.2.3). A host object defines only what
	// its code refers to (README PA31): a check reads the object's table,
	// which the unit constructing it defines.
	if (table && !host_objects_)
		class_table(entity);
	return typeinfo_symbol(entity);
}

void ProgramLowering::class_table(const sema::ClassEntity* entity)
{
	if (entity != nullptr && entity->dynamic && entity->complete && emits_vtable(entity))
		vtable_symbol(entity);
}

lowir::SymbolId ProgramLowering::type_name_object(const sema::Type* type, const std::string& label, bool internal)
{
	// Itanium 2.9.5p2: the type's encoding (without _Z), null terminated;
	// in object mode `*` first for a type its unit alone names (see
	// private_type_name).
	const std::string object = type_object(type, abi_mangle::ABI_TARGET_TYPEINFO_NAME);
	lowir::SymbolId symbol = lowir::kNoId;
	if (internal)
	{
		symbol = new_symbol("__typeinfo_name_" + label);
		symbol_objects_[symbol] = local_object(object, false);
	}
	else
		symbol = external_symbol(object, "__typeinfo_name_" + label);
	if (program_.symbols[symbol].kind != lowir::SymbolKind::Unresolved)
		return symbol;
	const std::string name = (object_mode_ && internal && private_type_name(type) ? "*" : "") +
		type_object(type, abi_mangle::ABI_TARGET_TYPE);
	const uint32_t first = static_cast<uint32_t>(program_.data_items.size());
	for (const char c : name)
		data_item(ScalarItem(lowir::TypeKind::I8, static_cast<uint8_t>(c)));
	data_item(ScalarItem(lowir::TypeKind::I8, 0));
	lowir::SymbolMetadata metadata;
	metadata.binding = internal ? ir_model::SBM_INTERNAL : ir_model::SBM_WEAK;
	define_data(symbol, first, metadata);
	return symbol;
}

std::string ProgramLowering::type_object(const sema::Type* type, abi_mangle::AbiTargetKind kind)
{
	abi_mangle::AbiTarget target;
	target.kind = kind;
	target.node = abi_type(type);
	std::string out;
	encoder_->encode(target, out);
	++counters_.mangled_names;
	return out;
}

lowir::SymbolId ProgramLowering::rtti_vtable(TypeInfoKind kind)
{
	// The runtime's type information kinds' vtables, one symbol each in the
	// program: the class kinds' and the pointer kind's singleton roles (a
	// standalone runtime matches pointer handlers through the latter,
	// lowir.md), the others' rtti_data.
	const size_t index = static_cast<size_t>(kind);
	if (rtti_vtables_[index] != lowir::kNoId)
		return rtti_vtables_[index];
	static const char* const kNames[] = { "__class_type_info", "__si_class_type_info", "__vmi_class_type_info",
		"__fundamental_type_info", "__pointer_type_info", "__pointer_to_member_type_info", "__function_type_info",
		"__array_type_info", "__enum_type_info" };
	static const char* const kObjects[] = { "_ZTVN10__cxxabiv117__class_type_infoE",
		"_ZTVN10__cxxabiv120__si_class_type_infoE", "_ZTVN10__cxxabiv121__vmi_class_type_infoE",
		"_ZTVN10__cxxabiv123__fundamental_type_infoE", "_ZTVN10__cxxabiv119__pointer_type_infoE",
		"_ZTVN10__cxxabiv129__pointer_to_member_type_infoE", "_ZTVN10__cxxabiv120__function_type_infoE",
		"_ZTVN10__cxxabiv117__array_type_infoE", "_ZTVN10__cxxabiv116__enum_type_infoE" };
	static const ir_model::SymbolRole kRoles[] = { ir_model::SR_RTTI_CLASS, ir_model::SR_RTTI_SI,
		ir_model::SR_RTTI_VMI };
	const lowir::SymbolId symbol = new_symbol(std::string("__external_rtti_vtable__") + kNames[index]);
	lowir::Global global = lowir::Global();
	global.typed = false;
	global.init = lowir::GlobalInit::None;
	program_.declare_global(symbol, lowir::SymbolKind::GlobalDeclaration, global);
	lowir::SymbolMetadata& metadata = program_.symbols[symbol].metadata;
	metadata.role = index < 3           ? kRoles[index]
		: kind == TypeInfoKind::Pointer ? ir_model::SR_RTTI_POINTER
										: ir_model::SR_RTTI_DATA;
	metadata.binding = ir_model::SBM_STRONG;
	metadata.object = intern(kObjects[index], static_cast<uint32_t>(std::strlen(kObjects[index])));
	rtti_vtables_[index] = symbol;
	++counters_.declarations;
	return symbol;
}

} // namespace lower
