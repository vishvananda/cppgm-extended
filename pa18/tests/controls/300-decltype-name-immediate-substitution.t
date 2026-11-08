// expect exit 0
// Reduced GCC sfinae7.C (GPL-3.0-or-later); N3485 7.1.6.2/14.8.2.
struct Names { static void f(); static void f(int); typedef int g; };
struct Value { static int f; static int g; };
template<class T> auto first(int) -> decltype(T::f)*;
template<class> char first(...);
template<class T> auto second(int) -> decltype(T::g)*;
template<class> char second(...);
static_assert(sizeof(first<Names>(0)) == 1 && sizeof(second<Names>(0)) == 1, "invalid expression names");
static_assert(sizeof(first<Value>(0)) == sizeof(void*) && sizeof(second<Value>(0)) == sizeof(void*), "value names");
int main() { return 0; }
