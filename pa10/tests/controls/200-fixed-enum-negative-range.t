// expect exit 0
// Valid range-behavior adaptation of GCC cpp0x/enum_base_warn.C.
// Define the helper so correctness does not depend on dead-call elimination.
// N3485 4.5, 4.7 and 5.2.9: fixed unsigned underlying type and promotion.
enum Alpha : unsigned char { ZERO = 0, ONE, TWO, THREE };
Alpha a2;
int calls;
int m1 = -1;
int GetM1() { return m1; }
void link_error() { ++calls; }
int main() {
  a2 = static_cast<Alpha>(GetM1());
  if (a2 == -1) link_error();
  if (-1 == a2) link_error();
  return calls;
}
