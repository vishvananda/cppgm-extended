// expect exit 0
// Reduced GCC sfinae28.C (GPL-3.0-or-later); N3485 14.8.2p8.
struct Complete {}; struct Incomplete;
template<class T> auto pick(T*) -> decltype(T{})*;
template<class T> char pick(T);
static_assert(sizeof(pick((Complete*)0)) == sizeof(void*), "complete temporary");
static_assert(sizeof(pick((Incomplete*)0)) == 1, "incomplete temporary removes candidate");
int main() { return 0; }
