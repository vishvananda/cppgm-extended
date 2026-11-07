// N3485 15.4p3-p4 and adopted CWG1330: preserve exception-specification agreement after complete-class lookup.
// expect reject
namespace incompatible {
void g() noexcept(true);
struct GoodTag { static constexpr bool value = true; };
struct BadTag { static constexpr bool value = false; };
template<class T> struct A {
  static constexpr bool x = T::value;
  friend void g() noexcept(x);
};
template struct A<GoodTag>;
template struct A<BadTag>;
}
