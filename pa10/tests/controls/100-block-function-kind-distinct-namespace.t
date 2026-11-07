// N3485 focus: 3.3.1p4, 3.5p6-p9 [basic.scope.declarative/basic.link].
// expect exit 0
namespace Name {}
namespace Other { int Name() { return 3; } int probe() { int Name(); return Name(); } }
int main() { return Other::probe() - 3; }
