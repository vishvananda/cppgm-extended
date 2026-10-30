using V=long long __attribute__((vector_size(16)));
V f(){return __builtin_ia32_pandn128(1,2);}
