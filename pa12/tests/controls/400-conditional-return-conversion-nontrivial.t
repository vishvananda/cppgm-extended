// Return a conversion function's nontrivial class result from a glvalue conditional.
// expect exit 0
int live, conversions;
struct Result {
  int value;
  Result(int v) : value(v) { ++live; }
  Result(const Result& other) : value(other.value) { ++live; }
  ~Result() { --live; }
};
struct Source {
  int value;
  operator Result() const { ++conversions; return Result(value); }
};
Result convert(bool select, const Source& a, const Source& b) {
  return select ? a : b;
}
int main() {
  Source a = {17}, b = {29};
  {
    Result first = convert(true, a, b);
    Result second = convert(false, a, b);
    if (first.value != 17 || second.value != 29 || conversions != 2 || live != 2) return 1;
  }
  return live == 0 ? 0 : 2;
}
