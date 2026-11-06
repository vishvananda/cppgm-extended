// P1825R0, adopted as a DR by N4829 CWG motion 23 (July 2019).
// Related: clang/test/SemaCXX/P1155.cpp, LLVM21.1.8,2078da43e25a4623cab2d0d60decddf709aaea28.
// expect reject
struct Value {
  int tag;
  Value() : tag(1) {}
  Value(const Value&) = delete;
  Value(Value&&) : tag(3) {}
};
template<class T> void act(T value) try { throw value; } catch (...) { throw; }
int main() { Value input; act(static_cast<Value&&>(input)); }
