// N3485 8.5p8, 5.19: value initialization invokes no trivial default constructor. expect exit 0
int calls;
struct E { E() = default; E(const E& other) : value(other.value) { ++calls; } int value; };
const E (&values)[2] = {};
constexpr int read(const E (&v)[2]) { return v[1].value; }
static_assert(values[1].value == 0 && read({}) == 0, "defaulted constructor is not invoked");
int main() { const volatile int* p=&values[1].value;return *p || calls; }
