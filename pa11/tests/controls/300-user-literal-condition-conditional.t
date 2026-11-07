// N3485 2.14.8 and 4.12: a user-defined literal is a call, whose result decides the condition.
// expect exit 0
int calls;
int operator "" _value(unsigned long long n) { ++calls; return n == 0; }
int main() { int x = (0_value) ? 2 : 3; return x != 2 || calls != 1; }
