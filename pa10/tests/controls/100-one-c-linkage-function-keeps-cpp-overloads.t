// expect exit 0
extern "C" int take(int x) { return x + 3; }
int take(char) { return 8; }
int main() { return take(4) == 7 && take('x') == 8 ? 0 : 1; }
