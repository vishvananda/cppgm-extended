enum class Flag : bool { no, yes };
constexpr Flag invalid = __builtin_bit_cast(Flag, static_cast<unsigned char>(2));
