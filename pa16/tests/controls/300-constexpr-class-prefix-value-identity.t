// N3485 5.19, 12.6: shared class payloads preserve independent values, addresses and caller lifetime. expect exit 0
struct Leaf { int value; };
struct Pair { Leaf left, right; };
struct E {
    Pair pair;
    constexpr E(int n) : pair{{n}, {n+1}} {}
};
constexpr const E (&values)[2] = {{3}, {7}};
constexpr int read(const E& e) { return e.pair.left.value + e.pair.right.value; }
static_assert(values[0].pair.left.value == 3 && values[1].pair.left.value == 7, "separate writes");
static_assert(values[0].pair.right.value == 4 && values[1].pair.right.value == 8, "nested caller storage");
static_assert(&values[0].pair.left != &values[0].pair.right && &values[0].pair.left != &values[1].pair.left, "identity");
static_assert(read({11}) == 23 && read({17}) == 35 && read(values[0]) == 7, "call lifetime");
int main() { return values[0].pair.left.value != 3 || values[1].pair.right.value != 8; }
