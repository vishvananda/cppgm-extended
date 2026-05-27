#include <cstddef>
#include <cstdlib>

int *null_return() {
  return NULL;
}

bool null_compare(int *p) {
  return p == NULL || p != NULL;
}

char *null_realpath(const char *path) {
  return realpath(path, NULL);
}

unsigned long long null_strtoull(const char *value) {
  return strtoull(value, NULL, 0);
}
