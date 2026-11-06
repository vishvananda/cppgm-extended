// N3485 8.5.1p7: omitted elements run their required constructors;
// expect exit 0. CXX11-007 neighbor: checking one element type must
// preserve the construction and initialization of every element.
int constructed = 0;
struct Element {
    int value;
    Element() : value(++constructed) {}
};
struct Row { Element elements[3]; };
Row rows[2] = {};

int main()
{
    if (constructed != 6 || rows[0].elements[0].value != 1 || rows[1].elements[2].value != 6)
        return 1;
    Row automatic[2] = {};
    return constructed != 12 || automatic[0].elements[0].value != 7 ||
        automatic[1].elements[2].value != 12;
}
