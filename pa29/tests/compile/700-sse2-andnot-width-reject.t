using V=long long __attribute__((vector_size(8)));
V f(V a){return __builtin_ia32_pandn128(a,a);}
