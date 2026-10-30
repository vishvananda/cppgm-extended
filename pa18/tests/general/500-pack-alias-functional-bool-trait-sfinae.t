struct true_type {
  static const bool value = true;
};

struct false_type {
  static const bool value = false;
};

template<bool B, class T = void>
struct enable_if {};

template<class T>
struct enable_if<true, T> {
  typedef T type;
};

template<bool B, class T = void>
using enable_if_t = typename enable_if<B, T>::type;

template<class T, class...>
using first_t = T;

template<class... Bn>
auto and_fn(int) -> first_t<true_type, enable_if_t<bool(Bn::value)>...>;

template<class... Bn>
auto and_fn(...) -> false_type;

template<class... Bn>
struct and_ : decltype(and_fn<Bn...>(0)) {};

template<class T>
struct is_good : true_type {};

struct A {};
struct B {};

static_assert(and_<is_good<A>, is_good<B> >::value, "");
// An unused alias pack must still substitute each enable_if. Check rejection
// as well as success: the ellipsis fallback is the only viable false case.
static_assert(and_<>::value, "empty conjunction");
static_assert(!and_<false_type>::value, "erased alias argument still substitutes");
static_assert(!and_<true_type, false_type>::value, "later pack failure");

int main()
{
  return 0;
}
