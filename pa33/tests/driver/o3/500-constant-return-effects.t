// Constant results may simplify a caller's guard, but never erase effects.
volatile unsigned state;

__attribute__((noinline)) void touch(unsigned value)
{
  state = state * 33u + value;
}

__attribute__((always_inline)) inline void expand(unsigned value)
{
  touch(value + 0u);
  touch(value + 1u);
  touch(value + 2u);
  touch(value + 3u);
  touch(value + 4u);
  touch(value + 5u);
  touch(value + 6u);
  touch(value + 7u);
  touch(value + 8u);
  touch(value + 9u);
  touch(value + 10u);
  touch(value + 11u);
}

inline bool yes(unsigned value)
{
  expand(value);
  return true;
}

inline bool no(unsigned value)
{
  expand(value);
  return false;
}

inline bool mixed(unsigned value)
{
  expand(value);
  return (value & 1u) != 0;
}

__attribute__((noinline)) unsigned exercise(unsigned value)
{
  touch(value ^ 20u);
  touch(value ^ 21u);
  touch(value ^ 22u);
  touch(value ^ 23u);
  touch(value ^ 24u);
  touch(value ^ 25u);
  touch(value ^ 26u);
  touch(value ^ 27u);
  touch(value ^ 28u);
  touch(value ^ 29u);
  touch(value ^ 30u);
  touch(value ^ 31u);
  unsigned result = 0;
  if (yes(value)) result += 1;
  if (no(value + 1)) result += 2;
  if (mixed(value + 2)) result += 4;
  if (mixed(value + 3)) result += 8;
  return result;
}

inline int seventeen(unsigned value)
{
  expand(value);
  return 17;
}

inline unsigned long large_literal(unsigned value)
{
  expand(value);
  return (1ul << 40) + 17;
}

inline unsigned __int128 wide_literal(unsigned value)
{
  expand(value);
  return (static_cast<unsigned __int128>(1) << 100) + 17;
}

inline unsigned __int128 different_high_bits(unsigned value)
{
  expand(value);
  if (value & 1u) return (static_cast<unsigned __int128>(1) << 100) + 17;
  return (static_cast<unsigned __int128>(2) << 100) + 17;
}

__attribute__((noinline)) unsigned general_literals(unsigned value)
{
  touch(value ^ 40u);
  touch(value ^ 41u);
  touch(value ^ 42u);
  touch(value ^ 43u);
  touch(value ^ 44u);
  touch(value ^ 45u);
  touch(value ^ 46u);
  touch(value ^ 47u);
  touch(value ^ 48u);
  touch(value ^ 49u);
  touch(value ^ 50u);
  touch(value ^ 51u);
  unsigned result = 0;
  if (seventeen(value) == 17) result += 1;
  if (large_literal(value + 1) == (1ul << 40) + 17) result += 2;
  if (wide_literal(value + 2) == (static_cast<unsigned __int128>(1) << 100) + 17) result += 4;
  unsigned __int128 expected = (static_cast<unsigned __int128>((value & 1u) ? 1 : 2) << 100) + 17;
  if (different_high_bits(value) == expected) result += 8;
  return result;
}

int main()
{
  state = 0;
  if (exercise(0u) != 9u || state != 910840962u) return 1;
  state = 0;
  if (exercise(1u) != 5u || state != 1897896562u) return 1;
  state = 0;
  if (exercise(4294967294u) != 9u || state != 2177805746u) return 1;
  state = 0;
  if (general_literals(0u) != 15u || state != 3506336142u) return 1;
  state = 0;
  if (general_literals(1u) != 15u || state != 198424446u) return 1;
  state = 0;
  if (general_literals(4294967294u) != 15u || state != 1748549342u) return 1;
  return 0;
}
