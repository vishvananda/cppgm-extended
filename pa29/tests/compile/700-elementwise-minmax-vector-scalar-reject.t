using V=int __attribute__((vector_size(16)));V bad(V a){return __builtin_elementwise_max(a,1);}
