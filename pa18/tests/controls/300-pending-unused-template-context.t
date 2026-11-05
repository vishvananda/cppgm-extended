// N3485 14.5.6.1: equivalent operands exclude unrelated template arguments.
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
static_assert(!pointer<int, 0>::value, "value parameter stays outside the shared expression");
static_assert(pointer<void, 1>::value, "shared operand keeps its referenced type");
static_assert(callable<void, int>::value, "earlier context remains substitutable");
static_assert(!callable<int, void>::value, "unrelated helper argument does not affect the value");
int main() { return 0; }
