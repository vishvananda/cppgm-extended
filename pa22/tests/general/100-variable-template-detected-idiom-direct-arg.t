// VALIDATION: run-pass
// N3485 focus: 14.5.6 [temp.var], 14.8.2 [temp.deduct], 14.8.3 [temp.over]
// Direct fixed arguments after a dependent decltype pattern must be deduced
// before the dependent pattern is checked.

template<class T>
T&& declval();

template<class, class A, class... Args>
inline const bool has_make_impl = false;

template<class A, class... Args>
inline const bool has_make_impl<
    decltype((void)declval<A>().make(declval<Args>()...)), A, Args...> = true;

template<class A, class... Args>
inline const bool has_make_v = has_make_impl<void, A, Args...>;

struct maker
{
  void make(int *, int) {}
};

int main()
{
  return has_make_v<maker, int *, int> ? 0 : 1;
}
