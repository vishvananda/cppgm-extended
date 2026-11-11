template<unsigned N> struct Bytes { unsigned char data[N]; };
#define EACH_SIZE(M) M(1) M(2) M(3) M(4) M(5) M(6) M(7) M(8) M(9) M(10) M(11) M(12) M(13) M(14) M(15) M(16)
#define DECLARE(N) \
  extern "C" Bytes<N> host##N(Bytes<N>); \
  extern "C" Bytes<N> course##N(Bytes<N>); \
  extern "C" const Bytes<N>* source##N(unsigned); \
  extern "C" Bytes<N> (*choose##N())(Bytes<N>);
EACH_SIZE(DECLARE)
#undef DECLARE
extern "C" int host_check(unsigned);

// Seven integer arguments put one word on the stack before the aggregate.
// The aggregate follows at an eight-byte boundary, without i128's extra padding.
#define STACK_ARGS unsigned long a, unsigned long b, unsigned long c, unsigned long d, \
                   unsigned long e, unsigned long f, unsigned long g, Bytes<16> value, unsigned long tail
extern "C" Bytes<16> host_stack(STACK_ARGS);
extern "C" Bytes<16> course_stack(STACK_ARGS);
