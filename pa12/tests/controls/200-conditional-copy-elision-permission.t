int direct_constructions;
int copies;
int moves;
int destructions;
bool choose_left = true;

struct Value
{
  int payload;

  explicit Value(int value) : payload(value) { ++direct_constructions; }
  Value(const Value & other) : payload(other.payload) { ++copies; }
  Value(Value && other) : payload(other.payload)
  {
    ++moves;
    other.payload = -1;
  }
  ~Value() { ++destructions; }
};

int main()
{
  {
    Value result = choose_left ? Value(22) : Value(33);
    if (result.payload != 22 || direct_constructions != 1 ||
        copies != 0 || moves > 1 || destructions != moves)
      return 1;
  }
  return direct_constructions == 1 && copies == 0 && moves <= 1 &&
    destructions == moves + 1 ? 0 : 2;
}
