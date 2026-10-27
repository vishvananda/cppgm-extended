// VALIDATION: compile-pass
// Resolve an overloaded address after substituting its dependent pointer target.
int calls;
struct Good {
  static void call() { ++calls; }
  static void call(int) { calls = 100; }
};
struct Bad { static void call(int); static void call(double); };
template<class T, T value> struct Tag { static void run() { value(); } };
template<class> struct Fn { typedef void (*type)(); };
template<class T> char probe(Tag<typename Fn<T>::type, &T::call>*);
template<class> long probe(...);
static_assert(sizeof(probe<Good>(0)) == sizeof(char), "matching address");
static_assert(sizeof(probe<Bad>(0)) == sizeof(long), "substitution failure");
template<class T> void invoke(Tag<typename Fn<T>::type, &T::call>*) {
  Tag<typename Fn<T>::type, &T::call>::run();
}
int main() { invoke<Good>(0); return calls == 1 ? 0 : 1; }
