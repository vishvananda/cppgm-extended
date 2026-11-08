// expect exit 0
// Reduced GCC sfinae41.C (GPL-3.0-or-later); N3485 14.8.2p8.
struct Pair { Pair(int, int); };
template<class T> auto pick(int) -> decltype(new T(1, 2), char());
template<class> long pick(...);
static_assert(sizeof(pick<int>(0)) == sizeof(long), "scalar rejects multiple arguments");
static_assert(sizeof(pick<Pair>(0)) == 1, "class constructor remains viable");
int main() { return 0; }
