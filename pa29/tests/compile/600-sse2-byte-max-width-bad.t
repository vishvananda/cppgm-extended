using Bytes=char __attribute__((vector_size(16)));
using Small=char __attribute__((vector_size(8)));
using Ints=int __attribute__((vector_size(16)));
Bytes f(Bytes a,Small b,Ints c){return __builtin_ia32_pmaxub128(a,b);}
