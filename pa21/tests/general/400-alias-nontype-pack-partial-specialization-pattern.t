// N3485 focus: [temp.class.spec.match] partial specialization matching.
// Alias-template patterns with non-type packs must be expanded before matching
// the aliased class-template-id against the actual specialization arguments.
template<class T, T... I>
struct integer_sequence {};

template<unsigned long... I>
using index_sequence = integer_sequence<unsigned long, I...>;

template<class... T>
using index_sequence_for = index_sequence<0, 1>;

template<class Indices, class... T>
struct tuple_impl;

template<unsigned long... I, class... T>
struct tuple_impl<index_sequence<I...>, T...> {
  enum { value = sizeof...(I) };
};

template<class... T>
struct tuple {
  typedef tuple_impl<index_sequence_for<T...>, T...> BaseT;
  BaseT base;
};

int main()
{
  tuple<int &&, char &&> t;
  (void)t;
  return tuple<int &&, char &&>::BaseT::value == 2 ? 0 : 1;
}
