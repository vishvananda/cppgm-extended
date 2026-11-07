// CWG1467/2137; reduced from GCC cpp0x aggregate list-initialization cases; expect reject
struct E { E() = default; E(const E&) = delete; }; int main() { E a; E b{a}; }
