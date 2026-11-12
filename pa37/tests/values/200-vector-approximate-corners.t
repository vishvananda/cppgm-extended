using Word = unsigned int;
using Words = Word __attribute__((vector_size(16)));
using Singles = float __attribute__((vector_size(16)));
static const Word inputs[][4] = {
 {0,0x80000000u,1,0x80000001u},
 {0x7f800000u,0xff800000u,0x7fc12345u,0x7f812345u},
 {0x3f800000u,0x40800000u,0xbf800000u,0xc0800000u},
 {0x00800000u,0x7f7fffffu,0x40000000u,0x3e800000u},
 {0xff7fffffu,0x80800000u,0xbf000000u,0xffc54321u}
};
static bool result_ok(Word input, Word output, bool root) {
 Word magnitude = input & 0x7fffffffu;
 Word sign = input & 0x80000000u;
 if (magnitude > 0x7f800000u) return output == (input | 0x00400000u);
 if (magnitude < 0x00800000u) return output == (sign | 0x7f800000u);
 if (root && sign) return (output & 0x7fffffffu) > 0x7f800000u;
 if (magnitude == 0x7f800000u) return output == sign;
 if (!root && magnitude > 0x7e800000u) return output == sign;
 float a, y;
 __builtin_memcpy(&a,&input,4); __builtin_memcpy(&y,&output,4);
 double error = root ? double(y)*double(y)*double(a)-1.0 : double(y)*double(a)-1.0;
 if (error < 0) error = -error;
 return error <= 0.0009765625;
}
extern "C" int observe() {
 for (unsigned row = 0; row != 5; ++row)
  for (unsigned rotate = 0; rotate != 4; ++rotate) {
   Words bits = {inputs[row][rotate],inputs[row][(rotate+1)&3],inputs[row][(rotate+2)&3],inputs[row][(rotate+3)&3]};
   Singles value; __builtin_memcpy(&value,&bits,16);
   Singles computed[4] = {__builtin_ia32_rcpps(value),__builtin_ia32_rsqrtps(value),
    __builtin_ia32_rcpss(value),__builtin_ia32_rsqrtss(value)};
   for (unsigned operation = 0; operation != 4; ++operation) {
    Words actual; __builtin_memcpy(&actual,&computed[operation],16);
    for (unsigned lane = 0; lane != 4; ++lane)
     if (operation >= 2 && lane != 0) {
      if (actual[lane] != bits[lane]) return 1;
     } else if (!result_ok(bits[lane],actual[lane],(operation&1)!=0)) return 2;
   }
  }
 return 17;
}
