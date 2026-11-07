// N3485 6.7: these remain declarations in an ordinary compound statement.
namespace N { int value; }
void f() {
  static_assert(true, "block assertion");
  using namespace N;
  using N::value;
  using T = int;
  for (T i = 0; i != 1; ++i) { static_assert(true, "loop body"); }
}
