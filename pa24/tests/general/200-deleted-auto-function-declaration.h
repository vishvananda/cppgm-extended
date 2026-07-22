#pragma once

inline constexpr auto blocked(long double) = delete;

inline constexpr int blocked(int value)
{
  return value;
}
