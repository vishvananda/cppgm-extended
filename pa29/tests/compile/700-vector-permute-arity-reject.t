using V=int __attribute__((vector_size(16)));
V f(V a){return __builtin_shuffle(a);}
