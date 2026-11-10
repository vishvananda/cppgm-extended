using Vector = long long __attribute__((vector_size(16)));
Vector value = __builtin_ia32_pslldqi128(Vector{}, 7);
