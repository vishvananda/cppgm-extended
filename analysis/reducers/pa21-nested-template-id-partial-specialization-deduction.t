template<class First, class Second>
struct pair_like {};

namespace lib {

enum tree_type_enum { red_black_tree };

namespace detail {
struct empty_hook {};
struct fallback {};

template<class Node>
struct node_value {
  typedef fallback type;
};

template<class VoidPointer, tree_type_enum TreeType, bool OptimizeSize>
struct hook;

template<class VoidPointer, bool OptimizeSize>
struct hook<VoidPointer, red_black_tree, OptimizeSize> {
  typedef empty_hook type;
};
}

template<class T, class HookDefiner, bool PairBased = false>
struct base_node : HookDefiner::type {
  typedef T value_type;
};

namespace detail {
template<class T, class VoidPointer, tree_type_enum TreeType, bool OptimizeSize>
struct node_value<base_node<T, hook<VoidPointer, TreeType, OptimizeSize>, true> > {
  typedef T type;
};
}

}

typedef lib::base_node<
    pair_like<int, char>,
    lib::detail::hook<void *, lib::red_black_tree, true>,
    true> node_t;
typedef lib::detail::node_value<node_t>::type value_type;

char check(pair_like<int, char> *);
int check(...);

static_assert(sizeof(check((value_type *)0)) == sizeof(char), "");

int main()
{
  return 0;
}
