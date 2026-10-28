#include "lower/debug_info.h"

#include "lex/identifier_table.h"
#include "lower/program_lowering.h"
#include "sema/integral_arithmetic.h"
#include "sema/template_records.h"

namespace lower {

void ProgramLowering::debug_local(lowir::SymbolId function, lowir::SlotId slot, const sema::VariableEntity* variable,
	uint32_t location, uint32_t lexical_scope)
{
	if (source_debug_)
		source_debug_->local(function, slot, variable, location, lexical_scope);
}

void ProgramLowering::debug_function(const sema::FunctionEntity* function)
{
	if (source_debug_ && function != nullptr && function->owner != nullptr &&
		function->owner->kind == sema::ScopeKind::Class)
		source_debug_->type(function->owner->entity->type);
}

DebugInfo::DebugInfo(ProgramLowering& lowering) : lowering_(lowering), info_(*lowering.program().debug)
{}

lowir::NameId DebugInfo::intern(const std::string& text)
{
	return text.empty() ? 0 : lowering_.intern(text.data(), static_cast<uint32_t>(text.size()));
}

uint32_t DebugInfo::add(const lowir::DebugType& record)
{
	const uint32_t id = static_cast<uint32_t>(info_.types.size());
	info_.types.push_back(record);
	return id;
}

uint32_t DebugInfo::scope(const sema::Scope* owner)
{
	if (owner == nullptr)
		return 0;
	if (owner->kind == sema::ScopeKind::Class && owner->entity != nullptr)
		return type(owner->entity->type);
	if (owner->kind != sema::ScopeKind::Namespace || owner->entity == nullptr || owner->entity->name == nullptr)
		return scope(owner->parent);
	const uint32_t found = scopes_.find(owner);
	if (found != PointerMap::kMissing)
		return found;
	lowir::DebugType record;
	record.kind = lowir::DebugKind::Namespace;
	record.name = intern(owner->entity->name->spelling);
	record.parent = scope(owner->parent);
	const uint32_t id = add(record);
	scopes_.set(owner, id);
	return id;
}

void DebugInfo::local(lowir::SymbolId function, lowir::SlotId slot, const sema::VariableEntity* variable,
	uint32_t location, uint32_t lexical_scope)
{
	if (slot == lowir::kNoId || variable->name == nullptr)
		return;
	const uint32_t id = type(variable->type);
	if (function >= info_.functions.size())
		info_.functions.resize(function + 1);
	info_.functions[function].locals.push_back(lowir::DebugLocal{
		slot, id, intern(variable->name->spelling), lowering_.debug_location(location), lexical_scope });
}

uint32_t DebugInfo::type(const sema::Type* source)
{
	if (source == nullptr)
		return 0;
	const uint32_t found = types_.find(source);
	if (found != PointerMap::kMissing)
	{
		if (source->kind == sema::TypeKind::Class && source->cv == 0 &&
			(info_.types[found].flags & lowir::DF_DECLARATION) != 0)
		{
			const sema::ClassEntity* entity = static_cast<const sema::ClassEntity*>(source->entity);
			if (entity->layout == sema::FactState::Done)
			{
				info_.types[found].flags &= ~lowir::DF_DECLARATION;
				info_.types[found].bytes = entity->size;
				if (entity->polymorphic)
					info_.types[found].flags |= lowir::DF_POLYMORPHIC;
				members(found, entity);
			}
		}
		return found;
	}
	const uint32_t id = add(lowir::DebugType());
	types_.set(source, id);
	lowir::DebugType record;
	if (source->cv != 0)
	{
		record.type = type(source->unqualified);
		record.kind = (source->cv & sema::kConst) != 0 ? lowir::DebugKind::Const
			: (source->cv & sema::kVolatile) != 0      ? lowir::DebugKind::Volatile
													   : lowir::DebugKind::Alias;
		if (record.kind == lowir::DebugKind::Alias)
			record.name = intern("_Atomic(" + name(source->unqualified) + ")");
		if ((source->cv & (sema::kConst | sema::kVolatile)) == (sema::kConst | sema::kVolatile))
		{
			lowir::DebugType qualified;
			qualified.kind = lowir::DebugKind::Volatile;
			qualified.type = record.type;
			record.type = add(qualified);
		}
	}
	else
		switch (source->kind)
		{
		case sema::TypeKind::Fundamental:
		{
			const FundamentalType fundamental = source->fundamental;
			record.kind = lowir::DebugKind::Base;
			record.name = intern(FundamentalTypeName(fundamental));
			record.bytes = FundamentalTypeSize(fundamental);
			record.encoding = IsFloatingFundamental(fundamental)      ? lowir::DebugEncoding::Floating
				: fundamental == FundamentalType::Bool                ? lowir::DebugEncoding::Boolean
				: fundamental == FundamentalType::NullptrT            ? lowir::DebugEncoding::Address
				: sema::IsSignedConstant(sema::Constant{ 0, source }) ? lowir::DebugEncoding::Signed
																	  : lowir::DebugEncoding::Unsigned;
			if ((fundamental == FundamentalType::Char && !source->plain_char_unsigned) ||
				fundamental == FundamentalType::SignedChar)
				record.encoding = lowir::DebugEncoding::SignedChar;
			if (fundamental == FundamentalType::UnsignedChar || source->plain_char_unsigned)
				record.encoding = lowir::DebugEncoding::UnsignedChar;
			if (fundamental == FundamentalType::Void)
				record.kind = lowir::DebugKind::Unspecified;
			break;
		}
		case sema::TypeKind::Pointer:
		case sema::TypeKind::LValueReference:
		case sema::TypeKind::RValueReference:
			record.kind = source->kind == sema::TypeKind::Pointer ? lowir::DebugKind::Pointer
				: source->kind == sema::TypeKind::LValueReference ? lowir::DebugKind::Reference
																  : lowir::DebugKind::RvalueReference;
			record.bytes = 8;
			record.type = type(source->element);
			break;
		case sema::TypeKind::Array:
			record.kind = lowir::DebugKind::Array;
			record.type = type(source->element);
			record.count = source->bound;
			break;
		case sema::TypeKind::Class:
		{
			const sema::ClassEntity* entity = static_cast<const sema::ClassEntity*>(source->entity);
			record.kind = entity->key == TokenKind::KW_UNION ? lowir::DebugKind::Union : lowir::DebugKind::Structure;
			record.name = intern(entity_name(entity, false));
			record.parent = scope(entity->owner);
			if (entity->layout == sema::FactState::Done)
			{
				record.bytes = entity->size;
				if (entity->polymorphic)
					record.flags |= lowir::DF_POLYMORPHIC;
				members(id, entity);
			}
			else
				record.flags |= lowir::DF_DECLARATION;
			break;
		}
		case sema::TypeKind::Enum:
		{
			const sema::EnumEntity* entity = static_cast<const sema::EnumEntity*>(source->entity);
			record.kind = lowir::DebugKind::Enumeration;
			if (entity->scoped)
				record.flags |= lowir::DF_SCOPED_ENUM;
			record.name = intern(entity_name(entity, false));
			record.parent = scope(entity->owner);
			record.type = type(entity->underlying);
			record.bytes = entity->underlying != nullptr ? FundamentalTypeSize(entity->underlying->fundamental) : 0;
			for (const sema::Binding* binding = entity->scope != nullptr ? entity->scope->bindings : nullptr;
				binding != nullptr; binding = binding->next)
			{
				if (binding->entity == nullptr || binding->entity->kind != sema::EntityKind::Enumerator)
					continue;
				const sema::EnumeratorEntity* value = static_cast<const sema::EnumeratorEntity*>(binding->entity);
				if (value->enumeration != entity)
					continue;
				lowir::DebugType child;
				child.kind = lowir::DebugKind::Enumerator;
				child.parent = id;
				child.name = value->name != nullptr ? intern(value->name->spelling) : 0;
				child.constant = value->value.bits;
				add(child);
			}
			break;
		}
		case sema::TypeKind::Function:
			record.kind = lowir::DebugKind::Subroutine;
			record.type = type(source->element);
			for (uint32_t i = 0; i < source->param_count; ++i)
			{
				lowir::DebugType child;
				child.kind = lowir::DebugKind::Parameter;
				child.parent = id;
				child.type = type(source->params[i]);
				add(child);
			}
			break;
		default:
			record.name = intern(name(source));
			break;
		}
	info_.types[id] = record;
	return id;
}

} // namespace lower
