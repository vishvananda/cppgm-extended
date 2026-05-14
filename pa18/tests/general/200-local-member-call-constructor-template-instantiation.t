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
  Iter p;

  Wrap() : p(0) {}
  explicit Wrap(Iter q) : p(q) {}

  template<class OtherIter,
           typename EnableIf<
               IsConvertible<const OtherIter &, Iter>::value &&
                   (IsSame<reference,
                           typename IteratorTraits<OtherIter>::reference>::value ||
                    IsSame<reference,
                           typename MakeConstLValueRef<
                               typename IteratorTraits<OtherIter>::reference>::type>::value),
               int>::type = 0>
  Wrap(const Wrap<OtherIter> & other) : p(other.p) {}
};

template<class Iter>
Wrap<Iter> operator+(Wrap<Iter> it, int)
{
  return it;
}

template<class T>
struct MiniVec
{
  typedef T * pointer;
  typedef const T * const_pointer;
  typedef Wrap<pointer> iterator;
  typedef Wrap<const_pointer> const_iterator;

  iterator begin() { return iterator(); }
  int erase(const_iterator) { return 0; }
};

template<class T>
int call_erase(MiniVec<T> & active, int ai)
{
  return active.erase(active.begin() + ai);
}

int main()
{
  struct ActiveInterval
  {
    int x;
  };

  MiniVec<ActiveInterval> active;
  return call_erase(active, 0);
}
