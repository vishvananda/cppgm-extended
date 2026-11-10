// Normal return alternatives preserve calls, signed extension and nonpower controls.
using U=unsigned long;
volatile U ticks=0;
extern "C" __attribute__((noinline)) unsigned align_value(unsigned selector) {
  ++ticks;
  switch(selector%7){case 0:return 1; case 1:return 2; case 2:return 4; case 3:return 8; case 4:return selector&32?16:32; case 5:return 64; default:return 128;}
}
extern "C" __attribute__((noinline)) unsigned sometimes_nonpower(unsigned selector) { return selector==2?3:4; }
extern "C" __attribute__((noinline)) unsigned sometimes_zero(unsigned selector) { return selector==7?0:4; }
extern "C" __attribute__((noinline,weak)) unsigned weak_alignment(unsigned selector) { return 4; }
extern "C" __attribute__((noinline)) signed char signed_alignment(unsigned selector) { return selector&1?-128:2; }
extern "C" __attribute__((noinline)) U round_up(U x,unsigned selector) {U a=align_value(selector);return x?((x-1)/a+1)*a:0;}
extern "C" __attribute__((noinline)) U divide_value(U x,unsigned selector) {return x/U(align_value(selector));}
extern "C" __attribute__((noinline)) U modulo_value(U x,unsigned selector) {return x%U(align_value(selector));}
extern "C" __attribute__((noinline)) U divide_nonpower(U x,unsigned selector) {return x/U(sometimes_nonpower(selector));}
extern "C" __attribute__((noinline)) U divide_maybe_zero(U x,unsigned selector) {return x/U(sometimes_zero(selector));}
extern "C" __attribute__((noinline)) U divide_weak(U x,unsigned selector) {return x/U(weak_alignment(selector));}
extern "C" __attribute__((noinline)) U divide_signed_byte(U x,unsigned selector) {return x/U(signed_alignment(selector));}
int main(){U h=0, seed=1; for(unsigned i=0;i<65536;++i){seed=seed*6364136223846793005UL+1;U x=i<4?(i==0?0:i==1?1:i==2?~0UL:U(1)<<63):seed;h=h*31+round_up(x,i);h=h*31+divide_value(x,i);h=h*31+modulo_value(x,i);h=h*31+divide_nonpower(x,i%6);h=h*31+divide_maybe_zero(x,i%7);h=h*31+divide_weak(x,i);h=h*31+divide_signed_byte(x,i);}return h != 6345242704293526140UL || ticks != 196608UL;}
