// CWG1467/2137; reduced from GCC cpp0x aggregate list-initialization cases; expect exit 0
struct E { int x; }; constexpr E b{E{43}}; static_assert(b.x == 43, "elided aggregate"); int main() { return b.x != 43; }
