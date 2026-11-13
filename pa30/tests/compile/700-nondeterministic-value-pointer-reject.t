// __builtin_nondeterministic_value takes a fundamental or vector operand.
int* p;
int* f() { return __builtin_nondeterministic_value(p); }
int main() { return 0; }
