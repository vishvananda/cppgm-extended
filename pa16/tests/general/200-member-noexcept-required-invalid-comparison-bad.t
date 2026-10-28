struct Value {};
template<class T> struct Wrapper {
  static int test(T value) noexcept(noexcept(value == value)) { return 0; }
};
int main() { return Wrapper<Value>::test(Value{}); }
