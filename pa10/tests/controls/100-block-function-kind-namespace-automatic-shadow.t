// N3485 focus: 3.3.1p4, 3.5p6-p9 [basic.scope.declarative/basic.link].
// expect exit 0
namespace Name {}
int main() { int Name = 3; return Name - 3; }
