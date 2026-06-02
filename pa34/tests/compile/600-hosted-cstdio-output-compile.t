#include <cstdio>
#include <type_traits>
static_assert(std::is_same<decltype(std::puts("")), int>::value, "<cstdio> puts -> int");
static_assert(std::is_same<decltype(std::fputs("", stdout)), int>::value, "<cstdio> fputs -> int");
