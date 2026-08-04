#include <type_traits>
struct H { void operator()(int = 0); };
template<class T> struct B {
  template<class... A> typename std::result_of<T(A...)>::type operator()(A&&...);
};
void use(B<H>& value) { value(); }
