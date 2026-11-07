// N3485 focus: 3.3.1p4, 3.5p6-p9 [basic.scope.declarative/basic.link].
// expect reject
void probe() { void Name(); }
namespace Name {}
int main() { return 0; }
