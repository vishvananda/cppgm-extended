struct Counter {
  int* total;
  int operator()(int value) const { return *total += value; }
};
int main() {
  int total = 1;
  auto get = [&](int n) { return total + n; };
  auto visit = [&](int n, auto&& self) -> int {
    if (n == 0) return get(2);
    return self(n - 1, self);
  };
  if (visit(2, visit) != 3) return 1;
  Counter counter = {&total};
  auto copied = [=](auto n) { return counter(n); };
  if (copied(4) != 5) return 2;
  auto unevaluated = [=](auto n) { return sizeof(get(0)) + n; };
  if (sizeof(unevaluated) != 1 || unevaluated(0) != sizeof(int)) return 3;
  return 0;
}
