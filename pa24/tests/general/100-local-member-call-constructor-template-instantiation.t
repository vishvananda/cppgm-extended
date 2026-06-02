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

  template<class OtherIter>
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
