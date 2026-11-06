// N3485 8.5p7, 8.5.1p7: omitted subarrays value-initialize every
// literal-class element, including a reference parameter's list. expect exit 0
struct Element {
    int value;
    constexpr Element(int n = 7) : value(n) {}
};
constexpr const Element (&empty)[2][3] = {};
constexpr const Element (&partial)[2][3] = {{{1}, {2}}};
constexpr const Element (&deep)[2][2][3] = {};
constexpr int read(const Element (&values)[2][3])
{
    return values[0][0].value + values[1][2].value;
}
struct Holder { const Element (&values)[2][3]; };
constexpr Holder holder = {{}};
static_assert(empty[0][0].value == 7 && empty[1][2].value == 7, "empty rows");
static_assert(partial[0][0].value == 1 && partial[0][1].value == 2 && partial[1][2].value == 7, "partial rows");
static_assert(deep[1][1][2].value == 7 && holder.values[1][2].value == 7, "nested defaults");
static_assert(read({}) == 14 && read({{{1}}}) == 8, "parameter defaults");
int main() { return empty[1][2].value != 7 || partial[1][2].value != 7 || read({}) != 14; }
