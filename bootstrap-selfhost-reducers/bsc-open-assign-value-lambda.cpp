#include <type_traits>
#include <utility>

template<class T>
struct Holder {
  template<class U>
  static constexpr bool pred = false;

  template<class From, class ValueT = T, std::enable_if_t<pred<ValueT>, int> = 0>
  static void assign_value(T& lhs, From&& rhs) {
    lhs = std::forward<From>(rhs);
  }

  template<class To, class From, class ValueT = T, std::enable_if_t<!pred<ValueT>, int> = 0>
  static void assign_value(To& lhs, From&& rhs) {
    lhs = std::forward<From>(rhs);
  }

  static void run(T& lhs, T& rhs) {
    auto fn = [](T& a, T& b) { assign_value(a, b); };
    fn(lhs, rhs);
  }
};

int main() {
  int a = 1;
  int b = 2;
  Holder<int>::run(a, b);
  return a == 2 ? 0 : 1;
}
