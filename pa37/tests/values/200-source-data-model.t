static_assert(sizeof(void*) == 4, "pointer");
static_assert(sizeof(long) == 4, "long");
static_assert(sizeof(long long) == 8, "long long");
static_assert(sizeof(long double) == 16, "long double");
static_assert(sizeof(decltype(2147483648L)) == 8, "decimal long candidate");
static_assert(sizeof(decltype(0x80000000L)) == 4, "hex unsigned long candidate");
static_assert((~0UL) == 4294967295UL, "unsigned long constant width");
#if !defined(__wasm32__) || defined(__x86_64__) || defined(__linux__)
#error incorrect target macros
#endif
struct Pair { void* pointer; long value; };
static_assert(sizeof(Pair) == 8, "class layout");
extern "C" int source_sum() {
  int values[4] = {1, 2, 3, 4};
  long total = 0;
  for (int i = 0; i < 4; ++i) total += values[i];
  return total;
}
