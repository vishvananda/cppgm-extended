using Bytes=char __attribute__((vector_size(16)));
Bytes bad = __builtin_ia32_punpckhbw128(Bytes{});
