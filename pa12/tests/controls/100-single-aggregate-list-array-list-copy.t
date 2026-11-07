// CWG1467/2137; whole-object and member/list-constructor neighbors; expect exit 0
struct E { int x; }; int main() { E a = {53}; E w[1] = {{a}}; return w[0].x != 53; }
