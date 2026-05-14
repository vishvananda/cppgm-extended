#include "nsinit_image.h"

namespace nsinit_image {

std::size_t align_up(std::size_t value, std::size_t alignment)
{
  if(alignment <= 1) {
    return value;
  }
  const std::size_t rem = value % alignment;
  if(rem == 0) {
    return value;
  }
  return value + (alignment - rem);
}

void append_zero_bytes(std::vector<char> & out, std::size_t count)
{
  out.insert(out.end(), count, '\0');
}

std::vector<char> little_endian_bytes(unsigned long long value, std::size_t size)
{
  std::vector<char> result(size, '\0');
  for(std::size_t i = 0; i < size; ++i) {
    result[i] = static_cast<char>((value >> (i * 8)) & 0xffU);
  }
  return result;
}

void layout_symbol(std::vector<char> & image, ImageSymbol & symbol)
{
  const std::size_t aligned = align_up(image.size(), symbol.alignment);
  append_zero_bytes(image, aligned - image.size());
  symbol.offset = image.size();
  image.insert(image.end(), symbol.bytes.begin(), symbol.bytes.end());
}

void apply_symbol_relocations(std::vector<char> & image,
                              const ImageSymbol & symbol)
{
  for(std::size_t i = 0; i < symbol.relocations.size(); ++i) {
    const Relocation & relocation = symbol.relocations[i];
    const unsigned long long absolute =
        relocation.target->offset + relocation.addend;
    const std::vector<char> encoded = little_endian_bytes(absolute, 8);
    const std::size_t image_offset = symbol.offset + relocation.byte_offset;
    for(std::size_t j = 0; j < encoded.size(); ++j) {
      image[image_offset + j] = encoded[j];
    }
  }
}

}  // namespace nsinit_image
