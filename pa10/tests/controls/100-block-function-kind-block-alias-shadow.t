// N3485 focus: 3.3.1p4,7.3.2: a block namespace alias targets only that block.
// expect exit 0
namespace Original { int value = 3; }
void probe() { void Name(); }
int main() { namespace Name = Original; return Name::value - 3; }
