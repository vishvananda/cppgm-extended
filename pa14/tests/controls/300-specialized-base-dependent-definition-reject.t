// N3485 14.6.2.3: unresolved dependent bases are excluded from unqualified
// lookup at template definition time, even if a fixed instantiation has the type; expect reject
int storage; int& get() { return storage; }
struct Context { typedef int item_type; };
template<class C> struct Root { typedef typename C::item_type item_type; };
template<class C> struct Middle : Root<C> {};
template<class C> struct Invalid : Middle<C> { void run() { item_type& value = get(); } };
int main() { return 0; }
