// N3485 2.14.8 and 4.12: a user-defined literal is a call, whose result decides the condition.
// expect exit 0
int operator "" _integer(const unsigned long long n) { return n == 42 ? 0 : 1; }
int operator "" _floating(long double) { return 0; }
int main() { return 42_integer || 1.5_floating; }
