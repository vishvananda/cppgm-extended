// expect exit 0
// Reduced GCC sfinae68.C (GPL-3.0-or-later): substitute the empty pack's parameter type.
template<bool, class T = int> struct Enable {};
template<class T> struct Enable<true, T> { typedef T type; };
template<int I, typename Enable<I == 0>::type...> char pick();
template<int I, typename Enable<I != 0>::type...> long pick();
static_assert(sizeof(pick<0>()) == 1, "first empty pack");
static_assert(sizeof(pick<1>()) == sizeof(long), "second empty pack");
int main() { return 0; }
