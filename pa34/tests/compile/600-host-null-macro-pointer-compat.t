#include <cstddef>
#include <cstdlib>
#include <type_traits>

static_assert(sizeof(std::size_t) == sizeof(void *), "<cstddef> std::size_t is pointer-sized");
static_assert(std::is_same<std::size_t, decltype(sizeof(int))>::value, "std::size_t identity");

int *null_return() { return NULL; }
bool null_compare(int *p) { return p == NULL || p != NULL; }
char *null_realpath(const char *path) { return realpath(path, NULL); }
unsigned long long null_strtoull(const char *value) { return strtoull(value, NULL, 0); }
