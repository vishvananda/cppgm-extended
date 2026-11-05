template<class T> int nested(T initial) {
  auto outer = [initial](auto extra) {
    auto inner = [=](const auto& scale) { return (initial + extra) * scale; };
    return inner(3);
  };
  return outer(2);
}
struct Record { int value; };
int main() {
  auto less = [](const auto& a, const auto& b) { return a.value < b.value; };
  Record a = {2}, b = {7};
  if (!less(a, b) || less(b, a)) return 1;
  auto forward = [](auto&& value) -> decltype(auto) { return static_cast<decltype(value)>(value); };
  forward(a).value = 9;
  if (a.value != 9) return 2;
  auto count = [](auto... values) { return sizeof...(values); };
  if (count() != 0 || count(1, 2.0, 'x') != 3) return 3;
  int total = 1;
  auto state = [total](auto value) mutable { total += value; return total; };
  if (state(2) != 3 || state(4) != 7) return 4;
  return nested(4) != 18;
}
