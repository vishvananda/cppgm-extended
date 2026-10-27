// expect reject
// CWG 1391 excludes a parameter that becomes nondependent only through
// explicit template arguments: signature substitution still instantiates Poison.
template<class T> struct Poison { typedef typename T::missing type; };
struct Item {};
template<class T> typename Poison<T>::type select(T, Item);
int main() { return select<Item>(1, Item()); }
