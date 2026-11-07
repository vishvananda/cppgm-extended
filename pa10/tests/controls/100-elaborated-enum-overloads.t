// N3485 3.3.1/3.4.4/7.2: elaborated enum references preserve ordinary lookup.
// expect exit 0
enum E { value = 2 }; int E(int n) { return n; } int E(long n) { return int(n) + 1; } int main() { enum E object = value; return E(3) + E(4L) - object - 6; }
