// N3485 2.14.8 and 4.12: a user-defined literal is a call, whose result decides the condition.
// expect exit 0
constexpr int operator "" _value(unsigned long long n) { return n == 0; }
static_assert(0_value, "call result true");
static_assert(!1_value, "call result false");
int main() { if (1_value) return 1; if (!0_value) return 2; return !(1_value || 0_value); }
