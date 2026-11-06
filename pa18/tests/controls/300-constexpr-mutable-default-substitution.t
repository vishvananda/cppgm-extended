// Mutable reads fail substitution in an immediate default-argument context;
// nonmutable reads remain viable. expect exit 0
// CXX11-019; CWG1405 with N3485 14.8.2p8.
struct A { int fixed; mutable int cache; };
struct Source { static constexpr A object = {7, 8}; };
constexpr A Source::object;
template<class T, int = T::object.cache> char select(int);
template<class T> long select(...);
template<class T, int = T::object.fixed> char valid(int);
template<class T> long valid(...);
static_assert(sizeof(select<Source>(0)) == sizeof(long), "mutable default fails substitution");
static_assert(sizeof(valid<Source>(0)) == sizeof(char), "nonmutable default remains viable");
int main() { return 0; }
