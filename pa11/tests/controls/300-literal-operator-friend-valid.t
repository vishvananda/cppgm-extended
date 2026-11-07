// N3485 13.5.8p2-p6: validate literal operator declaration scope and parameter forms.
// expect exit 0
int operator "" _friend(unsigned long long);
struct Owner { friend int operator "" _friend(unsigned long long n) { return n != 7; } };
int main() { return 7_friend; }
