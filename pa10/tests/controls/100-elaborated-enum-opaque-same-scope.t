// N3485 3.3.1/3.4.4/7.2: elaborated enum references preserve ordinary lookup.
// expect exit 0
enum E : int; int E(int n) { return n; } enum E : int; int main() { return E(3) - 3; }
