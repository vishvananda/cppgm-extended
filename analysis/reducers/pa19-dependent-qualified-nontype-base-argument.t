template<bool B>
struct bool_constant {
  static const bool value = B;
};

template<class T, bool Cached>
struct Node {
  typedef T value_type;
};

template<class Alloc, class T>
struct Rebind {
  typedef T value_type;
};

template<class NodeAlloc>
struct AllocBox {
  typedef NodeAlloc node_alloc;
};

template<class Traits>
struct Table
  : AllocBox<Rebind<int, Node<int, Traits::hash_cached::value> > > {
  typedef typename AllocBox<Rebind<int, Node<int, Traits::hash_cached::value> > >::node_alloc node_alloc;
};

struct Traits {
  typedef bool_constant<false> hash_cached;
};

int accept(AllocBox<Rebind<int, Node<int, false> > > *);

int main() {
  Table<Traits> *table = 0;
  return accept(table);
}
