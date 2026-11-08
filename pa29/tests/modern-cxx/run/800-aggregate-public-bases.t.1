struct Empty {};
struct Base { int value; constexpr Base(int n = 5) : value(n) {} };
struct Derived : Empty, Base { int tail = 9; };
constexpr Derived constant{{}, {7}, 11};
static_assert(constant.value == 7 && constant.tail == 11, "constant base initialization");
Derived global{{}, {13}, 17};
struct Plain { int value; };
struct Elided : Plain { int tail; };
int events, failure;
struct First {
  int value;
  First(int n = 1) : value(n) { events = events * 10 + n; }
  ~First() { events = events * 10 + value; }
};
struct Second {
  int value;
  Second(int n = 2) : value(n) { events = events * 10 + n; if (n == failure) throw n; }
  ~Second() { events = events * 10 + value; }
};
struct Lifetime : First, Second { int tail = 5; };
int main() {
  Derived direct{{}, {19}, 23};
  Derived omitted{};
  Elided elided{29, 31};
  if (direct.value != 19 || direct.tail != 23 || omitted.value != 5 || omitted.tail != 9 ||
      global.value != 13 || global.tail != 17 || constant.value != 7 || constant.tail != 11 ||
      elided.value != 29 || elided.tail != 31) return 1;
  { Lifetime value{{1}, {2}, 3}; if (events != 12 || value.tail != 3) return 2; }
  if (events != 1221) return 3;
  events = 0; failure = 2;
  try { Lifetime value{{1}, {2}}; return 4; }
  catch (int value) { if (value != 2 || events != 121) return 5; }
  return 0;
}
