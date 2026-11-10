// A narrow select condition needs only its own bits. Wider readers of
// the same parameter still need its sign or zero extension.
template<class T> __attribute__((noinline))
long choose(T condition, long yes, long no) {
  return condition ? yes : no;
}
template<class T> __attribute__((noinline))
long widening_choice(T condition) {
  return condition ? static_cast<long>(condition) : 37;
}
template<class T> __attribute__((noinline))
long choice_and_wide(T condition, long yes, long no) {
  long selected = condition ? yes : no;
  return selected + static_cast<long>(condition);
}
template<class T> bool check(T condition, long expected) {
  const long selected = condition ? 11 : 29;
  return choose(condition, 11, 29) == selected &&
         widening_choice(condition) == (condition ? expected : 37) &&
         choice_and_wide(condition, 11, 29) == selected + expected;
}
int main() {
  for (int n = -128; n != 256; ++n) {
    const signed char s = static_cast<signed char>(n);
    const unsigned char u = static_cast<unsigned char>(n);
    if (!check(s, static_cast<long>(s)) || !check(u, static_cast<long>(u)))
      return 1;
  }
  const long values[] = {0, 1, 127, 128, 255, 256, 32767, 32768,
                         65535, 65536, 2147483647L, 2147483648L,
                         4294967295L};
  for (unsigned n = 0; n != sizeof(values)/sizeof(values[0]); ++n) {
    unsigned short h = static_cast<unsigned short>(values[n]);
    short s = static_cast<short>(values[n]);
    unsigned w = static_cast<unsigned>(values[n]);
    int i = static_cast<int>(values[n]);
    bool b = values[n] != 0;
    if (!check(h, static_cast<long>(h)) || !check(s, static_cast<long>(s)) ||
        !check(w, static_cast<long>(w)) || !check(i, static_cast<long>(i)) ||
        !check(b, static_cast<long>(b)))
      return 2;
  }
  return 0;
}
