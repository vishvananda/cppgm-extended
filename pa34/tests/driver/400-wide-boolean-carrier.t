struct Flags { const void* pointer; bool strict; unsigned char mode; };

__attribute__((noinline)) int dirty(int value)
{
  asm volatile("" : "+r"(value));
  return value;
}

__attribute__((noinline)) Flags make_flags(const void* pointer, int value, unsigned char mode)
{
  return Flags{pointer, dirty(value) != 0, mode};
}

__attribute__((noinline)) bool check(Flags value, const void* pointer, int flag, unsigned char mode)
{
  return value.pointer == pointer && value.strict == (flag != 0) && value.mode == mode;
}

int main()
{
  const int values[] = {-2147483647 - 1, -65536, -257, -1, 0, 1, 256, 2147483647};
  for (int value : values)
    for (unsigned mode = 0; mode != 256; ++mode) {
      Flags flags = make_flags(values, value, static_cast<unsigned char>(mode));
      if (!check(flags, values, value, static_cast<unsigned char>(mode))) return 1;
    }
}
