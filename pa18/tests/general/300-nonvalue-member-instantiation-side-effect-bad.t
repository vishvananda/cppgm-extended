// Class-instantiation side effects are outside a deduction's immediate context.
template<class T> T&& declval();
template<class T> struct Body { typedef decltype(declval<T>().setup()) type; };
template<class T> char select(typename Body<T>::type*);
template<class T> int select(...);
struct Object { typedef int setup; };
static_assert(sizeof(select<Object>(0)) == sizeof(int), "must not hide the class error");
int main() { return 0; }
