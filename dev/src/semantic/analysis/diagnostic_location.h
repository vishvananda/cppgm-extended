#pragma once

#include "syntax/model/arena.h"

namespace cppgm
{
namespace semantic
{

// The syntax node the analyzer is currently working on, published so that the
// semantic throw helpers can name a source location without every one of the
// throw sites having to carry a node.  Only the error path reads it; the guard
// costs two stores and two restores.
extern const syntax::SyntaxArena* diagnostic_location_arena;
extern syntax::NodeId diagnostic_location_node;

std::string DescribeDiagnosticLocation();

class ScopedDiagnosticLocation
{
public:
	// A node without a source range would otherwise erase a located outer one
	// and leave the diagnostic with nothing to point at, so only a node that
	// can actually name a place replaces what is already published.
	ScopedDiagnosticLocation(const syntax::SyntaxArena* arena,
		syntax::NodeId node)
		: arena_(diagnostic_location_arena), node_(diagnostic_location_node)
	{
		if (arena == 0 || node == syntax::kNoNode ||
			!arena->HasSourceLocation(node)) return;
		diagnostic_location_arena = arena;
		diagnostic_location_node = node;
	}
	~ScopedDiagnosticLocation()
	{
		diagnostic_location_arena = arena_;
		diagnostic_location_node = node_;
	}

private:
	ScopedDiagnosticLocation(const ScopedDiagnosticLocation&);
	ScopedDiagnosticLocation& operator=(const ScopedDiagnosticLocation&);
	const syntax::SyntaxArena* arena_;
	syntax::NodeId node_;
};

}
}
