// VALIDATION: compile-fail
// N3485 focus: 4.4 [conv.qual]
// Expected: `int **` cannot convert implicitly to `const int **`.

int main()
{
  int value = 0;
  int * p = &value;
  int ** pp = &p;
  const int ** cpp = pp;
  return cpp != 0;
}
