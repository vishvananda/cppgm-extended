// CWG1467/2137; reduced from GCC cpp0x aggregate list-initialization cases; expect exit 0
struct B { int x; }; struct D : B { D() { x = 23; } }; int main() { D d; B b{d}; return b.x != 23; }
