#if !__has_builtin(__builtin_ia32_pslldqi128) || !__has_builtin(__builtin_ia32_psrldqi128)
#error missing whole-register byte shifts
#endif
using Vector = long long __attribute__((vector_size(16)));
static_assert(__is_same(decltype(__builtin_ia32_pslldqi128(Vector{}, 8)), Vector), "left result");
static_assert(__is_same(decltype(::__builtin_ia32_psrldqi128(Vector{}, 8)), Vector), "right result");
Vector left(Vector value) { return __builtin_ia32_pslldqi128(value, 64); }
Vector right(Vector value) { return __builtin_ia32_psrldqi128(value, 120); }
