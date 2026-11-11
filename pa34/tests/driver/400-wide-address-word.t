// An address carried in one aggregate word remains valid after its base spills.
using U = unsigned long;
struct Pair { const unsigned char* pointer; U count; };

__attribute__((noinline)) void disturb(void* memory)
{
  asm volatile("" : : "r"(memory) : "memory");
}

__attribute__((noinline)) U consume(Pair pair)
{
  return *pair.pointer + pair.count;
}

__attribute__((noinline)) U bridge(const unsigned char* base, U count)
{
  disturb(__builtin_alloca((count & 7) + 1));
  asm volatile("" ::: "rbx", "r12", "r13", "r14", "r15");
  return consume(Pair{base + 48, count});
}

int main()
{
  unsigned char data[64] = {};
  for (unsigned i = 0; i != 257; ++i) {
    data[48] = static_cast<unsigned char>(i * 17);
    if (bridge(data, i) != U(data[48]) + i) return 1;
  }
}
