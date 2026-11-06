// CWG 1405: block scope does not make an earlier constexpr object's
// mutable member readable in a later constant evaluation. expect reject
// CXX11-019; reduced from GCC constexpr-mutable6.C.
struct A { mutable int cache = 1 + 1; };
int main()
{
    constexpr A source = {};
    constexpr int value = source.cache;
    return value;
}
