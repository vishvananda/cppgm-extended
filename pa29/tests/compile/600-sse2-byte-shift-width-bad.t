using Small = long long __attribute__((vector_size(8)));
Small value = __builtin_ia32_psrldqi128(Small{}, 8);
