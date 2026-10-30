using V=int __attribute__((vector_size(16)));
using M=short __attribute__((vector_size(8)));
V f(V a,M m){return __builtin_shuffle(a,m);}
