// expect exit 0
// Different array element types keep ordinary conversion ranking, not bound preference.
int pick(const int (&)[3]) { return 3; } int pick(const long (&)[2]) { return 2; }
int main() { int a[3] = {}; return pick({1,2}) == 3 && pick(a) == 3 ? 0 : 1; }
