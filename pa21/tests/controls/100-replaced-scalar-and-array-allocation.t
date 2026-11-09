// N3485 17.6.4.6p2, 18.6.1.1, 18.6.1.2: a program may replace operator
// new, operator new[], operator delete and operator delete[] together; each
// retained allocation/deallocation uses the program's function for its form;
// replaceable global calls may be elided. A class's operator new (12.5) is
// still found first for its class. 5.3.4p18: when the nested new V[1] in T's initializer throws, T's
// storage is freed by the replaced operator delete. In the program's
// LowIR (pa8/lowir.md) each singleton role has one owner: the scalar forms
// own allocate_memory and free_memory, and the replaced array forms have no
// role. expect exit 0
typedef decltype(sizeof(0)) size_type;
alignas(16) static char pool[1024];
static size_type used;
static int scalars, arrays, members;
static void* take(size_type n) { void* p = pool + used; used += (n + 15) / 16 * 16; return p; }
void* operator new(size_type n) { ++scalars; return take(n); }
void operator delete(void* p) noexcept { if (p) --scalars; }
void* operator new[](size_type n) { ++arrays; return take(n); }
void operator delete[](void* p) noexcept { if (p) --arrays; }
struct V { V() { throw 1; } };
struct T { int v; T(int x) : v(x) {} };
struct M {
  int m;
  static void* operator new(size_type n) { ++members; return take(n); }
  static void operator delete(void*) { --members; }
};
int main() {
  void* raw_scalar = ::operator new(sizeof(int));
  if (scalars != 1 || arrays != 0 || members != 0) return 11;
  ::operator delete(raw_scalar);
  void* raw_array = ::operator new[](3 * sizeof(int));
  if (scalars != 0 || arrays != 1 || members != 0) return 12;
  ::operator delete[](raw_array);
  if (scalars != 0 || arrays != 0 || members != 0) return 13;
  try { new T((new V[1], 1)); return 1; } catch (int) {}
  if (scalars != 0 || arrays != 0) return 2;
  int* p = new int(4);
  if (*p != 4 || scalars < 0 || scalars > 1 || arrays != 0 || members != 0) return 3;
  delete p;
  if (scalars != 0 || arrays != 0 || members != 0) return 4;

  int* q = new int[3];
  if (scalars != 0 || arrays < 0 || arrays > 1 || members != 0) return 5;
  delete[] q;
  if (scalars != 0 || arrays != 0 || members != 0) return 6;

  M* m = new M;
  if (scalars != 0 || arrays != 0 || members != 1) return 7;
  delete m;
  if (scalars != 0 || arrays != 0 || members != 0) return 8;

  M* g = ::new M;
  if (scalars < 0 || scalars > 1 || arrays != 0 || members != 0) return 9;
  ::delete g;
  return scalars == 0 && arrays == 0 && members == 0 ? 0 : 10;
}
