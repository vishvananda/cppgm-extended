#if !__has_builtin(__builtin_ffs) || !__has_builtin(__builtin_ffsl) || !__has_builtin(__builtin_ffsll)
#error missing first-set-bit builtin
#endif
static_assert(__builtin_ffs(0) == 0, "zero");
static_assert(__builtin_ffsl(0L) == 0, "long zero");
static_assert(__builtin_ffsll(0LL) == 0, "long long zero");
static_assert(__builtin_ffs(8) == 4, "one-based index");
static_assert(__builtin_ffs(-8) == 4, "signed bits");
static_assert(__builtin_ffsl(static_cast<long>(1UL << (sizeof(long) * 8 - 1))) == sizeof(long) * 8, "long width");
static_assert(__builtin_ffsll(static_cast<long long>(1ULL << 63)) == 64, "long long width");
static_assert(__builtin_ffs(static_cast<int>(1ULL << 40)) == 0, "converted operand");
template<class A, class B> struct Same { static const bool value = false; };
template<class A> struct Same<A, A> { static const bool value = true; };
static_assert(Same<decltype(__builtin_ffs(1)), int>::value, "int result");
static_assert(Same<decltype(__builtin_ffsl(1L)), int>::value, "long result");
static_assert(Same<decltype(__builtin_ffsll(1LL)), int>::value, "long long result");
struct Value { constexpr operator int() const { return 8; } };
static_assert(__builtin_ffs(Value{}) == 4, "ordinary argument conversion");
