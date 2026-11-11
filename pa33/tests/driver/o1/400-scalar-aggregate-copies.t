using U = unsigned long long;
struct Pair { U low, high; };

extern "C" __attribute__((noinline)) Pair edit8(Pair value, const unsigned char* source)
{
  __builtin_memcpy(reinterpret_cast<unsigned char*>(&value) + 4, source, 8);
  return value;
}

extern "C" __attribute__((noinline)) Pair edit12(Pair value, const unsigned char* source)
{
  __builtin_memcpy(&value, source, 12);
  return value;
}

struct Node { unsigned value; };
struct Iterator {
  Node* node;
  unsigned index;
  Iterator(Node* n, unsigned i) : node(n), index(i) {}
};
struct Range {
  Iterator first, last;
  Range(Iterator a, Iterator b) : first(a), last(b) {}
};
__attribute__((noinline)) unsigned count(Node* node) { return node->value ^ 0x98765432U; }
extern "C" __attribute__((noinline)) Range children(Node* node)
{
  return Range(Iterator(node, 0), Iterator(node, node ? count(node) : 0));
}

int main()
{
  unsigned char source[12];
  for (unsigned trial = 0; trial != 257; ++trial) {
    Pair value = {0xabcdef1234567890ULL + trial, 0x123456789abcdef0ULL - trial};
    unsigned char original[16];
    __builtin_memcpy(original, &value, 16);
    for (unsigned i = 0; i != 12; ++i) source[i] = static_cast<unsigned char>(trial * 31 + i * 17);
    Pair a = edit8(value, source), b = edit12(value, source);
    const unsigned char* ap = reinterpret_cast<const unsigned char*>(&a);
    const unsigned char* bp = reinterpret_cast<const unsigned char*>(&b);
    for (unsigned i = 0; i != 16; ++i) {
      if (ap[i] != (i >= 4 && i < 12 ? source[i - 4] : original[i])) return 1;
      if (bp[i] != (i < 12 ? source[i] : original[i])) return 2;
    }
    Node node = {trial * 1664525U};
    Range r = children(&node);
    if (r.first.node != &node || r.last.node != &node || r.first.index || r.last.index != count(&node)) return 3;
  }
  Range empty = children(nullptr);
  return empty.first.node || empty.last.node || empty.first.index || empty.last.index;
}
