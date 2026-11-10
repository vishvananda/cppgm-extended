struct Padded { char first; unsigned second; };
constexpr Padded value = {'a', 19};
constexpr unsigned long long bad = __builtin_bit_cast(unsigned long long, value);
