using Shorts = short __attribute__((vector_size(16)));
using Ints = int __attribute__((vector_size(16)));
using Longs = long long __attribute__((vector_size(16)));
#if !__has_builtin(__builtin_ia32_psllwi128)
#error missing packed lane shift
#endif
static_assert(__is_same(decltype(__builtin_ia32_psllwi128(Shorts{},1)), Shorts),"result");
#if !__has_builtin(__builtin_ia32_psrlwi128)
#error missing packed lane shift
#endif
static_assert(__is_same(decltype(__builtin_ia32_psrlwi128(Shorts{},1)), Shorts),"result");
#if !__has_builtin(__builtin_ia32_psrawi128)
#error missing packed lane shift
#endif
static_assert(__is_same(decltype(__builtin_ia32_psrawi128(Shorts{},1)), Shorts),"result");
#if !__has_builtin(__builtin_ia32_pslldi128)
#error missing packed lane shift
#endif
static_assert(__is_same(decltype(__builtin_ia32_pslldi128(Ints{},1)), Ints),"result");
#if !__has_builtin(__builtin_ia32_psrldi128)
#error missing packed lane shift
#endif
static_assert(__is_same(decltype(__builtin_ia32_psrldi128(Ints{},1)), Ints),"result");
#if !__has_builtin(__builtin_ia32_psradi128)
#error missing packed lane shift
#endif
static_assert(__is_same(decltype(__builtin_ia32_psradi128(Ints{},1)), Ints),"result");
#if !__has_builtin(__builtin_ia32_psllqi128)
#error missing packed lane shift
#endif
static_assert(__is_same(decltype(__builtin_ia32_psllqi128(Longs{},1)), Longs),"result");
#if !__has_builtin(__builtin_ia32_psrlqi128)
#error missing packed lane shift
#endif
static_assert(__is_same(decltype(__builtin_ia32_psrlqi128(Longs{},1)), Longs),"result");
