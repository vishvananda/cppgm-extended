// N3485 focus: 14.8.2 substitution failure in function-template overloads.
// Internal hosted type-transform aliases must be resolved as concrete unary
// transforms instead of forcing template-id text fallback while checking SFINAE.
template<bool B, class T = void>
struct enable_if {};

template<class T>
struct enable_if<true, T> {
  typedef T type;
};

template<bool B, class T = void>
using enable_if_t = typename enable_if<B, T>::type;

template<class T, class U>
struct is_not_same {
  static const bool value = true;
};

template<class T>
using __remove_cvref_t = __remove_cvref(T);

struct true_type {
  static const bool value = true;
};

struct false_type {
  static const bool value = false;
};

template<class...>
using expand_to_true = true_type;

template<class... Pred>
expand_to_true<enable_if_t<Pred::value>...> and_helper(int);

template<class...>
false_type and_helper(...);

template<class... Pred>
using and_t = decltype(and_helper<Pred...>(0));

template<class R, class... Args>
struct function;

template<class R, class... Args>
struct function<R(Args...)> {
  template<class F>
  using enable_callable =
      enable_if_t<and_t<is_not_same<__remove_cvref_t<F>, function>,
                        is_not_same<F, R>>::value>;

  template<class F, class = enable_callable<F>>
  function & operator=(F);
};

struct S {
  function<void(int)> f;

  void g()
  {
    struct G {
      void operator()(int) const {}
    };
    f = G();
  }
};

int main()
{
  S s;
  s.g();
}
