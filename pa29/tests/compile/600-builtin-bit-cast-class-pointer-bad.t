struct Pointer { int* value; };
constexpr Pointer value = {nullptr};
constexpr unsigned long bad = __builtin_bit_cast(unsigned long, value);
