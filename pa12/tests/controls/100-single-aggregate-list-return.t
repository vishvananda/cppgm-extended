// CWG1467/2137; reduced from GCC cpp0x aggregate list-initialization cases; expect exit 0
struct E { int x; }; E f(const E& a) { return {a}; } int main() { E a = {29}; return f(a).x != 29; }
