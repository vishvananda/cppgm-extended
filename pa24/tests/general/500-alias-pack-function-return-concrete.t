template<class... T>
struct tuple {};

template<unsigned long I, class T>
struct tuple_element;

template<unsigned long I, class... T>
struct tuple_element<I, tuple<T...> > {
  typedef __type_pack_element<I, T...> type;
};

template<unsigned long I, class T>
using tuple_element_t = typename tuple_element<I, T>::type;

template<unsigned long I, class... T>
tuple_element_t<I, tuple<T...> > & get(tuple<T...> &);

struct First {};
struct Second {};

void consume(Second &);

int main() {
  tuple<First, Second> t;
  consume(get<1>(t));
  return 0;
}
