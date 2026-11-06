// N3485 8.5p8: an optional probe must leave runtime construction usable. expect exit 0
int next() { return 7; }
struct E {
    int value;
    E() = default;
    E(int n) : value(n) {}
};
const E (&values)[1048576] = {next()};
static_assert(sizeof(values) == 4194304ULL, "full span");
int main() { return values[0].value != 7 || values[1048575].value != 0; }
