// N3485 7.2p4: define an opaque namespace enumeration in an enclosing scope.
// expect exit 0
namespace n { enum E : int; }
enum n::E : int { zero };
int main() { return n::zero; }
