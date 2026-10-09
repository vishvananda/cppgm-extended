#pragma once

#include "native/mir/model.h"
#include "native/object/code_buffer.h"

#include <string>
#include <vector>

namespace lowir_native {
namespace eh {

// The standalone runtime's handler matching (15.3p3), called with the
// thrown type_info in rdi, the handler's in rsi and the exception object in
// rdx. It returns the handler's adjusted value in rax (a class subobject,
// the converted pointer, or the exception object) and 1 or 0 in rdx.
extern const char kCatchMatcher[];

void emit_catch_matcher(elf_detail::CodeBuffer & out,
                        const std::vector<mir_model::MirRuntimeData> & data);
// Storage for a runtime type declaration no definition supplies. A
// type_info ("_ZTI" + mangling) keeps its name, which the matcher reads.
void emit_runtime_data_placeholder(elf_detail::CodeBuffer & out,
                                   const std::string & object_name);

}  // namespace eh
}  // namespace lowir_native
