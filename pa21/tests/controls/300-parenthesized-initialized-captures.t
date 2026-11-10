// N3648: direct initialized captures keep their initialization and lookup rules.
typedef int Hidden;
int events;
struct Value {
  int value;
  explicit Value(int n): value(n) { events = events * 10 + 1; }
  Value(const Value& other): value(other.value) { events = events * 10 + 2; }
  ~Value() { events = events * 10 + 3; }
};
int fail() { throw 7; }
int main() {
  int value = 7;
  auto first = [Copy(value), &Reference(value), Hidden(3)]() mutable {
    Copy += Hidden; ++Reference; Hidden = 5; return Copy + Reference;
  };
  if (first() != 18 || value != 8) return 1;
  if (first() != 24 || value != 9) return 2;
  {
    Value original(9);
    auto second = [Captured(original)] { return Captured.value; };
    if (second() != 9 || events != 12) return 3;
  }
  if (events != 1233) return 4;
  events = 0;
  try { auto doomed = [Captured(Value(1)), Later(fail())] { return Captured.value + Later; }; return 5; }
  catch (int result) { if (result != 7 || (events != 0 && events % 10 != 3)) return 6; }
  return 0;
}
