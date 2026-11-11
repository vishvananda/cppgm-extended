#pragma once

#include "wasm/object.h"
#include "wasm/object_cursor.h"

namespace wasm {

void read_object_bytes(const Bytes& bytes, Object& object);
void read_linking(Cursor& input, Object& object);
void read_relocations(Cursor& input, Object& object, uint8_t section);
void validate_object_code(const Object& object);
void validate_object(const Object& object);

} // namespace wasm
