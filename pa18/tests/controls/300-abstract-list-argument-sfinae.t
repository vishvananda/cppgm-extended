// N3485 10.4p2, 8.5.4p3, 14.8.2p8: constructing an abstract reference
// temporary in a dependent call fails in immediate context. expect exit 0
struct Abstract { virtual int value() const = 0; };
struct Concrete : Abstract { int value() const { return 7; } };
template<class T> int take(const T&);
template<class T> auto choose(int) -> decltype(take<T>({}), char());
template<class> long choose(...);
static_assert(sizeof(choose<Abstract>(0)) == sizeof(long), "abstract temporary fails substitution");
static_assert(sizeof(choose<Concrete>(0)) == sizeof(char), "concrete temporary remains viable");
int main() { return 0; }
