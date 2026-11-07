// N3485 13.5.8p2-p6: validate literal operator declaration scope and parameter forms.
// expect exit 0
int operator "" _integer(const unsigned long long n) { return n == 42 ? 0 : 1; }
int operator "" _floating(long double) { return 0; }
int main() { return 42_integer != 0 || 1.5_floating != 0; }
