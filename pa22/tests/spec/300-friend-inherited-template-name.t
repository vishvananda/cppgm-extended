// N3485 14.6.1p1, p4 and 14.5.4: inherited injected-class-names
// from different specializations still name one friend template.
template<class T> struct Base { static int read(); };
struct Derived: Base<int>, Base<char> {
  template<class> friend struct Base;
private:
  static int value;
};
int Derived::value = 7;
template<class T> int Base<T>::read() { return Derived::value; }
int main() { return Base<long>::read() == 7 ? 0 : 1; }
