// A failed mutable read in the selected fallback body remains a hard error.
// expect reject. CXX11-019; an invalid default selects the fallback first.
struct A { int fixed; mutable int cache; };
struct Source { static constexpr A object = {7, 8}; };
constexpr A Source::object;
template<class T, int = T::object.cache> char select(int) { return 0; }
template<class T> long select(...) {
    constexpr int value = T::object.cache;
    return value;
}
int main() { return select<Source>(0); }
