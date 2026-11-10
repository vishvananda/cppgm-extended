#pragma once

// C++14 permits a deleted declaration with a placeholder return type.
// The viable overload still has a usable declared return type.
inline constexpr auto blocked(long double) = delete;

inline constexpr int blocked(int value)
{
  return value;
}
