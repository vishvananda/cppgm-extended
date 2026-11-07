// N3485 13.5.8p2-p6: validate literal operator declaration scope and parameter forms.
// expect exit 0
static int operator "" _raw(const char* p) { return p[0] != '1' || p[2] != '3'; }
int main() { return 123_raw; }
