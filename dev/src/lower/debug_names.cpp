#include "lower/debug_info.h"

#include "lex/identifier_table.h"
#include "sema/integral_arithmetic.h"
#include "sema/semantic_graph.h"
#include "sema/template_records.h"

namespace lower {

void DebugInfo::finish(const sema::Scope* root)
{
	std::vector<const sema::Scope*> pending;
	if (root != nullptr)
		pending.push_back(root);
	while (!pending.empty())
	{
		const sema::Scope* current = pending.back();
		pending.pop_back();
		for (const sema::Scope* child = current->children; child != nullptr; child = child->next_sibling)
			pending.push_back(child);
		if (current->kind != sema::ScopeKind::Namespace && current->kind != sema::ScopeKind::Class)
			continue;
		if (current->kind == sema::ScopeKind::Class &&
			(current->entity == nullptr || types_.find(current->entity->type) == PointerMap::kMissing))
			continue;
		for (const sema::Binding* binding = current->bindings; binding != nullptr; binding = binding->next)
		{
			const sema::Entity* entity = binding->entity;
			if (entity == nullptr || entity->kind != sema::EntityKind::Typedef || entity->name == nullptr ||
				aliases_.find(entity) != PointerMap::kMissing)
				continue;
			const uint32_t referenced = types_.find(entity->type);
			if (referenced == PointerMap::kMissing)
				continue;
			lowir::DebugType alias;
			alias.kind = lowir::DebugKind::Alias;
			alias.parent = scope(entity->owner);
			alias.name = intern(entity->name->spelling);
			alias.type = referenced;
			aliases_.set(entity, add(alias));
		}
	}
}

std::string DebugInfo::entity_name(const sema::Entity* entity, bool qualified)
{
	std::string result;
	const sema::Scope* owner = entity->owner;
	if (qualified)
	{
		while (owner != nullptr && owner->kind != sema::ScopeKind::Namespace && owner->kind != sema::ScopeKind::Class)
			owner = owner->parent;
		if (owner != nullptr && owner->entity != nullptr && owner->entity->name != nullptr)
			result = entity_name(owner->entity, true) + "::";
	}
	if (entity->name != nullptr)
		result += entity->name->spelling;
	const sema::ClassEntity* cls = entity->kind == sema::EntityKind::Class
		? static_cast<const sema::ClassEntity*>(entity)
		: nullptr;
	if (cls != nullptr && cls->templ != nullptr && cls->templ->arguments != nullptr)
	{
		result += '<';
		for (uint32_t i = 0; i < cls->templ->argument_count; ++i)
		{
			if (i != 0)
				result += ", ";
			result += name(cls->templ->arguments[i], true);
		}
		result += '>';
	}
	return result;
}

std::string DebugInfo::name(const sema::Type* source, bool qualified)
{
	if (source == nullptr)
		return "void";
	if (source->cv != 0)
	{
		const std::string qualifiers = std::string((source->cv & sema::kConst) != 0 ? " const" : "") +
			((source->cv & sema::kVolatile) != 0 ? " volatile" : "");
		return source->kind == sema::TypeKind::Pointer
			? name(source->unqualified, qualified) + qualifiers
			: qualifiers.substr(1) + " " + name(source->unqualified, qualified);
	}
	switch (source->kind)
	{
	case sema::TypeKind::Fundamental:
		return FundamentalTypeName(source->fundamental);
	case sema::TypeKind::Class:
	case sema::TypeKind::Enum:
		return entity_name(source->entity, qualified);
	case sema::TypeKind::Pointer:
		return name(source->element, qualified) + '*';
	case sema::TypeKind::LValueReference:
		return name(source->element, qualified) + '&';
	case sema::TypeKind::RValueReference:
		return name(source->element, qualified) + "&&";
	case sema::TypeKind::Array:
		return name(source->element, qualified) + '[' + std::to_string(source->bound) + ']';
	case sema::TypeKind::Pack:
	{
		std::string result;
		for (uint32_t i = 0; i < source->param_count; ++i)
		{
			if (i != 0)
				result += ", ";
			result += name(source->params[i], qualified);
		}
		return result;
	}
	case sema::TypeKind::Value:
		if (source->entity != nullptr)
			return (source->element->is_reference() ? "" : "&") + entity_name(source->entity, true);
		if (source->element != nullptr && source->element->kind == sema::TypeKind::Pointer)
			return "nullptr";
		if (source->element != nullptr && source->element->kind == sema::TypeKind::Fundamental &&
			source->element->fundamental == FundamentalType::Bool)
			return source->bound != 0 ? "true" : "false";
		return sema::IsSignedConstant(sema::Constant{ source->bound, source->element })
			? std::to_string(static_cast<int64_t>(source->bound))
			: std::to_string(source->bound);
	default:
		return source->entity != nullptr ? entity_name(source->entity, qualified) : "";
	}
}

void DebugInfo::members(uint32_t id, const sema::ClassEntity* entity)
{
	if (entity->polymorphic)
	{
		lowir::DebugType function;
		function.kind = lowir::DebugKind::Subroutine;
		uint32_t pointee = add(function);
		for (unsigned level = 0; level < 2; ++level)
		{
			lowir::DebugType pointer;
			pointer.kind = lowir::DebugKind::Pointer;
			pointer.type = pointee;
			pointer.bytes = 8;
			pointee = add(pointer);
		}
		lowir::DebugType vptr;
		vptr.kind = lowir::DebugKind::Member;
		vptr.parent = id;
		vptr.name = intern("_vptr." + entity_name(entity, false));
		vptr.type = pointee;
		vptr.flags = lowir::DF_ARTIFICIAL;
		add(vptr);
	}
	for (const sema::DataMember* member = entity->members; member != nullptr; member = member->next)
	{
		if (member->bit_field && member->width == 0)
			continue;
		lowir::DebugType child;
		child.kind = lowir::DebugKind::Member;
		child.parent = id;
		child.name = member->variable->name != nullptr ? intern(member->variable->name->spelling) : 0;
		child.type = type(member->variable->type);
		child.bit_offset = member->offset;
		if (member->bit_field)
		{
			child.flags |= lowir::DF_BIT_FIELD;
			child.count = member->width;
		}
		add(child);
	}
	for (const sema::BaseClass* base = entity->bases; base != nullptr; base = base->next)
	{
		if (base->dependent || base->is_virtual)
			continue;
		lowir::DebugType child;
		child.kind = lowir::DebugKind::Inheritance;
		child.parent = id;
		child.type = type(base->base->type);
		child.bit_offset = base->offset * 8;
		add(child);
	}
	for (const sema::Binding* binding = entity->scope != nullptr ? entity->scope->bindings : nullptr;
		binding != nullptr; binding = binding->next)
	{
		if (binding->entity != nullptr && binding->entity->kind == sema::EntityKind::Function)
		{
			const sema::FunctionEntity* function = static_cast<const sema::FunctionEntity*>(binding->entity);
			if (!function->is_virtual || function->owner != entity->scope)
				continue;
			lowir::DebugType child;
			child.kind = lowir::DebugKind::Method;
			child.parent = id;
			child.flags = lowir::DF_DECLARATION | lowir::DF_VIRTUAL_METHOD;
			child.name = intern(function->destructor ? "~" + entity_name(entity, false)
					: function->name != nullptr      ? function->name->spelling
													 : "");
			child.type = type(function->type->element);
			const uint32_t method = add(child);
			for (uint32_t i = 0; i < function->type->param_count; ++i)
			{
				lowir::DebugType parameter;
				parameter.kind = lowir::DebugKind::Parameter;
				parameter.parent = method;
				parameter.type = type(function->type->params[i]);
				add(parameter);
			}
			continue;
		}
		if (binding->entity == nullptr || binding->entity->kind != sema::EntityKind::Variable)
			continue;
		const sema::VariableEntity* variable = static_cast<const sema::VariableEntity*>(binding->entity);
		if (!variable->is_static || variable->constant != sema::ConstantState::Value)
			continue;
		lowir::DebugType child;
		child.kind = lowir::DebugKind::Constant;
		child.parent = id;
		child.name = variable->name != nullptr ? intern(variable->name->spelling) : 0;
		child.type = type(variable->type);
		child.constant = variable->value.bits;
		add(child);
	}
}

} // namespace lower
