#if !__has_builtin(__builtin_ia32_pmovmskb128)
#error missing packed byte sign mask
#endif
using Bytes = char __attribute__((vector_size(16)));
static_assert(__is_same(decltype(__builtin_ia32_pmovmskb128(Bytes{})), int), "int bit mask");
int mask(Bytes bytes) { return ::__builtin_ia32_pmovmskb128(bytes); }
