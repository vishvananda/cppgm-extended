using V=int __attribute__((vector_size(16)));
using F=float __attribute__((vector_size(16)));
V f(V a,F b,V m){return __builtin_shuffle(a,b,m);}
