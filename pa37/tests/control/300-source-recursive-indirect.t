typedef int (*Operation)(int);
int increment(int x) { return x + 3; }
int twice(int x) { return x * 2; }
Operation operations[2] = {increment, twice};
int factorial(int n) { return n < 2 ? 1 : n * factorial(n - 1); }
extern "C" int recursive_indirect(int index, int x) {
  return operations[index](x) + factorial(x);
}
