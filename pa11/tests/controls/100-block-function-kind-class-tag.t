// N3485 focus: 3.3.1p4, 3.5p6-p9 [basic.scope.declarative/basic.link].
// expect exit 0
struct Name {};
int Name(int n) { return n; }
int main() { int Name(int); struct Name *p = 0; return p == 0 ? Name(3) - 3 : 1; }
