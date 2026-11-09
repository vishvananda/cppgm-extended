// Itanium ABI 2.7: new P[n] for a class with a trivial destructor (and usual
// operator delete[](void*)) has no cookie: operator new[] receives exactly
// n*sizeof(P). Class-specific hooks keep this observable even under allocation
// elision; expect exit 0
static unsigned long requested;
static int released;
alignas(16) static char heap[256];
struct P {
  int a, b;
  static void* operator new[](unsigned long n) { requested = n; return heap; }
  static void operator delete[](void* p) noexcept { released = p == heap; }
};
int main() { volatile int n = 3; P* p = new P[n]; delete[] p; return requested == 3 * sizeof(P) && released ? 0 : 1; }
