// N3485 8.5p8, 5.19: actual forbidden calls remain outside constant evaluation. expect reject
struct E { E() = default; E(const E& other) : value(other.value) {} int value; };
const E (&values)[2] = {};
constexpr int read(const E (&v)[2]) { return v[1].value; }
static_assert(read({values[0]}) == 0, "non-constexpr copy call");
int main() { return 0; }
