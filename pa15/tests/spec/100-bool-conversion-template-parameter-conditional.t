template<bool V> struct flag {
  operator bool() const { return V; }
};
int main() {
  flag<true> yes;
  flag<false> no;
  return (yes ? 3 : 5) != 3 || (no ? 7 : 9) != 9;
}
