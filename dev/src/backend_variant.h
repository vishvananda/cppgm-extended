#pragma once
#include <string>

// Design variants of the course solution.  Each is a correct backend that
// differs from the default in a choice the course contract leaves open; the
// course suites run under every variant (`make test-variants`) so that a
// fixture which encodes the default's shape rather than the contract is
// caught before it merges.  A variant is selected with the
// `--backend-variant <name>` option of cppgm++, lowiropt and lowir2native;
// the course harness passes it from CPPGM_BACKEND_VARIANT.
//
//   pool-reverse   the reactive and planned callee-saved register pools in
//                  the opposite order
//   frame-pad      every frame begins with sixteen bytes of padding
//   rpo-layout     the optimizer serializes every function in reverse
//                  postorder, undoing cold-block sinking
//   colouring      planned register assignment by interference-graph
//                  colouring instead of the claim-driven linear scan; no
//                  local-phi or cyclic-region plans
//   linear-scan    the newcomer's allocator, written from the PA38 README
//                  and planning_seam.h alone (linear_scan.cpp)
namespace cppgm_variant {

inline std::string & selection()
{
  static std::string name;
  return name;
}

// Called once per invocation while its options are read; an invocation
// without the option selects the default (the empty name).
inline void select(const std::string & name)
{
  selection() = name;
}

inline bool selected(const char * name)
{
  return selection() == name;
}

}  // namespace cppgm_variant
