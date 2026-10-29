using Ints=int __attribute__((vector_size(16)));
using Doubles=double __attribute__((vector_size(16)));
using Longs=long long __attribute__((vector_size(16)));
using Small=int __attribute__((vector_size(8)));
Doubles f(Ints a,Doubles d,int n){return __builtin_ia32_shufpd(d,1);}
