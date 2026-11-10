using V=int __attribute__((vector_size(16)));
V f(V a,int n){return __builtin_shufflevector(a,a,0,n,2,3);}
