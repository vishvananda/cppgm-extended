#pragma once

#include "wasm/object.h"
#include "wasm/object_cursor.h"

namespace wasm {

Bytes read_link_file(const std::string& path);
void read_object_bytes(const Bytes& bytes, Object& object);
void read_linking(Cursor& input, Object& object);
void read_relocations(Cursor& input, Object& object, uint8_t section);
void validate_object_code(const Object& object);
void validate_object(const Object& object);
ValueType read_object_type(Cursor& input);
void read_global_definitions(Cursor& input, Object& object);
void read_object_elements(Cursor& input, Object& object);
ObjectChunk read_constant_expression(Cursor& input);
void decode_object_chunk(const ObjectChunk& chunk, const Signature& signature, const Object& object,
	const std::vector<uint32_t>& globals, Function& function, bool expression = false, bool declaration = false);
void validate_constant_function(const Function& function, ValueType type, uint32_t globals, const Module& module);
void validate_link_features(const std::vector<Object>& objects, const std::vector<bool>& active);
void read_object_customs(
	const std::vector<std::pair<std::string, Bytes>>& customs, const std::vector<uint8_t>& sections, Object& object);

} // namespace wasm
