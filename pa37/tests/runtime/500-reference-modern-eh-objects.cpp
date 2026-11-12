static int destroyed;
struct Guard { ~Guard() { ++destroyed; } };
static int raise(int n) {
 Guard guard;
 if (n) throw n;
 return 0;
}
static int relay(int n) {
 try { return raise(n); }
 catch (int value) {
  if (value == 4) throw;
  return value + destroyed;
 }
}
extern "C" int reference_case(int n) {
 destroyed = 0;
 try {
  int result = relay(n);
  return result + 100 * destroyed;
 }
 catch (...) { return 17 + 100 * destroyed; }
}
