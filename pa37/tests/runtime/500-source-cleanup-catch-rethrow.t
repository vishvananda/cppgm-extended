extern int destroyed;
extern int caught;
extern "C" int relay_value(int);
extern "C" int observe(int n) {
 destroyed = 0;
 caught = 0;
 try {
  int value = relay_value(n);
  return value + 100 * destroyed + 10000 * caught;
 } catch (...) {
  return 17 + 100 * destroyed + 10000 * caught;
 }
}
