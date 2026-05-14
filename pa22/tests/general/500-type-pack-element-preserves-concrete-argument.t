template<class T>
struct remove_reference {
  typedef T type;
};

template<class T>
struct remove_reference<T &> {
  typedef T type;
};

template<class T>
struct remove_reference<T &&> {
  typedef T type;
};

template<class... T>
struct tuple {};

template<unsigned long I, class T>
struct tuple_element;

template<unsigned long I, class... T>
struct tuple_element<I, tuple<T...> > {
  typedef __type_pack_element<I, T...> type;
};

template<unsigned long I, class... T>
typename tuple_element<I, tuple<T...> >::type & get(tuple<T...> &);

template<class T>
T && forward(typename remove_reference<T>::type &);

struct V {};

void consume(V const &);

int main() {
  tuple<V const &> t;
  consume(forward<V const &>(get<0>(t)));
  return 0;
}
