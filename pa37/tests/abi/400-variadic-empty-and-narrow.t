#include <stdarg.h>
struct Empty {};
struct Byte { unsigned char value; };
__attribute__((noinline)) int read(int seed, ...) {
 va_list arguments; va_start(arguments, seed);
 Empty first = va_arg(arguments, Empty);
 Byte second = va_arg(arguments, Byte);
 Empty third = va_arg(arguments, Empty);
 Byte fourth = va_arg(arguments, Byte);
 int last = va_arg(arguments, int);
 Empty tail = va_arg(arguments, Empty);
 va_end(arguments);
 return seed + second.value + fourth.value + last;
}
extern "C" int observe() { return read(7, Empty{}, Byte{5}, Empty{}, Byte{9}, 11, Empty{}) == 32 ? 17 : 1; }
