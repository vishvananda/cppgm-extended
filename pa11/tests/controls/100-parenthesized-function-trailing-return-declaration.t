// expect exit 0
int value() { return 9; }
int main() { auto (value)() -> int; return value() == 9 ? 0 : 1; }
