// expect exit 0
// N3485 14.1p4: the resolved return type may depend on an earlier template parameter.
template<class T, auto Fn() -> T> struct DependentParameter {};
int main() { return 0; }
