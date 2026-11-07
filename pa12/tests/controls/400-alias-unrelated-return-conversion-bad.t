// CXX11-118: a required ordinary function body cannot return an unrelated class.
// N3485 6.6.3p2 and 8.5; neighbor of GCC cpp0x/alias-decl-61.C.
// expect reject
class Value { typedef int Private; operator Private(); };
struct Result { using Element = Value; };
using Alias = Result;
Alias convert(int) { return Alias::Element{}; }
