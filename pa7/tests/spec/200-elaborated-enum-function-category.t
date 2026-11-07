// N3485 3.4.4: an elaborated enum reference does not change function lookup.
enum E { value = 1 }; int E(int n) { return n; } int main() { int E(int); enum E object = value; return E(3) - 2 - object; }
