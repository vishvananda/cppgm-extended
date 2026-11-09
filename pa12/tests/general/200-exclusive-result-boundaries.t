struct Result { long first, second, third; };
struct Input { long value; };
Result make(const Input& input) {
  Result out = {input.value, input.value + 1, input.value + 2};
  return out;
}
Result direct(long n) { return Result{n, n + 1, n + 2}; }
typedef Result (*Maker)(const Input&);
Result invoke(Maker maker, const Input& input) { return maker(input); }
int main() {
  Input input = {13};
  Result a = make(input), b = invoke(make, input), c = direct(17);
  return a.first + a.second + a.third + b.first + b.second + b.third + c.first + c.second + c.third - 138;
}
