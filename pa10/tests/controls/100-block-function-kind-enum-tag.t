// N3485 focus: 3.3.1p4, 3.5p6-p9 [basic.scope.declarative/basic.link].
// expect exit 0
enum Name { one = 1 };
int Name(int n) { return n; }
int main() { int Name(int); return Name(3) - 2 - one; }
