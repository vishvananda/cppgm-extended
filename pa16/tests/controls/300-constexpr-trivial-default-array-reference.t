// N3485 8.5p8, 5.19: value initialization invokes no trivial default constructor. expect exit 0
struct Empty {};
struct E : Empty { int value; };
const E (&values)[2] = {};
constexpr const E (&constant)[2] = {};
constexpr const E (&&rvalue)[2] = {};
const E (&matrix)[2][3] = {};
constexpr int read(const E (&v)[2]) { return v[1].value; }
static_assert(values[0].value == 0 && constant[1].value == 0 && rvalue[1].value == 0, "trivial zero initialization");
static_assert(matrix[1][2].value == 0 && read({}) == 0, "nested and parameter");
static_assert(&values[0] != &values[1], "identity");
int main() { const volatile int* p=&matrix[1][2].value;return *p; }
