#if !__has_builtin(__builtin_ia32_pminub128)
#error missing packed unsigned byte minimum
#endif
using Bytes = char __attribute__((vector_size(16)));
static_assert(__is_same(decltype(__builtin_ia32_pminub128(Bytes{}, Bytes{})), Bytes), "char vector result");
Bytes minimum(Bytes a, Bytes b) { return ::__builtin_ia32_pminub128(a, b); }
