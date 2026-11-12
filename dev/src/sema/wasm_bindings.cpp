#include "sema/analyzer.h"

#include <cstring>

#include "sema/string_literals.h"

namespace sema {

SemanticGraph::WasmBinding SemanticGraph::wasm_binding(const FunctionEntity* function) const
{
	WasmBinding result;
	if (wasm_bindings.empty())
		return result;
	for (; function != nullptr; function = function->primary)
	{
		const uint32_t index = wasm_binding_index.find(function);
		if (index == PointerMap::kMissing)
			continue;
		const WasmBinding& binding = wasm_bindings[index];
		if (result.module == nullptr)
			result.module = binding.module;
		if (result.import_name == nullptr)
			result.import_name = binding.import_name;
		if (result.export_name == nullptr)
			result.export_name = binding.export_name;
	}
	return result;
}

SemanticGraph::WasmBinding& SemanticGraph::own_wasm_binding(const Entity* entity)
{
	const uint32_t index = wasm_binding_index.find(entity);
	if (index != PointerMap::kMissing)
		return wasm_bindings[index];
	wasm_binding_index.set(entity, static_cast<uint32_t>(wasm_bindings.size()));
	wasm_bindings.push_back(WasmBinding());
	return wasm_bindings.back();
}

void Analyzer::wasm_binding_attributes(
	Entity* entity, const syntax::Attribute* leading, const syntax::Declarator* declarator)
{
	const auto apply = [&](const syntax::Attribute* list) {
		for (const syntax::Attribute* attribute = list; attribute != nullptr; attribute = attribute->next)
		{
			using syntax::AttributeKind;
			if (attribute->kind != AttributeKind::WasmImportModule &&
				attribute->kind != AttributeKind::WasmImportName && attribute->kind != AttributeKind::WasmExportName)
				continue;
			if (entity->kind != EntityKind::Function)
				error(attribute->location, "a Wasm function binding attribute requires a function");
			FunctionEntity* function = static_cast<FunctionEntity*>(entity);
			if (function->denotes != nullptr)
				function = function->denotes;
			if (function->internal_linkage && !function->defined && attribute->kind != AttributeKind::WasmExportName)
				error(attribute->location, "a Wasm import requires external linkage");
			StringLiteralFacts facts;
			std::vector<uint64_t> units;
			if (attribute->expression == nullptr || attribute->expression->kind != syntax::ExprKind::Literal ||
				!StringLiteralUnits(
					static_cast<const syntax::LiteralExpr*>(attribute->expression)->spelling, facts, units) ||
				facts.element != FundamentalType::Char || facts.suffix_length != 0 || facts.numeric_escapes)
				error(attribute->location, "a Wasm binding name requires an ordinary string literal");
			std::string name;
			for (size_t i = 0; i + 1 < units.size(); ++i)
			{
				if (units[i] == 0)
					error(attribute->location, "null byte in Wasm binding name");
				name.push_back(static_cast<char>(units[i]));
			}
			SemanticGraph::WasmBinding& binding = graph_.own_wasm_binding(function);
			const char*& field = attribute->kind == AttributeKind::WasmImportModule ? binding.module
				: attribute->kind == AttributeKind::WasmImportName                  ? binding.import_name
																					: binding.export_name;
			if (field != nullptr && name != field)
				error(attribute->location, "conflicting Wasm function binding declarations");
			if (field == nullptr)
				field = copy_label(name.data(), name.size());
		}
	};
	apply(leading);
	for (const syntax::Declarator* part = declarator; part != nullptr; part = part->nested)
		apply(part->attributes);
}

} // namespace sema
