using V=int __attribute__((vector_size(16)));
V f(V a){return __builtin_shufflevector(a,a,0,8,2,3);}
