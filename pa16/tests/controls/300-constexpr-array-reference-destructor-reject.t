// N3485 8.5p8, 5.19: actual forbidden calls remain outside constant evaluation. expect reject
struct E { int value; ~E() {} };
constexpr int read(const E (&v)[2]) { return v[1].value; }
static_assert(read({}) == 0, "nontrivial temporary destructor");
int main() { return 0; }
