// CXX11-118: independent valid declaration shape from GCC cpp0x/alias-decl-61.C.
// The unused invalid function template is not part of this required control.
// expect exit 0
class Value { typedef int Private; operator Private(); };
struct Result { using Element = Value; };
using Alias = Result;
int main() { return sizeof(Alias::Element) != sizeof(Value); }
