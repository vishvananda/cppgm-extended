// VALIDATION: run-pass
// N3485 focus: ref-qualifier rules plus overload resolution

struct S
{
  int f() & { return 1; }
  int f() && { return 2; }
};

int main()
{
  S s;
  return s.f() == 1 && S().f() == 2 ? 0 : 1;
}
