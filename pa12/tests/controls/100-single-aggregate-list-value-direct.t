// CWG1467/2137; reduced from GCC cpp0x aggregate list-initialization cases; expect exit 0
struct E { int x; }; int main() { E a = {17}; E b{a}; return b.x != 17; }
