// Implicit guides use constructor default availability without evaluating it.
int scalar_calls;
int class_calls;
int next() { return ++scalar_calls; }
struct Default { Default() { ++class_calls; } };
struct Instruction {};
struct Compare : Instruction {};
template<class Policy = int, class Callback = long> struct Builder {
  int value;
  Builder(Instruction*, void* = nullptr, Default = {}): value(next()) {}
};
template<class T> struct Deduced {
  T value;
  int count;
  Deduced(T v, int n = next()): value(v), count(n) {}
};
template<class T> struct Lazy {
  Lazy(T, int = sizeof(typename T::missing)) {}
};
int main() {
  Compare instruction;
  Builder first(&instruction);
  if (scalar_calls != 1 || class_calls != 1 || first.value != 1) return 1;
  Builder<> second(&instruction, nullptr, Default{});
  if (scalar_calls != 2 || class_calls != 2 || second.value != 2) return 2;
  Deduced third(7);
  if (third.value != 7 || third.count != 3 || scalar_calls != 3) return 3;
  Deduced fourth(8, 42);
  if (fourth.count != 42 || scalar_calls != 3) return 4;
  Lazy unused_default(1, 0);
  return 0;
}
