static_assert(__INTPTR_MAX__ == 2147483647L, "wasm32 signed pointer limit");
static_assert(__UINTPTR_MAX__ == 4294967295UL, "wasm32 unsigned pointer limit");
static_assert(__PTRDIFF_MAX__ == __INTPTR_MAX__, "pointer difference limit");
static_assert(__SIZE_MAX__ == __UINTPTR_MAX__, "size limit");
extern "C" int observe() { return 17; }
