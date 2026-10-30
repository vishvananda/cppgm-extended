#if !__has_builtin(__builtin_ia32_pslldqi128_byteshift) || !__has_builtin(__builtin_ia32_psrldqi128_byteshift)
#error missing Clang byte-count shifts
#endif
using V=long long __attribute__((vector_size(16)));
static_assert(sizeof(__builtin_ia32_pslldqi128_byteshift(V{},1))==16, "left type");
static_assert(sizeof(__builtin_ia32_psrldqi128_byteshift(V{},255))==16, "right type");
V left(V a){return __builtin_ia32_pslldqi128_byteshift(a,1);}
V right(V a){return __builtin_ia32_psrldqi128_byteshift(a,15);}
