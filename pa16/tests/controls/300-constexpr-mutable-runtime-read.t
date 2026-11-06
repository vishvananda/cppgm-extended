// Ordinary execution must read the current mutable value after a write.
// expect exit 0. CXX11-019 neighbor; no required constant-expression read.
struct A { int fixed; mutable int cache; };
constexpr A source = {7, 8};
int main()
{
    source.cache = 9;
    return source.fixed != 7 || source.cache != 9;
}
