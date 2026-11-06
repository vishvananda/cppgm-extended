// N3485 7.1.5, 8.5.4p3: an ordinary reference does not require a dense
// constant-evaluation image of every nonzero default. expect exit 0
struct Element {
    int value;
    constexpr Element() : value(7) {}
};
int main()
{
    const Element (&values)[1048576] = {};
    return values[0].value != 7 || values[1048575].value != 7;
}
