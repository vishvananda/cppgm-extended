// P1825R0 (P0527/P1155 DR), applied in the course's C++11 mode:
// eligible return operands try rvalue conversion functions first.
// Regression inspired by GCC PR c++/87109, cpp0x/ref-qual19.C and
// ref-qual20.C, GCC 15.2.0, 5115c7e447fc07457443df874bf57840e8316d5f.
// expect exit 0
struct Result { int value; };
struct Source {
  int value;
  operator Result() & { return { value + 1 }; }
  operator Result() && { return { value + 2 }; }
  operator int() & { return value + 3; }
  operator int() && { return value + 4; }
};
Source global_source = { 10 };
Result parameter(Source source) { return (source); }
Result local() { Source source = { 20 }; return source; }
Result rvalue_parameter(Source&& source) { return source; }
Result local_reference(Source& source) { Source&& reference = static_cast<Source&&>(source); return reference; }
Result lvalue_parameter(Source& source) { return source; }
Result global() { return global_source; }
int scalar(Source source) { return source; }
struct ConstSource {
  operator Result() const & { return { 41 }; }
  operator Result() const && { return { 42 }; }
};
Result const_local() { const ConstSource source; return source; }
struct Fallback { operator Result() & { return { 51 }; } };
Result fallback() { Fallback source; return source; }
struct VolatileSource {
  operator Result() volatile & { return { 61 }; }
  operator Result() volatile && { return { 62 }; }
};
Result volatile_local() { volatile VolatileSource source; return source; }
int main() {
  Source source = { 10 };
  if (parameter(source).value != 12 || local().value != 22) return 1;
  if (rvalue_parameter(static_cast<Source&&>(source)).value != 12) return 2;
  if (local_reference(source).value != 12) return 3;
  if (lvalue_parameter(source).value != 11 || global().value != 11) return 4;
  if (scalar(source) != 13) return 5;
  if (const_local().value != 42 || fallback().value != 51) return 6;
  return volatile_local().value == 61 ? 0 : 7;
}
