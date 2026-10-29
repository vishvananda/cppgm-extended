using Ints=int __attribute__((vector_size(16)));
using Doubles=double __attribute__((vector_size(16)));
using Longs=long long __attribute__((vector_size(16)));
#if !__has_builtin(__builtin_ia32_pshufd) || !__has_builtin(__builtin_ia32_shufpd)
#error missing SSE2 lane shuffle
#endif
static_assert(__is_same(decltype(::__builtin_ia32_pshufd(Ints{},255)),Ints),"integer result");
static_assert(__is_same(decltype(::__builtin_ia32_shufpd(Doubles{},Doubles{},3)),Doubles),"double result");
