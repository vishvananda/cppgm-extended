// CWG1467/2137; reduced from GCC cpp0x aggregate list-initialization cases; expect exit 0
struct E {}; int main() { E a; E b{a}; (void)b; return 0; }
