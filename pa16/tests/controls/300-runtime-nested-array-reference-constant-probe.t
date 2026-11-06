// N3485 8.5p7: probing nested nonzero constexpr defaults must remain
// bounded when an ordinary reference can initialize at runtime. expect exit 0
struct Element {
    int value;
    constexpr Element(int n = 7) : value(n) {}
};
int main()
{
    const Element (&values)[2][524288] = {};
    return values[0][0].value != 7 || values[1][524287].value != 7;
}
