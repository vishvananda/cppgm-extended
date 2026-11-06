// P1825R0, adopted as a DR by N4829 CWG motion 23 (July 2019).
// Related: clang/test/SemaCXX/P1155.cpp, LLVM21.1.8,2078da43e25a4623cab2d0d60decddf709aaea28.
// expect exit 0
struct Value { int tag; Value() : tag(1) {} Value(Value&) : tag(2) {} Value(const Value&&) : tag(4) {} Value(volatile Value&&) : tag(7) {} };
void act(Value value) { throw value; }
int main() {
  Value input;
  try { act(input); }
  catch (const Value& out) { return (out.tag == 2) ? 0 : 1; }
  return 2;
}
