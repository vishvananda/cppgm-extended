struct Fields { unsigned low : 4; int high : 4; unsigned rest : 24; };
constexpr Fields fields = {5, -2, 0x123456};
static_assert(__builtin_bit_cast(unsigned, fields) == 0x123456e5u, "bit-field encoding");
constexpr Fields field_copy = __builtin_bit_cast(Fields, 0x123456e5u);
static_assert(field_copy.low == 5 && field_copy.high == -2 && field_copy.rest == 0x123456, "bit-field decoding");
int main() { return 0; }
