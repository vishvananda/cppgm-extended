#include <assert.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
static int sum(int count, ...) {
  va_list args; va_start(args, count);
  int result = 0;
  while (count--) result += va_arg(args, int);
  va_end(args); return result;
}
int main(void) {
  uint32_t *data = malloc(3 * sizeof(*data));
  if (!data) return 1;
  data[0] = 17; data[1] = 29; data[2] = 42;
  uint32_t copy[3]; memcpy(copy, data, sizeof(copy)); free(data);
  bool okay = true;
  return !(okay && copy[1] == 29 && copy[2] == 42 && sum(3, (char)1, (short)2, 3) == 6);
}
