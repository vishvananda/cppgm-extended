using Bytes=char __attribute__((vector_size(8)));
auto bad = __builtin_ia32_punpckhbw128(Bytes{}, Bytes{});
