// Both normal and exceptional exits from a conditional-bound loop preserve cleanup.
static int trace;
struct Guard {
  int tag;
  ~Guard() { trace = trace * 10 + tag; }
};
static const unsigned char data[8] = {1, 2, 3, 4, 5, 6, 7, 8};
__attribute__((noinline)) static unsigned char read_byte(unsigned byte, int fail) {
  if (static_cast<int>(byte) == fail) throw static_cast<int>(byte);
  return data[byte];
}
__attribute__((noinline)) static unsigned long long decode(int form, int fail) {
  Guard guard = {2};
  if (form != 3 && form != 4) return 0;
  unsigned long long value = 0;
  for (unsigned byte = 0; byte < (form == 3 ? 4u : 8u); ++byte)
    value |= static_cast<unsigned long long>(read_byte(byte, fail)) << (byte * 8);
  return value;
}
extern "C" int observe(int form, int fail) {
  trace = 0;
  try {
    Guard guard = {1};
    unsigned long long value = decode(form, fail);
    return static_cast<int>(value & 255) + trace * 100;
  } catch (int byte) {
    return 1000 + trace * 100 + byte;
  }
}
extern "C" int cleanup_trace() { return trace; }
