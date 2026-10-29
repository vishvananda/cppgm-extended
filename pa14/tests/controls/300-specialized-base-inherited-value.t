// N3485 10.2: an inherited value hides an outer typedef, so this remains
// a bitwise expression rather than a declaration; expect exit 0
int effects;
typedef int item;
struct Context {};
template<class C> struct Root { enum { item = 7 }; };
template<class C> struct Middle : Root<C> {};
struct Derived : Middle<Context> { void run() { item & ++effects; } };
int main() { Derived object; object.run(); return effects != 1; }
