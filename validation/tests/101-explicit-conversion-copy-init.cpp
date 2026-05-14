// VALIDATION: compile-fail
// N3485 focus: 12.3.2 [class.conv.fct], 8.5 [dcl.init]
// Expected: copy-initialization from S to bool is ill-formed because the
// conversion operator is explicit.

struct S
{
  explicit operator bool() const
  {
    return true;
  }
};

bool test()
{
  S s;
  bool b = s;
  return b;
}

int main()
{
  return 0;
}
