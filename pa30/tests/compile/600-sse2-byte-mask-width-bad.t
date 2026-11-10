using Bytes = char __attribute__((vector_size(8)));
int value = __builtin_ia32_pmovmskb128(Bytes{});
