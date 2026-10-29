// N3485 14.6: fixed bases remain available while defining an unused template;
// its member body does not have to be instantiated to classify the name; expect exit 0
struct Context { typedef int item_type; };
template<class C> struct Root { typedef typename C::item_type item_type; item_type value; item_type& get() { return value; } };
template<class C> struct Middle : Root<C> {};
template<class T> struct Unused : Middle<Context> { void run() { item_type& value = get(); value = 3; } };
int main() { return 0; }
