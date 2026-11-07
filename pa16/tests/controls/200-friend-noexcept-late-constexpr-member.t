// Adopted CWG1330 restores complete-class lookup inside a friend noexcept specification.
// expect exit 0
namespace ordinary {
void f() noexcept;
void g() noexcept;
struct A {
  friend void f() noexcept;
  friend void g() noexcept(x);
  static constexpr bool x = true;
};
void f() noexcept {}
void g() noexcept {}
}
int main() { ordinary::A a; ordinary::f(); ordinary::g(); return noexcept(ordinary::g()) && ordinary::A::x ? 0 : 1; }
