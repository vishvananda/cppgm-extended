// expect exit 0
// A non-list array argument uses the ordinary reference-binding rules.
int pick(const int (&)[2]) { return 2; } int pick(const int (&)[3]) { return 3; }
int main() { int a[2] = {}; int b[3] = {}; return pick(a) == 2 && pick(b) == 3 ? 0 : 1; }
