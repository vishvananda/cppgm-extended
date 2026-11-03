// A later normal-path overwrite must not discard the value a handler reads
// when the preceding call throws. Include nested handlers and partial stores.
volatile int choice = 1;
__attribute__((noinline)) void may_throw(int n) { if (n) throw n; }

__attribute__((noinline)) int ordinary(int n)
{
  int value = 11;
  try {
    may_throw(n);
    value = 29;
    may_throw(n + 1);
    value = 37;
  } catch (int) { return value; }
  return value;
}

__attribute__((noinline)) int nested(int n)
{
  int outer = 13, inner = 17;
  try {
    try {
      may_throw(n);
      inner = 31;
      may_throw(n + 1);
    } catch (int) {
      outer += inner;
      may_throw(n + 2);
      outer = 43;
    }
  } catch (int) { return outer; }
  return outer + inner;
}

struct Pair { long first, second; };
__attribute__((noinline)) long parts(int n)
{
  Pair value = { 19, 23 };
  try {
    may_throw(n);
    value.first = 41;
    may_throw(n + 1);
    value.second = 47;
  } catch (int) { return value.first + value.second; }
  return value.first + value.second;
}

int main()
{
  int n = choice;
  if (ordinary(n) != 11 || ordinary(n - 1) != 29) return 1;
  if (nested(n) != 30 || nested(n - 1) != 44) return 2;
  if (parts(n) != 42 || parts(n - 1) != 64) return 3;
  return 0;
}
