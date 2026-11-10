#pragma once

#include "lowir/program.h"

namespace opt {

// Exact pointer derivations and a conservative capture proof for fresh
// results. Valid while pointer definitions and their uses remain unchanged;
// moving a load or adding a preheader does not invalidate it.
class AllocationOrigins
{
public:
	AllocationOrigins(const lowir::Program& program, const lowir::FunctionBody& body);
	bool apart(lowir::Value a, lowir::Value b) const;

private:
	lowir::TempId root(lowir::Value value) const;
	std::vector<lowir::TempId> roots_;
	std::vector<uint8_t> captured_;
};

} // namespace opt
