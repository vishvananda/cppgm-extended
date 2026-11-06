// N3485 5.19, 12.2: completed temporary storage is reclaimed across constexpr calls. expect exit 0
struct S { int value, other; constexpr S(int n) : value(n), other(n) {} };
constexpr unsigned sum(int n) { return n == 0 ? S(1).value : sum(n-1) + sum(n-1); }
static_assert(sum(18) == 262144u, "value");
int main(){}
