// N3485 5.19p2, 8.5.4p3, 12.2p5: reference member and literal-class
// temporary lists retain the constructor and array facts. expect exit 0
struct Holder { const int (&values)[3]; };
constexpr Holder holder = {{1, 2}};
struct Element {
    int value;
    constexpr Element(int v = 0) : value(v) {}
};
constexpr const Element &one = {3};
constexpr const Element &direct{4};
constexpr const Element (&elements)[3] = {{5}, {6}};
struct Bits { unsigned low : 3; unsigned high : 5; int value; };
constexpr const Bits (&bits)[33] = {{3, 5, 9}, {7, 1, 11}};
constexpr int get(const Element &value) { return value.value; }
static_assert(holder.values[0] == 1 && holder.values[1] == 2 && holder.values[2] == 0, "member");
static_assert(one.value == 3 && direct.value == 4 && get({7}) == 7, "class temporary");
static_assert(elements[0].value == 5 && elements[1].value == 6 && elements[2].value == 0, "class array");
static_assert(bits[0].low == 3 && bits[0].high == 5 && bits[1].value == 11 && bits[32].high == 0, "bit-field array");
int main() { return holder.values[1] != 2 || one.value != 3 || elements[2].value != 0 || bits[0].high != 5 || bits[32].value != 0; }
