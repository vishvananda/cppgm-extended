using Ints=int __attribute__((vector_size(16)));
using Small=int __attribute__((vector_size(8)));
void f(){__builtin_ia32_psrldi128(Ints{});}
