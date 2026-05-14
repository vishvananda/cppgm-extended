// N3485 focus: 6.8 [stmt.ambig] declaration statement ambiguity resolution
int f() {
  int(a);
  a = 1;
  return a;
}
