// N3485 focus: 8.4.2p4, 12.8p12. A first-declaration default stays trivial.
template<class T> struct R {
  T* value;
  explicit R(T* input) : value(input) {}
  R(R const&) = default;
};
int take(R<int> value) { return *value.value; }
int main() {
  int value = 7;
  R<int> first(&value);
  R<int> second(first);
  return take(second) != 7;
}
