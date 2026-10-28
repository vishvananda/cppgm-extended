// Rebinding the owner must still reject an unmatched partial pattern.
template<class T> struct Owner {
  template<class Sequence, int Index> struct Iterator {
    template<class> struct Dereference;
    template<class Other> struct Dereference<Iterator<Other, 0>> {};
  };
};
struct Invalid : Owner<int>::Iterator<void, 1>::Dereference<
    Owner<int>::Iterator<void, 1>> {};
