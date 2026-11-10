// Typed table offsets, narrow mask flags and an address live past its load.
extern "C" {
__attribute__((noinline)) int masked8(unsigned char x) {
  unsigned char v = x & 63;
  return v ? v : 1000;
}
__attribute__((noinline)) int masked16(unsigned short x) {
  unsigned short v = x & 127;
  return v ? v : 1000;
}
__attribute__((noinline)) int signed8(signed char x) {
  signed char v = static_cast<signed char>(x & 128);
  return v < 0 ? v : 1000;
}
__attribute__((noinline)) int signed16(short x) {
  short v = static_cast<short>(x & 32768);
  return v < 0 ? v : 1000;
}
}

signed char offsets[] = {16, 24, 16, 24, 16, 24, 16, 24};
struct Node { unsigned short tag; char pad[14]; Node *left, *right; };
__attribute__((noinline)) Node *walk(Node *p) {
  signed char off = offsets[p->tag & 7];
  return *reinterpret_cast<Node **>(reinterpret_cast<char *>(p) + off);
}
__attribute__((noinline)) signed char *live_address(unsigned key, signed char *out) {
  signed char *at = offsets + key;
  *out = *at;
  return at;
}

int main() {
  for (unsigned i = 0; i < 65536; ++i) {
    if (masked8(static_cast<unsigned char>(i)) != ((i & 63) ? int(i & 63) : 1000)) return 1;
    if (masked16(static_cast<unsigned short>(i)) != ((i & 127) ? int(i & 127) : 1000)) return 2;
    if (signed8(static_cast<signed char>(i)) != ((i & 128) ? -128 : 1000)) return 3;
    if (signed16(static_cast<short>(i)) != ((i & 32768) ? -32768 : 1000)) return 4;
  }
  Node a = {}, b = {}, root = {};
  root.left = &a; root.right = &b;
  for (unsigned i = 0; i < 256; ++i) {
    root.tag = static_cast<unsigned short>(i);
    if (walk(&root) != ((i & 1) ? &b : &a)) return 5;
    signed char out = 0;
    unsigned key = i & 7;
    if (live_address(key, &out) != offsets + key || out != offsets[key]) return 6;
  }
  return 0;
}
