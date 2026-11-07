// CWG1467/2137; reduced from GCC cpp0x aggregate list-initialization cases; expect reject
struct E {}; int main() { E a; E b{a,a}; }
