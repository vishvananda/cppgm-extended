union Word { unsigned value; };
constexpr Word value = {37};
constexpr unsigned bad = __builtin_bit_cast(unsigned, value);
