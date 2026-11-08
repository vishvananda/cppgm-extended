// Deduction defaults retain the enclosing specialization only when needed.
template<bool B> struct Enable {};
template<> struct Enable<true> { using type = int; };
template<class... T> struct Tuple {
  template<class U> static constexpr bool valid() { return sizeof...(T) == 1; }
  template<class, class, class... Tail> static constexpr bool valid() { return sizeof...(T) == sizeof...(Tail) + 2; }
  template<class... U, bool Good = valid<U...>(), typename Enable<Good>::type = 0>
  constexpr Tuple(U&&...): count(sizeof...(U)) {}
  int count;
};
template<> struct Tuple<> { constexpr Tuple(): count(0) {} int count; };
template<class... T> Tuple(T...) -> Tuple<T...>;
template<int N> struct Tag {};
template<int N> struct Pick {
  template<class U> static constexpr int bound() { return N; }
  template<class U, int Value = bound<U>(), typename Enable<Value == N>::type = 0>
  constexpr Pick(Tag<N>, U): value(Value) {}
  int value;
};
template<int N> struct Lazy {
  static_assert(N != 2, "no eager class instantiation");
  template<class U, bool Good = (N == 1), typename Enable<Good>::type = 0>
  Lazy(Tag<N>, U);
};
template<> struct Lazy<99> {
  template<class U> constexpr Lazy(Tag<2>, U): value(99) {}
  int value;
};
template<class U> Lazy(Tag<2>, U) -> Lazy<99>;
constexpr Tuple one(1);
constexpr Tuple two(1, 2);
constexpr Tuple three(1, 2, 3);
constexpr Pick first(Tag<1>{}, 7);
constexpr Pick second(Tag<3>{}, 7);
constexpr Pick repeated(Tag<1>{}, 7);
constexpr Lazy lazy(Tag<2>{}, 7);
static_assert(one.count == 1 && two.count == 2 && three.count == 3, "class packs");
static_assert(first.value == 1 && second.value == 3 && repeated.value == 1, "enclosing identity");
static_assert(lazy.value == 99, "unused nonviable guide must not complete Lazy<2>");
int main() {
  volatile int input = 7;
  Tuple a(1, input, 3);
  Pick b(Tag<3>{}, input);
  Lazy c(Tag<2>{}, input);
  return a.count == 3 && b.value == 3 && c.value == 99 ? 0 : 1;
}
