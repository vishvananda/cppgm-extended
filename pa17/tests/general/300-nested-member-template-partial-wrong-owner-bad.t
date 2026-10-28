// Identically named member templates of distinct owners stay distinct.
template<class T> struct Owner {
  template<class Sequence, int Index> struct Iterator {
    template<class> struct Dereference;
    template<class Other> struct Dereference<Iterator<Other, 0>> {};
  };
};
struct Invalid : Owner<int>::Iterator<void, 0>::Dereference<
    Owner<long>::Iterator<void, 0>> {};
