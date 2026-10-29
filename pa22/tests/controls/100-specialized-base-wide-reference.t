// N3485 10.2, 14.6.2: the indexed wide-base path preserves a specialized
// target just as the small-base scan does; expect exit 0
struct Context { typedef int item_type; };
template<class C> struct Root { typedef typename C::item_type item_type; item_type value; item_type& get() { return value; } };
template<class C> struct Middle : Root<C> {};
struct A0 {}; struct A1 {}; struct A2 {}; struct A3 {}; struct A4 {};
struct A5 {}; struct A6 {}; struct A7 {}; struct A8 {};
struct Derived : Middle<Context>, A0, A1, A2, A3, A4, A5, A6, A7, A8 {
  int run() { item_type& value = get(); value = 13; return value; }
};
int main() { Derived object; return object.run() != 13; }
