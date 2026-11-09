// N3485 5.3.4p10/5.3.3p6: operator new receives sizeof(T) as std::size_t; a
// class of 0x80000010 bytes requests exactly that (not sign-extended from 32
// bits). A class-specific non-throwing allocator observes the request without
// allocating an enormous object or returning insufficient storage; expect exit 0
static unsigned long requested;
struct Big {
  char bytes[0x80000010ul];
  static void* operator new(unsigned long n) noexcept { requested = n; return 0; }
};
int main() { Big* b = new Big; return b == 0 && requested == 0x80000010ul ? 0 : 1; }
