#pragma once

#include <cstddef>
#include <vector>

namespace nsinit_image {

struct ImageSymbol;

struct Relocation
{
  std::size_t byte_offset;
  ImageSymbol * target;
  std::size_t addend;
};

struct ImageSymbol
{
  std::vector<char> bytes;
  std::vector<Relocation> relocations;
  std::size_t alignment = 1;
  std::size_t offset = 0;
};

std::size_t align_up(std::size_t value, std::size_t alignment);
void append_zero_bytes(std::vector<char> & out, std::size_t count);
std::vector<char> little_endian_bytes(unsigned long long value, std::size_t size);
void layout_symbol(std::vector<char> & image, ImageSymbol & symbol);
void apply_symbol_relocations(std::vector<char> & image, const ImageSymbol & symbol);

}  // namespace nsinit_image
