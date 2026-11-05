// N3485 8.5.4p3, 8.5.3p5: a single related element binds directly, including
// references to scalars, arrays, functions and a noncopyable class. expect exit 0
struct Value {
  int value;
  explicit Value(int n) : value(n) {}
  Value(const Value&) = delete;
};
Value global(7);
int read(const Value& value = { global }) { return &value == &global ? value.value : 0; }
int scalar(int& value) { ++value; return value; }
int array(int (&value)[2]) { return value[1]; }
int function(int (&value)(int)) { return value(3); }
int twice(int value) { return value * 2; }
int main() {
  int value = 1;
  int values[2] = { 4, 5 };
  if (read() != 7 || read({global}) != 7) return 1;
  if (scalar({value}) != 2 || value != 2) return 2;
  if (array({values}) != 5 || function({twice}) != 6) return 3;
  return 0;
}
