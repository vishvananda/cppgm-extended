// N3485 13.3.1p4: a base assignment introduced by a using-declaration
// has the derived class as its implicit object parameter for ranking.
struct Base {
  unsigned value;
  Base() : value(0) {}
  unsigned operator=(unsigned n) { value = n; return n + 10; }
  unsigned operator=(unsigned n) volatile { value = n; return n + 20; }
};
struct Value : Base {
  Value(unsigned n) { value = n; }
  Value& operator=(const Value&) = delete;
  Value& operator=(const Value&) volatile = delete;
  using Base::operator=;
};
struct Hidden : Base {
  Hidden(unsigned n) { value = n; }
};
int main() {
  Value v(1);
  volatile Value w(1);
  if (v.operator=(2u) != 12 || (v = 3u) != 13 || v.value != 3) return 1;
  if ((w = 4u) != 24 || w.value != 4) return 2;
  short n = 5;
  if ((v = n) != 15 || v.value != 5) return 3;
  Base& base = v;
  if ((base = 6u) != 16 || v.value != 6) return 4;
  Hidden hidden(0);
  hidden = 7u;
  return hidden.value != 7;
}
