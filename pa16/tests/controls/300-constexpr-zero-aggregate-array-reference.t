// N3485 5.19p2, 8.5.1p7: constant reads of aggregate zero tails retain
// the full bound without storing every aggregate value. expect exit 0
struct Element { int value; };
constexpr const Element (&values)[1048576] = {};
static_assert(values[0].value == 0 && values[1048575].value == 0, "zero aggregate tail");
int main() { return values[1048575].value; }
