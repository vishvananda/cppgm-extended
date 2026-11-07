// N3485 15.4p3-p4: a used bad specialization must diagnose incompatible friend redeclaration.
// A<GoodTag> preserves a valid specialization; the unused-template IFNDR exception does not apply.
// expect reject
namespace incompatible {
void g() noexcept(true);
struct GoodTag { static constexpr bool value = true; };
struct BadTag { static constexpr bool value = false; };
template<class T> struct A {
  friend void g() noexcept(x);
  static constexpr bool x = T::value;
};
template struct A<GoodTag>;
template struct A<BadTag>;
}
