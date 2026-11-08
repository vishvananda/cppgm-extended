// expect exit 0
// N3485 14.8.2p8, 13.3.1.7; reduced GCC initlist-explicit-sfinae.C (GPL-3.0-or-later).
struct E { explicit E(int); }; struct I { I(int); };
template<class T> void sink(T);
template<class T> auto pick(int) -> decltype(sink<T>({1}), char());
template<class> long pick(...);
static_assert(sizeof(pick<E>(0)) == sizeof(long), "explicit copy-list is substitution failure");
static_assert(sizeof(pick<I>(0)) == 1, "implicit copy-list is viable");
int main() { return 0; }
