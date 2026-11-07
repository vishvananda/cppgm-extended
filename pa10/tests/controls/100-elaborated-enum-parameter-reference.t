// N3485 3.3.1/3.4.4/7.2: elaborated enum references preserve ordinary lookup.
// expect exit 0
enum E { value = 2 }; int E(int n) { return n; } int read(enum E object) { return E(5) - object; } int main() { return read(value) - 3; }
