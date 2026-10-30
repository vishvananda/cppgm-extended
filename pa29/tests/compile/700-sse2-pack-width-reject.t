using V=int __attribute__((vector_size(8)));
auto f(V a)->decltype(__builtin_ia32_packssdw128(a,a)){return __builtin_ia32_packssdw128(a,a);}
