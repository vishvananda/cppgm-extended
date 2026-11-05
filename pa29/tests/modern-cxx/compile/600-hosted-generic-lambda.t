template<class A, class B> struct Same { static constexpr bool value = false; };
template<class A> struct Same<A, A> { static constexpr bool value = true; };
struct Record { int value; };
void check() {
  auto compare = [](const auto& a, const auto& b) { return a.value < b.value; };
  Record a = {1}, b = {2};
  static_assert(Same<decltype(compare(a, b)), bool>::value, "deduced const-reference parameters");
  auto identity = [](auto&& value) -> decltype(auto) { return static_cast<decltype(value)>(value); };
  static_assert(Same<decltype(identity(a)), Record&>::value, "lvalue forwarding");
  static_assert(Same<decltype(identity(Record{3})), Record&&>::value, "rvalue forwarding");
  auto count = [](auto... args) { return sizeof...(args); };
  static_assert(Same<decltype(count(1, 2.0, 'x')), decltype(sizeof(int))>::value, "invented type pack");
}
