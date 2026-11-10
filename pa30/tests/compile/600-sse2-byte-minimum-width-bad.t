using Bytes = char __attribute__((vector_size(8)));
auto value = __builtin_ia32_pminub128(Bytes{}, Bytes{});
