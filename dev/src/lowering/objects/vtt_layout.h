#pragma once

#include "semantic/model/graph.h"
#include "semantic/model/program.h"

#include <cstdint>
#include <limits>
#include <vector>

namespace cppgm
{
namespace lowering
{

const std::uint32_t kNoVttIndex = std::numeric_limits<std::uint32_t>::max();

// A class's VTT shape (Itanium C++ ABI 2.6.2), which does not depend on the
// complete class it is a sub-VTT of: entry 0 holds the primary vtable; then
// each direct non-virtual base with virtual bases has a sub-VTT (its own
// shape without virtual sub-VTTs) in declaration order; then the secondary
// virtual pointers, one for each dynamic base subobject that has virtual
// bases or lies in a virtual base and is not a non-virtual primary base, in
// inheritance graph order (a virtual base where the walk first meets it).
// A complete class's VTT then ends with a sub-VTT for each virtual base that
// has virtual bases, in inheritance graph order.
struct ClassVttLayout
{
	bool computed;
	std::uint32_t size, complete_size;
	// Per direct base ordinal: the start of its sub-VTT.
	std::vector<std::uint32_t> base_index;
	// Per polymorphic view: its secondary virtual pointer's index.
	std::vector<std::uint32_t> view_index;
	// The views holding secondary virtual pointers, in VTT order.
	std::vector<std::uint32_t> secondary_views;
	// Per virtual base ordinal: the start of its sub-VTT in a complete VTT.
	std::vector<std::uint32_t> virtual_index;

	ClassVttLayout() : computed(false), size(0), complete_size(0) {}
};

// Computes VTT shapes on demand, each once.
class VttLayoutBuilder
{
public:
	VttLayoutBuilder(const semantic::Program& program,
		const std::vector<semantic::ClassPolymorphismFacts>& facts,
		std::vector<ClassVttLayout>* layouts)
		: program_(program), facts_(facts), layouts_(*layouts) {}

	const ClassVttLayout& Layout(semantic::EntityId entity);

private:
	void WalkSecondaryPointers(semantic::EntityId root,
		semantic::EntityId current, std::uint64_t offset, bool morally_virtual,
		std::vector<semantic::EntityId>* visited,
		std::vector<std::uint32_t>* views);

	const semantic::Program& program_;
	const std::vector<semantic::ClassPolymorphismFacts>& facts_;
	std::vector<ClassVttLayout>& layouts_;
};

}
}
