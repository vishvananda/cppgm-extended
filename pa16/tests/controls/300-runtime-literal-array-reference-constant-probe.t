// N3485 7.1.5, 8.5.4p3: a literal type can still require runtime calls
// in its prefix or default construction. Probe those before the tail. expect exit 0
int calls;
int next() { return ++calls; }
struct Element {
    int value;
    constexpr Element(int n) : value(n) {}
    Element() : value(next()) {}
};
int prefix()
{
    const Element (&values)[1048576] = {next()};
    return calls != 1048576 || values[0].value != 1 || values[1048575].value != 1048576;
}
int defaults()
{
    const Element (&values)[1048576] = {};
    return calls != 1048576 || values[0].value != 1 || values[1048575].value != 1048576;
}
int main()
{
    if (prefix()) return 1;
    calls = 0;
    return defaults();
}
