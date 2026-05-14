// VALIDATION: run-pass
// N3485 focus: 12.6.2 [class.base.init]

struct Value
{
  int x;

  Value() : Value(7) {}
  Value(int v) : x(v) {}
};

int main()
{
  Value v;
  return v.x == 7 ? 0 : 1;
}
