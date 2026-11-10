#if !__has_builtin(__builtin_ia32_cvttps2dq)
#error missing packed truncating conversion
#endif
using Floats = float __attribute__((vector_size(16)));
using Ints = int __attribute__((vector_size(16)));
static_assert(__is_same(decltype(__builtin_ia32_cvttps2dq(Floats{})), Ints), "four signed int lanes");
static_assert(__is_same(decltype(::__builtin_ia32_cvttps2dq(Floats{})), Ints), "global builtin");
Ints convert(const Floats& value) { return __builtin_ia32_cvttps2dq(value); }
