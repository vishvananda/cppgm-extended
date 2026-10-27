// N3485 5.2.3p1, 5.2.9p6, 5p11: void(e) evaluates its discarded operand.
// expect exit 0
int calls;
int effect() { return ++calls; }
int main() {
  int i = 0, j = 0;
  typedef void Discard;
  void(++i);
  Discard(j++);
  void((++i, ++j));
  (void(effect()));
  (void)effect();
  static_cast<void>(effect());
  if (i != 2 || j != 2 || calls != 3) return 1;
  volatile int value = 0;
  void(++value);
  if (value != 1) return 2;
  int data[3] = { 2, 3, 5 };
  int *p = data;
  int index = 0, turns = 0, sum = 0;
  for (; index < 3; void(++index), void(++p)) {
    if (++turns > 3) return 3;
    sum += *p;
  }
  void();
  return index == 3 && p == data + 3 && sum == 10 ? 0 : 4;
}
