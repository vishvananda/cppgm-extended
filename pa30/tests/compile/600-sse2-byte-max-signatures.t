using Bytes=char __attribute__((vector_size(16)));
#if !__has_builtin(__builtin_ia32_pminub128) || !__has_builtin(__builtin_ia32_pmaxub128)
#error missing SSE2 byte minimum/maximum
#endif
static_assert(__is_same(decltype(::__builtin_ia32_pmaxub128(Bytes{},Bytes{})),Bytes),"maximum result");
static_assert(__is_same(decltype(::__builtin_ia32_pminub128(Bytes{},Bytes{})),Bytes),"minimum result");
