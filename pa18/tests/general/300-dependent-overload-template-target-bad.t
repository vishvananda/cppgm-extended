// VALIDATION: compile-fail
// A concrete target still rejects an overload set with no matching function.
struct Bad { static void call(int); static void call(double); };
template<class T, T value> struct Tag {};
template<class> struct Fn { typedef void (*type)(); };
template<class T> void invoke(Tag<typename Fn<T>::type, &T::call>*);
int main() { invoke<Bad>(0); }
