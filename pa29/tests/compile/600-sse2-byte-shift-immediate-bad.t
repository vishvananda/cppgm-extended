using Vector = long long __attribute__((vector_size(16)));
Vector invalid(Vector value, int bits) { return __builtin_ia32_psrldqi128(value, bits); }
