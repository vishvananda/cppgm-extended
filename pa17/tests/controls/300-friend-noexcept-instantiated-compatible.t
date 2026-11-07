// N3485 15.4p3-p4 and adopted CWG1330: preserve exception-specification agreement after complete-class lookup.
// expect exit 0
namespace good { void g() noexcept; struct GoodTag { static constexpr bool value=true; }; template<class T> struct A { friend void g() noexcept(x); static constexpr bool x=T::value; }; template struct A<GoodTag>; void g() noexcept {} } int main() { good::g(); return noexcept(good::g()) ? 0 : 1; }
