#pragma once

#include "native/object/object_file.h"

// Direct x86-64 Mach-O encoding, adapted from v3's machine_object.cpp.
// The backend's section/symbol/explicit-addend model stays shared with ELF;
// this writer maps it to Mach-O sections, nlist_64 and inline addends.
namespace native {
namespace object {

bool encode_macho_object(const ObjectFile& object, std::vector<uint8_t>& out, std::string& error);
bool write_macho_object(const ObjectFile& object, const std::string& path, std::string& error);
bool read_macho_object(const std::string& path, ObjectFile& object, std::string& error);

} // namespace object
} // namespace native
