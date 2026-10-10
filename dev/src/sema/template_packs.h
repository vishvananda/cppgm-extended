#pragma once

#include <utility>
#include <vector>

#include "sema/analyzer.h"
#include "sema/instantiation_context.h"

namespace sema {

// Any depth, for Analyzer::unexpanded_packs.
const uint32_t kAnyDepth = 0xffffffffu;

// The parameter packs an instantiation's copies expand (PA15, see
// syntax::PackExpander): the template parameter packs the instantiation
// frames on a scope's lookup chain bind to argument packs, each element
// named by Analyzer::pack_element_name. `get` is null when there are none,
// so a copy for a pack-free instantiation does no pack work.
class Analyzer::FrameExpander : public syntax::PackExpander
{
public:
	FrameExpander(Analyzer& analyzer, const Scope* scope);

	syntax::PackExpander* get() { return packs_.empty() ? nullptr : this; }
	void add(const Identifier* name, uint32_t count) { packs_.push_back(std::make_pair(name, count)); }

	int32_t pack_length(const Identifier* name) override;
	const Identifier* element_name(const Identifier* name, uint32_t index) override;
	void mismatch(uint32_t location) override;

private:
	Analyzer& analyzer_;
	std::vector<std::pair<const Identifier*, uint32_t>> packs_;
};

} // namespace sema
