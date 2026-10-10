#pragma once

#include <cstdint>
#include <string>
#include <vector>

#include "lowir/program.h"
#include "support/flat_map.h"

namespace lowir {

// Spells the presentation names of one function body's locals at a time
// (lowir/local_kinds.h), for the writer.
class LocalNames
{
public:
	explicit LocalNames(const Program& program) : program_(program), body_(nullptr) {}

	// The body whose locals are spelled next (its parameters print as a
	// definition's: unnamed ones as `__param<i>`).
	void reset(const FunctionBody& body);

	// Appends a local's spelling, without its sigil.
	void temp(std::string& out, TempId temp);
	void slot(std::string& out, SlotId slot);
	void block(std::string& out, BlockId block);
	// A declaration's or call boundary's parameter (unnamed: `arg<i>`);
	// independent of the body being spelled.
	void boundary_parameter(std::string& out, const Signature& signature, uint32_t index);

private:
	struct NameHash
	{
		size_t operator()(NameId name) const { return name; }
	};
	// The explicit names a generated spelling could equal (those ending in
	// a digit or an underscore), if any: the temporaries' and slots'
	// together (the course's generated names skip any source name), the
	// blocks'.
	struct Taken
	{
		bool any = false;
		FlatMap<NameId, uint8_t, NameHash> names;
	};

	void collect(Taken& taken, NameId name);
	bool taken(Taken& taken, const std::string& spelling);
	// Appends `scratch_` to `out`, made free of `taken`'s names.
	void unique(std::string& out, Taken& taken);
	void ordinals(const FunctionBody& body);

	const Program& program_;
	const FunctionBody* body_;
	// Per slot: the ordinal a generated, shadow or range-for kind prints;
	// per temporary after the parameters, when an explicit name is one a
	// temporary would print (`t<n>`), the ordinal skipping it.
	std::vector<uint32_t> slot_ordinals_;
	std::vector<uint32_t> temp_ordinals_;
	Taken values_;
	Taken blocks_;
	std::string scratch_;
};

} // namespace lowir
