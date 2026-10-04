#pragma once

// Shared hash mixing for numeric ABI graph and substitution keys.  Each key
// owner remains responsible for selecting and ordering its encoded fields.

#include <cstddef>
#include <functional>
#include <vector>

namespace abi_mangle {
namespace detail {

inline std::size_t mix_hash(std::size_t seed, std::size_t value)
{
  return seed ^ (value + static_cast<std::size_t>(0x9e3779b9U) +
                 (seed << 6) + (seed >> 2));
}

template<class T>
std::size_t vector_hash(std::size_t seed, const std::vector<T> & values)
{
  for(const T & value : values) seed = mix_hash(seed, std::hash<T>()(value));
  return seed;
}

}  // namespace detail
}  // namespace abi_mangle
