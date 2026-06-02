#include <climits>

static_assert(LLONG_MIN == -LLONG_MAX - 1, "LLONG_MIN must equal -(LLONG_MAX)-1");
static_assert(LLONG_MIN < 0, "LLONG_MIN must be negative");
static_assert(sizeof(long long) * CHAR_BIT >= 64, "long long must be at least 64 bits");
