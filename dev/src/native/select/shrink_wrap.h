#pragma once

#include "native/mir/mir.h"

namespace native {

// O1+, before branch layout: a frame-pointer-free function whose entry
// reaches returns without needing its frame runs that region frameless
// and sets the frame up on each edge out of it (shrink_wrap.cpp).
// Returns whether it did.
bool shrink_wrap(Function& function);

} // namespace native
