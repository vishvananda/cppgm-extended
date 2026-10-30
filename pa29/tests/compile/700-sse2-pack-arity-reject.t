using V=short __attribute__((vector_size(16)));
auto f(V a)->decltype(__builtin_ia32_packuswb128(a)){return __builtin_ia32_packuswb128(a);}
