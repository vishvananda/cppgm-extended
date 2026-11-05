// N3485 7.3.3: importing identical base signatures is valid before a call.
// expect exit 0
struct First { void f() = delete; };
struct Second : First { void f() = delete; static int f(int) { return 7; } };
struct Derived : Second { using First::f; using Second::f; };
int main() { return Derived::f(0) != 7; }
