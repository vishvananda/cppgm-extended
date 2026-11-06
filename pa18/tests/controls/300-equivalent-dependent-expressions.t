// N3485 14.1, 14.3: unused template arguments do not change dependent defaults.
// expect exit 0
template<class T> struct is_void { static const bool value = false; };
template<> struct is_void<void> { static const bool value = true; };
template<class F, class... Args> struct invoke_result {};
template<class Result, class Ret, bool B = is_void<Ret>::value> struct check { static const bool value = B; };
template<class Ret, class F, class... Args>
struct callable : check<invoke_result<F, Args...>, Ret> {};
template<class T, int Policy, bool = is_void<T>::value> struct access { static const bool value = is_void<T>::value; };
template<class T, int Policy> struct pointer : access<T, Policy> {};
pointer<int, 0> p;
static_assert(!pointer<int, 0>::value, "unused value argument does not change the condition");
static_assert(pointer<void, 1>::value, "condition still depends on the referenced type");
static_assert(callable<void, int>::value, "void result selects the true condition");
static_assert(!callable<int, void>::value, "non-void result selects the false condition");
int main() { return 0; }
