#pragma once

#include "semantic/model/graph.h"

namespace cppgm
{
namespace semantic
{

struct GraphStorage
{
	InternedStringTable strings;
	Program program;
	DumpArena dump;
	std::vector<NamespaceObjectAction> namespace_objects;
	std::vector<LocalStaticObjectAction> local_static_objects;
	std::vector<AggregateHelperInfo> aggregate_helpers;
	std::vector<ClassPolymorphismFacts> class_polymorphism;
	std::vector<ConstexprAddressValue> constant_addresses;
	std::vector<ConstexprObjectValue> constant_objects;
	std::vector<ConstexprObjectElement> constant_object_elements;
	std::uint32_t root;

	GraphStorage()
		: strings(), program(strings), root(kNoDumpEdge) {}
	SemanticGraphView View() const;
};

}
}
