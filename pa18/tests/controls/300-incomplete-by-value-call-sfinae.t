// N3485 5.2.2p4, 14.8.2p8: an incomplete class cannot be a by-value
// call argument, including an unevaluated call in an immediate context.
// Regression inspired by GCC PR c++/71495, g++.dg/cpp0x/sfinae57.C,
// GCC 15.2.0 (5115c7e447fc07457443df874bf57840e8316d5f).
// expect exit 0
struct Incomplete;
struct Complete {};
template <class T> T&& source();
template <class T> void consume(T);
template <class T> auto by_value(int) -> decltype(consume<T>(source<T>()), char());
template <class T> long by_value(...);
template <class T> auto by_reference(int) -> decltype(consume<T&>(source<T&>()), char());
template <class T> long by_reference(...);
static_assert(sizeof(by_value<Incomplete>(0)) == sizeof(long), "incomplete parameter discards candidate");
static_assert(sizeof(by_value<Complete>(0)) == sizeof(char), "complete parameter remains viable");
static_assert(sizeof(by_reference<Incomplete>(0)) == sizeof(char), "incomplete reference remains viable");
struct Incomplete {};
static_assert(sizeof(by_value<Incomplete>(0)) == sizeof(char), "completion permits a later call");
int main() { return 0; }
