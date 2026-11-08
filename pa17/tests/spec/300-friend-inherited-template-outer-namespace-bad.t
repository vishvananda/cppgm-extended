// N3485 7.3.1.2p3: a friend template cannot redeclare a template from
// outside the innermost namespace, even when a base injects its name.
template<class T> struct Base { static int read(); };
namespace inner {
struct Derived: Base<int> {
  template<class> friend struct Base;
private:
  static int value;
};
int Derived::value = 7;
}
template<class T> int Base<T>::read() { return inner::Derived::value; }
int main() { return Base<long>::read(); }
