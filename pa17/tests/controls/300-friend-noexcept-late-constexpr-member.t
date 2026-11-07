// Adopted CWG1330: the complete-class noexcept context includes later members.
// A namespace redeclaration with noexcept(false) remains potentially throwing.
// expect exit 0
namespace instantiated {
void f() noexcept;
void g();
template<class T> struct A {
  friend void f() noexcept;
  friend void g() noexcept(x);
  static constexpr bool x = false;
};
template struct A<int>;
void f() noexcept {}
void g() {}
}
int main() { instantiated::A<int> a; instantiated::f(); instantiated::g(); return !noexcept(instantiated::g()) && !instantiated::A<int>::x ? 0 : 1; }
