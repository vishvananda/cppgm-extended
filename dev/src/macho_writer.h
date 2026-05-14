#pragma once

#include <cstdint>
#include <string>
#include <vector>

void write_macho_x86_64_executable(const std::string & outfile,
                                   const std::vector<unsigned char> & payload,
                                   std::uint64_t entry_offset,
                                   std::uint64_t base_vaddr);
