// expect exit 0
// C++17 [except.spec], [over.match]: evaluating the selected function's lazy
// noexcept expression must preserve the pending call's argument conversions.
template<class T> T&& declval();
struct Register {
  int value;
  operator unsigned() const { return value; }
};
int inspect(unsigned x) { return x; }
struct Functor {
  int operator()(Register a, Register& b) const { return a.value + b.value; }
};
template<class F, class... A>
auto invoke(F&& f, A&&... a) noexcept(noexcept(inspect(declval<Register&>())))
    -> decltype(declval<F>()(declval<A>()...)) {
  return static_cast<F&&>(f)(static_cast<A&&>(a)...);
}
int main() {
  Register r = {3};
  return invoke(Functor{}, r, r) == 6 ? 0 : 1;
}
