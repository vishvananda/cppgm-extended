#pragma once

#include <cstdint>
#include <string>
#include <vector>

#include "native/object/object_file.h"

// The ELF64 x86-64 relocatable (ET_REL) encoding of an ObjectFile.
namespace native {
namespace object {

// Encodes `object`: the null section, its COMDAT groups (SHT_GROUP before
// their members, as ELF requires), its sections, their SHT_RELA tables,
// the symbol table (locals first), the string tables and an empty
// .note.GNU-stack.
void encode_elf_object(const ObjectFile& object, std::vector<uint8_t>& out);
bool write_elf_object(const ObjectFile& object, const std::string& path, std::string& error);

// Decodes an x86-64 ET_REL image into `object`. Sections a static
// executable never loads (notes, unwind tables, debug and comment
// sections) are kept as SectionKind::Null with their relocations dropped.
// Returns false with `error` set for anything else outside the linkable
// subset (other machines or types, REL tables, thread-local storage,
// indirect functions, extended section indexes) or a malformed image.
bool decode_elf_object(const std::vector<uint8_t>& image, ObjectFile& object, std::string& error);
bool read_elf_object(const std::string& path, ObjectFile& object, std::string& error);

// Whether a file starts with the ELF magic (an object input in link mode).
bool is_elf_image(const std::vector<uint8_t>& image);

} // namespace object
} // namespace native
