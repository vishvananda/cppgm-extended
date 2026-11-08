// expect exit 0
// The first plain specification has no pending operand; its later definition
// still needs the private-friend constant in the actual class specialization.
template<class> struct X;
struct Key {
private:
  static constexpr bool yes() { return true; }
  friend struct X<int>;
};
template<class T> struct X {
  static const bool b=Key::yes();
  static void f() noexcept;
};
template<class U> void X<U>::f() noexcept(b) {}
struct Demand { enum { value = noexcept(X<int>::f()) }; };
int main() { X<int>::f(); return !Demand::value; }
