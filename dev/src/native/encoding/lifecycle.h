#pragma once
#include "lowir/model/program.h"
#include "native/mir/model.h"
namespace lowir_native {
namespace elf_detail { class CodeBuffer; }
namespace lifecycle_detail {
void plan_runtime(const lowir_model::LowirProgram& source, mir_model::MirProgram* target);
bool has_runtime(const mir_model::MirProgram& program);
void emit_registration(elf_detail::CodeBuffer& out);
void emit_drain(elf_detail::CodeBuffer& out);
void emit_data(elf_detail::CodeBuffer& out);
}
}
