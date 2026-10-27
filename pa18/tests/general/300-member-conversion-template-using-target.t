// VALIDATION: compile-pass
// A conversion target remains part of a member-template name during importing.
struct Base {
  template<class T = void> operator int() const { return 1; }
};
struct Derived : private Base {
  template<class T = void> operator long() const { return 2; }
  using Base::operator int;
};
int main() {
  Derived object;
  return static_cast<int>(object) == 1 && static_cast<long>(object) == 2 ? 0 : 1;
}
