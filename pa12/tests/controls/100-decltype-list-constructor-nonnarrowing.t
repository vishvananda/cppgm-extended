// CXX11-118: unevaluated constructor initialization accepts an exact argument type.
// expect exit 0
struct Target { Target(int); };
int value(int);
typedef decltype(Target{value(0)}) Result;
int main() { return sizeof(Result) != sizeof(Target); }
