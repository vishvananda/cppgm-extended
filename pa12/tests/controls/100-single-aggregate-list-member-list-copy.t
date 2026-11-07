// CWG1467/2137; whole-object and member/list-constructor neighbors; expect exit 0
struct E { int x; }; struct W { E e; }; int main() { E a = {47}; W w = {{a}}; return w.e.x != 47; }
