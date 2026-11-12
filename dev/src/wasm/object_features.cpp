#include "wasm/object_reader.h"

#include <set>

namespace wasm {

void validate_link_features(const std::vector<Object>& objects, const std::vector<bool>& active)
{
	std::set<std::string> used, forbidden;
	for (uint32_t i = 0; i < objects.size(); ++i)
		if (active[i])
			for (const auto& feature : objects[i].features)
				(feature.first == '+' ? used : forbidden).insert(feature.second);
	for (auto* features : { &used, &forbidden })
	{
		if (features->count("bulk-memory"))
			features->insert("bulk-memory-opt");
		if (features->count("reference-types"))
			features->insert("call-indirect-overlong");
	}
	for (const auto& feature : used)
		if (forbidden.count(feature))
			throw WasmError("conflicting Wasm object feature: " + feature);
}

} // namespace wasm
