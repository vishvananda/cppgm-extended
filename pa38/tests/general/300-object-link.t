#include "300-object-link.h"
#include <stdarg.h>
const int c_constant = 37;
struct Pair c_transform(struct Pair value, int (*callback)(int)) {
  value.left = callback(value.left);
  value.right = value.right * 2.0 + c_constant;
  return value;
}
int c_variadic(int count, ...) {
  va_list args; va_start(args, count);
  int result = 0;
  while (count--) result += va_arg(args, int);
  va_end(args); return result;
}
