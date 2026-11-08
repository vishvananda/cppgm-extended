// expect exit 0
// PA17 accepts variable templates in C++11. A consumed outer pack's
// selected type bindings are scalar arguments of the retained member template.
namespace detail { template<class... Types> constexpr int arity=sizeof...(Types); }
template<class... Types> struct Box {
  template<class T> static constexpr int count=detail::arity<T, Types...>;
};
int main() { return Box<>::count<double> != 1; }
