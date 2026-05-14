// VALIDATION: run-pass
// N3485 focus: 14.5.2 [temp.mem], 14.6.1 [temp.local], 14.8 [temp.fct.spec]

template<bool Cond, class T = void>
struct enable_if
{
};

template<class T>
struct enable_if<true, T>
{
  typedef T type;
};

template<class A, class B>
struct is_same
{
  enum
  {
    value = __is_same(A, B)
  };
};

template<class From, class To>
struct is_convertible
{
  enum
  {
    value = __is_convertible(From, To)
  };
};

template<class T>
struct remove_reference
{
  typedef T type;
};

template<class T>
struct remove_reference<T &>
{
  typedef T type;
};

template<class T>
struct add_const
{
  typedef const T type;
};

template<class T>
struct add_lvalue_reference
{
  typedef T & type;
};

template<class T>
struct make_const_lvalue_ref
{
  typedef typename add_lvalue_reference<
      typename add_const<typename remove_reference<T>::type>::type>::type type;
};

template<class T>
struct iterator_traits;

template<class T>
struct iterator_traits<T *>
{
  typedef T & reference;
};

template<class Iter>
struct wrap
{
  typedef typename iterator_traits<Iter>::reference reference;

  wrap() {}

  template<class OtherIter,
           typename enable_if<
               is_convertible<const OtherIter &, Iter>::value &&
                   (is_same<reference,
                            typename iterator_traits<OtherIter>::reference>::value ||
                    is_same<reference,
                            typename make_const_lvalue_ref<
                                typename iterator_traits<OtherIter>::reference>::type>::value),
               int>::type = 0>
  wrap(const wrap<OtherIter> &) {}
};

template<class T>
struct mini_vec
{
  typedef wrap<T *> iterator;
  typedef wrap<const T *> const_iterator;
};

int main()
{
  struct local
  {
    int x;
  };

  mini_vec<local>::iterator it;
  mini_vec<local>::const_iterator cit(it);
  (void)cit;
  return 0;
}
