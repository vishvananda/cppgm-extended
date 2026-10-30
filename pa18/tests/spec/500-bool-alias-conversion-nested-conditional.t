template<bool V> struct flag {
  static const bool value = V;
  operator bool() const { return value; }
};
template<bool V> using bool_constant = flag<V>;
int hits;
int chosen() { ++hits; return 7; }
int rejected() { hits += 100; return 9; }
int main() {
  bool_constant<true> yes;
  bool_constant<false> no;
  int result = (yes ? true : false) ? (no ? rejected() : chosen()) : rejected();
  return result != 7 || hits != 1;
}
