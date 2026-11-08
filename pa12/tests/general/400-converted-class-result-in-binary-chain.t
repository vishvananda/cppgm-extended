// A conversion makes the whole class-valued operator a leaf of a binary chain.
int evaluations;
int destructions;
struct Size {
  long value;
  bool scalable;
  Size(long n, bool s = false): value(n), scalable(s) {}
  ~Size() { ++destructions; }
  operator long() const { return value; }
};
Size operator*(const Size& size, int factor) { ++evaluations; return Size(size.value * factor, size.scalable); }
Size get() { ++evaluations; return Size(3); }
int main() {
  bool result = get() * 2 >= 5;
  if (!result || evaluations != 2 || destructions < 2) return 1;
  evaluations = 0;
  long value = get() * 3 + 2;
  return value == 11 && evaluations == 2 ? 0 : 2;
}
