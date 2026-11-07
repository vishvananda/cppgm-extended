// N3485 3.3.1/3.4.4/7.2: elaborated enum references preserve ordinary lookup.
// expect exit 0
int E(int n) { return n; } int main() { enum E : int; E* object = 0; return object != 0; }
