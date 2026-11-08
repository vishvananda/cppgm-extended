// expect exit 0
// A specialization with access exists; this is not an always-invalid unused template.
template<class T> struct Probe;
class Hidden { int value; friend struct Probe<int>; } hidden;
template<class T> struct Probe {
 template<class U> decltype(hidden.value) select(int) { return 1; }
 template<class U> int select(...) { return 2; }
};
int main() { return Probe<int>().select<void>(0) == 1 && Probe<char>().select<void>(0) == 2 ? 0 : 1; }
