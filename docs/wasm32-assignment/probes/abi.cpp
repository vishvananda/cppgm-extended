// Header-free LLVM wasm32 ABI research probe, not a student fixture.
static_assert(sizeof(void*) == 4, "pointer");
static_assert(sizeof(long) == 4, "long");
static_assert(sizeof(long double) == 16, "long double");
static_assert(alignof(long double) == 16, "long double alignment");
static_assert(__LDBL_MANT_DIG__ == 113, "binary128");
struct One { int x; };
struct Pair { int x, y; };
struct Nested { One x; };
struct alignas(16) Aligned { int x; };
extern "C" {
One one(One x) { return x; }
Pair pair(Pair x) { x.x += x.y; return x; }
Nested nested(Nested x) { return x; }
Aligned aligned(Aligned x) { return x; }
__int128 wide(__int128 x) { return x + 1; }
long double quad(long double x) { return x + 1; }
signed char narrow(signed char x) { return x; }
int calls(int (*f)(int), int x) { return f(x); }
int sum(int n, ...) {
  __builtin_va_list ap;
  __builtin_va_start(ap, n);
  int result = 0;
  for (int i = 0; i < n; ++i) result += __builtin_va_arg(ap, int);
  __builtin_va_end(ap);
  return result;
}
int run() { Pair p = {4, 7}; return pair(p).x + sum(3, 2, 3, 5); }
}
