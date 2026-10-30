using V=long long __attribute__((vector_size(16)));
V bad(V a){return __builtin_ia32_psrldqi128_byteshift(a);}
