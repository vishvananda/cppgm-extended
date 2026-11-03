// Return, tail-transfer and cleanup paths may need different frame teardown.
// The object route also exercises unwind rows around stack-passed arguments.
volatile int input = 1;
int cleaned = 0;
struct Guard {
  int amount;
  Guard(int n) : amount(n) {}
  ~Guard() { cleaned += amount; }
};

__attribute__((noinline)) long many(long a, long b, long c, long d,
                                  long e, long f, long g, long h, long i)
{
  if (a < 0) throw static_cast<int>(i);
  return a + b + c + d + e + f + g + h + i;
}

__attribute__((noinline)) long protected_return(int n)
{
  Guard guard(n + 3);
  if (n == 0) return 71;
  return many(n, 2, 3, 4, 5, 6, 7, 8, 9);
}

__attribute__((noinline)) int plain(int n) { return n ? n + 4 : 7; }
__attribute__((noinline)) int tail(int n) { return plain(n); }
__attribute__((noinline)) long double floating(int n)
{
  if (n) return 2.5L;
  return -1.25L;
}
__attribute__((noinline, noreturn)) void only_throw(int n) { throw n; }

int main()
{
  int n = input;
  if (protected_return(n) != 45 || cleaned != 4) return 1;
  if (protected_return(n - 1) != 71 || cleaned != 7) return 2;
  try {
    protected_return(-n);
    return 3;
  } catch (int value) {
    if (value != 9 || cleaned != 9) return 4;
  }
  if (tail(n) != 5 || tail(n - 1) != 7) return 5;
  if (floating(n) != 2.5L || floating(n - 1) != -1.25L) return 6;
  try { only_throw(n + 10); }
  catch (int value) { return value == 11 ? 0 : 7; }
  return 8;
}
