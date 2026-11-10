#include <cstdlib>

int* published;
__attribute__((noinline)) void capture(int* p) { published = p; }

__attribute__((noinline)) int scan(int** holder, int publish) {
  int* buffer = static_cast<int*>(std::malloc(4 * sizeof(int)));
  if (!buffer) return -1;
  if (publish == 1) published = buffer;
  if (publish == 2) capture(buffer);
  buffer[0] = 7;
  int sum = 0;
  for (int i = 0; i != 4; ++i) {
    int* source = *holder;
    sum += *source;
    buffer[i] = i;
  }
  int observed = *static_cast<volatile int*>(buffer + 3);
  std::free(buffer);
  return sum + observed;
}

int main() {
  int value = 9;
  int* source = &value;
  if (scan(&source, 0) != 39) return 1;
  if (scan(&published, 1) != 10) return 2;
  if (scan(&published, 2) != 10) return 3;
  return 0;
}
