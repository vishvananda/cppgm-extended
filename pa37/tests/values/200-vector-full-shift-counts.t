using Words = short __attribute__((vector_size(16)));
using Quads = long long __attribute__((vector_size(16)));
extern "C" int observe() {
 const unsigned long long counts[] = {0,1,15,16,31,32,63,64,255,0x100000000ULL,~0ULL};
 Words value = {-32768,32767,-1,0,1,17,-12,99};
 for (unsigned k = 0; k != sizeof(counts)/sizeof(counts[0]); ++k) {
  unsigned long long count = counts[k];
  Words packed_count = (Words)(Quads){static_cast<long long>(count),17};
  Words left = __builtin_ia32_psllw128(value,packed_count);
  Words right = __builtin_ia32_psrlw128(value,packed_count);
  Words arithmetic = __builtin_ia32_psraw128(value,packed_count);
  for (unsigned i = 0; i != 8; ++i) {
   unsigned short a = static_cast<unsigned short>(value[i]);
   unsigned short l = count >= 16 ? 0 : static_cast<unsigned short>(a << count);
   unsigned short r = count >= 16 ? 0 : static_cast<unsigned short>(a >> count);
   short s = count >= 16 ? (value[i] < 0 ? -1 : 0) : static_cast<short>(value[i] >> count);
   if (static_cast<unsigned short>(left[i]) != l || static_cast<unsigned short>(right[i]) != r || arithmetic[i] != s)
    return 1;
  }
 }
 return 17;
}
