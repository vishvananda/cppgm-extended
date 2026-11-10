using Bytes = char __attribute__((vector_size(16)));
Bytes value = __builtin_ia32_pminub128(Bytes{});
