// Reduced from Boost.Core type_name. Ref-qualified function-type partial
// specializations must keep the function suffix instead of collapsing to the
// result type.

template<class T>
struct holder {
  static const int value = 0;
};

template<class R, class... A>
struct holder<R(A...)> {
  static const int value = 1;
};

template<class R, class... A>
struct holder<R(A...) &> {
  static const int value = 2;
};

template<class R, class... A>
struct holder<R(A...) const &> {
  static const int value = 3;
};

template<class R, class... A>
struct holder<R(A...) &&> {
  static const int value = 4;
};

static_assert(holder<void()>::value == 1, "");
static_assert(holder<void() &>::value == 2, "");
static_assert(holder<void() const &>::value == 3, "");
static_assert(holder<void() &&>::value == 4, "");

int main()
{
  return holder<void() &>::value == 2 ? 0 : 1;
}
