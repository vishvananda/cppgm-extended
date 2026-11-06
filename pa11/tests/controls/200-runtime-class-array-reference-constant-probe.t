// N3485 8.5.4p3, 12.1: nonliteral array temporaries construct at runtime;
// a failed constant-initialization probe remains bounded. expect exit 0
int calls;
struct Element {
    int value;
    Element() : value(++calls) {}
};
int main()
{
    const Element (&values)[1048576] = {};
    return calls != 1048576 || values[0].value != 1 || values[1048575].value != 1048576;
}
