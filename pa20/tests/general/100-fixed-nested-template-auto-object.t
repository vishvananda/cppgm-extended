// An initializer can be value-dependent while its resulting class is fixed.
template<class> struct Tag {};
struct Value {
  template<class T> static Value make() { return Value(); }
  template<class T> int read() const { return sizeof(T); }
};
Value make(unsigned long) { return Value(); }
template<class T> int nested() {
  const auto& object = Value::make<Tag<int>>();
  return object.read<T>();
}
template<class T> int sized() {
  auto object = make(sizeof(T));
  return object.read<T>();
}
int main() { return nested<unsigned char>() + sized<unsigned char>() == 2 ? 0 : 1; }
