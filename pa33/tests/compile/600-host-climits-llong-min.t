#include <climits>

long long f(long long x, long long y) {
  return x == LLONG_MIN && y == -1 ? 1 : 0;
}
