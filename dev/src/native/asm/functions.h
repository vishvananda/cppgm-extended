#pragma once

#include "lowir/assembly.h"

namespace native {
class Image;

// Raw functions own their entire ABI. No frame, register setup or unwind
// promise is inferred from their opaque body.
void emit_assembly_function(Image& image, const lowir::Program& program, const lowir::AssemblyFunction& function);

} // namespace native
