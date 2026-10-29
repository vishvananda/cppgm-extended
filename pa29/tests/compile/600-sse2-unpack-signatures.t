using Bytes = char __attribute__((vector_size(16)));
using Shorts = short __attribute__((vector_size(16)));
using Ints = int __attribute__((vector_size(16)));
using Longs = long long __attribute__((vector_size(16)));
#if !__has_builtin(__builtin_ia32_punpcklbw128)
#error missing SSE2 unpack
#endif
static_assert(__is_same(decltype(__builtin_ia32_punpcklbw128(Bytes{}, Bytes{})), Bytes), "lane type");
#if !__has_builtin(__builtin_ia32_punpckhbw128)
#error missing SSE2 unpack
#endif
static_assert(__is_same(decltype(__builtin_ia32_punpckhbw128(Bytes{}, Bytes{})), Bytes), "lane type");
#if !__has_builtin(__builtin_ia32_punpcklwd128)
#error missing SSE2 unpack
#endif
static_assert(__is_same(decltype(__builtin_ia32_punpcklwd128(Shorts{}, Shorts{})), Shorts), "lane type");
#if !__has_builtin(__builtin_ia32_punpckhwd128)
#error missing SSE2 unpack
#endif
static_assert(__is_same(decltype(__builtin_ia32_punpckhwd128(Shorts{}, Shorts{})), Shorts), "lane type");
#if !__has_builtin(__builtin_ia32_punpckldq128)
#error missing SSE2 unpack
#endif
static_assert(__is_same(decltype(__builtin_ia32_punpckldq128(Ints{}, Ints{})), Ints), "lane type");
#if !__has_builtin(__builtin_ia32_punpckhdq128)
#error missing SSE2 unpack
#endif
static_assert(__is_same(decltype(__builtin_ia32_punpckhdq128(Ints{}, Ints{})), Ints), "lane type");
#if !__has_builtin(__builtin_ia32_punpcklqdq128)
#error missing SSE2 unpack
#endif
static_assert(__is_same(decltype(__builtin_ia32_punpcklqdq128(Longs{}, Longs{})), Longs), "lane type");
#if !__has_builtin(__builtin_ia32_punpckhqdq128)
#error missing SSE2 unpack
#endif
static_assert(__is_same(decltype(__builtin_ia32_punpckhqdq128(Longs{}, Longs{})), Longs), "lane type");
Bytes global(const Bytes& a, const Bytes& b) { return ::__builtin_ia32_punpckhbw128(a, b); }
