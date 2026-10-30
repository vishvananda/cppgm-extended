using V=double __attribute__((vector_size(8)));
int f(V x){return __builtin_ia32_movmskpd(x);}
