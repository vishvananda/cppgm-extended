// expect exit 0
// Omitted data-member pointers require their null representation, including
// the runtime-bound remainder after an explicit member-pointer initializer.
struct M { int a, b; };
int n(int x) { return x; }
int main() {
  int M::** p = new int M::*[n(4)]();
  int M::** q = new int M::*[n(5)]{&M::b};
  int r = 0;
  for (int i = 0; i < 4; ++i) if (p[i] != nullptr) r |= 1;
  if (q[0] != &M::b) r |= 2;
  for (int i = 1; i < 5; ++i) if (q[i] != nullptr) r |= 4;
  delete[] p; delete[] q;
  return r;
}
