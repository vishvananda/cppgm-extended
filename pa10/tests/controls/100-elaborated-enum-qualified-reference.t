// N3485 3.3.1/3.4.4/7.2: elaborated enum references preserve ordinary lookup.
// expect exit 0
namespace N { enum E { value = 2 }; int E(int n) { return n; } } enum N::E object = N::value; int main() { return N::E(5) - object - 3; }
