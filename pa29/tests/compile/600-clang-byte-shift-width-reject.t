using V=int __attribute__((vector_size(8)));
V bad(V a){return __builtin_ia32_pslldqi128_byteshift(a,1);}
