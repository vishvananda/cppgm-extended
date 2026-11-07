// CWG1467/2137; reduced from GCC cpp0x aggregate list-initialization cases; expect reject
struct E { int x; }; int main() { E a = {3}; E b{{a}}; }
