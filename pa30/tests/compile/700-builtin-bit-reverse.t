#if !__has_builtin(__builtin_bitreverse8) || !__has_builtin(__builtin_bitreverse16) || !__has_builtin(__builtin_bitreverse32) || !__has_builtin(__builtin_bitreverse64)
#error bit reversal builtins must be advertised
#endif
static_assert(__builtin_bitreverse8(0x96) == 0x69, "byte reversal");
static_assert(__builtin_bitreverse8(0x196) == 0x69, "narrow argument conversion");
static_assert(__builtin_bitreverse16(0x1234) == 0x2c48, "short reversal");
static_assert(__builtin_bitreverse32(0x12345678u) == 0x1e6a2c48u, "word reversal");
static_assert(__builtin_bitreverse64(0x0123456789abcdefULL) == 0xf7b3d591e6a2c480ULL, "wide reversal");
static_assert(__builtin_bitreverse64(0) == 0, "zero is defined");
static_assert(__builtin_bitreverse16(-1) == 65535u, "signed argument conversion");
static_assert(__is_same(decltype(__builtin_bitreverse8(0)), unsigned char), "byte type");
static_assert(__is_same(decltype(__builtin_bitreverse16(0)), unsigned short), "short type");
static_assert(__is_same(decltype(__builtin_bitreverse32(0)), unsigned int), "word type");
static_assert(__is_same(decltype(__builtin_bitreverse64(0)), unsigned long), "target uint64 type");
