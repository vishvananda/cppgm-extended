// P1825R0, adopted as a DR by N4829 CWG motion 23 (July 2019).
// Related: clang/test/SemaCXX/P1155.cpp, LLVM21.1.8,2078da43e25a4623cab2d0d60decddf709aaea28.
// expect reject
struct Value {
  int tag;
  Value() : tag(1) {}
  Value(const Value&) : tag(2) {}
private:
  Value(Value&&) : tag(3) {}
};
void act(Value& input) { try { throw input; } catch (Value value) { throw value; } }
