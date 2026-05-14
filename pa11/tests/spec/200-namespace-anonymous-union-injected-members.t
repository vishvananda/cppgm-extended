// N3485 focus: 9.5 [class.union] anonymous union members
union {
  int t;
  long a;
};

int f(int x) {
  t = x;
  return (int)a;
}
