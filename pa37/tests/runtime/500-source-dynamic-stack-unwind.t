extern "C" unsigned stack_position();
static __attribute__((noinline)) void may_throw(int value) {
 volatile int scratch[37] = {value};
 if (scratch[0]) throw value;
}
extern "C" int observe(unsigned size) {
 volatile unsigned char* live = static_cast<unsigned char*>(__builtin_alloca(size));
 live[0] = 17;
 live[size - 1] = 91;
 unsigned saved = stack_position();
 for (int i = 1; i != 5; ++i) {
  try { may_throw(i); }
  catch (int value) { if (value != i) return 1; }
  if (stack_position() != saved) return 2;
  if (live[0] != 17 || live[size - 1] != 91) return 3;
 }
 return 17;
}
