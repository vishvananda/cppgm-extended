// CWG1467/2137; reduced from GCC cpp0x aggregate list-initialization cases; expect exit 0
struct E { int x; }; constexpr E a = {41}; constexpr E b{a}; static_assert(b.x == 41, "copy aggregate"); int main() { return b.x != 41; }
