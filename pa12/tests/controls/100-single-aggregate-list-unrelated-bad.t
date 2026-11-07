// CWG1467/2137; reduced from GCC cpp0x aggregate list-initialization cases; expect reject
struct E {}; struct F {}; int main() { F a; E b{a}; }
