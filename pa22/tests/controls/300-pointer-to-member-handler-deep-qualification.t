// expect exit 0
// N3485 [conv.qual], [except.handle]: qualifying a member pointer through
// a pointer requires const on the intervening level, even in a catch.
struct S { int member; };
int main()
{
  int S::* member = &S::member;
  int matched = 0;
  try {
    try { throw &member; }
    catch (const int S::**) { return 1; }
  } catch (int S::** result) {
    matched = result == &member;
  }
  if (!matched) return 2;
  try { throw &member; }
  catch (const int S::* const* result) {
    S object;
    object.member = 9;
    return object.**result == 9 ? 0 : 3;
  }
  return 4;
}
