// N3485 2.14.8 and 4.12: a user-defined literal is a call, whose result decides the condition.
// expect exit 0
int calls;
int operator "" _value(unsigned long long n) { ++calls; return n == 0; }
int main() {
  if (8_value) return 1;
  if (calls != 1) return 2;
  if (!0_value) return 3;
  int right = 0;
  bool a = 8_value && (++right, true);
  bool b = 0_value || (++right, false);
  if (a || !b || right || calls != 4) return 4;
  bool c = 8_value || (++right, true);
  return !c || right != 1 || calls != 5;
}
