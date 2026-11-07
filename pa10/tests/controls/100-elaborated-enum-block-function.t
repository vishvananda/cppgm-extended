// N3485 3.3.1/3.4.4/7.2: elaborated enum references preserve ordinary lookup.
// expect exit 0
enum E { value = 1 }; int E(int n) { return n; } int main() { int E(int); enum E object = value; return E(3) - 2 - object; }
