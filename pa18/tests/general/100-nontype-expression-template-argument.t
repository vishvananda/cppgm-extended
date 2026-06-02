typedef unsigned long uintptr_t;

template<class T, T v>
struct integral_constant {};

struct X {
  typedef integral_constant<uintptr_t, (1ULL << ((__CHAR_BIT__ * sizeof(uintptr_t)) - 1))> B;
};

int main() {
  return 0;
}
