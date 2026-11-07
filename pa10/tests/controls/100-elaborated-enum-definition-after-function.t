// N3485 3.3.1/3.4.4/7.2: elaborated enum references preserve ordinary lookup.
// expect exit 0
int E(int n) { return n; } enum E { value = 2 }; int main() { enum E object = value; return E(5) - object - 3; }
