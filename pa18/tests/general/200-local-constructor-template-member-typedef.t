template<bool Cond, class T>
struct EnableIf
{
};

template<class T>
struct EnableIf<true, T>
{
  typedef T type;
};

template<class A, class B>
struct IsSame
{
  enum
  {
    value = __is_same(A, B)
  };
};

template<class From, class To>
struct IsConvertible
{
  enum
  {
    value = __is_convertible(From, To)
  };
};

template<class T>
struct RemoveReference
{
  typedef T type;
};

template<class T>
struct RemoveReference<T &>
{
  typedef T type;
};

template<class T>
struct AddConst
{
  typedef const T type;
};

template<class T>
struct AddLValueReference
{
  typedef T & type;
};

template<class T>
struct MakeConstLValueRef
{
  typedef typename AddLValueReference<
      typename AddConst<typename RemoveReference<T>::type>::type>::type type;
};

template<class T>
struct IteratorTraits;

template<class T>
struct IteratorTraits<T *>
{
  typedef T & reference;
};

template<class Iter>
struct Wrap
{
  typedef typename IteratorTraits<Iter>::reference reference;

  Wrap() {}

  template<class OtherIter,
           typename EnableIf<
               IsConvertible<const OtherIter &, Iter>::value &&
                   (IsSame<reference,
                           typename IteratorTraits<OtherIter>::reference>::value ||
                    IsSame<reference,
                           typename MakeConstLValueRef<
                               typename IteratorTraits<OtherIter>::reference>::type>::value),
               int>::type = 0>
  Wrap(const Wrap<OtherIter> &) {}
};

template<class T>
struct MiniVec
{
  typedef Wrap<T *> iterator;
  typedef Wrap<const T *> const_iterator;
};

int main()
{
  struct ActiveInterval
  {
    int x;
  };

  MiniVec<ActiveInterval>::iterator it;
  MiniVec<ActiveInterval>::const_iterator cit(it);
  return 0;
}
