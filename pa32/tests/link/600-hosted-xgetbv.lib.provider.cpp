#include <cpuid.h>
extern "C" bool host_xstate_available() {
  unsigned a, b, c, d;
  return __get_cpuid(1, &a, &b, &c, &d) && (c & (1U << 27)) != 0;
}
extern "C" unsigned long long host_xstate() {
  unsigned low, high;
  asm volatile("xgetbv" : "=a"(low), "=d"(high) : "c"(0));
  return low | (static_cast<unsigned long long>(high) << 32);
}
