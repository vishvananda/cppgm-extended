// N3485 5.3.4p7: an invalid array extent must not wrap the allocation size.
// Accept throwing failure or a null result from the non-throwing form;
// any observed allocation request must be saturated, never wrapped. expect exit 0
static unsigned long requested = 1;
struct Element {
  int value;
  static void* operator new[](unsigned long n) noexcept { requested = n; return 0; }
  static void operator delete[](void*) noexcept {}
};
volatile unsigned long n = 0x4000000000000001ul;
int main() {
  try { Element* p = new Element[n]; if (p != 0) return 1; }
  catch (...) {}
  return requested == 1 || requested == ~0ul ? 0 : 2;
}
