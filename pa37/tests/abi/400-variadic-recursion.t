#include <stdarg.h>
extern "C" __attribute__((noinline)) long long recurse(int depth, ...) {
 va_list arguments; va_start(arguments, depth);
 long long value = va_arg(arguments, long long);
 int step = va_arg(arguments, int);
 va_end(arguments);
 return depth == 0 ? value : recurse(depth - 1, value + step, step);
}
extern "C" int observe() { return recurse(8, 10LL, 3) == 34 ? 17 : 1; }
