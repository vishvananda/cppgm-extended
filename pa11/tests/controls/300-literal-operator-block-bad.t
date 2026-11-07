// N3485 13.5.8p2-p6: validate literal operator declaration scope and parameter forms.
// expect reject
int main() { int operator "" _bad(unsigned long long); return 0; }
