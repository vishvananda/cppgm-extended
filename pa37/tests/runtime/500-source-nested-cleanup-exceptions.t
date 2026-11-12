static int caught;
struct Guard {
 __attribute__((always_inline)) ~Guard() noexcept {
  try { throw 2; }
  catch (int value) { caught += value; }
 }
};
static void raise_outer() {
 Guard guard;
 throw 1;
}
extern "C" int observe() {
 caught = 0;
 try { raise_outer(); }
 catch (int value) { return value == 1 && caught == 2 ? 17 : 1; }
 return 2;
}
