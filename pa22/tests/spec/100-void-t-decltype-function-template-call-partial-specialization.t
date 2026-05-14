// VALIDATION: run-pass
// N3485 focus: 14.8.2 [temp.deduct], 14.8.3 [temp.over]

template<class...>
struct make_void
{
  typedef void type;
};

template<class... Ts>
using void_t = typename make_void<Ts...>::type;

template<class T>
T && declval();

template<class T>
void g(T &);

template<class T, class = void>
struct probe
{
  static const bool value = false;
};

template<class T>
struct probe<T, void_t<decltype(g(declval<T &>()))> >
{
  static const bool value = true;
};

int main()
{
  return probe<int>::value ? 0 : 1;
}
