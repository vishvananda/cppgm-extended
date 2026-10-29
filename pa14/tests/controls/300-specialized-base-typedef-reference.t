// N3485 10.2, 14.6.2: typedef names retain the specialized base context;
// expect exit 0
struct Context { typedef int item_type; };
template<class C> struct Root { typedef typename C::item_type item_type; item_type value; item_type& get() { return value; } };
template<class C> struct Middle : Root<C> {};
typedef Middle<Context> fixed_base;
struct Derived : fixed_base { int run() { item_type& value = get(); value = 9; return value; } };
int main() { Derived object; return object.run() != 9; }
